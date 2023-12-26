#include "Player.h"

namespace Helpers
{
	Player& GetPlayer()
	{
		// TODO: Use sigs, other mods like chimera might break things
		// Find a reliable place in code that references the camera, 
		// read the op code that loads it then use that to do a look up

		return *reinterpret_cast<Player*>(0x6ac664);
	}
}