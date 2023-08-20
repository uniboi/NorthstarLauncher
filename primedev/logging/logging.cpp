#include "logging.h"

#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"

#include <winternl.h>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <shellapi.h>

#include "util/utils.h"
#include "windows/wconsole.h"

//-----------------------------------------------------------------------------
// Purpose: Checks if install folder is writable, exits if it is not
//-----------------------------------------------------------------------------
void SpdLog_PreInit(void)
{
	// NOTE [Fifty]: Instead of checking every reason as to why we might not be able to write to a directory
	//               it is easier to just try to create a file
	FILE* pFile = fopen(".nstemp", "w");
	if (pFile)
	{
		fclose(pFile);
		remove(".nstemp");
		return;
	}

	// Show message box
	int iAction = MessageBoxA(
		NULL,
		"The current directory isn't writable!\n"
		"Please move the game into a writable directory to be able to continue\n\n"
		"Click \"OK\" to open the wiki in your browser",
		"Northstar Prime Error",
		MB_ICONERROR | MB_OKCANCEL);

	// User chose to open the troubleshooting wiki page
	if (iAction == IDOK)
	{
		ShellExecuteA(
			NULL,
			NULL,
			"https://r2northstar.gitbook.io/r2northstar-wiki/installing-northstar/"
			"troubleshooting#cannot-write-log-file-when-using-northstar-on-ea-app",
			NULL,
			NULL,
			SW_NORMAL);
	}

	// Gracefully close
	TerminateProcess(GetCurrentProcess(), -1);
}

//-----------------------------------------------------------------------------
// Purpose: Initilases spdlog + windows console logger
//-----------------------------------------------------------------------------
void SpdLog_Init(void)
{
	g_WinLogger = spdlog::stdout_logger_mt("win_console");
	spdlog::set_level(spdlog::level::trace);

	// NOTE [Fifty]: This may be bad as it writes to disk for every message?
	//               Seems to fix logs not flushing properly, still needs testing
	spdlog::flush_on(spdlog::level::trace);

	if (g_bConsole_UseAnsiColor)
		g_WinLogger->set_pattern("%v\u001b[0m");
	else
		g_WinLogger->set_pattern("%v");
}

//-----------------------------------------------------------------------------
// Purpose: Creates all loggers we use
//-----------------------------------------------------------------------------
void SpdLog_CreateLoggers(void)
{
	spdlog::rotating_logger_mt<spdlog::synchronous_factory>(
		"northstar(info)", fmt::format("{:s}\\{:s}", g_svLogDirectory, "message.txt"), SPDLOG_MAX_LOG_SIZE, SPDLOG_MAX_FILES)
		->set_pattern("[%Y-%m-%d %H:%M:%S.%e] %v");
	spdlog::rotating_logger_mt<spdlog::synchronous_factory>(
		"northstar(warning)", fmt::format("{:s}\\{:s}", g_svLogDirectory, "warning.txt"), SPDLOG_MAX_LOG_SIZE, SPDLOG_MAX_FILES)
		->set_pattern("[%Y-%m-%d %H:%M:%S.%e] %v");
	spdlog::rotating_logger_mt<spdlog::synchronous_factory>(
		"northstar(error)", fmt::format("{:s}\\{:s}", g_svLogDirectory, "error.txt"), SPDLOG_MAX_LOG_SIZE, SPDLOG_MAX_FILES)
		->set_pattern("[%Y-%m-%d %H:%M:%S.%e] %v");
}

//-----------------------------------------------------------------------------
// Purpose: Shutdowns spdlog
//-----------------------------------------------------------------------------
void SpdLog_Shutdown(void)
{
	spdlog::shutdown();
}
