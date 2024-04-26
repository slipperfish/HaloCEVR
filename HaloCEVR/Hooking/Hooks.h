#pragma once
#include <d3d9.h>
#include "Hook.h"
#include "SigScanner.h"
#include "FunctionTypeDefs.h"
#include "../Helpers/Renderer.h"

#define DEFINE_HOOK_FULL(Name, RetCall, ...) static inline Hook<Func_##Name> Name; static RetCall H_##Name(__VA_ARGS__)
#define DEFINE_HOOK(Name, ...) DEFINE_HOOK_FULL(Name, void, __VA_ARGS__)

class Hooks
{
public:
	static void InitHooks();

	static void EnableAllHooks();

	static void SetByte(long long Address, byte Byte);
	static void SetBytes(long long Address, int Length, byte* Bytes);
	static void NOPInstructions(long long Address, int Length);

	// All Hooks go here:
	DEFINE_HOOK(InitDirectX);
	DEFINE_HOOK_FULL(DrawFrame, void, Renderer* param1, short param2, short* param3, float param4, float deltaTime);
	DEFINE_HOOK(DrawHUD);
	DEFINE_HOOK(DrawMenu);
	DEFINE_HOOK_FULL(DrawScope, void __stdcall, void* param1); // Disabled
	DEFINE_HOOK(DrawLoadingScreen);
	DEFINE_HOOK(SetViewModelPosition);
	DEFINE_HOOK(SetViewportSize); // Unused
	DEFINE_HOOK(HandleInputs);
	DEFINE_HOOK(UpdatePitchYaw);
	DEFINE_HOOK(SetViewportScale);
	DEFINE_HOOK(SetMousePosition);
	DEFINE_HOOK(UpdateMouseInfo);

	// All direct patches go here:
	static void P_FixTabOut();
	static void P_RemoveCutsceneFPSCap();
private:
	static inline Offsets o;
};

#undef DEFINE_HOOK
#undef DEFINE_HOOK_FULL