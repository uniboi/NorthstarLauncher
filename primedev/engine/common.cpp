
AUTOHOOK_INIT()

// clang-format off
AUTOHOOK(COM_ExplainDisconnection, engine.dll + 0x1342F0,
void, __fastcall, (bool bPrint, const char* fmt, ...))
// clang-format on
{
	va_list va;
	va_start(va, fmt);
	char szString[1024];
	vsnprintf_s(szString, 1024, fmt, va);
	va_end(va);

	if (bPrint)
	{
		DevMsg(eLog::ENGINE, "%s\n", szString);
	}

	// [Bob]: when COM_ExplainDisconnection is called from engine.dll + 19ff1c for connection rejected, it doesn't
	//        call Host_Disconnect, which properly shuts down listen server
	//        not doing this gets our client in a pretty weird state so we need to shut it down manually here

	// [Fifty]: Removed check for a specific message to catch all occurences of this bug. Calling an extra 'disconnect'
	//          doesn't hurt

	// don't call Cbuf_Execute because we don't need this called immediately
	Cbuf_AddText(Cbuf_GetCurrentPlayer(), "disconnect", cmd_source_t::kCommandSrcCode);

	return COM_ExplainDisconnection(bPrint, szString);
}

ON_DLL_LOAD_CLIENT("engine.dll", RejectConnectionFixes, (CModule module))
{
	AUTOHOOK_DISPATCH()
}
