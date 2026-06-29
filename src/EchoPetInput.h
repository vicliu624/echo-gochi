#pragma once

#include <stdint.h>

namespace echopet {

enum class ConnectButtonEvent : uint8_t {
  kNone,
  kA,
  kB,
  kC,
  kALong,
  kBLong,
  kCLong,
};

bool beginEchoPetInput();
bool echoPetInputUsesKeyShield();
ConnectButtonEvent pollEchoPetInput();
const char* connectButtonLabel(ConnectButtonEvent event);

}  // namespace echopet
