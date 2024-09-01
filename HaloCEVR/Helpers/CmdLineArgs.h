#pragma once

struct CmdLineArgs
{
	int NoSound;
	int NoVideo;
	int NoNetwork;
	int Width640;
	int SafeMode;
	int NoWinKey;
};

namespace Helpers
{
	CmdLineArgs& GetCmdLineArgs();
}