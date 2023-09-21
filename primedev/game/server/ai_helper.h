#pragma once

#include "game/server/ai_node.h"

class CAI_Helper
{
  public:
	void SaveNetworkGraph(CAI_Network* pNetwork);
	void DrawNetwork(CAI_Network* pNetwork);
};

inline CAI_Helper* g_pAIHelper = nullptr;
