#pragma once
#include "SigScanner.h"
#include "MinHook.h"
#include "../Logger.h"

template <typename T>
class Hook
{
public:

	void CreateHook(Offset& FunctionOffset, LPVOID Func)
	{
		if (!Func)
		{
			Logger::log << "Hook: Target Function pointer not provided to hook, skipping " << typeid(T).name() << std::endl;
			return;
		}

		if (SigScanner::UpdateOffset(FunctionOffset) < 0)
		{
			Logger::log << "Hook: signature was invalid, skipping " << typeid(T).name() << std::endl;
			return;
		}

		MH_STATUS HookStatus = MH_CreateHook(reinterpret_cast<LPVOID>(FunctionOffset.Address), Func, reinterpret_cast<LPVOID*>(&Original));

		if (HookStatus != MH_OK)
		{
			Logger::log << "Hook: Could not create hook for " << typeid(T).name() << " : " << MH_StatusToString(HookStatus) << std::endl;
			return;
		}

		TargetFunc = reinterpret_cast<LPVOID>(FunctionOffset.Address);
		Logger::log << "Hook: Created hook for " << typeid(T).name() << std::endl;
	}

	void EnableHook()
	{
		if (bIsEnabled)
		{
			Logger::log << "Hook: cannot enable " << typeid(T).name() << " as it is already enabled" << std::endl;
			return;
		}

		if (!TargetFunc)
		{
			Logger::log << "Hook: cannot enable " << typeid(T).name() << " as it is has not been successfully created" << std::endl;
			return;
		}

		MH_STATUS HookStatus = MH_EnableHook(TargetFunc);

		if (HookStatus != MH_OK)
		{
			Logger::log << "Hook: Could not enable hook for " << typeid(T).name() << " : " << MH_StatusToString(HookStatus) << std::endl;
			return;
		}

		bIsEnabled = true;
		Logger::log << "Hook: Enabled hook for " << typeid(T).name() << std::endl;
	}

	void DisableHook()
	{
		if (!bIsEnabled)
		{
			Logger::log << "Hook: cannot disable " << typeid(T).name() << " as it is not enabled" << std::endl;
			return;
		}

		if (!TargetFunc)
		{
			Logger::log << "Hook: cannot disable " << typeid(T).name() << " as the current target function is invalid" << std::endl;
			return;
		}

		MH_STATUS HookStatus = MH_DisableHook(TargetFunc);

		if (HookStatus != MH_OK)
		{
			Logger::log << "Hook: Could not enable hook for " << typeid(T).name() << " : " << MH_StatusToString(HookStatus) << std::endl;
		}

		bIsEnabled = false;
		TargetFunc = nullptr;
		Logger::log << "Hook: Disabled hook for " << typeid(T).name() << std::endl;
	}

protected:
	T Original;

private:
	bool bIsEnabled = false;
	LPVOID TargetFunc = 0;

	friend class Hooks;
};
