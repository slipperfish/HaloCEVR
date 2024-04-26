#include "Menus.h"

bool Helpers::IsMouseVisible()
{
	return !*reinterpret_cast<bool*>(0x686a98);
}
