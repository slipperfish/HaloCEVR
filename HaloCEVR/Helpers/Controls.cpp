#include "Controls.h"

namespace Helpers
{
	Controls& GetControls()
	{
		// TODO: Use sigs

		return *reinterpret_cast<Controls*>(0x712498);
	}
}