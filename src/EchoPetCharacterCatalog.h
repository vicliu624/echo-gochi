#pragma once

#include <stdint.h>

#include "EchoPetModel.h"

namespace echopet {

constexpr uint8_t kCharacterCatalogCount = 50;

constexpr uint8_t kCharacterGenerationFirst = 1 << 0;
constexpr uint8_t kCharacterGenerationOdd = 1 << 1;
constexpr uint8_t kCharacterGenerationEven = 1 << 2;
constexpr uint8_t kCharacterGenerationAfterFirst =
    kCharacterGenerationOdd | kCharacterGenerationEven;
constexpr uint8_t kCharacterGenerationAny =
    kCharacterGenerationFirst | kCharacterGenerationOdd |
    kCharacterGenerationEven;

constexpr uint8_t kCharacterTierOne = 1 << 0;
constexpr uint8_t kCharacterTierTwo = 1 << 1;
constexpr uint8_t kCharacterTierThree = 1 << 2;
constexpr uint8_t kCharacterTierFour = 1 << 3;
constexpr uint8_t kCharacterTierSpecial = 1 << 4;
constexpr uint8_t kCharacterTierHealthy =
    kCharacterTierOne | kCharacterTierTwo;
constexpr uint8_t kCharacterTierUnhealthy =
    kCharacterTierThree | kCharacterTierFour;
constexpr uint8_t kCharacterTierAny =
    kCharacterTierOne | kCharacterTierTwo | kCharacterTierThree |
    kCharacterTierFour | kCharacterTierSpecial;

constexpr uint8_t kCharacterGenderBoy = 1 << 0;
constexpr uint8_t kCharacterGenderGirl = 1 << 1;
constexpr uint8_t kCharacterGenderAny =
    kCharacterGenderBoy | kCharacterGenderGirl;

struct CharacterCatalogEntry {
  const char* name;
  Stage stage;
  GrowthRoute route;
  CharacterKind archetype;
  uint8_t careBand;
  uint8_t generationMask;
  uint8_t tierMask;
  uint8_t genderMask;
  uint8_t sourcePage;
  uint8_t sourceSlot;
  uint8_t frameFamily;
  uint8_t visualTraits;
};

const CharacterCatalogEntry& characterCatalogEntry(uint8_t index);
const char* characterCatalogLabel(uint8_t index);

}  // namespace echopet
