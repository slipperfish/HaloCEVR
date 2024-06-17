#include "Player.h"
#include "../Hooking/Hooks.h"

namespace Helpers
{
	Player& GetPlayer()
	{
		return *reinterpret_cast<Player*>(Hooks::o.LocalPlayer);
	}
}