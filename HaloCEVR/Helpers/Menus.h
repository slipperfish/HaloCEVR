#pragma once

struct MouseInfo
{
	int x;
	int y;
	int z;
	char buttonState[8];
	char buttonState2[8];
};

namespace Helpers
{
	bool IsMouseVisible();
}