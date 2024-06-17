#include "Controls.h"
#include "../Hooking/Hooks.h"

namespace Helpers
{
	Controls& GetControls()
	{
		return *reinterpret_cast<Controls*>(Hooks::o.Controls);
	}
}