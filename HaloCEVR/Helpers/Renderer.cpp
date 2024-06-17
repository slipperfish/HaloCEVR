#include "Renderer.h"
#include "../Hooking/Hooks.h"

namespace Helpers
{
	sRect* GetWindowRect()
	{
		return reinterpret_cast<sRect*>(Hooks::o.WindowRect);
	}
}