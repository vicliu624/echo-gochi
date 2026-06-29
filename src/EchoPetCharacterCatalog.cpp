#include "EchoPetCharacterCatalog.h"

namespace echopet {

namespace {

#define CHARACTER_TRAITS(care, page, slot) \
  static_cast<uint8_t>(0x80 | (((page) & 0x07) << 5) | \
                       (((slot) & 0x07) << 2) | ((care) & 0x03))

#define CHARACTER_ROW(name, stage, route, archetype, care, gen, tier, gender, \
                      page, slot, family) \
  {name, stage, route, archetype, care, gen, tier, gender, page, slot, family, \
   CHARACTER_TRAITS(care, page, slot)}

#define CHARACTER_ROW_TRAITS(name, stage, route, archetype, care, gen, tier, \
                             gender, page, slot, family, traits) \
  {name, stage, route, archetype, care, gen, tier, gender, page, slot, family, \
   traits}

constexpr uint8_t kMametchiValidationTraits = 0x80;

const CharacterCatalogEntry kCharacters[kCharacterCatalogCount] = {
    CHARACTER_ROW("Kuroteletchi", Stage::kBaby, GrowthRoute::kBalanced,
                  CharacterKind::kBaby, 0, kCharacterGenerationAny,
                  kCharacterTierAny, kCharacterGenderBoy, 1, 1, 0),
    CHARACTER_ROW("Shiroteletchi", Stage::kBaby, GrowthRoute::kBalanced,
                  CharacterKind::kBaby, 0, kCharacterGenerationAny,
                  kCharacterTierAny, kCharacterGenderGirl, 1, 2, 0),
    CHARACTER_ROW("Tamatchi", Stage::kChild, GrowthRoute::kBalanced,
                  CharacterKind::kSprout, 0, kCharacterGenerationAny,
                  kCharacterTierUnhealthy, kCharacterGenderAny, 1, 3, 1),
    CHARACTER_ROW("Mizutamatchi", Stage::kChild, GrowthRoute::kAthlete,
                  CharacterKind::kSprout, 1, kCharacterGenerationAfterFirst,
                  kCharacterTierHealthy, kCharacterGenderAny, 1, 4, 1),
    CHARACTER_ROW("Kuchitamatchi", Stage::kChild, GrowthRoute::kDreamer,
                  CharacterKind::kSprout, 2, kCharacterGenerationAfterFirst,
                  kCharacterTierHealthy, kCharacterGenderAny, 1, 5, 1),
    CHARACTER_ROW("Mohitamatchi", Stage::kChild, GrowthRoute::kRascal,
                  CharacterKind::kRascal, 3, kCharacterGenerationAny,
                  kCharacterTierUnhealthy, kCharacterGenderAny, 1, 6, 6),
    CHARACTER_ROW("Obotchi", Stage::kTeen, GrowthRoute::kBalanced,
                  CharacterKind::kBuddy, 0, kCharacterGenerationOdd,
                  kCharacterTierHealthy, kCharacterGenderAny, 1, 7, 2),
    CHARACTER_ROW("Youngmametchi", Stage::kTeen, GrowthRoute::kScholar,
                  CharacterKind::kQuill, 1, kCharacterGenerationOdd,
                  kCharacterTierHealthy, kCharacterGenderAny, 1, 8, 7),
    CHARACTER_ROW("Batabatchi", Stage::kTeen, GrowthRoute::kAthlete,
                  CharacterKind::kBolt, 1, kCharacterGenerationOdd,
                  kCharacterTierHealthy, kCharacterGenderAny, 2, 1, 5),
    CHARACTER_ROW("Ichigotchi", Stage::kTeen, GrowthRoute::kSocial,
                  CharacterKind::kDream, 1, kCharacterGenerationOdd,
                  kCharacterTierHealthy, kCharacterGenderAny, 2, 2, 4),
    CHARACTER_ROW("Nikatchi", Stage::kTeen, GrowthRoute::kRascal,
                  CharacterKind::kRascal, 3, kCharacterGenerationOdd,
                  kCharacterTierUnhealthy, kCharacterGenderAny, 2, 3, 6),
    CHARACTER_ROW("Pirorirotchi", Stage::kTeen, GrowthRoute::kDreamer,
                  CharacterKind::kDream, 2, kCharacterGenerationOdd,
                  kCharacterTierUnhealthy, kCharacterGenderAny, 2, 4, 4),
    CHARACTER_ROW("Hikotchi", Stage::kTeen, GrowthRoute::kScholar,
                  CharacterKind::kQuill, 2, kCharacterGenerationEven,
                  kCharacterTierHealthy, kCharacterGenderAny, 2, 5, 7),
    CHARACTER_ROW("Hinatchi", Stage::kTeen, GrowthRoute::kSocial,
                  CharacterKind::kBuddy, 2, kCharacterGenerationEven,
                  kCharacterTierHealthy, kCharacterGenderAny, 2, 6, 2),
    CHARACTER_ROW("Youngmimitchi", Stage::kTeen, GrowthRoute::kBalanced,
                  CharacterKind::kBuddy, 1, kCharacterGenerationEven,
                  kCharacterTierHealthy, kCharacterGenderAny, 2, 7, 2),
    CHARACTER_ROW("Ringotchi", Stage::kTeen, GrowthRoute::kSocial,
                  CharacterKind::kDream, 0, kCharacterGenerationEven,
                  kCharacterTierHealthy, kCharacterGenderAny, 2, 8, 4),
    CHARACTER_ROW("Hinotamatchi", Stage::kTeen, GrowthRoute::kAthlete,
                  CharacterKind::kBolt, 2, kCharacterGenerationEven,
                  kCharacterTierUnhealthy, kCharacterGenderAny, 3, 1, 5),
    CHARACTER_ROW("Hashitamatchi", Stage::kTeen, GrowthRoute::kDreamer,
                  CharacterKind::kDream, 3, kCharacterGenerationEven,
                  kCharacterTierUnhealthy, kCharacterGenderAny, 3, 2, 4),
    CHARACTER_ROW_TRAITS("Mametchi", Stage::kAdult, GrowthRoute::kBalanced,
                         CharacterKind::kBuddy, 0, kCharacterGenerationOdd,
                         kCharacterTierOne, kCharacterGenderAny, 3, 3, 3,
                         kMametchiValidationTraits),
    CHARACTER_ROW("Furawatchi", Stage::kAdult, GrowthRoute::kSocial,
                  CharacterKind::kDream, 0, kCharacterGenerationOdd,
                  kCharacterTierOne, kCharacterGenderAny, 3, 4, 4),
    CHARACTER_ROW("Pyonkotchi", Stage::kAdult, GrowthRoute::kAthlete,
                  CharacterKind::kBolt, 1, kCharacterGenerationOdd,
                  kCharacterTierOne, kCharacterGenderAny, 3, 5, 5),
    CHARACTER_ROW("Kuchipatchi", Stage::kAdult, GrowthRoute::kDreamer,
                  CharacterKind::kDream, 1, kCharacterGenerationOdd,
                  kCharacterTierTwo, kCharacterGenderAny, 3, 6, 4),
    CHARACTER_ROW("Memetchi", Stage::kAdult, GrowthRoute::kSocial,
                  CharacterKind::kDream, 1, kCharacterGenerationOdd,
                  kCharacterTierTwo, kCharacterGenderAny, 3, 7, 4),
    CHARACTER_ROW("Billotchi", Stage::kAdult, GrowthRoute::kScholar,
                  CharacterKind::kQuill, 0, kCharacterGenerationOdd,
                  kCharacterTierTwo, kCharacterGenderAny, 3, 8, 7),
    CHARACTER_ROW("Tarakotchi", Stage::kAdult, GrowthRoute::kRascal,
                  CharacterKind::kRascal, 2, kCharacterGenerationOdd,
                  kCharacterTierThree, kCharacterGenderAny, 4, 1, 6),
    CHARACTER_ROW("Paparatchi", Stage::kAdult, GrowthRoute::kSocial,
                  CharacterKind::kBuddy, 3, kCharacterGenerationOdd,
                  kCharacterTierThree, kCharacterGenderAny, 4, 2, 3),
    CHARACTER_ROW("Mimiyoritchi", Stage::kAdult, GrowthRoute::kBalanced,
                  CharacterKind::kBuddy, 2, kCharacterGenerationOdd,
                  kCharacterTierThree, kCharacterGenderAny, 4, 3, 3),
    CHARACTER_ROW("Hanatchi", Stage::kAdult, GrowthRoute::kDreamer,
                  CharacterKind::kDream, 3, kCharacterGenerationOdd,
                  kCharacterTierFour, kCharacterGenderAny, 4, 4, 4),
    CHARACTER_ROW("Hashizotchi", Stage::kAdult, GrowthRoute::kDreamer,
                  CharacterKind::kDream, 2, kCharacterGenerationOdd,
                  kCharacterTierFour, kCharacterGenderAny, 4, 5, 4),
    CHARACTER_ROW("Tsunotchi", Stage::kAdult, GrowthRoute::kAthlete,
                  CharacterKind::kBolt, 3, kCharacterGenerationOdd,
                  kCharacterTierFour, kCharacterGenderAny, 4, 6, 5),
    CHARACTER_ROW("Maskutchi", Stage::kAdult, GrowthRoute::kRascal,
                  CharacterKind::kRascal, 3, kCharacterGenerationOdd,
                  kCharacterTierSpecial, kCharacterGenderAny, 4, 7, 6),
    CHARACTER_ROW("Megatchi", Stage::kAdult, GrowthRoute::kAthlete,
                  CharacterKind::kBolt, 2, kCharacterGenerationOdd,
                  kCharacterTierSpecial, kCharacterGenderAny, 4, 8, 5),
    CHARACTER_ROW("Mimitchi", Stage::kAdult, GrowthRoute::kBalanced,
                  CharacterKind::kBuddy, 1, kCharacterGenerationEven,
                  kCharacterTierOne, kCharacterGenderAny, 5, 1, 3),
    CHARACTER_ROW("Chomametchi", Stage::kAdult, GrowthRoute::kScholar,
                  CharacterKind::kQuill, 1, kCharacterGenerationEven,
                  kCharacterTierOne, kCharacterGenderAny, 5, 2, 7),
    CHARACTER_ROW("Dekotchi", Stage::kAdult, GrowthRoute::kRascal,
                  CharacterKind::kRascal, 1, kCharacterGenerationEven,
                  kCharacterTierOne, kCharacterGenderAny, 5, 3, 6),
    CHARACTER_ROW("Hidatchi", Stage::kAdult, GrowthRoute::kAthlete,
                  CharacterKind::kBolt, 0, kCharacterGenerationEven,
                  kCharacterTierTwo, kCharacterGenderAny, 5, 4, 5),
    CHARACTER_ROW("Debatchi", Stage::kAdult, GrowthRoute::kAthlete,
                  CharacterKind::kBolt, 3, kCharacterGenerationEven,
                  kCharacterTierTwo, kCharacterGenderAny, 5, 5, 5),
    CHARACTER_ROW("Bunbutchi", Stage::kAdult, GrowthRoute::kSocial,
                  CharacterKind::kDream, 2, kCharacterGenerationEven,
                  kCharacterTierTwo, kCharacterGenderAny, 5, 6, 4),
    CHARACTER_ROW("Pipotchi", Stage::kAdult, GrowthRoute::kSocial,
                  CharacterKind::kBuddy, 2, kCharacterGenerationEven,
                  kCharacterTierThree, kCharacterGenderAny, 5, 7, 3),
    CHARACTER_ROW("Drotchi", Stage::kAdult, GrowthRoute::kRascal,
                  CharacterKind::kRascal, 1, kCharacterGenerationEven,
                  kCharacterTierThree, kCharacterGenderAny, 5, 8, 6),
    CHARACTER_ROW("Bill", Stage::kAdult, GrowthRoute::kScholar,
                  CharacterKind::kQuill, 0, kCharacterGenerationEven,
                  kCharacterTierThree, kCharacterGenderAny, 6, 1, 7),
    CHARACTER_ROW("Robotchi", Stage::kAdult, GrowthRoute::kScholar,
                  CharacterKind::kQuill, 2, kCharacterGenerationEven,
                  kCharacterTierFour, kCharacterGenderAny, 6, 2, 7),
    CHARACTER_ROW("Wooltchi", Stage::kAdult, GrowthRoute::kDreamer,
                  CharacterKind::kDream, 0, kCharacterGenerationEven,
                  kCharacterTierFour, kCharacterGenderAny, 6, 3, 4),
    CHARACTER_ROW("Teketchi", Stage::kAdult, GrowthRoute::kDreamer,
                  CharacterKind::kDream, 2, kCharacterGenerationEven,
                  kCharacterTierFour, kCharacterGenderAny, 6, 4, 4),
    CHARACTER_ROW("Gozarutchi", Stage::kAdult, GrowthRoute::kAthlete,
                  CharacterKind::kBolt, 2, kCharacterGenerationEven,
                  kCharacterTierSpecial, kCharacterGenderAny, 6, 5, 5),
    CHARACTER_ROW("Warusotchi", Stage::kAdult, GrowthRoute::kRascal,
                  CharacterKind::kRascal, 3, kCharacterGenerationEven,
                  kCharacterTierSpecial, kCharacterGenderAny, 6, 6, 6),
    CHARACTER_ROW("Sekitoritchi", Stage::kAdult, GrowthRoute::kAthlete,
                  CharacterKind::kBolt, 2, kCharacterGenerationEven,
                  kCharacterTierSpecial, kCharacterGenderAny, 6, 7, 5),
    CHARACTER_ROW("Oyajitchi", Stage::kAdult, GrowthRoute::kRascal,
                  CharacterKind::kSage, 2, kCharacterGenerationAny,
                  kCharacterTierSpecial, kCharacterGenderBoy, 6, 8, 8),
    CHARACTER_ROW("Ojitchi", Stage::kElder, GrowthRoute::kBalanced,
                  CharacterKind::kSage, 0, kCharacterGenerationAny,
                  kCharacterTierAny, kCharacterGenderBoy, 7, 1, 8),
    CHARACTER_ROW("Otokitchi", Stage::kElder, GrowthRoute::kScholar,
                  CharacterKind::kSage, 1, kCharacterGenerationAny,
                  kCharacterTierAny, kCharacterGenderGirl, 7, 2, 8),
};

#undef CHARACTER_ROW
#undef CHARACTER_ROW_TRAITS
#undef CHARACTER_TRAITS

}  // namespace

const CharacterCatalogEntry& characterCatalogEntry(uint8_t index) {
  if (index >= kCharacterCatalogCount) {
    index = 0;
  }
  return kCharacters[index];
}

const char* characterCatalogLabel(uint8_t index) {
  return characterCatalogEntry(index).name;
}

}  // namespace echopet
