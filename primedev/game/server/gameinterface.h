#pragma once

#include "engine/eiface.h"

class CServerGameClients : public IServerGameClients
{
	//
};

inline CServerGameClients* g_pServerGameClients;

class CServerGameDLL;

inline CServerGameDLL* g_pServerGameDLL;

inline void(__fastcall* CServerGameDLL__OnReceivedSayTextMessage)(CServerGameDLL* self, unsigned int senderPlayerId, const char* text, int channelId);

//
enum class CustomMessageType : char
{
	Chat = 1,
	Whisper = 2
};

constexpr unsigned char CUSTOM_MESSAGE_INDEX_BIT = 0b10000000;
constexpr unsigned char CUSTOM_MESSAGE_INDEX_MASK = (unsigned char)~CUSTOM_MESSAGE_INDEX_BIT;

// Send a vanilla chat message as if it was from the player.
void Chat_SendMessage(unsigned int playerIndex, const char* text, bool isTeam);

// Send a custom message.
//   fromPlayerIndex: set to -1 for a [SERVER] message, or another value to send from a specific player
//   toPlayerIndex: set to -1 to send to all players, or another value to send to a single player
//   isTeam: display a [TEAM] badge
//   isDead: display a [DEAD] badge
//   messageType: send a specific message type
void Chat_BroadcastMessage(int fromPlayerIndex, int toPlayerIndex, const char* text, bool isTeam, bool isDead, CustomMessageType messageType);
