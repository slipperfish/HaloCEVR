#pragma once
#include "../Maths/Vectors.h"
#include <cstdint>

struct InputData
{
	char pad_0000[8]; //0x0000
	float N00000679; //0x0008
	float N0000067A; //0x000C
	uint16_t N0000067B; //0x0010
	uint16_t N00000696; //0x0012
	uint32_t jumpState; //0x0014
	uint16_t N0000067D; //0x0018
	uint16_t N00000693; //0x001A
	float yaw; //0x001C
	float pitch; //0x0020
	float forward; //0x0024
	float left; //0x0028
	float firing; //0x002C
	uint16_t N00000683; //0x0030
	uint16_t N00000690; //0x0032
	int16_t zoomLevel; //0x0034
	char pad_0036[6]; //0x0036
	float N00000686; //0x003C
};

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
	InputData& GetInputData();
}