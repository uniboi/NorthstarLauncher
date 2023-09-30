#pragma once
#include "convar.h"
#include "appframework/IAppSystem.h"

typedef int CVarDLLIdentifier_t;

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
class ConCommandBase;
class ConCommand;
class ConVar;

//-----------------------------------------------------------------------------
// Internals for ICVarIterator
//-----------------------------------------------------------------------------
class CCVarIteratorInternal // Fully reversed table, just look at the virtual function table and rename the function.
{
  public:
	virtual void SetFirst(void) = 0; // 0
	virtual void Next(void) = 0; // 1
	virtual bool IsValid(void) = 0; // 2
	virtual ConCommandBase* Get(void) = 0; // 3
};

//-----------------------------------------------------------------------------
// Default implementation
//-----------------------------------------------------------------------------
class ICvar : public IAppSystem
{
  public:
	virtual CVarDLLIdentifier_t AllocateDLLIdentifier() = 0;

	// Register, unregister commands
	virtual void RegisterConCommand(ConCommandBase* pCommandBase) = 0;
	virtual void UnregisterConCommand(ConCommandBase* pCommandBase) = 0;
	virtual void UnregisterConCommands(CVarDLLIdentifier_t id) = 0;

	virtual const char* GetCommandLineValue(const char* pVariableName) = 0;

	virtual ConCommandBase* FindCommandBase(const char* name) = 0;
	virtual const ConCommandBase* FindCommandBase(const char* name) const = 0;
	virtual ConVar* FindVar(const char* var_name) = 0;
	virtual const ConVar* FindVar(const char* var_name) const = 0;
	virtual ConCommand* FindCommand(const char* name) = 0;
	virtual const ConCommand* FindCommand(const char* name) const = 0;

	virtual void sub_18000CF30() = 0;
	virtual void sub_18000E6F0() = 0;

	virtual void sub_18000C200() = 0;
	virtual void sub_18000E610() = 0;
	virtual void sub_180009DE0() = 0;

	virtual void sub_18000C190() = 0; // InstallConsoleDisplayFunc
	virtual void sub_18000E5A0() = 0; // RemoveConsoleDisplayFunc

	virtual void ConsoleColorPrintf(const SourceColor& clr, const char* pFormat, ...) const = 0;
	virtual void ConsolePrintf(const char* pFormat, ...) const = 0;
	virtual void ConsoleDPrintf(const char* pFormat, ...) const = 0;

	virtual void sub_18000EC50() = 0;

	virtual void sub_18000C170() = 0;

	virtual void sub_18000F4B0() = 0;
	virtual void sub_unk() = 0;

	virtual void sub_18000AEB0() = 0;
	virtual void sub_18000AEC0() = 0;

	virtual void sub_18000C530() = 0;
	virtual void sub_18000DD60() = 0;
	virtual void sub_18000DCD0() = 0;
	virtual void sub_18000DDF0() = 0;
	virtual void sub_18000B8A0() = 0;
	virtual void sub_18000D3C0() = 0;

	virtual CCVarIteratorInternal* FactoryInternalIterator() = 0;
};

class CCvar : public ICvar
{
  public:
	std::string GetFlagsString(int nFlags, bool bConVar);
	void PrintHelpString(const char* szName);
};

extern CCvar* g_pCVar;
