#define EMULATE_VR 0
#include "Game.h"
#include "Logger.h"
#include "Hooking/Hooks.h"
#include "Helpers/DX9.h"
#include "Helpers/RenderTarget.h"
#include "Helpers/Renderer.h"
#include "Helpers/Camera.h"
#include "Helpers/Controls.h"
#include "Helpers/Menus.h"
#include "Helpers/Objects.h"
#include "Helpers/Maths.h"
#include "Helpers/Assets.h"

#if EMULATE_VR
#include "VR/VREmulator.h"
#else
#include "VR/OpenVR.h"
#endif
#include "DirectXWrappers/IDirect3DDevice9ExWrapper.h"


void Game::Init()
{
	Logger::log << "HaloCEVR initialising..." << std::endl;

	SetupConfigs();

	CreateConsole();

	PatchGame();

#if EMULATE_VR
	vr = new VREmulator();
#else
	vr = new OpenVR();
#endif

	vr->Init();

	inputHandler.RegisterInputs();

	backBufferWidth = vr->GetViewWidth();
	backBufferHeight = vr->GetViewHeight();

	Logger::log << "HaloCEVR initialised" << std::endl;
}

void Game::Shutdown()
{
	Logger::log << "HaloCEVR shutting down..." << std::endl;

	MH_STATUS hookStatus = MH_DisableHook(MH_ALL_HOOKS);

	if (hookStatus != MH_OK)
	{
		Logger::log << "Could not remove hooks: " << MH_StatusToString(hookStatus) << std::endl;
	}

	hookStatus = MH_Uninitialize();

	if (hookStatus != MH_OK)
	{
		Logger::log << "Could not uninitialise MinHook: " << MH_StatusToString(hookStatus) << std::endl;
	}

	if (c_ShowConsole && c_ShowConsole->Value())
	{
		if (consoleOut)
		{
			fclose(consoleOut);
		}
		FreeConsole();
	}
}

void Game::OnInitDirectX()
{
	Logger::log << "Game has finished DirectX initialisation" << std::endl;

	if (!Helpers::GetDirect3DDevice9())
	{
		Logger::log << "Couldn't get game's direct3d device" << std::endl;
		return;
	}

	SetForegroundWindow(GetActiveWindow());

	vr->OnGameFinishInit();

	uiSurface = vr->GetUISurface();
	crosshairSurface = vr->GetCrosshairSurface();

	scopeSurfaces[0] = vr->GetScopeSurface();
	scopeTextures[0] = vr->GetScopeTexture();

	D3DSURFACE_DESC desc;
	scopeSurfaces[0]->GetDesc(&desc);

	CreateTextureAndSurface(desc.Width, desc.Height, desc.Usage, desc.Format, &scopeSurfaces[1], &scopeTextures[1]);
	CreateTextureAndSurface(desc.Width / 2, desc.Height / 2, desc.Usage, desc.Format, &scopeSurfaces[2], &scopeTextures[2]);
}

void Game::PreDrawFrame(struct Renderer* renderer, float deltaTime)
{
	lastDeltaTime = deltaTime;

	renderState = ERenderState::UNKNOWN;

	//CalcFPS(deltaTime);

	vr->SetMouseVisibility(Helpers::IsMouseVisible());
	vr->UpdatePoses();

	UpdateCrosshairAndScope();

	StoreRenderTargets();

	sRect* window = Helpers::GetWindowRect();
	window->top = 0;
	window->left = 0;
	window->right = vr->GetViewWidth();
	window->bottom = vr->GetViewHeight();

	// Clear UI surfaces
	IDirect3DSurface9* currentSurface = nullptr;
	Helpers::GetDirect3DDevice9()->GetRenderTarget(0, &currentSurface);
	Helpers::GetDirect3DDevice9()->SetRenderTarget(0, uiSurface);
	Helpers::GetDirect3DDevice9()->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
	Helpers::GetDirect3DDevice9()->SetRenderTarget(0, crosshairSurface);
	Helpers::GetDirect3DDevice9()->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
	Helpers::GetDirect3DDevice9()->SetRenderTarget(0, scopeSurfaces[0]);
	Helpers::GetDirect3DDevice9()->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(255, 0, 0, 0), 1.0f, 0);
	Helpers::GetDirect3DDevice9()->SetRenderTarget(0, currentSurface);
	currentSurface->Release();

	frustum1 = renderer->frustum;
	frustum2 = renderer->frustum2;

	if (bNeedsRecentre)
	{
		bNeedsRecentre = false;
		vr->Recentre();
	}

	vr->PreDrawFrame(renderer, deltaTime);
}

