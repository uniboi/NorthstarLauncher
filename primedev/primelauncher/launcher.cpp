#include "launcher.h"

#include <Windows.h>
#include <Shlwapi.h>
#include "util/filesystem.h"
#include "util/utils.h"
#include "tier0/commandline.h"


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
		// TODO [Fifty]: Use Error
		MessageBoxA(NULL, "Titanfall2.exe not found, make sure you installed northstar correctly.", "Northstar Prime error", MB_ICONERROR);
		TerminateProcess(GetCurrentProcess(), -1);
	}

	// TODO: Verify install dir

	// Verify profile
	if (m_svProfile.empty() || !CreateDirectories(m_svProfile))
	{
		// TODO [Fifty]: Use Error
		std::string svErrorMsg = FormatA("Profile: '%s' is invalid. Make sure you used valid characters!", m_svProfile.c_str());
		MessageBoxA(NULL, svErrorMsg.c_str(), "Northstar Prime error", MB_ICONERROR);
		TerminateProcess(GetCurrentProcess(), -1);
	}

	// Load tier0.dll
	std::wstring wsvTier0Path = FormatW(L"%s\\bin\\x64_retail\\%s", m_wsvExePath.c_str(), L"tier0.dll");
	HMODULE hTier0 = LoadLibraryExW(wsvTier0Path.c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);

	if (!hTier0 && FileExists(wsvTier0Path))
	{
		MessageBoxA(NULL, "Failed to load tier0.dll. The file exists. This means you're missing msvc redistributables.\n\nPlease verify your files and try again.", "Northstar Prime error", MB_ICONERROR);
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
	// crashhandler
	// spdlog
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
		// TODO [Fifty]: Use Error
		MessageBoxA(NULL, "Failed to load Northstar.dll.\n\nPlease verify your files and try again.", "Northstar Prime error", MB_ICONERROR);
		TerminateProcess(GetCurrentProcess(), -1);
	}

	bool (*NorthstarPrime_Initilase)();
	NorthstarPrime_Initilase = reinterpret_cast<bool (*)()>(GetProcAddress(hNorthstar, "InitialiseNorthstar"));

	if (!NorthstarPrime_Initilase)
	{
		// TODO [Fifty]: Use Error
		MessageBoxA(
			NULL, "Loaded Northstar.dll doesn't export InitialiseNorthstar!\n\nPlease verify your files and try again.", "Northstar Prime error", MB_ICONERROR);
		TerminateProcess(GetCurrentProcess(), -1);
	}

	NorthstarPrime_Initilase();

#ifdef LAUNCHER
	// Launch the game
	std::wstring wsvLauncher = FormatW(L"%s\\bin\\x64_retail\\%s", m_wsvExePath.c_str(), L"launcher.dll");
	HMODULE hLauncher = LoadLibraryExW(wsvLauncher.c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);

	if (!hLauncher)
	{
		// TODO [Fifty]: Use Error
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
