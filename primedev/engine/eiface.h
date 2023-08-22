#pragma once

#include "engine/edict.h"
#include "shared/keyvalues.h"

class IVEngineServer
{
  public:
	virtual void Changelevel(const char* s1, const char* s2) = 0;
	virtual void sub_18011B140() = 0;
	virtual void sub_18011B410() = 0;
	virtual void sub_18011B6F0() = 0;
	virtual void sub_18011B3A0() = 0;
	virtual void sub_18011B3C0() = 0;
	virtual KeyValues* GetLaunchOptions() = 0;

	virtual int PrecacheModel(const char* szName) = 0;
	virtual void sub_18011B440() = 0;

	virtual void sub_18011B520() = 0;

	virtual void sub_18011ACB0() = 0;
	virtual void sub_18011A9C0() = 0;
	virtual void sub_18011AA00() = 0;
	virtual void sub_18011A860() = 0;
	virtual void sub_18011AD40() = 0;
	virtual void sub_18011C730() = 0;
	virtual void sub_18011C790() = 0;
	virtual void sub_18011C8B0() = 0;
	virtual void sub_18011A650() = 0;
	virtual void sub_18011C870() = 0;

	virtual void FadeClientVolume(const edict_t* pEdict, float flFadePercent, float flFadeOutSeconds, float flHoldTime, float flFadeInSeconds) = 0;

	virtual void ServerCommand(const char* szCommand) = 0;
	virtual void ServerExecute() = 0;

	virtual void ClientCommand(edict_t* pEdict, const char* szFmt, ...) = 0;

	virtual void LightStyle(int nStyle, const char* szVal) = 0;

	virtual void* UserMessageBegin(__int64 a2, int a3, __int64 a4, int a5) = 0;
	virtual void UserMessageEnd() = 0;

	virtual void ClientPrintf(edict_t nEdict, const char* szMsg) = 0;

	virtual void Con_NPrintf(int nPos, const char* szFmt, ...) = 0;
	virtual void Con_NXPrintf(void* pInfo, const char* szFmt, ...) = 0;

	virtual void sub_18011CDD0() = 0;
	virtual void sub_18011B190() = 0;
	virtual void sub_18011CD10() = 0;
	virtual void sub_18011CCD0() = 0;

	virtual void CrosshairAngle(const edict_t nClient, float flPitch, float flYaw) = 0;

	virtual bool GrantClientSidePickup(const edict_t nClient, int a3, int a4, int* a5, int a6, int a7) = 0;

	virtual void GetGameDir(char* szGetGameDir, int nMaxlength) = 0;

	virtual int CompareFileTime(const char* szFilename1, const char* szFilename2, int* iCompare) = 0;

	virtual void LockNetworkStringTables(bool bLock) = 0;

	virtual void sub_18011AD70() = 0;
	virtual void sub_18011AD30() = 0;
	virtual void sub_18011AD80() = 0;

	virtual edict_t CreateFakeClient(const char* szName, const char* szUnk, const char* szPlaylist, int nTeam) = 0;

