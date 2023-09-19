#include "materialsystem/cmaterialglue.h"

CMaterialGlue* (*GetMaterialAtCrossHair)();

void (*o_CC_mat_crosshair_printmaterial_f)(const CCommand& args);

void h_CC_mat_crosshair_printmaterial_f(const CCommand& args)
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
	o_CC_mat_crosshair_printmaterial_f = module.Offset(0xB3C40).RCast<void (*)(const CCommand&)>();
	HookAttach(&(PVOID&)o_CC_mat_crosshair_printmaterial_f, (PVOID)h_CC_mat_crosshair_printmaterial_f);

	GetMaterialAtCrossHair = module.Offset(0xB37D0).RCast<CMaterialGlue* (*)()>();
}
