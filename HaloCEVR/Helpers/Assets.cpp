#include "Assets.h"
#include "../Hooking/Hooks.h"

Asset_Generic* Helpers::GetAssetArray()
{
    return *reinterpret_cast<Asset_Generic**>(Hooks::o.AssetsArray);
}
