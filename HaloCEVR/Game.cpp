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

	//*
	sRect* Window = reinterpret_cast<sRect*>(0x69c634);
	Window->top = 0;
	Window->left = 0;
	Window->right = 600;
	Window->bottom = 600;

	(*reinterpret_cast<short*>(0x69c642)) = 600 - 8;
	(*reinterpret_cast<short*>(0x69c640)) = 600 - 8;
	//*/

	D3DPRESENT_PARAMETERS* present = reinterpret_cast<D3DPRESENT_PARAMETERS*>(0x7c04a0);
	present->BackBufferWidth = 600;
	present->BackBufferHeight = 600;
	present->PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	Helpers::GetDirect3DDevice9()->Reset(present);
	
	vrEmulator.Init();
}

void Game::PreDrawFrame(struct Renderer* renderer, float deltaTime)
{
	CalcFPS(deltaTime);
	
	// WaitGetPoses will go here most likely

	StoreRenderTargets();

	IDirect3DSurface9* BackBuffer;

	Helpers::GetDirect3DDevice9()->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &BackBuffer);

	D3DSURFACE_DESC Desc;

	BackBuffer->GetDesc(&Desc);

	Logger::log << Desc.Width << ", " << Desc.Height << std::endl;

	BackBuffer->Release();

	//*
	sRect* Window = reinterpret_cast<sRect*>(0x69c634);
	Window->top = 0;
	Window->left = 0;
	Window->right = 600;
	Window->bottom = 600;

	(*reinterpret_cast<short*>(0x69c642)) = 600 - 8;
	(*reinterpret_cast<short*>(0x69c640)) = 600 - 8;
	//*/

	vrEmulator.PreDrawFrame(renderer, deltaTime);
	
	/*
	D3DSURFACE_DESC desc;

	Logger::log << "Pre frame" << std::endl;
	for (int i = 0; i < 8; i++)
	{
		Logger::log << i << ": ";
		Logger::log << Helpers::GetRenderTargets()[i].width;
		Logger::log << "x";
		Logger::log << Helpers::GetRenderTargets()[i].height;
		Logger::log << " vs ";
		Helpers::GetRenderTargets()[i].renderSurface->GetDesc(&desc);
		Logger::log << desc.Width;
		Logger::log << "x";
		Logger::log << desc.Height;
		Logger::log << std::endl;
	}
	*/
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

	D3DSURFACE_DESC Desc;
	gameRenderTargets[0].renderSurface->GetDesc(&Desc);

	//for (int i = 0; i < 8; i++)
	//{
	//	Logger::log << i << ": " << primaryRenderTarget[i].width << "x" << primaryRenderTarget[i].height << std::endl;
	//}


	/*
	sRect* Window = reinterpret_cast<sRect*>(0x69c634);
	Window->top = 0;
	Window->left = 0;
	Window->right = Desc.Width;
	Window->bottom = Desc.Height;

	(*reinterpret_cast<short*>(0x69c642)) = Desc.Width - 8;
	(*reinterpret_cast<short*>(0x69c640)) = Desc.Height - 8;
	*/

	for (CameraFrustum* f : { &renderer->frustum, &renderer->frustum2 })
	{
		f->Viewport.right = Desc.Width;
		f->Viewport.bottom = Desc.Height;
		f->oViewport.right = Desc.Width;
		f->oViewport.bottom = Desc.Height;
	}
}

void Game::PostDrawMirror(struct Renderer* renderer, float deltaTime)
{

}

void Game::PostDrawFrame(struct Renderer* renderer, float deltaTime)
{
	// Copy VR View back onto real screen (broken)

	/*
	IDirect3DSurface9* GameSurface = nullptr;

	HRESULT result = Helpers::GetDirect3DDevice9()->GetRenderTarget(0, &GameSurface);

	if (FAILED(result))
	{
		Logger::log << "Failed to get game surface: " << result << std::endl;
	}

	result = Helpers::GetDirect3DDevice9()->BeginScene();
	if (FAILED(result))
	{
		Logger::log << "Failed to call begin scene: " << result << std::endl;
	}
	result = Helpers::GetDirect3DDevice9()->StretchRect(SharedTarget, nullptr, GameSurface, nullptr, D3DTEXF_NONE);
	if (FAILED(result))
	{
		Logger::log << "Failed to copy to shared target: " << result << std::endl;
	}
	result = Helpers::GetDirect3DDevice9()->EndScene();
	if (FAILED(result))
	{
		Logger::log << "Failed to call end scene: " << result << std::endl;
	}
	GameSurface->Release();
	*/

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
