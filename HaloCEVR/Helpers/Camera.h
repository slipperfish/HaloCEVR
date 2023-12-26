#pragma once
#include "Vector3.h"
#include <cstdint>

struct Camera
{
	// Camera position
	Vector3 position;

	// Unknown
	std::uint8_t unk_0[20];

	// Camera look/facing direction
	Vector3 lookDir;
	// Needs verifying, probably an up vector
	Vector3 lookDirUp;

	// Field of View in radians, typically 1.22 (70 deg)
	float fov;
};
static_assert(sizeof(Camera) == 0x3c);

namespace Helpers
{
	Camera& GetCamera();
}