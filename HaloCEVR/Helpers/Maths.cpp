#include "Maths.h"

void Helpers::MakeTransformFromXZ(const Vector3* FacingVector, const Vector3* UpVector, Transform* OutTransform)
{
	OutTransform->Scale = 1.0;
	OutTransform->Rotation[0] = UpVector->x;
	OutTransform->Rotation[1] = UpVector->y;
	OutTransform->Rotation[2] = UpVector->z;
	OutTransform->Rotation[3] = FacingVector->y * UpVector->z - FacingVector->z * UpVector->y;
	OutTransform->Rotation[4] = FacingVector->z * UpVector->x - UpVector->z * FacingVector->x;
	OutTransform->Rotation[5] = FacingVector->x * UpVector->y - FacingVector->y * UpVector->x;
	OutTransform->Rotation[6] = FacingVector->x;
	OutTransform->Rotation[7] = FacingVector->y;
	OutTransform->Rotation[8] = FacingVector->z;
	(OutTransform->Translation).x = 0.0;
	(OutTransform->Translation).y = 0.0;
	(OutTransform->Translation).z = 0.0;
}

void Helpers::MakeTransformFromQuat(const Vector4* Quaternion, Transform* OutTransform)
{
	float fVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float twoOverLength;

	float len = Quaternion->w * Quaternion->w + Quaternion->z * Quaternion->z + Quaternion->y * Quaternion->y + Quaternion->x * Quaternion->x;
	if (len == 0.0f) {
		twoOverLength = 0.0f;
	}
	else {
		twoOverLength = 2.0f / len;
	}
	float twoX = twoOverLength * Quaternion->x;
	float twoY = twoOverLength * Quaternion->y;
	float twoZ = twoOverLength * Quaternion->z;
	fVar2 = twoX * Quaternion->w;
	fVar3 = twoY * Quaternion->w;
	fVar4 = twoZ * Quaternion->w;
	len = twoX * Quaternion->x;
	fVar5 = twoY * Quaternion->x;
	fVar6 = twoZ * Quaternion->x;
	fVar7 = twoY * Quaternion->y;
	fVar8 = twoZ * Quaternion->y;
	fVar1 = twoZ * Quaternion->z;
	OutTransform->Scale = 1.0f;
	(OutTransform->Translation).x = 0.0f;
	(OutTransform->Translation).y = 0.0f;
	(OutTransform->Translation).z = 0.0f;
	OutTransform->Rotation[0] = 1.0f - (fVar7 + fVar1);
	OutTransform->Rotation[1] = fVar5 - fVar4;
	OutTransform->Rotation[2] = fVar6 + fVar3;
	OutTransform->Rotation[3] = fVar5 + fVar4;
	OutTransform->Rotation[4] = 1.0f - (fVar1 + len);
	OutTransform->Rotation[5] = fVar8 - fVar2;
	OutTransform->Rotation[6] = fVar6 - fVar3;
	OutTransform->Rotation[7] = fVar8 + fVar2;
	OutTransform->Rotation[8] = 1.0f - (fVar7 + len);
}

void Helpers::CombineTransforms(const Transform* TransformA, const Transform* TransformB, Transform* OutTransform)
{
	Transform TempTransform;

	if (TransformA == OutTransform) {
		TempTransform = *TransformA;
		TransformA = &TempTransform;
	}
	if (TransformB == OutTransform) {
		TempTransform = *TransformB;
		TransformB = &TempTransform;
	}
	OutTransform->Rotation[0] =
		TransformB->Rotation[2] * TransformA->Rotation[6] +
		TransformA->Rotation[0] * TransformB->Rotation[0] + TransformA->Rotation[3] * TransformB->Rotation[1];
	OutTransform->Rotation[1] =
		TransformA->Rotation[4] * TransformB->Rotation[1] +
		TransformA->Rotation[1] * TransformB->Rotation[0] + TransformA->Rotation[7] * TransformB->Rotation[2];
	OutTransform->Rotation[2] =
		TransformA->Rotation[5] * TransformB->Rotation[1] +
		TransformA->Rotation[2] * TransformB->Rotation[0] + TransformA->Rotation[8] * TransformB->Rotation[2];
	OutTransform->Rotation[3] =
		TransformA->Rotation[3] * TransformB->Rotation[4] +
		TransformB->Rotation[5] * TransformA->Rotation[6] + TransformA->Rotation[0] * TransformB->Rotation[3];
	OutTransform->Rotation[4] =
		TransformA->Rotation[1] * TransformB->Rotation[3] +
		TransformA->Rotation[4] * TransformB->Rotation[4] + TransformA->Rotation[7] * TransformB->Rotation[5];
	OutTransform->Rotation[5] =
		TransformA->Rotation[2] * TransformB->Rotation[3] +
		TransformA->Rotation[5] * TransformB->Rotation[4] + TransformA->Rotation[8] * TransformB->Rotation[5];
	OutTransform->Rotation[6] =
		TransformA->Rotation[3] * TransformB->Rotation[7] +
		TransformA->Rotation[0] * TransformB->Rotation[6] + TransformB->Rotation[8] * TransformA->Rotation[6];
	OutTransform->Rotation[7] =
		TransformB->Rotation[7] * TransformA->Rotation[4] +
		TransformB->Rotation[8] * TransformA->Rotation[7] + TransformA->Rotation[1] * TransformB->Rotation[6];
	OutTransform->Rotation[8] =
		TransformB->Rotation[7] * TransformA->Rotation[5] +
		TransformB->Rotation[8] * TransformA->Rotation[8] + TransformA->Rotation[2] * TransformB->Rotation[6];
	(OutTransform->Translation).x =
		((TransformB->Translation).z * TransformA->Rotation[6] +
			TransformA->Rotation[3] * (TransformB->Translation).y + TransformA->Rotation[0] * (TransformB->Translation).x) * TransformA->Scale +
		(TransformA->Translation).x;
	(OutTransform->Translation).y =
		((TransformB->Translation).x * TransformA->Rotation[1] +
			(TransformB->Translation).y * TransformA->Rotation[4] + TransformA->Rotation[7] * (TransformB->Translation).z) * TransformA->Scale +
		(TransformA->Translation).y;
	(OutTransform->Translation).z =
		((TransformB->Translation).x * TransformA->Rotation[2] +
			(TransformB->Translation).y * TransformA->Rotation[5] + TransformA->Rotation[8] * (TransformB->Translation).z) * TransformA->Scale +
		(TransformA->Translation).z;
	OutTransform->Scale = TransformA->Scale * TransformB->Scale;
}