void Game::PreDrawEye(Renderer* renderer, float deltaTime, int eye)
{
	renderState = eye == 0 ? ERenderState::LEFT_EYE : ERenderState::RIGHT_EYE;

	renderer->frustum = frustum1;
	renderer->frustum2 = frustum2;

	vr->UpdateCameraFrustum(&renderer->frustum, eye);
	vr->UpdateCameraFrustum(&renderer->frustum2, eye);

	RenderTarget* primaryRenderTarget = Helpers::GetRenderTargets();

	primaryRenderTarget[0].renderSurface = vr->GetRenderSurface(eye);
	primaryRenderTarget[0].renderTexture = vr->GetRenderTexture(eye);
	primaryRenderTarget[0].width = vr->GetViewWidth();
	primaryRenderTarget[0].height = vr->GetViewHeight();

	inGameRenderer.ExtractMatrices(renderer);
}


void Game::PostDrawEye(struct Renderer* renderer, float deltaTime, int eye)
{
	inGameRenderer.Render(Helpers::GetDirect3DDevice9());
}

bool Game::PreDrawScope(Renderer* renderer, float deltaTime)
{
	UnitDynamicObject* player = static_cast<UnitDynamicObject*>(Helpers::GetLocalPlayer());

	if (!player || player->zoom == -1)
	{
		return false;
	}

	renderState = ERenderState::SCOPE;

	renderer->frustum = frustum1;
	renderer->frustum2 = frustum2;

	Vector3 aimPos, aimDir, upDir;

	weaponHandler.GetWorldWeaponAim(aimPos, aimDir);

	upDir = Vector3(0.0f, 0.0f, 1.0f);
	upDir = aimDir.cross(upDir);
	upDir = upDir.cross(aimDir);

	renderer->frustum.position = aimPos;
	renderer->frustum2.position = aimPos;
	renderer->frustum.facingDirection = aimDir;
	renderer->frustum2.facingDirection = aimDir;
	renderer->frustum.upDirection = upDir;
	renderer->frustum2.upDirection = upDir;

	RenderTarget* primaryRenderTarget = Helpers::GetRenderTargets();

	primaryRenderTarget[0].renderSurface = scopeSurfaces[0];
	primaryRenderTarget[0].renderTexture = scopeTextures[0];
	primaryRenderTarget[0].width = vr->GetScopeWidth();
	primaryRenderTarget[0].height = vr->GetScopeHeight();
	primaryRenderTarget[1].renderSurface = scopeSurfaces[1];
	primaryRenderTarget[1].renderTexture = scopeTextures[1];
	primaryRenderTarget[1].width = vr->GetScopeWidth();
	primaryRenderTarget[1].height = vr->GetScopeHeight();
	primaryRenderTarget[2].renderSurface = scopeSurfaces[2];
	primaryRenderTarget[2].renderTexture = scopeTextures[2];
	primaryRenderTarget[2].width = vr->GetScopeWidth() / 2;
	primaryRenderTarget[2].height = vr->GetScopeHeight() / 2;


	sRect* windowMain = Helpers::GetWindowRect();
	windowMain->top = 0;
	windowMain->left = 0;
	windowMain->right = vr->GetScopeWidth();
	windowMain->bottom = vr->GetScopeHeight();

	{
		sRect& window = renderer->frustum.WindowViewport;
		window.top = 0;
		window.left = 0;
		window.right = vr->GetScopeWidth();
		window.bottom = vr->GetScopeHeight();

		sRect& window2 = renderer->frustum2.WindowViewport;
		window2.top = 0;
		window2.left = 0;
		window2.right = vr->GetScopeWidth();
		window2.bottom = vr->GetScopeHeight();
	}

	return true;
}

