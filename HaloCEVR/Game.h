#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d9.h>
#include "Config/Config.h"
#include "VR/IVR.h"
#include "Helpers/Renderer.h"
#include "Helpers/RenderTarget.h"
#include "Maths/Vectors.h"

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

	void UpdateViewModel(struct Vector3* pos, struct Vector3* facing, struct Vector3* up);

	void UpdateInputs();
	void UpdateCamera(float& yaw, float& pitch);

	bool GetDrawMirror() const { return c_DrawMirror->Value(); }

	ERenderState GetRenderState() const { return RenderState; }

	static float MetresToWorld(float m);
	static float WorldToMetres(float w);

	UINT BackBufferWidth = 600;
	UINT BackBufferHeight = 600;

protected:

	void CreateConsole();

	void PatchGame();

	void SetupConfigs();

	void CalcFPS(float deltaTime);

	void StoreRenderTargets();
	void RestoreRenderTargets();

	bool bNeedsRecentre = true;
	Vector3 Offset;
	char LastSnapState = 0;
	float LastDeltaTime = 0.0f;


	float TimeSinceFPSUpdate = 0.0f;
	int FramesSinceFPSUpdate = 0;

	int FPS = 0;

	FILE* ConsoleOut = nullptr;

	Config config;

	IVR* vr;

	RenderTarget gameRenderTargets[8];

	struct IDirect3DSurface9* UISurface;
	struct IDirect3DSurface9* UIRealSurface;
	struct IDirect3DSurface9* UIRealSurface2;

	ERenderState RenderState = ERenderState::UNKNOWN;

	CameraFrustum frustum1;
	CameraFrustum frustum2;

	//======Controls======//
	InputBindingID Jump;
	InputBindingID SwitchGrenades;
	InputBindingID Interact;
	InputBindingID SwitchWeapons;
	InputBindingID Melee;
	InputBindingID Flashlight;
	InputBindingID Grenade;
	InputBindingID Fire;
	InputBindingID MenuForward;
	InputBindingID MenuBack;
	InputBindingID Crouch;
	InputBindingID Zoom;
	InputBindingID Reload;
	InputBindingID Move;
	InputBindingID Look;
	// Temp?
	InputBindingID Recentre;

	//======Configs======//
public:

	BoolProperty* c_ShowConsole = nullptr;
	BoolProperty* c_DrawMirror = nullptr;
	FloatProperty* c_UIOverlayDistance = nullptr;
	FloatProperty* c_UIOverlayScale = nullptr;
	FloatProperty* c_UIOverlayCurvature = nullptr;
	BoolProperty* c_SnapTurn = nullptr;
	FloatProperty* c_SnapTurnAmount = nullptr;
	FloatProperty* c_SmoothTurnAmount = nullptr;
};

