#include "Hooks.h"
#include "../Helpers/Player.h"
#include "../Helpers/Renderer.h"
#include "../Helpers/DX9.h"
#include "../Game.h"

#define CREATEHOOK(Func) Func##.CreateHook(o.##Func##, &H_##Func##)

bool bPotentiallyFoundChimera = false;

void Hooks::InitHooks()
{
	CREATEHOOK(InitDirectX);
	CREATEHOOK(DrawFrame);
	CREATEHOOK(DrawHUD);
	CREATEHOOK(DrawMenu);
	//CREATEHOOK(DrawScope);
	CREATEHOOK(DrawLoadingScreen);
	CREATEHOOK(SetViewModelPosition);
	//CREATEHOOK(SetViewportSize);
	CREATEHOOK(HandleInputs);
	CREATEHOOK(UpdatePitchYaw);

	// These are handled with a direct patch, so manually scan them
	bPotentiallyFoundChimera |= SigScanner::UpdateOffset(o.TabOutVideo) < 0;
	bPotentiallyFoundChimera |= SigScanner::UpdateOffset(o.TabOutVideo2) < 0;
	bPotentiallyFoundChimera |= SigScanner::UpdateOffset(o.TabOutVideo3) < 0;
	SigScanner::UpdateOffset(o.CutsceneFPSCap);
}

void Hooks::EnableAllHooks()
{
	InitDirectX.EnableHook();
	DrawFrame.EnableHook();
	DrawHUD.EnableHook();
	DrawMenu.EnableHook();
	//DrawScope.EnableHook();
	DrawLoadingScreen.EnableHook();
	SetViewModelPosition.EnableHook();
	//SetViewportSize.EnableHook();
	HandleInputs.EnableHook();
	UpdatePitchYaw.EnableHook();

	P_RemoveCutsceneFPSCap();

	// If we think the user has chimera installed, don't try to patch their patches
	if (!bPotentiallyFoundChimera)
	{
		P_FixTabOut();

		// TODO: Test this (and get a proper signature etc)
		//NOPInstructions(0x4c90ea, 6);
	}
}

//===============================//Helpers//===================================//

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


void __declspec(naked) Hooks::H_DrawLoadingScreen()
{
	// Function has two params
	// int param1 : EAX
	// Renderer* renderer : Stack[0x4]

	static int param1;
	static Renderer* param2;

	_asm
	{
		// Grab param1
		mov param1, eax
		// Grab param2
		mov eax, [esp + 0x4]
		mov param2, eax

		pushad
	}

	if (Game::instance.PreDrawLoading(param1, param2))
	{

		_asm
		{
			popad

			// Re-push parameters
			push eax
			mov eax, param1
		}

		DrawLoadingScreen.Original();

		_asm
		{
			pushad
		}

		Game::instance.PostDrawLoading(param1, param2);

		_asm
		{
			popad

			add esp, 0x4
			ret
		}
	}
	else
	{
		_asm
		{
			popad

			ret
		}
	}
}

void __declspec(naked) Hooks::H_SetViewModelPosition()
{
	static Vector3* pos;
	static Vector3* facing;
	static Vector3* up;

	_asm
	{
		mov pos, ecx
		mov ecx, [esp + 0x10]
		mov up, ecx
		push ecx
		mov ecx, [esp + 0x10]
		mov facing, ecx
		push ecx
		mov ecx, [esp + 0x10]
		push ecx
		mov ecx, [esp + 0x10]
		push ecx
		mov ecx, pos
	}

	_asm { pushad }
	Game::instance.UpdateViewModel(pos, facing, up);
	_asm { popad }

	SetViewModelPosition.Original();

	_asm
	{
		add esp, 0x10
		ret
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

void Hooks::H_HandleInputs()
{
	HandleInputs.Original();

	Game::instance.UpdateInputs();
}

void __declspec(naked) Hooks::H_UpdatePitchYaw()
{
	static short playerId;
	static float yaw;
	static float pitch;

	// Extract parameters
	_asm
	{
		mov playerId, ax
		mov eax, [esp + 0x4]
		mov yaw, eax
		mov eax, [esp + 0x8]
		mov pitch, eax
		movsx eax, playerId
	}

	/*
	Original stack:
	0x0: ret address
	0x4: yaw
	0x8: pitch
	
	Post-hook stack:
	0x0: hook ret address
	0x4: ret address
	0x8: yaw
	0xc: pitch

	Fixed stack:
	0x0: hook ret address
	0x4: yaw
	0x8: pitch
	0xc: ret address
	0x10: yaw
	0x14: pitch
	*/

	_asm
	{
		pushad
	}

	Game::instance.UpdateCamera(yaw, pitch);

	_asm
	{
		popad
	}

	// Re-push parameters to fix stack for original call
	_asm
	{
		push pitch
		push yaw
	}

	UpdatePitchYaw.Original();

	// Cleanup and return
	_asm
	{
		add esp, 0x8
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

void Hooks::P_RemoveCutsceneFPSCap()
{
	// Instead of setting the flag for capping the fps to 1 when we are in a cutscene, just clear it
	byte bytes[2]{ 0x32, 0xdb };
	SetBytes(o.CutsceneFPSCap.Address, 2, bytes);
}
