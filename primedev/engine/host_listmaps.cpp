#include "mods/modmanager.h"

int (*o_Host_Map_f_CompletionFunc)(const char* const cmdname, const char* const partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]);
// clang-format off
int h_Host_Map_f_CompletionFunc(const char *const cmdname, const char *const partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])
// clang-format on
{
	// use a custom autocomplete func for all map loading commands
	const int cmdLength = strlen(cmdname);
	const char* query = partial + cmdLength;
	const int queryLength = strlen(query);

	int numMaps = 0;
	for (int i = 0; i < g_pModManager->m_vMapList.size() && numMaps < COMMAND_COMPLETION_MAXITEMS; i++)
	{
		if (!strncmp(query, g_pModManager->m_vMapList[i].svName.c_str(), queryLength))
		{
			strcpy(commands[numMaps], cmdname);
			strncpy_s(commands[numMaps++] + cmdLength, COMMAND_COMPLETION_ITEM_LENGTH, &g_pModManager->m_vMapList[i].svName[0], COMMAND_COMPLETION_ITEM_LENGTH - cmdLength);
		}
	}

	return numMaps;
}

ON_DLL_LOAD("engine.dll", HostListMaps, (CModule module))
{
	o_Host_Map_f_CompletionFunc = module.Offset(0x161AE0).RCast<int (*)(const char* const, const char* const, char[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])>();
	HookAttach(&(PVOID&)o_Host_Map_f_CompletionFunc, (PVOID)h_Host_Map_f_CompletionFunc);
}
