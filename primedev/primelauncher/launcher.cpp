#include "launcher.h"

#include <Windows.h>
#include <Shlwapi.h>
#include "spdlog/spdlog.h"
#include "util/filesystem.h"
#include "util/utils.h"
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
// Purpose: Print northstar information
//-----------------------------------------------------------------------------
void Launcher_PrintNSPInfo()
{
	DevMsg(eLog::NONE, "Northstar Prime version: %s\n", "0.0-dev");
	DevMsg(eLog::NONE, "CommandLine: %s\n", GetCommandLineA());
	DevMsg(eLog::NONE, "+---------------------------------------------------------------+\n");
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CNorthstarLauncher::CNorthstarLauncher()
{
	m_svProfile = Launcher_GetProfile();
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
	if (m_svProfile.empty() || !CreateDirectories(m_svProfile))
	{
		std::string svErrorMsg = FormatA("Profile: '%s' is invalid. Make sure you used valid characters!", m_svProfile.c_str());
		MessageBoxA(NULL, svErrorMsg.c_str(), "Northstar Prime error", MB_ICONERROR);
		TerminateProcess(GetCurrentProcess(), -1);
	}

	// Load tier0.dll
	std::wstring wsvTier0Path = FormatW(L"%s\\bin\\x64_retail\\%s", m_wsvExePath.c_str(), L"tier0.dll");
	HMODULE hTier0 = LoadLibraryExW(wsvTier0Path.c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);

	if (!hTier0 && FileExists(wsvTier0Path))
	{
		MessageBoxA(NULL, "Failed to load tier0.dll. The file exists. This means you're missing x64 msvc 2012 redistributables.\n\nPlease verify your files and try again.", "Northstar Prime error", MB_ICONERROR);
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
	g_svLogDirectory = fmt::format("{:s}\\logs\\{:s}", m_svProfile, CreateTimeStamp());

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
	Launcher_PrintNSPInfo();
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
	std::wstring wsvNorthstar = FormatW(L"%s\\%s", m_wsvExePath.c_str(), L"Northstar.dll");
	HMODULE hNorthstar = LoadLibraryExW(wsvNorthstar.c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);

	if (!hNorthstar)
	{
		MessageBoxA(NULL, "Failed to load Northstar.dll.\n\nPlease verify your files and try again.", "Northstar Prime error", MB_ICONERROR);
		TerminateProcess(GetCurrentProcess(), -1);
	}

	bool (*NorthstarPrime_Initilase)(LogMsgFn pLogMsg, const char* pszProfile);
	NorthstarPrime_Initilase = reinterpret_cast<bool (*)(LogMsgFn, const char*)>(GetProcAddress(hNorthstar, "NorthstarPrime_Initilase"));

	if (!NorthstarPrime_Initilase)
	{
		MessageBoxA(
			NULL, "Loaded Northstar.dll doesn't export NorthstarPrime_Initilase!\n\nPlease verify your files and try again.", "Northstar Prime error", MB_ICONERROR);
		TerminateProcess(GetCurrentProcess(), -1);
	}

	NorthstarPrime_Initilase(reinterpret_cast<LogMsgFn>(LogMsg), m_svProfile.c_str());

#ifdef LAUNCHER
	// Launch the game
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
