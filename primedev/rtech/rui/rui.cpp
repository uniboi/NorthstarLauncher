
AUTOHOOK_INIT()

// clang-format off
AUTOHOOK(RUI_Draw, engine.dll + 0xFC500,
bool, __fastcall, (void* a1, float* a2))
// clang-format on
{
	if (!Cvar_rui_drawEnable->GetBool())
		return 0;

	return RUI_Draw(a1, a2);
}

ON_DLL_LOAD_CLIENT_RELIESON("engine.dll", RTechRUIRUI, ConVar, (CModule module))
{
	AUTOHOOK_DISPATCH()
}
