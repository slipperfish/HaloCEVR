#include "Hooks.h"
#include "../Helpers/Player.h"
#include "../Game.h"

#define CREATEHOOK(Func) Func##.CreateHook(o.##Func##, &H_##Func##)

void Hooks::InitHooks()
{
	//CREATEHOOK(UpdateCameraRotation);
	CREATEHOOK(InitDirectX);
	CREATEHOOK(DrawFrame);
	CREATEHOOK(SetViewportSize);

	// These are handled with a direct patch, so manually scan them
	SigScanner::UpdateOffset(o.TabOutVideo);
	SigScanner::UpdateOffset(o.TabOutVideo2);
	SigScanner::UpdateOffset(o.TabOutVideo3);
}

void Hooks::EnableAllHooks()
{
	//UpdateCameraRotation.EnableHook();
	InitDirectX.EnableHook();
	DrawFrame.EnableHook();
	SetViewportSize.EnableHook();

	P_FixTabOut();
}

void Hooks::SetByte(Offset& Offset, long long Byte, byte opcode)
{
	LPVOID Addr = reinterpret_cast<LPVOID>(Offset.Address + Byte);
	DWORD OldProt;
	VirtualProtect(Addr, 1, PAGE_EXECUTE_READWRITE, &OldProt);
	*reinterpret_cast<byte*>(Offset.Address + Byte) = opcode;
	if (OldProt != PAGE_EXECUTE_READWRITE)
	{
		DWORD NewProt;
		VirtualProtect(Addr, 1, OldProt, &NewProt);
	}
}

//===============================//Hooks//===================================//

void __declspec(naked) Hooks::H_UpdateCameraRotation()
{
	UpdateCameraRotation.Original();

	static float t = 0.0f;

	t += 1.0f / 60.0f;
	
	Helpers::GetPlayer().camera.fov = 0.5f + abs(sin(t));

	//float newFov = Helpers::GetPlayer().camera.fov;
	//float newFovDeg = newFov * (180.0f / 3.1415926f);

	//Logger::log << t << ": " << newFov << " (" << newFovDeg << ")" << std::endl;

	_asm
	{
		ret
	}
}

void Hooks::H_InitDirectX()
{
	/*
	sRect* Window = reinterpret_cast<sRect*>(0x69c634);
	Window->top = 0;
	Window->left = 0;
	Window->right = 600;
	Window->bottom = 600;
	*/

	InitDirectX.Original();

	Game::instance.OnInitDirectX();
}

void Hooks::H_DrawFrame(Renderer* param1, short param2, short* param3, float tickProgress, float deltaTime)
{
	Game::instance.PreDrawFrame(param1, deltaTime);
	Game::instance.PreDrawEye(param1, deltaTime, 0);
	DrawFrame.Original(param1, param2, param3, tickProgress, deltaTime);
	Game::instance.PostDrawEye(param1, deltaTime, 0);
	Game::instance.PreDrawEye(param1, deltaTime, 1);
	DrawFrame.Original(param1, param2, param3, tickProgress, deltaTime);
	Game::instance.PostDrawEye(param1, deltaTime, 1);
	if (Game::instance.GetDrawMirror())
	{
		Game::instance.PreDrawMirror(param1, deltaTime);
		DrawFrame.Original(param1, param2, param3, tickProgress, deltaTime);
		Game::instance.PostDrawMirror(param1, deltaTime);
	}
	Game::instance.PostDrawFrame(param1, deltaTime);
}

#include "../Helpers/Renderer.h"

int numViews = 0;
sRect* otherVP;
sRect* vp;

void OnSetViewportSize()
{
	//Logger::log << "(" << (int)Game::instance.GetDrawState() << ") " << "Num Views = " << numViews << std::endl;
	if (otherVP)
	{
		//Logger::log << "Other VP: " << otherVP->left << "-" << otherVP->right << ", " << otherVP->top << "-" << otherVP->bottom << std::endl;
	}

	if (vp)
	{
		//Logger::log << "VP: " << vp->left << "-" << vp->right << ", " << vp->top << "-" << vp->bottom << std::endl;
	}

	/*
	sRect* windowRect = reinterpret_cast<sRect*>(0x69c634);

	windowRect->right = 600;
	windowRect->bottom = 600;

	
	otherVP->left = 0;
	otherVP->top = 0;

	otherVP->right = 600;
	otherVP->bottom = 600;

	vp->left = 8;
	vp->top = 8;

	vp->right = 592;
	vp->bottom = 592;
	//*/
}

void __declspec(naked) Hooks::H_SetViewportSize()
{
	
	// Don't mangle the stack
	_asm
	{
		sub esp, 0x4
		push eax
		mov eax, [esp + 0xc]
		add esp, 0x8
		push eax
		sub esp, 0x4
		pop eax
	}

	// Yoink some of the parameters
	_asm
	{
		mov numViews, eax
		mov vp, ecx
		push eax
		mov eax, [esp + 0x4]
		mov otherVP, eax
		pop eax
	}

	SetViewportSize.Original();

	// Store registers, since calling conventions may mess with it
	_asm
	{
		PUSHAD
	}

	OnSetViewportSize();
	
	// Restore registers
	_asm
	{
		POPAD
	}

	_asm
	{
		add esp, 0x4
		ret
	}
}

//================================//Patches//================================//

void Hooks::P_FixTabOut()
{	
	// There are 3 checks in the main loop for focus
	// Attempting to patch this out with minhook would be painful
	
	// Always jump past the first check
	SetByte(o.TabOutVideo, 6, 0xEB);
	// NOP a JNZ in the second check
	SetByte(o.TabOutVideo2, 0, 0x90);
	SetByte(o.TabOutVideo2, 1, 0x90);
	// Always jump past the final check
	SetByte(o.TabOutVideo3, 6, 0xEB);

	// TODO: There's a more complex patch I think to do with fullscreen
}
