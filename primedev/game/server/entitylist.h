#pragma once

#include "game/server/baseentity.h"

inline void* gEntList;

inline CBaseEntity*(*CGlobalEntityList__FirstEnt)(void* self, void* index);
