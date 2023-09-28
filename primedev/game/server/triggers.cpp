#include "game/server/triggers.h"

#include "game/client/cdll_client_int.h"
#include "game/server/entitylist.h"
#include "engine/debugoverlay.h"

void* g_pCTonemapTrigger_VFTable = nullptr;
void* g_pCFogTrigger_VFTable = nullptr;
void* g_pCTriggerSoundScape_VFTable = nullptr;
void* g_pCTriggerMultiple_VFTable = nullptr;
void* g_pCTriggerHurt_VFTable = nullptr;
void* g_pCTriggerCylinder_VFTable = nullptr;
void* g_pCTriggerPointGravity_VFTable = nullptr;
void* g_pCTriggerRemove_VFTable = nullptr;
void* g_pCTriggerOnce_VFTable = nullptr;
void* g_pCTriggerLook_VFTable = nullptr;
void* g_pCTriggerPush_VFTable = nullptr;
void* g_pCTriggerTeleport_VFTable = nullptr;
void* g_pCTriggerGravity_VFTable = nullptr;
void* g_pCTriggerProximity_VFTable = nullptr;
void* g_pCTriggerImpact_VFTable = nullptr;
void* g_pCTriggerPlayerMovement_VFTable = nullptr;
void* g_pCTriggerAutoCrouch_VFTable = nullptr;
void* g_pCTriggerLocation_VFTable = nullptr;
void* g_pCTriggerLocationSP_VFTable = nullptr;
void* g_pCTriggerNoGrapple_VFTable = nullptr;

