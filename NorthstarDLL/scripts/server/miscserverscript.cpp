#include "squirrel/squirrel.h"
#include "masterserver/masterserver.h"
#include "server/auth/serverauthentication.h"
#include "dedicated/dedicated.h"
#include "client/r2client.h"
#include "server/r2server.h"

#include <fstream>
#include <iostream>
#include <string.h>
#include <string>
#include "pch.h"

#include <cstdint>

// ========================================
// > ANIM TEST
// ========================================

AUTOHOOK_INIT()

VAR_AT(server.dll + 0xbce630, int, g_CreatedRecordedAnimCount);
FUNCTION_AT(server.dll + 0x996e0, RecordedAnimation*, __fastcall, CreateNewRecordedAnimation, (void))
FUNCTION_AT(server.dll + 0x99270, long long, __fastcall, InsertAnimInLoadedList, (RecordedAnimation * anim))
FUNCTION_AT(server.dll + 0x99c50, void, __fastcall, sq_PushRecordedAnimationFromData, (HSquirrelVM * sqvm, RecordedAnimation* anim))

constexpr int BINARY_FORMAT_VERSION = 1;

constexpr size_t SEQUENCES_1_LEN = 15;
constexpr size_t UNKNOWN_120_LEN = 15;
constexpr size_t SEQUENCES_LEN = 47;
constexpr size_t UNKNOWN_616_LEN = 17;
constexpr size_t FRAMES_LEN = 1;
constexpr size_t LAYERS_LEN = 3000;

void serializeStrings(std::ofstream* stream, char** data, const size_t arrLen)
{
	for (size_t i = 0; i < arrLen; i++)
	{
		if (data[i])
			stream->write(data[i], strlen(data[i]) + 1);
		else
			stream->write("\0", 1);
	}
}

void serializeBlob(std::ofstream* stream, void* data, const size_t blobLen)
{
	stream->write((char*)data, blobLen);
}

void deserializeStrings(std::ifstream* stream, char** data, const size_t arrLen)
{
	for (size_t i = 0; i < arrLen; i++)
	{
		char c;
		std::string sequence;

		while (*stream >> c)
		{
			if (c == '\0')
			{
				size_t len = sequence.length();
				if (len > 0)
				{
					char* p = new char[len + 1];
					strcpy_s(p, len + 1, sequence.c_str());
					data[i] = p;
				}
				else
				{
					data[i] = 0;
				}
				break;
			}
			else
			{
				sequence.push_back(c);
			}
		}
	}
}

void deserializeBlob(std::ifstream* stream, void* data, const size_t blobLen)
{
	stream->read((char*)data, blobLen);
}

void serialize(RecordedAnimation* data, const char* fname)
{
	std::ofstream stream(fname, std::ios::binary);

	spdlog::info("write version");
	serializeBlob(&stream, (void*)&BINARY_FORMAT_VERSION, sizeof(int));
	spdlog::info("write sequences1");
	serializeStrings(&stream, data->sequences_1, SEQUENCES_1_LEN);

	spdlog::info("write unknown120");
	for (int i = 0; i < UNKNOWN_120_LEN; i++)
	{
		serializeBlob(&stream, &data->unknown_120[i].unknown_0, sizeof(float));
		serializeBlob(&stream, &data->unknown_120[i].unknown_1, sizeof(float));
	}

	spdlog::info("write sequences");
	serializeStrings(&stream, data->sequences, SEQUENCES_LEN);

	spdlog::info("write unk616");
	for (int i = 0; i < UNKNOWN_616_LEN; i++)
	{
		serializeBlob(&stream, &data->unknown_616[i], sizeof(long long));
	}

	spdlog::info("write origin");
	serializeBlob(&stream, &data->origin, sizeof(Vector3));
	spdlog::info("write angles");
	serializeBlob(&stream, &data->angles, sizeof(Vector3));
	spdlog::info("write framecount");
	serializeBlob(&stream, &data->frameCount, sizeof(int));
	spdlog::info("write layercount");
	serializeBlob(&stream, &data->layerCount, sizeof(int));
	spdlog::info("write frames");
	serializeBlob(&stream, data->frames, sizeof(RecordedAnimationFrame) * data->frameCount);
	spdlog::info("write layers");
	serializeBlob(&stream, data->layers, sizeof(RecordedAnimationLayer) * data->layerCount);
	spdlog::info("write loffset");
	serializeBlob(&stream, &data->loadedOffset_maybe, sizeof(long long));
	spdlog::info("write uoffset");
	serializeBlob(&stream, &data->userdataOffset_maybe, sizeof(int));
	spdlog::info("write isrefcount");
	serializeBlob(&stream, &data->notRefcounted_maybe, sizeof(bool));
	spdlog::info("write unk813");
	serializeBlob(&stream, &data->unknown_813, sizeof(char));
	spdlog::info("write refcount");
	serializeBlob(&stream, &data->refcount_maybe, sizeof(short));

	spdlog::info("closing stream");
	stream.close();
	spdlog::info("finished writing");
}

