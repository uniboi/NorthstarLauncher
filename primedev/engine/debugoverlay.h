#pragma once

#include "engine/ivdebugoverlay.h"
#include "engine/vphysics_interface.h"

class CIVDebugOverlay : public IVDebugOverlay, public IVPhysicsDebugOverlay
{
  public:
};

inline CIVDebugOverlay* g_pDebugOverlay = nullptr;
