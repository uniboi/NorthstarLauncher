#include "vscript/languages/squirrel_re/include/squirrel.h"

#include "vscript/languages/squirrel_re/squirrel/sqvm.h"
#include "vscript/languages/squirrel_re/squirrel/sqstate.h"
#include "vscript/languages/squirrel_re/vsquirrel.h"

#include "mods/modmanager.h"

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
const SQChar* sq_getstring(HSQUIRRELVM sqvm, SQInteger iStackpos)
{
	Assert(sqvm->_stackOfCurrentFunction[iStackpos]._Type == OT_STRING);

	// FIXME [Fifty]: Reimplement this
	if (sqvm->sharedState->cSquirrelVM->vmContext == (int)ScriptContext::SERVER)
		return o_sq_getstringServer(sqvm, iStackpos);
	else
		return o_sq_getstringClient(sqvm, iStackpos);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
SQInteger sq_getinteger(HSQUIRRELVM sqvm, SQInteger iStackpos)
{
	Assert(sqvm->_stackOfCurrentFunction[iStackpos]._Type == OT_INTEGER);

	// FIXME [Fifty]: Reimplement this
	if (sqvm->sharedState->cSquirrelVM->vmContext == (int)ScriptContext::SERVER)
		return o_sq_getintegerServer(sqvm, iStackpos);
	else
		return o_sq_getintegerClient(sqvm, iStackpos);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
SQFloat sq_getfloat(HSQUIRRELVM sqvm, SQInteger iStackpos)
{
	Assert(sqvm->_stackOfCurrentFunction[iStackpos]._Type == OT_FLOAT);

	// FIXME [Fifty]: Reimplement this
	if (sqvm->sharedState->cSquirrelVM->vmContext == (int)ScriptContext::SERVER)
		return o_sq_getfloatServer(sqvm, iStackpos);
	else
		return o_sq_getfloatClient(sqvm, iStackpos);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
SQBool sq_getbool(HSQUIRRELVM sqvm, SQInteger iStackpos)
{
	Assert(sqvm->_stackOfCurrentFunction[iStackpos]._Type == OT_BOOL);

	// FIXME [Fifty]: Reimplement this
	if (sqvm->sharedState->cSquirrelVM->vmContext == (int)ScriptContext::SERVER)
		return o_sq_getboolServer(sqvm, iStackpos);
	else
		return o_sq_getboolClient(sqvm, iStackpos);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
SQRESULT sq_get(HSQUIRRELVM sqvm, SQInteger iStackpos)
{
	// FIXME [Fifty]: Reimplement this
	if (sqvm->sharedState->cSquirrelVM->vmContext == (int)ScriptContext::SERVER)
		return o_sq_getServer(sqvm, iStackpos);
	else
		return o_sq_getClient(sqvm, iStackpos);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
SQRESULT sq_getasset(HSQUIRRELVM sqvm, SQInteger iStackpos, const char** pResult)
{
	Assert(sqvm->_stackOfCurrentFunction[iStackpos]._Type == OT_ASSET);

	// FIXME [Fifty]: Reimplement this
	if (sqvm->sharedState->cSquirrelVM->vmContext == (int)ScriptContext::SERVER)
		return o_sq_getassetServer(sqvm, iStackpos, pResult);
	else
		return o_sq_getassetClient(sqvm, iStackpos, pResult);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
SQRESULT sq_getuserdata(HSQUIRRELVM sqvm, SQInteger iStackpos, void** pData, uint64_t* pId)
{
	Assert(sqvm->_stackOfCurrentFunction[iStackpos]._Type == OT_USERDATA);

	// FIXME [Fifty]: Reimplement this
	if (sqvm->sharedState->cSquirrelVM->vmContext == (int)ScriptContext::SERVER)
		return o_sq_getuserdataServer(sqvm, iStackpos, pData, pId);
	else
		return o_sq_getuserdataClient(sqvm, iStackpos, pData, pId);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
SQFloat* sq_getvector(HSQUIRRELVM sqvm, SQInteger iStackpos)
{
	Assert(sqvm->_stackOfCurrentFunction[iStackpos]._Type == OT_VECTOR);

	// FIXME [Fifty]: Reimplement this
	if (sqvm->sharedState->cSquirrelVM->vmContext == (int)ScriptContext::SERVER)
		return o_sq_getvectorServer(sqvm, iStackpos);
	else
		return o_sq_getvectorClient(sqvm, iStackpos);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
SQBool sq_getthisentity(HSQUIRRELVM sqvm, void** ppEntity)
{
	Assert(sqvm->_stackOfCurrentFunction->_Type == 0x408000);

	// FIXME [Fifty]: Reimplement this
	if (sqvm->sharedState->cSquirrelVM->vmContext == (int)ScriptContext::SERVER)
		return o_sq_getthisentityServer(sqvm, ppEntity);
	else
		return o_sq_getthisentityClient(sqvm, ppEntity);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void sq_getobject(HSQUIRRELVM sqvm, SQInteger iStackpos, SQObject* pOutObj)
{
	// FIXME [Fifty]: Reimplement this
	if (sqvm->sharedState->cSquirrelVM->vmContext == (int)ScriptContext::SERVER)
		return o_sq_getobjectServer(sqvm, iStackpos, pOutObj);
	else
		return o_sq_getobjectClient(sqvm, iStackpos, pOutObj);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int sq_getfunction(HSQUIRRELVM sqvm, const char* pszName, SQObject* pReturnObj, const char* pszSignature)
{
	// FIXME [Fifty]: Reimplement this
	if (sqvm->sharedState->cSquirrelVM->vmContext == (int)ScriptContext::SERVER)
		return o_sq_getfunctionServer(sqvm, pszName, pReturnObj, pszSignature);
	else
		return o_sq_getfunctionClient(sqvm, pszName, pReturnObj, pszSignature);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void sq_pushroottable(HSQUIRRELVM sqvm)
{
	// FIXME [Fifty]: Reimplement this
	if (sqvm->sharedState->cSquirrelVM->vmContext == (int)ScriptContext::SERVER)
		o_sq_pushroottableServer(sqvm);
	else
		o_sq_pushroottableClient(sqvm);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void sq_pushstring(HSQUIRRELVM sqvm, const SQChar* pStr, SQInteger iLength)
{
	// FIXME [Fifty]: Reimplement this
	if (sqvm->sharedState->cSquirrelVM->vmContext == (int)ScriptContext::SERVER)
		o_sq_pushstringServer(sqvm, pStr, iLength);
	else
		o_sq_pushstringClient(sqvm, pStr, iLength);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void sq_pushinteger(HSQUIRRELVM sqvm, SQInteger i)
{
	// FIXME [Fifty]: Reimplement this
	if (sqvm->sharedState->cSquirrelVM->vmContext == (int)ScriptContext::SERVER)
		o_sq_pushintegerServer(sqvm, i);
	else
		o_sq_pushintegerClient(sqvm, i);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void sq_pushfloat(HSQUIRRELVM sqvm, SQFloat f)
{
	// FIXME [Fifty]: Reimplement this
	if (sqvm->sharedState->cSquirrelVM->vmContext == (int)ScriptContext::SERVER)
		o_sq_pushfloatServer(sqvm, f);
	else
		o_sq_pushfloatClient(sqvm, f);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void sq_pushbool(HSQUIRRELVM sqvm, SQBool b)
{
	// FIXME [Fifty]: Reimplement this
	if (sqvm->sharedState->cSquirrelVM->vmContext == (int)ScriptContext::SERVER)
		o_sq_pushboolServer(sqvm, b);
	else
		o_sq_pushboolClient(sqvm, b);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void sq_pushasset(HSQUIRRELVM sqvm, const SQChar* str, SQInteger iLength)
{
	// FIXME [Fifty]: Reimplement this
	if (sqvm->sharedState->cSquirrelVM->vmContext == (int)ScriptContext::SERVER)
		o_sq_pushassetServer(sqvm, str, iLength);
	else
		o_sq_pushassetClient(sqvm, str, iLength);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void sq_pushvector(HSQUIRRELVM sqvm, const SQFloat* pVec)
{
	// FIXME [Fifty]: Reimplement this
	if (sqvm->sharedState->cSquirrelVM->vmContext == (int)ScriptContext::SERVER)
		o_sq_pushvectorServer(sqvm, pVec);
	else
		o_sq_pushvectorClient(sqvm, pVec);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void sq_pushobject(HSQUIRRELVM sqvm, SQObject* pObj)
{
	// FIXME [Fifty]: Reimplement this
	if (sqvm->sharedState->cSquirrelVM->vmContext == (int)ScriptContext::SERVER)
		o_sq_pushobjectServer(sqvm, pObj);
	else
		o_sq_pushobjectClient(sqvm, pObj);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void sq_newarray(HSQUIRRELVM sqvm, SQInteger iStackpos)
{
	// FIXME [Fifty]: Reimplement this
	if (sqvm->sharedState->cSquirrelVM->vmContext == (int)ScriptContext::SERVER)
		return o_sq_newarrayServer(sqvm, iStackpos);
	else
		return o_sq_newarrayClient(sqvm, iStackpos);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
SQRESULT sq_arrayappend(HSQUIRRELVM sqvm, SQInteger iStackpos)
{
	// FIXME [Fifty]: Reimplement this
	if (sqvm->sharedState->cSquirrelVM->vmContext == (int)ScriptContext::SERVER)
		return o_sq_arrayappendServer(sqvm, iStackpos);
	else
		return o_sq_arrayappendClient(sqvm, iStackpos);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
SQRESULT sq_newtable(HSQUIRRELVM sqvm)
{
	// FIXME [Fifty]: Reimplement this
	if (sqvm->sharedState->cSquirrelVM->vmContext == (int)ScriptContext::SERVER)
		return o_sq_newtableServer(sqvm);
	else
		return o_sq_newtableClient(sqvm);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
SQRESULT sq_newslot(HSQUIRRELVM sqvm, SQInteger idx, SQBool bStatic)
{
	// FIXME [Fifty]: Reimplement this
	if (sqvm->sharedState->cSquirrelVM->vmContext == (int)ScriptContext::SERVER)
		return o_sq_newslotServer(sqvm, idx, bStatic);
	else
		return o_sq_newslotClient(sqvm, idx, bStatic);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
SQRESULT sq_pushnewstructinstance(HSQUIRRELVM sqvm, int fieldCount)
{
	// FIXME [Fifty]: Reimplement this
	if (sqvm->sharedState->cSquirrelVM->vmContext == (int)ScriptContext::SERVER)
		return o_sq_pushnewstructinstanceServer(sqvm, fieldCount);
	else
		return o_sq_pushnewstructinstanceClient(sqvm, fieldCount);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
SQRESULT sq_sealstructslot(HSQUIRRELVM sqvm, int slotIndex)
{
	// FIXME [Fifty]: Reimplement this
	if (sqvm->sharedState->cSquirrelVM->vmContext == (int)ScriptContext::SERVER)
		return o_sq_sealstructslotServer(sqvm, slotIndex);
	else
		return o_sq_sealstructslotClient(sqvm, slotIndex);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void* sq_createuserdata(HSQUIRRELVM sqvm, SQInteger iSize)
{
	void* pData;
	// FIXME [Fifty]: Reimplement this
	if (sqvm->sharedState->cSquirrelVM->vmContext == (int)ScriptContext::SERVER)
		pData = o_sq_createuserdataServer(sqvm, iSize);
	else
		pData = o_sq_createuserdataClient(sqvm, iSize);

	memset(pData, 0, iSize);
	return pData;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
SQRESULT sq_setuserdatatypeid(HSQUIRRELVM sqvm, SQInteger iStackpos, uint64_t iTypeId)
{
	// FIXME [Fifty]: Reimplement this
	if (sqvm->sharedState->cSquirrelVM->vmContext == (int)ScriptContext::SERVER)
		return o_sq_setuserdatatypeidServer(sqvm, iStackpos, iTypeId);
	else
		return o_sq_setuserdatatypeidClient(sqvm, iStackpos, iTypeId);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
long long sq_stackinfos(HSQUIRRELVM sqvm, int iLevel, SQStackInfos* pOutObj)
{
	// FIXME [Fifty]: Reimplement this
	if (sqvm->sharedState->cSquirrelVM->vmContext == (int)ScriptContext::SERVER)
		return o_sq_stackinfosServer(sqvm, iLevel, pOutObj, sqvm->_callstacksize);
	else
		return o_sq_stackinfosClient(sqvm, iLevel, pOutObj, sqvm->_callstacksize);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
SQRESULT sq_compilebuffer(HSQUIRRELVM sqvm, SQBufferState* compileBuffer, const char* file, int a1, SQBool bShouldThrowError)
{
	// FIXME [Fifty]: Reimplement this
	if (sqvm->sharedState->cSquirrelVM->vmContext == (int)ScriptContext::SERVER)
		return o_sq_compilebufferServer(sqvm, compileBuffer, file, a1, bShouldThrowError);
	else
		return o_sq_compilebufferClient(sqvm, compileBuffer, file, a1, bShouldThrowError);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
SQRESULT sq_call(HSQUIRRELVM sqvm, SQInteger iArgs, SQBool bShouldReturn, SQBool bThrowError)
{
	// FIXME [Fifty]: Reimplement this
	if (sqvm->sharedState->cSquirrelVM->vmContext == (int)ScriptContext::SERVER)
		return o_sq_callServer(sqvm, iArgs, bShouldReturn, bThrowError);
	else
		return o_sq_callClient(sqvm, iArgs, bShouldReturn, bThrowError);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
SQInteger sq_raiseerror(HSQUIRRELVM sqvm, const SQChar* pError)
{
	// FIXME [Fifty]: Reimplement this
	if (sqvm->sharedState->cSquirrelVM->vmContext == (int)ScriptContext::SERVER)
		return o_sq_raiseerrorServer(sqvm, pError);
	else
		return o_sq_raiseerrorClient(sqvm, pError);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
Mod* sq_getcallingmod(HSQUIRRELVM sqvm, int depth)
{
	if (1 + depth >= sqvm->_callstacksize)
		return nullptr;

	SQStackInfos stackInfo {};
	sq_stackinfos(sqvm, 1 + depth, &stackInfo);

	std::string sourceName = stackInfo._sourceName;
	std::replace(sourceName.begin(), sourceName.end(), '/', '\\');
	std::string filename = g_pModManager->NormaliseModFilePath(fs::path("scripts\\vscripts\\" + sourceName));

	if (auto res = g_pModManager->m_ModFiles.find(filename); res != g_pModManager->m_ModFiles.end())
	{
		return res->second.m_pOwningMod;
	}

	return nullptr;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CBaseEntity* sq_getentity(HSQUIRRELVM sqvm, SQInteger iStackPos)
{
	SQObject obj;
	sq_getobject(sqvm, iStackPos, &obj);

	CBaseEntity* pEntity = (CBaseEntity*)sqvm->sharedState->cSquirrelVM->GetEntityFromObject(&obj);

	return pEntity;
}

ON_DLL_LOAD("client.dll", SQAPIClient, (CModule module))
{
	o_sq_getstringClient = module.Offset(0x60C0).RCast<const SQChar* (*)(HSQUIRRELVM, SQInteger)>();
	o_sq_getintegerClient = module.Offset(0x60E0).RCast<SQInteger (*)(HSQUIRRELVM, SQInteger)>();
	o_sq_getfloatClient = module.Offset(0x6100).RCast<SQFloat (*)(HSQUIRRELVM, SQInteger)>();
	o_sq_getboolClient = module.Offset(0x6130).RCast<SQBool (*)(HSQUIRRELVM, SQInteger)>();
	o_sq_getClient = module.Offset(0x7C30).RCast<SQRESULT (*)(HSQUIRRELVM, SQInteger)>();
	o_sq_getassetClient = module.Offset(0x6010).RCast<SQRESULT (*)(HSQUIRRELVM, SQInteger, const char**)>();
	o_sq_getuserdataClient = module.Offset(0x63D0).RCast<SQRESULT (*)(HSQUIRRELVM, SQInteger, void**, uint64_t*)>();
	o_sq_getvectorClient = module.Offset(0x6140).RCast<SQFloat* (*)(HSQUIRRELVM, SQInteger)>();
	o_sq_getthisentityClient = module.Offset(0x12F80).RCast<SQBool (*)(HSQUIRRELVM, void**)>();
	o_sq_getobjectClient = module.Offset(0x6160).RCast<void (*)(HSQUIRRELVM, SQInteger, SQObject*)>();
	o_sq_getfunctionClient = module.Offset(0x6CB0).RCast<int (*)(HSQUIRRELVM, const char*, SQObject*, const char*)>();

	o_sq_pushroottableClient = module.Offset(0x5860).RCast<void (*)(HSQUIRRELVM)>();
	o_sq_pushstringClient = module.Offset(0x3440).RCast<void (*)(HSQUIRRELVM, const SQChar*, SQInteger)>();
	o_sq_pushintegerClient = module.Offset(0x36A0).RCast<void (*)(HSQUIRRELVM, SQInteger)>();
	o_sq_pushfloatClient = module.Offset(0x3800).RCast<void (*)(HSQUIRRELVM, SQFloat)>();
	o_sq_pushboolClient = module.Offset(0x3710).RCast<void (*)(HSQUIRRELVM, SQBool)>();
	o_sq_pushassetClient = module.Offset(0x3560).RCast<void (*)(HSQUIRRELVM, const SQChar*, SQInteger)>();
	o_sq_pushvectorClient = module.Offset(0x3780).RCast<void (*)(HSQUIRRELVM, const SQFloat*)>();
	o_sq_pushobjectClient = module.Offset(0x83D0).RCast<void (*)(HSQUIRRELVM, SQObject*)>();

	o_sq_newarrayClient = module.Offset(0x39F0).RCast<void (*)(HSQUIRRELVM, SQInteger)>();
	o_sq_arrayappendClient = module.Offset(0x3C70).RCast<SQRESULT (*)(HSQUIRRELVM, SQInteger)>();

	o_sq_newtableClient = module.Offset(0x3960).RCast<SQRESULT (*)(HSQUIRRELVM)>();
	o_sq_newslotClient = module.Offset(0x70B0).RCast<SQRESULT (*)(HSQUIRRELVM, SQInteger, SQBool)>();

	o_sq_pushnewstructinstanceClient = module.Offset(0x5400).RCast<SQRESULT (*)(HSQUIRRELVM, int)>();
	o_sq_sealstructslotClient = module.Offset(0x5530).RCast<SQRESULT (*)(HSQUIRRELVM, int)>();

	o_sq_createuserdataClient = module.Offset(0x38D0).RCast<void* (*)(HSQUIRRELVM, SQInteger)>();
	o_sq_setuserdatatypeidClient = module.Offset(0x6490).RCast<SQRESULT (*)(HSQUIRRELVM, SQInteger, uint64_t)>();

	o_sq_stackinfosClient = module.Offset(0x35970).RCast<long long (*)(HSQUIRRELVM, int, SQStackInfos*, int)>();

	o_sq_compilebufferClient = module.Offset(0x3110).RCast<SQRESULT (*)(HSQUIRRELVM, SQBufferState*, const char*, int, SQBool)>();
	o_sq_callClient = module.Offset(0x8650).RCast<SQRESULT (*)(HSQUIRRELVM, SQInteger, SQBool, SQBool)>();
	o_sq_raiseerrorClient = module.Offset(0x8470).RCast<SQInteger (*)(HSQUIRRELVM, const SQChar*)>();
}

ON_DLL_LOAD("server.dll", SQAPIServer, (CModule module))
{
	o_sq_getstringServer = module.Offset(0x60A0).RCast<const SQChar* (*)(HSQUIRRELVM, SQInteger)>();
	o_sq_getintegerServer = module.Offset(0x60C0).RCast<SQInteger (*)(HSQUIRRELVM, SQInteger)>();
	o_sq_getfloatServer = module.Offset(0x60E0).RCast<SQFloat (*)(HSQUIRRELVM, SQInteger)>();
	o_sq_getboolServer = module.Offset(0x6110).RCast<SQBool (*)(HSQUIRRELVM, SQInteger)>();
	o_sq_getServer = module.Offset(0x7C00).RCast<SQRESULT (*)(HSQUIRRELVM, SQInteger)>();
	o_sq_getassetServer = module.Offset(0x5FF0).RCast<SQRESULT (*)(HSQUIRRELVM, SQInteger, const char**)>();
	o_sq_getuserdataServer = module.Offset(0x63B0).RCast<SQRESULT (*)(HSQUIRRELVM, SQInteger, void**, uint64_t*)>();
	o_sq_getvectorServer = module.Offset(0x6120).RCast<SQFloat* (*)(HSQUIRRELVM, SQInteger)>();
	o_sq_getthisentityServer = module.Offset(0x203B0).RCast<SQBool (*)(HSQUIRRELVM, void**)>();
	o_sq_getobjectServer = module.Offset(0x6140).RCast<void (*)(HSQUIRRELVM, SQInteger, SQObject*)>();
	o_sq_getfunctionServer = module.Offset(0x6C80).RCast<int (*)(HSQUIRRELVM, const char*, SQObject*, const char*)>();

	o_sq_pushroottableServer = module.Offset(0x5840).RCast<void (*)(HSQUIRRELVM)>();
	o_sq_pushstringServer = module.Offset(0x3440).RCast<void (*)(HSQUIRRELVM, const SQChar*, SQInteger)>();
	o_sq_pushintegerServer = module.Offset(0x36A0).RCast<void (*)(HSQUIRRELVM, SQInteger)>();
	o_sq_pushfloatServer = module.Offset(0x3800).RCast<void (*)(HSQUIRRELVM, SQFloat)>();
	o_sq_pushboolServer = module.Offset(0x3710).RCast<void (*)(HSQUIRRELVM, SQBool)>();
	o_sq_pushassetServer = module.Offset(0x3560).RCast<void (*)(HSQUIRRELVM, const SQChar*, SQInteger)>();
	o_sq_pushvectorServer = module.Offset(0x3780).RCast<void (*)(HSQUIRRELVM, const SQFloat*)>();
	o_sq_pushobjectServer = module.Offset(0x83A0).RCast<void (*)(HSQUIRRELVM, SQObject*)>();

	o_sq_newarrayServer = module.Offset(0x39F0).RCast<void (*)(HSQUIRRELVM, SQInteger)>();
	o_sq_arrayappendServer = module.Offset(0x3C70).RCast<SQRESULT (*)(HSQUIRRELVM, SQInteger)>();

	o_sq_newtableServer = module.Offset(0x3960).RCast<SQRESULT (*)(HSQUIRRELVM)>();
	o_sq_newslotServer = module.Offset(0x7080).RCast<SQRESULT (*)(HSQUIRRELVM, SQInteger, SQBool)>();

	o_sq_pushnewstructinstanceServer = module.Offset(0x53e0).RCast<SQRESULT (*)(HSQUIRRELVM, int)>();
	o_sq_sealstructslotServer = module.Offset(0x5510).RCast<SQRESULT (*)(HSQUIRRELVM, int)>();

	o_sq_createuserdataServer = module.Offset(0x38D0).RCast<void* (*)(HSQUIRRELVM, SQInteger)>();
	o_sq_setuserdatatypeidServer = module.Offset(0x6470).RCast<SQRESULT (*)(HSQUIRRELVM, SQInteger, uint64_t)>();

	o_sq_stackinfosServer = module.Offset(0x35920).RCast<long long (*)(HSQUIRRELVM, int, SQStackInfos*, int)>();

	o_sq_compilebufferServer = module.Offset(0x3110).RCast<SQRESULT (*)(HSQUIRRELVM, SQBufferState*, const char*, int, SQBool)>();
	o_sq_callServer = module.Offset(0x8620).RCast<SQRESULT (*)(HSQUIRRELVM, SQInteger, SQBool, SQBool)>();
	o_sq_raiseerrorServer = module.Offset(0x8440).RCast<SQInteger (*)(HSQUIRRELVM, const SQChar*)>();
}
