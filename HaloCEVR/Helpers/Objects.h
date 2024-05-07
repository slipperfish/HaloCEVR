#pragma once
#include <cstdint>
#include <ostream>
#include "../Maths/Vectors.h"

struct HaloID
{
	uint16_t Index; //0x0000
	uint16_t ID; //0x0002
};

inline std::ostream& operator<<(std::ostream& os, const HaloID& id) 
{
	os << "[ID: " << id.ID << ", Index: " << id.Index << "]";
	return os;
}

inline bool operator==(HaloID& a, HaloID& b)
{
	return a.ID == b.ID && a.Index == b.Index;
}

enum class ObjectProperties : uint16_t
{
	NoCollision = 1 << 0,
	OnGround = 1 << 1,
	NoGravity = 1 << 2,
	Unk0 = 1 << 3,
	Unk1 = 1 << 4,
	Stationary = 1 << 5,
	Unk2 = 1 << 6,
	NoCollision2 = 1 << 7,
	Unk3 = 1 << 8,
	Unk4 = 1 << 9,
	Unk5 = 1 << 10,
	NoShadow = 1 << 11,
	Unk6 = 1 << 12,
	Unk7 = 1 << 13,
	OutOfBounds = 1 << 14,
	Unk8 = 1 << 15
};

enum class ObjectType : uint16_t
{
	BIPED,
	VEHICLE,
	WEAPON,
	EQUIPMENT,
	GARBAGE,
	PROJECTILE,
	SCENERY,
	DEVICE_MACHINE,
	DEVICE_CONTROL,
	DEVICE_LIGHT_FIXTURE,
	PLACEHOLDER,
	SOUND_SCENERY
};

struct BaseTable
{
public:
	char Name[32]; //0x0000
	uint16_t MaxElements; //0x0020
	uint16_t ElementSize; //0x0022
	uint32_t N00000198; //0x0024
	uint32_t N00000199; //0x0028
	uint16_t N000001B3; //0x002C
	uint16_t CurrentSize; //0x002E
	uint16_t Count; //0x0030
	uint16_t NextID; //0x0032
};

struct ObjectTable : public BaseTable
{
	struct ObjectDatum* FirstElement; //0x0034
};

struct ObjectDatum
{
	uint16_t ID; //0x0000
	uint16_t N000001BD; //0x0002
	uint16_t N00000231; //0x0004
	uint16_t N00000233; //0x0006
	struct BaseDynamicObject* DynamicObject; //0x0008
};

struct BaseDynamicObject
{
public:
	HaloID TagID; //0x0000
	uint32_t N0000025C; //0x0004
	uint32_t N0000025D; //0x0008
	int32_t Age; //0x000C
	ObjectProperties N0000025F; //0x0010
	char pad_0012[2]; //0x0012
	float N00000260; //0x0014
	float N00000261; //0x0018
	float N00000262; //0x001C
	float N00000263; //0x0020
	float N00000264; //0x0024
	float N00000265; //0x0028
	float N00000266; //0x002C
	float N00000267; //0x0030
	float N00000268; //0x0034
	float N00000269; //0x0038
	float N0000026A; //0x003C
	float N0000026B; //0x0040
	float N0000026C; //0x0044
	float N0000026D; //0x0048
	float N0000026E; //0x004C
	float N0000026F; //0x0050
	float N00000270; //0x0054
	float N00000271; //0x0058
	Vector3 Position; //0x005C
	Vector3 Velocity; //0x0068
	Vector3 FacingDir; //0x0074
	Vector3 UpDirection; //0x0080
	float RotVelPitch; //0x008C
	float RotVelYaw; //0x0090
	float RotVelRoll; //0x0094
	uint32_t LocationID; //0x0098
	char pad_009C[4]; //0x009C
	Vector3 Centre; //0x00A0
	float N0000027C; //0x00AC
	float Scale; //0x00B0
	ObjectType N0000027E; //0x00B4
	uint16_t N0000027F; //0x00B6
	char pad_00B8[20]; //0x00B8
	uint32_t AnimTagID; //0x00CC
	uint16_t Animation; //0x00D0
	uint16_t AnimFrame; //0x00D2
	char pad_00D4[4]; //0x00D4
	float BaseHealth; //0x00D8
	float BaseShield; //0x00DC
	float Health; //0x00E0
	float Shield; //0x00E4
	float CurrentShieldDmg; //0x00E8
	float CurrentHealthDmg; //0x00EC
	char pad_00F0[4]; //0x00F0
	float RecentShieldDmg; //0x00F4
	float RecentHealthDmg; //0x00F8
	int32_t ShieldDmgTime; //0x00FC
	int32_t HealthDmgTime; //0x0100
	uint16_t ShieldStunTime; //0x0104
	uint16_t N00000311; //0x0106
	char pad_0108[16]; //0x0108
	HaloID Weapon; //0x0118
	HaloID Parent; //0x011C
	uint32_t ParentSeatIndex; //0x0120
	char pad_0124[208]; //0x0124
};

struct UnitDynamicObject : public BaseDynamicObject
{
	char pad_01F4[36]; //0x01F4
	HaloID PlayerID; //0x0218
	uint16_t N00000313; //0x021C
	uint16_t N0000034E; //0x021E
	uint32_t LastBulletTime; //0x0220
	Vector3 Facing; //0x0224
	Vector3 DesiredAim; //0x0230
	Vector3 Aim; //0x023C
	Vector3 AimVelocity; //0x0248
	Vector3 Aim2; //0x0254
	Vector3 Aim3; //0x0260
	char pad_026C[12]; //0x026C
	float Run; //0x0278
	float Strafe; //0x027C
	float Ascend; //0x0280
	float Shooting; //0x0284
	char pad_0288[12]; //0x0288
	HaloID ThrownGrenade; //0x0294
	char pad_0298[32]; //0x0298
	float N0000032E; //0x02B8
	float N0000032F; //0x02BC
	float N00000330; //0x02C0
	float N00000331; //0x02C4
	float N00000332; //0x02C8
	float N00000333; //0x02CC
	float N00000334; //0x02D0
	float N00000335; //0x02D4
	char pad_02D8[80]; //0x02D8
};

struct PlayerTable : public BaseTable
{
	struct PlayerDatum* FirstPlayer; //0x0034
};

struct PlayerDatum
{
public:
	uint16_t PlayerID; //0x0000
	uint16_t LocalID; //0x0002
	char Name[24]; //0x0004
	char pad_001C[4]; //0x001C
	int8_t Team; //0x0020
	char pad_0021[3]; //0x0021
	HaloID InteractionObjectID; //0x0024
	uint16_t InteractionObjectType; //0x0028
	uint16_t InteractionObjectSeat; //0x002A
	uint32_t RespawnTime; //0x002C
	uint32_t RespawnTimeGrowth; //0x0030
	HaloID ObjectID; //0x0034
	char pad_0038[216]; //0x0038
};

namespace Helpers
{
	ObjectTable& GetObjectTable();
	PlayerTable& GetPlayerTable();

	BaseDynamicObject* GetDynamicObject(HaloID& ID);
	bool GetLocalPlayerID(HaloID& OutID);
	BaseDynamicObject* GetLocalPlayer();
}