#pragma once
#include <cstdint>
#include "Vector3.h"
#include "Vector4.h"

struct sRect
{
	short top;
	short left;
	short bottom;
	short right;
};
static_assert(sizeof(sRect) == 0x8);

struct CameraFrustum
{
	Vector3 position;
	Vector3 facingDirection;
	Vector3 upDirection;
	// Maybe meant to control first vs third person?
	bool drawPlayer;
	std::uint8_t unk0[3];
	float fov;
	sRect oViewport;
	sRect Viewport;
	// Not sure if these are the znear/far values, but they have values in the correct range
	float zNear;
	float zFar;
	Vector4 unk1;
};
static_assert(sizeof(CameraFrustum) == 0x54);

struct Renderer
{
	short playerId;
	bool unk1;
	// Possibly just padding
	std::uint8_t unk2;
	// For reasons I've yet to decipher there are 2 nearly identical structs that both need setting
	// Maybe one does culling and one does rendering? Setting one of them doesn't seem to work
	CameraFrustum frustum;
	CameraFrustum frustum2;
};
static_assert(sizeof(Renderer) == 0xac);