#include "logging/logging.h"
#include "core/memalloc.h"
#include "config/profile.h"
#include "util/version.h"
#include "squirrel/squirrel.h"
#include "shared/gamepresence.h"
#include "server/serverpresence.h"

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "rapidjson/error/en.h"

#include <string.h>
#include <filesystem>
#include <util/utils.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	return TRUE;
}

//-----------------------------------------------------------------------------
// Purpose: Northstar Entry point
//-----------------------------------------------------------------------------
bool NorthstarPrime_Initilase(LogMsgFn pLogMsg, const char* pszProfile)
{
	// TODO[Fifty]: Check arguments
	static bool bInitialised = false;
	if (bInitialised)
		return false;

	bInitialised = true;

	g_svLogDirectory = pszProfile;
	g_pLogMsg = pLogMsg;

	InitialiseVersion();

	InstallInitialHooks();

	g_pServerPresence = new ServerPresenceManager();

	g_pGameStatePresence = new GameStatePresence();

	InitialiseSquirrelManagers();

	// Fix some users' failure to connect to respawn datacenters
	SetEnvironmentVariableA("OPENSSL_ia32cap", "~0x200000200000000");

	curl_global_init_mem(CURL_GLOBAL_DEFAULT, _malloc_base, _free_base, _realloc_base, _strdup_base, _calloc_base);

	// run callbacks for any libraries that are already loaded by now
	CallAllPendingDLLLoadCallbacks();

	return true;
}
