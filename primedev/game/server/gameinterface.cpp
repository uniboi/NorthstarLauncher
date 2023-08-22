#include "game/server/gameinterface.h"

ON_DLL_LOAD("server.dll", GameInterface, (CModule module))
{
	g_pServerGameClients = Sys_GetFactoryPtr("server.dll", "ServerGameClients004").RCast<CServerGameClients*>();
}
