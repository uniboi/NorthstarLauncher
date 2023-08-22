#include "engine/vengineserver_impl.h"

ON_DLL_LOAD("engine.dll", EngineServerInterface, (CModule module))
{
	g_pEngineServer = Sys_GetFactoryPtr("engine.dll", "VEngineServer022").RCast<CVEngineServer*>();
}
