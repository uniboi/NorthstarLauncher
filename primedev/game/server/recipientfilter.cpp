#include "game/server/recipientfilter.h"

ON_DLL_LOAD("server.dll", RecipientFilter, (CModule module))
{
	CRecipientFilter__Construct = module.Offset(0x1E9440).RCast<void(__fastcall*)(CRecipientFilter*)>();
	CRecipientFilter__Destruct = module.Offset(0x1E9700).RCast<void(__fastcall*)(CRecipientFilter*)>();
	CRecipientFilter__AddAllPlayers = module.Offset(0x1E9940).RCast<void(__fastcall*)(CRecipientFilter*)>();
	CRecipientFilter__AddRecipient = module.Offset(0x1E9B30).RCast<void(__fastcall*)(CRecipientFilter*, const CPlayer*)>();
	CRecipientFilter__MakeReliable = module.Offset(0x1EA4E0).RCast<void(__fastcall*)(CRecipientFilter*)>();
}
