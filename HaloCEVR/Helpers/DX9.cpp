#include "DX9.h"
#include "../Hooking/Hooks.h"

IDirect3D9* Helpers::GetDirect3D9()
{
	return *reinterpret_cast<IDirect3D9**>(Hooks::o.DirectX9);
}

IDirect3DDevice9* Helpers::GetDirect3DDevice9()
{
	return *reinterpret_cast<IDirect3DDevice9**>(Hooks::o.DirectX9Device);
}