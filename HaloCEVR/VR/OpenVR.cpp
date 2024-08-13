#include <d3d11.h>
#include <d3d9.h>
#include <algorithm>
#include <filesystem>
#include "OpenVR.h"
#include "../Logger.h"
#include "../Game.h"
#include "../Helpers/DX9.h"
#include "../Helpers/Renderer.h"
#include "../Helpers/RenderTarget.h"
#include "../Helpers/Camera.h"

#pragma comment(lib, "openvr_api.lib")
#pragma comment(lib, "d3d11.lib")

void OpenVR::Init()
{
	Logger::log << "[OpenVR] Initialising OpenVR" << std::endl;

	vr::EVRInitError initError = vr::VRInitError_None;
	vrSystem = vr::VR_Init(&initError, vr::VRApplication_Scene);

	if (initError != vr::VRInitError_None)
	{
		Logger::err << "[OpenVR] VR_Init failed: " << vr::VR_GetVRInitErrorAsEnglishDescription(initError) << std::endl;
		return;
	}

	vrCompositor = vr::VRCompositor();

	if (!vrCompositor)
	{
		Logger::err << "[OpenVR] VRCompositor failed" << std::endl;
		return;
	}

	vrOverlay = vr::VROverlay();

	if (!vrOverlay)
	{
		Logger::err << "[OpenVR] VROverlay failed" << std::endl;
		return;
	}

	vrInput = vr::VRInput();

	if (!vrInput)
	{
		Logger::err << "[OpenVR] VRInput failed" << std::endl;
		return;
	}

	vrOverlay->CreateOverlay("UIOverlay", "UIOverlay", &uiOverlay);
	vrOverlay->SetOverlayFlag(uiOverlay, vr::VROverlayFlags_MakeOverlaysInteractiveIfVisible, true);
	vrOverlay->SetOverlayFlag(uiOverlay, vr::VROverlayFlags_IsPremultiplied, true);
	vrOverlay->ShowOverlay(uiOverlay);

	vrOverlay->CreateOverlay("CrosshairOverlay", "CrosshairOverlay", &crosshairOverlay);
	vrOverlay->SetOverlayFlag(crosshairOverlay, vr::VROverlayFlags_IsPremultiplied, true);
	vrOverlay->ShowOverlay(crosshairOverlay);

	std::filesystem::path cwd = std::filesystem::current_path() / "VR" / "OpenVR" / "actions.json";
	vrInput->SetActionManifestPath(cwd.string().c_str());

	vr::EVRInputError ActionSetError = vrInput->GetActionSetHandle("/actions/default", &actionSets[0].ulActionSet);

	if (ActionSetError != vr::EVRInputError::VRInputError_None)
	{
		Logger::err << "[OpenVR] Could not get action set: " << ActionSetError << std::endl;
	}

	vrSystem->GetRecommendedRenderTargetSize(&recommendedWidth, &recommendedHeight);

	// Voodoo magic to convert normal view frustums into asymmetric ones through selective cropping

	float l_left = 0.0f, l_right = 0.0f, l_top = 0.0f, l_bottom = 0.0f;
	vrSystem->GetProjectionRaw(vr::EVREye::Eye_Left, &l_left, &l_right, &l_top, &l_bottom);

	float r_left = 0.0f, r_right = 0.0f, r_top = 0.0f, r_bottom = 0.0f;
	vrSystem->GetProjectionRaw(vr::EVREye::Eye_Right, &r_left, &r_right, &r_top, &r_bottom);

	float tanHalfFov[2];

	tanHalfFov[0] = (std::max)({ -l_left, l_right, -r_left, r_right });
	tanHalfFov[1] = (std::max)({ -l_top, l_bottom, -r_top, r_bottom });

	textureBounds[0].uMin = 0.5f + 0.5f * l_left / tanHalfFov[0];
	textureBounds[0].uMax = 0.5f + 0.5f * l_right / tanHalfFov[0];
	textureBounds[0].vMin = 0.5f - 0.5f * l_bottom / tanHalfFov[1];
	textureBounds[0].vMax = 0.5f - 0.5f * l_top / tanHalfFov[1];

	textureBounds[1].uMin = 0.5f + 0.5f * r_left / tanHalfFov[0];
	textureBounds[1].uMax = 0.5f + 0.5f * r_right / tanHalfFov[0];
	textureBounds[1].vMin = 0.5f - 0.5f * r_bottom / tanHalfFov[1];
	textureBounds[1].vMax = 0.5f - 0.5f * r_top / tanHalfFov[1];

	aspect = tanHalfFov[0] / tanHalfFov[1];
	fov = 2.0f * atan(tanHalfFov[0]);

	realWidth = recommendedWidth;
	realHeight = recommendedHeight;

	recommendedWidth = static_cast<uint32_t>(recommendedWidth / (std::max)(textureBounds[0].uMax - textureBounds[0].uMin, textureBounds[1].uMax - textureBounds[1].uMin));
	recommendedHeight = static_cast<uint32_t>(recommendedHeight / (std::max)(textureBounds[0].vMax - textureBounds[0].vMin, textureBounds[1].vMax - textureBounds[1].vMin));

	Logger::log << "[OpenVR] Stretched Width/Height from " << realWidth << "x" << realHeight << " to " << recommendedWidth << "x" << recommendedHeight << std::endl;

	Logger::log << "[OpenVR] VR systems created successfully" << std::endl;
}

