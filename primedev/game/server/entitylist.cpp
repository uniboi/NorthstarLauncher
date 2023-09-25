#include "game/server/entitylist.h"

ON_DLL_LOAD("server.dll", ServerEntityList, (CModule module))
{
	gEntList = module.Offset(0x112D770).RCast<void*>();
	CGlobalEntityList__FirstEnt = module.Offset(0x452F00).RCast<CBaseEntity* (*)(void*, void*)>();
}