void Game::PostDrawScope(Renderer* renderer, float deltaTime)
{
	RestoreRenderTargets();
}

void Game::PreDrawMirror(struct Renderer* renderer, float deltaTime)
{
	renderState = ERenderState::GAME;

	renderer->frustum = frustum1;
	renderer->frustum2 = frustum2;

	RestoreRenderTargets();

	sRect* windowMain = Helpers::GetWindowRect();
	windowMain->top = 0;
	windowMain->left = 0;
	windowMain->right = Helpers::GetRenderTargets()[0].width;
	windowMain->bottom = Helpers::GetRenderTargets()[0].height;

	inGameRenderer.ExtractMatrices(renderer);
}

void Game::PostDrawMirror(struct Renderer* renderer, float deltaTime)
{
	// Do something here to copy the image into the backbuffer correctly
	inGameRenderer.Render(Helpers::GetDirect3DDevice9());
}

void Game::PostDrawFrame(struct Renderer* renderer, float deltaTime)
{
	RestoreRenderTargets();
	vr->PostDrawFrame(renderer, deltaTime);
	inGameRenderer.PostRender();
}

bool Game::PreDrawHUD()
{
	// Only render UI once per frame
	if (GetRenderState() != ERenderState::LEFT_EYE)
	{
		// Remove zoom effect from game view
		if (GetRenderState() == ERenderState::GAME)
		{
			short* zoom = &Helpers::GetInputData().zoomLevel;
			realZoom = *zoom;
			*zoom = -1;
		}

		// ...but try to avoid breaking the game view (for now at least)
		return GetRenderState() == ERenderState::GAME || GetRenderState() == ERenderState::SCOPE;
	}

	short* zoom = &Helpers::GetInputData().zoomLevel;
	realZoom = *zoom;
	*zoom = -1;

	Helpers::GetDirect3DDevice9()->GetRenderTarget(0, &uiRealSurface);
	Helpers::GetDirect3DDevice9()->SetRenderTarget(0, uiSurface);
	uiRealSurface = Helpers::GetRenderTargets()[1].renderSurface;
	Helpers::GetRenderTargets()[1].renderSurface = uiSurface;

	return true;
}

void Game::PostDrawHUD()
{
	// Only render UI once per frame
	if (GetRenderState() != ERenderState::LEFT_EYE)
	{
		// Remove zoom effect from game view
		if (GetRenderState() == ERenderState::GAME)
		{
			short* zoom = &Helpers::GetInputData().zoomLevel;
			*zoom = realZoom;
		}

		return;
	}

	short* zoom = &Helpers::GetInputData().zoomLevel;
	*zoom = realZoom;

	Helpers::GetRenderTargets()[1].renderSurface = uiRealSurface;
	Helpers::GetDirect3DDevice9()->SetRenderTarget(0, uiRealSurface);
	uiRealSurface->Release();
}

bool Game::PreDrawMenu()
{
	// Only render UI once per frame
	if (GetRenderState() != ERenderState::LEFT_EYE)
	{
		// ...but try to avoid breaking the game view (for now at least)
		return GetRenderState() == ERenderState::GAME;
	}

	Helpers::GetDirect3DDevice9()->GetRenderTarget(0, &uiRealSurface);
	Helpers::GetDirect3DDevice9()->SetRenderTarget(0, uiSurface);
	uiRealSurface = Helpers::GetRenderTargets()[1].renderSurface;
	Helpers::GetRenderTargets()[1].renderSurface = uiSurface;

	return true;
}

void Game::PostDrawMenu()
{
	// Only render UI once per frame
	if (GetRenderState() != ERenderState::LEFT_EYE)
	{
		return;
	}

	Helpers::GetRenderTargets()[1].renderSurface = uiRealSurface;
	Helpers::GetDirect3DDevice9()->SetRenderTarget(0, uiRealSurface);
	uiRealSurface->Release();
}


