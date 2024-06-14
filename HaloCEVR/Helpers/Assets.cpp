#include "Assets.h"

Asset_Generic* Helpers::GetAssetArray()
{
    return *reinterpret_cast<Asset_Generic**>(0x87bc14);
}
