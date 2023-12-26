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

	static void SetByte(struct Offset& Offset, long long Byte, byte opcode);

	// All Hooks go here:
	static inline Hook<Func_UpdateCameraRotation> UpdateCameraRotation;	
	static inline Hook<Func_InitDirectX> InitDirectX;
	static inline Hook<Func_DrawFrame> DrawFrame;
	static inline Hook<Func_SetViewportSize> SetViewportSize;

	// All Hook implementations go here:
	static void H_UpdateCameraRotation();
	static void H_InitDirectX();
	static void H_DrawFrame(Renderer* param1, short param2, short* param3, float param4, float deltaTime);
	static void H_SetViewportSize();

	// All direct patches go here:
	static void P_FixTabOut();
private:
	static inline Offsets o;
};