void OpenVR::OnGameFinishInit()
{
	HRESULT result = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, NULL, NULL, D3D11_SDK_VERSION, &d3dDevice, NULL, NULL);

	if (FAILED(result))
	{
		Logger::log << "[OpenVR] Could not initialise DirectX11: " << result << std::endl;
	}
	
	D3DSURFACE_DESC desc, desc2;
	Helpers::GetRenderTargets()[0].renderSurface->GetDesc(&desc);
	Helpers::GetRenderTargets()[1].renderSurface->GetDesc(&desc2);

	// Create shared textures
	// Eyes
	CreateTexAndSurface(0, recommendedWidth, recommendedHeight, desc.Usage, desc.Format);
	CreateTexAndSurface(1, recommendedWidth, recommendedHeight, desc.Usage, desc.Format);
	// UI Layers
	CreateTexAndSurface(uiSurface, Game::instance.c_UIOverlayWidth->Value(), Game::instance.c_UIOverlayHeight->Value(), desc2.Usage, desc2.Format);
	CreateTexAndSurface(crosshairSurface, Game::instance.c_UIOverlayWidth->Value(), Game::instance.c_UIOverlayHeight->Value(), desc2.Usage, desc2.Format);
	scopeWidth = static_cast<uint32_t>(Game::instance.c_ScopeRenderScale->Value() * recommendedWidth);
	scopeHeight = static_cast<uint32_t>(Game::instance.c_ScopeRenderScale->Value() * recommendedWidth * 0.75f); // Maintain the 4x3 aspect ratio halo works best with
	CreateTexAndSurface(scopeSurface, scopeWidth, scopeHeight, desc2.Usage, desc2.Format);

	vr::EVROverlayError err;

	err = vrOverlay->SetOverlayWidthInMeters(uiOverlay, Game::instance.c_UIOverlayScale->Value());
	if (err != vr::VROverlayError_None)
	{
		Logger::log << "[OpenVR] Error setting overlay width: " << err << std::endl;
	}
	Logger::log << "[OpenVR] Set UI Width = " << Game::instance.c_UIOverlayScale->Value() << std::endl;
	err = vrOverlay->SetOverlayWidthInMeters(crosshairOverlay, Game::instance.c_UIOverlayScale->Value());
	if (err != vr::VROverlayError_None)
	{
		Logger::log << "[OpenVR] Error setting overlay width: " << err << std::endl;
	}
	Logger::log << "[OpenVR] Set Crosshair Width = " << Game::instance.c_UIOverlayScale->Value() << std::endl;


	float curvature = Game::instance.c_UIOverlayCurvature->Value();
	if (curvature != 0.0f)
	{
		vrOverlay->SetOverlayCurvature(uiOverlay, curvature);
		Logger::log << "[OpenVR] Set UI Curvature = " << curvature << std::endl;
	}

	vr::HmdMatrix34_t overlayTransform = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 2.0f,
		0.0f, 0.0f, 1.0f, -3.0f
	};

	vrOverlay->SetOverlayTransformAbsolute(uiOverlay, vr::ETrackingUniverseOrigin::TrackingUniverseStanding, &overlayTransform);
	vrOverlay->SetOverlayTransformAbsolute(crosshairOverlay, vr::ETrackingUniverseOrigin::TrackingUniverseStanding, &overlayTransform);

	Logger::log << "[OpenVR] Finished Initialisation" << std::endl;
}


