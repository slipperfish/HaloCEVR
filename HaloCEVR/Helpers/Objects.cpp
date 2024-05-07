#include "Objects.h"

ObjectTable& Helpers::GetObjectTable()
{
	return **reinterpret_cast<ObjectTable**>(0x8603b0);
}

PlayerTable& Helpers::GetPlayerTable()
{
	return **reinterpret_cast<PlayerTable**>(0x87a480);
}

BaseDynamicObject* Helpers::GetDynamicObject(HaloID& ID)
{
	ObjectTable& Table = GetObjectTable();

	if (ID.Index > Table.CurrentSize)
	{
		return nullptr;
	}

	return Table.FirstElement[ID.Index].DynamicObject;
}

bool Helpers::GetLocalPlayerID(HaloID& OutID)
{
	PlayerTable& Table = GetPlayerTable();

	if (Table.CurrentSize == 0)
	{
		return false;
	}

	OutID = Table.FirstPlayer->ObjectID;
	return true;
}

BaseDynamicObject* Helpers::GetLocalPlayer()
{
	// TODO: Test this in MP, it may not be that the first player is the local one

	HaloID PlayerID;
	if (!GetLocalPlayerID(PlayerID))
	{
		return nullptr;
	}

	return GetDynamicObject(PlayerID);
}
