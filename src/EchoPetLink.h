#pragma once

#include <stddef.h>

#include "EchoPetModel.h"

namespace echopet {

constexpr size_t kFriendHexChars = sizeof(FriendPacket) * 2;
constexpr size_t kFriendLineChars = kFriendHexChars + 3;
constexpr size_t kFriendLineBufferSize = kFriendLineChars + 1;

bool encodeFriendPacketHex(const FriendPacket& packet, char* out,
                           size_t outSize);
bool decodeFriendPacketHex(const char* text, FriendPacket& packet);

}  // namespace echopet
