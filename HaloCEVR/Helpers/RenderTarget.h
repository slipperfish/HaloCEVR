#pragma once
#include <d3d9.h>

// There's an array of 8 of these kept in global memory
struct RenderTarget
{
	IDirect3DSurface9* renderSurface;
	IDirect3DTexture9* renderTexture;
	UINT width;
	UINT height;
	D3DFORMAT format;
};
static_assert(sizeof(RenderTarget) == 0x14);

namespace Helpers
{
	RenderTarget* GetRenderTargets();
}