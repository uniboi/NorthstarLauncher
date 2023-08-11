#pragma once

#if defined(LAUNCHER) || defined(WSOCKPROXY)
inline bool g_bConsole_UseAnsiColor = false;

void Console_Init(void);
#endif
#ifdef NORTHSTAR
void Console_PostInit(void);
#endif
