#include "codecs/miles/core.h"

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void (*o_MilesLog)(int level, const char* string);

void h_MilesLog(int level, const char* string)
{
	DevMsg(eLog::AUDIO, "%i - %s\n", level, string);
}

ON_DLL_LOAD_CLIENT("client.dll", AudioHooks, (CModule module))
{
	o_MilesLog = module.Offset(0x57DAD0).RCast<void (*)(int, const char*)>();
	HookAttach(&(PVOID&)o_MilesLog, (PVOID)h_MilesLog);

	MilesStopAll = module.Offset(0x580850).RCast<void (*)()>();
}
