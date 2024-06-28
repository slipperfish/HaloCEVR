#pragma once
#include "Maths/Vectors.h"
#include <d3d9.h>


class DebugRenderer
{
public:
	void Init(struct IDirect3DDevice9* pDevice);
	int AddLine2D(Vector2 start, Vector2 end, D3DCOLOR color);
	int AddLine3D(Vector3 start, Vector3 end, D3DCOLOR color);
	void SetLine2D(int index, Vector2 start, Vector2 end, D3DCOLOR color);
	void SetLine3D(int index, Vector3 start, Vector3 end, D3DCOLOR color);
	void Render(struct IDirect3DDevice9* pDevice);
	void Shutdown();

protected:

	void ExtractMatrices();

	static constexpr int MAX_LINES = 32;

	struct VertexData2D
	{
		float x, y, z, rhw;
		DWORD color;
	};

	struct VertexData3D
	{
		float x, y, z;
		DWORD color;
	};

	VertexData2D vertices2D[MAX_LINES];
	VertexData3D vertices3D[MAX_LINES];
	int vertex2DCount = 0;
	int vertex3DCount = 0;

	D3DMATRIX world;
	D3DMATRIX view;
	D3DMATRIX projection;
};