void OpenVR::CreateTexAndSurface(int index, UINT Width, UINT Height, DWORD Usage, D3DFORMAT Format)
{
	HANDLE sharedHandle = nullptr;

	// Create texture on game
	HRESULT result = Helpers::GetDirect3DDevice9()->CreateTexture(Width, Height, 1, Usage, Format, D3DPOOL_DEFAULT, &gameRenderTexture[index], &sharedHandle);
	if (FAILED(result))
	{
		Logger::err << "[OpenVR] Failed to create game " << index << " texture: " << result << std::endl;
		return;
	}

	result = gameRenderTexture[index]->GetSurfaceLevel(0, &gameRenderSurface[index]);
	if (FAILED(result))
	{
		Logger::err << "[OpenVR] Failed to retrieve game " << index << " surface: " << result << std::endl;
		return;
	}

	ID3D11Resource* tempResource = nullptr;

	// Open shared texture on vr
	result = d3dDevice->OpenSharedResource(sharedHandle, __uuidof(ID3D11Resource), (void**)&tempResource);

	if (FAILED(result))
	{
		Logger::err << "[OpenVR] Failed to open shared resource " << index << ": " << result << std::endl;
		return;
	}

	result = tempResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&vrRenderTexture[index]);
	tempResource->Release();

	if (FAILED(result))
	{
		Logger::err << "[OpenVR] Failed to query texture interface " << index << ": " << result << std::endl;
		return;
	}

	D3D11_TEXTURE2D_DESC desc;
	vrRenderTexture[index]->GetDesc(&desc);

	Logger::log << "[OpenVR] Created shared texture " << index << ", " << desc.Width << "x" << desc.Height << std::endl;
}


void OpenVR::UpdatePoses()
{
	if (!vrCompositor)
	{
		return;
	}

	vrCompositor->WaitGetPoses(renderPoses, vr::k_unMaxTrackedDeviceCount, gamePoses, vr::k_unMaxTrackedDeviceCount);

	if (!vrOverlay || !bMouseVisible)
	{
		return;
	}

	vr::VREvent_t vrEvent;
	while (vrOverlay->PollNextOverlayEvent(uiOverlay, &vrEvent, sizeof(vrEvent)))
	{
		switch (vrEvent.eventType)
		{
		case vr::VREvent_MouseMove:
			mousePos.x = vrEvent.data.mouse.x;
			mousePos.y = 1.0f - vrEvent.data.mouse.y;
			break;
		case vr::VREvent_MouseButtonDown:
			bMouseDown = true;
			break;
		case vr::VREvent_MouseButtonUp:
			bMouseDown = false;
			break;
		default:
			break;
		}
	}
}

void OpenVR::PreDrawFrame(Renderer* renderer, float deltaTime)
{
}

void OpenVR::PositionOverlay()
{
	// Get the HMD's position and rotation
	vr::HmdMatrix34_t mat = renderPoses[vr::k_unTrackedDeviceIndex_Hmd].mDeviceToAbsoluteTracking;
	vr::HmdVector3_t position;
	position.v[0] = mat.m[0][3];
	position.v[1] = mat.m[1][3];
	position.v[2] = mat.m[2][3];

	float distance = Game::instance.c_UIOverlayDistance->Value();

	float len = sqrt(mat.m[0][2] * mat.m[0][2] + mat.m[2][2] * mat.m[2][2]);

	distance /= len;

	position.v[0] += mat.m[0][2] * -distance;
	position.v[2] += mat.m[2][2] * -distance;

	// Rotate only around Y for yaw
	float yaw = atan2(-mat.m[2][0], mat.m[2][2]);
	vr::HmdMatrix34_t transform = {
		cos(yaw), 0, sin(yaw), position.v[0],
		0, 1, 0, position.v[1],
		-sin(yaw), 0, cos(yaw), position.v[2]
	};

	// Set the transform for the overlay
	vrOverlay->SetOverlayTransformAbsolute(uiOverlay, vr::TrackingUniverseStanding, &transform);
}

