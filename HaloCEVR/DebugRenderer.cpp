#include "DebugRenderer.h"
#include "Helpers/RenderTarget.h"
#include "Helpers/Renderer.h"
#include "Helpers/Maths.h"
#include "Logger.h"
#include "Game.h"
#include "Helpers/Camera.h"
#include "Hooking/Hooks.h"

//================================//Debug API//================================//

void DebugRenderer::DrawLine2D(Vector2& start, Vector2& end, D3DCOLOR color)
{
	VertexData2D startData
	{
		start.x,
		start.y,
		0.0f,
		1.0f,
		color

	};
	lines2D.push_back(startData);

	VertexData2D endData
	{
		end.x,
		end.y,
		0.0f,
		1.0f,
		color

	};
	lines2D.push_back(endData);
}

void DebugRenderer::DrawLine3D(Vector3& start, Vector3& end, D3DCOLOR color, bool bRespectDepth, float thickness)
{
	VertexData3D startData
	{
		start.x,
		start.y,
		start.z,
		color

	};

	if (bRespectDepth)
	{
		depthLines3D.push_back(startData);
	}
	else
	{
		lines3D.push_back(startData);
	}

	VertexData3D endData
	{
		end.x,
		end.y,
		end.z,
		color

	};

	if (bRespectDepth)
	{
		depthLines3D.push_back(endData);
	}
	else
	{
		lines3D.push_back(endData);
	}
}

void DebugRenderer::DrawCoordinate(Vector3& pos, Matrix3& rot, float size, bool bRespectDepth)
{
	// TODO: Check these are the right way round
	Vector3 up = pos + rot * Vector3(0.0f, 0.0f, size);
	Vector3 forward = pos + rot * Vector3(0.0f, size, 0.0f);
	Vector3 left = pos + rot * Vector3(size, 0.0f, 0.0f);

	DrawLine3D(pos, up, D3DCOLOR_XRGB(0, 0, 255), bRespectDepth, 0.01f);
	DrawLine3D(pos, forward, D3DCOLOR_XRGB(0, 255, 0), bRespectDepth, 0.01f);
	DrawLine3D(pos, left, D3DCOLOR_XRGB(255, 0, 0), bRespectDepth, 0.01f);
}

void DebugRenderer::DrawRenderTarget(IDirect3DSurface9* renderTarget, Vector3& pos, Matrix3& rot, Vector2& size)
{
	// todo
}

//================================//Core Functions//================================//

void DebugRenderer::ExtractMatrices(Renderer* playerRenderer)
{
	CameraRenderMatrices& cameraMatrices = *Helpers::GetActiveCameraMatrices();
	Game::instance.SetViewportScale(&cameraMatrices.viewport);

	Hooks::SetCameraMatrices(&cameraMatrices.viewport, &playerRenderer->frustum, &cameraMatrices, true);


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

void DebugRenderer::Render(IDirect3DDevice9* pDevice)
{
	LPDIRECT3DSTATEBLOCK9 pStateBlock = NULL;
	pDevice->CreateStateBlock(D3DSBT_ALL, &pStateBlock);

	Draw2DLines(pDevice);

	Draw3DLines(pDevice);

	pStateBlock->Apply();
	pStateBlock->Release();
}

void DebugRenderer::PostRender()
{
	lines2D.clear();
	lines3D.clear();
	depthLines3D.clear();
	renderTargets.clear();
	renderTargetCoords.clear();
}

//================================//Internal Functions//================================//

void DebugRenderer::Draw2DLines(IDirect3DDevice9* pDevice)
{
	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	pDevice->SetRenderState(D3DRS_LIGHTING, false);
	pDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	pDevice->DrawPrimitiveUP(D3DPT_LINELIST, vertex2DCount / 2, vertices2D, sizeof(VertexData2D));
}

void DebugRenderer::Draw3DLines(IDirect3DDevice9* pDevice)
{
	if (lines3D.empty() && depthLines3D.empty())
	{
		return;
	}

	pDevice->SetRenderState(D3DRS_ZENABLE, true);
	pDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	pDevice->SetTransform(D3DTS_WORLD, &world);
	pDevice->SetTransform(D3DTS_VIEW, &view);
	pDevice->SetTransform(D3DTS_PROJECTION, &projection);

	// TODO: replace this with actual line thickness
	/*
	for (int i = 0; i < lines3D.size(); i++)
	{
		Vector3 centre = Vector3(vertices3D[i].x, vertices3D[i].y, vertices3D[i].z);
		DWORD color = vertices3D[i].color;
		float size = 0.05f;

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
	*/

	pDevice->DrawPrimitiveUP(D3DPT_LINELIST, depthLines3D.size() / 2, depthLines3D.data(), sizeof(VertexData3D));
	pDevice->SetRenderState(D3DRS_ZENABLE, false);
	pDevice->DrawPrimitiveUP(D3DPT_LINELIST, lines3D.size() / 2, lines3D.data(), sizeof(VertexData3D));
}