	virtual void sub_18011AAD0() = 0;
	virtual void sub_18011AB60() = 0;
	virtual void sub_18011AB40() = 0;
	virtual void sub_18011AB80() = 0;
	virtual void sub_18011C8A0() = 0;
	virtual void sub_18011CC50() = 0;
	virtual void sub_18011AF10() = 0;
	virtual void sub_18011B0F0() = 0;
	virtual void sub_18011C880() = 0;
	virtual void sub_18011CC00() = 0;
	virtual void sub_18011CDA0() = 0;
	virtual void sub_18011B0D0() = 0;
	virtual void sub_18011CB50() = 0;
	virtual void sub_18011CA90() = 0;
	virtual void sub_18011CAF0() = 0;
	virtual void sub_18011BA90() = 0;
	virtual void sub_18011A640() = 0;
	virtual void sub_18011AA50() = 0;
	virtual void sub_18011B5A0() = 0;
	virtual void sub_18011C960() = 0;
	virtual void sub_18011B110() = 0;
	virtual void sub_18011C970() = 0;
	virtual void sub_18011B4D0() = 0;
	virtual void sub_18011B4B0() = 0;
	virtual void sub_18011AD10() = 0;
	virtual void sub_18011ADC0() = 0;
	virtual void sub_18011B080() = 0;
	virtual void sub_18011B090() = 0;
	virtual void sub_18011B0B0() = 0;
	virtual void sub_18011CD90() = 0;
	virtual void sub_18011B010() = 0;
	virtual void sub_18011B030() = 0;
	virtual void sub_18011B050() = 0;
	virtual void sub_18011B060() = 0;
	virtual void sub_18011A400() = 0;
	virtual void sub_18011CBB0() = 0;
	virtual void sub_18011ABA0() = 0;
	virtual void sub_18011ACD0() = 0;
	virtual void sub_18011CBC0() = 0;
	virtual void sub_18011AC60() = 0;
	virtual void sub_18011AC70() = 0;
	virtual void sub_18011AC80() = 0;
	virtual void sub_18011AC40() = 0;
	virtual void sub_18011AFF0() = 0;
	virtual void sub_18011ABB0() = 0;
	virtual void sub_18011AF60() = 0;
	virtual void sub_18011AFD0() = 0;
	virtual void sub_18011AC20() = 0;
	virtual void sub_18011AC10() = 0;
	virtual void sub_18011AFC0() = 0;
	virtual void sub_18011ABE0() = 0;
	virtual void sub_18011AF90() = 0;
	virtual void sub_18011AC50() = 0;
	virtual void sub_18011B000() = 0;
	virtual void sub_18011B430() = 0;
	virtual void sub_18011A890() = 0;
	virtual void sub_18011B7F0() = 0;
	virtual void sub_18011B7E0() = 0;
	virtual void sub_18011C760() = 0;
	virtual void sub_18011B4A0() = 0;
	virtual void sub_18011B340() = 0;
	virtual void sub_18011B460() = 0;
	virtual void sub_18011CE60() = 0;
	virtual void sub_18011CE70() = 0;
	virtual void sub_18011CE30() = 0;
	virtual void sub_18011CEA0() = 0;
	virtual void sub_18011A700() = 0;
	virtual void sub_18011A7D0() = 0;
	virtual void sub_18011AA60() = 0;
	virtual void sub_18011B170() = 0;
	virtual void sub_18011B490() = 0;
	virtual void sub_18011B150() = 0;
	virtual void sub_18011B3D0() = 0;
	virtual void sub_18011B4C0() = 0;
	virtual void sub_18011A9B0() = 0;
	virtual void sub_18011B3F0() = 0;
	virtual void sub_18011B2F0() = 0;
	virtual void sub_18011B290() = 0;
	virtual void sub_18011AE90() = 0;
	virtual void sub_18011AED0() = 0;
	virtual void sub_18011CBD0() = 0;
	virtual void sub_18011B350() = 0;
	virtual void sub_18011B370() = 0;
	virtual void sub_18011B3B0() = 0;
	virtual void sub_18011CDC0() = 0;
	virtual void sub_18011CBF0() = 0;
	virtual void sub_18011ACE0() = 0;
	virtual void sub_18011CE10() = 0;
	virtual void sub_18011A4D0() = 0;
	virtual void sub_18011B790() = 0;
	virtual void sub_18011B750() = 0;
	virtual void sub_18011CD50() = 0;
	virtual void sub_18011AF30() = 0;
	virtual void sub_18011AEA0() = 0;
	virtual void sub_18011A820() = 0;
	virtual void sub_18011B990() = 0;
	virtual void sub_18011B940() = 0;
	virtual void nullsub_216() = 0;
	virtual void nullsub_217() = 0;
	virtual void nullsub_218() = 0;
	virtual void sub_18011A2E0() = 0;
	virtual void sub_18011B660() = 0;
	virtual void sub_18011B5D0() = 0;
	virtual void sub_18011B600() = 0;
	virtual void sub_18011B690() = 0;
	virtual void sub_18011B630() = 0;
	virtual void sub_18011A8B0() = 0;
	virtual void sub_18011A8C0() = 0;
	virtual void sub_18011A8A0() = 0;
	virtual void sub_18011CFB0() = 0;
	virtual void sub_18011C250() = 0;
	virtual void sub_18011BEF0() = 0;
	virtual void sub_18011C280() = 0;
	virtual void sub_18011BA60() = 0;
	virtual void nullsub_219() = 0;
	virtual void sub_18011AC90() = 0;
	virtual void sub_18011B4F0() = 0;
	virtual void sub_18011BA20() = 0;
	virtual void sub_18011A410() = 0;
	virtual void sub_18011C850() = 0;
	virtual void sub_18011C860() = 0;
	virtual void sub_18011AD20() = 0;
	virtual void sub_18011AD60() = 0;
	virtual void sub_18011ADD0() = 0;
	virtual void sub_18011B120() = 0;
	virtual void sub_18011B130() = 0;
	virtual void sub_18011B480() = 0;
	virtual void sub_18011C9F0() = 0;
	virtual void nullsub_220() = 0;
	virtual void sub_18011B180() = 0;
	virtual void sub_18011AA40() = 0;
	virtual void nullsub_221() = 0;
	virtual void nullsub_222() = 0;
	virtual void sub_18011BAB0() = 0;
	virtual void sub_18011BAC0() = 0;
	virtual void sub_18011BAD0() = 0;
	virtual void sub_18011BB20() = 0;
	virtual void sub_18011BB30() = 0;
	virtual void sub_18011BB50() = 0;
	virtual void sub_18011BB00() = 0;
	virtual void sub_18011BAE0() = 0;
	virtual void sub_18011BBB0() = 0;
	virtual void sub_18011BBF0() = 0;
	virtual void sub_18011BBD0() = 0;
	virtual void sub_18011BBE0() = 0;
	virtual void sub_18011BB10() = 0;
	virtual void sub_18011BB90() = 0;
	virtual void sub_18011BB80() = 0;
	virtual void sub_18011BBA0() = 0;
	virtual void sub_18011BB70() = 0;
	virtual void sub_18011BB60() = 0;
	virtual void sub_18011BBC0() = 0;
	virtual void sub_18011BC00() = 0;
	virtual void sub_18011BC40() = 0;
	virtual void sub_18011BC30() = 0;
	virtual void sub_18011BC50() = 0;
	virtual void sub_18011BC20() = 0;
	virtual void sub_18011ACA0() = 0;
	virtual void sub_18011CE50() = 0;
	virtual void sub_18011C9D0() = 0;
	virtual void sub_18011B470() = 0;
	virtual void sub_18011A7E0() = 0;
	virtual void sub_18011A810() = 0;
	virtual void nullsub_225() = 0;
	virtual void sub_18011B510() = 0;
	virtual void sub_18011B3E0() = 0;
	virtual void sub_18011A9A0() = 0;
	virtual void sub_18011CE80() = 0;
};


