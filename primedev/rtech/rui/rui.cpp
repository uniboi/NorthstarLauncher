

bool (*o_RUI_Draw)(void* a1, void* a2);

bool h_RUI_Draw(void* a1, float* a2)
{
	if (!Cvar_rui_drawEnable->GetBool())
		return 0;

	return o_RUI_Draw(a1, a2);
}

ON_DLL_LOAD_CLIENT("engine.dll", RTechRUIRUI, (CModule module))
{
	o_RUI_Draw = module.Offset(0xFC500).RCast<bool (*)(void*, void*)>();
	HookAttach(&(PVOID&)o_RUI_Draw, (PVOID)h_RUI_Draw);
}
