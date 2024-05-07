#include "Hooks.h"
#include "../Helpers/Player.h"
#include "../Helpers/Renderer.h"
#include "../Helpers/DX9.h"
#include "../Game.h"
#include "../Helpers/Menus.h"
#include "../Helpers/Maths.h"

#include <tlhelp32.h>

#define CREATEHOOK(Func) Func##.CreateHook(#Func, o.##Func##, &H_##Func##)

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
	CREATEHOOK(SetViewportScale);
	CREATEHOOK(SetMousePosition);
	CREATEHOOK(UpdateMouseInfo);
	CREATEHOOK(FireWeapon);

	// These are handled with a direct patch, so manually scan them
	bPotentiallyFoundChimera |= SigScanner::UpdateOffset(o.TabOutVideo) < 0;
	bPotentiallyFoundChimera |= SigScanner::UpdateOffset(o.TabOutVideo2) < 0;
	bPotentiallyFoundChimera |= SigScanner::UpdateOffset(o.TabOutVideo3) < 0;
	SigScanner::UpdateOffset(o.CutsceneFPSCap);
	SigScanner::UpdateOffset(o.CreateMouseDevice);
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
	SetViewportScale.EnableHook();
	SetMousePosition.EnableHook();
	UpdateMouseInfo.EnableHook();
	FireWeapon.EnableHook();

	Freeze();

	P_RemoveCutsceneFPSCap();
	P_DontStealMouse();

	// If we think the user has chimera installed, don't try to patch their patches
	if (!bPotentiallyFoundChimera)
	{
		P_FixTabOut();

		// TODO: Test this (and get a proper signature etc)
		//NOPInstructions(0x4c90ea, 6);
	}

	Unfreeze();
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

// Below is borrowed from minhook to freeze threads while applying patches
#define THREAD_ACCESS \
    (THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION | THREAD_SET_CONTEXT)
#define INITIAL_THREAD_CAPACITY 128

HANDLE g_hHeap = NULL;

typedef struct _FROZEN_THREADS
{
	LPDWORD pItems;         // Data heap
	UINT    capacity;       // Size of allocated data heap, items
	UINT    size;           // Actual number of data items
} FROZEN_THREADS, * PFROZEN_THREADS;

static BOOL EnumerateThreads(PFROZEN_THREADS pThreads)
{
	LPDWORD p;
	BOOL succeeded = FALSE;

	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (hSnapshot != INVALID_HANDLE_VALUE)
	{
		THREADENTRY32 te;
		te.dwSize = sizeof(THREADENTRY32);
		if (Thread32First(hSnapshot, &te))
		{
			succeeded = TRUE;
			do
			{
				if (te.dwSize >= (FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) + sizeof(DWORD))
					&& te.th32OwnerProcessID == GetCurrentProcessId()
					&& te.th32ThreadID != GetCurrentThreadId())
				{
					if (pThreads->pItems == NULL)
					{
						pThreads->capacity = INITIAL_THREAD_CAPACITY;
						pThreads->pItems
							= (LPDWORD)HeapAlloc(g_hHeap, 0, pThreads->capacity * sizeof(DWORD));
						if (pThreads->pItems == NULL)
						{
							succeeded = FALSE;
							break;
						}
					}
					else if (pThreads->size >= pThreads->capacity)
					{
						pThreads->capacity *= 2;
						p = (LPDWORD)HeapReAlloc(
							g_hHeap, 0, pThreads->pItems, pThreads->capacity * sizeof(DWORD));
						if (p == NULL)
						{
							succeeded = FALSE;
							break;
						}

						pThreads->pItems = p;
					}
					pThreads->pItems[pThreads->size++] = te.th32ThreadID;
				}

				te.dwSize = sizeof(THREADENTRY32);
			} while (Thread32Next(hSnapshot, &te));

			if (succeeded && GetLastError() != ERROR_NO_MORE_FILES)
				succeeded = FALSE;

			if (!succeeded && pThreads->pItems != NULL)
			{
				HeapFree(g_hHeap, 0, pThreads->pItems);
				pThreads->pItems = NULL;
			}
		}
		CloseHandle(hSnapshot);
	}

	return succeeded;
}

//-------------------------------------------------------------------------
FROZEN_THREADS Threads;

