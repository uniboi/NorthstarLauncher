#include "init.h"

#include "logging/logging.h"
#include "core/memalloc.h"
#include "config/profile.h"
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

#include "tier0/memstd.h"

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

	g_svProfileDir = pszProfile;
	g_pLogMsg = pLogMsg;

	Tier0_Init();

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

//-----------------------------------------------------------------------------
// Purpose: Returns the version of this dll
//-----------------------------------------------------------------------------
const char* NorthstarPrime_GetVersion()
{
	return NORTHSTAR_VERSION;
}

//-----------------------------------------------------------------------------
// Purpose: Init tier0 exports we use before we can index the whole dll
//-----------------------------------------------------------------------------
void Tier0_Init()
{
	HMODULE hTier0 = GetModuleHandleA("tier0.dll");

	if (!hTier0)
	{
		Error(eLog::NS, EXIT_FAILURE, "tier0.dll not loaded while 'Tier0_Init' is called!\n");
	}

	CModule mTier0(hTier0);

	// commandline
	CommandLine = mTier0.GetExport("CommandLine").RCast<CCommandLine* (*)()>();

	// platform
	Plat_FloatTime = mTier0.GetExport("Plat_FloatTime").RCast<float (*)()>();

	// threadhtools
	ThreadCouldDoServerWork = mTier0.GetExport("ThreadCouldDoServerWork").RCast<bool (*)()>();
	ThreadInMainOrServerFrameThread = mTier0.GetExport("ThreadInMainOrServerFrameThread").RCast<bool (*)()>();
	ThreadInMainThread = mTier0.GetExport("ThreadInMainThread").RCast<bool (*)()>();
	ThreadInServerFrameThread = mTier0.GetExport("ThreadInServerFrameThread").RCast<bool (*)()>();

	// memstd
	// initilased in memstd.h
}
