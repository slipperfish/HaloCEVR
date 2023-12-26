#include "Camera.h"
#include "Player.h"

Camera& Helpers::GetCamera()
{
	return Helpers::GetPlayer().camera;
}
