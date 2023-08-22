#include "engine/server/server.h"


ON_DLL_LOAD("engine.dll", EngineServer, (CModule module))
{
	g_pServer = module.Offset(0x12A53D40).RCast<CServer*>();
}
