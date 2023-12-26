#pragma once

struct Vector4
{
	float x;
	float y;
	float z;
	float w;
};
static_assert(sizeof(Vector4) == 0x10);