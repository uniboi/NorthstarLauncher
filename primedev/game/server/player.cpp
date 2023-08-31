#include "game/server/player.h"

ON_DLL_LOAD("server.dll", GameServerPlayer, (CModule module))
{
	CPlayer__RunNullCommand = module.Offset(0x5A9FD0).RCast<void (*__fastcall)(CPlayer*)>();
}
