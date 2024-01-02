#include "Renderer.h"

namespace Helpers
{
	sRect* GetWindowRect()
	{
		// TODO: Use sigs, other mods like chimera might break things
		// Find a reliable place in code that references the camera, 
		// read the op code that loads it then use that to do a look up

		return reinterpret_cast<sRect*>(0x69c634);
	}
}