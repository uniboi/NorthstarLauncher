#include "pch.h"
#include "squirrel/squirrel.h"
#include "util/wininfo.h"

ADD_SQFUNC("array<int>", NSGetCursorPosition, "", "", ScriptContext::UI)
{
	std::cout << GetForegroundWindow() << "\n";
	std::cout << g_gameHWND << "\n";

	// SetForegroundWindow(*g_gameHWND);
	std::cout << GetForegroundWindow() << "\n=======\n";
	//std::cout << GetWindowThreadProcessId(*g_gameHWND, &GetProcessId(*g_gameHWND)) << "\n";

	DWORD processID = GetProcessId(*g_gameHWND);
	DWORD threadID = GetWindowThreadProcessId(*g_gameHWND, &processID);

	std::cout << threadID << "\n";

	POINT p;
	if (GetCursorPos(&p) && ScreenToClient(*g_gameHWND, &p))
	{
		g_pSquirrel<context>->newarray(sqvm, 0);
		g_pSquirrel<context>->pushinteger(sqvm, p.x);
		g_pSquirrel<context>->arrayappend(sqvm, -2);
		g_pSquirrel<context>->pushinteger(sqvm, p.y);
		g_pSquirrel<context>->arrayappend(sqvm, -2);
		return SQRESULT_NOTNULL;
	}
	g_pSquirrel<context>->raiseerror(
		sqvm, "Failed retrieving cursor position of game window");
	return SQRESULT_ERROR;
}
