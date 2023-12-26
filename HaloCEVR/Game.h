#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d9.h>
#include "Config/Config.h"
#include "VREmulator.h"
#include "Helpers/RenderTarget.h"

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


	bool GetDrawMirror() const { return c_DrawMirror->Value(); }

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

	//======Configs======//

	BoolProperty* c_ShowConsole = nullptr;
	BoolProperty* c_DrawMirror = nullptr;
};

