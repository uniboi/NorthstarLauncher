#pragma once

#include "game/server/baseentity.h"
#include "game/server/ai_node.h"

class CAI_NetworkManager : public CBaseEntity
{
  public:
	void* m_pEditOps; // 0x9e0 ( Size: 8 )
	CAI_Network* m_pNetwork; // 0x9e8 ( Size: 8 )
	bool m_fInitalized; // 0x9f0 ( Size: 1 )
	bool m_bDontSaveGraph; // 0x9f1 ( Size: 1 )
	char gap_9f2[2];
	int m_ainVersion; // 0x9f4 ( Size: 4 )
	int m_ainMapFilesCRC; // 0x9f8 ( Size: 4 )
	int m_runtimeCreatedAINMapFilesCRC; // 0x9fc ( Size: 4 )
	bool m_calculatedRuntimeAINMapFilesCRC; // 0xa00 ( Size: 1 )
	char gap_a01[7];
	char m_ThreadedBuild[72]; // 0xa08 ( Size: 72 )
};

inline CAI_Network** g_pAINetwork = nullptr;
inline CAI_NetworkManager** g_pNetworkManager = nullptr;
