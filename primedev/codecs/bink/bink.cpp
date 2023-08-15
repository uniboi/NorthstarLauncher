
ON_DLL_LOAD_CLIENT("engine.dll", Bink, (CModule module))
{
	// remove engine check for whether the bik we're trying to load exists in r2/media, as this will fail for biks in mods
	// note: the check in engine is actually unnecessary, so it's just useless in practice and we lose nothing by removing it
	module.Offset(0x459AD).NOP(6);
}
