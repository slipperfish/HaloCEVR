#pragma once
#include "Objects.h"

struct Sound
{
public:
	char* N00000551; //0x0000
}; //Size: 0x0004

struct Bone
{
public:
	char BoneName[32]; //0x0000
	int16_t LeftLeaf; //0x0020
	int16_t RightLeaf; //0x0022
	int16_t Parent; //0x0024
	char pad_0026[26]; //0x0026
}; //Size: 0x0040

struct Animation
{
public:
	char N00000429[32]; //0x0000
	char pad_0020[12]; //0x0020
	int32_t NumBones; //0x002C
	char pad_0030[20]; //0x0030
	float N00000433; //0x0044
	char pad_0048[108]; //0x0048
}; //Size: 0x00B4

struct AssetData
{
public:
	char pad_0000[76]; //0x0000
	void* N00000324; //0x004C
	char pad_0050[8]; //0x0050
	void* N00000327; //0x0058
	char pad_005C[12]; //0x005C
	int32_t NumBones; //0x0068
	Bone* BoneArray; //0x006C
	char pad_0070[4]; //0x0070
	int32_t NumAnimations; //0x0074
	Animation* AnimationArray; //0x0078
	char pad_007C[20]; //0x007C
	int32_t NumSounds; //0x0090
	Sound* Sounds; //0x0094
	char pad_0098[4]; //0x0098
}; //Size: 0x161C

struct Asset
{
public:
	char pad_0000[12]; //0x0000
	HaloID SelfID; //0x000C
	char* Shader; //0x0010
	AssetData* Data; //0x0014
	char pad_0018[8]; //0x0018
};

namespace Helpers
{
	Asset* GetAssetArray();
}