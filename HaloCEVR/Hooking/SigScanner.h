#pragma once
#include <string>
#include <wtypes.h>
#include <Psapi.h>

struct Offset
{
	// Module the function exists in
	std::string Module;
	// Offset of the function in memory
	long long Offset;
	// Hex Signature of the function (?/?? signifies an unknown value)
	std::string Signature;
	// Offset of the start of the signature from the function entry point
	long long SignatureOffset = 0;
	// The actual address, including the module offset
	long long Address;
};

class Offsets
{
public:
	// This sig probably works, but seems a bit iffy
	Offset UpdateCameraRotation =	{ "halo.exe", 0x48900, "83 ec 14 53 0f bf d8" };
	Offset TabOutVideo =			{ "halo.exe", 0xc7b74, "38 1D ?? ?? ?? ?? 74 0E 83 FD 01 74 09 83 FD 02 0F 85" };
	Offset TabOutVideo2 =			{ "halo.exe", 0xc801c, "75 11 38 1D ?? ?? ?? ?? 75 04 3A C3 74 05 C6 44 24 17 01" };
	Offset TabOutVideo3 =			{ "halo.exe", 0xc829c, "38 1D ?? ?? ?? ?? 74 29 0F BF 05 ?? ?? ?? ?? 3B C3 7E 0E" };

	Offset InitDirectX =			{ "halo.exe", 0x1169c0, "83 ec 60 8b 15 ?? ?? ?? ?? 8b 0d ?? ?? ?? ?? 53 33 db"};
	Offset DrawFrame =				{ "halo.exe", 0x10bea0, "83 ec 14 8b 15 ?? ?? ?? ?? 8b 44 24 24 8b 4c 24 28 42" };
	Offset SetViewportSize =		{ "halo.exe", 0xc8da0, "83 ec 10 53 55 56 57 8b f8 33 c0 83 ff 01 0f 9e c0" };

	Offset DrawFrameMPLoading =		{ "halo.exe", 0x10c590, "8b 15 ?? ?? ?? ?? 81 ec 70 02 00 00 53 56 42 57" };
};

class SigScanner
{
public:

	// Try to update the offset based on its signature
	// Returns -1 if unsuccessful, 0 if offset already matched, NewOffset if updated
	static long long UpdateOffset(struct Offset& Offset);
};
