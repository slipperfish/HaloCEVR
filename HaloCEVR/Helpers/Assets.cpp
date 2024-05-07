#include "Assets.h"

Asset* Helpers::GetAssetArray()
{
    return *reinterpret_cast<Asset**>(0x87bc14);;
}
