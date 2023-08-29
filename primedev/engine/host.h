#pragma once

struct EngineParms_t
{
	char* szBaseDirectory;
	char* szModName;
	char* szRootGameName;
	unsigned int nMemSizeAvailable;
};

inline EngineParms_t* g_pEngineParms;
