#include "originsdk/origin.h"

ON_DLL_LOAD("engine.dll", OriginSDK, (CModule module))
{
	g_pLocalPlayerUserID = module.Offset(0x13F8E688).RCast<char*>();
	g_pLocalPlayerOriginToken = module.Offset(0x13979C80).RCast<char*>();
}
