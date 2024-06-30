#pragma once
#include <cstdint>
#include <d3d9.h>
#include "../Maths/Vectors.h"
#include "Maths.h"

struct Viewport
{
	float left;
	float right;
	float top;
	float bottom;
};
static_assert(sizeof(Viewport) == 0x10);

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
	sRect WindowViewport;
	sRect InnerViewport;
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

struct CameraRenderMatrices
{
	Viewport viewport;
	Transform viewMatrix;
	Transform matrix;
	Vector4 quaternions[6];
	float zNear;
	float zFar;
	Vector3 vectors[4];
	Vector3 cameraPosition;
	Vector3 vector;
	float floats[6];
	uint32_t unk;
	D3DMATRIX projectionMatrix;
};

namespace Helpers
{
	sRect* GetWindowRect();
	CameraRenderMatrices* GetActiveCameraMatrices();
}