#pragma once

#include <string>

std::string Launcher_GetProfile();
std::wstring Launcher_GetExePath();

class CNorthstarLauncher
{
  public:
	CNorthstarLauncher();

	void WaitForDebugger();
	bool ShouldInject();
	void InitCoreSubsystems();
#ifdef LAUNCHER
	void InitOrigin();
#endif
	void InitNorthstarSubsystems();
	void InjectNorthstar();

  private:
	std::string m_svProfile;
	std::wstring m_wsvExePath;
};

extern CNorthstarLauncher* g_pLauncher;
