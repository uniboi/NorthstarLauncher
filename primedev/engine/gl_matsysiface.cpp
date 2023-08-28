#include "materialsystem/cmaterialglue.h"

CMaterialGlue* (*GetMaterialAtCrossHair)();

AUTOHOOK_INIT()

AUTOHOOK(CC_mat_crosshair_printmaterial_f, engine.dll + 0xB3C40, void, __fastcall, (const CCommand& args))
{
	CMaterialGlue* pMat = GetMaterialAtCrossHair();

	if (!pMat)
	{
		Error(eLog::MAT, NO_ERROR, "Not looking at a material!\n");
		return;
	}

	std::function<void(CMaterialGlue * pGlue, const char* szName)> fnPrintGlue = [](CMaterialGlue* pGlue, const char* szName)
	{
		DevMsg(eLog::MAT, "|-----------------------------------------------\n");

		if (!pGlue)
		{
			DevMsg(eLog::MAT, "|-- %s is NULL\n", szName);
			return;
		}

		DevMsg(eLog::MAT, "|-- Name: %s\n", szName);
		DevMsg(eLog::MAT, "|-- GUID: %llX\n", pGlue->m_GUID);
		DevMsg(eLog::MAT, "|-- Name: %s\n", pGlue->m_pszName);
		DevMsg(eLog::MAT, "|-- Width : %d\n", pGlue->m_iWidth);
		DevMsg(eLog::MAT, "|-- Height: %d\n", pGlue->m_iHeight);
	};

	DevMsg(eLog::MAT, "|- GUID: %llX\n", pMat->m_GUID);
	DevMsg(eLog::MAT, "|- Name: %s\n", pMat->m_pszName);
	DevMsg(eLog::MAT, "|- Width : %d\n", pMat->m_iWidth);
	DevMsg(eLog::MAT, "|- Height: %d\n", pMat->m_iHeight);

	fnPrintGlue(pMat->m_pDepthShadow, "DepthShadow");
	fnPrintGlue(pMat->m_pDepthPrepass, "DepthPrepass");
	fnPrintGlue(pMat->m_pDepthVSM, "DepthVSM");
	fnPrintGlue(pMat->m_pColPass, "ColPass");
}

ON_DLL_LOAD("engine.dll", GlMatSysIFace, (CModule module))
{
	AUTOHOOK_DISPATCH()

	GetMaterialAtCrossHair = module.Offset(0xB37D0).RCast<CMaterialGlue* (*)()>();
}
