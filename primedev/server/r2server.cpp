#include "r2server.h"

ON_DLL_LOAD("server.dll", R2GameServer, (CModule module))
{
	Server_GetEntityByIndex = module.Offset(0xFB820).RCast<CBaseEntity* (*)(int)>();
	UTIL_PlayerByIndex = module.Offset(0x26AA10).RCast<CBasePlayer*(__fastcall*)(int)>();
	CBasePlayer__RunNullCommand = module.Offset(0x5A9FD0).RCast<void (*__fastcall)(CBasePlayer*)>();
}
