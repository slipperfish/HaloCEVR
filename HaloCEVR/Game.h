#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d9.h>
#include "Config/Config.h"
#include "VREmulator.h"
#include "Helpers/RenderTarget.h"
#include "Helpers/Vector3.h"

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

	bool GetDrawMirror() const { return c_DrawMirror->Value(); }

	ERenderState GetRenderState() const { return RenderState; }

protected:

	void CreateConsole();

	void PatchGame();

	void SetupConfigs();

	void CalcFPS(float deltaTime);

	void StoreRenderTargets();
	void RestoreRenderTargets();

	float TimeSinceFPSUpdate = 0.0f;
	int FramesSinceFPSUpdate = 0;

	int FPS = 0;

	FILE* ConsoleOut = nullptr;

	Config config;

	VREmulator vrEmulator;

	RenderTarget gameRenderTargets[8];

	struct IDirect3DSurface9* UISurface;
	struct IDirect3DSurface9* UIRealSurface;

	ERenderState RenderState = ERenderState::UNKNOWN;

	Vector3 frustumPos;
	Vector3 frustum2Pos;

	//======Configs======//

	BoolProperty* c_ShowConsole = nullptr;
	BoolProperty* c_DrawMirror = nullptr;
};

