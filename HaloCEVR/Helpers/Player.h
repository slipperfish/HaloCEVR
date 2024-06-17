#pragma once
#include "Camera.h"

// The local player in campaign mode

struct Player
{
	float deltaTime;
	// Unknown
	uint8_t unk_0[116];
	// Camera
	Camera camera;
	// Position
	Vector3 position;
	// Unknown
	uint8_t unk_1[20];
	// Field of View in radians, typically 1.22 (70 deg)
	float fov;
	// Camera look/facing direction
	Vector3 lookDir;
	// Needs verifying, probably an up vector
	Vector3 lookDirUp;
	// Unknown
	uint8_t unk_2[440];
};
static_assert(sizeof(Player) == 0x2a8);

namespace Helpers
{
	Player& GetPlayer();
}