#pragma once
#include <cstdint>

struct CutsceneData
{
	float progress;
	float lastUpdate;
	bool bHasCinematicBars;
	bool bInCutscene;
	uint8_t padding[2];
	uint32_t data[4];
};

namespace Helpers
{
	CutsceneData* GetCutsceneData();
}