bool Game::PreDrawLoading(int param1, struct Renderer* renderer)
{
	// Only render UI once per frame
	if (GetRenderState() != ERenderState::LEFT_EYE)
	{
		// ...but try to avoid breaking the game view (for now at least)
		return GetRenderState() == ERenderState::GAME;
	}

	uiRealSurface = Helpers::GetRenderTargets()[1].renderSurface;
	Helpers::GetRenderTargets()[1].renderSurface = uiSurface;

	return true;
}

void Game::PostDrawLoading(int param1, struct Renderer* renderer)
{
	// Only render UI once per frame
	if (GetRenderState() != ERenderState::LEFT_EYE)
	{
		return;
	}

	Helpers::GetRenderTargets()[1].renderSurface = uiRealSurface;
}

void Game::PreDrawCrosshair(short* anchorLocation)
{
	// Draw things normally for the scope
	if (GetRenderState() == ERenderState::SCOPE)
	{
		return;
	}

	crosshairRealSurface = Helpers::GetRenderTargets()[1].renderSurface;
	if (anchorLocation && *anchorLocation == 4) // Centre = 4
	{
		Helpers::GetRenderTargets()[1].renderSurface = crosshairSurface;
		Helpers::GetDirect3DDevice9()->SetRenderTarget(0, crosshairSurface);
	}
}

void Game::PostDrawCrosshair()
{
	// Draw things normally for the scope
	if (GetRenderState() == ERenderState::SCOPE)
	{
		return;
	}

	Helpers::GetRenderTargets()[1].renderSurface = crosshairRealSurface;
	Helpers::GetDirect3DDevice9()->SetRenderTarget(0, crosshairRealSurface);
}

void Game::UpdateViewModel(HaloID& id, Vector3* pos, Vector3* facing, Vector3* up, TransformQuat* BoneTransforms, Transform* OutBoneTransforms)
{
	weaponHandler.UpdateViewModel(id, pos, facing, up, BoneTransforms, OutBoneTransforms);
}

void Game::PreFireWeapon(HaloID& weaponID, short param2, bool param3)
{
	weaponHandler.PreFireWeapon(weaponID, param2, param3);
}

void Game::PostFireWeapon(HaloID& weaponID, short param2, bool param3)
{
	weaponHandler.PostFireWeapon(weaponID, param2, param3);
}

void Game::UpdateInputs()
{
	inputHandler.UpdateInputs();
}


void Game::UpdateCamera(float& yaw, float& pitch)
{
	// Don't bother simulating inputs if we aren't actually in vr
#if EMULATE_VR
	return;
#endif
	inputHandler.UpdateCamera(yaw, pitch);
}

void Game::SetMousePosition(int& x, int& y)
{
	// Don't bother simulating inputs if we aren't actually in vr
#if EMULATE_VR
	return;
#endif
	inputHandler.SetMousePosition(x, y);
}

void Game::UpdateMouseInfo(MouseInfo* mouseInfo)
{
	// Don't bother simulating inputs if we aren't actually in vr
#if EMULATE_VR
	return;
#endif
	inputHandler.UpdateMouseInfo(mouseInfo);
}

void Game::SetViewportScale(Viewport* viewport)
{
	float width = vr->GetViewWidthStretch();
	float height = vr->GetViewHeightStretch();

	viewport->left = -width;
	viewport->right = width;
	viewport->bottom = height;
	viewport->top = -height;
}

float Game::MetresToWorld(float m)
{
	return m / 3.048f;
}

float Game::WorldToMetres(float w)
{
	return w * 3.048f;
}

void Game::CreateConsole()
{
	if (!c_ShowConsole || !c_ShowConsole->Value())
	{
		return;
	}

	AllocConsole();
	freopen_s(&consoleOut, "CONOUT$", "w", stdout);
	std::cout.clear();
}

void Game::PatchGame()
{
	MH_STATUS hookStatus;

	if ((hookStatus = MH_Initialize()) != MH_OK)
	{
		Logger::log << "Could not initialise MinHook: " << MH_StatusToString(hookStatus) << std::endl;
	}
	else
	{
		Logger::log << "Creating hooks" << std::endl;
		Hooks::InitHooks();
		Logger::log << "Enabling hooks" << std::endl;
		Hooks::EnableAllHooks();
	}
}

