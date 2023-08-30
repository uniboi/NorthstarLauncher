#pragma once

#include "mathlib/vector.h"
#include "game/server/player.h"

inline CBaseEntity* (*Server_GetEntityByIndex)(int index);

inline CPlayer*(__fastcall* UTIL_PlayerByIndex)(int playerIndex);
inline void (*__fastcall CPlayer__RunNullCommand)(CPlayer* self);
