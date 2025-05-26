#include "common/callbacks.h"

#include "engine/cdll_engine_int.h"
#include "client/localchatwriter.h"
#include "gameui/GameConsole.h"
#include "mods/modmanager.h"
#include "networksystem/bansystem.h"
#include "engine/client/client.h"
#include "engine/server/server.h"
#include "engine/hoststate.h"
#include "originsdk/origin.h"
#include "shared/playlist.h"
#include "engine/datamap.h"
#include "game/server/gameinterface.h"
#include "engine/vengineserver_impl.h"
#include "rtech/datatable.h"
#include "networksystem/atlas.h"
#include "game/server/entitylist.h"
#include "game/server/triggers.h"

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void NS_ServerName_f(ConVar* cvar, const char* pOldValue, float flOldValue)
{
	NOTE_UNUSED(pOldValue);
	NOTE_UNUSED(flOldValue);
	NOTE_UNUSED(cvar);
	// update engine hostname cvar
	Cvar_hostname->SetValue(Cvar_ns_server_name->GetString());
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void NS_ServerDesc_f(ConVar* cvar, const char* pOldValue, float flOldValue)
{
	NOTE_UNUSED(pOldValue);
	NOTE_UNUSED(flOldValue);
	NOTE_UNUSED(cvar);
	//
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void NS_ServerPass_f(ConVar* cvar, const char* pOldValue, float flOldValue)
{
	NOTE_UNUSED(pOldValue);
	NOTE_UNUSED(flOldValue);
	NOTE_UNUSED(cvar);
	//
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_dump_datamap(const CCommand& args)
{
	NOTE_UNUSED(args);
	HMODULE hServer = GetModuleHandleA("server.dll");
	if (!hServer)
	{
		Error(eLog::ENGINE, NO_ERROR, "server.dll not loaded!\n");
	}

	CModule mServer(hServer);

	// CTriggerHurt: 0xB2DE80
	// CPlayer: 0xB86460
	// CFuncBrush: 0xB145C0
	// CBaseEntity: 0xB63070
	// CAI_NetworkManager: 0xB51720
	// CTriggerMultiple: 0xB2E4C0
	// CTriggerBrush: 0xB14650
	// CWeaponX: 0xBAF1B0
	datamap_t* pCAI_BaseNPC = mServer.Offset(0xb3a830).RCast<datamap_t*>();
	datamap_t* pCAI_ChangeTarget = mServer.Offset(0xb2e090).RCast<datamap_t*>();
	datamap_t* pCAI_Combatant = mServer.Offset(0xb490f0).RCast<datamap_t*>();
	datamap_t* pCAI_DynamicLink = mServer.Offset(0xb4a0b0).RCast<datamap_t*>();
	datamap_t* pCAI_DynamicLinkController = mServer.Offset(0xb4a0f0).RCast<datamap_t*>();
	datamap_t* pCAI_Hint = mServer.Offset(0xb4b290).RCast<datamap_t*>();
	datamap_t* pCAI_NetworkManager = mServer.Offset(0xb51720).RCast<datamap_t*>();
	datamap_t* pCAI_RadialLinkController = mServer.Offset(0xb4a130).RCast<datamap_t*>();
	datamap_t* pCAI_SkitNode = mServer.Offset(0xb61d30).RCast<datamap_t*>();
	datamap_t* pCAI_TestHull = mServer.Offset(0xb4c1c0).RCast<datamap_t*>();
	datamap_t* pCAmbientGeneric = mServer.Offset(0xaf0220).RCast<datamap_t*>();
	datamap_t* pCAssaultPoint = mServer.Offset(0xb45c10).RCast<datamap_t*>();

	datamap_t* pCBaseAnimating = mServer.Offset(0xaf1170).RCast<datamap_t*>();
	datamap_t* pCBaseDMStart = mServer.Offset(0xb2a2a0).RCast<datamap_t*>();
	datamap_t* pCBaseEntity = mServer.Offset(0xB63070).RCast<datamap_t*>();
	datamap_t* pCBaseGrenade = mServer.Offset(0xaf9cb0).RCast<datamap_t*>();
	datamap_t* pCBaseTrigger = mServer.Offset(0xb2df50).RCast<datamap_t*>();
	datamap_t* pCBaseViewModel = mServer.Offset(0xafabe0).RCast<datamap_t*>();
	datamap_t* pCBeam = mServer.Offset(0xafb4c0).RCast<datamap_t*>();
	datamap_t* pCBeamSpotlight = mServer.Offset(0xafc3c0).RCast<datamap_t*>();
	datamap_t* pCBoneFollower = mServer.Offset(0xb7a600).RCast<datamap_t*>();
	datamap_t* pCBreakable = mServer.Offset(0xb736f0).RCast<datamap_t*>();
	datamap_t* pCBreakableSurface = mServer.Offset(0xb749b0).RCast<datamap_t*>();

	datamap_t* pCCascadeLight = mServer.Offset(0xb05f00).RCast<datamap_t*>();
	datamap_t* pCColorCorrection = mServer.Offset(0xaff8e0).RCast<datamap_t*>();
	datamap_t* pCConstraintAnchor = mServer.Offset(0xb784c0).RCast<datamap_t*>();
	datamap_t* pCCrossbowBolt = mServer.Offset(0xbacce0).RCast<datamap_t*>();

	datamap_t* pCDropPodProp = mServer.Offset(0xb1d420).RCast<datamap_t*>();
	datamap_t* pCDropPodSpawnPoint = mServer.Offset(0xb2cef0).RCast<datamap_t*>();
	datamap_t* pCDropPoint = mServer.Offset(0xb1b2f0).RCast<datamap_t*>();
	datamap_t* pCDropShipSpawnPoint = mServer.Offset(0xb2cef0).RCast<datamap_t*>();
	datamap_t* pCDynamicLight = mServer.Offset(0xb04a40).RCast<datamap_t*>();
	datamap_t* pCDynamicProp = mServer.Offset(0xb1d4b0).RCast<datamap_t*>();
	datamap_t* pCDynamicPropLightweight = mServer.Offset(0xb1d660).RCast<datamap_t*>();

	datamap_t* pCEnableMotionFixup = mServer.Offset(0xb63070).RCast<datamap_t*>();
	datamap_t* pCEntityBlocker = mServer.Offset(0xb63070).RCast<datamap_t*>();
	datamap_t* pCEntityDissolve = mServer.Offset(0xb6a2b0).RCast<datamap_t*>();
	datamap_t* pCEnvBeam = mServer.Offset(0xb07ff0).RCast<datamap_t*>();
	datamap_t* pCEnvDropZone = mServer.Offset(0xb1b4d0).RCast<datamap_t*>();
	datamap_t* pCEnvExplosion = mServer.Offset(0xb0a200).RCast<datamap_t*>();
	datamap_t* pCEnvLaser = mServer.Offset(0xb08c10).RCast<datamap_t*>();
	datamap_t* pCEnvLight = mServer.Offset(0xb12140).RCast<datamap_t*>();
	datamap_t* pCEnvShake = mServer.Offset(0xb09410).RCast<datamap_t*>();
	datamap_t* pCEnvSoundScape = mServer.Offset(0xb26f60).RCast<datamap_t*>();
	datamap_t* pCEnvSoundScapeProxy = mServer.Offset(0xb26fa0).RCast<datamap_t*>();
	datamap_t* pCEnvSoundScapeTriggerable = mServer.Offset(0xb26fe0).RCast<datamap_t*>();
	datamap_t* pCEnvTilt = mServer.Offset(0xb093d0).RCast<datamap_t*>();
	datamap_t* pCEnvTonemapController = mServer.Offset(0xb06590).RCast<datamap_t*>();
	datamap_t* pCEnvWind = mServer.Offset(0xb05460).RCast<datamap_t*>();

	datamap_t* pCFirstPersonProxy = mServer.Offset(0xba0380).RCast<datamap_t*>();
	datamap_t* pCFogController = mServer.Offset(0xb0af30).RCast<datamap_t*>();
	datamap_t* pCFogTrigger = mServer.Offset(0xb0af70).RCast<datamap_t*>();
	datamap_t* pCFogVolume = mServer.Offset(0xb0c0f0).RCast<datamap_t*>();
	datamap_t* pCFuncBrush = mServer.Offset(0xb145c0).RCast<datamap_t*>();
	datamap_t* pCFuncBrushLightweight = mServer.Offset(0xb14610).RCast<datamap_t*>();
	datamap_t* pCFuncMoveLinear = mServer.Offset(0xb756e0).RCast<datamap_t*>();

	datamap_t* pCGameGibManager = mServer.Offset(0xb20d80).RCast<datamap_t*>();
	datamap_t* pCGameOperator = mServer.Offset(0xb0ca70).RCast<datamap_t*>();
	datamap_t* pCGamePlayerEquip = mServer.Offset(0xb131c0).RCast<datamap_t*>();
	datamap_t* pCGamePlayerTeam = mServer.Offset(0xb13200).RCast<datamap_t*>();
	datamap_t* pCGameRulesProxy = mServer.Offset(0xb63070).RCast<datamap_t*>();
	datamap_t* pCGameText = mServer.Offset(0xb13360).RCast<datamap_t*>();
	datamap_t* pCGameUI = mServer.Offset(0xb0cbd0).RCast<datamap_t*>();
	datamap_t* pCGib = mServer.Offset(0xb0f4d0).RCast<datamap_t*>();
	datamap_t* pCGlobalNonRewinding = mServer.Offset(0xb0fc80).RCast<datamap_t*>();
	datamap_t* pCGrappleHook = mServer.Offset(0xba1430).RCast<datamap_t*>();

	datamap_t* pCHardPointEntity = mServer.Offset(0xb2a2e0).RCast<datamap_t*>();
	datamap_t* pCHardPointFrontierEntity = mServer.Offset(0xb2a2e0).RCast<datamap_t*>();
	datamap_t* pCHealthKit = mServer.Offset(0xb11780).RCast<datamap_t*>();
	datamap_t* pCHumanSizeNPCSpawnPoint = mServer.Offset(0xb2cef0).RCast<datamap_t*>();
	datamap_t* pCHumanSpawnPoint = mServer.Offset(0xb2cef0).RCast<datamap_t*>();

	datamap_t* pCInfoCameraLink = mServer.Offset(0xb10da0).RCast<datamap_t*>();
	datamap_t* pCInfoIntermission = mServer.Offset(0xb63070).RCast<datamap_t*>();
	datamap_t* pCInfoPlacementHelper = mServer.Offset(0xb9a4b0).RCast<datamap_t*>();
	datamap_t* pCInfoTarget = mServer.Offset(0xb63070).RCast<datamap_t*>();
	datamap_t* pcInfoTargetMinimap = mServer.Offset(0xb63070).RCast<datamap_t*>();

	datamap_t* pCKeepUpright = mServer.Offset(0xb76870).RCast<datamap_t*>();

	datamap_t* pCLight = mServer.Offset(0xb12100).RCast<datamap_t*>();
	datamap_t* pCLogicAuto = mServer.Offset(0xb12940).RCast<datamap_t*>();

	datamap_t* pCMessageEntity = mServer.Offset(0xb14080).RCast<datamap_t*>();
	datamap_t* pCMissile = mServer.Offset(0xbad690).RCast<datamap_t*>();
	datamap_t* pCMovementSpeedMod = mServer.Offset(0xb861b0).RCast<datamap_t*>();
	datamap_t* pMovieDisplay = mServer.Offset(0xb15450).RCast<datamap_t*>();

	datamap_t* pCNodeEnt = mServer.Offset(0xb4c140).RCast<datamap_t*>();
	datamap_t* pCNPC_Bullseye = mServer.Offset(0xb76320).RCast<datamap_t*>();
	datamap_t* pCNPC_Drone = mServer.Offset(0xba1ec0).RCast<datamap_t*>();
	datamap_t* pCNPC_Dropship = mServer.Offset(0xba22a0).RCast<datamap_t*>();
	datamap_t* pCNPC_Flyer = mServer.Offset(0xba2920).RCast<datamap_t*>();
	datamap_t* pCPNC_Gunship = mServer.Offset(0xba2be0).RCast<datamap_t*>();
	datamap_t* pCNPC_Marvin = mServer.Offset(0xba30d0).RCast<datamap_t*>();
	datamap_t* pCNPC_MeleeOnly = mServer.Offset(0xba33e0).RCast<datamap_t*>();
	datamap_t* pCNPC_SentryTurret = mServer.Offset(0xba5600).RCast<datamap_t*>();
	datamap_t* pCNPC_Soldier = mServer.Offset(0xba3720).RCast<datamap_t*>();
	datamap_t* pCNPC_Spectre = mServer.Offset(0xb490f0).RCast<datamap_t*>();
	datamap_t* pCNPC_SuperSpectre = mServer.Offset(0xb490f0).RCast<datamap_t*>();
	datamap_t* pCNPC_Titan = mServer.Offset(0xba3d30).RCast<datamap_t*>();
	datamap_t* pCNPC_ProwlerSpawnPoint = mServer.Offset(0xb2cef0).RCast<datamap_t*>();
	datamap_t* pCPNC_NullEntity = mServer.Offset(0xb63070).RCast<datamap_t*>();

	datamap_t* pCParticleSystem = mServer.Offset(0xb16990).RCast<datamap_t*>();
	datamap_t* pCPathCorner = mServer.Offset(0xb178b0).RCast<datamap_t*>();
	datamap_t* pCPathCornerCrash = mServer.Offset(0xb178b0).RCast<datamap_t*>();
	datamap_t* pCPathTrack = mServer.Offset(0xb18430).RCast<datamap_t*>();
	datamap_t* pCPatrolPath = mServer.Offset(0xb63070).RCast<datamap_t*>();
	datamap_t* pCPhysBallSocket = mServer.Offset(0xb78440).RCast<datamap_t*>();
	datamap_t* pCPhysBox = mServer.Offset(0xb7cd50).RCast<datamap_t*>();
	datamap_t* pCPhysConstraintSystem = mServer.Offset(0xb78720).RCast<datamap_t*>();
	datamap_t* pCPhysConvert = mServer.Offset(0xb7cf20).RCast<datamap_t*>();

	datamap_t* pCPhysExplosion = mServer.Offset(0xb7cd90).RCast<datamap_t*>();
	datamap_t* pCPhysFixed = mServer.Offset(0xb78440).RCast<datamap_t*>();
	datamap_t* pCPhysHinge = mServer.Offset(0xb78400).RCast<datamap_t*>();
	datamap_t* pCPhysicsEntitySolver = mServer.Offset(0xb7bb30).RCast<datamap_t*>();
	datamap_t* pCPhysicsNPCSolver = mServer.Offset(0xb7baf0).RCast<datamap_t*>();
	datamap_t* pCPhysicsProp = mServer.Offset(0xb1d460).RCast<datamap_t*>();
	datamap_t* pCPhysicsSpring = mServer.Offset(0xb7ccf0).RCast<datamap_t*>();
	datamap_t* pCPhysImpact = mServer.Offset(0xb7cdd0).RCast<datamap_t*>();
	datamap_t* pCPhysLength = mServer.Offset(0xb785c0).RCast<datamap_t*>();
	datamap_t* pCPhysMotor = mServer.Offset(0xb76970).RCast<datamap_t*>();
	datamap_t* pCPhysPulley = mServer.Offset(0xb78480).RCast<datamap_t*>();
	datamap_t* pCPhysThruster = mServer.Offset(0xb76ad0).RCast<datamap_t*>();
	datamap_t* pCPhysTorque = mServer.Offset(0xb768b0).RCast<datamap_t*>();
	datamap_t* pCPlayer = mServer.Offset(0xb86460).RCast<datamap_t*>();
	datamap_t* pCPlayerDecoy = mServer.Offset(0xb18e40).RCast<datamap_t*>();
	datamap_t* pCPlayerResource = mServer.Offset(0xb94380).RCast<datamap_t*>();
	datamap_t* pCPointBroadcastClientCommand = mServer.Offset(0xafe850).RCast<datamap_t*>();
	datamap_t* pCPointCamera = mServer.Offset(0xb19ab0).RCast<datamap_t*>();
	datamap_t* pCPointClientCommand = mServer.Offset(0xafe6b0).RCast<datamap_t*>();
	datamap_t* pCPointEntity = mServer.Offset(0xb63070).RCast<datamap_t*>();
	datamap_t* pCPointPlayerMoveContraint = mServer.Offset(0xb1a310).RCast<datamap_t*>();
	datamap_t* pCPointServerCommand = mServer.Offset(0xafe810).RCast<datamap_t*>();
	datamap_t* pCPointSpotlight = mServer.Offset(0xb1a8d0).RCast<datamap_t*>();
	datamap_t* pCPointTemplate = mServer.Offset(0xb1b370).RCast<datamap_t*>();
	datamap_t* pCPortal_PointBrush = mServer.Offset(0xb9abd0).RCast<datamap_t*>();
	datamap_t* pCPostProcessController = mServer.Offset(0xb1c850).RCast<datamap_t*>();
	datamap_t* pCPredictedFirstPersonProxy = mServer.Offset(0xba03c0).RCast<datamap_t*>();
	datamap_t* pCPushable = mServer.Offset(0xb736f0).RCast<datamap_t*>();

	datamap_t* pCRevertSaved = mServer.Offset(0xb85fa0).RCast<datamap_t*>();
	datamap_t* pCRopeKeyFrame = mServer.Offset(0x0b7eb90).RCast<datamap_t*>();

	datamap_t* pCScriptNetData_SNDC_GLOBAL = mServer.Offset(0xb226f0).RCast<datamap_t*>();
	datamap_t* pCScriptNetData_SNDC_PLAYER_EXCLUSIVE = mServer.Offset(0xb22600).RCast<datamap_t*>();
	datamap_t* pCScriptNetData_SNDC_PLAYER_GLOBAL = mServer.Offset(0xb22690).RCast<datamap_t*>();
	datamap_t* pCScriptNetData_SNDC_TITAN_SOULS = mServer.Offset(0xb225a0).RCast<datamap_t*>();
	datamap_t* pCScriptNetDataGlobal = mServer.Offset(0xb226f0).RCast<datamap_t*>();
	datamap_t* pCScriptProp = mServer.Offset(0xb1d6a0).RCast<datamap_t*>();
	datamap_t* pCScriptTraceVolume = mServer.Offset(0xbaa470).RCast<datamap_t*>();
	datamap_t* pCScriptSearchPath = mServer.Offset(0xb178f0).RCast<datamap_t*>();
	datamap_t* pCSimplePhysicsBrush = mServer.Offset(0xb63070).RCast<datamap_t*>();
	datamap_t* pCSimplePhysicsProp = mServer.Offset(0xaf1170).RCast<datamap_t*>();
	datamap_t* pCSkyboxSwapper = mServer.Offset(0xb244a0).RCast<datamap_t*>();
	datamap_t* pCSkyCamera = mServer.Offset(0xb24800).RCast<datamap_t*>();
	datamap_t* pCSnapshotCanaryA = mServer.Offset(0xb253d0).RCast<datamap_t*>();
	datamap_t* pCSnapshotCanaryB = mServer.Offset(0xb25430).RCast<datamap_t*>();
	datamap_t* pCSound = mServer.Offset(0xb26750).RCast<datamap_t*>();
	datamap_t* pCSpawner = mServer.Offset(0xb281c0).RCast<datamap_t*>();
	datamap_t* pCSpawnPointFlag = mServer.Offset(0x0b2cef0).RCast<datamap_t*>();
	datamap_t* pCSpotlightEnd = mServer.Offset(0xb285b0).RCast<datamap_t*>();
	datamap_t* pCSprite = mServer.Offset(0xb289f0).RCast<datamap_t*>();
	datamap_t* pCSpriteOriented = mServer.Offset(0xb289f0).RCast<datamap_t*>();
	datamap_t* pCStatusEffectPlugin = mServer.Offset(0xb29b90).RCast<datamap_t*>();

	datamap_t* pCTeam = mServer.Offset(0xb2c2d0).RCast<datamap_t*>();
	datamap_t* pCTeamSpawnPoint = mServer.Offset(0xb2cef0).RCast<datamap_t*>();
	datamap_t* pCTeamVehicleSpawnPoint = mServer.Offset(0xb2cf30).RCast<datamap_t*>();
	datamap_t* pCTempEntTester = mServer.Offset(0xbb32e0).RCast<datamap_t*>();
	datamap_t* pCTitanSoul = mServer.Offset(0xba3cb0).RCast<datamap_t*>();
	datamap_t* pCTitanSpawnPoint = mServer.Offset(0xb2cef0).RCast<datamap_t*>();
	datamap_t* pCToneMapTrigger = mServer.Offset(0xb06550).RCast<datamap_t*>();
	datamap_t* pCTraversalRef = mServer.Offset(0xb4c180).RCast<datamap_t*>();
	datamap_t* pCTriggerAutoCrouch = mServer.Offset(0xb2e190).RCast<datamap_t*>();
	datamap_t* pCTriggerBrush = mServer.Offset(0xb14650).RCast<datamap_t*>();
	datamap_t* pCTriggerCamera = mServer.Offset(0xb2dfd0).RCast<datamap_t*>();
	datamap_t* pCTriggerCylinder = mServer.Offset(0xb2e050).RCast<datamap_t*>();
	datamap_t* pCTriggerGravity = mServer.Offset(0xb2df90).RCast<datamap_t*>();
	datamap_t* pCTriggerHurt = mServer.Offset(0xb2de80).RCast<datamap_t*>();
	datamap_t* pCTriggerImpact = mServer.Offset(0xb2e1d0).RCast<datamap_t*>();
	datamap_t* pCTriggerLocation = mServer.Offset(0xb2e210).RCast<datamap_t*>();
	datamap_t* pCTriggerLocationSP = mServer.Offset(0xb2e210).RCast<datamap_t*>();
	datamap_t* pCTriggerLook = mServer.Offset(0xb2dec0).RCast<datamap_t*>();
	datamap_t* pCTriggerMultiple = mServer.Offset(0xb2e4c0).RCast<datamap_t*>();
	datamap_t* pCTriggerNoGrapple = mServer.Offset(0xb2df50).RCast<datamap_t*>();
	datamap_t* pCTriggerOnce = mServer.Offset(0xb2e4c0).RCast<datamap_t*>();
	datamap_t* pCTriggerPlayerMovement = mServer.Offset(0xb2e480).RCast<datamap_t*>();
	datamap_t* pCTriggerPointGravity = mServer.Offset(0xb2e110).RCast<datamap_t*>();
	datamap_t* pCTriggerProximity = mServer.Offset(0xb2e0d0).RCast<datamap_t*>();
	datamap_t* pCTriggerPush = mServer.Offset(0xb2e150).RCast<datamap_t*>();
	datamap_t* pCTriggerRemove = mServer.Offset(0xb2de40).RCast<datamap_t*>();
	datamap_t* pCTriggerSoundscape = mServer.Offset(0xb27020).RCast<datamap_t*>();
	datamap_t* pCTriggerTeleport = mServer.Offset(0xb2e370).RCast<datamap_t*>();
	datamap_t* pCTriggerViewProxy = mServer.Offset(0xb2e010).RCast<datamap_t*>();
	datamap_t* pCTriggerWind = mServer.Offset(0xb2e550).RCast<datamap_t*>();
	datamap_t* pCTurret = mServer.Offset(0xbab7f0).RCast<datamap_t*>();

	datamap_t* pCVGuiScreen = mServer.Offset(0xb35c70).RCast<datamap_t*>();
	datamap_t* pCVortexSphere = mServer.Offset(0xbac190).RCast<datamap_t*>();
	datamap_t* pCWallrunCurve = mServer.Offset(0xba0c00).RCast<datamap_t*>();
	datamap_t* pCWaterLODControl = mServer.Offset(0xb385b0).RCast<datamap_t*>();
	datamap_t* pCWeaponX = mServer.Offset(0xBAF1B0).RCast<datamap_t*>();
	datamap_t* pCWindowHint = mServer.Offset(0xba0bc0).RCast<datamap_t*>();
	datamap_t* pCWindowPane = mServer.Offset(0x0b74960).RCast<datamap_t*>();
	datamap_t* pCWorld = mServer.Offset(0xb389c0).RCast<datamap_t*>();
	datamap_t* pCWorldItem = mServer.Offset(0xb117c0).RCast<datamap_t*>();

	DataMap_Dump({
		pCVGuiScreen,
		pCVortexSphere,
		pCWallrunCurve,
		pCWaterLODControl,
		pCWeaponX,
		pCWindowHint,
		pCWindowPane,
		pCWorld,
		pCWorldItem,

		pCTeam,
		pCTeamSpawnPoint,
		pCTeamVehicleSpawnPoint,
		pCTempEntTester,
		pCTitanSoul,
		pCTitanSpawnPoint,
		pCToneMapTrigger,
		pCTraversalRef,
		pCTriggerAutoCrouch,
		pCTriggerBrush,
		pCTriggerCamera,
		pCTriggerCylinder,
		pCTriggerGravity,
		pCTriggerHurt,
		pCTriggerImpact,
		pCTriggerLocation,
		pCTriggerLocationSP,
		pCTriggerLook,
		pCTriggerMultiple,
		pCTriggerNoGrapple,
		pCTriggerOnce,
		pCTriggerPlayerMovement,
		pCTriggerPointGravity,
		pCTriggerProximity,
		pCTriggerPush,
		pCTriggerRemove,
		pCTriggerSoundscape,
		pCTriggerTeleport,
		pCTriggerViewProxy,
		pCTriggerWind,
		pCTurret,

		pCScriptNetData_SNDC_GLOBAL,
		pCScriptNetData_SNDC_PLAYER_EXCLUSIVE,
		pCScriptNetData_SNDC_PLAYER_GLOBAL,
		pCScriptNetData_SNDC_TITAN_SOULS,
		pCScriptNetDataGlobal,
		pCScriptProp,
		pCScriptTraceVolume,
		pCScriptSearchPath,
		pCSimplePhysicsBrush,
		pCSimplePhysicsProp,
		pCSkyboxSwapper,
		pCSkyCamera,
		pCSnapshotCanaryA,
		pCSnapshotCanaryB,
		pCSound,
		pCSpawner,
		pCSpawnPointFlag,
		pCSpotlightEnd,
		pCSprite,
		pCSpriteOriented,
		pCStatusEffectPlugin,

		pCRevertSaved,
		pCRopeKeyFrame,

		pCPhysExplosion,
		pCPhysFixed,
		pCPhysHinge,
		pCPhysicsEntitySolver,
		pCPhysicsNPCSolver,
		pCPhysicsProp,
		pCPhysicsSpring,
		pCPhysImpact,
		pCPhysLength,
		pCPhysMotor,
		pCPhysPulley,
		pCPhysThruster,
		pCPhysTorque,
		pCPlayer,
		pCPlayerDecoy,
		pCPlayerResource,
		pCPointBroadcastClientCommand,
		pCPointCamera,
		pCPointClientCommand,
		pCPointEntity,
		pCPointPlayerMoveContraint,
		pCPointServerCommand,
		pCPointSpotlight,
		pCPointTemplate,
		pCPortal_PointBrush,
		pCPostProcessController,
		pCPredictedFirstPersonProxy,
		pCPushable,


		pCParticleSystem,
		pCPathCorner,
		pCPathCornerCrash,
		pCPathTrack,
		pCPatrolPath,
		pCPhysBallSocket,
		pCPhysBox,
		pCPhysConstraintSystem,
		pCPhysConvert,

		pCNodeEnt,
		pCNPC_Bullseye,
		pCNPC_Drone,
		pCNPC_Dropship,
		pCNPC_Flyer,
		pCPNC_Gunship,
		pCNPC_Marvin,
		pCNPC_MeleeOnly,
		pCNPC_SentryTurret,
		pCNPC_Soldier,
		pCNPC_Spectre,
		pCNPC_SuperSpectre,
		pCNPC_Titan,
		pCNPC_ProwlerSpawnPoint,
		pCPNC_NullEntity,

		pCMessageEntity,
		pCMissile,
		pCMovementSpeedMod,
		pMovieDisplay,

		pCLight,
		pCLogicAuto,

		pCKeepUpright,

		pCInfoCameraLink,
		pCInfoIntermission,
		pCInfoPlacementHelper,
		pCInfoTarget,
		pcInfoTargetMinimap,

		pCHardPointEntity,
		pCHardPointFrontierEntity,
		pCHealthKit,
		pCHumanSizeNPCSpawnPoint,
		pCHumanSpawnPoint,

		pCGameGibManager,
		pCGameOperator,
		pCGamePlayerEquip,
		pCGamePlayerTeam,
		pCGameRulesProxy,
		pCGameText,
		pCGameUI,
		pCGib,
		pCGlobalNonRewinding,
		pCGrappleHook,

		pCFirstPersonProxy,
		pCFogController,
		pCFogTrigger,
		pCFogVolume,
		pCFuncBrush,
		pCFuncBrushLightweight,
		pCFuncMoveLinear,

		pCAI_BaseNPC,
		pCAI_ChangeTarget,
		pCAI_Combatant,
		pCAI_DynamicLink,
		pCAI_DynamicLinkController,
		pCAI_Hint,
		pCAI_NetworkManager,
		pCAI_RadialLinkController,
		pCAI_SkitNode,
		pCAI_TestHull,
		pCAmbientGeneric,
		pCAssaultPoint,

		pCBaseAnimating,
		pCBaseDMStart,
		pCBaseEntity,
		pCBaseGrenade,
		pCBaseTrigger,
		pCBaseViewModel,
		pCBaseViewModel,
		pCBeam,
		pCBeamSpotlight,
		pCBoneFollower,
		pCBreakable,
		pCBreakableSurface,

		pCCascadeLight,
		pCColorCorrection,
		pCConstraintAnchor,
		pCCrossbowBolt,

		pCDropPodProp,
		pCDropPodSpawnPoint,
		pCDropPoint,
		pCDropShipSpawnPoint,
		pCDynamicLight,
		pCDynamicProp,
		pCDynamicPropLightweight,

		pCEnableMotionFixup,
		pCEntityBlocker,
		pCEntityDissolve,
		pCEnvBeam,
		pCEnvDropZone,
		pCEnvExplosion,
		pCEnvLaser,
		pCEnvLight,
		pCEnvShake,
		pCEnvSoundScape,
		pCEnvSoundScapeProxy,
		pCEnvSoundScapeTriggerable,
		pCEnvTilt,
		pCEnvTonemapController,
		pCEnvWind,
	});
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_Say_f(const CCommand& args)
{
	if (args.ArgC() >= 2)
		CEngineClient__Saytext(nullptr, args.ArgS(), true, false);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_SayTeam_f(const CCommand& args)
{
	if (args.ArgC() >= 2)
		CEngineClient__Saytext(nullptr, args.ArgS(), true, true);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_Log_f(const CCommand& args)
{
	if (args.ArgC() >= 2)
	{
		LocalChatWriter(LocalChatWriter::GameContext).WriteLine(args.ArgS());
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_ToggleConsole_f(const CCommand& args)
{
	NOTE_UNUSED(args);
	if (g_pGameConsole->IsConsoleVisible())
		g_pGameConsole->Hide();
	else
		g_pGameConsole->Activate();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_ShowConsole_f(const CCommand& args)
{
	NOTE_UNUSED(args);
	g_pGameConsole->Activate();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_HideConsole_f(const CCommand& args)
{
	NOTE_UNUSED(args);
	g_pGameConsole->Hide();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_reload_mods_f(const CCommand& args)
{
	NOTE_UNUSED(args);
	g_pModManager->LoadMods();
}
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_ns_fetchservers_f(const CCommand& args)
{
	NOTE_UNUSED(args);
	//
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_ns_script_servertoclientstringcommand_f(const CCommand& arg)
{
	if (g_pClientVM && g_pClientVM->GetVM())
	{
		ScriptContext nContext = (ScriptContext)g_pClientVM->vmContext;
		HSQUIRRELVM hVM = g_pClientVM->GetVM();
		const char* pszFuncName = "NSClientCodeCallback_RecievedServerToClientStringCommand";

		SQObject oFunction {};
		int nResult = sq_getfunction(hVM, pszFuncName, &oFunction, 0);
		if (nResult != 0)
		{
			Error(VScript_GetNativeLogContext(nContext), NO_ERROR, "Call was unable to find function with name '%s'. Is it global?\n", pszFuncName);
			return;
		}

		// Push
		sq_pushobject(hVM, &oFunction);
		sq_pushroottable(hVM);

		sq_newarray(hVM, 0);

		for (int i = 0; i < arg.ArgC(); i++)
		{
			sq_pushstring(hVM, arg.Arg(i), -1);
			sq_arrayappend(hVM, -2);
		}

		(void)sq_call(hVM, arg.ArgC() + 1, false, false);
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_dump_datatable_f(const CCommand& args)
{
	if (args.ArgC() < 2)
	{
		DevMsg(eLog::NS, "usage: dump_datatable datatable/tablename.rpak\n");
		return;
	}

	DumpDatatable(args.Arg(1));
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_dump_datatables_f(const CCommand& args)
{
	NOTE_UNUSED(args);
	// likely not a comprehensive list, might be missing a couple?
	static const std::vector<const char*> VANILLA_DATATABLE_PATHS = {"datatable/burn_meter_rewards.rpak",
																	 "datatable/burn_meter_store.rpak",
																	 "datatable/calling_cards.rpak",
																	 "datatable/callsign_icons.rpak",
																	 "datatable/camo_skins.rpak",
																	 "datatable/default_pilot_loadouts.rpak",
																	 "datatable/default_titan_loadouts.rpak",
																	 "datatable/faction_leaders.rpak",
																	 "datatable/fd_awards.rpak",
																	 "datatable/features_mp.rpak",
																	 "datatable/non_loadout_weapons.rpak",
																	 "datatable/pilot_abilities.rpak",
																	 "datatable/pilot_executions.rpak",
																	 "datatable/pilot_passives.rpak",
																	 "datatable/pilot_properties.rpak",
																	 "datatable/pilot_weapons.rpak",
																	 "datatable/pilot_weapon_features.rpak",
																	 "datatable/pilot_weapon_mods.rpak",
																	 "datatable/pilot_weapon_mods_common.rpak",
																	 "datatable/playlist_items.rpak",
																	 "datatable/titans_mp.rpak",
																	 "datatable/titan_abilities.rpak",
																	 "datatable/titan_executions.rpak",
																	 "datatable/titan_fd_upgrades.rpak",
																	 "datatable/titan_nose_art.rpak",
																	 "datatable/titan_passives.rpak",
																	 "datatable/titan_primary_mods.rpak",
																	 "datatable/titan_primary_mods_common.rpak",
																	 "datatable/titan_primary_weapons.rpak",
																	 "datatable/titan_properties.rpak",
																	 "datatable/titan_skins.rpak",
																	 "datatable/titan_voices.rpak",
																	 "datatable/unlocks_faction_level.rpak",
																	 "datatable/unlocks_fd_titan_level.rpak",
																	 "datatable/unlocks_player_level.rpak",
																	 "datatable/unlocks_random.rpak",
																	 "datatable/unlocks_titan_level.rpak",
																	 "datatable/unlocks_weapon_level_pilot.rpak",
																	 "datatable/weapon_skins.rpak",
																	 "datatable/xp_per_faction_level.rpak",
																	 "datatable/xp_per_fd_titan_level.rpak",
																	 "datatable/xp_per_player_level.rpak",
																	 "datatable/xp_per_titan_level.rpak",
																	 "datatable/xp_per_weapon_level.rpak",
																	 "datatable/faction_leaders_dropship_anims.rpak",
																	 "datatable/score_events.rpak",
																	 "datatable/startpoints.rpak",
																	 "datatable/sp_levels.rpak",
																	 "datatable/community_entries.rpak",
																	 "datatable/spotlight_images.rpak",
																	 "datatable/death_hints_mp.rpak",
																	 "datatable/flightpath_assets.rpak",
																	 "datatable/earn_meter_mp.rpak",
																	 "datatable/battle_chatter_voices.rpak",
																	 "datatable/battle_chatter.rpak",
																	 "datatable/titan_os_conversations.rpak",
																	 "datatable/faction_dialogue.rpak",
																	 "datatable/grunt_chatter_mp.rpak",
																	 "datatable/spectre_chatter_mp.rpak",
																	 "datatable/pain_death_sounds.rpak",
																	 "datatable/caller_ids_mp.rpak"};

	for (const char* datatable : VANILLA_DATATABLE_PATHS)
		DumpDatatable(datatable);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_ban_f(const CCommand& args)
{
	if (args.ArgC() < 2)
		return;

	for (int i = 0; i < g_pServerGlobalVariables->m_nMaxClients; i++)
	{
		CClient* player = g_pServer->GetClient(i);

		if (!strcmp(player->m_szServerName, args.Arg(1)) || !strcmp(player->m_UID, args.Arg(1)))
		{
			g_pBanSystem->BanUID(strtoull(player->m_UID, nullptr, 10));
			CClient__Disconnect(player, 1, "Banned from server");
			break;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_unban_f(const CCommand& args)
{
	if (args.ArgC() < 2)
		return;

	// assumedly the player being unbanned here wasn't already connected, so don't need to iterate over players or anything
	g_pBanSystem->UnbanUID(strtoull(args.Arg(1), nullptr, 10));
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_clearbanlist_f(const CCommand& args)
{
	NOTE_UNUSED(args);
	g_pBanSystem->ClearBanlist();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_playlist_f(const CCommand& args)
{
	if (args.ArgC() < 2)
		return;

	SetCurrentPlaylist(args.Arg(1));
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_setplaylistvaroverride_f(const CCommand& args)
{
	if (args.ArgC() < 3)
		return;

	for (int i = 1; i < args.ArgC(); i += 2)
		SetPlaylistVarOverride(args.Arg(i), args.Arg(i + 1));
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_script_ui_f(const CCommand& args)
{
	if (g_pUIVM)
		g_pUIVM->ExecuteBuffer(args.ArgS());
	else
		Error(eLog::UI, NO_ERROR, "UI VM is null!\n");
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_script_cl_f(const CCommand& args)
{
	if (g_pClientVM)
		g_pClientVM->ExecuteBuffer(args.ArgS());
	else
		Error(eLog::CLIENT, NO_ERROR, "CLIENT VM is null!\n");
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_script_sv_f(const CCommand& args)
{
	if (g_pServerVM)
		g_pServerVM->ExecuteBuffer(args.ArgS());
	else
		Error(eLog::SERVER, NO_ERROR, "SERVER VM is null!\n");
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_find_f(const CCommand& arg)
{
	if (arg.ArgC() < 2)
	{
		DevMsg(eLog::NS, "Usage: find <string> [<string>...]\n");
		return;
	}

	CCVarIteratorInternal* pIter = g_pCVar->FactoryInternalIterator();

	char szLowerName[256];
	char szLowerTerm[256];

	// Loop through all commandbases
	for (pIter->SetFirst(); pIter->IsValid(); pIter->Next())
	{
		ConCommandBase* pCmdBase = pIter->Get();
		bool bFound = false;

		// Loop through all passed arguments
		for (int i = 0; i < arg.ArgC() - 1; i++)
		{
			strncpy_s(szLowerName, sizeof(szLowerName), pCmdBase->m_pszName, sizeof(szLowerName) - 1);
			strncpy_s(szLowerTerm, sizeof(szLowerTerm), arg.Arg(i + 1), sizeof(szLowerTerm) - 1);

			for (int i = 0; szLowerName[i]; i++)
				szLowerName[i] = (char)tolower(szLowerName[i]);

			for (int i = 0; szLowerTerm[i]; i++)
				szLowerTerm[i] = (char)tolower(szLowerTerm[i]);

			if (strstr(szLowerName, szLowerTerm))
			{
				bFound = true;
				break;
			}
		}

		if (bFound)
		{
			g_pCVar->PrintHelpString(pCmdBase->m_pszName);
		}
	}

	delete pIter;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_help_f(const CCommand& arg)
{
	if (arg.ArgC() < 2)
	{
		DevMsg(eLog::NS, "Usage: help <string>\n");
		return;
	}

	g_pCVar->PrintHelpString(arg.Arg(1));
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_printcvaraddr_f(const CCommand& arg)
{
	if (arg.ArgC() < 2)
	{
		DevMsg(eLog::NS, "Usage: cvar_printaddr <string>\n");
		return;
	}

	const ConCommandBase* pCmdBase = g_pCVar->FindCommandBase(arg.Arg(1));

	if (!pCmdBase)
	{
		Error(eLog::NS, NO_ERROR, "'%s' doesn't exist!\n", arg.Arg(1));
		return;
	}

	LogPtrAdr(pCmdBase->m_pszName, (void*)pCmdBase);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_maps_f(const CCommand& args)
{
	if (args.ArgC() < 2)
	{
		DevMsg(eLog::NS, "Usage: maps <substring>\n");
		DevMsg(eLog::NS, "maps * for full listing\n");
		return;
	}

	for (MapVPKInfo_t& map : g_pModManager->m_vMapList)
	{
		if ((*args.Arg(1) == '*' && !args.Arg(1)[1]) || strstr(map.svName.c_str(), args.Arg(1)))
		{
			DevMsg(eLog::NS, "(%i) %s\n", map.eSource, map.svName.c_str());
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_CreateFakePlayer_f(const CCommand& args)
{
	if (args.ArgC() < 3)
	{
		DevMsg(eLog::NS, "Usage: sv_addbot <name> <team>\n");
		return;
	}

	// TODO [Fifty]: Check teams to not script error

	const char* szName = args.Arg(1);
	int nTeam = std::atoi(args.Arg(2));

	g_pEngineServer->LockNetworkStringTables(true);

	edict_t nHandle = g_pEngineServer->CreateFakeClient(szName, "", "", nTeam);
	g_pServerGameClients->ClientFullyConnect(nHandle, false);

	g_pEngineServer->LockNetworkStringTables(false);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_DumpTriggersInMap_f(const CCommand& args)
{
	NOTE_UNUSED(args);
	DevMsg(eLog::ENGINE, "Dumping all triggers in this level: -----------------------------------\n");
	std::unordered_set<std::string> uNames;
	for (CBaseEntity* pEntity = CGlobalEntityList__FirstEnt(gEntList, nullptr); pEntity; pEntity = CGlobalEntityList__FirstEnt(gEntList, pEntity))
	{
		if (!IsTriggerEntity(pEntity))
			continue;

		if (!pEntity->m_iClassname)
			continue;

		uNames.insert(std::string((const char*)pEntity->m_iClassname));
	}

	for (std::string svName : uNames)
	{
		DevMsg(eLog::ENGINE, "%s\n", svName.c_str());
	}

	DevMsg(eLog::ENGINE, "Finished! -------------------------------------------------------------\n");
}
