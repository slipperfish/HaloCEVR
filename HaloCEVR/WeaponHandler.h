#pragma once
#include "Maths/Vectors.h"
#include "Maths/Matrices.h"
#include "Helpers/Objects.h"
#define DRAW_DEBUG_AIM 0

class WeaponHandler
{
public:
	void UpdateViewModel(struct HaloID& id, struct Vector3* pos, struct Vector3* facing, struct Vector3* up, struct TransformQuat* boneTransforms, struct Transform* outBoneTransforms);
	void PreFireWeapon(HaloID& weaponID, short param2, bool param3);
	void PostFireWeapon(HaloID& weaponID, short param2, bool param3);

	bool GetLocalWeaponAim(Vector3& outPosition, Vector3& outAim) const;
	bool GetWorldWeaponAim(Vector3& outPosition, Vector3& outAim) const;
	bool GetLocalWeaponScope(Vector3& outPosition, Vector3& outAim, Vector3& upDir) const;

	Vector3 localOffset;
	Vector3 localRotation;

protected:
	inline void CreateEndCap(int boneIndex, const struct Bone& currentBone, struct Transform* outBoneTransforms);
	inline void MoveBoneToTransform(int boneIndex, const class Matrix4& newTransform, struct Transform* realTransforms, struct Transform* outBoneTransforms);
	inline void UpdateCache(struct HaloID& id, struct AssetData_ModelAnimations* animationData);

	struct ViewModelCache
	{
		HaloID currentAsset{ 0, 0 };
		int leftWristIndex = 0;
		int rightWristIndex = 0;
		int gunIndex = 0;
		Vector3 cookedFireOffset;
		Matrix3 cookedFireRotation;
		Vector3 fireOffset;
		Matrix3 fireRotation;
	} cachedViewModel;

	UnitDynamicObject* weaponFiredPlayer = nullptr;
	Vector3 realPlayerPosition;
	Vector3 realPlayerAim;

	// Debug stuff for checking where bullets are coming from/going
#if DRAW_DEBUG_AIM
	mutable Vector3 lastFireLocation;
	mutable Vector3 lastFireAim;
#endif
};

