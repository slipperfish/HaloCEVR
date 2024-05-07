#pragma once
#include "../Maths/Matrices.h"

struct Transform
{
	float Scale;
	float Rotation[9];
	Vector3 Translation;
};

struct TransformQuat
{
	Vector4 Rotation; // Quaternion
	Vector3 Translation;
	float Scale;
};

namespace Helpers
{
	void MakeTransformFromXZ(const Vector3* FacingVector, const Vector3* UpVector, Transform* OutTransform);
	void MakeTransformFromQuat(const Vector4* Quaternion, Transform* OutTransform);
	void CombineTransforms(const Transform* TransformA, const Transform* TransformB, Transform* OutTransform);
}