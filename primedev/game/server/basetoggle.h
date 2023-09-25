#pragma once

#include "game/server/baseentity.h"

class CBaseToggle : public CBaseEntity
{
  public:
	int m_toggle_state; // 0x9e0 ( Size: 4 )
	float m_flMoveDistance; // 0x9e4 ( Size: 4 )
	float m_flWait; // 0x9e8 ( Size: 4 )
	float m_flLip; // 0x9ec ( Size: 4 )
	Vector3 m_vecPosition1; // 0x9f0 ( Size: 12 )
	Vector3 m_vecPosition2; // 0x9fc ( Size: 12 )
	Vector3 m_vecMoveAng; // 0xa08 ( Size: 12 )
	Vector3 m_vecAngle1; // 0xa14 ( Size: 12 )
	Vector3 m_vecAngle2; // 0xa20 ( Size: 12 )
	float m_flHeight; // 0xa2c ( Size: 4 )
	int /*EHANDLE*/ m_hActivator; // 0xa30 ( Size: 4 )
	Vector3 m_vecFinalDest; // 0xa34 ( Size: 12 )
	Vector3 m_vecFinalAngle; // 0xa40 ( Size: 12 )
	int m_movementType; // 0xa4c ( Size: 4 )
	int m_flMoveTargetTime; // 0xa50 ( Size: 4 )
	char gap_a54[4];
	__int64 m_sMaster; // 0xa58 ( Size: 8 )
};
static_assert(sizeof(CBaseToggle) == 0xa60);
