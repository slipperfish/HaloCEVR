#include "Cutscene.h"
#include "../Hooking/Hooks.h"

CutsceneData* Helpers::GetCutsceneData()
{
    return *reinterpret_cast<CutsceneData**>(Hooks::o.CutsceneData);
}
