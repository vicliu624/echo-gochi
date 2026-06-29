#pragma once

#include <stdint.h>

namespace echopet {

constexpr uint8_t kCatalogItemCount = 160;
constexpr uint8_t kCatalogItemBitBytes = (kCatalogItemCount + 7) / 8;
constexpr uint8_t kCatalogSouvenirCount = 64;
constexpr uint8_t kCatalogSouvenirBitBytes = (kCatalogSouvenirCount + 7) / 8;
constexpr uint8_t kShopSlotCount = 4;
constexpr uint8_t kCatalogFlagSnack = 0x01;
constexpr uint8_t kCatalogFlagTravel = 0x02;
constexpr uint8_t kCatalogFlagAdultOnly = 0x04;
constexpr uint8_t kCatalogFlagReusable = 0x08;
constexpr uint8_t kCatalogFlagChildPlus = 0x10;
constexpr uint8_t kCatalogFlagTeenPlus = 0x20;
constexpr uint8_t kCatalogFlagSecret = 0x80;
constexpr uint8_t kCatalogSecretFirst = 144;
constexpr uint8_t kCatalogSecretCount = 7;
constexpr uint8_t kCatalogClockIndex = 146;
constexpr uint8_t kCatalogYogurtIndex = 87;
constexpr uint8_t kCatalogSteakIndex = 145;
constexpr uint8_t kCatalogHoneyIndex = 148;
constexpr uint8_t kCatalogRcCar3Index = 149;
constexpr uint8_t kCatalogNyatchiIndex = 150;
constexpr uint8_t kCatalogHohotchiIndex = 151;
constexpr uint8_t kCatalogPlantIndex = 49;
constexpr uint8_t kCatalogShovelIndex = 54;
constexpr uint8_t kCatalogTicket2Index = 61;
constexpr uint8_t kCatalogBallIndex = 60;
constexpr uint8_t kCatalogTrumpetAIndex = 84;
constexpr uint8_t kCatalogActionFigureIndex = 62;
constexpr uint8_t kCatalogBoomBoxIndex = 65;
constexpr uint8_t kCatalogChestIndex = 66;
constexpr uint8_t kCatalogDoll1Index = 68;
constexpr uint8_t kCatalogFishingPoleIndex = 69;
constexpr uint8_t kCatalogLampIndex = 70;
constexpr uint8_t kCatalogMirrorIndex = 71;
constexpr uint8_t kCatalogMusicDiscIndex = 72;
constexpr uint8_t kCatalogTicket1Index = 80;
constexpr uint8_t kCatalogTicket3Index = 81;
constexpr uint8_t kCatalogTicket4Index = 82;
constexpr uint8_t kCatalogTicket5Index = 83;
constexpr uint8_t kCatalogDoll2Index = 92;
constexpr uint8_t kCatalogMakeupIndex = 93;
constexpr uint8_t kCatalogShaverIndex = 94;
constexpr uint8_t kCatalogTamaDrinkIndex = 95;
constexpr uint8_t kCatalogHairGelIndex = 147;
constexpr uint8_t kCatalogBalloonIndex = 120;
constexpr uint8_t kCatalogBuildingBlockIndex = 133;
constexpr uint8_t kCatalogRcCar1Index = 135;
constexpr uint8_t kCatalogRcCar2Index = 136;
constexpr uint8_t kCatalogRopeIndex = 137;
constexpr uint8_t kCatalogTrumpetBIndex = 141;
constexpr uint8_t kCatalogUseMeal = 1;
constexpr uint8_t kCatalogUseSnack = 2;
constexpr uint8_t kCatalogUsePlay = 3;
constexpr uint8_t kCatalogUseStudy = 4;
constexpr uint8_t kCatalogUseMusic = 5;
constexpr uint8_t kCatalogUseTravel = 6;
constexpr uint8_t kCatalogUseMemory = 7;
constexpr uint8_t kCatalogFoodIconTurkey = 8;
constexpr uint8_t kCatalogFoodIconCake = 9;
constexpr uint8_t kCatalogFoodIconYogurt = 10;
constexpr uint8_t kCatalogItemIconTv = 8;
constexpr uint8_t kCatalogItemIconWeights = 9;
constexpr uint8_t kCatalogItemIconWig = 10;
constexpr uint8_t kCatalogItemIconWings = 11;
constexpr uint8_t kCatalogItemIconDoll = 12;
constexpr uint8_t kCatalogItemIconMakeup = 13;
constexpr uint8_t kCatalogItemIconShaver = 14;
constexpr uint8_t kCatalogItemIconTamaDrink = 15;
constexpr uint8_t kCatalogItemIconPencil = 16;
constexpr uint8_t kCatalogItemIconPlant = 17;
constexpr uint8_t kCatalogItemIconUmbrella = 18;
constexpr uint8_t kCatalogItemIconGlasses = 19;
constexpr uint8_t kCatalogItemIconBow = 20;
constexpr uint8_t kCatalogItemIconDrum = 21;
constexpr uint8_t kCatalogItemIconShovel = 22;
constexpr uint8_t kCatalogItemIconSkates = 23;
constexpr uint8_t kCatalogItemIconBalloon = 24;
constexpr uint8_t kCatalogItemIconBowTie = 25;
constexpr uint8_t kCatalogItemIconCap = 26;
constexpr uint8_t kCatalogItemIconChest = 27;
constexpr uint8_t kCatalogItemIconDarts = 28;
constexpr uint8_t kCatalogItemIconPole = 29;
constexpr uint8_t kCatalogItemIconLamp = 30;
constexpr uint8_t kCatalogItemIconMirror = 31;
constexpr uint8_t kCatalogItemIconPhonograph = 32;
constexpr uint8_t kCatalogItemIconCar = 33;
constexpr uint8_t kCatalogItemIconShirt = 34;
constexpr uint8_t kCatalogItemIconShoes = 35;
constexpr uint8_t kCatalogItemIconThrone = 36;
constexpr uint8_t kCatalogItemIconClock = 37;
constexpr uint8_t kCatalogItemIconHairGel = 38;
constexpr uint8_t kCatalogItemIconHoney = 39;
constexpr uint8_t kCatalogItemIconCharacter = 40;

enum class CatalogKind : uint8_t {
  kFood,
  kItem,
  kSouvenir,
};

struct CatalogEntry {
  CatalogKind kind;
  uint8_t behavior;
  uint16_t price;
  uint8_t icon;
  uint8_t flags;
  uint8_t sourceGroup;
  uint8_t useScene;
  uint8_t visualTraits;
};

CatalogEntry catalogEntry(uint8_t index);
uint8_t catalogShopIndex(uint8_t slot, uint8_t month, uint8_t day,
                         uint8_t birthdayMonth, uint8_t birthdayDay,
                         uint32_t clockMinutes);
uint8_t catalogShopIndex(uint8_t slot, uint8_t day, uint32_t clockMinutes);
bool catalogShopIsVendorVisit(uint32_t clockMinutes);
bool catalogShopIsSale(uint8_t day, uint32_t clockMinutes);
uint16_t catalogShopPrice(uint8_t index, uint8_t day, uint32_t clockMinutes);
void catalogItemLabel(uint8_t index, char* buffer, uint8_t size);
void catalogSouvenirLabel(uint8_t index, char* buffer, uint8_t size);
uint8_t catalogSouvenirVisualTraits(uint8_t index);
bool catalogBitTest(const uint8_t* bits, uint8_t index);
void catalogBitSet(uint8_t* bits, uint8_t index);
void catalogBitClear(uint8_t* bits, uint8_t index);
uint8_t catalogBitCount(const uint8_t* bits, uint8_t bitCount);

}  // namespace echopet