void OpenVR::PostDrawFrame(Renderer* renderer, float deltaTime)
{
	if (!vrCompositor)
	{
		return;
	}

	// Wait for frame to finish rendering
	IDirect3DQuery9* pEventQuery = nullptr;
	Helpers::GetDirect3DDevice9()->CreateQuery(D3DQUERYTYPE_EVENT, &pEventQuery);
	if (pEventQuery != nullptr)
	{
		pEventQuery->Issue(D3DISSUE_END);
		while (pEventQuery->GetData(nullptr, 0, D3DGETDATA_FLUSH) != S_OK);
		pEventQuery->Release();
	}

	vr::Texture_t leftEye { (void*)vrRenderTexture[0], vr::TextureType_DirectX, vr::ColorSpace_Auto};
	vr::EVRCompositorError error = vrCompositor->Submit(vr::Eye_Left, &leftEye, &textureBounds[0], vr::Submit_Default);

	if (error != vr::VRCompositorError_None)
	{
		Logger::log << "[OpenVR] Could not submit left eye texture: " << error << std::endl;
	}

	vr::Texture_t rightEye{ (void*)vrRenderTexture[1], vr::TextureType_DirectX, vr::ColorSpace_Auto };
	error = vrCompositor->Submit(vr::Eye_Right, &rightEye, &textureBounds[1], vr::Submit_Default);

	if (error != vr::VRCompositorError_None)
	{
		Logger::log << "[OpenVR] Could not submit right eye texture: " << error << std::endl;
	}

	PositionOverlay();

	vr::Texture_t uiTex{ (void*)vrRenderTexture[uiSurface], vr::TextureType_DirectX, vr::ColorSpace_Auto };
	vr::EVROverlayError oError = vrOverlay->SetOverlayTexture(uiOverlay, &uiTex);

	if (oError != vr::EVROverlayError::VROverlayError_None)
	{
		Logger::log << "[OpenVR] Could not submit ui texture: " << oError << std::endl;
	}

	vr::Texture_t crossTex{ (void*)vrRenderTexture[crosshairSurface], vr::TextureType_DirectX, vr::ColorSpace_Auto };
	vr::EVROverlayError o2Error = vrOverlay->SetOverlayTexture(crosshairOverlay, &crossTex);

	if (o2Error != vr::EVROverlayError::VROverlayError_None)
	{
		Logger::log << "[OpenVR] Could not submit crosshair texture: " << o2Error << std::endl;
	}

	vrCompositor->PostPresentHandoff();
}

void OpenVR::UpdateCameraFrustum(CameraFrustum* frustum, int eye)
{
	frustum->fov = fov;

	Matrix4 eyeMatrix = GetHMDMatrixPoseEye((vr::Hmd_Eye) eye);

	Matrix4 headMatrix = GetHMDTransform(true);

	Matrix4 viewMatrix = (headMatrix * eyeMatrix.invert()).scale(Game::MetresToWorld(1.0f));

	Matrix3 rotationMatrix = GetRotationMatrix(headMatrix);

	// Ignore all existing rotation for now
	frustum->facingDirection = Vector3(1.0f, 0.0f, 0.0f);
	frustum->upDirection = Vector3(0.0f, 0.0f, 1.0f);

	frustum->facingDirection = (rotationMatrix * frustum->facingDirection).normalize();
	frustum->upDirection = (rotationMatrix * frustum->upDirection).normalize();

	Vector3 newPos = viewMatrix * Vector3(0.0f, 0.0f, 0.0f);

	frustum->position = frustum->position + newPos;
}

int OpenVR::GetViewWidth()
{
    return recommendedWidth;
}

int OpenVR::GetViewHeight()
{
    return recommendedHeight;
}

