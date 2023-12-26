#include "DX9.h"

IDirect3D9* Helpers::GetDirect3D9()
{
	return *reinterpret_cast<IDirect3D9**>(0x71d178);
}

IDirect3DDevice9* Helpers::GetDirect3DDevice9()
{
	return *reinterpret_cast<IDirect3DDevice9**>(0x71d174);
}