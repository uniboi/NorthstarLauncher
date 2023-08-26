#include "cvar.h"
#include "convar.h"
#include "tier1/cmd.h"

//-----------------------------------------------------------------------------
// Purpose: returns all ConVars
//-----------------------------------------------------------------------------
std::unordered_map<std::string, ConCommandBase*> CCvar::DumpToMap()
{
	std::stringstream ss;
	CCVarIteratorInternal* itint = FactoryInternalIterator(); // Allocate new InternalIterator.

	std::unordered_map<std::string, ConCommandBase*> allConVars;

	for (itint->SetFirst(); itint->IsValid(); itint->Next()) // Loop through all instances.
	{
		ConCommandBase* pCommand = itint->Get();
		const char* pszCommandName = pCommand->m_pszName;
		allConVars[pszCommandName] = pCommand;
	}

	return allConVars;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
std::string CCvar::GetFlagsString(int nFlags, bool bConVar)
{
	std::string svString = " ";

	for (int i = 0; i < 32; i++)
	{
		int nFlag = 1 << i;

		if (!(nFlags & nFlag))
			continue;

		switch (nFlag)
		{
		case FCVAR_UNREGISTERED:
			svString += "UNREGISTERED ";
			break;
		case FCVAR_DEVELOPMENTONLY:
			svString += "DEVELOPMENTONLY ";
			break;
		case FCVAR_GAMEDLL:
			svString += "GAMEDLL ";
			break;
		case FCVAR_CLIENTDLL:
			svString += "CLIENTDLL ";
			break;
		case FCVAR_HIDDEN:
			svString += "HIDDEN ";
			break;
		case FCVAR_PROTECTED:
			svString += "PROTECTED ";
			break;
		case FCVAR_SPONLY:
			svString += "SPONLY ";
			break;
		case FCVAR_ARCHIVE:
			svString += "ARCHIVE ";
			break;
		case FCVAR_NOTIFY:
			svString += "NOTIFY ";
			break;
		case FCVAR_USERINFO:
			svString += "USERINFO ";
			break;
		case FCVAR_PRINTABLEONLY:
		{
			if (bConVar)
				svString += "PRINTABLEONLY ";
			else
				svString += "GAMEDLL_FOR_REMOTE_CLIENTS ";

			break;
		}
		case FCVAR_UNLOGGED:
			svString += "UNLOGGED ";
			break;
		case FCVAR_NEVER_AS_STRING:
			svString += "NEVER_AS_STRING ";
			break;
		case FCVAR_REPLICATED:
			svString += "REPLICATED ";
			break;
		case FCVAR_CHEAT:
			svString += "CHEAT ";
			break;
		case FCVAR_SS:
			svString += "SS ";
			break;
		case FCVAR_DEMO:
			svString += "DEMO ";
			break;
		case FCVAR_DONTRECORD:
			svString += "DONTRECORD ";
			break;
		case FCVAR_SS_ADDED:
			svString += "SS_ADDED ";
			break;
		case FCVAR_RELEASE:
			svString += "RELEASE ";
			break;
		case FCVAR_RELOAD_MATERIALS:
			svString += "RELOAD_MATERIALS ";
			break;
		case FCVAR_RELOAD_TEXTURES:
			svString += "RELOAD_TEXTURES ";
			break;
		case FCVAR_NOT_CONNECTED:
			svString += "NOT_CONNECTED ";
			break;
		case FCVAR_MATERIAL_SYSTEM_THREAD:
			svString += "MATERIAL_SYSTEM_THREAD ";
			break;
		case FCVAR_ARCHIVE_PLAYERPROFILE:
			svString += "ARCHIVE_PLAYERPROFILE ";
			break;
		case FCVAR_SERVER_CAN_EXECUTE:
			svString += "SERVER_CAN_EXECUTE ";
			break;
		case FCVAR_SERVER_CANNOT_QUERY:
			svString += "SERVER_CANNOT_QUERY ";
			break;
		case FCVAR_CLIENTCMD_CAN_EXECUTE:
			svString += "UNKNOWN ";
			break;
		case FCVAR_ACCESSIBLE_FROM_THREADS:
			svString += "ACCESSIBLE_FROM_THREADS ";
			break;
		}
	}

	return svString;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CCvar::PrintHelpString(const char* szName)
{
	ConCommandBase* pCmdBase = FindCommandBase(szName);

	if (!pCmdBase)
	{
		DevMsg(eLog::NS, "Couldn't find '%s'!\n", szName);
		return;
	}

	ConVar* pCvar = FindVar(szName);

	std::string svFlags = GetFlagsString(pCmdBase->m_nFlags, !!pCvar);

	if (pCvar)
	{
		DevMsg(eLog::NS, "%s = %s (%s) - %s\n", szName, pCvar->GetString(), svFlags.c_str(), pCmdBase->m_pszHelpString);
	}
	else
	{
		DevMsg(eLog::NS, "%s (%s) - %s\n", szName, svFlags.c_str(), pCmdBase->m_pszHelpString);
	}
}

CCvar* g_pCVar;
