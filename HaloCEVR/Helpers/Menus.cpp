#include "Menus.h"
#include "../Hooking/Hooks.h"

bool Helpers::IsMouseVisible()
{
	return !*reinterpret_cast<bool*>(Hooks::o.HideMouse);
}

bool Helpers::IsLoading()
{
    return *reinterpret_cast<int*>(Hooks::o.LoadingState) != 0;
}
