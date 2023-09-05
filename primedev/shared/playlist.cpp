#include "playlist.h"
#include "tier1/cmd.h"
#include "tier1/convar.h"
#include "engine/edict.h"
#include "squirrel/squirrel.h"
#include "engine/hoststate.h"

AUTOHOOK_INIT()

// use the R2 namespace for game funcs
namespace R2
{
	DEFINED_VAR_AT(engine.dll + 0x18C640, GetCurrentPlaylistName);
	DEFINED_VAR_AT(engine.dll + 0x18EB20, SetCurrentPlaylist);
	DEFINED_VAR_AT(engine.dll + 0x18ED00, SetPlaylistVarOverride);
	DEFINED_VAR_AT(engine.dll + 0x18C680, GetCurrentPlaylistVar);
} // namespace R2

// clang-format off
AUTOHOOK(clc_SetPlaylistVarOverride__Process, engine.dll + 0x222180,
char, __fastcall, (void* a1, void* a2))
// clang-format on
{
	// the private_match playlist on mp_lobby is the only situation where there should be any legitimate sending of this netmessage
	if (!Cvar_ns_use_clc_SetPlaylistVarOverride->GetBool() || strcmp(R2::GetCurrentPlaylistName(), "private_match") || strcmp(g_pServerGlobalVariables->m_pMapName, "mp_lobby"))
		return 1;

	return clc_SetPlaylistVarOverride__Process(a1, a2);
}

// clang-format off
AUTOHOOK(SetCurrentPlaylist, engine.dll + 0x18EB20,
bool, __fastcall, (const char* pPlaylistName))
// clang-format on
{
	bool bSuccess = SetCurrentPlaylist(pPlaylistName);

	if (bSuccess)
	{
		DevMsg(eLog::NS, "Set playlist to %s\n", R2::GetCurrentPlaylistName());
	}

	return bSuccess;
}

// clang-format off
AUTOHOOK(SetPlaylistVarOverride, engine.dll + 0x18ED00,
void, __fastcall, (const char* pVarName, const char* pValue))
// clang-format on
{
	if (strlen(pValue) >= 64)
		return;

	SetPlaylistVarOverride(pVarName, pValue);
}

// clang-format off
AUTOHOOK(GetCurrentPlaylistVar, engine.dll + 0x18C680,
const char*, __fastcall, (const char* pVarName, bool bUseOverrides))
// clang-format on
{
	if (!bUseOverrides && !strcmp(pVarName, "max_players"))
		bUseOverrides = true;

	return GetCurrentPlaylistVar(pVarName, bUseOverrides);
}

// clang-format off
AUTOHOOK(GetCurrentGamemodeMaxPlayers, engine.dll + 0x18C430,
int, __fastcall, ())
// clang-format on
{
	const char* pMaxPlayers = R2::GetCurrentPlaylistVar("max_players", 0);
	if (!pMaxPlayers)
		return GetCurrentGamemodeMaxPlayers();

	int iMaxPlayers = atoi(pMaxPlayers);
	return iMaxPlayers;
}

ON_DLL_LOAD("engine.dll", PlaylistHooks, (CModule module))
{
	AUTOHOOK_DISPATCH()

	// patch to prevent clc_SetPlaylistVarOverride from being able to crash servers if we reach max overrides due to a call to Error (why is
	// this possible respawn, wtf) todo: add a warning for this
	module.Offset(0x18ED8D).Patch("C3");

	// patch to allow setplaylistvaroverride to be called before map init on dedicated and private match launched through the game
	module.Offset(0x18ED17).NOP(6);
}
