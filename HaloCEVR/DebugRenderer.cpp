#include "DebugRenderer.h"
#include "Helpers/RenderTarget.h"
#include "Helpers/Renderer.h"
#include "Helpers/Maths.h"
#include "Logger.h"
#include "Game.h"


#pragma optimize("", off)

void DebugRenderer::Init(struct IDirect3DDevice9* pDevice)
{
}

int DebugRenderer::AddLine2D(Vector2 start, Vector2 end, D3DCOLOR color)
{
	int index = vertex2DCount;
	
	SetLine2D(index, start, end, color);
	
	vertex2DCount += 2;

	vertex2DCount = vertex2DCount % MAX_LINES;

	return index;
}

void DebugRenderer::Shutdown()
{
}

void DebugRenderer::ExtractMatrices()
{

	// This really needs replacing with signatures/structures, but eh
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

	CameraRenderMatrices& cameraMatrices2 = *reinterpret_cast<CameraRenderMatrices*>(0x7c127c);
	CameraRenderMatrices& cameraMatrices = *reinterpret_cast<CameraRenderMatrices*>(0x7c3168);

	// The view matrix is bugged for left eye, this is only for debug so just hack around it
	//if (Game::instance.GetRenderState() == ERenderState::RIGHT_EYE)
	{
		view._13 = cameraMatrices.viewMatrix.rotation[2];
		view._12 = cameraMatrices.viewMatrix.rotation[1];
		view._23 = cameraMatrices.viewMatrix.rotation[5];
		view._11 = cameraMatrices.viewMatrix.rotation[0];
		view._22 = cameraMatrices.viewMatrix.rotation[4];
		view._33 = cameraMatrices.viewMatrix.rotation[8];
		view._21 = cameraMatrices.viewMatrix.rotation[3];
		view._32 = cameraMatrices.viewMatrix.rotation[7];
		view._43 = cameraMatrices.viewMatrix.translation.z;
		view._31 = cameraMatrices.viewMatrix.rotation[6];
		view._42 = cameraMatrices.viewMatrix.translation.y;
		view._14 = 0.0;
		view._24 = 0.0;
		view._34 = 0.0;
		view._41 = cameraMatrices.viewMatrix.translation.x;
		view._44 = 1.0;
	}
	world._43 = 0.0;
	world._42 = 0.0;
	world._41 = 0.0;
	world._34 = 0.0;
	world._32 = 0.0;
	world._31 = 0.0;
	world._24 = 0.0;
	world._23 = 0.0;
	world._21 = 0.0;
	world._14 = 0.0;
	world._13 = 0.0;
	world._12 = 0.0;
	world._44 = 1.0;
	world._33 = 1.0;
	world._22 = 1.0;
	world._11 = 1.0;
	projection = cameraMatrices.projectionMatrix;
}

int DebugRenderer::AddLine3D(Vector3 start, Vector3 end, D3DCOLOR color)
{
	int index = vertex3DCount;

	SetLine3D(index, start, end, color);

	vertex3DCount += 2;

	vertex3DCount = vertex3DCount % MAX_LINES;

	return index;
}

void DebugRenderer::SetLine2D(int index, Vector2 start, Vector2 end, D3DCOLOR color)
{
	vertices2D[index].x = start.x;
	vertices2D[index].y = start.y;
	vertices2D[index].z = 0.0f;
	vertices2D[index].rhw = 1.0f;
	vertices2D[index].color = color;
	index++;
	vertices2D[index].x = end.x;
	vertices2D[index].y = end.y;
	vertices2D[index].z = 0.0f;
	vertices2D[index].rhw = 1.0f;
	vertices2D[index].color = color;
}

void DebugRenderer::SetLine3D(int index, Vector3 start, Vector3 end, D3DCOLOR color)
{
	vertices3D[index].x = start.x;
	vertices3D[index].y = start.y;
	vertices3D[index].z = start.z;
	vertices3D[index].color = color;
	index++;
	vertices3D[index].x = end.x;
	vertices3D[index].y = end.y;
	vertices3D[index].z = end.z;
	vertices3D[index].color = color;
}

void DebugRenderer::Render(struct IDirect3DDevice9* pDevice)
{
	LPDIRECT3DSTATEBLOCK9 pStateBlock = NULL;
	pDevice->CreateStateBlock(D3DSBT_ALL, &pStateBlock);

	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	pDevice->SetRenderState(D3DRS_LIGHTING, false);
	pDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	pDevice->DrawPrimitiveUP(D3DPT_LINELIST, vertex2DCount / 2, vertices2D, sizeof(VertexData2D));
	pDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);

	ExtractMatrices();
	
	pDevice->SetTransform(D3DTS_WORLD, &world);
	pDevice->SetTransform(D3DTS_VIEW, &view);
	pDevice->SetTransform(D3DTS_PROJECTION, &projection);

	//Logger::log << (int)Game::instance.GetRenderState() << std::endl;

	auto LogMatrix = [](D3DMATRIX& m) {
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				Logger::log << m.m[i][j] << " ";
			}
		}
		Logger::log << std::endl;
	};

	//LogMatrix(world);
	//LogMatrix(view);
	//LogMatrix(projection);

	for (int i = 0; i < vertex3DCount; i++)
	{
		Vector3 centre = Vector3(vertices3D[i].x, vertices3D[i].y, vertices3D[i].z);
		DWORD color = vertices3D[i].color;
		float size = 0.2f;

		const float halfSize = size * 0.5f;
		VertexData3D vertices[] = {
			{centre.x - halfSize, centre.y + halfSize, centre.z - halfSize, color}, // 0
			{centre.x - halfSize, centre.y - halfSize, centre.z - halfSize, color}, // 1
			{centre.x + halfSize, centre.y + halfSize, centre.z - halfSize, color}, // 2
			{centre.x + halfSize, centre.y - halfSize, centre.z - halfSize, color}, // 3
			{centre.x - halfSize, centre.y + halfSize, centre.z + halfSize, color}, // 4
			{centre.x - halfSize, centre.y - halfSize, centre.z + halfSize, color}, // 5
			{centre.x + halfSize, centre.y + halfSize, centre.z + halfSize, color}, // 6
			{centre.x + halfSize, centre.y - halfSize, centre.z + halfSize, color}  // 7
		};

		WORD indices[] = {
			0, 1, 3, 0, 3, 2,
			7, 5, 4, 6, 7, 4,
			4, 0, 2, 4, 2, 6,
			1, 5, 7, 1, 7, 3,
			5, 1, 0, 4, 5, 0,
			2, 3, 7, 2, 7, 6
		};
		pDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, 8, 12, indices, D3DFMT_INDEX16, vertices, sizeof(VertexData3D));
	}

	pDevice->DrawPrimitiveUP(D3DPT_LINELIST, vertex3DCount / 2, vertices3D, sizeof(VertexData3D));

	pStateBlock->Apply();
	pStateBlock->Release();
}
#pragma optimize("", on)
