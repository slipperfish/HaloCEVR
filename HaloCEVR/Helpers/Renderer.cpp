#include "Renderer.h"
#include "../Hooking/Hooks.h"

namespace Helpers
{
	sRect* GetWindowRect()
	{
		return reinterpret_cast<sRect*>(Hooks::o.WindowRect);
	}

	sRect* GetCurrentRect()
	{
		return reinterpret_cast<sRect*>(Hooks::o.CurrentRect);
	}

    CameraRenderMatrices* GetActiveCameraMatrices()
    {
        return reinterpret_cast<CameraRenderMatrices*>(Hooks::o.CameraMatrices);
    }
}