bool Hooks::Freeze()
{
	Threads.pItems = NULL;
	Threads.capacity = 0;
	Threads.size = 0;
	g_hHeap = HeapCreate(0, 0, 0);
	if (!EnumerateThreads(&Threads))
	{
		return false;
	}

	if (Threads.pItems != NULL)
	{
		UINT i;
		for (i = 0; i < Threads.size; ++i)
		{
			HANDLE hThread = OpenThread(THREAD_ACCESS, FALSE, Threads.pItems[i]);
			if (hThread != NULL)
			{
				SuspendThread(hThread);
				CloseHandle(hThread);
			}
		}
	}

	return true;
}

//-------------------------------------------------------------------------
void Hooks::Unfreeze()
{
	if (Threads.pItems != NULL)
	{
		UINT i;
		for (i = 0; i < Threads.size; ++i)
		{
			HANDLE hThread = OpenThread(THREAD_ACCESS, FALSE, Threads.pItems[i]);
			if (hThread != NULL)
			{
				ResumeThread(hThread);
				CloseHandle(hThread);
			}
		}

		HeapFree(g_hHeap, 0, Threads.pItems);
	}
	HeapDestroy(g_hHeap);
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
	static HaloID id;
	static TransformQuat* quatTrans;
	static Transform* trans;

	// TODO: Before submitting, remove the repushing + fix up the esp + 0xXX movs

	_asm
	{
		mov id, eax // Get ID from EAX param
		mov pos, ecx // Get Position from ECX param
		mov ecx, [esp + 0x10] // Get UpVector from 0x10 param
		mov up, ecx
		push ecx // Re-push UpVector
		mov ecx, [esp + 0x10] // Get FacingVector from 0xc param
		mov facing, ecx
		push ecx // Re-push FacingVector
		mov ecx, [esp + 0x10] // Get QuatTransforms from 0x8 param
		mov quatTrans, ecx
		push ecx // Re-push QuatTransforms
		mov ecx, [esp + 0x10] // Get Transforms from 0x4 param
		mov trans, ecx
		push ecx // Re-push Transforms
		mov ecx, pos // Restore Position in ECX register
	}

	// The original function was simple enough + required enough patches for VR
	// that it is easier to just fully reimplement it in the mod
	Game::instance.UpdateViewModel(id, pos, facing, up, quatTrans, trans);

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


void __declspec(naked) Hooks::H_SetViewportScale()
{
	static Viewport* view;

	_asm
	{
		mov view, ecx
	}

	SetViewportScale.Original();

	_asm
	{
		pushad
	}

	Game::instance.SetViewportScale(view);

	_asm
	{
		popad
	}

	_asm
	{
		ret
	}
}

void __declspec(naked) Hooks::H_SetMousePosition()
{
	static int x;
	static int y;

	_asm
	{
		mov x, eax
		mov y, ecx
	}

	_asm
	{
		pushad
	}

	Game::instance.SetMousePosition(x, y);

	_asm
	{
		popad
	}

	_asm
	{
		mov eax, x
		mov ecx, y
	}

	SetMousePosition.Original();

	_asm
	{
		ret
	}
}

void __declspec(naked) Hooks::H_UpdateMouseInfo()
{
	static MouseInfo* mouseInfo;

	_asm
	{
		push eax
		mov eax, [esp + 0x8]
		mov mouseInfo, eax
		pop eax
	}

	_asm
	{
		push mouseInfo
	}

	UpdateMouseInfo.Original();

	_asm
	{
		pushad
	}

	Game::instance.UpdateMouseInfo(mouseInfo);

	_asm
	{
		popad
	}

	_asm
	{
		add esp, 0x4
		ret
	}
}

#pragma optimize("", off)

void Hooks::H_FireWeapon(HaloID param1, short param2, bool param3)
{
	Game::instance.PreFireWeapon(param1, param2, param3);

	FireWeapon.Original(param1, param2, param3);

	Game::instance.PostFireWeapon(param1, param2, param3);

}

#pragma optimize("", on)

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

void Hooks::P_DontStealMouse()
{
	// Halo claims exclusive rights over the mouse which causes all sorts of chaos when trying to pause the game with a debugger
	// Changing it to non-exclusive is fine for our purposes (VR/testing),
	// but does cause the mouse to stop being locked to the window in game
	SetByte(o.CreateMouseDevice.Address + 0x5B, 6);
}
