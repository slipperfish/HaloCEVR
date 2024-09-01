#include "CmdLineArgs.h"
#include "../Hooking/Hooks.h"

CmdLineArgs& Helpers::GetCmdLineArgs()
{
	return *reinterpret_cast<CmdLineArgs*>(Hooks::o.CmdLineArgs);
}