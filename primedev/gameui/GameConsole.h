#pragma once

//-----------------------------------------------------------------------------
// Purpose: VGui implementation of the game/dev console
//-----------------------------------------------------------------------------
class CGameConsole // : public IGameConsole
{
  public:
	virtual ~CGameConsole() = 0;

	// activates the console, makes it visible and brings it to the foreground
	virtual void Activate() = 0;

	virtual void Initialize() = 0;

	// hides the console
	virtual void Hide() = 0;

	// clears the console
	virtual void Clear() = 0;

	// return true if the console has focus
	virtual bool IsConsoleVisible() = 0;

	virtual void SetParent(int parent) = 0;

	bool m_bInitialized;
	void /*CConsoleDialog*/* m_pConsole;
};

extern CGameConsole* g_pGameConsole;
