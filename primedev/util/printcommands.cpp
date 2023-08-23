#include "printcommands.h"
#include "tier1/convar.h"
#include "tier1/cmd.h"

void PrintCommandHelpDialogue(const ConCommandBase* command, const char* name)
{
	if (!command)
	{
		DevMsg(eLog::NS, "unknown command %s\n", name);
		return;
	}

	// temp because command->IsCommand does not currently work
	ConVar* cvar = g_pCVar->FindVar(command->m_pszName);

	// build string for flags if not FCVAR_NONE
	std::string flagString;
	if (command->GetFlags() != FCVAR_NONE)
	{
		flagString = "( ";

		for (auto& flagPair : g_PrintCommandFlags)
		{
			if (command->GetFlags() & flagPair.first)
			{
				// special case, slightly hacky: PRINTABLEONLY is for commands, GAMEDLL_FOR_REMOTE_CLIENTS is for concommands, both have the
				// same value
				if (flagPair.first == FCVAR_PRINTABLEONLY)
				{
					if (cvar && !strcmp(flagPair.second, "GAMEDLL_FOR_REMOTE_CLIENTS"))
						continue;

					if (!cvar && !strcmp(flagPair.second, "PRINTABLEONLY"))
						continue;
				}

				flagString += flagPair.second;
				flagString += " ";
			}
		}

		flagString += ") ";
	}

	if (cvar)
		DevMsg(eLog::NS, "\"%s\" = \"%s\" %s- %s\n", cvar->GetBaseName(), cvar->GetString(), flagString.c_str(), cvar->GetHelpText());
	else
		DevMsg(eLog::NS, "\"%s\" %s - %s\n", command->m_pszName, flagString.c_str(), command->GetHelpText());
}

void TryPrintCvarHelpForCommand(const char* pCommand)
{
	// try to display help text for an inputted command string from the console
	int pCommandLen = strlen(pCommand);
	char* pCvarStr = new char[pCommandLen];
	strcpy(pCvarStr, pCommand);

	// trim whitespace from right
	for (int i = pCommandLen - 1; i; i--)
	{
		if (isspace(pCvarStr[i]))
			pCvarStr[i] = '\0';
		else
			break;
	}

	// check if we're inputting a cvar, but not setting it at all
	ConVar* cvar = g_pCVar->FindVar(pCvarStr);
	if (cvar)
		PrintCommandHelpDialogue(&cvar->m_ConCommandBase, pCvarStr);

	delete[] pCvarStr;
}

void ConCommand_help(const CCommand& arg)
{
	if (arg.ArgC() < 2)
	{
		DevMsg(eLog::NS, "Usage: help <cvarname>\n");
		return;
	}

	PrintCommandHelpDialogue(g_pCVar->FindCommandBase(arg.Arg(1)), arg.Arg(1));
}

void InitialiseCommandPrint()
{
	// help is already a command, so we need to modify the preexisting command to use our func instead
	// and clear the flags also
	ConCommand* helpCommand = g_pCVar->FindCommand("help");
	helpCommand->m_nFlags = FCVAR_NONE;
	helpCommand->m_pCommandCallback = ConCommand_help;
}
