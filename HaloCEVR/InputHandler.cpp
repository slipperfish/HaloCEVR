#include "InputHandler.h"
#include "Game.h"
#include "Helpers/Controls.h"
#include "Helpers/Camera.h"
#include "Helpers/Menus.h"


#define RegisterBoolInput(x) x = vr->RegisterBoolInput("default", #x);
#define RegisterVector2Input(x) x = vr->RegisterVector2Input("default", #x);
#define ApplyBoolInput(x) controls.##x = vr->GetBoolInput(x) ? 127 : 0;
#define ApplyImpulseBoolInput(x) controls.##x = vr->GetBoolInput(x, bHasChanged) && bHasChanged ? 127 : 0;

void InputHandler::RegisterInputs()
{
	IVR* vr = Game::instance.GetVR();

	RegisterBoolInput(Jump);
	RegisterBoolInput(SwitchGrenades);
	RegisterBoolInput(Interact);
	RegisterBoolInput(SwitchWeapons);
	RegisterBoolInput(Melee);
	RegisterBoolInput(Flashlight);
	RegisterBoolInput(Grenade);
	RegisterBoolInput(Fire);
	RegisterBoolInput(MenuForward);
	RegisterBoolInput(MenuBack);
	RegisterBoolInput(Crouch);
	RegisterBoolInput(Zoom);
	RegisterBoolInput(Reload);

	RegisterVector2Input(Move);
	RegisterVector2Input(Look);

	RegisterBoolInput(Recentre);
}

void InputHandler::UpdateInputs()
{
	IVR* vr = Game::instance.GetVR();

	vr->UpdateInputs();

	static bool bHasChanged = false;

	Controls& controls = Helpers::GetControls();

	ApplyBoolInput(Jump);
	ApplyImpulseBoolInput(SwitchGrenades);
	ApplyBoolInput(Interact);
	ApplyImpulseBoolInput(SwitchWeapons);
	ApplyBoolInput(Melee);
	ApplyBoolInput(Flashlight);
	ApplyBoolInput(Grenade);
	ApplyBoolInput(Fire);
	ApplyBoolInput(MenuForward);
	ApplyBoolInput(MenuBack);
	ApplyBoolInput(Crouch);
	ApplyImpulseBoolInput(Zoom);
	ApplyBoolInput(Reload);

	unsigned char MotionControlFlashlight = UpdateFlashlight();
	if (MotionControlFlashlight > 0)
	{
		controls.Flashlight = MotionControlFlashlight;
	}

	unsigned char MotionControlMelee = UpdateMelee();
	if (MotionControlMelee > 0)
	{
		controls.Melee = MotionControlMelee;
	}

	if (vr->GetBoolInput(Recentre))
	{
		Game::instance.bNeedsRecentre = true;
	}

	bool bMenuChanged;
	bool bMenuPressed = vr->GetBoolInput(MenuBack, bMenuChanged);

	// Opening the menu is seemingly hardcoded to the escape key
	if (bMenuChanged)
	{
		INPUT input{};
		input.type = INPUT_KEYBOARD;
		input.ki.dwFlags = KEYEVENTF_SCANCODE; // DirectInput only detects scancodes
		input.ki.wScan = 01; // Escape
		if (!bMenuPressed)
		{
			input.ki.dwFlags |= KEYEVENTF_KEYUP;
		}
		SendInput(1, &input, sizeof(INPUT));
	}

	Vector2 MoveInput = vr->GetVector2Input(Move);

	controls.Left = -MoveInput.x;
	controls.Forwards = MoveInput.y;
}

void InputHandler::UpdateCamera(float& yaw, float& pitch)
{
	IVR* vr = Game::instance.GetVR();

	Vector2 lookInput = vr->GetVector2Input(Look);

	float yawOffset = vr->GetYawOffset();

	if (Game::instance.c_SnapTurn->Value())
	{
		if (lastSnapState == 1)
		{
			if (lookInput.x < 0.4f)
			{
				lastSnapState = 0;
			}
		}
		else if (lastSnapState == -1)
		{
			if (lookInput.x > -0.4f)
			{
				lastSnapState = 0;
			}
		}
		else
		{
			if (lookInput.x > 0.6f)
			{
				lastSnapState = 1;
				yawOffset += Game::instance.c_SnapTurnAmount->Value();
			}
			else if (lookInput.x < -0.6f)
			{
				lastSnapState = -1;
				yawOffset -= Game::instance.c_SnapTurnAmount->Value();
			}
		}
	}
	else
	{
		yawOffset += lookInput.x * Game::instance.c_SmoothTurnAmount->Value() * Game::instance.lastDeltaTime;
	}

	vr->SetYawOffset(yawOffset);

	Vector3 lookHMD = vr->GetHMDTransform().getLeftAxis();
	// Get current camera angle
	Vector3 lookGame = Helpers::GetCamera().lookDir;
	// Apply deltas
	float yawHMD = atan2(lookHMD.y, lookHMD.x);
	float yawGame = atan2(lookGame.y, lookGame.x);
	yaw = (yawHMD - yawGame);

	float pitchHMD = atan2(lookHMD.z, sqrt(lookHMD.x * lookHMD.x + lookHMD.y * lookHMD.y));
	float pitchGame = atan2(lookGame.z, sqrt(lookGame.x * lookGame.x + lookGame.y * lookGame.y));
	pitch = (pitchHMD - pitchGame);
}

unsigned char InputHandler::UpdateFlashlight()
{
	IVR* vr = Game::instance.GetVR();

	Vector3 headPos = vr->GetHMDTransform() * Vector3(0.0f, 0.0f, 0.0f);

	float leftDistance = Game::instance.c_LeftHandFlashlightDistance->Value();
	float rightDistance = Game::instance.c_RightHandFlashlightDistance->Value();

	if (leftDistance > 0.0f)
	{
		Vector3 handPos = vr->GetControllerTransform(ControllerRole::Left) * Vector3(0.0f, 0.0f, 0.0f);

		if ((headPos - handPos).lengthSqr() < leftDistance * leftDistance)
		{
			return 127;
		}
	}

	if (rightDistance > 0.0f)
	{
		Vector3 handPos = vr->GetControllerTransform(ControllerRole::Right) * Vector3(0.0f, 0.0f, 0.0f);

		if ((headPos - handPos).lengthSqr() < rightDistance * rightDistance)
		{
			return 127;
		}
	}

	return 0;
}

unsigned char InputHandler::UpdateMelee()
{
	IVR* vr = Game::instance.GetVR();

	Vector3 handVel = vr->GetControllerVelocity(ControllerRole::Right);

	handVel *= Game::WorldToMetres(1.0f);

	if (abs(handVel.z) > Game::instance.c_MeleeSwingSpeed->Value())
	{
		return 127;
	}


    return 0;
}

void InputHandler::SetMousePosition(int& x, int& y)
{
	Vector2 mousePos = Game::instance.GetVR()->GetMousePos();
	// Best I can tell the mouse is always scaled to a 640x480 canvas
	x = static_cast<int>(mousePos.x * 640);
	y = static_cast<int>(mousePos.y * 480);
}

void InputHandler::UpdateMouseInfo(MouseInfo* mouseInfo)
{
	if (Game::instance.GetVR()->GetMouseDown())
	{
		if (mouseDownState < 255)
		{
			mouseDownState++;
		}
	}
	else
	{
		mouseDownState = 0;
	}

	mouseInfo->buttonState[0] = mouseDownState;
}

#undef ApplyBoolInput
#undef ApplyImpulseBoolInput
#undef RegisterBoolInput
#undef RegisterVector2Input