void deserialize(const char* fname, RecordedAnimation* out)
{
	std::ifstream stream(fname, std::ios::binary);

	int fileVersion;
	deserializeBlob(&stream, (void*)&fileVersion, sizeof(int));
	std::cout << "animdata version " << fileVersion << "\n";
	deserializeStrings(&stream, out->sequences_1, SEQUENCES_1_LEN);

	for (int i = 0; i < UNKNOWN_120_LEN; i++)
	{
		deserializeBlob(&stream, &out->unknown_120[i].unknown_0, sizeof(float));
		deserializeBlob(&stream, &out->unknown_120[i].unknown_1, sizeof(float));
	}

	deserializeStrings(&stream, out->sequences, SEQUENCES_LEN);

	for (int i = 0; i < UNKNOWN_616_LEN; i++)
	{
		deserializeBlob(&stream, &out->unknown_616[i], sizeof(long long));
	}

	deserializeBlob(&stream, &out->origin, sizeof(Vector3));
	deserializeBlob(&stream, &out->angles, sizeof(Vector3));
	deserializeBlob(&stream, &out->frameCount, sizeof(int));
	deserializeBlob(&stream, &out->layerCount, sizeof(int));

	std::cout << "framecount " << out->frameCount << "\n";

	out->frames = new RecordedAnimationFrame[3000];
	for (size_t i = 0; i < out->frameCount; i++)
	{
		deserializeBlob(&stream, &out->frames[i], sizeof(RecordedAnimationFrame));
	}

	out->layers = new RecordedAnimationLayer[3000];
	for (size_t i = 0; i < out->layerCount; i++)
	{
		deserializeBlob(&stream, &out->layers[i], sizeof(RecordedAnimationLayer));
	}

	deserializeBlob(&stream, &out->loadedOffset_maybe, sizeof(long long));
	deserializeBlob(&stream, &out->userdataOffset_maybe, sizeof(int));
	deserializeBlob(&stream, &out->notRefcounted_maybe, sizeof(bool));
	deserializeBlob(&stream, &out->unknown_813, sizeof(char));
	deserializeBlob(&stream, &out->refcount_maybe, sizeof(short));

	stream.close();
}

void deserialize2(const char* fname, RecordedAnimation* out)
{
	std::ifstream stream(fname, std::ios::binary);
	deserializeBlob(&stream, out, sizeof(*out));
	stream.close();
}

ADD_SQFUNC("void", LoaderGetAnim, "var anim", "", ScriptContext::SERVER)
{
	spdlog::info("loading anim from stack");
	RecordedAnimation* anim = g_pSquirrel<context>->sq_getrecordedanimation(sqvm, 2);

	if (!anim)
	{
		spdlog::info("undefined animation");
		return SQRESULT_ERROR;
	}

	spdlog::info("loaded {} frames", anim->frameCount);
	spdlog::info("loaded sequence {}", anim->sequences[0]);

	UnknownAnimData unk = anim->unknown_120[0];
	spdlog::info("loaded unknown {}", unk.unknown_0);

	spdlog::info("framecount: {}", anim->frameCount);
	spdlog::info("layercount: {}", anim->layerCount);

	spdlog::info("loaded origin from stack <{}, {}, {}>", anim->origin.x, anim->origin.y, anim->origin.z);
	spdlog::info("loaded angles from stack <{}, {}, {}>", anim->angles.x, anim->angles.y, anim->angles.z);

	serialize(anim, "anim_save.anim");

	return SQRESULT_NULL;
}

