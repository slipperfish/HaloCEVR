#include "RenderTarget.h"
#include "../Hooking/Hooks.h"

RenderTarget* Helpers::GetRenderTargets()
{
	return reinterpret_cast<RenderTarget*>(Hooks::o.RenderTargets);
}
