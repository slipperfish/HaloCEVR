#pragma once
#include "IVR.h"
#include <wtypes.h>
#include <d3d9.h>
#include <d3d11.h>
#include "../../ThirdParty/OpenVR/include/openvr.h"

class OpenVR : public IVR
{
public:
	// Start Interface IVR
	void Init() override;
	void OnGameFinishInit();
	void UpdatePoses() override;
	void PreDrawFrame(Renderer* renderer, float deltaTime) override;
	void PostDrawFrame(Renderer* renderer, float deltaTime) override;
	void UpdateCameraFrustum(CameraFrustum* frustum, int eye) override;
	int GetViewWidth() override;
	int GetViewHeight() override;
	float GetViewWidthStretch() override;
	float GetViewHeightStretch() override;
	float GetAspect() override;
	void SetLocationOffset(Vector3 NewOffset) override;
	Vector3 GetLocationOffset() override;
	void SetYawOffset(float NewOffset) override;
	float GetYawOffset() override;
	Matrix4 GetHMDTransform(bool bRenderPose = false) override;
	IDirect3DSurface9* GetRenderSurface(int eye) override;
	IDirect3DTexture9* GetRenderTexture(int eye) override;
	IDirect3DSurface9* GetUISurface() override;
	void UpdateInputs() override;
	InputBindingID RegisterBoolInput(std::string set, std::string action) override;
	InputBindingID RegisterVector2Input(std::string set, std::string action) override;
	bool GetBoolInput(InputBindingID id) override;
	bool GetBoolInput(InputBindingID id, bool& bHasChanged) override;
	Vector2 GetVector2Input(InputBindingID id) override;
	// End Interface IVR

protected:
	void CreateTexAndSurface(int index, UINT Width, UINT Height, DWORD Usage, D3DFORMAT Format);
	void PositionOverlay();

	vr::IVRSystem* VRSystem;
	vr::IVRCompositor* VRCompositor;
	vr::IVRInput* VRInput;
	vr::IVROverlay* VROverlay;

	vr::VRTextureBounds_t TextureBounds[2];
	uint32_t RealWidth;
	uint32_t RealHeight;
	uint32_t RecommendedWidth;
	uint32_t RecommendedHeight;
	float Aspect;
	float FOV;

	Vector3 PositionOffset;
	float YawOffset;

	vr::VRActiveActionSet_t ActionSets[1];

	vr::VROverlayHandle_t UIOverlay;

	vr::TrackedDevicePose_t GamePoses[vr::k_unMaxTrackedDeviceCount];
	vr::TrackedDevicePose_t RenderPoses[vr::k_unMaxTrackedDeviceCount];

	IDirect3DSurface9* GameRenderSurface[3];
	IDirect3DTexture9* GameRenderTexture[3];

	ID3D11Device* D3DDevice;

	ID3D11Texture2D* VRRenderTexture[3];


	Matrix4 ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t& matPose)
	{
		Matrix4 matrixObj(
			 matPose.m[2][2],  matPose.m[0][2], -matPose.m[1][2], 0.0,
			 matPose.m[2][0],  matPose.m[0][0], -matPose.m[1][0], 0.0,
			-matPose.m[2][1], -matPose.m[0][1],  matPose.m[1][1], 0.0,
			-matPose.m[2][3], -matPose.m[0][3],  matPose.m[1][3], 1.0f
		);

		return matrixObj;
	}

	Matrix4 GetHMDMatrixPoseEye(vr::Hmd_Eye nEye)
	{
		if (!VRSystem)
			return Matrix4();

		vr::HmdMatrix34_t matEyeRight = VRSystem->GetEyeToHeadTransform(nEye);

		Matrix4 matrixObj = ConvertSteamVRMatrixToMatrix4(matEyeRight);

		return matrixObj.invert();
	}

	Matrix3 GetRotationMatrix(Matrix4& matrix)
	{
		return Matrix3(
			matrix[0], matrix[1], matrix[2],
			matrix[4], matrix[5], matrix[6],
			matrix[8], matrix[9], matrix[10]
		);
	}

	Vector3 GetVector3(Vector4& v)
	{
		return Vector3(v.x, v.y, v.z);
	}
};

