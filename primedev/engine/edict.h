#pragma once

#include "engine/globalvars_base.h"

typedef uint16_t edict_t;

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
enum MapLoadType_t
{
	MapLoad_NewGame = 0,
	MapLoad_Background = 2
};

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class CGlobalVars : public CGlobalVarsBase
{
  public:
	const char* m_pMapName;
	int m_nMapVersion;
	const char* m_pTest;
	MapLoadType_t m_MapLoadType;
	__int64 gap0;
	void* m_pUnk0;
	__int64 gap;
	void* m_pUnk1;
	__int64 m_Unk2;
};
static_assert(sizeof(CGlobalVars) == 0xA8);

inline CGlobalVars* g_pServerGlobalVariables;
inline CGlobalVarsBase* g_pClientGlobalVariables;