void Game::SetupConfigs()
{
	Config config;

	// Put all mod configs here
	c_ShowConsole = config.RegisterBool("ShowConsole", "Create a console window at launch for debugging purposes", false);
	c_DrawMirror = config.RegisterBool("DrawMirror", "Update the desktop window display to show the current game view, rather than leaving it on the splash screen", true);
	c_UIOverlayDistance = config.RegisterFloat("UIOverlayDistance", "Distance in metres in front of the HMD to display the UI", 15.0f);
	c_UIOverlayScale = config.RegisterFloat("UIOverlayScale", "Width of the UI overlay in metres", 10.0f);
	c_UIOverlayCurvature = config.RegisterFloat("UIOverlayCurvature", "Curvature of the UI Overlay, on a scale of 0 to 1", 0.1f);
	c_UIOverlayWidth = config.RegisterInt("UIOverlayWidth", "Width of the UI overlay in pixels", 600);
	c_UIOverlayHeight = config.RegisterInt("UIOverlayHeight", "Height of the UI overlay in pixels", 600);
	c_SnapTurn = config.RegisterBool("SnapTurn", "The look input will instantly rotate the view by a fixed amount, rather than smoothly rotating", true);
	c_SnapTurnAmount = config.RegisterFloat("SnapTurnAmount", "Rotation in degrees a single snap turn will rotate the view by", 45.0f);
	c_SmoothTurnAmount = config.RegisterFloat("SmoothTurnAmount", "Rotation in degrees per second the view will turn at when not using snap turning", 90.0f);
	c_LeftHandFlashlightDistance = config.RegisterFloat("LeftHandFlashlight", "Bringing the left hand within this distance of the head will toggle the flashlight (<0 to disable)", 0.2f);
	c_RightHandFlashlightDistance = config.RegisterFloat("RightHandFlashlight", "Bringing the right hand within this distance of the head will toggle the flashlight (<0 to disable)", -1.0f);
	c_ControllerOffsetX = config.RegisterFloat("ControllerOffset.X", "Offset from the controller's position used when calculating the in game hand position", -0.045f);
	c_ControllerOffsetY = config.RegisterFloat("ControllerOffset.Y", "Offset from the controller's position used when calculating the in game hand position", 0.01f);
	c_ControllerOffsetZ = config.RegisterFloat("ControllerOffset.Z", "Offset from the controller's position used when calculating the in game hand position", 0.0f);
	c_ControllerRotationX = config.RegisterFloat("ControllerRotation.X", "Rotation added to the controller when calculating the in game hand rotation", 10.0f);
	c_ControllerRotationY = config.RegisterFloat("ControllerRotation.Y", "Rotation added to the controller when calculating the in game hand rotation", 0.0f);
	c_ControllerRotationZ = config.RegisterFloat("ControllerRotation.Z", "Rotation added to the controller when calculating the in game hand rotation", 0.0f);
	c_ScopeRenderScale = config.RegisterFloat("ScopeRenderScale", "Size of the scope render target, expressed as a proportion of the headset's render scale (e.g. 0.5 = half resolution)", 0.75f);
	// Do we want to expose this to users?
	//c_ScopeScale = config.RegisterFloat("ScopeScale", "Width of the scope view in metres", 0.05f);

	config.LoadFromFile("VR/config.txt");
	config.SaveToFile("VR/config.txt");

	weaponHandler.localOffset = Vector3(c_ControllerOffsetX->Value(), c_ControllerOffsetY->Value(), c_ControllerOffsetZ->Value());
	weaponHandler.localRotation = Vector3(c_ControllerRotationX->Value(), c_ControllerRotationY->Value(), c_ControllerRotationZ->Value());

	Logger::log << "Loaded configs" << std::endl;
}

