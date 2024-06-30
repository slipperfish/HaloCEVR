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
}

void Game::PreDrawFrame(struct Renderer* renderer, float deltaTime)
{
	lastDeltaTime = deltaTime;

	renderState = ERenderState::UNKNOWN;

	//CalcFPS(deltaTime);

	vr->SetMouseVisibility(Helpers::IsMouseVisible());
	vr->UpdatePoses();

	UpdateCrosshair();

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
	Helpers::GetDirect3DDevice9()->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(25, 0, 0, 0), 1.0f, 0);
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

	// For performance reasons, we should prevent the game from calling SceneStart/SceneEnd for each eye
	if (eye == 0)
	{
		reinterpret_cast<IDirect3DDevice9ExWrapper*>(Helpers::GetDirect3DDevice9())->bIgnoreNextEnd = true;
	}
	else
	{
		reinterpret_cast<IDirect3DDevice9ExWrapper*>(Helpers::GetDirect3DDevice9())->bIgnoreNextStart = true;
	}

	vr->UpdateCameraFrustum(&renderer->frustum, eye);
	vr->UpdateCameraFrustum(&renderer->frustum2, eye);

	RenderTarget* primaryRenderTarget = Helpers::GetRenderTargets();

	primaryRenderTarget[0].renderSurface = vr->GetRenderSurface(eye);
	primaryRenderTarget[0].renderTexture = vr->GetRenderTexture(eye);
	primaryRenderTarget[0].width = vr->GetViewWidth();
	primaryRenderTarget[0].height = vr->GetViewHeight();

	debug.ExtractMatrices(renderer);
}


void Game::PostDrawEye(struct Renderer* renderer, float deltaTime, int eye)
{
	// UI is usually drawn via an overlay, emulate it in flat (intentionally squashed)

#if EMULATE_VR
	RECT targetRect{ 0, 0, 200, 200 };
	Helpers::GetDirect3DDevice9()->StretchRect(uiSurface, NULL, Helpers::GetRenderTargets()[0].renderSurface, &targetRect, D3DTEXF_NONE);

	targetRect = { 400, 0, 600, 200 };
	Helpers::GetDirect3DDevice9()->StretchRect(crosshairSurface, NULL, Helpers::GetRenderTargets()[0].renderSurface, &targetRect, D3DTEXF_NONE);
#endif

	debug.Render(Helpers::GetDirect3DDevice9());
}

void Game::PreDrawMirror(struct Renderer* renderer, float deltaTime)
{
	renderState = ERenderState::GAME;

	renderer->frustum = frustum1;
	renderer->frustum2 = frustum2;

	RestoreRenderTargets();

	debug.ExtractMatrices(renderer);
}

void Game::PostDrawMirror(struct Renderer* renderer, float deltaTime)
{
	// Do something here to copy the image into the backbuffer correctly
	debug.Render(Helpers::GetDirect3DDevice9());
}

void Game::PostDrawFrame(struct Renderer* renderer, float deltaTime)
{
	vr->PostDrawFrame(renderer, deltaTime);
}

bool Game::PreDrawHUD()
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

void Game::PostDrawHUD()
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
	// Unlike pre/post ui calls we don't need to check render state
	// This code path only gets called if the UI needs rendering

	crosshairRealSurface = Helpers::GetRenderTargets()[1].renderSurface;
	if (anchorLocation && *anchorLocation == 4) // Centre = 4
	{
		Helpers::GetRenderTargets()[1].renderSurface = crosshairSurface;
		Helpers::GetDirect3DDevice9()->SetRenderTarget(0, crosshairSurface);
	}
}

void Game::PostDrawCrosshair()
{
	// Unlike pre/post ui calls we don't need to check render state
	// This code path only gets called if the UI needs rendering

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

void Game::UpdateCrosshair()
{
	Vector3 aimPos, aimDir;
	bool bHasCrosshair = weaponHandler.GetLocalWeaponAim(aimPos, aimDir);

	if (!bHasCrosshair)
	{
		return;
	}

	Matrix4 overlayTransform;

	Vector3 targetPos = aimPos + aimDir * c_UIOverlayDistance->Value();

	Vector3 hmdPos = vr->GetHMDTransform(true) * Vector3(0.0f, 0.0f, 0.0f);

	overlayTransform.translate(targetPos);
	overlayTransform.lookAt(hmdPos, Vector3(0.0f, 0.0f, 1.0f));
	
	overlayTransform.translate(-targetPos);
	overlayTransform.rotate(90.0f, overlayTransform.getUpAxis());
	overlayTransform.rotate(-90.0f, overlayTransform.getLeftAxis());
	overlayTransform.translate(targetPos);

	vr->SetCrosshairTransform(overlayTransform);
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
