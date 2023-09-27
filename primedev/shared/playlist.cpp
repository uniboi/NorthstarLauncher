#include "playlist.h"
#include "tier1/cmd.h"
#include "tier1/convar.h"
#include "engine/edict.h"
#include "engine/hoststate.h"

char (*o_clc_SetPlaylistVarOverride__Process)(void* a1, void* a2);

char h_clc_SetPlaylistVarOverride__Process(void* a1, void* a2)
{
	// the private_match playlist on mp_lobby is the only situation where there should be any legitimate sending of this netmessage
	if (!Cvar_ns_use_clc_SetPlaylistVarOverride->GetBool() || strcmp(GetCurrentPlaylistName(), "private_match") || strcmp(g_pServerGlobalVariables->m_pMapName, "mp_lobby"))
		return 1;

	return o_clc_SetPlaylistVarOverride__Process(a1, a2);
}

bool (*o_SetCurrentPlaylist)(const char* pPlaylistName);

bool SetCurrentPlaylist(const char* pPlaylistName)
{
	bool bSuccess = o_SetCurrentPlaylist(pPlaylistName);

	if (bSuccess)
	{
		DevMsg(eLog::NS, "Set playlist to %s\n", GetCurrentPlaylistName());
	}

	return bSuccess;
}

void (*o_SetPlaylistVarOverride)(const char* pVarName, const char* pValue);

void SetPlaylistVarOverride(const char* pVarName, const char* pValue)
{
	if (strlen(pValue) >= 64)
		return;

	o_SetPlaylistVarOverride(pVarName, pValue);
}

const char* (*o_GetCurrentPlaylistVar)(const char* pVarName, bool bUseOverrides);

const char* GetCurrentPlaylistVar(const char* pVarName, bool bUseOverrides)
{
	if (!bUseOverrides && !strcmp(pVarName, "max_players"))
		bUseOverrides = true;

	return o_GetCurrentPlaylistVar(pVarName, bUseOverrides);
}

int (*o_GetCurrentGamemodeMaxPlayers)();

int h_GetCurrentGamemodeMaxPlayers()
{
	const char* pMaxPlayers = GetCurrentPlaylistVar("max_players", 0);
	if (!pMaxPlayers)
		return o_GetCurrentGamemodeMaxPlayers();

	int iMaxPlayers = atoi(pMaxPlayers);
	return iMaxPlayers;
}

ON_DLL_LOAD("engine.dll", PlaylistHooks, (CModule module))
{
	o_clc_SetPlaylistVarOverride__Process = module.Offset(0x222180).RCast<char (*)(void*, void*)>();
	HookAttach(&(PVOID&)o_clc_SetPlaylistVarOverride__Process, (PVOID)h_clc_SetPlaylistVarOverride__Process);

	o_SetCurrentPlaylist = module.Offset(0x18EB20).RCast<bool (*)(const char*)>();
	HookAttach(&(PVOID&)o_SetCurrentPlaylist, (PVOID)SetCurrentPlaylist);

	o_SetPlaylistVarOverride = module.Offset(0x18ED00).RCast<void (*)(const char*, const char*)>();
	HookAttach(&(PVOID&)o_SetPlaylistVarOverride, (PVOID)SetPlaylistVarOverride);

	o_GetCurrentPlaylistVar = module.Offset(0x18C680).RCast<const char* (*)(const char*, bool)>();
	HookAttach(&(PVOID&)o_GetCurrentPlaylistVar, (PVOID)GetCurrentPlaylistVar);

	o_GetCurrentGamemodeMaxPlayers = module.Offset(0x18C430).RCast<int (*)()>();
	HookAttach(&(PVOID&)o_GetCurrentGamemodeMaxPlayers, (PVOID)h_GetCurrentGamemodeMaxPlayers);

	GetCurrentPlaylistName = module.Offset(0x18C640).RCast<const char* (*)()>();
	// patch to prevent clc_SetPlaylistVarOverride from being able to crash servers if we reach max overrides due to a call to Error (why is
	// this possible respawn, wtf) todo: add a warning for this
	module.Offset(0x18ED8D).Patch("C3");

	// patch to allow setplaylistvaroverride to be called before map init on dedicated and private match launched through the game
	module.Offset(0x18ED17).NOP(6);
}
