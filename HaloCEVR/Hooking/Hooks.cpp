#include "Hooks.h"
#include "../Helpers/Player.h"
#include "../Helpers/Renderer.h"
#include "../Helpers/DX9.h"
#include "../Game.h"

#define CREATEHOOK(Func) Func##.CreateHook(o.##Func##, &H_##Func##)

void Hooks::InitHooks()
{
	CREATEHOOK(InitDirectX);
	CREATEHOOK(DrawFrame);
	CREATEHOOK(DrawHUD);
	CREATEHOOK(DrawMenu);
	CREATEHOOK(DrawScope);
	//CREATEHOOK(UpdateCameraRotation);
	//CREATEHOOK(SetViewportSize);

	// These are handled with a direct patch, so manually scan them
	SigScanner::UpdateOffset(o.TabOutVideo);
	SigScanner::UpdateOffset(o.TabOutVideo2);
	SigScanner::UpdateOffset(o.TabOutVideo3);
}

void Hooks::EnableAllHooks()
{
	InitDirectX.EnableHook();
	DrawFrame.EnableHook();
	DrawHUD.EnableHook();
	DrawMenu.EnableHook();
	DrawScope.EnableHook();
	//UpdateCameraRotation.EnableHook();
	//SetViewportSize.EnableHook();

	P_FixTabOut();

	// Remove HUD
	//NOPInstructions(0x50c551, 5);
	// Remove Menus
	//NOPInstructions(0x50c55d, 5);
	// Remove scope
	//NOPInstructions(0x494a80, 5);
}

void Hooks::SetByte(long long Address, byte Byte)
{
	byte bytes[1]{ Byte };
	SetBytes(Address, 1, bytes);
}

void Hooks::SetBytes(long long Address, int Length, byte* Bytes)
{
	LPVOID Addr = reinterpret_cast<LPVOID>(Address);
	DWORD OldProt;
	VirtualProtect(Addr, Length, PAGE_EXECUTE_READWRITE, &OldProt);
	for (int i = 0; i < Length; i++)
	{
		*reinterpret_cast<char*>(Address + i) = Bytes[i];
	}
	if (OldProt != PAGE_EXECUTE_READWRITE)
	{
		DWORD NewProt;
		VirtualProtect(Addr, Length, OldProt, &NewProt);
	}
}

void Hooks::NOPInstructions(long long Address, int Length)
{
	LPVOID Addr = reinterpret_cast<LPVOID>(Address);
	DWORD OldProt;
	VirtualProtect(Addr, Length, PAGE_EXECUTE_READWRITE, &OldProt);
	for (int i = 0; i < Length; i++)
	{
		*reinterpret_cast<byte*>(Address + i) = 0x90;
	}
	if (OldProt != PAGE_EXECUTE_READWRITE)
	{
		DWORD NewProt;
		VirtualProtect(Addr, Length, OldProt, &NewProt);
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

void Hooks::H_DrawHUD()
{
	if (Game::instance.PreDrawHUD())
	{
		DrawHUD.Original();
		Game::instance.PostDrawHUD();
	}
}

void __declspec(naked) Hooks::H_DrawMenu()
{
	// Original function uses AX register as param_1
	_asm
	{
		pushad
	}

	if (Game::instance.PreDrawMenu())
	{
		DrawMenu.Original();
		Game::instance.PostDrawMenu();
	}

	_asm
	{
		popad
		ret
	}
}

void Hooks::H_DrawScope(void* param1)
{
	if (Game::instance.GetRenderState() == ERenderState::GAME)
	{
		DrawScope.Original(param1);
	}
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
		//mov numViews, eax
		//mov vp, ecx
		push eax
		mov eax, [esp + 0x4]
		//mov otherVP, eax
		pop eax
	}

	SetViewportSize.Original();

	// Store registers, since calling conventions may mess with it
	_asm
	{
		PUSHAD
	}

	//OnSetViewportSize();
	
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
	SetByte(o.TabOutVideo.Address + 6, 0xEB);
	// NOP a JNZ in the second check
	NOPInstructions(o.TabOutVideo2.Address, 2);
	// Always jump past the final check
	SetByte(o.TabOutVideo3.Address + 6, 0xEB);

	// TODO: There's a more complex patch I think to do with fullscreen
}