void Game::CalcFPS(float deltaTime)
{
	fpsTracker.framesSinceFPSUpdate++;
	fpsTracker.timeSinceFPSUpdate += deltaTime;

	if (fpsTracker.timeSinceFPSUpdate > 1.0f)
	{
		fpsTracker.timeSinceFPSUpdate = 0.0f;
		fpsTracker.fps = fpsTracker.framesSinceFPSUpdate;
		fpsTracker.framesSinceFPSUpdate = 0;
		Logger::log << fpsTracker.fps << std::endl;
	}
}

void Game::UpdateCrosshairAndScope()
{
	Vector3 aimPos, aimDir;
	bool bHasCrosshair = weaponHandler.GetLocalWeaponAim(aimPos, aimDir);

	if (!bHasCrosshair)
	{
		return;
	}

	auto fixupRotation = [](Matrix4& m, Vector3& pos) {
		m.translate(-pos);
		m.rotate(90.0f, m.getUpAxis());
		m.rotate(-90.0f, m.getLeftAxis());
		m.translate(pos);
	};

	Matrix4 overlayTransform;

	Vector3 targetPos = aimPos + aimDir * c_UIOverlayDistance->Value();

	Vector3 hmdPos = vr->GetHMDTransform(true) * Vector3(0.0f, 0.0f, 0.0f);

	overlayTransform.translate(targetPos);
	overlayTransform.lookAt(hmdPos, Vector3(0.0f, 0.0f, 1.0f));
	
	fixupRotation(overlayTransform, targetPos);

	vr->SetCrosshairTransform(overlayTransform);
	overlayTransform.identity();

	short zoom = Helpers::GetInputData().zoomLevel;

	Vector3 upDir = Vector3(0.0f, 0.0f, 1.0f);

	bool bHasScope = (zoom != -1) && weaponHandler.GetLocalWeaponScope(aimPos, aimDir, upDir);

	if (!bHasScope)
	{
		vr->SetScopeTransform(overlayTransform, false);
		return;
	}

	overlayTransform.translate(aimPos);
	overlayTransform.lookAt(aimPos - aimDir, upDir);

	fixupRotation(overlayTransform, aimPos);

	vr->SetScopeTransform(overlayTransform, true);
}

void Game::StoreRenderTargets()
{
	for (int i = 0; i < 8; i++)
	{
		gameRenderTargets[i].width = Helpers::GetRenderTargets()[i].width;
		gameRenderTargets[i].height = Helpers::GetRenderTargets()[i].height;
		gameRenderTargets[i].format = Helpers::GetRenderTargets()[i].format;
		gameRenderTargets[i].renderSurface = Helpers::GetRenderTargets()[i].renderSurface;
		gameRenderTargets[i].renderTexture = Helpers::GetRenderTargets()[i].renderTexture;
	}
}

void Game::RestoreRenderTargets()
{
	for (int i = 0; i < 8; i++)
	{
		Helpers::GetRenderTargets()[i].width = gameRenderTargets[i].width;
		Helpers::GetRenderTargets()[i].height = gameRenderTargets[i].height;
		Helpers::GetRenderTargets()[i].format = gameRenderTargets[i].format;
		Helpers::GetRenderTargets()[i].renderSurface = gameRenderTargets[i].renderSurface;
		Helpers::GetRenderTargets()[i].renderTexture = gameRenderTargets[i].renderTexture;
	}
}

void Game::CreateTextureAndSurface(UINT Width, UINT Height, DWORD Usage, D3DFORMAT Format, IDirect3DSurface9** OutSurface, IDirect3DTexture9** OutTexture)
{
	HRESULT result = Helpers::GetDirect3DDevice9()->CreateTexture(Width, Height, 1, Usage, Format, D3DPOOL_DEFAULT, OutTexture, nullptr);
	if (FAILED(result))
	{
		Logger::err << "[DX9] Failed to create game texture: " << result << std::endl;
		return;
	}

	result = (*OutTexture)->GetSurfaceLevel(0, OutSurface);
	if (FAILED(result))
	{
		Logger::err << "[DX9] Failed to retrieve game surface: " << result << std::endl;
		return;
	}
}
