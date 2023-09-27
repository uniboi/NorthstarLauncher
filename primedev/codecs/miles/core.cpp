#include "codecs/miles/core.h"

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void (*o_MilesLog)(int level, const char* pszMsg);

void h_MilesLog(int level, const char* pszMsg)
{
	// TODO [Fifty]: Add a convar to disable this
	eLogLevel eLevel = eLogLevel::LOG_ERROR;

	switch (level)
	{
	case 1:
		eLevel = eLogLevel::LOG_INFO;
		break;
	case 2:
		eLevel = eLogLevel::LOG_WARN;
		break;
	default:
		eLevel = eLogLevel::LOG_ERROR;
	}

	CoreMsg(eLog::AUDIO, eLevel, NO_ERROR, Log_GetContextString(eLog::AUDIO), "%s\n", pszMsg);
}

ON_DLL_LOAD_CLIENT("client.dll", AudioHooks, (CModule module))
{
	o_MilesLog = module.Offset(0x57DAD0).RCast<void (*)(int, const char*)>();
	HookAttach(&(PVOID&)o_MilesLog, (PVOID)h_MilesLog);

	MilesStopAll = module.Offset(0x580850).RCast<void (*)()>();
}
