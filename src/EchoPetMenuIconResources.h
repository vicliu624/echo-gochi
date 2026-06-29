#pragma once

#include <stdint.h>

namespace echopet {

constexpr uint8_t kEchoPetMenuIconCount = 10;
constexpr uint8_t kEchoPetMenuIconCompactSide = 24;
constexpr uint8_t kEchoPetMenuIconLargeSide = 30;
constexpr uint8_t kEchoPetMenuIconCompactBytes = 72;
constexpr uint8_t kEchoPetMenuIconLargeBytes = 120;

struct EchoPetMenuIconSemantic {
  const char* localId;
  const char* officialLabel;
};

extern const EchoPetMenuIconSemantic
    kEchoPetMenuIconSemantics[kEchoPetMenuIconCount];
extern const uint8_t
    kEchoPetMenuIcons64Compact[kEchoPetMenuIconCount]
                              [kEchoPetMenuIconCompactBytes];
extern const uint8_t
    kEchoPetMenuIcons128Large[kEchoPetMenuIconCount]
                             [kEchoPetMenuIconLargeBytes];

}  // namespace echopet
