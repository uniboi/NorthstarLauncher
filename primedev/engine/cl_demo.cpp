
ON_DLL_LOAD_CLIENT("engine.dll", EngineCLDemo, (CModule module))
{
	// allow demo recording on loopback
	module.Offset(0x8E1B1).NOP(2);
	module.Offset(0x56CC3).NOP(2);
}
