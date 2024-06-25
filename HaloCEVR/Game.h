#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d9.h>
#include "Config/Config.h"
#include "VR/IVR.h"
#include "Helpers/Renderer.h"
#include "Helpers/RenderTarget.h"
#include "Helpers/Objects.h"
#include "Maths/Vectors.h"
#include "WeaponHandler.h"
#include "InputHandler.h"

enum class ERenderState { UNKNOWN, LEFT_EYE, RIGHT_EYE, GAME};

class Game
{
public:
	static Game instance;

	void Init();
	void Shutdown();

	void OnInitDirectX();
	void PreDrawFrame(struct Renderer* renderer, float deltaTime);
	void PreDrawEye(struct Renderer* renderer, float deltaTime, int eye);
	void PostDrawEye(struct Renderer* renderer, float DeltaTime, int eye);
	void PreDrawMirror(struct Renderer* renderer, float deltaTime);
	void PostDrawMirror(struct Renderer* renderer, float deltaTime);
	void PostDrawFrame(struct Renderer* renderer, float deltaTime);

	bool PreDrawHUD();
	void PostDrawHUD();

	bool PreDrawMenu();
	void PostDrawMenu();

	bool PreDrawLoading(int param1, struct Renderer* renderer);
	void PostDrawLoading(int param1, struct Renderer* renderer);

	void PreDrawCrosshair(short* anchorLocation);
	void PostDrawCrosshair();

	void UpdateViewModel(HaloID& id, struct Vector3* pos, struct Vector3* facing, struct Vector3* up, struct TransformQuat* BoneTransforms, struct Transform* OutBoneTransforms);
	void PreFireWeapon(HaloID& WeaponID, short param2, bool param3);
	void PostFireWeapon(HaloID& WeaponID, short param2, bool param3);

	void UpdateInputs();
	void UpdateCamera(float& yaw, float& pitch);
	void SetMousePosition(int& x, int& y);
	void UpdateMouseInfo(struct MouseInfo* mouseInfo);

	void SetViewportScale(struct Viewport* viewport);

	bool GetDrawMirror() const { return c_DrawMirror->Value(); }

	ERenderState GetRenderState() const { return renderState; }

	static float MetresToWorld(float m);
	static float WorldToMetres(float w);

	inline IVR* GetVR() const { return vr; }

	UINT backBufferWidth = 600;
	UINT backBufferHeight = 600;

	// HACK: Some places it is hard to get the delta time (e.g. updating the camera)
	// Using the last known delta time should be good enough
	float lastDeltaTime = 0.0f;

	bool bNeedsRecentre = true;
protected:

	void CreateConsole();

	void PatchGame();

	void SetupConfigs();

	void CalcFPS(float deltaTime);

	void UpdateCrosshair();

	void StoreRenderTargets();
	void RestoreRenderTargets();

	WeaponHandler weaponHandler;
	InputHandler inputHandler;

	struct FPSTracker
	{
		float timeSinceFPSUpdate = 0.0f;
		int framesSinceFPSUpdate = 0;

		int fps = 0;
	} fpsTracker;

	FILE* consoleOut = nullptr;

	Config config;

	IVR* vr;

	RenderTarget gameRenderTargets[8];

	struct IDirect3DSurface9* uiSurface;
	struct IDirect3DSurface9* crosshairSurface;
	struct IDirect3DSurface9* uiRealSurface;
	struct IDirect3DSurface9* crosshairRealSurface;

	ERenderState renderState = ERenderState::UNKNOWN;

	CameraFrustum frustum1;
	CameraFrustum frustum2;


	//======Configs======//
public:

	BoolProperty* c_ShowConsole = nullptr;
	BoolProperty* c_DrawMirror = nullptr;
	FloatProperty* c_UIOverlayDistance = nullptr;
	FloatProperty* c_UIOverlayScale = nullptr;
	FloatProperty* c_UIOverlayCurvature = nullptr;
	IntProperty* c_UIOverlayWidth = nullptr;
	IntProperty* c_UIOverlayHeight = nullptr;
	BoolProperty* c_SnapTurn = nullptr;
	FloatProperty* c_SnapTurnAmount = nullptr;
	FloatProperty* c_SmoothTurnAmount = nullptr;
	FloatProperty* c_LeftHandFlashlightDistance = nullptr;
	FloatProperty* c_RightHandFlashlightDistance = nullptr;
	FloatProperty* c_ControllerOffsetX = nullptr;
	FloatProperty* c_ControllerOffsetY = nullptr;
	FloatProperty* c_ControllerOffsetZ = nullptr;
	FloatProperty* c_ControllerRotationX = nullptr;
	FloatProperty* c_ControllerRotationY = nullptr;
	FloatProperty* c_ControllerRotationZ = nullptr;
};

