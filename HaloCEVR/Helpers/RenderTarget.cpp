#include "RenderTarget.h"
#include "../Hooking/Hooks.h"

RenderTarget* Helpers::GetRenderTargets()
{
	// Source for this offset is accessing values in the middle of the struct
	return reinterpret_cast<RenderTarget*>(Hooks::o.RenderTargets - 0xc);
}
