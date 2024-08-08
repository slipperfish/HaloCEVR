#include "Camera.h"
#include "Player.h"
#include "../Hooking/Hooks.h"

Camera& Helpers::GetCamera()
{
	return Helpers::GetPlayer().camera;
}

InputData& Helpers::GetInputData()
{
	return **reinterpret_cast<InputData**>(Hooks::o.InputData);
}