ADD_SQFUNC("var", LoadAnim, "", "", ScriptContext::SERVER)
{
	spdlog::info("loading anim from disk");
	RecordedAnimation anim;
	deserialize("anim_save.anim", &anim);

	anim.userdataOffset_maybe = g_CreatedRecordedAnimCount++;
	anim.refcount_maybe = 0;

	long long insertResult = InsertAnimInLoadedList(&anim);

	if (insertResult == NULL)
	{
		spdlog::warn("could not load animation in list");
	}
	else
	{
		sq_PushRecordedAnimationFromData(sqvm, &anim);
	}

	return SQRESULT_NOTNULL;
}

ADD_SQFUNC("var", CopyAnim, "var anim", "", ScriptContext::SERVER)
{
	spdlog::info("copying animation from stack");
	RecordedAnimation* anim = g_pSquirrel<context>->sq_getrecordedanimation(sqvm, 2);

	std::ofstream ostream("anim_save.anim", std::ios::binary);
	ostream.write((char*)anim, sizeof(*anim));
	ostream.close();

	spdlog::info("saved animdata");

	// char *buffer = new char[816];
	RecordedAnimation buffer;
	deserialize2("anim_save.anim", &buffer);

	spdlog::info("loaded animdata");
	spdlog::info("anim diff: {}", memcmp(anim, &buffer, sizeof(RecordedAnimation)));

	buffer.userdataOffset_maybe = g_CreatedRecordedAnimCount++;
	buffer.refcount_maybe = 0;

	// RecordedAnimation* n = CreateNewRecordedAnimation();
	// sq_PushRecordedAnimationFromData(sqvm, n);

	// long long insertResult = InsertAnimInLoadedList(&buffer);

	// if (insertResult == NULL)
	// {
	// 	spdlog::warn("could not load animation in list");
	// }
	// else
	// {
	// 	sq_PushRecordedAnimationFromData(sqvm, anim);
	// }

	return SQRESULT_NOTNULL;
}

ADD_SQFUNC("void", AnimTest, "var anim", "", ScriptContext::SERVER)
{
	RecordedAnimation* anim = g_pSquirrel<context>->sq_getrecordedanimation(sqvm, 1);
	
	if(anim)
	{
		spdlog::info("valid recording");
	}
	else
	{
		
	}

	return SQRESULT_NULL;
}

ADD_SQFUNC("var", CopyAnimFromDisk, "var anim", "", ScriptContext::SERVER)
{
	spdlog::info("loading cached animation from stack");
	RecordedAnimation* anim = g_pSquirrel<context>->sq_getrecordedanimation(sqvm, 2);

	if (!anim)
	{
		spdlog::warn("unable to load animation from stack");
		return SQRESULT_ERROR;
	}

	serialize(anim, "anim_save.anim");
	spdlog::info("saved animation on disk");

	spdlog::info("loading anim from disk");
	RecordedAnimation copy;
	deserialize("anim_save.anim", &copy);

	spdlog::info("manipulating copy");
	copy.userdataOffset_maybe = g_CreatedRecordedAnimCount++;
	copy.refcount_maybe = 0;
	// copy.sequences_1 = anim->sequences_1;
	// copy.unknown_120 = anim->unknown_120;
	// copy.sequences = anim->sequences;
	// copy.unknown_616 = anim->unknown_616;
	copy.frames = anim->frames;
	copy.layers = anim->layers;

	long long insertResult = InsertAnimInLoadedList(&copy);

	if (insertResult == NULL)
	{
		spdlog::warn("could not load animation in list");
		return SQRESULT_ERROR;
	}
	else
	{
		spdlog::info("pushing copy on stack");
		sq_PushRecordedAnimationFromData(sqvm, &copy);
	}

	return SQRESULT_NOTNULL;
}

