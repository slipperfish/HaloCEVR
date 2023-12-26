#include "Game.h"
#include "Logger.h"
#include "Hooking/Hooks.h"
#include "Helpers/DX9.h"
#include "Helpers/RenderTarget.h"
#include "Helpers/Renderer.h"

void Game::Init()
{
	Logger::log << "HaloCEVR initialising..." << std::endl;

	SetupConfigs();

	CreateConsole();

	PatchGame();

	Logger::log << "HaloCEVR initialised" << std::endl;
}

void Game::Shutdown()
{
	Logger::log << "HaloCEVR shutting down..." << std::endl;

	MH_STATUS HookStatus = MH_RemoveHook(MH_ALL_HOOKS);

	if (HookStatus != MH_OK)
	{
		Logger::log << "Could not remove hooks: " << MH_StatusToString(HookStatus) << std::endl;
	}

	HookStatus = MH_Uninitialize();

	if (HookStatus != MH_OK)
	{
		Logger::log << "Could not uninitialise MinHook: " << MH_StatusToString(HookStatus) << std::endl;
	}

	if (c_ShowConsole && c_ShowConsole->Value())
	{
		if (ConsoleOut)
		{
			fclose(ConsoleOut);
		}
		FreeConsole();
	}
}

void Game::OnInitDirectX()
{
	Logger::log << "DirectX Created" << std::endl;

	if (!Helpers::GetDirect3DDevice9())
	{
		Logger::log << "Couldn't get game's direct3d device" << std::endl;
		return;
	}

	
	vrEmulator.Init();
}

void Game::PreDrawFrame(struct Renderer* renderer, float deltaTime)
{
	CalcFPS(deltaTime);
	
	// WaitGetPoses will go here most likely

	StoreRenderTargets();

	sRect* Window = reinterpret_cast<sRect*>(0x69c634);
	Window->top = 0;
	Window->left = 0;
	Window->right = 600;
	Window->bottom = 600;

	(*reinterpret_cast<short*>(0x69c642)) = 600 - 8;
	(*reinterpret_cast<short*>(0x69c640)) = 600 - 8;

	vrEmulator.PreDrawFrame(renderer, deltaTime);
}

void Game::PreDrawEye(Renderer* renderer, float deltaTime, int eye)
{
	vrEmulator.PreDrawEye(renderer, deltaTime, eye);
}


void Game::PostDrawEye(struct Renderer* renderer, float deltaTime, int eye)
{
	// TODO: Remove this, if not used. Can do post draw stuff at end of frame
	//vrEmulator.PostDrawEye(renderer, deltaTime, eye);
}

void Game::PreDrawMirror(struct Renderer* renderer, float deltaTime)
{
	RestoreRenderTargets();
}

void Game::PostDrawMirror(struct Renderer* renderer, float deltaTime)
{
	// Do something here to copy the image into the backbuffer correctly
}

void Game::PostDrawFrame(struct Renderer* renderer, float deltaTime)
{
	vrEmulator.PostDrawFrame(renderer, deltaTime);
}

void Game::CreateConsole()
{
	if (!c_ShowConsole || !c_ShowConsole->Value())
	{
		return;
	}

	AllocConsole();
	freopen_s(&ConsoleOut, "CONOUT$", "w", stdout);
	std::cout.clear();
}

void Game::PatchGame()
{
	MH_STATUS HookStatus;

	if ((HookStatus = MH_Initialize()) != MH_OK)
	{
		Logger::log << "Could not initialise MinHook: " << MH_StatusToString(HookStatus) << std::endl;
	}
	else
	{
		Logger::log << "Creating hooks" << std::endl;
		Hooks::InitHooks();
		Logger::log << "Enabling hooks" << std::endl;
		Hooks::EnableAllHooks();
	}
}

void Game::SetupConfigs()
{
	Config config;

	// Put all mod configs here
	c_ShowConsole = config.RegisterBool("ShowConsole", "Create a console window at launch for debugging purposes", false);
	c_DrawMirror = config.RegisterBool("DrawMirror", "Update the desktop window display to show the current game view, rather than leaving it on the splash screen", true);


	config.LoadFromFile("config.txt");
	config.SaveToFile("config.txt");

	Logger::log << "Loaded configs" << std::endl;
}

void Game::CalcFPS(float deltaTime)
{
	FramesSinceFPSUpdate++;
	TimeSinceFPSUpdate += deltaTime;

	if (TimeSinceFPSUpdate > 1.0f)
	{
		TimeSinceFPSUpdate = 0.0f;
		FPS = FramesSinceFPSUpdate;
		FramesSinceFPSUpdate = 0;
		Logger::log << FPS << std::endl;
	}
}

void Game::StoreRenderTargets()
{
	for (int i = 0; i < 8; i++)
	{
		gameRenderTargets[i].width = Helpers::GetRenderTargets()[i].width;
		gameRenderTargets[i].height = Helpers::GetRenderTargets()[i].height;
		gameRenderTargets[i].format = Helpers::GetRenderTargets()[i].format;
		gameRenderTargets[i].renderSurface = Helpers::GetRenderTargets()[i].renderSurface;
		gameRenderTargets[i].renderTexture = Helpers::GetRenderTargets()[i].renderTexture;
	}
}

void Game::RestoreRenderTargets()
{
	for (int i = 0; i < 8; i++)
	{
		Helpers::GetRenderTargets()[i].width = gameRenderTargets[i].width;
		Helpers::GetRenderTargets()[i].height = gameRenderTargets[i].height;
		Helpers::GetRenderTargets()[i].format = gameRenderTargets[i].format;
		Helpers::GetRenderTargets()[i].renderSurface = gameRenderTargets[i].renderSurface;
		Helpers::GetRenderTargets()[i].renderTexture = gameRenderTargets[i].renderTexture;
	}
}
