

bool (*o_CSourceAppSystemGroup__PreInit)(void* self);
bool h_CSourceAppSystemGroup__PreInit(void* self)
{
	DevMsg(eLog::ENGINE, "CSourceAppSystemGroup__PreInit\n");
	return o_CSourceAppSystemGroup__PreInit(self);
}

int (*o_LauncherMain)(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
int h_LauncherMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	DevMsg(eLog::ENGINE, "LauncherMain\n");
	return o_LauncherMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}

ON_DLL_LOAD("launcher.dll", Launcher, (CModule module))
{
	o_CSourceAppSystemGroup__PreInit = module.Offset(0xB730).RCast<bool (*)(void*)>();
	HookAttach(&(PVOID&)o_CSourceAppSystemGroup__PreInit, (PVOID)h_CSourceAppSystemGroup__PreInit);

	o_LauncherMain = module.GetExportedFunction("LauncherMain").RCast<int (*)(HINSTANCE, HINSTANCE, LPSTR, int)>();
	HookAttach(&(PVOID&)o_LauncherMain, (PVOID)h_LauncherMain);
}
