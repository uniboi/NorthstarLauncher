#include "game/server/baseentity.h"

ON_DLL_LOAD("server.dll", GameServerBaseEntity, (CModule module))
{
	g_pCPlayer_VFTable = module.Offset(0x9524F8).RCast<void*>();
}
