#pragma once

#include "networksystem/netchannel.h"

class IConnectionlessPacketHandler
{
  public:
	virtual ~IConnectionlessPacketHandler() = 0;

	virtual bool ProcessConnectionlessPacket(netpacket_t* packet) = 0;
};
