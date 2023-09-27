#include "init.h"

#include "logging/logging.h"
#include "core/memalloc.h"
#include "networksystem/bcrypt.h"
#include "tier0/taskscheduler.h"
#include "windows/libsys.h"
#include "networksystem/atlas.h"
#include "game/shared/vscript_shared.h"
#include "game/server/ai_helper.h"

#include <string>

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
	// Check args
	if (!pLogMsg)
	{
		Error(eLog::NS, EXIT_FAILURE, "%s: pLogMsg is null!\n", __FUNCTION__);
	}

	if (!pszProfile)
	{
		Error(eLog::NS, EXIT_FAILURE, "%s: pszProfile is null!\n", __FUNCTION__);
	}

	// Init
	static bool bInitialised = false;
	if (bInitialised)
		return false;

	bInitialised = true;

	g_svProfileDir = pszProfile;
	g_pLogMsg = pLogMsg;

	// Get tier0 exports
	Tier0_Init();

	// Verify bcrypt
	Bcrypt_Init();

	// Init minhook
	HookSys_Init();

	// Init loadlibrary callbacks
	LibSys_Init();

	g_pScriptHttp = new CScriptHttp();

	// Only run atlas client on client
	if (!CommandLine()->CheckParm("-dedicated"))
	{
		g_pAtlasClient = new CAtlasClient();
	}

	g_pAtlasServer = new CAtlasServer();

	g_pAIHelper = new CAI_Helper();

	g_pTaskScheduler = new CTaskScheduler();

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
	CommandLine = mTier0.GetExportedFunction("CommandLine").RCast<CCommandLine* (*)()>();

	// platform
	Plat_FloatTime = mTier0.GetExportedFunction("Plat_FloatTime").RCast<double (*)()>();

	// threadtools
	ThreadCouldDoServerWork = mTier0.GetExportedFunction("ThreadCouldDoServerWork").RCast<bool (*)()>();
	ThreadInMainOrServerFrameThread = mTier0.GetExportedFunction("ThreadInMainOrServerFrameThread").RCast<bool (*)()>();
	ThreadInMainThread = mTier0.GetExportedFunction("ThreadInMainThread").RCast<bool (*)()>();
	ThreadInServerFrameThread = mTier0.GetExportedFunction("ThreadInServerFrameThread").RCast<bool (*)()>();

	// memstd
	// initilased in memstd.h
}

//-----------------------------------------------------------------------------
// Purpose: Init and test bcrypt
//-----------------------------------------------------------------------------
void Bcrypt_Init()
{
	if (!InitHMACSHA256())
		Error(eLog::NS, EXIT_FAILURE, "Failed to initialize bcrypt\n");

	if (!VerifyHMACSHA256("test",
						  "\x88\xcd\x21\x08\xb5\x34\x7d\x97\x3c\xf3\x9c\xdf\x90\x53\xd7\xdd\x42\x70\x48\x76\xd8\xc9\xa9\xbd\x8e\x2d\x16\x82\x59\xd3\xdd"
						  "\xf7",
						  "test"))
		Error(eLog::NS, EXIT_FAILURE, "bcrypt HMAC-SHA256 is broken\n");
}

//-----------------------------------------------------------------------------
// Purpose: Init minhook
//-----------------------------------------------------------------------------
void HookSys_Init()
{
	if (MH_Initialize() != MH_OK)
	{
		Error(eLog::NS, NO_ERROR, "MH_Initialize (minhook initialization) failed\n");
	}
}