//-----------------------------------------------------------------------------
// Purpose: Checks if an entity inherits from CBaseTrigger
// Input  : *pEntity -
// Output : true if it inhertis from CBaseTrigger, false otherwise
//-----------------------------------------------------------------------------
bool IsTriggerEntity(CBaseEntity* pEntity)
{
	if (pEntity->vftable == g_pCTonemapTrigger_VFTable)
		return true;

	if (pEntity->vftable == g_pCFogTrigger_VFTable)
		return true;

	if (pEntity->vftable == g_pCTriggerSoundScape_VFTable)
		return true;

	if (pEntity->vftable == g_pCTriggerMultiple_VFTable)
		return true;

	if (pEntity->vftable == g_pCTriggerHurt_VFTable)
		return true;

	if (pEntity->vftable == g_pCTriggerCylinder_VFTable)
		return true;

	if (pEntity->vftable == g_pCTriggerPointGravity_VFTable)
		return true;

	if (pEntity->vftable == g_pCTriggerRemove_VFTable)
		return true;

	if (pEntity->vftable == g_pCTriggerOnce_VFTable)
		return true;

	if (pEntity->vftable == g_pCTriggerLook_VFTable)
		return true;

	if (pEntity->vftable == g_pCTriggerPush_VFTable)
		return true;

	if (pEntity->vftable == g_pCTriggerTeleport_VFTable)
		return true;

	if (pEntity->vftable == g_pCTriggerGravity_VFTable)
		return true;

	if (pEntity->vftable == g_pCTriggerProximity_VFTable)
		return true;

	if (pEntity->vftable == g_pCTriggerImpact_VFTable)
		return true;

	if (pEntity->vftable == g_pCTriggerPlayerMovement_VFTable)
		return true;

	if (pEntity->vftable == g_pCTriggerAutoCrouch_VFTable)
		return true;

	if (pEntity->vftable == g_pCTriggerLocation_VFTable)
		return true;

	if (pEntity->vftable == g_pCTriggerLocationSP_VFTable)
		return true;

	if (pEntity->vftable == g_pCTriggerNoGrapple_VFTable)
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Gets the trigger color for a class name
// Input  : *pszClassName -
// Output : The color for the given classname
//-----------------------------------------------------------------------------
Color Trigger_GetColorByClassName(const char* pszClassName)
{
	// 40 is the minimum as we subtract 40 to get a more saturated value for the outline
	Color clrInvalid = Color(255, 255, 0);
	Color clrAmbient = Color(213, 185, 236);
	Color clrGeneral = Color(232, 198, 186);
	Color clrZone = Color(235, 187, 203);
	Color clrCheckpoint = Color(187, 239, 203);
	Color clrFlag = Color(201, 239, 188);
	Color clrFriendly = Color(233, 186, 218);
	Color clrLevelTrans = Color(187, 239, 203);
	Color clrHurt = Color(235, 120, 120);
	Color clrOther = Color(187, 220, 237);

	if (!pszClassName)
		return clrInvalid;

	if (!strcmp(pszClassName, "envmap_volume"))
		return clrAmbient;
	if (!strcmp(pszClassName, "lightprobe_volume"))
		return clrAmbient;
	if (!strcmp(pszClassName, "light_environment_volume"))
		return clrAmbient;
	if (!strcmp(pszClassName, "trigger_indoor_area"))
		return clrAmbient;
	if (!strcmp(pszClassName, "trigger_soundscape"))
		return clrAmbient;

	if (!strcmp(pszClassName, "trigger_capture_point"))
		return clrGeneral;
	if (!strcmp(pszClassName, "trigger_multiple"))
		return clrGeneral;
	if (!strcmp(pszClassName, "trigger_once"))
		return clrGeneral;

	if (!strcmp(pszClassName, "trigger_out_of_bounds"))
		return clrZone;
	if (!strcmp(pszClassName, "trigger_no_grapple"))
		return clrZone;

	if (!strcmp(pszClassName, "trigger_checkpoint"))
		return clrCheckpoint;
	if (!strcmp(pszClassName, "trigger_checkpoint_forced"))
		return clrCheckpoint;
	if (!strcmp(pszClassName, "trigger_checkpoint_safe"))
		return clrCheckpoint;
	if (!strcmp(pszClassName, "trigger_checkpoint_silent"))
		return clrCheckpoint;
	if (!strcmp(pszClassName, "trigger_checkpoint_to_safe_spots"))
		return clrCheckpoint;

	if (!strcmp(pszClassName, "trigger_flag_clear"))
		return clrFlag;
	if (!strcmp(pszClassName, "trigger_flag_set"))
		return clrFlag;
	if (!strcmp(pszClassName, "trigger_flag_touching"))
		return clrFlag;

	if (!strcmp(pszClassName, "trigger_friendly"))
		return clrFriendly;
	if (!strcmp(pszClassName, "trigger_friendly_follow"))
		return clrFriendly;

	if (!strcmp(pszClassName, "trigger_level_transition"))
		return clrLevelTrans;

	if (!strcmp(pszClassName, "trigger_deadly_fog"))
		return clrHurt;
	if (!strcmp(pszClassName, "trigger_death_fall"))
		return clrHurt;
	if (!strcmp(pszClassName, "trigger_hurt"))
		return clrHurt;
	if (!strcmp(pszClassName, "trigger_quickdeath"))
		return clrHurt;
	if (!strcmp(pszClassName, "trigger_quickdeath_checkpoint"))
		return clrHurt;

	if (!strcmp(pszClassName, "trigger_fw_territory"))
		return clrOther;
	if (!strcmp(pszClassName, "trigger_mp_spawn_zone"))
		return clrOther;
	if (!strcmp(pszClassName, "trigger_spawn"))
		return clrOther;
	if (!strcmp(pszClassName, "trigger_teleporter"))
		return clrOther;

	return clrInvalid;
}

//-----------------------------------------------------------------------------
// Purpose: Draws all brush trigger entities using debug draw
//-----------------------------------------------------------------------------
void Triggers_Draw()
{
	Vector3 vCamera;
	QAngle aCamera;
	float fFov;
	g_pClientTools->GetLocalPlayerEyePosition(vCamera, aCamera, fFov);

	for (CBaseEntity* pEntity = CGlobalEntityList__FirstEnt(gEntList, nullptr); pEntity; pEntity = CGlobalEntityList__FirstEnt(gEntList, pEntity))
	{
		if (!pEntity->m_iClassname)
			continue;

		if (!IsTriggerEntity(pEntity))
			continue;

		if (strlen(Cvar_show_triggers_filter->GetString()))
		{
			if (!strstr((const char*)pEntity->m_iClassname, Cvar_show_triggers_filter->GetString()))
				continue;
		}

		const float fDist = Cvar_show_trigger_distance->GetFloat();

		if (pEntity->m_vecAbsOrigin.DistTo(vCamera) > fDist)
			continue;

		const CBaseTrigger* pTrigger = static_cast<const CBaseTrigger*>(pEntity);

		// Loop through all brushes in this trigger
		for (int i = 0; i < pTrigger->m_triggerBrushes.m_Size; i++)
		{
			const CUnknownTriggerBrushRef* pBrush = &pTrigger->m_triggerBrushes.m_Memory.m_pMemory[i];

			// Loop through all planes
			for (int j = 0; j < pBrush->m_vPlanes.m_Size; j++)
			{
				const CUnknownTriggerPlane* pPlane = (CUnknownTriggerPlane*)&pBrush->m_vPlanes.m_Memory.m_Memory[j * sizeof(CUnknownTriggerPlane)];

				if (pPlane->m_vVertices.m_Size < 3)
					continue;

				Color clrFill = Trigger_GetColorByClassName((const char*)pTrigger->m_iClassname);
				clrFill.SetColor(clrFill.r(), clrFill.g(), clrFill.b(), 100);

				Color clrOutline = clrFill;
				clrOutline.SetColor(clrOutline.r() - 40, clrOutline.g() - 40, clrOutline.b() - 40, 255);

				Vector3* v1 = (Vector3*)&pPlane->m_vVertices.m_Memory.m_Memory[0 * sizeof(Vector3)];
				Vector3 p1 = Vector3(v1->x + pTrigger->m_vecAbsOrigin.x, v1->y + pTrigger->m_vecAbsOrigin.y, v1->z + pTrigger->m_vecAbsOrigin.z);

				// Loop through all vertices and render them
				for (int k = 0; k < pPlane->m_vVertices.m_Size - 2; k++)
				{
					Vector3* v2 = (Vector3*)&pPlane->m_vVertices.m_Memory.m_Memory[(k + 1) * sizeof(Vector3)];
					Vector3* v3 = (Vector3*)&pPlane->m_vVertices.m_Memory.m_Memory[(k + 2) * sizeof(Vector3)];

					Vector3 p2 = Vector3(v2->x + pTrigger->m_vecAbsOrigin.x, v2->y + pTrigger->m_vecAbsOrigin.y, v2->z + pTrigger->m_vecAbsOrigin.z);
					Vector3 p3 = Vector3(v3->x + pTrigger->m_vecAbsOrigin.x, v3->y + pTrigger->m_vecAbsOrigin.y, v3->z + pTrigger->m_vecAbsOrigin.z);

					RenderTriangle(p1, p2, p3, clrFill, true);

					RenderLine(p2, p3, clrOutline, true);
				}

				// Render the lines the loop didnt
				Vector3* v2 = (Vector3*)&pPlane->m_vVertices.m_Memory.m_Memory[1 * sizeof(Vector3)];
				Vector3 p2 = Vector3(v2->x + pTrigger->m_vecAbsOrigin.x, v2->y + pTrigger->m_vecAbsOrigin.y, v2->z + pTrigger->m_vecAbsOrigin.z);

				Vector3* v3 = (Vector3*)&pPlane->m_vVertices.m_Memory.m_Memory[(pPlane->m_vVertices.m_Size - 1) * sizeof(Vector3)];
				Vector3 p3 = Vector3(v3->x + pTrigger->m_vecAbsOrigin.x, v3->y + pTrigger->m_vecAbsOrigin.y, v3->z + pTrigger->m_vecAbsOrigin.z);

				RenderLine(p1, p2, clrOutline, true);
				RenderLine(p3, p1, clrOutline, true);
			}
		}
	}
}

/*
// All of these inherit from CBaseTrigger
CTonemapTrigger 0x88D698
CFogTrigger 0x893C48
CTriggerSoundScape 0x8CD1C8
CTriggerMultiple 0x8DCB58
CTriggerHurt 0x8DDB38
CTriggerCylinder 0x8DF2A8
CTriggerPointGravity 0x8DFAA8
CTriggerRemove 0x8E1418
CTriggerOnce 0x8E1F88
CTriggerLook 0x8E27D8
CTriggerPush 0x8E3118
CTriggerTeleport 0x8E3A48
CTriggerGravity 0x8E4358
CTriggerProximity 0x8E6318
CTriggerImpact 0x8E75F8
CTriggerPlayerMovement 0x8E7EE8
CTriggerAutoCrouch 0x8E8758
CTriggerLocation 0x8E8F98
CTriggerLocationSP 0x8E97F8
CTriggerNoGrapple 0x8EA058
*/

ON_DLL_LOAD("server.dll", ServerTriggers, (CModule module))
{
	g_pCTonemapTrigger_VFTable = module.Offset(0x88D698).RCast<void*>();
	g_pCFogTrigger_VFTable = module.Offset(0x893C48).RCast<void*>();
	g_pCTriggerSoundScape_VFTable = module.Offset(0x8CD1C8).RCast<void*>();
	g_pCTriggerMultiple_VFTable = module.Offset(0x8DCB58).RCast<void*>();
	g_pCTriggerHurt_VFTable = module.Offset(0x8DDB38).RCast<void*>();
	g_pCTriggerCylinder_VFTable = module.Offset(0x8DF2A8).RCast<void*>();
	g_pCTriggerPointGravity_VFTable = module.Offset(0x8DFAA8).RCast<void*>();
	g_pCTriggerRemove_VFTable = module.Offset(0x8E1418).RCast<void*>();
	g_pCTriggerOnce_VFTable = module.Offset(0x8E1F88).RCast<void*>();
	g_pCTriggerLook_VFTable = module.Offset(0x8E27D8).RCast<void*>();
	g_pCTriggerPush_VFTable = module.Offset(0x8E3118).RCast<void*>();
	g_pCTriggerTeleport_VFTable = module.Offset(0x8E3A48).RCast<void*>();
	g_pCTriggerGravity_VFTable = module.Offset(0x8E4358).RCast<void*>();
	g_pCTriggerProximity_VFTable = module.Offset(0x8E6318).RCast<void*>();
	g_pCTriggerImpact_VFTable = module.Offset(0x8E75F8).RCast<void*>();
	g_pCTriggerPlayerMovement_VFTable = module.Offset(0x8E7EE8).RCast<void*>();
	g_pCTriggerAutoCrouch_VFTable = module.Offset(0x8E8758).RCast<void*>();
	g_pCTriggerLocation_VFTable = module.Offset(0x8E8F98).RCast<void*>();
	g_pCTriggerLocationSP_VFTable = module.Offset(0x8E97F8).RCast<void*>();
	g_pCTriggerNoGrapple_VFTable = module.Offset(0x8EA058).RCast<void*>();
}
