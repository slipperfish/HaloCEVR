#pragma once
#include <d3d9.h>
#include "Hook.h"
#include "SigScanner.h"
#include "FunctionTypeDefs.h"
#include "../Helpers/Renderer.h"


class Hooks
{
public:
	static void InitHooks();

	static void EnableAllHooks();

	static void SetByte(long long Address, byte Byte);
	static void SetBytes(long long Address, int Length, byte* Bytes);
	static void NOPInstructions(long long Address, int Length);

	// All Hooks go here:
	static inline Hook<Func_InitDirectX> InitDirectX;
	static inline Hook<Func_DrawFrame> DrawFrame;
	static inline Hook<Func_DrawHUD> DrawHUD;
	static inline Hook<Func_DrawMenu> DrawMenu;
	static inline Hook<Func_DrawScope> DrawScope;
	static inline Hook<Func_DrawLoadingScreen> DrawLoadingScreen;
	static inline Hook<Func_SetViewModelPosition> SetViewModelPosition;
	static inline Hook<Func_SetViewportSize> SetViewportSize; // Unused
	static inline Hook<Func_HandleInputs> HandleInputs;
	static inline Hook<Func_UpdatePitchYaw> UpdatePitchYaw;

	// All Hook implementations go here:
	static void H_InitDirectX();
	static void H_DrawFrame(Renderer* param1, short param2, short* param3, float param4, float deltaTime);
	static void H_DrawHUD();
	static void H_DrawMenu();
	static void __stdcall H_DrawScope(void* param1);
	static void H_DrawLoadingScreen();
	static void H_SetViewModelPosition();
	static void H_SetViewportSize();
	static void H_HandleInputs();
	static void H_UpdatePitchYaw();

	// All direct patches go here:
	static void P_FixTabOut();
	static void P_RemoveCutsceneFPSCap();
private:
	static inline Offsets o;
};