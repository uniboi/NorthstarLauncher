
AUTOHOOK_INIT()

AUTOHOOK(CSourceAppSystemGroup__PreInit, launcher.dll + 0xB730, bool, __fastcall, (void* thisptr))
{
	DevMsg(eLog::ENGINE, "CSourceAppSystemGroup__PreInit\n");
	return CSourceAppSystemGroup__PreInit(thisptr);
}

// clang-format off
AUTOHOOK_PROCADDRESS(LauncherMain, launcher.dll, LauncherMain, int, __fastcall,
	(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow))
// clang-format on
{
	DevMsg(eLog::ENGINE, "LauncherMain\n");
	return LauncherMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}

ON_DLL_LOAD("launcher.dll", Launcher, (CModule module))
{
	AUTOHOOK_DISPATCH()
}
