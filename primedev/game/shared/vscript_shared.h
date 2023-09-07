#pragma once

#include "squirrel/squirrel.h"

template <ScriptContext context>
class CScriptJson
{
  public:
	static void DecodeJson(HSquirrelVM* sqvm, nlohmann::json& jsObj);

	static void EncodeJsonTable(SQTable* pTable, nlohmann::json& jsObj);
	static void EncodeJsonArray(SQArray* pArray, nlohmann::json& jsObj);

  private:
	static void DecodeTable(HSquirrelVM* sqvm, nlohmann::json& jsObj);
	static void DecodeArray(HSquirrelVM* sqvm, nlohmann::json& jsObj);
};
