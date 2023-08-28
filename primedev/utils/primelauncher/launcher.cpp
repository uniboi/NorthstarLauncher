#include "launcher.h"

#include <Windows.h>
#include <Shlwapi.h>
#include "spdlog/spdlog.h"
#include "logging/logging.h"
#include "tier0/commandline.h"
#include "tier0/crashhandler.h"
#include "tier0/dbg.h"
#include "windows/wconsole.h"

//-----------------------------------------------------------------------------
// Purpose: Returns the profile to use
//-----------------------------------------------------------------------------
std::string Launcher_GetProfile()
{
	// TODO [Fifty]: Support unicode paths

	std::string svProfile = "R2Northstar";

	// Check if user has specified a profile path
	char* pProfile = strstr(GetCommandLineA(), "-profile=");

	if (pProfile == nullptr)
		return svProfile;

	// Parse the path
	svProfile = std::string(pProfile);

	// profile is not quoted
	if (strncmp(svProfile.substr(9, 1).c_str(), "\"", 1))
	{
		// Find space
		int space = svProfile.find(" ");

		svProfile = svProfile.substr(9, space - 9);
	}
	// profile is quoted
	else
	{
		char* pQuote1 = pProfile + 10;
		char* pQuote2 = strstr(pQuote1, "\"");
		if (pQuote2 != nullptr)
		{
			svProfile = std::string(pQuote1, pQuote2);
		}
		else
		{
			svProfile = "";
		}
	}

	// There is a chance we finish with svProfile being empty or having illegal symbols
	// This is intentional so we can later notify the user about this
	return svProfile;
}

//-----------------------------------------------------------------------------
// Purpose: Returns the path to the exe
//-----------------------------------------------------------------------------
std::wstring Launcher_GetExePath()
{
	// TODO [Fifty]: Add error checking here
	wchar_t wszExePath[MAX_PATH];
	GetModuleFileNameW(NULL, wszExePath, MAX_PATH);
	PathRemoveFileSpecW(wszExePath);

	return std::wstring(wszExePath);
}

