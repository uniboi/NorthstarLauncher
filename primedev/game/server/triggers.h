#pragma once

#include "game/server/basetoggle.h"
#include "tier1/utlvector.h"
#include "mathlib/vector.h"
#include "mathlib/vplane.h"

class CUnknownTriggerPlane
{
  public:
	VPlane m_Plane;
	float m_Unk; // inverted plane dist?
	int unki[3];
	CCopyableUtlVectorFixed<Vector3, 64> m_vVertices;
};

class CUnknownTriggerBrushRef
{
  public:
	float unkf[4];
	CUtlVectorFixed<CUnknownTriggerPlane, 64> m_vPlanes;
	char unk[4];
};
static_assert(sizeof(CUnknownTriggerBrushRef) == 52248);

class CBaseTrigger : public CBaseToggle
{
  public:
	bool m_bDisabled; // 0xa60 ( Size: 1 )
	char gap_a61[7];
	char m_OnStartTouch[40]; // 0xa68 ( Size: 40 )
	char m_OnStartTouchAll[40]; // 0xa90 ( Size: 40 )
	char m_OnEndTouch[40]; // 0xab8 ( Size: 40 )
	char m_OnEndTouchAll[40]; // 0xae0 ( Size: 40 )
	char m_OnTouching[40]; // 0xb08 ( Size: 40 )
	char m_OnNotTouching[40]; // 0xb30 ( Size: 40 )
	char m_onStartNearby[40]; // 0xb58 ( Size: 40 )
	char m_onEndNearby[40]; // 0xb80 ( Size: 40 )
	char m_hTouchingEntities[32]; // 0xba8 ( Size: 32 )
	char m_nearbyEntities[32]; // 0xbc8 ( Size: 32 )
	bool m_bClientSidePredicted; // 0xbe8 ( Size: 1 )
	bool m_triggerFilterUseNew; // 0xbe9 ( Size: 1 )
	bool m_triggerFilterNpcFlip; // 0xbea ( Size: 1 )
	char gap_beb[5];
	__int64 m_triggerFilterMask; // 0xbf0 ( Size: 8 )
	char m_enterCallback[16]; // 0xbf8 ( Size: 16 )
	char m_leaveCallback[16]; // 0xc08 ( Size: 16 )
	bool m_triggerInitialized; // 0xc18 ( Size: 1 )
	char gap_c19[3];
	float m_triggerNearbyRadius; // 0xc1c ( Size: 4 )
	CUtlVector<CUnknownTriggerBrushRef> m_nearbyBrushes; // 0xc20 ( Size: 32 ) // TODO [Fifty]: Check this is the same type as m_triggerBrushes
	CUtlVector<CUnknownTriggerBrushRef> m_triggerBrushes; // 0xc40 ( Size: 32 )
};
static_assert(sizeof(CBaseTrigger) == 0xc60);

void Triggers_Draw();
bool IsTriggerEntity(CBaseEntity* pEntity);
