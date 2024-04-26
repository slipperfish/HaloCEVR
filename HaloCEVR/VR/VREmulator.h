#pragma once
#include "IVR.h"
#include <wtypes.h>
#include <d3d9.h>

class VREmulator : public IVR
{
public:
	// Start Interface IVR
	void Init();
	void OnGameFinishInit();
	void UpdatePoses();
	void PreDrawFrame(struct Renderer* renderer, float deltaTime);
	void PostDrawFrame(struct Renderer* renderer, float deltaTime);

	void UpdateCameraFrustum(struct CameraFrustum* frustum, int eye);

	int GetViewWidth() { return 600; }
	int GetViewHeight() { return 600; }
	float GetViewWidthStretch() { return 1.0f; }
	float GetViewHeightStretch() { return 1.0f; }
	float GetAspect() { return 1.0f; }
	void SetLocationOffset(Vector3 NewOffset) {}
	Vector3 GetLocationOffset() { return Vector3(0.0f, 0.0f, 0.0f); }
	void SetYawOffset(float NewOffset) {}
	float GetYawOffset() { return 0.0f; }
	Matrix4 GetHMDTransform(bool bRenderPose = false) { return Matrix4(); }
	virtual struct IDirect3DSurface9* GetRenderSurface(int eye);
	virtual struct IDirect3DTexture9* GetRenderTexture(int eye);
	virtual struct IDirect3DSurface9* GetUISurface();
	void UpdateInputs();
	InputBindingID RegisterBoolInput(std::string set, std::string action);
	InputBindingID RegisterVector2Input(std::string set, std::string action);
	bool GetBoolInput(InputBindingID id);
	bool GetBoolInput(InputBindingID id, bool& bHasChanged);
	Vector2 GetVector2Input(InputBindingID id);
	// End Interface IVR

protected:
	void CreateSharedTarget();

	void CreateTexAndSurface(int index, UINT Width, UINT Height, DWORD Usage, D3DFORMAT Format);

	void DrawEye(struct Renderer* renderer, float deltaTime, int eye);

	HWND hWnd;
	struct IDirect3DDevice9* MirrorDevice = nullptr;

	struct IDirect3DSurface9* UISurface;
	struct IDirect3DSurface9* EyeSurface_Game[2][2];
	struct IDirect3DSurface9* EyeSurface_VR[2][2];

	struct IDirect3DTexture9* EyeTexture_Game[2][2];
	struct IDirect3DTexture9* EyeTexture_VR[2][2];
};