// ========================================
// > REGULAR SCRIPT
// ========================================

ADD_SQFUNC("void", NSEarlyWritePlayerPersistenceForLeave, "entity player", "", ScriptContext::SERVER)
{
	const R2::CBasePlayer* pPlayer = g_pSquirrel<context>->getentity<R2::CBasePlayer>(sqvm, 1);
	if (!pPlayer)
	{
		spdlog::warn("NSEarlyWritePlayerPersistenceForLeave got null player");

		g_pSquirrel<context>->pushbool(sqvm, false);
		return SQRESULT_NOTNULL;
	}

	R2::CBaseClient* pClient = &R2::g_pClientArray[pPlayer->m_nPlayerIndex - 1];
	if (g_pServerAuthentication->m_PlayerAuthenticationData.find(pClient) == g_pServerAuthentication->m_PlayerAuthenticationData.end())
	{
		g_pSquirrel<context>->pushbool(sqvm, false);
		return SQRESULT_NOTNULL;
	}

	g_pServerAuthentication->m_PlayerAuthenticationData[pClient].needPersistenceWriteOnLeave = false;
	g_pServerAuthentication->WritePersistentData(pClient);
	return SQRESULT_NULL;
}

ADD_SQFUNC("bool", NSIsWritingPlayerPersistence, "", "", ScriptContext::SERVER)
{
	g_pSquirrel<context>->pushbool(sqvm, g_pMasterServerManager->m_bSavingPersistentData);
	return SQRESULT_NOTNULL;
}

ADD_SQFUNC("bool", NSIsPlayerLocalPlayer, "entity player", "", ScriptContext::SERVER)
{
	const R2::CBasePlayer* pPlayer = g_pSquirrel<ScriptContext::SERVER>->getentity<R2::CBasePlayer>(sqvm, 1);
	if (!pPlayer)
	{
		spdlog::warn("NSIsPlayerLocalPlayer got null player");

		g_pSquirrel<context>->pushbool(sqvm, false);
		return SQRESULT_NOTNULL;
	}

	R2::CBaseClient* pClient = &R2::g_pClientArray[pPlayer->m_nPlayerIndex - 1];
	g_pSquirrel<context>->pushbool(sqvm, !strcmp(R2::g_pLocalPlayerUserID, pClient->m_UID));
	return SQRESULT_NOTNULL;
}

ADD_SQFUNC("bool", NSIsDedicated, "", "", ScriptContext::SERVER)
{
	g_pSquirrel<context>->pushbool(sqvm, IsDedicatedServer());
	return SQRESULT_NOTNULL;
}

ADD_SQFUNC(
	"bool",
	NSDisconnectPlayer,
	"entity player, string reason",
	"Disconnects the player from the server with the given reason",
	ScriptContext::SERVER)
{
	const R2::CBasePlayer* pPlayer = g_pSquirrel<context>->getentity<R2::CBasePlayer>(sqvm, 1);
	const char* reason = g_pSquirrel<context>->getstring(sqvm, 2);

	if (!pPlayer)
	{
		spdlog::warn("Attempted to call NSDisconnectPlayer() with null player.");

		g_pSquirrel<context>->pushbool(sqvm, false);
		return SQRESULT_NOTNULL;
	}

	// Shouldn't happen but I like sanity checks.
	R2::CBaseClient* pClient = &R2::g_pClientArray[pPlayer->m_nPlayerIndex - 1];
	if (!pClient)
	{
		spdlog::warn("NSDisconnectPlayer(): player entity has null CBaseClient!");

		g_pSquirrel<context>->pushbool(sqvm, false);
		return SQRESULT_NOTNULL;
	}

	if (reason)
	{
		R2::CBaseClient__Disconnect(pClient, 1, reason);
	}
	else
	{
		R2::CBaseClient__Disconnect(pClient, 1, "Disconnected by the server.");
	}

	g_pSquirrel<context>->pushbool(sqvm, true);
	return SQRESULT_NOTNULL;
}