float OpenVR::GetViewWidthStretch()
{
	return recommendedWidth / static_cast<float>(realWidth);
}

float OpenVR::GetViewHeightStretch()
{
	return recommendedHeight / static_cast<float>(realHeight);
}

float OpenVR::GetAspect()
{
	return aspect;
}

int OpenVR::GetScopeWidth()
{
	return scopeWidth;
}

int OpenVR::GetScopeHeight()
{
	return scopeHeight;
}

void OpenVR::Recentre()
{
	SetLocationOffset(ConvertSteamVRMatrixToMatrix4(renderPoses[vr::k_unTrackedDeviceIndex_Hmd].mDeviceToAbsoluteTracking) * Vector3(0.0f, 0.0f, 0.0f));
}

void OpenVR::SetLocationOffset(Vector3 newOffset)
{
	positionOffset = newOffset;
}

Vector3 OpenVR::GetLocationOffset()
{
	return positionOffset;
}

void OpenVR::SetYawOffset(float newOffset)
{
	yawOffset = newOffset;
}

float OpenVR::GetYawOffset()
{
	return yawOffset;
}

Matrix4 OpenVR::GetHMDTransform(bool bRenderPose)
{
	if (bRenderPose)
	{
		return ConvertSteamVRMatrixToMatrix4(renderPoses[vr::k_unTrackedDeviceIndex_Hmd].mDeviceToAbsoluteTracking).translate(-positionOffset).rotateZ(-yawOffset);
	}
	else
	{
		return ConvertSteamVRMatrixToMatrix4(gamePoses[vr::k_unTrackedDeviceIndex_Hmd].mDeviceToAbsoluteTracking).translate(-positionOffset).rotateZ(-yawOffset);
	}
}

Matrix4 OpenVR::GetControllerTransform(ControllerRole Role, bool bRenderPose)
{
	if (!vrSystem)
	{
		return Matrix4();
	}

	vr::TrackedDeviceIndex_t ControllerIndex = vrSystem->GetTrackedDeviceIndexForControllerRole(Role == ControllerRole::Left ? vr::TrackedControllerRole_LeftHand : vr::TrackedControllerRole_RightHand);

	if (bRenderPose)
	{
		return ConvertSteamVRMatrixToMatrix4(renderPoses[ControllerIndex].mDeviceToAbsoluteTracking).translate(-positionOffset).rotateZ(-yawOffset);
	}
	else
	{
		return ConvertSteamVRMatrixToMatrix4(gamePoses[ControllerIndex].mDeviceToAbsoluteTracking).translate(-positionOffset).rotateZ(-yawOffset);
	}
}

IDirect3DSurface9* OpenVR::GetRenderSurface(int eye)
{
    return gameRenderSurface[eye];
}

IDirect3DTexture9* OpenVR::GetRenderTexture(int eye)
{
    return gameRenderTexture[eye];
}

IDirect3DSurface9* OpenVR::GetUISurface()
{
    return gameRenderSurface[uiSurface];
}

IDirect3DSurface9* OpenVR::GetCrosshairSurface()
{
	return gameRenderSurface[crosshairSurface];
}

IDirect3DSurface9* OpenVR::GetScopeSurface()
{
	return gameRenderSurface[scopeSurface];
}

IDirect3DTexture9* OpenVR::GetScopeTexture()
{
	return gameRenderTexture[scopeSurface];
}

void OpenVR::SetMouseVisibility(bool bIsVisible)
{
	if (!vrOverlay)
	{
		return;
	}

	if (bMouseVisible == bIsVisible)
	{
		return;
	}

	bMouseVisible = bIsVisible;

	if (!bMouseVisible)
	{
		bMouseDown = false;
	}

	vrOverlay->SetOverlayInputMethod(uiOverlay, bIsVisible ? vr::VROverlayInputMethod_Mouse : vr::VROverlayInputMethod_None);
}

void OpenVR::SetCrosshairTransform(Matrix4& newTransform)
{
	vr::HmdMatrix34_t overlayMatrix = ConvertMatrixToSteamVRMatrix4(newTransform.rotateZ(yawOffset).translate(positionOffset));

	vrOverlay->SetOverlayTransformAbsolute(crosshairOverlay, vr::TrackingUniverseStanding, &overlayMatrix);
}

