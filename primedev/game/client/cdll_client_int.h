#pragma once

#include "toolframework/itoolentity.h"

inline IClientTools* g_pClientTools = nullptr;

inline void (*o_OnRenderStart)();
inline void (*LatencyFlex_WaitAndBeginFrame)();