//-----------------------------------------------------------------------------
// Purpose: Print emblem
//-----------------------------------------------------------------------------
void Launcher_PrintEmblem()
{
	for (int i = 0; i < ARRAY_SIZE(NSP_EMBLEM); i++)
	{
		DevMsg(eLog::NONE, "%s\n", NSP_EMBLEM[i]);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CNorthstarLauncher::CNorthstarLauncher()
{
	g_svProfileDir = Launcher_GetProfile();
	m_wsvExePath = Launcher_GetExePath();

	SetCurrentDirectoryW(m_wsvExePath.c_str());
}

//-----------------------------------------------------------------------------
// Purpose: If specified in launch arguments waits for a debugger to attach
//-----------------------------------------------------------------------------
void CNorthstarLauncher::WaitForDebugger()
{
	if (!CommandLine()->CheckParm("-waitfordebugger"))
		return;

	while (!IsDebuggerPresent())
	{
		Sleep(100);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Determines whether we should inject northstar into the game
//-----------------------------------------------------------------------------
bool CNorthstarLauncher::ShouldInject()
{
#if defined(LAUNCHER)
	return true;
#elif defined(WSOCKPROXY)
	return CommandLine()->CheckParm("-northstar");
#else
#error "Unknown launcher"
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Initilazes all subsystems we use before LauncherMain is called
//-----------------------------------------------------------------------------
void CNorthstarLauncher::InitCoreSubsystems()
{
	// Verify we were installed correctly
	if (!FileExists("Titanfall2.exe"))
	{
		MessageBoxA(NULL, "Titanfall2.exe not found, make sure you installed northstar correctly.", "Northstar Prime error", MB_ICONERROR);
		TerminateProcess(GetCurrentProcess(), -1);
	}

	// TODO: Verify install dir

	// Verify profile
	if (g_svProfileDir.empty() || !CreateDirectories(g_svProfileDir))
	{
		std::string svErrorMsg = FormatA("Profile: '%s' is invalid. Make sure you used valid characters!", g_svProfileDir.c_str());
		MessageBoxA(NULL, svErrorMsg.c_str(), "Northstar Prime error", MB_ICONERROR);
		TerminateProcess(GetCurrentProcess(), -1);
	}

	// Load tier0.dll
	std::wstring wsvTier0Path = FormatW(L"%s\\bin\\x64_retail\\%s", m_wsvExePath.c_str(), L"tier0.dll");
	HMODULE hTier0 = LoadLibraryExW(wsvTier0Path.c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);

	if (!hTier0 && FileExists(wsvTier0Path))
	{
		MessageBoxA(NULL,
					"Failed to load tier0.dll. The file exists. This means you're missing x64 msvc 2012 redistributables.\n\nPlease verify your "
					"files and try again.",
					"Northstar Prime error", MB_ICONERROR);
		TerminateProcess(GetCurrentProcess(), -1);
	}

	if (!hTier0)
	{
		MessageBoxA(NULL, "Failed to load tier0.dll.\n\nPlease verify your files and try again.", "Northstar Prime error", MB_ICONERROR);
		TerminateProcess(GetCurrentProcess(), -1);
	}

	CommandLine = reinterpret_cast<CCommandLine* (*)()>(GetProcAddress(hTier0, "CommandLine"));

	CommandLine()->CreateCmdLine(GetCommandLineA());
}

//-----------------------------------------------------------------------------
// Purpose: Initilazes all subsystems we use before LauncherMain is called
//-----------------------------------------------------------------------------
void CNorthstarLauncher::InitNorthstarSubsystems()
{
	g_svLogDirectory = fmt::format("{:s}\\logs\\{:s}", g_svProfileDir, CreateTimeStamp());

	// Check if install dir is writable
	SpdLog_PreInit();

	// Init crash handler as soon as possible
	g_pCrashHandler = new CCrashHandler();

	// Create console
	Console_Init();

	// Setup spdlog
	SpdLog_Init();
	SpdLog_CreateLoggers();

	// Print emblem and sys info
	Launcher_PrintEmblem();
}

#ifdef LAUNCHER
//-----------------------------------------------------------------------------
// Purpose: Initilazes origin
//-----------------------------------------------------------------------------
void CNorthstarLauncher::InitOrigin()
{
	// TODO
}
#endif

//-----------------------------------------------------------------------------
// Purpose: Injects Northstar
//-----------------------------------------------------------------------------
void CNorthstarLauncher::InjectNorthstar()
{
	//------------------------------------------------------
	// Load northstar
	//------------------------------------------------------
	std::wstring wsvNorthstar = FormatW(L"%s\\bin\\x64_retail\\%s", m_wsvExePath.c_str(), L"Prime.dll");
	HMODULE hNorthstar = LoadLibraryExW(wsvNorthstar.c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);

	if (!hNorthstar)
	{
		MessageBoxA(NULL, "Failed to load Prime.dll.\n\nPlease verify your files and try again.", "Northstar Prime error", MB_ICONERROR);
		TerminateProcess(GetCurrentProcess(), -1);
	}

	// Get 'NorthstarPrime_Initilase'
	bool (*NorthstarPrime_Initilase)(LogMsgFn pLogMsg, const char* pszProfile);
	NorthstarPrime_Initilase = reinterpret_cast<bool (*)(LogMsgFn, const char*)>(GetProcAddress(hNorthstar, "NorthstarPrime_Initilase"));

	if (!NorthstarPrime_Initilase)
	{
		MessageBoxA(NULL, "Loaded Prime.dll doesn't export NorthstarPrime_Initilase!\n\nPlease verify your files and try again.", "Northstar Prime error", MB_ICONERROR);
		TerminateProcess(GetCurrentProcess(), -1);
	}

	//------------------------------------------------------
	// Print some debug information
	//------------------------------------------------------
	DevMsg(eLog::NONE, "CommandLine      : %s\n", GetCommandLineA());

	DevMsg(eLog::NONE, "Profile          : %s\n", g_svProfileDir.c_str());

	// Get 'NorthstarPrime_GetVersion'
	const char* (*NorthstarPrime_GetVersion)();
	NorthstarPrime_GetVersion = reinterpret_cast<const char* (*)()>(GetProcAddress(hNorthstar, "NorthstarPrime_GetVersion"));

	const char* pszVersion = "Failed to get 'NorthstarPrime_GetVersion'";

	if (NorthstarPrime_GetVersion)
	{
		pszVersion = NorthstarPrime_GetVersion();
	}

	DevMsg(eLog::NONE, "Northstar version: %s\n", pszVersion);

	// Get 'RtlGetVersion' ( e.g. 10.0-19045 )
	HMODULE hNTdll = GetModuleHandleA("ntdll.dll");

	if (!hNTdll)
	{
		MessageBoxA(NULL, "Challenge Complete!\nHow Did We Get Here?", "Northstar Prime error", MB_ICONERROR);
		TerminateProcess(GetCurrentProcess(), -1);
	}

	DWORD(WINAPI * RtlGetVersion)(LPOSVERSIONINFOEXW);
	RtlGetVersion = reinterpret_cast<DWORD(WINAPI*)(LPOSVERSIONINFOEXW)>(GetProcAddress(hNTdll, "RtlGetVersion"));

	if (RtlGetVersion)
	{
		OSVERSIONINFOEXW osvi = {};
		RtlGetVersion(&osvi);

		DevMsg(eLog::NONE, "Windows version  : %d.%d-%d\n", osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber);
	}

	// Get 'wine_get_version' (7.0-rc1, 7.0, 7.11, etc)
	const char*(CDECL * wine_get_version)(void);
	wine_get_version = reinterpret_cast<const char*(CDECL*)(void)>(GetProcAddress(hNTdll, "wine_get_version"));

	if (wine_get_version)
	{
		DevMsg(eLog::NONE, "Wine version     : %s\n", wine_get_version());
	}

	// Get 'wine_get_build_id' (e.g., "wine-7.22 (Staging)")
	const char*(CDECL * wine_get_build_id)(void);
	wine_get_build_id = reinterpret_cast<const char*(CDECL*)(void)>(GetProcAddress(hNTdll, "wine_get_build_id"));

	if (wine_get_build_id)
	{
		DevMsg(eLog::NONE, "Wine build id    : %s\n", wine_get_build_id());
	}

	DevMsg(eLog::NONE, "+---------------------------------------------------------------+\n");

	//------------------------------------------------------
	// Initilase northstar
	//------------------------------------------------------
	NorthstarPrime_Initilase(reinterpret_cast<LogMsgFn>(LogMsg), g_svProfileDir.c_str());

#ifdef LAUNCHER
	//------------------------------------------------------
	// Launch the game
	//------------------------------------------------------
	std::wstring wsvLauncher = FormatW(L"%s\\bin\\x64_retail\\%s", m_wsvExePath.c_str(), L"launcher.dll");
	HMODULE hLauncher = LoadLibraryExW(wsvLauncher.c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);

	if (!hLauncher)
	{
		MessageBoxA(NULL, "Failed to load launcher.dll.\n\nPlease verify your files and try again.", "Northstar Prime error", MB_ICONERROR);
		TerminateProcess(GetCurrentProcess(), -1);
	}

	int (*LauncherMain)(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow);
	LauncherMain = reinterpret_cast<int (*)(HINSTANCE, HINSTANCE, PWSTR, int)>(GetProcAddress(hLauncher, "LauncherMain"));

	LauncherMain(NULL, NULL, NULL, 0);
#endif
}

//-----------------------------------------------------------------------------
CNorthstarLauncher* g_pLauncher = new CNorthstarLauncher();
