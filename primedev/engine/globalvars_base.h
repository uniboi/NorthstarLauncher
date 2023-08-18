#pragma once

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
enum GameMode_t
{
	NO_MODE = 0,
	MP_MODE,
	SP_MODE,
};

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class CGlobalVarsBase
{
  public:
	double m_flRealTime;
	int m_nFrameCount;
	float m_flAbsoluteFrameTime;
	float m_flCurTime;
	float m_fUnk0;
	float m_fUnk1;
	float m_fUnk2;
	float m_fUnk3;
	float m_fUnk4;
	float m_fUnk5;
	float m_fUnk6;
	float m_flFrameTime;
	int m_nMaxClients;
	GameMode_t m_nGameMode;
	float m_Unk7;
	float m_flTickInterval;
	float m_Unk8;
	float m_Unk9;
	float m_Unk10;
	float m_Unk11;
	float m_Unk12;
	float m_Unk13;
};
static_assert(sizeof(CGlobalVarsBase) == 0x60);