class IServerGameClients
{
  public:
	virtual void GetPlayerLimits(int& nMinPlayers, int& nMaxPlayers, int& nDefaultMaxPlayers) = 0;

	virtual void sub_180153970() = 0;

	virtual void ClientActive(edict_t pEntity, bool bLoadGame) = 0;

	virtual void ClientFullyConnect(edict_t nEntity, bool bRestore) = 0;

	virtual void sub_1801539A0() = 0;
	virtual void sub_180153BC0() = 0;
	virtual void sub_1801538C0() = 0;
	virtual void sub_180153CA0() = 0;

	virtual void ProcessUsercmds(edict_t nPlayer, __int64 a3, int a4, unsigned int a5, int a6, char a7, char a8) = 0;

	virtual void sub_1801575E0() = 0;
	virtual void sub_180153AC0() = 0;
	virtual void sub_180153920() = 0;
	virtual void sub_180153C30() = 0;
	virtual void sub_18015ACC0() = 0;
	virtual void sub_18015AD20() = 0;
	virtual void sub_180157490() = 0;
	virtual void sub_18015AC80() = 0;
	virtual void sub_180159B20() = 0;
	virtual void nullsub_243() = 0;
	virtual void sub_180157060() = 0;
	virtual void sub_180157040() = 0;
};
