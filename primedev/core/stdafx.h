#pragma once

#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#define RAPIDJSON_NOMEMBERITERATORCLASS // need this for rapidjson
#define NOMINMAX // this too
#define _WINSOCK_DEPRECATED_NO_WARNINGS // temp because i'm very lazy and want to use inet_addr, remove later
#define RAPIDJSON_HAS_STDSTRING 1

#ifdef NORTHSTAR
#include "core/memalloc.h"
#endif

#include <windows.h>
#include <psapi.h>
#include <set>
#include <map>
#include <filesystem>
#include <sstream>

#define EXPORT extern "C" __declspec(dllexport)
#define ARRAY_SIZE(arr) ((sizeof(arr) / sizeof(*arr)))

// Get around version gate
#define NORTHSTAR_USERAGENT "R2Northstar/0.0.0+dev"

#ifdef NORTHSTAR
inline std::string g_svProfileDir;

typedef void (*callable)();
typedef void (*callable_v)(void* v);

// clang-format off
#define assert_msg(exp, msg) assert((exp, msg))
//clang-format on
#endif

#include "util/filesystem.h"

#ifdef NORTHSTAR
#include "core/macros.h"
#include "core/structs.h"

#include "tier0/platform.h"
#include "tier0/threadtools.h"
#endif

#include "mathlib/color.h"

#include "logging/logging.h"

#include "tier0/commandline.h"
#include "tier0/dbg.h"

//#include "rapidjson/rapidjson.h"

#if defined(LAUNCHER) || defined(WSOCKPROXY)
#include "spdlog/spdlog.h"
#endif

#ifdef NORTHSTAR
#include "MinHook.h"
#include "curl/curl.h"

#include "silver-bun/module.h"
#include "silver-bun/memaddr.h"

#include "core/hooks.h"
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
inline void LogPtrAdr(const char* szName, void* pVar)
{
    LPCSTR pVarPtr = static_cast<LPCSTR>(pVar);
    HMODULE hPtrModule;
    if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, pVarPtr, &hPtrModule))
    {
        Error(eLog::NONE, NO_ERROR, "Failed to get module for var: '%s'\n", szName);
        return;
    }

    // Get module filename
    CHAR szModulePath[MAX_PATH];
    GetModuleFileNameExA(GetCurrentProcess(), hPtrModule, szModulePath, sizeof(szModulePath));

    const CHAR* pszModuleFileName = strrchr(szModulePath, '\\') + 1;

    // Get relative address
    LPCSTR pRelativeAddress = reinterpret_cast<LPCSTR>(pVarPtr - reinterpret_cast<LPCSTR>(hPtrModule));

    DevMsg(eLog::NONE, "Ptr '%s' is at: %s + %#x\n", szName, pszModuleFileName, (void*)pRelativeAddress);
}

//-----------------------------------------------------------------------------
// 
#if defined(LAUNCHER) || defined(WSOCKPROXY)
static const char* const NSP_EMBLEM[] = {
	R"(+---------------------------------------------------------------+)",
	R"(|  _  _         _   _       _              ___     _            |)",
	R"(| | \| |___ _ _| |_| |_  __| |_ __ _ _ _  | _ \_ _(_)_ __  ___  |)",
	R"(| | .` / _ \ '_|  _| ' \(_-<  _/ _` | '_| |  _/ '_| | '  \/ -_) |)",
	R"(| |_|\_\___/_|  \__|_||_/__/\__\__,_|_|   |_| |_| |_|_|_|_\___| |)",
	R"(|                                                               |)",
	R"(+---------------------------------------------------------------+)"
};
#endif
