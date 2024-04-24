#pragma once
#include <string>
#include <wtypes.h>
#include <Psapi.h>

struct Offset
{
	// Name (used for debugging only)
	std::string DebugName;
	// Offset of the function in memory
	long long Offset;
	// Hex Signature of the function (?/?? signifies an unknown value)
	std::string Signature;
	// Offset of the start of the signature from the function entry point
	long long SignatureOffset = 0;
	// The actual address, including the module offset
	long long Address;
};

#define OFFSET(Name, KnownOffset, Signature) Offset Name = {#Name, KnownOffset, Signature}

class Offsets
{
public:
	OFFSET(TabOutVideo,           0x0c7b74, "38 1D ?? ?? ?? ?? 74 0E 83 FD 01 74 09 83 FD 02 0F 85");
	OFFSET(TabOutVideo2,          0x0c801c, "75 11 38 1D ?? ?? ?? ?? 75 04 3A C3 74 05 C6 44 24 17 01");
	OFFSET(TabOutVideo3,          0x0c829c, "38 1D ?? ?? ?? ?? 74 29 0F BF 05 ?? ?? ?? ?? 3B C3 7E 0E");
	OFFSET(CutsceneFPSCap,        0x0c9fb5, "b3 01 eb ?? 32 db 8b 2d ?? ?? ?? ?? 8d 4c");

	OFFSET(InitDirectX,           0x1169c0, "83 ec 60 8b 15 ?? ?? ?? ?? 8b 0d ?? ?? ?? ?? 53 33 db");
	OFFSET(DrawFrame,             0x10bea0, "83 ec 14 8b 15 ?? ?? ?? ?? 8b 44 24 24 8b 4c 24 28 42");
	OFFSET(DrawHUD,               0x094730, "83 ec 40 66 83 3d ?? ?? ?? ?? ff ?? ?? ?? ?? ?? ?? 8d 04 24");
	OFFSET(DrawMenu,              0x0984c0, "33 c9 83 ec 14 66 3d ff ff 0f 94 c1 53 33 db 49 23 c8");
	OFFSET(DrawScope,             0x12d8a0, "8b 44 24 04 83 ec 4c e8 ?? ?? ?? ?? 8b c8 85 c9");
	OFFSET(DrawLoadingScreen,     0x10bdc0, "55 8b ec 83 e4 f8 81 ec 5c 02 00 00 53 56 8b 75 08");
	OFFSET(DrawFrameMPLoading,    0x10c590, "8b 15 ?? ?? ?? ?? 81 ec 70 02 00 00 53 56 42 57");

	OFFSET(SetViewModelPosition,  0x0d6880, "81 ec f0 00 00 00 53 55 25 ff ff 00 00 56 8b f1 8b 0d 14");
	OFFSET(HandleInputs,          0x08b4b0, "83 ec 08 56 57 8d 44 24 08 50 ff 15 ?? ?? ?? ??");
	OFFSET(UpdatePitchYaw,        0x072160, "81 ec a4 00 00 00 8b 15 ?? ?? ?? ?? 53 0f bf c8");
	
	OFFSET(SetViewportSize,       0x0c8da0, "83 ec 10 53 55 56 57 8b f8 33 c0 83 ff 01 0f 9e c0");
	OFFSET(SetViewportScale,      0x10ca90, "51 0f bf 50 2e 56 0f bf 70 30 57 0f bf 78 2c 2b f7");
};

class SigScanner
{
public:

	// Try to update the offset based on its signature
	// Returns -1 if unsuccessful, 0 if offset already matched, NewOffset if updated
	static long long UpdateOffset(struct Offset& Offset);

	static inline std::string ModuleName = "halo.exe";
};

#undef OFFSET