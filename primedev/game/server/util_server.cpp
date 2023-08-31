#include "game/server/util_server.h"

ON_DLL_LOAD("server.dll", GameServerUtilServer, (CModule module))
{
	UTIL_PlayerByIndex = module.Offset(0x26AA10).RCast<CPlayer*(__fastcall*)(int)>();
}
