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
	void Recentre() {};
	void SetLocationOffset(Vector3 newOffset) {}
	Vector3 GetLocationOffset() { return Vector3(0.0f, 0.0f, 0.0f); }
	void SetYawOffset(float newOffset) {}
	float GetYawOffset() { return 0.0f; }
	Matrix4 GetHMDTransform(bool bRenderPose = false) { return Matrix4(); }
	Matrix4 GetControllerTransform(ControllerRole Role, bool bRenderPose = false);
	virtual struct IDirect3DSurface9* GetRenderSurface(int eye);
	virtual struct IDirect3DTexture9* GetRenderTexture(int eye);
	virtual struct IDirect3DSurface9* GetUISurface();
	void SetMouseVisibility(bool bIsVisible) {}
	void UpdateInputs();
	InputBindingID RegisterBoolInput(std::string set, std::string action);
	InputBindingID RegisterVector2Input(std::string set, std::string action);
	bool GetBoolInput(InputBindingID id);
	bool GetBoolInput(InputBindingID id, bool& bHasChanged);
	Vector2 GetVector2Input(InputBindingID id);
	Vector2 GetMousePos() { return Vector2(0.0f, 0.0f); }
	bool GetMouseDown() { return false; }
	// End Interface IVR

protected:
	void CreateSharedTarget();

	void CreateTexAndSurface(int index, UINT width, UINT height, DWORD usage, D3DFORMAT format);

	void DrawEye(struct Renderer* renderer, float deltaTime, int eye);

	HWND hWnd;
	struct IDirect3DDevice9* mirrorDevice = nullptr;

	struct IDirect3DSurface9* uiSurface = nullptr;
	struct IDirect3DSurface9* eyeSurface_Game[2][2];
	struct IDirect3DSurface9* eyeSurface_VR[2][2];

	struct IDirect3DTexture9* eyeTexture_Game[2][2];
	struct IDirect3DTexture9* eyeTexture_VR[2][2];

	struct Binding
	{
		std::string bindingName;
		int virtualKey = 0;
		bool bHasChanged = false;
		bool bPressed = false;
	};

	Binding bindings[12] = {
		{"Jump", VK_SPACE},
		{"SwitchGrenades", 'G'},
		{"Interact", 'E'},
		{"SwitchWeapons", VK_TAB},
		{"Melee", 'Q'},
		{"Flashlight", 'F'},
		{"Grenade", VK_RBUTTON},
		{"Fire", VK_LBUTTON},
		{"MenuBack", 'P'}, // Intentionally weird binding because we don't override this in the same way and it would conflict
		{"Crouch", VK_LCONTROL},
		{"Zoom", 'Z'},
		{"Reload", 'R'},
	};

	struct AxisBinding
	{
		int virtualKey = 0;
		int scale = 0;
		int axisId = 0;
	};

	struct Axis2D
	{
		std::string axisName;
		int indexX = 0;
		int indexY = 0;
	};

	float axes1D[2] = {
		0.0f,
		0.0f
	};

	Axis2D axes2D[1] =
	{
		{"Move", 0, 1}
	};

	AxisBinding axisBindings[4] =
	{
		{'W', 1, 1},
		{'S', -1, 1},
		{'A', -1, 0},
		{'D', 1, 0},
	};
};

