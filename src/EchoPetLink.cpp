#include "EchoPetLink.h"

namespace echopet {

namespace {

char hexDigit(uint8_t value) {
  value &= 0x0F;
  return value < 10 ? static_cast<char>('0' + value)
                    : static_cast<char>('A' + value - 10);
}

int8_t hexValue(char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  }
  if (c >= 'a' && c <= 'f') {
    return c - 'a' + 10;
  }
  if (c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  }
  return -1;
}

const char* skipPrefix(const char* text) {
  if (!text) {
    return nullptr;
  }
  if ((text[0] == 'E' || text[0] == 'e') &&
      (text[1] == 'P' || text[1] == 'p') && text[2] == ':') {
    return text + 3;
  }
  return text;
}

}  // namespace

bool encodeFriendPacketHex(const FriendPacket& packet, char* out,
                           size_t outSize) {
  if (!out || outSize < kFriendLineBufferSize ||
      checksumFriendPacket(packet) != packet.checksum) {
    return false;
  }

  out[0] = 'E';
  out[1] = 'P';
  out[2] = ':';

  const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&packet);
  for (size_t i = 0; i < sizeof(FriendPacket); i++) {
    out[3 + i * 2] = hexDigit(bytes[i] >> 4);
    out[3 + i * 2 + 1] = hexDigit(bytes[i]);
  }
  out[kFriendLineChars] = '\0';
  return true;
}

bool decodeFriendPacketHex(const char* text, FriendPacket& packet) {
  text = skipPrefix(text);
  if (!text) {
    return false;
  }

  uint8_t* bytes = reinterpret_cast<uint8_t*>(&packet);
  for (size_t i = 0; i < sizeof(FriendPacket); i++) {
    const int8_t hi = hexValue(text[i * 2]);
    const int8_t lo = hexValue(text[i * 2 + 1]);
    if (hi < 0 || lo < 0) {
      return false;
    }
    bytes[i] = static_cast<uint8_t>((hi << 4) | lo);
  }

  return text[kFriendHexChars] == '\0' && validateFriendPacket(packet);
}

}  // namespace echopet
