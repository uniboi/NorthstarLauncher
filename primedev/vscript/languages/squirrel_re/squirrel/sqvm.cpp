#include "vscript/languages/squirrel_re/squirrel/sqvm.h"

#include "vscript/languages/squirrel_re/squirrel/sqstate.h"
#include "vscript/languages/squirrel_re/squirrel/sqcompiler.h"
#include "vscript/languages/squirrel_re/vsquirrel.h"
#include "vscript/vscript.h"

eLog VScript_GetNativeLogContext(ScriptContext nContext);
const char* VScript_GetContextAsString(ScriptContext nContext);

void (*o_SQVM__FatalCompileErrorCallbackClient)(HSQUIRRELVM sqvm, const char* error, const char* file, int line, int column);
void (*o_SQVM__FatalCompileErrorCallbackServer)(HSQUIRRELVM sqvm, const char* error, const char* file, int line, int column);

void h_SQVM__FatalCompileErrorCallback(HSQUIRRELVM sqvm, const char* error, const char* file, int line, int column)
{
	ScriptContext realContext = (ScriptContext)sqvm->sharedState->cSquirrelVM->vmContext;
	eLog eContext = VScript_GetNativeLogContext(realContext);

	Error(eContext, NO_ERROR, "COMPILE ERROR %s\n", error);
	Error(eContext, NO_ERROR, "%s line [%i] column [%i]\n", file, line, column);

	// use disconnect to display an error message for the compile error
	// kill dedicated server if we hit this
	if (IsDedicatedServer())
	{
		Error(eContext, EXIT_FAILURE, "Exiting dedicated server, compile error is fatal\n");
	}
	else
	{
		Cbuf_AddText(Cbuf_GetCurrentPlayer(), fmt::format("disconnect \"Encountered {} script compilation error, see console for details.\"", VScript_GetContextAsString(realContext)).c_str(), cmd_source_t::kCommandSrcCode);

		// likely temp: show console so user can see any errors, as error message wont display if ui is dead
		// maybe we could disable all mods other than the coremods and try a reload before doing this?
		// could also maybe do some vgui bullshit to show something visually rather than console
		if (realContext == ScriptContext::UI)
			Cbuf_AddText(Cbuf_GetCurrentPlayer(), "showconsole", cmd_source_t::kCommandSrcCode);
	}

	// Original function sends a report to respawn, then kills the game so let's not do that
}

SQInteger (*o_SQVM__PrintUI)(HSQUIRRELVM sqvm, const char* fmt, ...);
SQInteger (*o_SQVM__PrintClient)(HSQUIRRELVM sqvm, const char* fmt, ...);
SQInteger (*o_SQVM__PrintServer)(HSQUIRRELVM sqvm, const char* fmt, ...);

SQInteger h_SQVM__Print(HSQUIRRELVM sqvm, const char* fmt, ...)
{
	ScriptContext nContext = (ScriptContext)sqvm->sharedState->cSquirrelVM->vmContext;

	va_list va;
	va_start(va, fmt);
	std::string svMsg = FormatAV(fmt, va);
	va_end(va);

	std::replace_if(
		svMsg.begin(), svMsg.end(), [](char c) { return (c == '\n'); }, ' ');

	DevMsg(VScript_GetScriptLogContext(nContext), "%s\n", svMsg.c_str());

	return 0;
}

void (*o_SQVM__CreateCompilerClient)(HSQUIRRELVM sqvm, void* a2, void* a3, SQBool bShouldThrowError);
void (*o_SQVM__CreateCompilerServer)(HSQUIRRELVM sqvm, void* a2, void* a3, SQBool bShouldThrowError);

void h_SQVM__CreateCompilerClient(HSQUIRRELVM sqvm, void* a2, void* a3, SQBool bShouldThrowError)
{
	o_SQVM__CreateCompilerClient(sqvm, a2, a3, bShouldThrowError);

	// Compiler is only created once, but this is set in its ctor so we have to do it manually
	sqvm->sharedState->pCompiler->bFatalError = bShouldThrowError;
}

void h_SQVM__CreateCompilerServer(HSQUIRRELVM sqvm, void* a2, void* a3, SQBool bShouldThrowError)
{
	o_SQVM__CreateCompilerServer(sqvm, a2, a3, bShouldThrowError);

	// Compiler is only created once, but this is set in its ctor so we have to do it manually
	sqvm->sharedState->pCompiler->bFatalError = bShouldThrowError;
}

ON_DLL_LOAD("client.dll", SQVMClient, (CModule module))
{
	o_SQVM__FatalCompileErrorCallbackClient = module.Offset(0x79A50).RCast<void (*)(HSQUIRRELVM, const char*, const char*, int, int)>();
	HookAttach(&(PVOID&)o_SQVM__FatalCompileErrorCallbackClient, (PVOID)h_SQVM__FatalCompileErrorCallback);

	o_SQVM__PrintUI = module.Offset(0x12BA0).RCast<SQInteger (*)(HSQUIRRELVM, const char*, ...)>();
	HookAttach(&(PVOID&)o_SQVM__PrintUI, (PVOID)h_SQVM__Print);
	o_SQVM__PrintClient = module.Offset(0x12B00).RCast<SQInteger (*)(HSQUIRRELVM, const char*, ...)>();
	HookAttach(&(PVOID&)o_SQVM__PrintClient, (PVOID)h_SQVM__Print);

	o_SQVM__CreateCompilerClient = module.Offset(0x8AD0).RCast<void (*)(HSQUIRRELVM, void*, void*, SQBool)>();
	HookAttach(&(PVOID&)o_SQVM__CreateCompilerClient, (PVOID)h_SQVM__CreateCompilerClient);
}

ON_DLL_LOAD("server.dll", SQVMServer, (CModule module))
{
	o_SQVM__FatalCompileErrorCallbackServer = module.Offset(0x799E0).RCast<void (*)(HSQUIRRELVM, const char*, const char*, int, int)>();
	HookAttach(&(PVOID&)o_SQVM__FatalCompileErrorCallbackServer, (PVOID)h_SQVM__FatalCompileErrorCallback);

	o_SQVM__PrintServer = module.Offset(0x1FE90).RCast<SQInteger (*)(HSQUIRRELVM, const char*, ...)>();
	HookAttach(&(PVOID&)o_SQVM__PrintServer, (PVOID)h_SQVM__Print);

	o_SQVM__CreateCompilerServer = module.Offset(0x8AA0).RCast<void (*)(HSQUIRRELVM, void*, void*, SQBool)>();
	HookAttach(&(PVOID&)o_SQVM__CreateCompilerServer, (PVOID)h_SQVM__CreateCompilerServer);
}