void OpenVR::SetScopeTransform(Matrix4& newTransform, bool bIsVisible)
{
	// Use the dx9 renderer to draw this directly in game, since we can't adjust the near clip plane of steamvr overlays
	if (!bIsVisible)
	{
		return;
	}

	Vector3 pos = (newTransform * Vector3(0.0f, 0.0f, 0.0f)) * Game::instance.MetresToWorld(1.0f) + Helpers::GetCamera().position;
	Matrix3 rot;
	Vector2 size(1.0f, 0.75f);
	size *= Game::instance.MetresToWorld(Game::instance.GetScopeSize());

	newTransform.translate(-pos);
	newTransform.rotate(90.0f, newTransform.getLeftAxis());
	newTransform.rotate(-90.0f, newTransform.getUpAxis());
	newTransform.rotate(-90.0f, newTransform.getLeftAxis());

	for (int i = 0; i < 3; i++)
	{
		rot.setColumn(i, &newTransform.get()[i * 4]);
	}

	Game::instance.inGameRenderer.DrawRenderTarget(GetScopeTexture(), pos, rot, size, false);
}

void OpenVR::UpdateInputs()
{
	vr::EVRInputError error = vrInput->UpdateActionState(actionSets, sizeof(vr::VRActiveActionSet_t), 1);

	if (error != vr::VRInputError_None)
	{
		Logger::log << "[OpenVR] Could not update inputs: " << error << std::endl;
	}
}

InputBindingID OpenVR::RegisterBoolInput(std::string set, std::string action)
{
	InputBindingID id;
	vr::EVRInputError err = vrInput->GetActionHandle(("/actions/" + set + "/in/" + action).c_str(), &id);
	if (err != vr::VRInputError_None)
	{
		Logger::log << "[OpenVR] Could not register bool input /actions/" << set << "/in/" << action << ": " << err << std::endl;
	}
	else
	{
		Logger::log << "[OpenVR] Registered /actions/" << set << "/in/" << action << " with id " << id << std::endl;
	}
	return id;
}

InputBindingID OpenVR::RegisterVector2Input(std::string set, std::string action)
{
	InputBindingID id;
	vr::EVRInputError err = vrInput->GetActionHandle(("/actions/" + set + "/in/" + action).c_str(), &id);
	if (err != vr::VRInputError_None)
	{
		Logger::log << "[OpenVR] Could not register vector2 input /actions/" << set << "/in/" << action << ": " << err << std::endl;
	}
	else
	{
		Logger::log << "[OpenVR] Registered /actions/" << set << "/in/" << action << " with id " << id << std::endl;
	}
	return id;
}

bool OpenVR::GetBoolInput(InputBindingID id)
{
	static bool dummy = false;
	return GetBoolInput(id, dummy);
}

bool OpenVR::GetBoolInput(InputBindingID id, bool& bHasChanged)
{
	static vr::InputDigitalActionData_t digital;
	vr::EVRInputError err = vrInput->GetDigitalActionData(id, &digital, sizeof(vr::InputDigitalActionData_t), vr::k_ulInvalidInputValueHandle);
	if (err != vr::VRInputError_None)
	{
		Logger::log << "[OpenVR] Could not get digital action: " << err << std::endl;
	}

	bHasChanged = digital.bChanged;

	return digital.bState;
}

Vector2 OpenVR::GetVector2Input(InputBindingID id)
{
	static vr::InputAnalogActionData_t Analog;
	vr::EVRInputError err = vrInput->GetAnalogActionData(id, &Analog, sizeof(vr::InputAnalogActionData_t), vr::k_ulInvalidInputValueHandle);
	if (err != vr::VRInputError_None)
	{
		Logger::log << "[OpenVR] Could not get analog action: " << err << std::endl;
	}

	return Vector2(Analog.x, Analog.y);
}

Vector2 OpenVR::GetMousePos()
{
	return mousePos;
}

bool OpenVR::GetMouseDown()
{
	return bMouseDown;
}
