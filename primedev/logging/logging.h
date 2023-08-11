#pragma once
#include "spdlog/spdlog.h"
#include "spdlog/sinks/base_sink.h"
#include "spdlog/logger.h"

#ifdef NORTHSTAR
#include "core/math/color.h"
#endif

//-----------------------------------------------------------------------------
// Log context
enum class eLogLevel : int
{
	LOG_INFO = 0,
	LOG_WARN = 1,
	LOG_ERROR = 2
};

//-----------------------------------------------------------------------------
// Log context
enum class eLog : int
{
	//-----------------------------------------------------------------------------
	// Common
	//-----------------------------------------------------------------------------
	NONE = 0,
	NS = 1,

	//-----------------------------------------------------------------------------
	// Script
	//-----------------------------------------------------------------------------
	SCRIPT_SERVER = 2,
	SCRIPT_CLIENT = 3,
	SCRIPT_UI = 4,

	//-----------------------------------------------------------------------------
	// Native
	//-----------------------------------------------------------------------------
	SERVER = 5,
	CLIENT = 6,
	UI = 7,
	ENGINE = 8,
	RTECH = 9,
	FS = 10,
	MAT = 11,
	AUDIO = 12,
	VIDEO = 13,

	//-----------------------------------------------------------------------------
	// Custom systems
	//-----------------------------------------------------------------------------
	MS = 14, // Masterserver
	MODSYS = 15, // Mod system
	PLUGSYS = 16, // Plugin system
	PLUGIN = 17,

	//-----------------------------------------------------------------------------
	// Misc
	//-----------------------------------------------------------------------------
	CHAT = 18, // Chat

	LAUNCH = 19,

	// Used for static_assert
	SIZE = 20
};

// clang-format off
constexpr const char* sLogString[] =
{
	"",
	"NORTHSTAR",
	"SCRIPT SV",
	"SCRIPT CL",
	"SCRIPT UI",
	"NATIVE SV",
	"NATIVE CL",
	"NATIVE UI",
	"NATIVE EN",
	"RTECH SYS",
	"FILESYSTM",
	"MATERIAL ",
	"AUDIO SYS",
	"VIDEO SYS",
	"MASTERSVR",
	"MODSYSTEM",
	"PLUGSYSTM",
	"PLUGIN", // Swapped with plugin name
	"CHATSYSTM",
#ifdef WSOCKPROXY // Should not be used by dll so this should be fine
	"WSCKPROXY"
#else
	"LAUNCHER "
#endif
};
// Make sure eLog and sLogString are of the same length
static_assert(sizeof(sLogString)==static_cast<int>(eLog::SIZE)*8);

// Used to send logs to launcher which then saves to disk / prints to winconsole
typedef void(*LogMsgFn)(eLogLevel eLevel, const char* pszMessage, int nCode);

#ifdef NORTHSTAR
inline LogMsgFn g_pLogMsg = nullptr;
#endif

// Directory we put log files in ( %profile%\\logs\\%timestamp%\\ )
inline std::string g_svLogDirectory;

#if defined(LAUNCHER) || defined(WSOCKPROXY)

// Windows terminal logger
inline std::shared_ptr<spdlog::logger> g_WinLogger;

// Log file settings
constexpr int SPDLOG_MAX_LOG_SIZE = 10 * 1024 * 1024; // 10 MB max
constexpr int SPDLOG_MAX_FILES = 512;

//-----------------------------------------------------------------------------
void SpdLog_PreInit(void);
void SpdLog_Init(void);
void SpdLog_CreateLoggers(void);
void SpdLog_Shutdown(void);
#endif

//
// TODO [Fifty]: Print hardware and os info


