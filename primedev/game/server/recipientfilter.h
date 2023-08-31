#pragma once

#include "game/server/player.h"

class CRecipientFilter
{
	char unknown[58];
};

inline void(__fastcall* CRecipientFilter__Construct)(CRecipientFilter* self);
inline void(__fastcall* CRecipientFilter__Destruct)(CRecipientFilter* self);
inline void(__fastcall* CRecipientFilter__AddAllPlayers)(CRecipientFilter* self);
inline void(__fastcall* CRecipientFilter__AddRecipient)(CRecipientFilter* self, const CPlayer* player);
inline void(__fastcall* CRecipientFilter__MakeReliable)(CRecipientFilter* self);
