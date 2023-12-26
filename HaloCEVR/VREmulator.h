#pragma once

class VREmulator
{
public:
	void Init();

	void PreDrawFrame(struct Renderer* renderer, float deltaTime);
	void PreDrawEye(struct Renderer* renderer, float deltaTime, int eye);
	void PostDrawEye(struct Renderer* renderer, float deltaTime, int eye);
	void PostDrawFrame(struct Renderer* renderer, float deltaTime);

protected:
	void CreateSharedTarget();

	void CreateTexAndSurface(int index, UINT Width, UINT Height, DWORD Usage, D3DFORMAT Format);

	HWND hWnd;
	struct IDirect3DDevice9* MirrorDevice = nullptr;

	/*
	HANDLE SharedHandleL = 0;
	HANDLE SharedHandleR = 0;
	struct IDirect3DSurface9* SharedTarget_L = nullptr;
	struct IDirect3DSurface9* SharedTarget_L_M = nullptr;

	struct IDirect3DSurface9* SharedTarget_R = nullptr;
	struct IDirect3DSurface9* SharedTarget_R_M = nullptr;

	struct IDirect3DTexture9* LeftEyeTexture = nullptr;
	struct IDirect3DTexture9* RightEyeTexture = nullptr;

	struct IDirect3DTexture9* LeftEyeTexture_M = nullptr;
	struct IDirect3DTexture9* RightEyeTexture_M = nullptr;
	*/

	// ----

	struct IDirect3DSurface9* EyeSurface_Game[2][2];
	struct IDirect3DSurface9* EyeSurface_VR[2][2];

	struct IDirect3DTexture9* EyeTexture_Game[2][2];
	struct IDirect3DTexture9* EyeTexture_VR[2][2];
};

