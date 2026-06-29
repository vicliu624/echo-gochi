#pragma once

#include "EchoPetModel.h"

namespace echopet {

bool beginLoraTransport();
bool sendLoraFriendPacket(const FriendPacket& packet);
bool pollLoraFriendPacket(FriendPacket& packet);

}  // namespace echopet
