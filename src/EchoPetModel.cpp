#include "EchoPetModel.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "EchoPetCharacterCatalog.h"

namespace echopet {

namespace {

constexpr uint32_t kMagic = 0x45505445UL;  // "ETPE" little-endian marker.
constexpr uint16_t kVersion = 12;
constexpr uint16_t kMinReadableVersion = 4;
constexpr size_t kPetSaveV6Size = offsetof(PetSave, catalogStock);
constexpr size_t kPetSaveCatalogStockEnd =
    offsetof(PetSave, catalogStock) +
    sizeof(((PetSave*)0)->catalogStock);
constexpr size_t kPetSaveFamilyAncestryEnd =
    offsetof(PetSave, familyAncestry) +
    sizeof(((PetSave*)0)->familyAncestry);
constexpr uint16_t kFriendMagic = 0x5045;  // "EP"
constexpr uint8_t kFriendVersion = 4;
constexpr uint8_t kMaxFoodStock = 19;
constexpr uint8_t kMaxItemStock = 32;
constexpr uint32_t kGetRoundWindowMs = 1600;
constexpr uint32_t kFlagRoundWindowMs = 1200;
constexpr uint32_t kHoopsShotWindowMs = 1700;
// Timing is kept in real pet minutes. See CONNECT_GROWTH_SPEC.md.
constexpr uint32_t kEggHatchAgeMinutes = 5;
constexpr uint32_t kBabyToChildAgeMinutes = kEggHatchAgeMinutes + 60;
constexpr uint32_t kChildToTeenAgeMinutes = kBabyToChildAgeMinutes + 1440;
constexpr uint32_t kTeenToAdultAgeMinutes = kChildToTeenAgeMinutes + 4320;
constexpr uint32_t kAdultToElderAgeMinutes =
    kTeenToAdultAgeMinutes + 4UL * 1440UL;
constexpr uint32_t kMatchmakerFirstAgeMinutes =
    kTeenToAdultAgeMinutes + 4UL * 1440UL;
constexpr uint16_t kParentLeaveMinutes = 1440;
constexpr uint32_t kManualMatchmakerAgeMinutes = 2UL * 1440UL;
constexpr uint16_t kMaxGotchiPoints = 9999;
constexpr uint8_t kVisibleHeartMeterMax = 100;
constexpr uint8_t kHiddenHeartMeterMax = 150;
constexpr uint8_t kTasteUnknown = 0xFF;
constexpr uint8_t kFlagFakeSignal = 1;
constexpr uint8_t kFlagFakeSignalChancePercent = 20;
constexpr uint8_t kCareMistakeCap = 99;
constexpr uint8_t kGrowthFlagOyajitchiLineage = 0x01;

enum class StatMeter : uint8_t {
  kHunger,
  kHappiness,
  kEnergy,
  kHygiene,
};

struct StatDecayRule {
  StatMeter meter;
  uint16_t periodMinutes;
  int8_t delta;
  uint8_t skipAtBedtime;
};

const StatDecayRule kStatDecayRules[] = {
    {StatMeter::kHunger, 7, -2, 1},
    {StatMeter::kHappiness, 9, -2, 1},
    {StatMeter::kEnergy, 11, -2, 1},
    {StatMeter::kHygiene, 17, -3, 0},
};

struct StageCareTimingRule {
  Stage stage;
  uint16_t hungerMinutes;
  uint16_t happinessMinutes;
  uint16_t messMinutes;
};

struct SleepWindowRule {
  Stage stage;
  uint16_t sleepStartMinute;
  uint16_t wakeMinute;
};

enum class AttentionTimingKind : uint8_t {
  kOrdinaryMiss,
  kCareCall,
  kEmptyStatMistake,
  kSleepAttention,
  kLightsLeftOnMistake,
};

struct AttentionTimingRule {
  AttentionTimingKind kind;
  uint8_t periodMinutes;
  uint8_t minuteOffset;
};

struct GrowthScheduleRule {
  Stage result;
  uint32_t minAgeMinutes;
};

struct FamilyTimingRule {
  uint16_t parentLeaveMinutes;
  uint32_t manualMatchmakerAgeMinutes;
};

struct MatchmakerClockRule {
  uint32_t minAgeMinutes;
  uint16_t clockMinute;
};

struct CalendarEventRule {
  uint8_t startMonth;
  uint8_t startDay;
  uint8_t endMonth;
  uint8_t endDay;
  uint8_t slotMask;
  Notice notice;
};

struct StageBaseWeightRule {
  Stage stage;
  uint16_t baseWeight;
};

struct CharacterFoodTasteRule {
  uint8_t characterCatalogId;
  uint8_t likedCatalogIndex;
  uint8_t dislikedCatalogIndex;
};

const StageCareTimingRule kStageCareTimingRules[] = {
    {Stage::kEgg, 0, 0, 0},
    {Stage::kBaby, 3, 5, 6},
    {Stage::kChild, 40, 45, 80},
    {Stage::kTeen, 50, 55, 100},
    {Stage::kAdult, 70, 75, 140},
    {Stage::kParentCare, 70, 75, 140},
    {Stage::kElder, 80, 110, 160},
};

const SleepWindowRule kSleepWindowRules[] = {
    {Stage::kEgg, 23 * 60, 7 * 60},
    {Stage::kBaby, 20 * 60, 8 * 60},
    {Stage::kChild, 21 * 60, 8 * 60},
    {Stage::kTeen, 21 * 60, 7 * 60},
    {Stage::kAdult, 22 * 60, 7 * 60},
    {Stage::kParentCare, 22 * 60, 7 * 60},
    {Stage::kElder, 20 * 60, 8 * 60},
};

const AttentionTimingRule kAttentionTimingRules[] = {
    {AttentionTimingKind::kOrdinaryMiss, 15, 0},
    {AttentionTimingKind::kCareCall, 29, 0},
    {AttentionTimingKind::kEmptyStatMistake, 30, 0},
    {AttentionTimingKind::kSleepAttention, 30, 0},
    {AttentionTimingKind::kLightsLeftOnMistake, 60, 20},
};

const GrowthScheduleRule kGrowthScheduleRules[] = {
    {Stage::kElder, kAdultToElderAgeMinutes},
    {Stage::kAdult, kTeenToAdultAgeMinutes},
    {Stage::kTeen, kChildToTeenAgeMinutes},
    {Stage::kChild, kBabyToChildAgeMinutes},
    {Stage::kBaby, kEggHatchAgeMinutes},
};

const FamilyTimingRule kFamilyTimingRule = {
    kParentLeaveMinutes,
    kManualMatchmakerAgeMinutes,
};

const MatchmakerClockRule kMatchmakerClockRules[] = {
    {kMatchmakerFirstAgeMinutes, 10UL * 60UL + 30UL},
    {kMatchmakerFirstAgeMinutes, 15UL * 60UL},
    {kMatchmakerFirstAgeMinutes, 19UL * 60UL},
};

constexpr uint8_t kCalendarSlotMidnight = 0x01;
constexpr uint8_t kCalendarSlotMorning = 0x02;
constexpr uint8_t kCalendarSlotAfternoon = 0x04;
constexpr uint8_t kCalendarSlotEvening = 0x08;
constexpr uint8_t kCalendarSlotLate = 0x10;
constexpr uint8_t kCalendarStandardSlots =
    kCalendarSlotMorning | kCalendarSlotAfternoon | kCalendarSlotEvening;

const CalendarEventRule kCalendarEventRules[] = {
    // Curlour Connection 2024 event notes: date events play at 11:30,
    // 14:00, and 16:30 unless a specific time is listed.
    {1, 1, 1, 1, kCalendarSlotMidnight, Notice::kAnniversary},
    {2, 1, 2, 28, kCalendarStandardSlots, Notice::kAnniversary},
    {4, 8, 4, 10, kCalendarStandardSlots, Notice::kAnniversary},
    {8, 15, 9, 8, kCalendarStandardSlots, Notice::kAnniversary},
    {10, 31, 10, 31, kCalendarStandardSlots, Notice::kAnniversary},
    {11, 18, 11, 26, kCalendarStandardSlots, Notice::kAnniversary},
    {11, 30, 12, 25, kCalendarStandardSlots, Notice::kAnniversary},
    {12, 24, 12, 24, kCalendarSlotLate, Notice::kAnniversary},
};

const StageBaseWeightRule kStageBaseWeightRules[] = {
    {Stage::kEgg, 5},
    {Stage::kBaby, 5},
    {Stage::kChild, 10},
    {Stage::kTeen, 20},
    {Stage::kAdult, 30},
    {Stage::kParentCare, 30},
    {Stage::kElder, 30},
};

const CharacterFoodTasteRule kCharacterFoodTasteRules[] = {
    {0, 11, 47},   {1, 22, 14},   {2, 15, 6},    {3, 46, kTasteUnknown},
    {4, 29, kTasteUnknown},       {5, 20, kTasteUnknown},
    {6, 23, 27},   {7, 31, 14},   {8, 9, 6},     {9, 5, 15},
    {10, 23, kTasteUnknown},      {11, 6, kTasteUnknown},
    {12, 2, 9},    {13, 4, 22},   {14, 19, 46},  {15, 15, 29},
    {16, 31, kTasteUnknown},      {17, 39, kTasteUnknown},
    {18, 0, 1},    {19, 40, 29},  {20, 19, 20},  {21, 22, 2},
    {22, 15, 9},   {23, 10, 22},  {24, 1, kTasteUnknown},
    {25, 47, kTasteUnknown},      {26, 2, 39},   {27, 15, 11},
    {28, 14, kTasteUnknown},      {29, 145, 6},  {30, 29, 4},
    {31, 20, 22},  {32, 16, 46},  {33, 20, 2},   {34, 19, 5},
    {35, 2, 0},    {36, 5, 145},  {37, 6, kTasteUnknown},
    {38, 14, kTasteUnknown},      {39, kTasteUnknown, kTasteUnknown},
    {40, kTasteUnknown, kTasteUnknown},          {41, 20, kTasteUnknown},
    {42, 40, 39},  {43, 15, 23},  {44, 47, kTasteUnknown},
    {45, 14, kTasteUnknown},      {46, 2, 39},   {47, 3, kTasteUnknown},
    {48, 3, kTasteUnknown},       {49, 40, kTasteUnknown},
};

constexpr uint8_t kMaxMessCount = 4;
constexpr int8_t kMessHygienePenalty = -13;
constexpr uint8_t kStageSicknessPassAwayCount = 4;
constexpr uint16_t kSuperCleanerDonation = 1000;
constexpr uint8_t kSweetSnackToothacheStreak = 15;
constexpr uint16_t kSweetSnackShortPeriodMinutes = 60;
constexpr uint16_t kSweetSnackNoLastMinute = 0xFFFF;
constexpr uint16_t kPlantPointReward = 120;
constexpr uint16_t kShovelPointReward = 80;
constexpr uint16_t kChestPointReward = 300;
constexpr uint16_t kFishingPolePointReward = 300;
constexpr uint16_t kLampPointReward = 500;

enum class SicknessTriggerKind : uint8_t {
  kLowHygiene,
  kMess,
  kHighWeight,
  kSweetSnack,
};

struct SicknessTriggerRule {
  SicknessTriggerKind kind;
  uint8_t threshold;
  uint8_t chancePercent;
  uint8_t minSeverity;
  uint8_t severityRollCount;
};

enum PassAwayRuleFlags : uint8_t {
  kPassAwayNeedsSick = 1 << 0,
  kPassAwayNeedsElder = 1 << 1,
  kPassAwayNeedsLowFriend = 1 << 2,
  kPassAwayNeedsAnyLowStat = 1 << 3,
};

struct PassAwayRule {
  uint8_t flags;
  uint16_t minCareMistakes;
  uint32_t minAgeMinutes;
  uint8_t maxHunger;
  uint8_t maxHappiness;
  uint8_t maxHygiene;
};

enum AdultGrowthRuleFlags : uint8_t {
  kAdultGrowthRequiresHealthyParents = 1 << 0,
};

struct AdultGrowthRule {
  GrowthTier previousTier;
  uint8_t minPhysical;
  uint8_t maxPhysical;
  uint8_t minMental;
  uint8_t maxMental;
  uint8_t flags;
  AdultTier result;
};

constexpr uint8_t kPassAwayDontCare = 255;
constexpr uint8_t kLowFriendPassAwayLimit = 39;
constexpr uint8_t kAdultGrowthAny = 255;

const PassAwayRule kPassAwayRules[] = {
    {0, 14, 0, kPassAwayDontCare, kPassAwayDontCare, kPassAwayDontCare},
    {kPassAwayNeedsSick, 0, 0, 0, kPassAwayDontCare, 0},
    {static_cast<uint8_t>(kPassAwayNeedsElder | kPassAwayNeedsLowFriend |
                          kPassAwayNeedsAnyLowStat),
     5, kAdultToElderAgeMinutes + 1, 25, 25, 25},
};

const AdultGrowthRule kAdultGrowthRules[] = {
    {GrowthTier::kTierOne, 0, 1, 0, 1, kAdultGrowthRequiresHealthyParents,
     AdultTier::kSerious},
    {GrowthTier::kTierOne, 2, 2, 0, 2, 0, AdultTier::kNormal},
    {GrowthTier::kTierOne, 0, 1, 2, 2, 0, AdultTier::kNormal},
    {GrowthTier::kTierOne, 3, kAdultGrowthAny, 3, kAdultGrowthAny, 0,
     AdultTier::kSpecial},
    {GrowthTier::kTierOne, 0, 2, 3, kAdultGrowthAny, 0,
     AdultTier::kNaughty},
    {GrowthTier::kTierOne, 3, kAdultGrowthAny, 0, 2, 0, AdultTier::kFrail},
    {GrowthTier::kTierOne, 0, 1, 0, 1, 0, AdultTier::kNormal},

    {GrowthTier::kTierTwo, 0, 0, 0, 1, kAdultGrowthRequiresHealthyParents,
     AdultTier::kSerious},
    {GrowthTier::kTierTwo, 3, kAdultGrowthAny, 3, kAdultGrowthAny, 0,
     AdultTier::kSpecial},
    {GrowthTier::kTierTwo, 0, 2, 3, kAdultGrowthAny, 0,
     AdultTier::kNaughty},
    {GrowthTier::kTierTwo, 3, kAdultGrowthAny, 0, 2, 0, AdultTier::kFrail},
    {GrowthTier::kTierTwo, 0, kAdultGrowthAny, 0, kAdultGrowthAny, 0,
     AdultTier::kNormal},

    {GrowthTier::kTierThree, 0, 0, 0, 0, kAdultGrowthRequiresHealthyParents,
     AdultTier::kSerious},
    {GrowthTier::kTierThree, 0, 1, 0, 1, 0, AdultTier::kNormal},
    {GrowthTier::kTierThree, 2, kAdultGrowthAny, 2, kAdultGrowthAny, 0,
     AdultTier::kSpecial},
    {GrowthTier::kTierThree, 0, 1, 2, kAdultGrowthAny, 0,
     AdultTier::kNaughty},
    {GrowthTier::kTierThree, 2, kAdultGrowthAny, 0, 1, 0, AdultTier::kFrail},
    {GrowthTier::kTierThree, 0, kAdultGrowthAny, 0, kAdultGrowthAny, 0,
     AdultTier::kNormal},

    {GrowthTier::kTierFour, 0, 1, 0, 1, 0, AdultTier::kNormal},
    {GrowthTier::kTierFour, 2, kAdultGrowthAny, 2, kAdultGrowthAny, 0,
     AdultTier::kSpecial},
    {GrowthTier::kTierFour, 0, 1, 2, kAdultGrowthAny, 0,
     AdultTier::kNaughty},
    {GrowthTier::kTierFour, 2, kAdultGrowthAny, 0, 1, 0, AdultTier::kFrail},
    {GrowthTier::kTierFour, 0, kAdultGrowthAny, 0, kAdultGrowthAny, 0,
     AdultTier::kSpecial},
};

enum class PasswordRewardKind : uint8_t {
  kFood,
  kItem,
  kSouvenir,
};

struct PasswordReward {
  uint8_t digits[10];
  PasswordRewardKind kind;
  uint8_t id;
  uint8_t amount;
  uint8_t catalogIndex;
  uint8_t repeatable;
};

struct SecretCodeReward {
  uint8_t symbols[8];
  uint8_t catalogIndex;
};

struct GameRewardRule {
  Stage stage;
  MiniGameKind game;
  uint16_t prize;
  uint16_t heavyPrize;
  uint8_t heavyWeight;
};

struct GameUnlockRule {
  Stage stage;
  uint8_t count;
};

enum GamePacingFlags : uint8_t {
  kGamePacingTimedAuto = 1 << 0,
  kGamePacingAnyButton = 1 << 1,
  kGamePacingEpaperCompressed = 1 << 2,
};

struct GamePacingRule {
  MiniGameKind game;
  uint8_t sourceGoal;
  uint8_t roundLimit;
  uint8_t scoreLimit;
  uint16_t autoWindowMs;
  uint8_t flags;
};

struct BumpPrizeRow {
  uint8_t sourceRoundsWon;
  uint16_t prizeByWeightBand[4];
};

struct CatalogRandomRule {
  uint8_t catalogIndex;
  uint8_t rollCount;
  uint16_t pointReward;
  uint8_t breakChancePercent;
};

const PasswordReward kPasswordRewards[] = {
    {{5, 4, 8, 9, 0, 0, 7, 4, 4, 5}, PasswordRewardKind::kFood,
     static_cast<uint8_t>(FoodKind::kPudding), 1, 24, 0},  // Ice Cream.
    {{1, 2, 1, 5, 2, 4, 2, 6, 9, 4}, PasswordRewardKind::kFood,
     static_cast<uint8_t>(FoodKind::kPudding), 1, 25, 0},  // Soda.
    {{7, 9, 5, 6, 3, 3, 6, 7, 4, 3}, PasswordRewardKind::kFood,
     static_cast<uint8_t>(FoodKind::kTart), 1, 26, 0},   // Waffle.
    {{9, 4, 7, 3, 6, 8, 9, 3, 2, 1}, PasswordRewardKind::kFood,
     static_cast<uint8_t>(FoodKind::kPudding), 1, 27, 0},  // Sundae.
    {{1, 9, 7, 5, 4, 8, 1, 2, 0, 0}, PasswordRewardKind::kFood,
     static_cast<uint8_t>(FoodKind::kPudding), 1, 28, 0},  // Shaved Ice.
    {{9, 6, 4, 7, 1, 0, 5, 3, 8, 6}, PasswordRewardKind::kFood,
     static_cast<uint8_t>(FoodKind::kSushi), 1, 29, 0},   // Pizza.
    {{8, 0, 8, 2, 3, 5, 8, 1, 6, 3}, PasswordRewardKind::kFood,
     static_cast<uint8_t>(FoodKind::kScone), 1, 30, 0},   // Sweet Potato.
    {{9, 9, 0, 1, 6, 2, 3, 7, 2, 6}, PasswordRewardKind::kFood,
     static_cast<uint8_t>(FoodKind::kCereal), 1, 31, 0},   // Milk.
    {{7, 9, 4, 7, 6, 7, 5, 1, 7, 4}, PasswordRewardKind::kFood,
     static_cast<uint8_t>(FoodKind::kPudding), 1, 32, 0},  // Fruit Juice.
    {{4, 7, 0, 4, 8, 1, 7, 8, 9, 8}, PasswordRewardKind::kFood,
     static_cast<uint8_t>(FoodKind::kApple), 1, 33, 0},  // Grapes.
    {{9, 8, 5, 8, 3, 5, 7, 0, 4, 1}, PasswordRewardKind::kFood,
     static_cast<uint8_t>(FoodKind::kPudding), 1, 34, 0},  // Candy.
    {{9, 6, 6, 9, 0, 5, 2, 0, 1, 2}, PasswordRewardKind::kFood,
     static_cast<uint8_t>(FoodKind::kCone), 1, 35, 0},  // Popcorn.
    {{4, 2, 9, 7, 8, 0, 7, 6, 1, 9}, PasswordRewardKind::kFood,
     static_cast<uint8_t>(FoodKind::kPudding), 1, 36, 0},  // Gum.
    {{8, 8, 7, 0, 4, 4, 5, 5, 0, 2}, PasswordRewardKind::kFood,
     static_cast<uint8_t>(FoodKind::kTart), 1, 37, 0},   // Dango.
    {{2, 0, 7, 5, 4, 2, 9, 7, 7, 7}, PasswordRewardKind::kFood,
     static_cast<uint8_t>(FoodKind::kTart), 1, 38, 0},   // Cream Puff.
    {{9, 8, 4, 7, 6, 8, 2, 7, 8, 5}, PasswordRewardKind::kFood,
     static_cast<uint8_t>(FoodKind::kPudding), 1, 39, 0},  // Energy Drink.
    {{4, 1, 4, 7, 5, 7, 8, 3, 8, 6}, PasswordRewardKind::kFood,
     static_cast<uint8_t>(FoodKind::kSushi), 1, 40, 0},   // Sausage.
    {{1, 4, 7, 7, 0, 2, 0, 5, 3, 1}, PasswordRewardKind::kFood,
     static_cast<uint8_t>(FoodKind::kApple), 1, 41, 0},  // Cherry.
    {{6, 2, 0, 1, 7, 2, 4, 4, 2, 3}, PasswordRewardKind::kItem,
     static_cast<uint8_t>(ItemKind::kBook), 1, 48, 0},   // Pencil.
    {{2, 0, 2, 5, 4, 4, 7, 2, 9, 8}, PasswordRewardKind::kItem,
     static_cast<uint8_t>(ItemKind::kCharm), 1, 49, 0},  // Plant.
    {{2, 1, 9, 4, 6, 5, 3, 5, 7, 5}, PasswordRewardKind::kItem,
     static_cast<uint8_t>(ItemKind::kCharm), 1, 50, 0},  // Umbrella.
    {{8, 8, 5, 6, 6, 7, 9, 8, 6, 5}, PasswordRewardKind::kItem,
     static_cast<uint8_t>(ItemKind::kCharm), 1, 51, 0},  // Sunglasses.
    {{9, 2, 4, 8, 0, 0, 2, 3, 8, 9}, PasswordRewardKind::kItem,
     static_cast<uint8_t>(ItemKind::kCharm), 1, 52, 0},  // Bow.
    {{5, 4, 6, 4, 6, 2, 2, 5, 3, 8}, PasswordRewardKind::kItem,
     static_cast<uint8_t>(ItemKind::kTrumpet), 1, 53, 0},  // Drum.
    {{9, 8, 8, 4, 7, 3, 6, 5, 6, 8}, PasswordRewardKind::kItem,
     static_cast<uint8_t>(ItemKind::kToy), 1, 54, 0},    // Shovel.
    {{4, 1, 7, 7, 9, 1, 2, 7, 0, 9}, PasswordRewardKind::kItem,
     static_cast<uint8_t>(ItemKind::kToy), 1, 55, 0},    // Roller Blades.
    {{7, 1, 8, 3, 8, 3, 9, 2, 7, 1}, PasswordRewardKind::kItem,
     static_cast<uint8_t>(ItemKind::kToy), 1, 56, 0},    // Balloon.
    {{1, 3, 7, 8, 7, 5, 0, 1, 7, 6}, PasswordRewardKind::kItem,
     static_cast<uint8_t>(ItemKind::kCharm), 1, 57, 0},  // Bow Tie.
    {{0, 1, 4, 5, 2, 7, 4, 8, 1, 1}, PasswordRewardKind::kItem,
     static_cast<uint8_t>(ItemKind::kCharm), 1, 58, 0},  // Cap.
    {{6, 5, 8, 0, 1, 6, 4, 8, 2, 8}, PasswordRewardKind::kItem,
     static_cast<uint8_t>(ItemKind::kTrumpet), 1, 59, 0},  // Music.
    {{7, 5, 6, 4, 2, 9, 3, 0, 2, 6}, PasswordRewardKind::kItem,
     static_cast<uint8_t>(ItemKind::kBall), 1, 60, 0},   // Ball.
    {{2, 5, 0, 9, 4, 5, 7, 5, 3, 7}, PasswordRewardKind::kItem,
     static_cast<uint8_t>(ItemKind::kTicket), 1, 61, 0},  // Ticket 2.
};

// A=0, B=1, C=2. Source: 2024 Connection secret-code lists.
const SecretCodeReward kSecretCodeRewards[] = {
    {{0, 1, 1, 0, 0, 2, 1, 0}, 144},  // ABBAACBA: Cake.
    {{1, 2, 0, 1, 0, 2, 1, 2}, 145},  // BCABACBC: Steak.
    {{2, 1, 0, 2, 2, 0, 1, 2}, 146},  // CBACCABC: Cuckoo Clock.
    {{0, 2, 1, 1, 1, 0, 2, 2}, 147},  // ACBBBACC: Hair Gel.
    {{1, 2, 1, 2, 2, 0, 1, 0}, 148},  // BCBCCABA: Love Potion/Honey.
    {{0, 0, 1, 1, 2, 0, 2, 1}, 149},  // AABBCACB: RC Car.
    {{2, 0, 2, 0, 1, 0, 1, 2}, 150},  // CACABABC: Stuffed Tama/Nyatchi.
};

const GameRewardRule kGameRewardRules[] = {
    {Stage::kBaby, MiniGameKind::kGet, 100, 0, 0},
    {Stage::kChild, MiniGameKind::kGet, 150, 0, 0},
    {Stage::kChild, MiniGameKind::kBump, 1800, 0, 0},
    {Stage::kTeen, MiniGameKind::kGet, 225, 0, 0},
    {Stage::kTeen, MiniGameKind::kBump, 1800, 0, 0},
    {Stage::kTeen, MiniGameKind::kFlag, 300, 0, 0},
    {Stage::kTeen, MiniGameKind::kHeading, 400, 0, 0},
    {Stage::kAdult, MiniGameKind::kGet, 300, 0, 0},
    {Stage::kAdult, MiniGameKind::kBump, 1200, 0, 0},
    {Stage::kAdult, MiniGameKind::kFlag, 600, 0, 0},
    {Stage::kAdult, MiniGameKind::kHeading, 400, 0, 0},
    {Stage::kAdult, MiniGameKind::kMemory, 400, 0, 0},
    {Stage::kAdult, MiniGameKind::kSprint, 400, 0, 0},
    {Stage::kAdult, MiniGameKind::kHoops, 100, 0, 0},
    {Stage::kElder, MiniGameKind::kGet, 300, 0, 0},
    {Stage::kElder, MiniGameKind::kBump, 600, 0, 0},
    {Stage::kElder, MiniGameKind::kFlag, 400, 0, 0},
    {Stage::kElder, MiniGameKind::kHeading, 100, 0, 0},
};

const uint8_t kGameScorePrizePercent[6] = {0, 8, 22, 45, 72, 100};

const GameUnlockRule kGameUnlockRules[] = {
    {Stage::kEgg, 0},
    {Stage::kBaby, 1},
    {Stage::kChild, 2},
    {Stage::kTeen, 4},
    {Stage::kAdult, 7},
    {Stage::kParentCare, 7},
    {Stage::kElder, 4},
};

const GamePacingRule kGamePacingRules[] = {
    {MiniGameKind::kGet, 100, 100, 100, kGetRoundWindowMs,
     kGamePacingTimedAuto},
    {MiniGameKind::kBump, 8, 8, 8, 0, kGamePacingAnyButton},
    {MiniGameKind::kFlag, 9, 9, 9, kFlagRoundWindowMs,
     kGamePacingTimedAuto},
    {MiniGameKind::kHeading, 20, 20, 20, 0, 0},
    {MiniGameKind::kMemory, 8, 20, 20, 0, 0},
    {MiniGameKind::kSprint, 8, 8, 8, 0, kGamePacingAnyButton},
    {MiniGameKind::kHoops, 30, 30, 30, kHoopsShotWindowMs,
     kGamePacingTimedAuto},
};

const BumpPrizeRow kBumpPrizeChart[] = {
    {0, {5, 5, 5, 5}},
    {1, {80, 60, 40, 20}},
    {2, {160, 120, 80, 40}},
    {3, {240, 180, 120, 60}},
    {4, {320, 240, 160, 80}},
    {5, {400, 300, 180, 100}},
    {6, {600, 450, 300, 150}},
    {7, {1200, 900, 600, 300}},
    {8, {2400, 1800, 1200, 600}},
};

const CatalogRandomRule kCatalogRandomRules[] = {
    {kCatalogPlantIndex, 3, kPlantPointReward, 0},
    {kCatalogShovelIndex, 3, kShovelPointReward, 0},
    {kCatalogChestIndex, 6, kChestPointReward, 0},
    {kCatalogFishingPoleIndex, 4, kFishingPolePointReward, 0},
    {kCatalogLampIndex, 6, kLampPointReward, 25},
};

const SicknessTriggerRule kSicknessTriggerRules[] = {
    {SicknessTriggerKind::kLowHygiene, 18, 8, 1, 2},
    {SicknessTriggerKind::kMess, 3, 18, 1, 2},
    {SicknessTriggerKind::kHighWeight, 35, 8, 1, 2},
    {SicknessTriggerKind::kSweetSnack, 16, 9, 1, 1},
};

uint8_t clampAddTo(uint8_t value, int delta, uint8_t limit) {
  int next = static_cast<int>(value) + delta;
  if (next < 0) return 0;
  if (next > limit) return limit;
  return static_cast<uint8_t>(next);
}

uint8_t clampAdd(uint8_t value, int delta) {
  return clampAddTo(value, delta, 100);
}

uint8_t clampAddHungryHappy(uint8_t value, int delta) {
  return clampAddTo(value, delta, kHiddenHeartMeterMax);
}

uint8_t visibleHeartMeterValue(uint8_t value) {
  return value > kVisibleHeartMeterMax ? kVisibleHeartMeterMax : value;
}

uint16_t clampAdd16(uint16_t value, int delta, uint16_t limit) {
  int32_t next = static_cast<int32_t>(value) + delta;
  if (next < 0) return 0;
  if (next > limit) return limit;
  return static_cast<uint16_t>(next);
}

uint8_t foodIndex(FoodKind food) {
  return static_cast<uint8_t>(food);
}

bool isSnackFood(FoodKind food) {
  return foodIndex(food) >= kDefaultMealCount;
}

uint8_t itemIndex(ItemKind item) {
  return static_cast<uint8_t>(item);
}

uint8_t guardedSickChance(uint8_t chance, bool tamaDrinkGuard) {
  return tamaDrinkGuard ? static_cast<uint8_t>((chance + 1) / 2) : chance;
}

const SicknessTriggerRule& sicknessTriggerRuleFor(
    SicknessTriggerKind kind) {
  static const SicknessTriggerRule kFallback = {
      SicknessTriggerKind::kLowHygiene, 0, 0, 1, 1};
  for (const SicknessTriggerRule& rule : kSicknessTriggerRules) {
    if (rule.kind == kind) return rule;
  }
  return kFallback;
}

bool stageCanGetToothache(Stage stage) {
  return stage == Stage::kChild || stage == Stage::kTeen;
}

uint16_t petMinuteStamp(uint32_t ageMinutes) {
  return static_cast<uint16_t>(ageMinutes & 0xFFFFU);
}

bool sweetSnackWithinShortPeriod(uint16_t previous, uint16_t current) {
  if (previous == kSweetSnackNoLastMinute) return false;
  return static_cast<uint16_t>(current - previous) <=
         kSweetSnackShortPeriodMinutes;
}

bool actionBlockedBySickness(Action action) {
  return action == Action::kMeal || action == Action::kSnack ||
         action == Action::kGame || action == Action::kItem;
}

uint8_t ticketSouvenirForCatalogIndex(uint8_t index) {
  switch (index) {
    case kCatalogTicket1Index:
      return 9;   // Souvenir 10: Skis.
    case kCatalogTicket2Index:
      return 10;  // Souvenir 11: Palm Tree.
    case kCatalogTicket3Index:
      return 11;  // Souvenir 12: Surfboard.
    case kCatalogTicket4Index:
      return 12;  // Souvenir 13: Panda Bear.
    case kCatalogTicket5Index:
      return 13;  // Souvenir 14: Maracas.
    default:
      return 0xFF;
  }
}

uint8_t chestLampExclusiveCatalogItem(uint8_t roll) {
  const uint8_t kExclusiveItems[] = {
      kCatalogDoll2Index,
      kCatalogMakeupIndex,
      kCatalogShaverIndex,
      kCatalogTamaDrinkIndex,
  };
  return kExclusiveItems[roll % (sizeof(kExclusiveItems) /
                                 sizeof(kExclusiveItems[0]))];
}

const CatalogRandomRule& catalogRandomRuleFor(uint8_t catalogIndex) {
  static const CatalogRandomRule kFallback = {0xFF, 1, 0, 0};
  for (const CatalogRandomRule& rule : kCatalogRandomRules) {
    if (rule.catalogIndex == catalogIndex) return rule;
  }
  return kFallback;
}

uint8_t gameValue(MiniGameKind game) {
  return static_cast<uint8_t>(game);
}

uint8_t linkGameValue(LinkGameKind game) {
  return static_cast<uint8_t>(game);
}

bool catalogOwnedAny(const uint8_t* catalogOwned, const uint8_t* indices,
                     uint8_t count) {
  if (!catalogOwned) return false;
  for (uint8_t i = 0; i < count; i++) {
    if (catalogBitTest(catalogOwned, indices[i])) return true;
  }
  return false;
}

bool linkGameOwnedInBits(const uint8_t* catalogOwned, LinkGameKind game) {
  switch (game) {
    case LinkGameKind::kGotchiPoint:
      return true;
    case LinkGameKind::kBall: {
      const uint8_t indices[] = {kCatalogBallIndex};
      return catalogOwnedAny(catalogOwned, indices,
                             sizeof(indices) / sizeof(indices[0]));
    }
    case LinkGameKind::kRcCar: {
      const uint8_t indices[] = {kCatalogRcCar1Index, kCatalogRcCar2Index,
                                 kCatalogRcCar3Index};
      return catalogOwnedAny(catalogOwned, indices,
                             sizeof(indices) / sizeof(indices[0]));
    }
    case LinkGameKind::kRope: {
      const uint8_t indices[] = {kCatalogRopeIndex};
      return catalogOwnedAny(catalogOwned, indices,
                             sizeof(indices) / sizeof(indices[0]));
    }
    case LinkGameKind::kBuildingBlock: {
      const uint8_t indices[] = {kCatalogBuildingBlockIndex};
      return catalogOwnedAny(catalogOwned, indices,
                             sizeof(indices) / sizeof(indices[0]));
    }
    case LinkGameKind::kBalloon: {
      const uint8_t indices[] = {kCatalogBalloonIndex};
      return catalogOwnedAny(catalogOwned, indices,
                             sizeof(indices) / sizeof(indices[0]));
    }
    case LinkGameKind::kTrumpet: {
      const uint8_t indices[] = {kCatalogTrumpetAIndex, kCatalogTrumpetBIndex};
      return catalogOwnedAny(catalogOwned, indices,
                             sizeof(indices) / sizeof(indices[0]));
    }
    case LinkGameKind::kNone:
      break;
  }
  return false;
}

uint16_t linkGamePrize(LinkGameKind game, bool win) {
  switch (game) {
    case LinkGameKind::kGotchiPoint:
      return win ? 200 : 50;
    case LinkGameKind::kBall:
    case LinkGameKind::kRcCar:
    case LinkGameKind::kRope:
    case LinkGameKind::kBuildingBlock:
    case LinkGameKind::kBalloon:
    case LinkGameKind::kTrumpet:
      return win ? 120 : 30;
    case LinkGameKind::kNone:
      break;
  }
  return 0;
}

const GamePacingRule& gamePacingRuleFor(MiniGameKind game) {
  static const GamePacingRule kFallback = {MiniGameKind::kNone, 0, 0, 0, 0, 0};
  for (const GamePacingRule& rule : kGamePacingRules) {
    if (rule.game == game) return rule;
  }
  return kFallback;
}

uint8_t gameRoundLimitFor(MiniGameKind game) {
  return gamePacingRuleFor(game).roundLimit;
}

uint8_t gameScoreLimitFor(MiniGameKind game) {
  return gamePacingRuleFor(game).scoreLimit;
}

uint8_t sourceRoundsForCompressedScore(MiniGameKind game, uint8_t score) {
  const GamePacingRule& rule = gamePacingRuleFor(game);
  if (!(rule.flags & kGamePacingEpaperCompressed) || rule.sourceGoal == 0 ||
      rule.scoreLimit == 0) {
    return score;
  }
  const uint16_t scaled =
      static_cast<uint16_t>(score) * rule.sourceGoal + rule.scoreLimit / 2;
  uint8_t rounds = static_cast<uint8_t>(scaled / rule.scoreLimit);
  if (rounds > rule.sourceGoal) rounds = rule.sourceGoal;
  return rounds;
}

uint8_t scaledProgress(uint8_t value, uint8_t limit, uint8_t span) {
  if (limit <= 1 || span == 0) return 0;
  if (value >= limit) value = static_cast<uint8_t>(limit - 1);
  return static_cast<uint8_t>(
      (static_cast<uint16_t>(value) * span) / (limit - 1));
}

uint8_t gameScoreTier(uint8_t score, uint8_t scoreLimit) {
  if (scoreLimit == 0) return 0;
  if (score >= scoreLimit) return 5;
  return static_cast<uint8_t>(
      (static_cast<uint16_t>(score) * 5U + scoreLimit / 2U) / scoreLimit);
}

uint16_t gameAutoWindowFor(MiniGameKind game) {
  return gamePacingRuleFor(game).autoWindowMs;
}

uint8_t buttonValue(GameButton button) {
  return static_cast<uint8_t>(button);
}

uint8_t memorySequenceAt(uint32_t sequence, uint8_t index) {
  return static_cast<uint8_t>((sequence >> (index * 2)) & 0x03);
}

uint8_t memoryPatternLengthForRound(uint8_t round) {
  static const uint8_t kPatternLengths[] = {
      2, 2, 2, 3, 3, 3, 4, 4, 4, 5,
      5, 5, 6, 6, 6, 7, 7, 7, 8, 8};
  const uint8_t last =
      static_cast<uint8_t>((sizeof(kPatternLengths) / sizeof(kPatternLengths[0])) - 1);
  return kPatternLengths[round < last ? round : last];
}

uint16_t baseWeightForStage(Stage stage) {
  for (const StageBaseWeightRule& rule : kStageBaseWeightRules) {
    if (rule.stage == stage) return rule.baseWeight;
  }
  return 5;
}

const CharacterFoodTasteRule* characterFoodTasteRuleFor(uint8_t catalogId) {
  for (const CharacterFoodTasteRule& rule : kCharacterFoodTasteRules) {
    if (rule.characterCatalogId == catalogId) return &rule;
  }
  return nullptr;
}

bool catalogFoodLikedByAll(uint8_t catalogIndex) {
  return catalogIndex == kCatalogYogurtIndex ||
         catalogIndex == kCatalogSteakIndex;
}

const StageCareTimingRule& careTimingForStage(Stage stage) {
  for (const StageCareTimingRule& rule : kStageCareTimingRules) {
    if (rule.stage == stage) return rule;
  }
  return kStageCareTimingRules[0];
}

const SleepWindowRule& sleepWindowForStage(Stage stage) {
  for (const SleepWindowRule& rule : kSleepWindowRules) {
    if (rule.stage == stage) return rule;
  }
  return kSleepWindowRules[0];
}

const AttentionTimingRule& attentionTimingRuleFor(
    AttentionTimingKind kind) {
  static const AttentionTimingRule kFallback = {
      AttentionTimingKind::kOrdinaryMiss, 1, 0};
  for (const AttentionTimingRule& rule : kAttentionTimingRules) {
    if (rule.kind == kind) return rule;
  }
  return kFallback;
}

Stage scheduledStageForAge(Stage current, uint8_t growthFlags,
                           uint32_t ageMinutes) {
  if ((growthFlags & kGrowthFlagOyajitchiLineage) &&
      ageMinutes >= kBabyToChildAgeMinutes) {
    return Stage::kAdult;
  }
  for (const GrowthScheduleRule& rule : kGrowthScheduleRules) {
    if (ageMinutes >= rule.minAgeMinutes) return rule.result;
  }
  return current == Stage::kEgg ? Stage::kEgg : current;
}

bool matchmakerNoticeReady(uint32_t ageMinutes, uint16_t clockMinutes) {
  for (const MatchmakerClockRule& rule : kMatchmakerClockRules) {
    if (ageMinutes >= rule.minAgeMinutes && clockMinutes == rule.clockMinute) {
      return true;
    }
  }
  return false;
}

uint16_t statDecayPeriodForStage(StatMeter meter, Stage stage,
                                 uint16_t fallbackMinutes) {
  const StageCareTimingRule& timing = careTimingForStage(stage);
  switch (meter) {
    case StatMeter::kHunger:
      return timing.hungerMinutes ? timing.hungerMinutes : fallbackMinutes;
    case StatMeter::kHappiness:
      return timing.happinessMinutes ? timing.happinessMinutes
                                     : fallbackMinutes;
    case StatMeter::kEnergy:
    case StatMeter::kHygiene:
      return fallbackMinutes;
  }
  return fallbackMinutes;
}

uint16_t messPeriodForStage(Stage stage) {
  const StageCareTimingRule& timing = careTimingForStage(stage);
  return timing.messMinutes;
}

uint8_t laneLeft(uint8_t lane) {
  return lane == 0 ? 0 : static_cast<uint8_t>(lane - 1);
}

uint8_t laneRight(uint8_t lane) {
  return lane >= 2 ? 2 : static_cast<uint8_t>(lane + 1);
}

Stage rewardStage(Stage stage) {
  return stage == Stage::kParentCare ? Stage::kAdult : stage;
}

bool validStage(uint8_t value) {
  return value <= static_cast<uint8_t>(Stage::kElder);
}

bool validRoute(uint8_t value) {
  return value <= static_cast<uint8_t>(GrowthRoute::kRascal);
}

bool validCharacter(uint8_t value) {
  return value <= static_cast<uint8_t>(CharacterKind::kSage);
}

uint8_t catalogIdForCharacterKind(CharacterKind kind) {
  for (uint8_t i = 0; i < kCharacterCatalogCount; i++) {
    if (characterCatalogEntry(i).archetype == kind) return i;
  }
  return 0;
}

bool validGrowthTier(uint8_t value) {
  return value <= static_cast<uint8_t>(GrowthTier::kTierFour);
}

bool validAdultTier(uint8_t value) {
  return value <= static_cast<uint8_t>(AdultTier::kSpecial);
}

bool validLinkKind(uint8_t value) {
  return value <= static_cast<uint8_t>(LinkKind::kLove);
}

bool validLinkGameKind(uint8_t value) {
  return value >= static_cast<uint8_t>(LinkGameKind::kGotchiPoint) &&
         value <= static_cast<uint8_t>(LinkGameKind::kTrumpet);
}

bool validGender(uint8_t value) {
  return value <= static_cast<uint8_t>(Gender::kGirl);
}

bool validRelation(uint8_t value) {
  return value <= static_cast<uint8_t>(RelationLevel::kPartner);
}

uint8_t capCareMistake(uint8_t value) {
  return value > kCareMistakeCap ? kCareMistakeCap : value;
}

GrowthTier tierFromStageMistakes(uint8_t physical, uint8_t mental) {
  const uint8_t total = static_cast<uint8_t>(physical + mental);
  if (total <= 1) return GrowthTier::kTierOne;
  if (total <= 3) return GrowthTier::kTierTwo;
  if (total <= 5) return GrowthTier::kTierThree;
  return GrowthTier::kTierFour;
}

bool sameAdultPair(AdultTier a, AdultTier b, AdultTier x, AdultTier y) {
  return (a == x && b == y) || (a == y && b == x);
}

GrowthTier childTierFromParentAdultTiers(AdultTier a, AdultTier b) {
  if (sameAdultPair(a, b, AdultTier::kSerious, AdultTier::kSerious) ||
      sameAdultPair(a, b, AdultTier::kSerious, AdultTier::kNormal)) {
    return GrowthTier::kTierOne;
  }
  if (sameAdultPair(a, b, AdultTier::kSerious, AdultTier::kNaughty) ||
      sameAdultPair(a, b, AdultTier::kSerious, AdultTier::kFrail) ||
      sameAdultPair(a, b, AdultTier::kSerious, AdultTier::kSpecial) ||
      sameAdultPair(a, b, AdultTier::kNormal, AdultTier::kNormal) ||
      sameAdultPair(a, b, AdultTier::kNormal, AdultTier::kNaughty)) {
    return GrowthTier::kTierTwo;
  }
  if (sameAdultPair(a, b, AdultTier::kNormal, AdultTier::kFrail) ||
      sameAdultPair(a, b, AdultTier::kNormal, AdultTier::kSpecial) ||
      sameAdultPair(a, b, AdultTier::kNaughty, AdultTier::kNaughty) ||
      sameAdultPair(a, b, AdultTier::kNaughty, AdultTier::kFrail)) {
    return GrowthTier::kTierThree;
  }
  return GrowthTier::kTierFour;
}

GrowthTier firstGenerationTeenTierFromMistakes(uint8_t physical,
                                               uint8_t mental) {
  const uint8_t worst = physical > mental ? physical : mental;
  if (worst <= 1) return GrowthTier::kTierOne;
  if (worst == 2) return GrowthTier::kTierTwo;
  if (worst == 3) return GrowthTier::kTierThree;
  return GrowthTier::kTierFour;
}

GrowthTier teenTierFromChildTier(GrowthTier childTier, uint8_t physical,
                                 uint8_t mental, uint16_t generation) {
  if (generation <= 1 || childTier == GrowthTier::kFirstGeneration) {
    return firstGenerationTeenTierFromMistakes(physical, mental);
  }
  switch (childTier) {
    case GrowthTier::kTierOne:
      return physical >= 3 && mental >= 3 ? GrowthTier::kTierTwo
                                          : GrowthTier::kTierOne;
    case GrowthTier::kTierTwo:
      return physical >= 3 && mental >= 3 ? GrowthTier::kTierThree
                                          : GrowthTier::kTierTwo;
    case GrowthTier::kTierThree:
      return physical >= 4 && mental >= 4 ? GrowthTier::kTierFour
                                          : GrowthTier::kTierThree;
    case GrowthTier::kTierFour:
      return GrowthTier::kTierFour;
    case GrowthTier::kFirstGeneration:
      break;
  }
  return firstGenerationTeenTierFromMistakes(physical, mental);
}

bool adultGrowthRuleMatches(const AdultGrowthRule& rule,
                            GrowthTier previousTier, uint8_t physical,
                            uint8_t mental, bool bornFromUnhealthyParents) {
  if (rule.previousTier != previousTier) return false;
  if (physical < rule.minPhysical || physical > rule.maxPhysical) return false;
  if (mental < rule.minMental || mental > rule.maxMental) return false;
  if ((rule.flags & kAdultGrowthRequiresHealthyParents) &&
      bornFromUnhealthyParents) {
    return false;
  }
  return true;
}

AdultTier adultTierFromStageMistakes(GrowthTier previousTier,
                                     uint8_t physical, uint8_t mental,
                                     bool bornFromUnhealthyParents) {
  const GrowthTier normalized =
      previousTier == GrowthTier::kFirstGeneration ? GrowthTier::kTierOne
                                                   : previousTier;
  for (const AdultGrowthRule& rule : kAdultGrowthRules) {
    if (adultGrowthRuleMatches(rule, normalized, physical, mental,
                               bornFromUnhealthyParents)) {
      return rule.result;
    }
  }
  return AdultTier::kNormal;
}

bool adultTierIsUnhealthy(AdultTier tier) {
  return tier == AdultTier::kNaughty || tier == AdultTier::kFrail ||
         tier == AdultTier::kSpecial;
}

uint8_t characterGenerationMask(uint16_t generation) {
  uint8_t mask = generation & 0x01 ? kCharacterGenerationOdd
                                   : kCharacterGenerationEven;
  if (generation <= 1) mask |= kCharacterGenerationFirst;
  return mask;
}

uint8_t characterGenderMask(Gender gender) {
  return gender == Gender::kGirl ? kCharacterGenderGirl : kCharacterGenderBoy;
}

uint8_t babyCatalogIdForGender(Gender gender) {
  const uint8_t mask = characterGenderMask(gender);
  for (uint8_t i = 0; i < kCharacterCatalogCount; i++) {
    const CharacterCatalogEntry& entry = characterCatalogEntry(i);
    if (entry.stage == Stage::kBaby && (entry.genderMask & mask)) {
      return i;
    }
  }
  return gender == Gender::kGirl ? 1 : 0;
}

uint8_t characterGrowthTierMask(GrowthTier tier) {
  switch (tier) {
    case GrowthTier::kTierOne:
      return kCharacterTierOne;
    case GrowthTier::kTierTwo:
      return kCharacterTierTwo;
    case GrowthTier::kTierThree:
      return kCharacterTierThree;
    case GrowthTier::kTierFour:
      return kCharacterTierFour;
    case GrowthTier::kFirstGeneration:
      break;
  }
  return kCharacterTierHealthy;
}

uint8_t characterAdultTierMask(AdultTier tier) {
  switch (tier) {
    case AdultTier::kSerious:
      return kCharacterTierOne;
    case AdultTier::kNormal:
      return kCharacterTierTwo;
    case AdultTier::kNaughty:
      return kCharacterTierThree;
    case AdultTier::kFrail:
      return kCharacterTierFour;
    case AdultTier::kSpecial:
      return kCharacterTierSpecial;
  }
  return kCharacterTierTwo;
}

uint8_t characterTierMaskForStage(Stage stage, uint16_t generation,
                                  GrowthTier growthTier,
                                  AdultTier adultTier) {
  switch (stage) {
    case Stage::kBaby:
    case Stage::kEgg:
    case Stage::kElder:
      return kCharacterTierAny;
    case Stage::kChild:
      if (generation <= 1 || growthTier == GrowthTier::kFirstGeneration) {
        return kCharacterTierAny;
      }
      return characterGrowthTierMask(growthTier);
    case Stage::kTeen:
      return characterGrowthTierMask(growthTier);
    case Stage::kAdult:
    case Stage::kParentCare:
      return characterAdultTierMask(adultTier);
  }
  return kCharacterTierAny;
}

bool characterSourcePoolMatches(const CharacterCatalogEntry& entry,
                                Stage stage, uint8_t generationMask,
                                uint8_t tierMask, uint8_t genderMask) {
  return entry.stage == stage &&
         (entry.generationMask & generationMask) != 0 &&
         (entry.tierMask & tierMask) != 0 &&
         (entry.genderMask & genderMask) != 0;
}

bool valueAtOrBelow(uint8_t value, uint8_t maxValue) {
  return maxValue == kPassAwayDontCare || value <= maxValue;
}

bool passAwayRuleMatches(const PassAwayRule& rule, Stage stage,
                         uint32_t ageMinutes, uint16_t careMistakes,
                         uint8_t sickness, uint8_t hunger, uint8_t happiness,
                         uint8_t hygiene, RelationLevel relation) {
  if (careMistakes < rule.minCareMistakes) return false;
  if (ageMinutes < rule.minAgeMinutes) return false;
  if ((rule.flags & kPassAwayNeedsSick) && !sickness) return false;
  if ((rule.flags & kPassAwayNeedsElder) && stage != Stage::kElder) {
    return false;
  }
  if ((rule.flags & kPassAwayNeedsLowFriend) &&
      relation >= RelationLevel::kFriend) {
    return false;
  }
  if (rule.flags & kPassAwayNeedsAnyLowStat) {
    return hunger <= rule.maxHunger || happiness <= rule.maxHappiness ||
           hygiene <= rule.maxHygiene;
  }
  return valueAtOrBelow(hunger, rule.maxHunger) &&
         valueAtOrBelow(happiness, rule.maxHappiness) &&
         valueAtOrBelow(hygiene, rule.maxHygiene);
}

bool decodeCatalogPassword(const uint8_t digits[10], uint8_t& index) {
  if (digits[0] != 9 || digits[1] != 9 || digits[5] != 2 || digits[6] != 0 ||
      digits[7] != 2 || digits[8] != 6) {
    return false;
  }
  const uint16_t raw =
      static_cast<uint16_t>(digits[2]) * 100U +
      static_cast<uint16_t>(digits[3]) * 10U + digits[4];
  if (raw >= kCatalogItemCount) {
    return false;
  }
  const uint8_t check = static_cast<uint8_t>((raw * 7U + 6U) % 10U);
  if (digits[9] != check) {
    return false;
  }
  index = static_cast<uint8_t>(raw);
  return true;
}

uint16_t fullPrizeFor(Stage stage, MiniGameKind game, uint16_t weight) {
  if (game == MiniGameKind::kNone) return 0;
  const Stage normalized = rewardStage(stage);
  for (const GameRewardRule& rule : kGameRewardRules) {
    if (rule.stage != normalized || rule.game != game) continue;
    if (rule.heavyPrize && weight >= rule.heavyWeight) return rule.heavyPrize;
    return rule.prize;
  }
  return 0;
}

uint8_t bumpWeightBand(uint16_t weight) {
  if (weight <= 50) return 0;
  if (weight <= 75) return 1;
  if (weight <= 90) return 2;
  return 3;
}

uint16_t bumpPrizeForScore(uint8_t score, uint16_t weight) {
  const uint8_t sourceRounds =
      sourceRoundsForCompressedScore(MiniGameKind::kBump, score);
  const uint8_t band = bumpWeightBand(weight);
  for (const BumpPrizeRow& row : kBumpPrizeChart) {
    if (row.sourceRoundsWon == sourceRounds) {
      return row.prizeByWeightBand[band];
    }
  }
  return 0;
}

uint16_t gamePrizeForScore(MiniGameKind game, uint16_t fullPrize, uint8_t score,
                           uint16_t weight) {
  if (game == MiniGameKind::kBump) {
    return bumpPrizeForScore(score, weight);
  }
  const uint8_t tier = gameScoreTier(score, gameScoreLimitFor(game));
  return static_cast<uint16_t>(
      (static_cast<uint32_t>(fullPrize) * kGameScorePrizePercent[tier] +
       50U) /
      100U);
}

uint8_t sprintTapTargetFor(uint16_t weight) {
  if (weight <= 20) return 2;
  if (weight <= 40) return 3;
  if (weight <= 65) return 4;
  return 5;
}

uint8_t sprintWinChanceFor(uint16_t weight, uint8_t round) {
  int chance = 93 - static_cast<int>(weight / 2U) - static_cast<int>(round * 5U);
  if (chance < 35) chance = 35;
  if (chance > 94) chance = 94;
  return static_cast<uint8_t>(chance);
}

uint8_t ownedSecretCodeCount(const uint8_t* bits) {
  uint8_t count = 0;
  for (uint8_t i = 0; i < kCatalogSecretCount; i++) {
    const uint8_t index = static_cast<uint8_t>(kCatalogSecretFirst + i);
    if (catalogBitTest(bits, index)) count++;
  }
  return count;
}

bool actionAddressesAttention(Action action, AttentionReason reason) {
  switch (reason) {
    case AttentionReason::kHungry:
      return action == Action::kMeal || action == Action::kSnack;
    case AttentionReason::kSad:
      return action == Action::kSnack || action == Action::kGame ||
             action == Action::kItem || action == Action::kPraise;
    case AttentionReason::kDirty:
      return action == Action::kToilet;
    case AttentionReason::kSick:
      return action == Action::kMedicine;
    case AttentionReason::kNaughty:
      return action == Action::kDiscipline;
    case AttentionReason::kPraise:
      return action == Action::kPraise;
    case AttentionReason::kNone:
      return false;
  }
  return false;
}

void copySetupName(char* dest, const char* source, const char* fallback) {
  for (uint8_t i = 0; i < kNameChars; i++) {
    char c = source && source[i] ? source[i] : fallback[i];
    if (c >= 'a' && c <= 'z') c = static_cast<char>(c - 'a' + 'A');
    const bool digit = c >= '0' && c <= '9';
    const bool upper = c >= 'A' && c <= 'Z';
    if (!digit && !upper) c = ' ';
    dest[i] = c;
  }
  dest[kNameChars] = '\0';
}

uint8_t clampMonth(uint8_t value) {
  if (value < 1) return 1;
  return value > 12 ? 12 : value;
}

uint8_t daysInMonth(uint8_t month) {
  static const uint8_t kDays[] = {
      31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,
  };
  month = clampMonth(month);
  return kDays[month - 1];
}

uint8_t clampDay(uint8_t value, uint8_t month) {
  if (value < 1) return 1;
  const uint8_t maxDay = daysInMonth(month);
  return value > maxDay ? maxDay : value;
}

uint16_t dayOfYear(uint8_t month, uint8_t day) {
  month = clampMonth(month);
  day = clampDay(day, month);
  uint16_t total = 0;
  for (uint8_t m = 1; m < month; m++) {
    total += daysInMonth(m);
  }
  return static_cast<uint16_t>(total + day);
}

bool dateInRange(uint8_t month, uint8_t day, uint8_t startMonth,
                 uint8_t startDay, uint8_t endMonth, uint8_t endDay) {
  const uint16_t current = dayOfYear(month, day);
  const uint16_t start = dayOfYear(startMonth, startDay);
  const uint16_t end = dayOfYear(endMonth, endDay);
  if (start <= end) {
    return current >= start && current <= end;
  }
  return current >= start || current <= end;
}

uint8_t calendarSlotMaskForMinute(uint32_t clockMinutes) {
  const uint16_t minute = static_cast<uint16_t>(clockMinutes % 1440UL);
  switch (minute) {
    case 0:
      return kCalendarSlotMidnight;
    case 11 * 60 + 30:
      return kCalendarSlotMorning;
    case 14 * 60:
      return kCalendarSlotAfternoon;
    case 16 * 60 + 30:
      return kCalendarSlotEvening;
    case 22 * 60 + 30:
      return kCalendarSlotLate;
    default:
      return 0;
  }
}

uint16_t calendarEventKey(uint8_t month, uint8_t day, uint8_t slotMask) {
  return static_cast<uint16_t>(dayOfYear(month, day) * 32U + slotMask);
}

}  // namespace

uint32_t checksumSaveBytes(const PetSave& save, size_t size);

void EchoPetModel::begin(uint32_t nowMs) {
  lastMs_ = nowMs;
  minuteRemainderMs_ = 0;
  updateGrowth();
  updateMood();
}

void EchoPetModel::reset(uint32_t seed, uint32_t nowMs) {
  rngState_ = seed ? seed : 0xC0FFEE12UL;
  petId_ = (seed ^ 0x45504354UL) ? (seed ^ 0x45504354UL) : 0x45504354UL;
  lastMs_ = nowMs;
  minuteRemainderMs_ = 0;
  generation_ = 1;
  points_ = 300;
  donations_ = 0;
  souvenirMask_ = 0;
  memset(catalogOwned_, 0, sizeof(catalogOwned_));
  memset(catalogStock_, 0, sizeof(catalogStock_));
  memset(souvenirOwned_, 0, sizeof(souvenirOwned_));
  friendCount_ = 0;
  memset(friends_, 0, sizeof(friends_));
  memset(family_, 0, sizeof(family_));
  memset(familyAncestry_, 0, sizeof(familyAncestry_));
  gender_ = (seed & 0x01) ? Gender::kGirl : Gender::kBoy;
  makeDefaultNames();
  resetPetBody(seed, false);
  notice_ = Notice::kBorn;
}

void EchoPetModel::resetPetBody(uint32_t seed, bool preserveSocial) {
  const uint8_t keptGrowthFlags =
      preserveSocial ? (growthFlags_ & kGrowthFlagOyajitchiLineage) : 0;
  ageMinutes_ = 0;
  clockMinutes_ = 9 * 60;
  month_ = 1;
  day_ = 1;
  birthdayMonth_ = 6;
  birthdayDay_ = 16;
  parentCareMinutes_ = 0;
  lastAnniversaryDay_ = 0xFFFF;
  weight_ = 5;
  careMistakes_ = 0;
  physicalCareMistakes_ = 0;
  mentalCareMistakes_ = 0;
  growthTier_ = generation_ <= 1 ? GrowthTier::kFirstGeneration
                                 : GrowthTier::kTierOne;
  adultTier_ = AdultTier::kNormal;
  if (!preserveSocial) {
    parentAdultTierA_ = AdultTier::kNormal;
    parentAdultTierB_ = AdultTier::kNormal;
    parentCatalogIdA_ = kCatalogIdUnknown;
    parentCatalogIdB_ = kCatalogIdUnknown;
    bornFromUnhealthyParents_ = 0;
  }
  growthFlags_ = keptGrowthFlags;
  hunger_ = 78;
  happiness_ = 72;
  energy_ = 80;
  hygiene_ = 90;
  discipline_ = 12;
  friendship_ = preserveSocial ? friendship_ : 20;
  messCount_ = 0;
  sickness_ = 0;
  toothache_ = 0;
  stageSicknesses_ = 0;
  attention_ = 0;
  attentionMinutes_ = 0;
  attentionReason_ = AttentionReason::kNone;
  lightsOff_ = 0;
  soundOn_ = 1;

  for (uint8_t i = 0; i < kFoodKindCount; i++) foodCounts_[i] = 0;
  for (uint8_t i = 0; i < kItemKindCount; i++) itemCounts_[i] = 0;
  itemCounts_[itemIndex(ItemKind::kToy)] = 1;
  awardCatalogItem(0);
  awardCatalogItem(24);
  awardCatalogItem(48);

  gameWins_ = 0;
  gamePlays_ = 0;
  gameStreak_ = 0;
  bestGameScore_ = 0;
  friendVisits_ = preserveSocial ? friendVisits_ : 0;
  memoryFlags_ =
      preserveSocial
          ? (memoryFlags_ & ~(kMemoryFlagHoneyPending |
                              kMemoryFlagTamaDrinkGuard |
                              kMemoryFlagCostumeMask))
          : 0;
  careScore_ = 12;
  playScore_ = 0;
  socialScore_ = preserveSocial ? socialScore_ : 0;
  disciplineScore_ = 0;
  snackScore_ = 0;
  sweetSnackStreak_ = 0;
  lastSweetSnackMinute_ = kSweetSnackNoLastMinute;
  lastGameKind_ = 0;
  linkSequence_ = 0;
  gameRound_ = 0;
  gameScore_ = 0;
  gameExpectedYes_ = 0;
  gameCursor_ = 1;
  gameTarget_ = 1;
  gameHazard_ = 0;
  gameRoundStartMs_ = 0;
  lifeState_ = LifeState::kAlive;
  route_ = GrowthRoute::kBalanced;
  character_ = CharacterKind::kShell;
  characterCatalogId_ = 0;
  stage_ = Stage::kEgg;
  mood_ = Mood::kOkay;
  miniGame_ = MiniGameKind::kNone;
  gamePrompt_ = GamePrompt::kNone;
  lastLinkKind_ = LinkKind::kVisit;
  rngState_ ^= seed + 0x9E3779B9UL;
}

void EchoPetModel::configureSetup(uint8_t month, uint8_t day, uint8_t hour,
                                  uint8_t minute, uint8_t birthdayMonth,
                                  uint8_t birthdayDay, const char* userName,
                                  const char* petName, Gender gender) {
  month_ = clampMonth(month);
  day_ = clampDay(day, month_);
  birthdayMonth_ = clampMonth(birthdayMonth);
  birthdayDay_ = clampDay(birthdayDay, birthdayMonth_);
  clockMinutes_ = static_cast<uint16_t>((hour % 24) * 60 + (minute % 60));
  gender_ = gender;
  copySetupName(nickname_, userName, "USER1");
  copySetupName(petName_, petName, "MAME1");
  notice_ = Notice::kBorn;
  updateMood();
}

bool EchoPetModel::setClock(uint8_t hour, uint8_t minute) {
  const uint32_t next =
      static_cast<uint32_t>((hour % 24) * 60 + (minute % 60));
  if (clockMinutes_ == next) {
    notice_ = Notice::kStatus;
    return false;
  }
  clockMinutes_ = next;
  notice_ = Notice::kStatus;
  updateMood();
  return true;
}

bool EchoPetModel::load(const PetSave& save, uint32_t nowMs) {
  if (save.magic != kMagic || save.version < kMinReadableVersion ||
      save.version > kVersion ||
      save.size < kPetSaveV6Size || save.size > sizeof(PetSave) ||
      checksumSaveBytes(save, save.size) != save.checksum) {
    return false;
  }

  ageMinutes_ = save.ageMinutes;
  clockMinutes_ = save.clockMinutes;
  rngState_ = save.rngState ? save.rngState : 0xC0FFEE12UL;
  petId_ = save.petId ? save.petId : (rngState_ ^ 0x45504354UL);
  lastFriendId_ = save.lastFriendId;
  generation_ = save.generation ? save.generation : 1;
  parentCareMinutes_ = save.parentCareMinutes;
  lastAnniversaryDay_ = save.lastAnniversaryDay;
  weight_ = save.weight;
  careMistakes_ = save.careMistakes;
  if (save.version >= 5) {
    physicalCareMistakes_ = capCareMistake(save.physicalCareMistakes);
    mentalCareMistakes_ = capCareMistake(save.mentalCareMistakes);
    growthTier_ = validGrowthTier(save.growthTier)
                      ? static_cast<GrowthTier>(save.growthTier)
                      : GrowthTier::kFirstGeneration;
    adultTier_ = validAdultTier(save.adultTier)
                     ? static_cast<AdultTier>(save.adultTier)
                     : AdultTier::kNormal;
    parentAdultTierA_ = validAdultTier(save.parentAdultTierA)
                            ? static_cast<AdultTier>(save.parentAdultTierA)
                            : AdultTier::kNormal;
    parentAdultTierB_ = validAdultTier(save.parentAdultTierB)
                            ? static_cast<AdultTier>(save.parentAdultTierB)
                            : AdultTier::kNormal;
    bornFromUnhealthyParents_ = save.bornFromUnhealthyParents ? 1 : 0;
    growthFlags_ = save.growthFlags;
  } else {
    const uint8_t migrated = careMistakes_ > kCareMistakeCap
                                 ? kCareMistakeCap
                                 : static_cast<uint8_t>(careMistakes_);
    physicalCareMistakes_ = static_cast<uint8_t>(migrated / 2);
    mentalCareMistakes_ = static_cast<uint8_t>(migrated - physicalCareMistakes_);
    growthTier_ = generation_ <= 1 ? GrowthTier::kFirstGeneration
                                   : tierFromStageMistakes(
                                         physicalCareMistakes_,
                                         mentalCareMistakes_);
    adultTier_ = AdultTier::kNormal;
    parentAdultTierA_ = AdultTier::kNormal;
    parentAdultTierB_ = AdultTier::kNormal;
    bornFromUnhealthyParents_ = 0;
    growthFlags_ = 0;
  }
  points_ = save.points;
  donations_ = save.donations;
  souvenirMask_ = save.souvenirMask;
  memcpy(catalogOwned_, save.catalogOwned, sizeof(catalogOwned_));
  memcpy(souvenirOwned_, save.souvenirOwned, sizeof(souvenirOwned_));
  for (uint8_t i = 0; i < kSouvenirCount && i < kCatalogSouvenirCount; i++) {
    if (souvenirMask_ & (1U << i)) catalogBitSet(souvenirOwned_, i);
  }
  month_ = save.month ? clampMonth(save.month) : 1;
  day_ = save.day ? clampDay(save.day, month_) : 1;
  birthdayMonth_ = save.birthdayMonth ? save.birthdayMonth : 6;
  birthdayMonth_ = clampMonth(birthdayMonth_);
  birthdayDay_ = save.birthdayDay ? clampDay(save.birthdayDay, birthdayMonth_)
                                   : 16;
  hunger_ = save.hunger;
  happiness_ = save.happiness;
  energy_ = save.energy;
  hygiene_ = save.hygiene;
  discipline_ = save.discipline;
  friendship_ = save.friendship;
  messCount_ = save.messCount;
  sickness_ = save.sickness;
  toothache_ = save.toothache ? 1 : 0;
  stageSicknesses_ = save.version >= 10 ? save.stageSicknesses : 0;
  attention_ = save.attention;
  attentionMinutes_ = save.attentionMinutes;
  attentionReason_ = static_cast<AttentionReason>(save.attentionReason);
  lightsOff_ = save.lightsOff ? 1 : 0;
  soundOn_ = save.soundOn ? 1 : 0;
  stage_ = validStage(save.stage) ? static_cast<Stage>(save.stage) : Stage::kEgg;
  lifeState_ = save.lifeState ? LifeState::kPassed : LifeState::kAlive;
  gender_ = save.gender ? Gender::kGirl : Gender::kBoy;
  route_ = validRoute(save.route) ? static_cast<GrowthRoute>(save.route)
                                  : GrowthRoute::kBalanced;
  character_ = validCharacter(save.character)
                   ? static_cast<CharacterKind>(save.character)
                   : CharacterKind::kShell;
  characterCatalogId_ =
      save.characterCatalogId < kCharacterCatalogCount
          ? save.characterCatalogId
          : chooseCharacterCatalogId(stage_, route_);
  memcpy(foodCounts_, save.foodCounts, sizeof(foodCounts_));
  memcpy(itemCounts_, save.itemCounts, sizeof(itemCounts_));
  if (save.version >= 7 && save.size >= kPetSaveCatalogStockEnd) {
    memcpy(catalogStock_, save.catalogStock, sizeof(catalogStock_));
  } else {
    rebuildCatalogStockFromOwned();
  }
  gameWins_ = save.gameWins;
  gamePlays_ = save.gamePlays;
  gameStreak_ = save.gameStreak;
  bestGameScore_ = save.bestGameScore;
  friendVisits_ = save.friendVisits;
  memoryFlags_ = save.memoryFlags;
  careScore_ = save.careScore;
  playScore_ = save.playScore;
  socialScore_ = save.socialScore;
  disciplineScore_ = save.disciplineScore;
  snackScore_ = save.snackScore;
  sweetSnackStreak_ = save.sweetSnackStreak > kSweetSnackToothacheStreak
                          ? kSweetSnackToothacheStreak
                          : save.sweetSnackStreak;
  if (save.version >= 8) {
    parentCatalogIdA_ = save.parentCatalogIdA < kCharacterCatalogCount
                            ? save.parentCatalogIdA
                            : kCatalogIdUnknown;
    parentCatalogIdB_ = save.parentCatalogIdB < kCharacterCatalogCount
                            ? save.parentCatalogIdB
                            : kCatalogIdUnknown;
  } else {
    parentCatalogIdA_ = kCatalogIdUnknown;
    parentCatalogIdB_ = kCatalogIdUnknown;
  }
  if (save.version >= 9) {
    lastSweetSnackMinute_ = save.lastSweetSnackMinute;
  } else {
    lastSweetSnackMinute_ = sweetSnackStreak_ ? petMinuteStamp(ageMinutes_)
                                              : kSweetSnackNoLastMinute;
  }
  lastGameKind_ = save.lastGameKind;
  linkSequence_ = save.linkSequence;
  friendCount_ = save.friendCount <= kMaxFriends ? save.friendCount : kMaxFriends;
  memcpy(petName_, save.petName, sizeof(petName_));
  memcpy(nickname_, save.nickname, sizeof(nickname_));
  petName_[kNameChars] = '\0';
  nickname_[kNameChars] = '\0';
  memcpy(friends_, save.friends, sizeof(friends_));
  memcpy(family_, save.family, sizeof(family_));
  if (save.version >= 12 && save.size >= kPetSaveFamilyAncestryEnd) {
    memcpy(familyAncestry_, save.familyAncestry, sizeof(familyAncestry_));
  } else {
    memset(familyAncestry_, 0, sizeof(familyAncestry_));
  }

  miniGame_ = MiniGameKind::kNone;
  gamePrompt_ = GamePrompt::kNone;
  gameRound_ = 0;
  gameScore_ = 0;
  gameExpectedYes_ = 0;
  gameCursor_ = 1;
  gameTarget_ = 1;
  gameHazard_ = 0;
  gameRoundStartMs_ = nowMs;
  lastMs_ = nowMs;
  minuteRemainderMs_ = 0;
  clampStats();
  updateGrowth();
  updateMood();
  notice_ = Notice::kStatus;
  return true;
}

void EchoPetModel::exportSave(PetSave& save) const {
  memset(&save, 0, sizeof(save));
  save.magic = kMagic;
  save.version = kVersion;
  save.size = sizeof(PetSave);
  save.ageMinutes = ageMinutes_;
  save.clockMinutes = clockMinutes_;
  save.rngState = rngState_;
  save.petId = petId_;
  save.lastFriendId = lastFriendId_;
  save.generation = generation_;
  save.parentCareMinutes = parentCareMinutes_;
  save.lastAnniversaryDay = lastAnniversaryDay_;
  save.weight = weight_;
  save.careMistakes = careMistakes_;
  save.physicalCareMistakes = physicalCareMistakes_;
  save.mentalCareMistakes = mentalCareMistakes_;
  save.growthTier = static_cast<uint8_t>(growthTier_);
  save.adultTier = static_cast<uint8_t>(adultTier_);
  save.parentAdultTierA = static_cast<uint8_t>(parentAdultTierA_);
  save.parentAdultTierB = static_cast<uint8_t>(parentAdultTierB_);
  save.bornFromUnhealthyParents = bornFromUnhealthyParents_ ? 1 : 0;
  save.growthFlags = growthFlags_;
  save.points = points_;
  save.donations = donations_;
  save.souvenirMask = souvenirMask_;
  memcpy(save.catalogOwned, catalogOwned_, sizeof(catalogOwned_));
  memcpy(save.catalogStock, catalogStock_, sizeof(catalogStock_));
  memcpy(save.souvenirOwned, souvenirOwned_, sizeof(souvenirOwned_));
  save.month = month_;
  save.day = day_;
  save.birthdayMonth = birthdayMonth_;
  save.birthdayDay = birthdayDay_;
  save.hunger = hunger_;
  save.happiness = happiness_;
  save.energy = energy_;
  save.hygiene = hygiene_;
  save.discipline = discipline_;
  save.friendship = friendship_;
  save.messCount = messCount_;
  save.sickness = sickness_;
  save.toothache = toothache_ ? 1 : 0;
  save.stageSicknesses = stageSicknesses_;
  save.attentionMinutes = attentionMinutes_;
  save.attention = attention_;
  save.attentionReason = static_cast<uint8_t>(attentionReason_);
  save.lightsOff = lightsOff_;
  save.soundOn = soundOn_;
  save.stage = static_cast<uint8_t>(stage_);
  save.lifeState = static_cast<uint8_t>(lifeState_);
  save.gender = static_cast<uint8_t>(gender_);
  save.route = static_cast<uint8_t>(route_);
  save.character = static_cast<uint8_t>(character_);
  save.characterCatalogId = characterCatalogId_;
  memcpy(save.foodCounts, foodCounts_, sizeof(foodCounts_));
  memcpy(save.itemCounts, itemCounts_, sizeof(itemCounts_));
  save.gameWins = gameWins_;
  save.gamePlays = gamePlays_;
  save.gameStreak = gameStreak_;
  save.bestGameScore = bestGameScore_;
  save.friendVisits = friendVisits_;
  save.memoryFlags = memoryFlags_;
  save.careScore = careScore_;
  save.playScore = playScore_;
  save.socialScore = socialScore_;
  save.disciplineScore = disciplineScore_;
  save.snackScore = snackScore_;
  save.sweetSnackStreak = sweetSnackStreak_;
  save.parentCatalogIdA = parentCatalogIdA_;
  save.parentCatalogIdB = parentCatalogIdB_;
  save.lastSweetSnackMinute = lastSweetSnackMinute_;
  save.lastGameKind = lastGameKind_;
  save.linkSequence = linkSequence_;
  save.friendCount = friendCount_;
  memcpy(save.petName, petName_, sizeof(petName_));
  memcpy(save.nickname, nickname_, sizeof(nickname_));
  memcpy(save.friends, friends_, sizeof(friends_));
  memcpy(save.family, family_, sizeof(family_));
  memcpy(save.familyAncestry, familyAncestry_, sizeof(familyAncestry_));
  save.checksum = checksumSave(save);
}

bool EchoPetModel::tick(uint32_t nowMs) {
  uint32_t elapsed = nowMs - lastMs_;
  lastMs_ = nowMs;
  minuteRemainderMs_ += elapsed;

  bool changed = false;
  if (tickMiniGame(nowMs)) {
    changed = true;
  }
  while (minuteRemainderMs_ >= ECHOPET_MINUTE_MS) {
    minuteRemainderMs_ -= ECHOPET_MINUTE_MS;
    tickOnePetMinute();
    changed = true;
  }
  return changed;
}

bool EchoPetModel::apply(Action action) {
  if (lifeState_ == LifeState::kPassed && action != Action::kReset &&
      action != Action::kHealth) {
    notice_ = Notice::kPassed;
    return false;
  }
  if (isGameActive() && action != Action::kHealth) {
    notice_ = Notice::kGameStart;
    return false;
  }

  bool changed = true;
  const bool sleeping = (mood_ == Mood::kAsleep);
  if ((sickness_ || toothache_) && actionBlockedBySickness(action)) {
    notice_ = Notice::kSickRefuse;
    return false;
  }

  switch (action) {
    case Action::kHealth:
      notice_ = Notice::kStatus;
      changed = false;
      break;

    case Action::kMeal:
      if (sleeping) {
        notice_ = Notice::kSleeping;
        return false;
      }
      changed = useFood(FoodKind::kScone, false);
      break;

    case Action::kSnack:
      if (sleeping) {
        notice_ = Notice::kSleeping;
        return false;
      }
      changed = useFood(FoodKind::kCone, true);
      break;

    case Action::kGame:
      if (sleeping) {
        notice_ = Notice::kSleeping;
        return false;
      }
      if (weight_ <= baseWeightForStage(stage_) || unlockedGameCount() == 0) {
        happiness_ = clampAddHungryHappy(happiness_, -3);
        notice_ = Notice::kGameTired;
        break;
      }
      startMiniGame(MiniGameKind::kNone);
      break;

    case Action::kShop:
      changed = buyShopStock();
      break;

    case Action::kItem:
      changed = useBestItem();
      break;

    case Action::kToilet:
      if (messCount_ == 0) {
        notice_ = Notice::kNoMess;
        changed = false;
        break;
      }
      hygiene_ = clampAdd(hygiene_, 38);
      messCount_ = 0;
      friendship_ = clampAdd(friendship_, 2);
      careScore_ = clampAdd(careScore_, 4);
      notice_ = Notice::kClean;
      break;

    case Action::kMedicine:
      if (sickness_) {
        sickness_--;
        happiness_ = clampAddHungryHappy(happiness_, 4);
        friendship_ = clampAdd(friendship_, 4);
        careScore_ = clampAdd(careScore_, 5);
        notice_ = sickness_ ? Notice::kNeedMoreMedicine : Notice::kMedicine;
      } else if (toothache_) {
        toothache_ = 0;
        sweetSnackStreak_ = 0;
        lastSweetSnackMinute_ = kSweetSnackNoLastMinute;
        happiness_ = clampAddHungryHappy(happiness_, 3);
        friendship_ = clampAdd(friendship_, 2);
        careScore_ = clampAdd(careScore_, 3);
        notice_ = Notice::kMedicine;
      } else {
        happiness_ = clampAddHungryHappy(happiness_, -4);
        notice_ = Notice::kNoMedicine;
      }
      break;

    case Action::kLights:
      if (!isBedtime() && !lightsOff_) {
        notice_ = Notice::kLightsOn;
        changed = false;
        break;
      }
      lightsOff_ = lightsOff_ ? 0 : 1;
      if (lightsOff_) memoryFlags_ &= ~kMemoryFlagCostumeMask;
      notice_ = lightsOff_ ? Notice::kLightsOff : Notice::kLightsOn;
      break;

    case Action::kDiscipline:
      if (sleeping) {
        notice_ = Notice::kSleeping;
        return false;
      }
      if (attentionReason_ == AttentionReason::kNaughty) {
        discipline_ = clampAdd(discipline_, 14);
        disciplineScore_ = clampAdd(disciplineScore_, 8);
        friendship_ = clampAdd(friendship_, 3);
        attention_ = 0;
        attentionReason_ = AttentionReason::kNone;
        notice_ = Notice::kTrain;
      } else {
        happiness_ = clampAddHungryHappy(happiness_, -5);
        notice_ = Notice::kWrongDiscipline;
      }
      break;

    case Action::kPraise:
      if (sleeping) {
        notice_ = Notice::kSleeping;
        return false;
      }
      if (attentionReason_ == AttentionReason::kPraise || happiness_ < 25) {
        happiness_ = clampAddHungryHappy(happiness_, 12);
        friendship_ = clampAdd(friendship_, 6);
        disciplineScore_ = clampAdd(disciplineScore_, 4);
        attention_ = 0;
        attentionReason_ = AttentionReason::kNone;
        notice_ = Notice::kPraise;
      } else {
        happiness_ = clampAddHungryHappy(happiness_, -5);
        notice_ = Notice::kWrongDiscipline;
      }
      break;

    case Action::kVisit:
      lastLinkKind_ = LinkKind::kVisit;
      notice_ = Notice::kFriendReady;
      changed = false;
      break;

    case Action::kFriendList:
      notice_ = Notice::kStatus;
      changed = false;
      break;

    case Action::kPresent:
      lastLinkKind_ = LinkKind::kPresent;
      notice_ = Notice::kFriendReady;
      changed = false;
      break;

    case Action::kLinkGame:
      lastLinkKind_ = LinkKind::kGame;
      notice_ = Notice::kFriendReady;
      changed = false;
      break;

    case Action::kFamily:
      if (stage_ == Stage::kParentCare) {
        notice_ = Notice::kBaby;
      } else if (isAdultLike() && bestRelation() >= RelationLevel::kBestFriend) {
        startBaby();
      } else if (isAdultLike() &&
                 ageMinutes_ >=
                     kFamilyTimingRule.manualMatchmakerAgeMinutes) {
        notice_ = Notice::kMatchmaker;
        startBaby();
      } else {
        notice_ = Notice::kFamily;
        changed = false;
      }
      break;

    case Action::kDonate:
      if (points_ >= 100) {
        const bool hadCleaner = donations_ >= kSuperCleanerDonation;
        points_ -= 100;
        donations_ = clampAdd16(donations_, 100, 65000);
        happiness_ = clampAddHungryHappy(happiness_, 3);
        friendship_ = clampAdd(friendship_, 2);
        if (!hadCleaner && donations_ >= kSuperCleanerDonation) {
          awardSouvenir(39);
          if (messCount_) {
            messCount_ = 0;
            hygiene_ = clampAdd(hygiene_, 4);
            notice_ = Notice::kClean;
          } else {
            notice_ = Notice::kDonate;
          }
        } else {
          notice_ = Notice::kDonate;
        }
      } else {
        notice_ = Notice::kNoPoints;
        changed = false;
      }
      break;

    case Action::kPassword:
      notice_ = Notice::kPasswordDone;
      changed = false;
      break;

    case Action::kSound:
      soundOn_ = soundOn_ ? 0 : 1;
      notice_ = soundOn_ ? Notice::kSoundOn : Notice::kSoundOff;
      break;

    case Action::kReset:
      if (lifeState_ == LifeState::kPassed) {
        reset(rngState_ ^ ageMinutes_ ^ clockMinutes_, lastMs_);
      } else {
        notice_ = Notice::kResetReady;
        changed = false;
      }
      break;

    case Action::kCount:
      changed = false;
      break;
  }

  clearAttentionFor(action);
  clampStats();
  updateGrowth();
  updateMood();
  return changed;
}

bool EchoPetModel::feed(FoodKind food) {
  if (lifeState_ == LifeState::kPassed) {
    notice_ = Notice::kPassed;
    return false;
  }
  if (mood_ == Mood::kAsleep) {
    notice_ = Notice::kSleeping;
    return false;
  }
  if (sickness_ || toothache_) {
    notice_ = Notice::kSickRefuse;
    return false;
  }
  const bool changed = useFood(food, false);
  if (changed) clearAttentionFor(Action::kMeal);
  clampStats();
  updateMood();
  return changed;
}

bool EchoPetModel::snack(FoodKind food) {
  if (lifeState_ == LifeState::kPassed) {
    notice_ = Notice::kPassed;
    return false;
  }
  if (mood_ == Mood::kAsleep) {
    notice_ = Notice::kSleeping;
    return false;
  }
  if (sickness_ || toothache_) {
    notice_ = Notice::kSickRefuse;
    return false;
  }
  const bool changed = useFood(food, true);
  if (changed) clearAttentionFor(Action::kSnack);
  clampStats();
  updateMood();
  return changed;
}

bool EchoPetModel::useItem(ItemKind item) {
  if (lifeState_ == LifeState::kPassed) {
    notice_ = Notice::kPassed;
    return false;
  }
  if (sickness_ || toothache_) {
    notice_ = Notice::kSickRefuse;
    return false;
  }
  const bool changed = useSpecificItem(item);
  clampStats();
  updateMood();
  return changed;
}

bool EchoPetModel::useCatalogItem(uint8_t index) {
  if (lifeState_ == LifeState::kPassed) {
    notice_ = Notice::kPassed;
    return false;
  }
  if (index >= kCatalogItemCount || !catalogBitTest(catalogOwned_, index)) {
    notice_ = Notice::kNoItem;
    return false;
  }
  if (sickness_ || toothache_) {
    notice_ = Notice::kSickRefuse;
    return false;
  }

  const CatalogEntry entry = catalogEntry(index);
  bool changed = false;
  switch (entry.kind) {
    case CatalogKind::kFood: {
      if (catalogStock_[index] == 0) {
        notice_ = Notice::kNoItem;
        changed = false;
        break;
      }
      const uint8_t food = entry.behavior % kFoodKindCount;
      changed = useFood(static_cast<FoodKind>(food),
                        (entry.flags & kCatalogFlagSnack) != 0, false);
      if (changed) {
        if (foodCounts_[food] > 0) {
          foodCounts_[food]--;
        }
        catalogStock_[index]--;
        clearCatalogStockIfEmpty(index, entry);
        applyCatalogFoodTaste(index);
      }
      break;
    }
    case CatalogKind::kItem: {
      const bool childPlusStage = stage_ == Stage::kChild ||
                                  stage_ == Stage::kTeen ||
                                  stage_ == Stage::kAdult ||
                                  stage_ == Stage::kParentCare ||
                                  stage_ == Stage::kElder;
      const bool teenPlusStage = stage_ == Stage::kTeen ||
                                 stage_ == Stage::kAdult ||
                                 stage_ == Stage::kParentCare ||
                                 stage_ == Stage::kElder;
      const bool adultStage = stage_ == Stage::kAdult ||
                              stage_ == Stage::kParentCare ||
                              stage_ == Stage::kElder;
      if ((entry.flags & kCatalogFlagChildPlus) && !childPlusStage) {
        notice_ = Notice::kNoItem;
        changed = false;
        break;
      }
      if ((entry.flags & kCatalogFlagTeenPlus) && !teenPlusStage) {
        notice_ = Notice::kNoItem;
        changed = false;
        break;
      }
      if ((entry.flags & kCatalogFlagAdultOnly) && !adultStage) {
        notice_ = Notice::kNoItem;
        changed = false;
        break;
      }
      if (index == kCatalogActionFigureIndex ||
          index == kCatalogDoll1Index ||
          index == kCatalogDoll2Index) {
        const bool liked =
            (index == kCatalogActionFigureIndex && gender_ == Gender::kBoy) ||
            ((index == kCatalogDoll1Index || index == kCatalogDoll2Index) &&
             gender_ == Gender::kGirl);
        happiness_ = clampAddHungryHappy(happiness_, liked ? 10 : -3);
        friendship_ = clampAdd(friendship_, liked ? 5 : 1);
        playScore_ = clampAdd(playScore_, liked ? 3 : 1);
        notice_ = Notice::kItemUsed;
        changed = true;
        break;
      }
      if (index == kCatalogPlantIndex || index == kCatalogShovelIndex) {
        if (!consumeCatalogItem(index, entry)) {
          changed = false;
          break;
        }
        const CatalogRandomRule& random = catalogRandomRuleFor(index);
        const uint8_t roll = nextRand(random.rollCount);
        if (roll == 0) {
          points_ =
              clampAdd16(points_, random.pointReward, kMaxGotchiPoints);
          notice_ = Notice::kPoints;
        } else if (roll == 1) {
          happiness_ = clampAddHungryHappy(happiness_, 12);
          notice_ = Notice::kItemUsed;
        } else {
          happiness_ = clampAddHungryHappy(happiness_, -8);
          notice_ = Notice::kItemUsed;
        }
        changed = true;
        break;
      }
      if (index == kCatalogChestIndex) {
        if (!consumeCatalogItem(index, entry)) {
          changed = false;
          break;
        }
        const CatalogRandomRule& random = catalogRandomRuleFor(index);
        switch (nextRand(random.rollCount)) {
          case 0:
          case 1:
          case 2:
          case 3:
            awardCatalogItem(chestLampExclusiveCatalogItem(nextRand(4)));
            notice_ = Notice::kGift;
            break;
          case 4:
            points_ =
                clampAdd16(points_, random.pointReward, kMaxGotchiPoints);
            notice_ = Notice::kPoints;
            break;
          default:
            hunger_ = 0;
            happiness_ = 0;
            notice_ = Notice::kItemUsed;
            break;
        }
        changed = true;
        break;
      }
      if (index == kCatalogFishingPoleIndex) {
        if (!consumeCatalogItem(index, entry)) {
          changed = false;
          break;
        }
        const CatalogRandomRule& random = catalogRandomRuleFor(index);
        const uint8_t roll = nextRand(random.rollCount);
        if (roll == 0) {
          awardCatalogItem(kCatalogFishingPoleIndex);
          happiness_ = clampAddHungryHappy(happiness_, -4);
          notice_ = Notice::kItemUsed;
        } else if (roll == 1) {
          happiness_ = clampAddHungryHappy(happiness_, 12);
          notice_ = Notice::kItemUsed;
        } else if (roll == 2) {
          points_ =
              clampAdd16(points_, random.pointReward, kMaxGotchiPoints);
          notice_ = Notice::kPoints;
        } else {
          happiness_ = clampAddHungryHappy(happiness_, 16);
          notice_ = Notice::kItemUsed;
        }
        changed = true;
        break;
      }
      if (index == kCatalogLampIndex) {
        const CatalogRandomRule& random = catalogRandomRuleFor(index);
        switch (nextRand(random.rollCount)) {
          case 0:
          case 1:
            awardCatalogItem(chestLampExclusiveCatalogItem(nextRand(4)));
            notice_ = Notice::kGift;
            break;
          case 2:
            points_ =
                clampAdd16(points_, random.pointReward, kMaxGotchiPoints);
            notice_ = Notice::kPoints;
            break;
          case 3:
            happiness_ = clampAddHungryHappy(happiness_, 15);
            notice_ = Notice::kItemUsed;
            break;
          default:
            happiness_ = clampAddHungryHappy(happiness_, -10);
            notice_ = Notice::kItemUsed;
            break;
        }
        if (random.breakChancePercent &&
            nextRand(100) < random.breakChancePercent) {
          catalogBitClear(catalogOwned_, kCatalogLampIndex);
        }
        changed = true;
        break;
      }
      if (index == kCatalogClockIndex) {
        happiness_ = clampAddHungryHappy(happiness_, 4);
        energy_ = clampAdd(energy_, 4);
        disciplineScore_ = clampAdd(disciplineScore_, 2);
        notice_ = Notice::kItemUsed;
        changed = true;
        break;
      }
      if (index == kCatalogHoneyIndex) {
        if (!consumeCatalogItem(index, entry)) {
          changed = false;
          break;
        }
        memoryFlags_ |= kMemoryFlagHoneyPending;
        happiness_ = clampAddHungryHappy(happiness_, 6);
        friendship_ = clampAdd(friendship_, 8);
        socialScore_ = clampAdd(socialScore_, 4);
        notice_ = Notice::kItemUsed;
        changed = true;
        break;
      }
      if (index == kCatalogRcCar3Index) {
        happiness_ = clampAddHungryHappy(happiness_, 12);
        friendship_ = clampAdd(friendship_, 8);
        playScore_ = clampAdd(playScore_, 6);
        notice_ = Notice::kItemUsed;
        changed = true;
        break;
      }
      if (index == kCatalogNyatchiIndex || index == kCatalogHohotchiIndex) {
        if (!consumeCatalogItem(index, entry)) {
          changed = false;
          break;
        }
        memoryFlags_ &= ~kMemoryFlagCostumeMask;
        memoryFlags_ |= (index == kCatalogHohotchiIndex)
                            ? kMemoryFlagHohotchiCostume
                            : kMemoryFlagNyatchiCostume;
        happiness_ = clampAddHungryHappy(happiness_, 10);
        friendship_ = clampAdd(friendship_, 4);
        notice_ = Notice::kItemUsed;
        changed = true;
        break;
      }
      const uint8_t souvenir = ticketSouvenirForCatalogIndex(index);
      if (souvenir != 0xFF) {
        if (!consumeCatalogItem(index, entry)) {
          changed = false;
          break;
        }
        awardSouvenir(souvenir);
        happiness_ = clampAddHungryHappy(happiness_, 15);
        socialScore_ = clampAdd(socialScore_, 2);
        notice_ = Notice::kSouvenir;
        changed = true;
        break;
      }
      if (index == kCatalogMusicDiscIndex) {
        if (!catalogBitTest(catalogOwned_, kCatalogBoomBoxIndex)) {
          notice_ = Notice::kNoItem;
          changed = false;
          break;
        }
        if (!consumeCatalogItem(index, entry)) {
          changed = false;
          break;
        }
        if (nextRand(100) < 20) {
          catalogBitClear(catalogOwned_, kCatalogBoomBoxIndex);
          happiness_ = clampAddHungryHappy(happiness_, -4);
        } else {
          happiness_ = clampAddHungryHappy(happiness_, 8);
          friendship_ = clampAdd(friendship_, 6);
          socialScore_ = clampAdd(socialScore_, 5);
        }
        notice_ = Notice::kItemUsed;
        changed = true;
        break;
      }
      if (index == kCatalogMakeupIndex) {
        if (!catalogBitTest(catalogOwned_, kCatalogMirrorIndex)) {
          notice_ = Notice::kNoItem;
          changed = false;
          break;
        }
        if (!consumeCatalogItem(index, entry)) {
          changed = false;
          break;
        }
        happiness_ = clampAddHungryHappy(happiness_, 10);
        friendship_ = clampAdd(friendship_, 3);
        socialScore_ = clampAdd(socialScore_, 3);
        notice_ = Notice::kItemUsed;
        changed = true;
        break;
      }
      if (index == kCatalogShaverIndex) {
        if (strcmp(characterCatalogEntry(characterCatalogId_).name,
                   "Oyajitchi") != 0) {
          notice_ = Notice::kNoItem;
          changed = false;
          break;
        }
        happiness_ = clampAddHungryHappy(happiness_, 8);
        friendship_ = clampAdd(friendship_, 4);
        notice_ = Notice::kItemUsed;
        changed = true;
        break;
      }
      if (index == kCatalogTamaDrinkIndex) {
        if (!consumeCatalogItem(index, entry)) {
          changed = false;
          break;
        }
        memoryFlags_ |= kMemoryFlagTamaDrinkGuard;
        happiness_ = clampAddHungryHappy(happiness_, 4);
        careScore_ = clampAdd(careScore_, 4);
        notice_ = Notice::kItemUsed;
        changed = true;
        break;
      }
      if (index == kCatalogHairGelIndex) {
        if (!consumeCatalogItem(index, entry)) {
          changed = false;
          break;
        }
        happiness_ = clampAddHungryHappy(happiness_, 6);
        friendship_ = clampAdd(friendship_, 2);
        notice_ = Notice::kItemUsed;
        changed = true;
        break;
      }
      const bool consume = (entry.flags & kCatalogFlagReusable) == 0;
      changed = useSpecificItem(
          static_cast<ItemKind>(entry.behavior % kItemKindCount), consume);
      break;
    }
    case CatalogKind::kSouvenir:
      notice_ = Notice::kSouvenir;
      changed = false;
      break;
  }
  clampStats();
  updateMood();
  return changed;
}

bool EchoPetModel::buyShopSlot(uint8_t slot) {
  return buyCatalogItem(catalogShopIndex(slot, month_, day_, birthdayMonth_,
                                         birthdayDay_, clockMinutes_));
}

bool EchoPetModel::enterPassword(const uint8_t digits[10]) {
  if (lifeState_ == LifeState::kPassed) {
    notice_ = Notice::kPassed;
    return false;
  }

  for (uint8_t i = 0; i < sizeof(kPasswordRewards) / sizeof(kPasswordRewards[0]);
       i++) {
    const PasswordReward& reward = kPasswordRewards[i];
    if (memcmp(digits, reward.digits, sizeof(reward.digits)) != 0) {
      continue;
    }

    if (!reward.repeatable && catalogBitTest(catalogOwned_, reward.catalogIndex)) {
      notice_ = Notice::kPasswordDone;
      return false;
    }

    awardCatalogItem(reward.catalogIndex);
    happiness_ = clampAddHungryHappy(happiness_, 3);
    notice_ = Notice::kPasswordOk;
    clampStats();
    return true;
  }

  uint8_t catalogIndex = 0;
  if (decodeCatalogPassword(digits, catalogIndex)) {
    if (catalogBitTest(catalogOwned_, catalogIndex)) {
      notice_ = Notice::kPasswordDone;
      return false;
    }
    awardCatalogItem(catalogIndex);
    happiness_ = clampAddHungryHappy(happiness_, 3);
    notice_ = Notice::kPasswordOk;
    clampStats();
    return true;
  }

  notice_ = Notice::kPasswordBad;
  return false;
}

bool EchoPetModel::enterSecretCode(const uint8_t symbols[8]) {
  if (lifeState_ == LifeState::kPassed) {
    notice_ = Notice::kPassed;
    return false;
  }

  for (uint8_t i = 0;
       i < sizeof(kSecretCodeRewards) / sizeof(kSecretCodeRewards[0]); i++) {
    const SecretCodeReward& reward = kSecretCodeRewards[i];
    if (memcmp(symbols, reward.symbols, sizeof(reward.symbols)) != 0) {
      continue;
    }

    const bool rewardOwned = catalogBitTest(catalogOwned_, reward.catalogIndex);
    const bool hohotchiOwned =
        catalogBitTest(catalogOwned_, kCatalogHohotchiIndex);
    const uint8_t secretCount = ownedSecretCodeCount(catalogOwned_);

    if (!rewardOwned && !hohotchiOwned &&
        secretCount + 1 >= kCatalogSecretCount) {
      awardCatalogItem(kCatalogHohotchiIndex);
      happiness_ = clampAddHungryHappy(happiness_, 3);
      notice_ = Notice::kPasswordOk;
      clampStats();
      return true;
    }

    if (rewardOwned || hohotchiOwned) {
      return buyCatalogItem(reward.catalogIndex);
    }

    awardCatalogItem(reward.catalogIndex);
    happiness_ = clampAddHungryHappy(happiness_, 3);
    notice_ = Notice::kPasswordOk;
    clampStats();
    return true;
  }

  notice_ = Notice::kPasswordBad;
  return false;
}

bool EchoPetModel::startGame(MiniGameKind game) {
  if (lifeState_ == LifeState::kPassed) {
    notice_ = Notice::kPassed;
    return false;
  }
  if (mood_ == Mood::kAsleep) {
    notice_ = Notice::kSleeping;
    return false;
  }
  if (sickness_ || toothache_) {
    notice_ = Notice::kSickRefuse;
    return false;
  }
  if (weight_ <= baseWeightForStage(stage_)) {
    notice_ = Notice::kGameTired;
    return false;
  }
  const uint8_t value = static_cast<uint8_t>(game);
  if (game == MiniGameKind::kNone || value > unlockedGameCount()) {
    notice_ = Notice::kGameTired;
    return false;
  }
  startMiniGame(game);
  return true;
}

bool EchoPetModel::isGameActive() const {
  return miniGame_ != MiniGameKind::kNone;
}

bool EchoPetModel::gameInput(GameButton button, uint32_t nowMs) {
  if (!isGameActive()) return false;

  bool advance = true;
  bool correct = false;
  uint8_t gain = 1;

  switch (miniGame_) {
    case MiniGameKind::kGet:
      if (button == GameButton::kA) {
        gameCursor_ = laneLeft(gameCursor_);
        advance = false;
      } else if (button == GameButton::kC) {
        gameCursor_ = laneRight(gameCursor_);
        advance = false;
      } else {
        advance = false;
      }
      break;

    case MiniGameKind::kBump: {
      const uint8_t progress =
          scaledProgress(gameRound_, gameRoundLimitFor(miniGame_), 100);
      const uint8_t power = static_cast<uint8_t>(
          26 + (weight_ > 60 ? 60 : weight_) + progress / 6 + nextRand(20));
      const uint8_t foe =
          static_cast<uint8_t>(44 + progress / 3 + nextRand(32));
      gameCursor_ = power > 99 ? 99 : power;
      gameTarget_ = foe > 99 ? 99 : foe;
      correct = power >= foe;
      gain = 1;
      break;
    }

    case MiniGameKind::kFlag:
      correct = gameHazard_ == 0 && buttonValue(button) == gameExpectedYes_;
      break;

    case MiniGameKind::kHeading:
      if (button == GameButton::kA) {
        gameCursor_ = laneLeft(gameCursor_);
        advance = false;
      } else if (button == GameButton::kC) {
        gameCursor_ = laneRight(gameCursor_);
        advance = false;
      } else {
        correct = gameCursor_ == gameTarget_;
      }
      break;

    case MiniGameKind::kMemory: {
      correct = buttonValue(button) == gameExpectedYes_;
      if (correct && gameCursor_ + 1 < gameTarget_) {
        gameCursor_++;
        gameExpectedYes_ = memorySequenceAt(gameHazard_, gameCursor_);
        notice_ = Notice::kGameGood;
        return true;
      }
      gain = 1;
      break;
    }

    case MiniGameKind::kSprint:
      gameTarget_ = sprintTapTargetFor(weight_);
      if (gameCursor_ < gameTarget_) gameCursor_++;
      if (gameCursor_ < gameTarget_) {
        advance = false;
      } else {
        correct = nextRand(100) < sprintWinChanceFor(weight_, gameRound_);
        gain = weight_ <= 20 ? 2 : 1;
        gameCursor_ = gameTarget_;
      }
      break;

    case MiniGameKind::kHoops:
      if (button == GameButton::kA) {
        gameCursor_ = laneLeft(gameCursor_);
        advance = false;
      } else if (button == GameButton::kC) {
        gameCursor_ = laneRight(gameCursor_);
        advance = false;
      } else {
        correct = gameCursor_ == gameTarget_;
      }
      break;

    case MiniGameKind::kNone:
      return false;
  }

  if (!advance) {
    notice_ = Notice::kGameStart;
    return true;
  }

  completeMiniGameRound(correct, gain, nowMs);
  return true;
}

void EchoPetModel::completeMiniGameRound(bool correct, uint8_t gain,
                                         uint32_t nowMs) {
  if (correct) {
    gameScore_ = clampAdd(gameScore_, gain);
    const uint8_t scoreLimit = gameScoreLimitFor(miniGame_);
    if (gameScore_ > scoreLimit) gameScore_ = scoreLimit;
    notice_ = Notice::kGameGood;
  } else {
    notice_ = Notice::kGameMiss;
  }
  gameRound_++;
  if (gameRound_ >= gameRoundLimitFor(miniGame_)) {
    finishMiniGame();
  } else {
    setupGameRound();
    gameRoundStartMs_ = nowMs;
  }
  clampStats();
  updateGrowth();
  updateMood();
}

bool EchoPetModel::tickMiniGame(uint32_t nowMs) {
  if (!isGameActive()) {
    return false;
  }

  const uint32_t elapsed = nowMs - gameRoundStartMs_;
  switch (miniGame_) {
    case MiniGameKind::kGet: {
      if (elapsed < gameAutoWindowFor(miniGame_)) return false;
      const bool correct =
          gameHazard_ ? (gameCursor_ != gameTarget_) : (gameCursor_ == gameTarget_);
      const uint8_t gain =
          gameHazard_ ? 1 : static_cast<uint8_t>(1 + (gameExpectedYes_ > 1));
      completeMiniGameRound(correct, gain, nowMs);
      return true;
    }
    case MiniGameKind::kHoops:
      if (elapsed < gameAutoWindowFor(miniGame_)) return false;
      completeMiniGameRound(false, 0, nowMs);
      return true;
    case MiniGameKind::kFlag:
      if (elapsed < gameAutoWindowFor(miniGame_)) return false;
      completeMiniGameRound(gameHazard_ == kFlagFakeSignal, 1, nowMs);
      return true;
    case MiniGameKind::kBump:
    case MiniGameKind::kHeading:
    case MiniGameKind::kMemory:
    case MiniGameKind::kSprint:
    case MiniGameKind::kNone:
      return false;
  }
  return false;
}

void EchoPetModel::fillFriendPacket(LinkKind kind, FriendPacket& packet) {
  memset(&packet, 0, sizeof(packet));
  packet.magic = kFriendMagic;
  packet.version = kFriendVersion;
  packet.kind = static_cast<uint8_t>(kind);
  packet.petId = petId_;
  packet.generation = generation_;
  packet.ageDays = static_cast<uint16_t>(ageMinutes_ / 1440UL);
  packet.stage = static_cast<uint8_t>(stage_);
  packet.character = static_cast<uint8_t>(character_);
  packet.friendship = friendship_;
  packet.relation = static_cast<uint8_t>(bestRelation());
  packet.sequence = ++linkSequence_;
  packet.growthTier = static_cast<uint8_t>(growthTier_);
  packet.adultTier = static_cast<uint8_t>(adultTier_);
  packet.gender = static_cast<uint8_t>(gender_);
  packet.catalogId = characterCatalogId_;
}

bool EchoPetModel::prepareFriendPacket(LinkKind kind, FriendPacket& packet,
                                       bool updateNotice) {
  if (kind == LinkKind::kLove && !isAdultLike()) {
    if (updateNotice) notice_ = Notice::kMatchmaker;
    return false;
  }
  fillFriendPacket(kind, packet);
  if (kind == LinkKind::kPresent) {
    GiftKind gift = GiftKind::kNone;
    uint8_t id = 0;
    uint8_t amount = 0;
    chooseGift(gift, id, amount);
    packet.giftKind = static_cast<uint8_t>(gift);
    packet.giftId = id;
    packet.giftAmount = amount;
  } else if (kind == LinkKind::kGame) {
    packet.gameKind = linkGameValue(LinkGameKind::kGotchiPoint);
    packet.gameScore = nextRand(6);
  }

  packet.checksum = checksumFriendPacket(packet);
  lastLinkKind_ = kind;
  if (updateNotice) notice_ = Notice::kFriendReady;
  return true;
}

bool EchoPetModel::prepareLinkReplyPacket(LinkKind kind, FriendPacket& packet) {
  if (kind == LinkKind::kLove &&
      !(isAdultLike() || stage_ == Stage::kParentCare)) {
    return false;
  }
  fillFriendPacket(kind, packet);
  packet.checksum = checksumFriendPacket(packet);
  lastLinkKind_ = kind;
  return true;
}

bool EchoPetModel::prepareGiftPacket(GiftKind kind, uint8_t id, uint8_t amount,
                                     FriendPacket& packet,
                                     bool updateNotice) {
  if (kind == GiftKind::kFood) {
    if (id >= kFoodKindCount || foodCounts_[id] < amount || amount == 0) {
      if (updateNotice) notice_ = Notice::kNoItem;
      return false;
    }
    foodCounts_[id] -= amount;
  } else if (kind == GiftKind::kItem) {
    if (id >= kItemKindCount || itemCounts_[id] < amount || amount == 0) {
      if (updateNotice) notice_ = Notice::kNoItem;
      return false;
    }
    itemCounts_[id] -= amount;
  } else if (kind == GiftKind::kPoints) {
    const uint16_t points = static_cast<uint16_t>(amount) * 10U;
    if (points_ < points || amount == 0) {
      if (updateNotice) notice_ = Notice::kNoPoints;
      return false;
    }
    points_ -= points;
  } else {
    return prepareFriendPacket(LinkKind::kPresent, packet, updateNotice);
  }

  fillFriendPacket(LinkKind::kPresent, packet);
  packet.giftKind = static_cast<uint8_t>(kind);
  packet.giftId = id;
  packet.giftAmount = amount;
  packet.checksum = checksumFriendPacket(packet);
  lastLinkKind_ = LinkKind::kPresent;
  if (updateNotice) notice_ = Notice::kFriendReady;
  return true;
}

bool EchoPetModel::prepareCatalogGiftPacket(uint8_t index,
                                            FriendPacket& packet,
                                            bool updateNotice) {
  if (!debitCatalogGift(index, updateNotice)) {
    return false;
  }

  fillFriendPacket(LinkKind::kPresent, packet);
  packet.giftKind = static_cast<uint8_t>(GiftKind::kCatalogItem);
  packet.giftId = index;
  packet.giftAmount = 1;
  packet.checksum = checksumFriendPacket(packet);
  lastLinkKind_ = LinkKind::kPresent;
  if (updateNotice) notice_ = Notice::kFriendReady;
  return true;
}

bool EchoPetModel::prepareLinkGamePacket(LinkGameKind game,
                                         FriendPacket& packet,
                                         bool updateNotice) {
  const uint8_t value = linkGameValue(game);
  if (game == LinkGameKind::kNone || !validLinkGameKind(value)) {
    if (updateNotice) notice_ = Notice::kNoItem;
    return false;
  }
  if (!ownsLinkGameItem(game)) {
    if (updateNotice) notice_ = Notice::kNoItem;
    return false;
  }

  fillFriendPacket(LinkKind::kGame, packet);
  packet.gameKind = value;
  packet.gameScore = nextRand(6);
  packet.checksum = checksumFriendPacket(packet);
  lastLinkKind_ = LinkKind::kGame;
  if (updateNotice) notice_ = Notice::kFriendReady;
  return true;
}

bool EchoPetModel::receiveFriendPacket(const FriendPacket& packet) {
  if (!validateFriendPacket(packet) || packet.petId == petId_ ||
      !validLinkKind(packet.kind) || !validStage(packet.stage)) {
    return false;
  }

  const LinkKind kind = static_cast<LinkKind>(packet.kind);
  if (kind == LinkKind::kGame) {
    const LinkGameKind game = static_cast<LinkGameKind>(packet.gameKind);
    if (!validLinkGameKind(packet.gameKind) || !ownsLinkGameItem(game)) {
      notice_ = Notice::kNoItem;
      return false;
    }
  }

  bool isNew = false;
  FriendRecord* friendRecord = addOrUpdateFriend(packet, &isNew);
  if (!friendRecord) {
    notice_ = Notice::kFriendFull;
    return false;
  }

  lastFriendId_ = packet.petId;
  friendVisits_ = clampAdd(friendVisits_, 1);
  friendship_ = clampAdd(friendship_, isNew ? 12 : 7);
  happiness_ = clampAddHungryHappy(happiness_, isNew ? 10 : 6);
  socialScore_ = clampAdd(socialScore_, isNew ? 9 : 5);
  const bool honeyPending = (memoryFlags_ & kMemoryFlagHoneyPending) != 0;
  memoryFlags_ |= kMemoryFlagFriendSeen;

  const bool localAdult = isAdultLike();
  const bool remoteAdult = static_cast<Stage>(packet.stage) >= Stage::kAdult;
  switch (kind) {
    case LinkKind::kVisit:
      notice_ = Notice::kFriendVisit;
      break;

    case LinkKind::kPresent: {
      bool acceptedGift = false;
      if (packet.giftKind == static_cast<uint8_t>(GiftKind::kCatalogItem) &&
          packet.giftId < kCatalogItemCount && packet.giftAmount == 1) {
        bool inventoryFull = false;
        acceptedGift = receiveCatalogGift(packet.giftId, inventoryFull);
        if (inventoryFull) {
          notice_ = Notice::kInventoryFull;
          break;
        }
      } else if (packet.giftKind == static_cast<uint8_t>(GiftKind::kFood) &&
          packet.giftId < kFoodKindCount && packet.giftAmount > 0) {
        const uint16_t next =
            static_cast<uint16_t>(foodCounts_[packet.giftId]) +
            packet.giftAmount;
        if (next > kMaxFoodStock) {
          notice_ = Notice::kInventoryFull;
          break;
        }
        awardFood(static_cast<FoodKind>(packet.giftId), packet.giftAmount);
        acceptedGift = true;
      } else if (packet.giftKind == static_cast<uint8_t>(GiftKind::kItem) &&
                 packet.giftId < kItemKindCount && packet.giftAmount > 0) {
        const uint16_t next =
            static_cast<uint16_t>(itemCounts_[packet.giftId]) +
            packet.giftAmount;
        if (next > kMaxItemStock) {
          notice_ = Notice::kInventoryFull;
          break;
        }
        awardItem(static_cast<ItemKind>(packet.giftId), packet.giftAmount);
        acceptedGift = true;
      } else if (packet.giftKind == static_cast<uint8_t>(GiftKind::kPoints)) {
        points_ = clampAdd16(points_, packet.giftAmount * 10, kMaxGotchiPoints);
        acceptedGift = packet.giftAmount > 0;
      }
      if (acceptedGift) {
        friendRecord->gifts = clampAdd(friendRecord->gifts, 1);
      }
      notice_ = Notice::kFriendPresent;
      break;
    }

    case LinkKind::kGame: {
      const LinkGameKind game = static_cast<LinkGameKind>(packet.gameKind);
      const uint8_t localScore = nextRand(6);
      const uint8_t peerScore = packet.gameScore > 5 ? 5 : packet.gameScore;
      const bool win = localScore >= peerScore;
      points_ = clampAdd16(points_, linkGamePrize(game, win), kMaxGotchiPoints);
      happiness_ = clampAddHungryHappy(happiness_, win ? 12 : 5);
      playScore_ = clampAdd(playScore_, win ? 6 : 2);
      socialScore_ = clampAdd(socialScore_, win ? 5 : 2);
      notice_ = Notice::kFriendGame;
      break;
    }

    case LinkKind::kLove:
      if (localAdult && remoteAdult) {
        friendRecord->visits = 24;
        friendRecord->relation = static_cast<uint8_t>(RelationLevel::kPartner);
        happiness_ = clampAddHungryHappy(happiness_, 18);
        friendship_ = clampAdd(friendship_, 14);
        socialScore_ = clampAdd(socialScore_, 12);
        notice_ = Notice::kPartner;
      } else {
        happiness_ = clampAddHungryHappy(happiness_, -2);
        notice_ = Notice::kLoveRejected;
      }
      break;
  }

  if (honeyPending) {
    friendRecord->visits = 24;
    friendRecord->relation = static_cast<uint8_t>(RelationLevel::kPartner);
    friendship_ = 100;
    socialScore_ = 100;
    happiness_ = clampAddHungryHappy(happiness_, 12);
    memoryFlags_ &= ~kMemoryFlagHoneyPending;
    notice_ = Notice::kPartner;
  }

  if (kind == LinkKind::kLove && localAdult && remoteAdult &&
      friendRecord->relation >= static_cast<uint8_t>(RelationLevel::kPartner)) {
    const bool oyajitchiLineage =
        stage_ == Stage::kElder &&
        static_cast<Stage>(packet.stage) == Stage::kElder &&
        static_cast<Gender>(packet.gender) != gender_;
    startBaby(static_cast<AdultTier>(packet.adultTier), oyajitchiLineage,
              packet.catalogId);
  }

  clampStats();
  updateGrowth();
  updateMood();
  return true;
}

bool EchoPetModel::deleteFriend(uint8_t index) {
  if (index >= friendCount_) {
    notice_ = Notice::kFriendFull;
    return false;
  }

  const uint32_t removedId = friends_[index].petId;
  for (uint8_t i = index; i + 1 < friendCount_; i++) {
    friends_[i] = friends_[i + 1];
  }
  friendCount_--;
  memset(&friends_[friendCount_], 0, sizeof(friends_[friendCount_]));
  if (lastFriendId_ == removedId) lastFriendId_ = 0;
  notice_ = Notice::kFriendDeleted;
  return true;
}

Snapshot EchoPetModel::snapshot() const {
  Snapshot s;
  memset(&s, 0, sizeof(s));
  s.ageMinutes = ageMinutes_;
  s.clockMinutes = clockMinutes_;
  s.petId = petId_;
  s.lastFriendId = lastFriendId_;
  s.generation = generation_;
  s.weight = weight_;
  s.careMistakes = careMistakes_;
  s.points = points_;
  s.donations = donations_;
  s.souvenirMask = souvenirMask_;
  memcpy(s.catalogOwned, catalogOwned_, sizeof(catalogOwned_));
  memcpy(s.catalogStock, catalogStock_, sizeof(catalogStock_));
  memcpy(s.souvenirOwned, souvenirOwned_, sizeof(souvenirOwned_));
  s.catalogOwnedCount =
      catalogBitCount(catalogOwned_, kCatalogItemCount);
  s.souvenirOwnedCount =
      catalogBitCount(souvenirOwned_, kCatalogSouvenirCount);
  s.careScore = careScore_;
  s.playScore = playScore_;
  s.socialScore = socialScore_;
  s.disciplineScore = disciplineScore_;
  s.snackScore = snackScore_;
  s.physicalCareMistakes = physicalCareMistakes_;
  s.mentalCareMistakes = mentalCareMistakes_;
  s.growthTier = static_cast<uint8_t>(growthTier_);
  s.adultTier = static_cast<uint8_t>(adultTier_);
  s.parentAdultTierA = static_cast<uint8_t>(parentAdultTierA_);
  s.parentAdultTierB = static_cast<uint8_t>(parentAdultTierB_);
  s.bornFromUnhealthyParents = bornFromUnhealthyParents_;
  s.growthFlags = growthFlags_;
  s.month = month_;
  s.day = day_;
  s.hour = static_cast<uint8_t>((clockMinutes_ / 60) % 24);
  s.minute = static_cast<uint8_t>(clockMinutes_ % 60);
  s.birthdayMonth = birthdayMonth_;
  s.birthdayDay = birthdayDay_;
  s.hunger = hunger_;
  s.happiness = happiness_;
  s.energy = energy_;
  s.hygiene = hygiene_;
  s.discipline = discipline_;
  s.friendship = friendship_;
  s.messCount = messCount_;
  s.sickness = sickness_;
  s.toothache = toothache_;
  s.attention = attention_;
  s.lightsOff = lightsOff_;
  s.soundOn = soundOn_;
  memcpy(s.foodCounts, foodCounts_, sizeof(foodCounts_));
  memcpy(s.itemCounts, itemCounts_, sizeof(itemCounts_));
  s.gameWins = gameWins_;
  s.gamePlays = gamePlays_;
  s.gameStreak = gameStreak_;
  s.bestGameScore = bestGameScore_;
  s.friendCount = friendCount_;
  s.friendVisits = friendVisits_;
  s.memoryFlags = memoryFlags_;
  s.gameRound = gameRound_;
  s.gameScore = gameScore_;
  s.gameRoundLimit = miniGame_ == MiniGameKind::kNone
                         ? 0
                         : gameRoundLimitFor(miniGame_);
  s.gameScoreLimit = miniGame_ == MiniGameKind::kNone
                         ? 0
                         : gameScoreLimitFor(miniGame_);
  s.gameExpectedYes = gameExpectedYes_;
  s.gameCursor = gameCursor_;
  s.gameTarget = gameTarget_;
  s.gameHazard = gameHazard_;
  s.unlockedGames = unlockedGameCount();
  memcpy(s.petName, petName_, sizeof(s.petName));
  memcpy(s.nickname, nickname_, sizeof(s.nickname));
  s.gender = gender_;
  s.lifeState = lifeState_;
  s.route = route_;
  s.character = character_;
  s.characterCatalogId = characterCatalogId_;
  if (stage_ == Stage::kAdult || stage_ == Stage::kParentCare ||
      stage_ == Stage::kElder) {
    if (memoryFlags_ & kMemoryFlagHohotchiCostume) {
      s.character = CharacterKind::kSage;
    } else if (memoryFlags_ & kMemoryFlagNyatchiCostume) {
      s.character = CharacterKind::kDream;
    }
  }
  s.stage = stage_;
  s.mood = mood_;
  s.miniGame = miniGame_;
  s.gamePrompt = gamePrompt_;
  s.lastLinkKind = lastLinkKind_;
  s.bestRelation = bestRelation();
  s.attentionReason = attentionReason_;
  s.notice = notice_;
  memcpy(s.friends, friends_, sizeof(friends_));
  memcpy(s.family, family_, sizeof(family_));
  memcpy(s.familyAncestry, familyAncestry_, sizeof(familyAncestry_));
  return s;
}

void EchoPetModel::clearNotice() {
  notice_ = Notice::kNone;
}

void EchoPetModel::tickOnePetMinute() {
  const Stage previousStage = stage_;
  ageMinutes_++;
  advanceClockOneMinute();

  if (lifeState_ == LifeState::kPassed) {
    updateMood();
    return;
  }

  if (stage_ == Stage::kParentCare) {
    parentCareMinutes_++;
    if (parentCareMinutes_ >= kFamilyTimingRule.parentLeaveMinutes) {
      parentLeaves();
    }
  }

  const bool bedtime = isBedtime();
  for (const StatDecayRule& rule : kStatDecayRules) {
    const uint16_t period =
        statDecayPeriodForStage(rule.meter, stage_, rule.periodMinutes);
    if (period == 0 || (ageMinutes_ % period) != 0) continue;
    if (rule.skipAtBedtime && bedtime) continue;
    switch (rule.meter) {
      case StatMeter::kHunger:
        hunger_ = clampAddHungryHappy(hunger_, rule.delta);
        break;
      case StatMeter::kHappiness:
        happiness_ = clampAddHungryHappy(happiness_, rule.delta);
        break;
      case StatMeter::kEnergy:
        energy_ = clampAdd(energy_, rule.delta);
        break;
      case StatMeter::kHygiene:
        hygiene_ = clampAdd(hygiene_, rule.delta);
        break;
    }
  }

  const uint16_t messPeriod = messPeriodForStage(stage_);
  if (messPeriod != 0 && (ageMinutes_ % messPeriod) == 0 &&
      messCount_ < kMaxMessCount) {
    messCount_++;
    hygiene_ = clampAdd(hygiene_, kMessHygienePenalty);
    if (donations_ >= kSuperCleanerDonation) {
      messCount_ = 0;
      hygiene_ = clampAdd(hygiene_, 4);
      notice_ = Notice::kClean;
    }
  }

  if (!bedtime && lightsOff_) {
    lightsOff_ = 0;
    notice_ = Notice::kLightsOn;
  }
  if (bedtime || lightsOff_) {
    memoryFlags_ &= ~kMemoryFlagCostumeMask;
  }
  if (bedtime && !lightsOff_) {
    const AttentionTimingRule& sleepAttention =
        attentionTimingRuleFor(AttentionTimingKind::kSleepAttention);
    if ((clockMinutes_ % sleepAttention.periodMinutes) ==
        sleepAttention.minuteOffset) {
      attention_ = 1;
      attentionReason_ = AttentionReason::kPraise;
      notice_ = Notice::kSleeping;
    }
    const AttentionTimingRule& lightsMistake =
        attentionTimingRuleFor(AttentionTimingKind::kLightsLeftOnMistake);
    if ((clockMinutes_ % lightsMistake.periodMinutes) ==
        lightsMistake.minuteOffset) {
      recordCareMistake(CareMistakeClass::kMental);
      careScore_ = clampAdd(careScore_, -4);
      happiness_ = clampAddHungryHappy(happiness_, -4);
    }
  }

  if (bedtime || lightsOff_) {
    energy_ = clampAdd(energy_, 3);
    happiness_ = clampAddHungryHappy(happiness_, 1);
  }

  if (attention_) {
    if (attentionMinutes_ < 255) attentionMinutes_++;
    const AttentionTimingRule& missWindow =
        attentionTimingRuleFor(AttentionTimingKind::kOrdinaryMiss);
    if (attentionMinutes_ >= missWindow.periodMinutes) {
      const bool missedToothacheOnly =
          attentionReason_ == AttentionReason::kSick && toothache_ &&
          !sickness_;
      if (missedToothacheOnly) {
        toothache_ = 0;
        becomeSick(1);
        attentionMinutes_ = 0;
        notice_ = Notice::kNeedsCare;
        happiness_ = clampAddHungryHappy(happiness_, -3);
      } else {
        switch (attentionReason_) {
          case AttentionReason::kHungry:
          case AttentionReason::kDirty:
          case AttentionReason::kSick:
          case AttentionReason::kNaughty:
            recordCareMistake(CareMistakeClass::kPhysical);
            break;
          case AttentionReason::kSad:
          case AttentionReason::kPraise:
          case AttentionReason::kNone:
            recordCareMistake(CareMistakeClass::kMental);
            break;
        }
        careScore_ = clampAdd(careScore_, -8);
        happiness_ = clampAddHungryHappy(happiness_, -5);
        attention_ = 0;
        attentionMinutes_ = 0;
        attentionReason_ = AttentionReason::kNone;
      }
    }
  }

  const AttentionTimingRule& emptyStatMistake =
      attentionTimingRuleFor(AttentionTimingKind::kEmptyStatMistake);
  if ((hunger_ == 0 || happiness_ == 0 || hygiene_ == 0) &&
      (ageMinutes_ % emptyStatMistake.periodMinutes) ==
          emptyStatMistake.minuteOffset) {
    if (hunger_ == 0 || hygiene_ == 0) {
      recordCareMistake(CareMistakeClass::kPhysical);
    }
    if (happiness_ == 0) {
      recordCareMistake(CareMistakeClass::kMental);
    }
    careScore_ = clampAdd(careScore_, -6);
  }

  const bool tamaDrinkGuard = (memoryFlags_ & kMemoryFlagTamaDrinkGuard) != 0;
  if (!sickness_) {
    for (const SicknessTriggerRule& rule : kSicknessTriggerRules) {
      if (rule.kind == SicknessTriggerKind::kSweetSnack) continue;
      bool triggerActive = false;
      switch (rule.kind) {
        case SicknessTriggerKind::kLowHygiene:
          triggerActive = hygiene_ < rule.threshold;
          break;
        case SicknessTriggerKind::kMess:
          triggerActive = messCount_ >= rule.threshold;
          break;
        case SicknessTriggerKind::kHighWeight:
          triggerActive = weight_ > rule.threshold;
          break;
        case SicknessTriggerKind::kSweetSnack:
          break;
      }
      if (!triggerActive) continue;
      if (nextRand(100) >=
          guardedSickChance(rule.chancePercent, tamaDrinkGuard)) {
        continue;
      }
      uint8_t severity = rule.minSeverity;
      if (rule.severityRollCount > 1) {
        severity += nextRand(rule.severityRollCount);
      }
      becomeSick(severity);
      break;
    }
  }

  const AttentionTimingRule& careCall =
      attentionTimingRuleFor(AttentionTimingKind::kCareCall);
  if ((ageMinutes_ % careCall.periodMinutes) == careCall.minuteOffset) {
    callForCare();
  }
  checkCalendarEvents();
  checkMatchmaker();
  checkDeath();

  clampStats();
  updateGrowth();
  updateMood();
  if (stage_ != previousStage && lifeState_ == LifeState::kAlive) {
    notice_ = Notice::kGrew;
  }
}

void EchoPetModel::advanceClockOneMinute() {
  clockMinutes_++;
  if (clockMinutes_ >= 24 * 60) {
    clockMinutes_ = 0;
    day_++;
    if (day_ > daysInMonth(month_)) {
      day_ = 1;
      month_++;
      if (month_ > 12) month_ = 1;
    }
  }
}

void EchoPetModel::updateGrowth() {
  if (lifeState_ == LifeState::kPassed || stage_ == Stage::kParentCare) return;

  const Stage next = scheduledStageForAge(stage_, growthFlags_, ageMinutes_);

  if (next != stage_) {
    const Stage previous = stage_;
    stage_ = next;
    applyGrowthTransition(previous, next);
    characterCatalogId_ = chooseCharacterCatalogId(stage_, route_);
    character_ = characterCatalogEntry(characterCatalogId_).archetype;
    if (next == Stage::kAdult &&
        (growthFlags_ & kGrowthFlagOyajitchiLineage)) {
      growthFlags_ &= static_cast<uint8_t>(~kGrowthFlagOyajitchiLineage);
    }
  } else if (stage_ == Stage::kEgg) {
    character_ = CharacterKind::kShell;
    characterCatalogId_ = 0;
  }
}

void EchoPetModel::updateMood() {
  if (lifeState_ == LifeState::kPassed) {
    mood_ = Mood::kPassed;
  } else if (isBedtime() || lightsOff_) {
    mood_ = Mood::kAsleep;
  } else if (sickness_ || toothache_) {
    mood_ = Mood::kSick;
  } else {
    const uint16_t average =
        visibleHeartMeterValue(hunger_) + visibleHeartMeterValue(happiness_) +
        energy_ + hygiene_ + friendship_;
    if (average >= 380) {
      mood_ = Mood::kGreat;
    } else if (average >= 250) {
      mood_ = Mood::kOkay;
    } else {
      mood_ = Mood::kSad;
    }
  }
}

void EchoPetModel::recordCareMistake(CareMistakeClass type) {
  if (careMistakes_ < 65535) careMistakes_++;
  if (type == CareMistakeClass::kPhysical) {
    if (physicalCareMistakes_ < kCareMistakeCap) physicalCareMistakes_++;
  } else {
    if (mentalCareMistakes_ < kCareMistakeCap) mentalCareMistakes_++;
  }
}

bool EchoPetModel::reverseCareMistake(CareMistakeClass type) {
  uint8_t& counter = type == CareMistakeClass::kPhysical
                         ? physicalCareMistakes_
                         : mentalCareMistakes_;
  if (counter == 0) return false;
  counter--;
  if (careMistakes_ > 0) careMistakes_--;
  careScore_ = clampAdd(careScore_, 2);
  return true;
}

void EchoPetModel::clearAttentionFor(Action action) {
  if (!attention_ || !actionAddressesAttention(action, attentionReason_)) {
    return;
  }
  const bool stillSick =
      attentionReason_ == AttentionReason::kSick && (sickness_ || toothache_);
  if (stillSick) {
    return;
  }
  attention_ = 0;
  attentionMinutes_ = 0;
  attentionReason_ = AttentionReason::kNone;
}

void EchoPetModel::resetStageCareMistakes() {
  physicalCareMistakes_ = 0;
  mentalCareMistakes_ = 0;
}

void EchoPetModel::applyGrowthTransition(Stage previous, Stage next) {
  memoryFlags_ &= ~kMemoryFlagTamaDrinkGuard;
  sweetSnackStreak_ = 0;
  lastSweetSnackMinute_ = kSweetSnackNoLastMinute;
  stageSicknesses_ = 0;
  const uint16_t baseWeight = baseWeightForStage(next);
  if (weight_ < baseWeight) weight_ = baseWeight;

  if (previous == Stage::kEgg && next == Stage::kBaby) {
    route_ = GrowthRoute::kBalanced;
    resetStageCareMistakes();
    return;
  }

  if (next == Stage::kChild) {
    growthTier_ = generation_ <= 1
                      ? GrowthTier::kFirstGeneration
                      : childTierFromParentAdultTiers(parentAdultTierA_,
                                                      parentAdultTierB_);
  } else if (next == Stage::kTeen) {
    growthTier_ = teenTierFromChildTier(growthTier_, physicalCareMistakes_,
                                        mentalCareMistakes_, generation_);
  } else if (next == Stage::kAdult) {
    if (growthFlags_ & kGrowthFlagOyajitchiLineage) {
      adultTier_ = AdultTier::kSpecial;
    } else {
      adultTier_ = adultTierFromStageMistakes(
          growthTier_, physicalCareMistakes_, mentalCareMistakes_,
          bornFromUnhealthyParents_ != 0);
    }
  } else if (next == Stage::kElder) {
    adultTier_ = gender_ == Gender::kBoy ? AdultTier::kSerious
                                         : AdultTier::kSpecial;
  }

  route_ = routeForGrowthState();
  resetStageCareMistakes();
}

GrowthRoute EchoPetModel::routeForGrowthState() const {
  if (stage_ == Stage::kBaby || stage_ == Stage::kEgg) {
    return GrowthRoute::kBalanced;
  }
  if (stage_ == Stage::kElder) {
    return gender_ == Gender::kBoy ? GrowthRoute::kBalanced
                                   : GrowthRoute::kScholar;
  }
  if (stage_ == Stage::kAdult || stage_ == Stage::kParentCare) {
    if (stage_ == Stage::kAdult &&
        (growthFlags_ & kGrowthFlagOyajitchiLineage)) {
      return GrowthRoute::kRascal;
    }
    if (socialScore_ >= 70 || friendVisits_ >= 5) return GrowthRoute::kSocial;
    switch (adultTier_) {
      case AdultTier::kSerious:
        return mentalCareMistakes_ > physicalCareMistakes_
                   ? GrowthRoute::kScholar
                   : GrowthRoute::kBalanced;
      case AdultTier::kNormal:
        return physicalCareMistakes_ > mentalCareMistakes_
                   ? GrowthRoute::kAthlete
                   : GrowthRoute::kDreamer;
      case AdultTier::kNaughty:
        return GrowthRoute::kAthlete;
      case AdultTier::kFrail:
        return GrowthRoute::kRascal;
      case AdultTier::kSpecial:
        return GrowthRoute::kSocial;
    }
  }

  switch (growthTier_) {
    case GrowthTier::kFirstGeneration:
    case GrowthTier::kTierOne:
      return mentalCareMistakes_ > physicalCareMistakes_
                 ? GrowthRoute::kScholar
                 : GrowthRoute::kBalanced;
    case GrowthTier::kTierTwo:
      return physicalCareMistakes_ > mentalCareMistakes_
                 ? GrowthRoute::kAthlete
                 : GrowthRoute::kSocial;
    case GrowthTier::kTierThree:
      return GrowthRoute::kDreamer;
    case GrowthTier::kTierFour:
      return GrowthRoute::kRascal;
  }
  return GrowthRoute::kBalanced;
}

void EchoPetModel::callForCare() {
  const AttentionReason previous = attentionReason_;
  if (sickness_ || toothache_) {
    attentionReason_ = AttentionReason::kSick;
  } else if (hunger_ < 35) {
    attentionReason_ = AttentionReason::kHungry;
  } else if (happiness_ < 30) {
    attentionReason_ = AttentionReason::kSad;
  } else if (hygiene_ < 30 || messCount_) {
    attentionReason_ = AttentionReason::kDirty;
  } else if (nextRand(100) < 12) {
    attentionReason_ = nextRand(2) ? AttentionReason::kNaughty
                                   : AttentionReason::kPraise;
  } else {
    return;
  }

  attention_ = 1;
  if (previous != attentionReason_) attentionMinutes_ = 0;
  if (attention_) notice_ = Notice::kNeedsCare;
}

void EchoPetModel::checkCalendarEvents() {
  if (lifeState_ == LifeState::kPassed || stage_ == Stage::kEgg) return;

  const uint8_t slot = calendarSlotMaskForMinute(clockMinutes_);
  if (!slot) return;
  const uint16_t key = calendarEventKey(month_, day_, slot);
  if (key == lastAnniversaryDay_) return;

  if ((slot & kCalendarStandardSlots) && month_ == birthdayMonth_ &&
      day_ == birthdayDay_) {
    lastAnniversaryDay_ = key;
    notice_ = Notice::kBirthday;
    return;
  }

  for (const CalendarEventRule& rule : kCalendarEventRules) {
    if ((rule.slotMask & slot) == 0) continue;
    if (!dateInRange(month_, day_, rule.startMonth, rule.startDay,
                     rule.endMonth, rule.endDay)) {
      continue;
    }
    lastAnniversaryDay_ = key;
    notice_ = rule.notice;
    return;
  }
}

void EchoPetModel::checkMatchmaker() {
  if (!isAdultLike() || stage_ == Stage::kParentCare ||
      bestRelation() >= RelationLevel::kPartner) {
    return;
  }
  if (matchmakerNoticeReady(ageMinutes_, clockMinutes_)) {
    notice_ = Notice::kMatchmaker;
  }
}

void EchoPetModel::checkDeath() {
  if (lifeState_ == LifeState::kPassed) return;
  if (stageSicknesses_ >= kStageSicknessPassAwayCount) {
    lifeState_ = LifeState::kPassed;
    notice_ = Notice::kPassed;
    return;
  }
  const RelationLevel relation = bestRelation();
  for (const PassAwayRule& rule : kPassAwayRules) {
    if (passAwayRuleMatches(rule, stage_, ageMinutes_, careMistakes_,
                            sickness_, hunger_, happiness_, hygiene_,
                            relation)) {
      lifeState_ = LifeState::kPassed;
      notice_ = Notice::kPassed;
      return;
    }
  }
}

void EchoPetModel::becomeSick(uint8_t severity) {
  if (lifeState_ == LifeState::kPassed || severity == 0) return;
  if (sickness_ == 0 &&
      stageSicknesses_ < kStageSicknessPassAwayCount) {
    stageSicknesses_++;
  }
  sickness_ = severity > 2 ? 2 : severity;
}

void EchoPetModel::startBaby(AdultTier partnerAdultTier,
                             bool oyajitchiLineage,
                             uint8_t partnerCatalogId) {
  if (!isAdultLike()) return;
  parentAdultTierA_ = adultTier_;
  parentAdultTierB_ = partnerAdultTier;
  parentCatalogIdA_ =
      characterCatalogId_ < kCharacterCatalogCount
          ? characterCatalogId_
          : catalogIdForCharacterKind(character_);
  parentCatalogIdB_ = partnerCatalogId < kCharacterCatalogCount
                          ? partnerCatalogId
                          : kCatalogIdUnknown;
  const bool bothParentsUnhealthy =
      adultTierIsUnhealthy(parentAdultTierA_) &&
      adultTierIsUnhealthy(parentAdultTierB_);
  bornFromUnhealthyParents_ = bothParentsUnhealthy ? 1 : 0;
  if (oyajitchiLineage) {
    growthFlags_ |= kGrowthFlagOyajitchiLineage;
  } else {
    growthFlags_ &= static_cast<uint8_t>(~kGrowthFlagOyajitchiLineage);
  }
  stage_ = Stage::kParentCare;
  route_ = routeForGrowthState();
  characterCatalogId_ = chooseCharacterCatalogId(stage_, route_);
  character_ = characterCatalogEntry(characterCatalogId_).archetype;
  parentCareMinutes_ = 0;
  friendship_ = clampAdd(friendship_, 15);
  happiness_ = clampAddHungryHappy(happiness_, 15);
  notice_ = Notice::kBaby;
}

void EchoPetModel::parentLeaves() {
  const bool oyajitchiLineage =
      (growthFlags_ & kGrowthFlagOyajitchiLineage) != 0;
  const Gender childGender =
      oyajitchiLineage
          ? Gender::kBoy
          : (nextRand(2) ? Gender::kGirl : Gender::kBoy);
  const uint8_t babyCatalogId = babyCatalogIdForGender(childGender);
  const GrowthTier childGrowthTier =
      childTierFromParentAdultTiers(parentAdultTierA_, parentAdultTierB_);
  addFamilyRecord(babyCatalogId, childGrowthTier, oyajitchiLineage);
  generation_++;
  donations_ = 0;
  gender_ = childGender;
  makeDefaultNames();
  resetPetBody(rngState_ ^ generation_, true);
  stage_ = Stage::kBaby;
  ageMinutes_ = kEggHatchAgeMinutes;
  route_ = routeForGrowthState();
  characterCatalogId_ = chooseCharacterCatalogId(stage_, route_);
  character_ = characterCatalogEntry(characterCatalogId_).archetype;
  notice_ = Notice::kParentLeft;
}

void EchoPetModel::addFamilyRecord(uint8_t babyCatalogId,
                                   GrowthTier childGrowthTier,
                                   bool oyajitchiLineage) {
  for (int8_t i = kFamilyHistoryCount - 1; i > 0; i--) {
    family_[i] = family_[i - 1];
    familyAncestry_[i] = familyAncestry_[i - 1];
  }
  const uint8_t parentCatalog =
      parentCatalogIdA_ < kCharacterCatalogCount
          ? parentCatalogIdA_
          : (characterCatalogId_ < kCharacterCatalogCount
                 ? characterCatalogId_
                 : catalogIdForCharacterKind(character_));
  family_[0].petId = petId_;
  family_[0].generation = generation_;
  family_[0].catalogId = parentCatalog;
  if (parentCatalogIdB_ < kCharacterCatalogCount) {
    family_[0].character = parentCatalogIdB_;
    family_[0].route = static_cast<uint8_t>(route_) |
                       kFamilyRecordPartnerCatalogValid;
  } else {
    family_[0].character =
        static_cast<uint8_t>(characterCatalogEntry(parentCatalog).archetype);
    family_[0].route = static_cast<uint8_t>(route_);
  }
  family_[0].careScore = careScore_;
  FamilyAncestryRecord& ancestry = familyAncestry_[0];
  memset(&ancestry, 0, sizeof(ancestry));
  ancestry.flags = kFamilyAncestryValid;
  if (parentCatalogIdB_ < kCharacterCatalogCount) {
    ancestry.flags |= kFamilyAncestryPartnerCatalogValid;
  }
  if (oyajitchiLineage) {
    ancestry.flags |= kFamilyAncestryOyajitchiLineage;
  }
  ancestry.parentCatalogIdA = parentCatalog;
  ancestry.parentCatalogIdB =
      parentCatalogIdB_ < kCharacterCatalogCount ? parentCatalogIdB_
                                                 : kCatalogIdUnknown;
  ancestry.babyCatalogId =
      babyCatalogId < kCharacterCatalogCount ? babyCatalogId
                                             : kCatalogIdUnknown;
  ancestry.parentAdultTierA = static_cast<uint8_t>(parentAdultTierA_);
  ancestry.parentAdultTierB = static_cast<uint8_t>(parentAdultTierB_);
  ancestry.childGrowthTier = static_cast<uint8_t>(childGrowthTier);
}

void EchoPetModel::clampStats() {
  if (hunger_ > kHiddenHeartMeterMax) hunger_ = kHiddenHeartMeterMax;
  if (happiness_ > kHiddenHeartMeterMax) happiness_ = kHiddenHeartMeterMax;
  if (energy_ > 100) energy_ = 100;
  if (hygiene_ > 100) hygiene_ = 100;
  if (discipline_ > 100) discipline_ = 100;
  if (friendship_ > 100) friendship_ = 100;
  if (points_ > kMaxGotchiPoints) points_ = kMaxGotchiPoints;
  if (messCount_ > 4) messCount_ = 4;
  if (sickness_ > 2) sickness_ = 2;
  if (toothache_ > 1) toothache_ = 1;
  if (stageSicknesses_ > kStageSicknessPassAwayCount) {
    stageSicknesses_ = kStageSicknessPassAwayCount;
  }
  if (sweetSnackStreak_ > kSweetSnackToothacheStreak) {
    sweetSnackStreak_ = kSweetSnackToothacheStreak;
  }
  if (sweetSnackStreak_ == 0) {
    lastSweetSnackMinute_ = kSweetSnackNoLastMinute;
  }
  for (uint8_t i = 0; i < kFoodKindCount; i++) {
    if (foodCounts_[i] > kMaxFoodStock) foodCounts_[i] = kMaxFoodStock;
  }
  for (uint8_t i = 0; i < kItemKindCount; i++) {
    if (itemCounts_[i] > kMaxItemStock) itemCounts_[i] = kMaxItemStock;
  }
}

void EchoPetModel::startMiniGame(MiniGameKind requested) {
  const uint8_t count = unlockedGameCount();
  uint8_t next = static_cast<uint8_t>(requested);
  if (next == 0) {
    next = lastGameKind_ + 1;
    if (next == 0 || next > count) next = 1;
  }
  if (next > count) next = count;
  miniGame_ = static_cast<MiniGameKind>(next);
  lastGameKind_ = next;
  gameRound_ = 0;
  gameScore_ = 0;
  gameExpectedYes_ = 0;
  gameCursor_ = 1;
  gameTarget_ = 1;
  gameHazard_ = 0;
  notice_ = Notice::kGameStart;
  setupGameRound();
  gameRoundStartMs_ = lastMs_;
}

void EchoPetModel::setupGameRound() {
  switch (miniGame_) {
    case MiniGameKind::kGet: {
      const uint8_t progress =
          scaledProgress(gameRound_, gameRoundLimitFor(miniGame_), 100);
      gameTarget_ = nextRand(3);
      gameHazard_ =
          nextRand(100) < static_cast<uint8_t>(18 + progress / 5);
      gameExpectedYes_ = gameHazard_ ? 0 : static_cast<uint8_t>(1 + nextRand(2));
      gamePrompt_ = gameHazard_ ? GamePrompt::kDodge : GamePrompt::kCatch;
      break;
    }
    case MiniGameKind::kBump: {
      const uint8_t progress =
          scaledProgress(gameRound_, gameRoundLimitFor(miniGame_), 100);
      gameCursor_ = 0;
      gameTarget_ = static_cast<uint8_t>(48 + progress / 3);
      gameHazard_ = 0;
      gameExpectedYes_ = buttonValue(GameButton::kB);
      gamePrompt_ = GamePrompt::kPush;
      break;
    }
    case MiniGameKind::kFlag: {
      const uint8_t flagKind = nextRand(3);
      gameExpectedYes_ = flagKind == buttonValue(GameButton::kB)
                             ? buttonValue(GameButton::kAC)
                             : flagKind;
      gameTarget_ = gameExpectedYes_ == buttonValue(GameButton::kAC)
                        ? 1
                        : (nextRand(2) == 0 ? 0 : 2);
      const bool fakeSignal =
          nextRand(100) < kFlagFakeSignalChancePercent;
      gameHazard_ = fakeSignal ? kFlagFakeSignal : 0;
      gamePrompt_ = fakeSignal ? GamePrompt::kWait : GamePrompt::kNone;
      break;
    }
    case MiniGameKind::kHeading: {
      gameTarget_ = nextRand(3);
      gameHazard_ = 0;
      gameExpectedYes_ = buttonValue(GameButton::kB);
      gamePrompt_ = GamePrompt::kHit;
      break;
    }
    case MiniGameKind::kMemory: {
      const uint8_t sequenceLen = memoryPatternLengthForRound(gameRound_);
      gameHazard_ = 0;
      for (uint8_t i = 0; i < sequenceLen; i++) {
        const uint8_t code = nextRand(3);
        gameHazard_ |= static_cast<uint32_t>(code) << (i * 2);
      }
      gameCursor_ = 0;
      gameTarget_ = sequenceLen;
      gameExpectedYes_ = memorySequenceAt(gameHazard_, 0);
      gamePrompt_ = GamePrompt::kCopy;
      break;
    }
    case MiniGameKind::kSprint: {
      gameCursor_ = 0;
      gameTarget_ = sprintTapTargetFor(weight_);
      gameHazard_ = 0;
      gameExpectedYes_ = buttonValue(GameButton::kB);
      gamePrompt_ = GamePrompt::kPress;
      break;
    }
    case MiniGameKind::kHoops: {
      gameCursor_ = 1;
      gameTarget_ = nextRand(3);
      gameHazard_ = 0;
      gameExpectedYes_ = buttonValue(GameButton::kB);
      gamePrompt_ = GamePrompt::kShoot;
      break;
    }
    case MiniGameKind::kNone:
      gameExpectedYes_ = 0;
      gamePrompt_ = GamePrompt::kNone;
      break;
  }
}

void EchoPetModel::finishMiniGame() {
  static const int8_t kHappyCurve[6] = {0, 3, 7, 13, 20, 28};
  static const int8_t kFriendCurve[6] = {0, 0, 1, 3, 5, 8};
  const uint8_t scoreLimit = gameScoreLimitFor(miniGame_);
  const uint8_t score = gameScore_ > scoreLimit ? scoreLimit : gameScore_;
  const uint8_t scoreTier = gameScoreTier(score, scoreLimit);
  const uint16_t fullPrize = fullPrizeFor(stage_, miniGame_, weight_);
  const uint16_t pointPrize =
      gamePrizeForScore(miniGame_, fullPrize, score, weight_);

  hunger_ = clampAddHungryHappy(hunger_, -static_cast<int>(3 + scoreTier));
  energy_ = clampAdd(energy_, -static_cast<int>(6 + scoreTier * 2));
  happiness_ = clampAddHungryHappy(happiness_, kHappyCurve[scoreTier]);
  friendship_ = clampAdd(friendship_, kFriendCurve[scoreTier]);
  points_ = clampAdd16(points_, pointPrize, kMaxGotchiPoints);
  playScore_ = clampAdd(playScore_, scoreTier * 3);
  gamePlays_ = clampAdd(gamePlays_, 1);
  if (score > bestGameScore_) bestGameScore_ = score;

  if (scoreLimit > 0 && score >= scoreLimit) {
    switch (miniGame_) {
      case MiniGameKind::kGet:
      case MiniGameKind::kFlag:
        reverseCareMistake(CareMistakeClass::kMental);
        break;
      case MiniGameKind::kBump:
      case MiniGameKind::kHeading:
        reverseCareMistake(CareMistakeClass::kPhysical);
        break;
      case MiniGameKind::kMemory:
      case MiniGameKind::kSprint:
      case MiniGameKind::kHoops:
      case MiniGameKind::kNone:
        break;
    }
  }

  if (scoreTier >= 3) {
    gameWins_ = clampAdd(gameWins_, 1);
    gameStreak_ = clampAdd(gameStreak_, 1);
    const uint16_t baseWeight = baseWeightForStage(stage_);
    if (weight_ > baseWeight) weight_--;
    if (scoreTier >= 4) {
      awardCatalogItem(static_cast<uint8_t>(
          (scoreTier * 19 + gameValue(miniGame_) * 11 + gameWins_) %
          kCatalogItemCount));
    }
    notice_ = Notice::kGameWin;
  } else {
    gameStreak_ = 0;
    notice_ = Notice::kGameLose;
  }

  miniGame_ = MiniGameKind::kNone;
  gamePrompt_ = GamePrompt::kNone;
  gameExpectedYes_ = 0;
  gameCursor_ = 1;
  gameTarget_ = 1;
  gameHazard_ = 0;
  gameRoundStartMs_ = 0;
}

bool EchoPetModel::buyShopStock() {
  return buyCatalogItem(catalogShopIndex(0, month_, day_, birthdayMonth_,
                                         birthdayDay_, clockMinutes_));
}

bool EchoPetModel::buyCatalogItem(uint8_t index) {
  if (index >= kCatalogItemCount) {
    notice_ = Notice::kNoItem;
    return false;
  }

  const CatalogEntry entry = catalogEntry(index);
  const uint16_t price = catalogShopPrice(index, day_, clockMinutes_);
  if (entry.kind == CatalogKind::kFood &&
      catalogStockTotal(CatalogKind::kFood) >= kMaxFoodStock) {
    notice_ = Notice::kInventoryFull;
    return false;
  }
  if (entry.kind == CatalogKind::kFood &&
      catalogStock_[index] >= kMaxFoodStock) {
    notice_ = Notice::kInventoryFull;
    return false;
  }
  if (entry.kind == CatalogKind::kItem &&
      (entry.flags & kCatalogFlagReusable) == 0 &&
      catalogStockTotal(CatalogKind::kItem) >= kMaxItemStock) {
    notice_ = Notice::kInventoryFull;
    return false;
  }
  if (entry.kind == CatalogKind::kItem &&
      (entry.flags & kCatalogFlagReusable) == 0 &&
      catalogStock_[index] >= kMaxItemStock) {
    notice_ = Notice::kInventoryFull;
    return false;
  }
  if (points_ < price) {
    notice_ = Notice::kNoPoints;
    return false;
  }

  awardCatalogItem(index);
  points_ -= price;
  notice_ = Notice::kBought;
  return true;
}

void EchoPetModel::awardFood(FoodKind food, uint8_t amount) {
  const uint8_t i = foodIndex(food);
  if (i >= kFoodKindCount) return;
  uint16_t next = static_cast<uint16_t>(foodCounts_[i]) + amount;
  if (next > kMaxFoodStock) next = kMaxFoodStock;
  foodCounts_[i] = static_cast<uint8_t>(next);
}

void EchoPetModel::awardItem(ItemKind item, uint8_t amount) {
  const uint8_t i = itemIndex(item);
  if (i >= kItemKindCount) return;
  uint16_t next = static_cast<uint16_t>(itemCounts_[i]) + amount;
  if (next > kMaxItemStock) next = kMaxItemStock;
  itemCounts_[i] = static_cast<uint8_t>(next);
}

void EchoPetModel::awardCatalogItem(uint8_t index) {
  if (index >= kCatalogItemCount) return;
  catalogBitSet(catalogOwned_, index);
  const CatalogEntry entry = catalogEntry(index);
  switch (entry.kind) {
    case CatalogKind::kFood:
      awardFood(static_cast<FoodKind>(entry.behavior % kFoodKindCount), 1);
      if (catalogStock_[index] < kMaxFoodStock) {
        catalogStock_[index]++;
      }
      break;
    case CatalogKind::kItem:
      if ((entry.flags & kCatalogFlagReusable) == 0) {
        awardItem(static_cast<ItemKind>(entry.behavior % kItemKindCount), 1);
        if (catalogStock_[index] < kMaxItemStock) {
          catalogStock_[index]++;
        }
      }
      break;
    case CatalogKind::kSouvenir:
      awardSouvenir(entry.behavior % kCatalogSouvenirCount);
      break;
  }
}

void EchoPetModel::awardSouvenir(uint8_t index) {
  if (index >= kCatalogSouvenirCount) return;
  if (index < kSouvenirCount) {
    souvenirMask_ |= static_cast<uint16_t>(1U << index);
  }
  catalogBitSet(souvenirOwned_, index);
  memoryFlags_ |= kMemoryFlagSouvenirSeen;
}

bool EchoPetModel::catalogGiftAvailable(uint8_t index) const {
  if (index >= kCatalogItemCount) return false;
  const CatalogEntry entry = catalogEntry(index);
  switch (entry.kind) {
    case CatalogKind::kFood:
      return catalogStock_[index] > 0;
    case CatalogKind::kItem:
      if ((entry.flags & kCatalogFlagReusable) != 0) {
        return catalogBitTest(catalogOwned_, index);
      }
      return catalogStock_[index] > 0;
    case CatalogKind::kSouvenir:
      return false;
  }
  return false;
}

bool EchoPetModel::debitCatalogGift(uint8_t index, bool updateNotice) {
  if (!catalogGiftAvailable(index)) {
    if (updateNotice) notice_ = Notice::kNoItem;
    return false;
  }

  const CatalogEntry entry = catalogEntry(index);
  switch (entry.kind) {
    case CatalogKind::kFood: {
      const uint8_t food =
          static_cast<uint8_t>(entry.behavior % kFoodKindCount);
      if (foodCounts_[food] > 0) {
        foodCounts_[food]--;
      }
      catalogStock_[index]--;
      clearCatalogStockIfEmpty(index, entry);
      return true;
    }
    case CatalogKind::kItem:
      if ((entry.flags & kCatalogFlagReusable) != 0) {
        catalogBitClear(catalogOwned_, index);
        return true;
      } else {
        const uint8_t item =
            static_cast<uint8_t>(entry.behavior % kItemKindCount);
        if (itemCounts_[item] > 0) {
          itemCounts_[item]--;
        }
        catalogStock_[index]--;
        clearCatalogStockIfEmpty(index, entry);
        return true;
      }
    case CatalogKind::kSouvenir:
      break;
  }

  if (updateNotice) notice_ = Notice::kNoItem;
  return false;
}

bool EchoPetModel::receiveCatalogGift(uint8_t index, bool& inventoryFull) {
  inventoryFull = false;
  if (index >= kCatalogItemCount) return false;

  const CatalogEntry entry = catalogEntry(index);
  switch (entry.kind) {
    case CatalogKind::kFood:
      if (catalogStockTotal(CatalogKind::kFood) >= kMaxFoodStock ||
          catalogStock_[index] >= kMaxFoodStock) {
        inventoryFull = true;
        return false;
      }
      awardCatalogItem(index);
      return true;

    case CatalogKind::kItem:
      if ((entry.flags & kCatalogFlagReusable) != 0) {
        if (catalogBitTest(catalogOwned_, index)) {
          inventoryFull = true;
          return false;
        }
        awardCatalogItem(index);
        return true;
      }
      if (catalogStockTotal(CatalogKind::kItem) >= kMaxItemStock ||
          catalogStock_[index] >= kMaxItemStock) {
        inventoryFull = true;
        return false;
      }
      awardCatalogItem(index);
      return true;

    case CatalogKind::kSouvenir:
      return false;
  }
  return false;
}

void EchoPetModel::clearCatalogStockIfEmpty(uint8_t index,
                                            const CatalogEntry& entry) {
  if (index >= kCatalogItemCount) return;
  switch (entry.kind) {
    case CatalogKind::kFood:
      if (catalogStock_[index] == 0) {
        catalogBitClear(catalogOwned_, index);
      }
      break;
    case CatalogKind::kItem:
      if ((entry.flags & kCatalogFlagReusable) == 0 &&
          catalogStock_[index] == 0) {
        catalogBitClear(catalogOwned_, index);
      }
      break;
    case CatalogKind::kSouvenir:
      break;
  }
}

void EchoPetModel::rebuildCatalogStockFromOwned() {
  memset(catalogStock_, 0, sizeof(catalogStock_));
  uint8_t foodRemaining[kFoodKindCount];
  uint8_t itemRemaining[kItemKindCount];
  memcpy(foodRemaining, foodCounts_, sizeof(foodRemaining));
  memcpy(itemRemaining, itemCounts_, sizeof(itemRemaining));

  for (uint8_t index = 0; index < kCatalogItemCount; index++) {
    if (!catalogBitTest(catalogOwned_, index)) {
      continue;
    }
    const CatalogEntry entry = catalogEntry(index);
    if (entry.kind == CatalogKind::kFood) {
      const uint8_t food = entry.behavior % kFoodKindCount;
      if (foodRemaining[food] == 0) {
        catalogBitClear(catalogOwned_, index);
        continue;
      }
      catalogStock_[index] = 1;
      foodRemaining[food]--;
    } else if (entry.kind == CatalogKind::kItem &&
               (entry.flags & kCatalogFlagReusable) == 0) {
      const uint8_t item = entry.behavior % kItemKindCount;
      if (itemRemaining[item] == 0) {
        catalogBitClear(catalogOwned_, index);
        continue;
      }
      catalogStock_[index] = 1;
      itemRemaining[item]--;
    }
  }
}

uint16_t EchoPetModel::catalogStockTotal(CatalogKind kind) const {
  uint16_t total = 0;
  for (uint8_t index = 0; index < kCatalogItemCount; index++) {
    const CatalogEntry entry = catalogEntry(index);
    if (entry.kind != kind) {
      continue;
    }
    if (entry.kind == CatalogKind::kItem &&
        (entry.flags & kCatalogFlagReusable) != 0) {
      continue;
    }
    total = static_cast<uint16_t>(total + catalogStock_[index]);
  }
  return total;
}

bool EchoPetModel::consumeCatalogItem(uint8_t index,
                                      const CatalogEntry& entry) {
  if ((entry.flags & kCatalogFlagReusable) != 0) {
    return true;
  }
  if (index >= kCatalogItemCount || catalogStock_[index] == 0) {
    notice_ = Notice::kNoItem;
    return false;
  }
  const uint8_t item = static_cast<uint8_t>(entry.behavior % kItemKindCount);
  if (itemCounts_[item] > 0) {
    itemCounts_[item]--;
  }
  catalogStock_[index]--;
  clearCatalogStockIfEmpty(index, entry);
  return true;
}

bool EchoPetModel::useBestItem() {
  ItemKind item = ItemKind::kCount;
  if ((sickness_ || toothache_) && itemCounts_[itemIndex(ItemKind::kCharm)] > 0) {
    item = ItemKind::kCharm;
  } else if (happiness_ < 65 && itemCounts_[itemIndex(ItemKind::kToy)] > 0) {
    item = ItemKind::kToy;
  } else if (discipline_ < 75 && itemCounts_[itemIndex(ItemKind::kBook)] > 0) {
    item = ItemKind::kBook;
  } else {
    for (uint8_t i = 0; i < kItemKindCount; i++) {
      if (itemCounts_[i]) {
        item = static_cast<ItemKind>(i);
        break;
      }
    }
  }

  return useSpecificItem(item);
}

bool EchoPetModel::useSpecificItem(ItemKind item, bool consume) {
  if (item == ItemKind::kCount || itemIndex(item) >= kItemKindCount ||
      (consume && itemCounts_[itemIndex(item)] == 0)) {
    notice_ = Notice::kNoItem;
    return false;
  }

  if (consume) itemCounts_[itemIndex(item)]--;
  switch (item) {
    case ItemKind::kToy:
    case ItemKind::kBall:
    case ItemKind::kBlocks:
    case ItemKind::kRope:
      happiness_ = clampAddHungryHappy(happiness_, 12);
      friendship_ = clampAdd(friendship_, 8);
      playScore_ = clampAdd(playScore_, 3);
      break;
    case ItemKind::kBook:
      discipline_ = clampAdd(discipline_, 10);
      happiness_ = clampAddHungryHappy(happiness_, 3);
      disciplineScore_ = clampAdd(disciplineScore_, 6);
      break;
    case ItemKind::kCharm:
      sickness_ = 0;
      toothache_ = 0;
      sweetSnackStreak_ = 0;
      lastSweetSnackMinute_ = kSweetSnackNoLastMinute;
      attention_ = 0;
      happiness_ = clampAddHungryHappy(happiness_, 6);
      friendship_ = clampAdd(friendship_, 4);
      careScore_ = clampAdd(careScore_, 5);
      break;
    case ItemKind::kTrumpet:
      happiness_ = clampAddHungryHappy(happiness_, 8);
      friendship_ = clampAdd(friendship_, 10);
      socialScore_ = clampAdd(socialScore_, 5);
      break;
    case ItemKind::kTicket:
      awardSouvenir((month_ + day_ + generation_) % kSouvenirCount);
      happiness_ = clampAddHungryHappy(happiness_, 15);
      notice_ = Notice::kSouvenir;
      return true;
    case ItemKind::kCount:
      break;
  }
  notice_ = Notice::kItemUsed;
  return true;
}

void EchoPetModel::applyCatalogFoodTaste(uint8_t catalogIndex) {
  if (catalogFoodLikedByAll(catalogIndex)) {
    happiness_ = kHiddenHeartMeterMax;
    friendship_ = clampAdd(friendship_, 5);
    careScore_ = clampAdd(careScore_, 3);
    return;
  }

  const CharacterFoodTasteRule* rule =
      characterFoodTasteRuleFor(characterCatalogId_);
  if (!rule) return;
  if (rule->likedCatalogIndex == catalogIndex) {
    happiness_ = kHiddenHeartMeterMax;
    friendship_ = clampAdd(friendship_, 5);
    careScore_ = clampAdd(careScore_, 3);
  } else if (rule->dislikedCatalogIndex == catalogIndex) {
    happiness_ = 0;
    friendship_ = clampAdd(friendship_, -3);
    careScore_ = clampAdd(careScore_, -3);
  }
}

bool EchoPetModel::useFood(FoodKind food, bool snack, bool consumeInventory) {
  FoodKind chosen = food;
  if (consumeInventory && foodCounts_[foodIndex(chosen)] == 0) {
    notice_ = Notice::kNoItem;
    return false;
  }
  if (!snack && hunger_ >= kHiddenHeartMeterMax) {
    notice_ = Notice::kFull;
    return false;
  }

  if (consumeInventory) {
    foodCounts_[foodIndex(chosen)]--;
  }
  if (snack || isSnackFood(chosen)) {
    const bool sweetSnack = isSnackFood(chosen);
    hunger_ = clampAddHungryHappy(hunger_, 8);
    happiness_ = clampAddHungryHappy(happiness_, 16);
    hygiene_ = clampAdd(hygiene_, -4);
    weight_ = clampAdd16(weight_, 2, 999);
    snackScore_ = clampAdd(snackScore_, 5);
    if (sweetSnack && stageCanGetToothache(stage_) && !toothache_) {
      const uint16_t currentSweetMinute = petMinuteStamp(ageMinutes_);
      if (!sweetSnackWithinShortPeriod(lastSweetSnackMinute_,
                                       currentSweetMinute)) {
        sweetSnackStreak_ = 0;
      }
      lastSweetSnackMinute_ = currentSweetMinute;
      if (++sweetSnackStreak_ >= kSweetSnackToothacheStreak) {
        toothache_ = 1;
        sweetSnackStreak_ = 0;
        lastSweetSnackMinute_ = kSweetSnackNoLastMinute;
      }
    } else if (!sweetSnack) {
      sweetSnackStreak_ = 0;
      lastSweetSnackMinute_ = kSweetSnackNoLastMinute;
    }
    const SicknessTriggerRule& sweetSickRule =
        sicknessTriggerRuleFor(SicknessTriggerKind::kSweetSnack);
    if (!toothache_ && sweetSnack &&
        nextRand(100) <
            guardedSickChance(sweetSickRule.chancePercent,
                              (memoryFlags_ & kMemoryFlagTamaDrinkGuard) !=
                                  0) &&
        weight_ > sweetSickRule.threshold) {
      uint8_t severity = sweetSickRule.minSeverity;
      if (sweetSickRule.severityRollCount > 1) {
        severity += nextRand(sweetSickRule.severityRollCount);
      }
      becomeSick(severity);
    }
    notice_ = Notice::kSnack;
  } else {
    hunger_ = clampAddHungryHappy(hunger_, 25);
    happiness_ = clampAddHungryHappy(happiness_, 3);
    hygiene_ = clampAdd(hygiene_, -2);
    weight_ = clampAdd16(weight_, 1, 999);
    careScore_ = clampAdd(careScore_, 2);
    sweetSnackStreak_ = 0;
    lastSweetSnackMinute_ = kSweetSnackNoLastMinute;
    notice_ = Notice::kMeal;
  }
  return true;
}

FriendRecord* EchoPetModel::findFriend(uint32_t petId) {
  for (uint8_t i = 0; i < friendCount_; i++) {
    if (friends_[i].petId == petId) return &friends_[i];
  }
  return nullptr;
}

FriendRecord* EchoPetModel::addOrUpdateFriend(const FriendPacket& packet,
                                               bool* isNew) {
  FriendRecord* record = findFriend(packet.petId);
  if (record) {
    if (isNew) *isNew = false;
  } else {
    if (friendCount_ >= kMaxFriends) {
      uint8_t weakest = 0;
      uint16_t weakestScore = 0xFFFF;
      for (uint8_t i = 0; i < friendCount_; i++) {
        const uint16_t score =
            static_cast<uint16_t>(friends_[i].relation) * 32U +
            static_cast<uint16_t>(friends_[i].visits) +
            static_cast<uint16_t>(friends_[i].gifts) * 2U;
        if (score < weakestScore) {
          weakestScore = score;
          weakest = i;
        }
      }
      record = &friends_[weakest];
      if (lastFriendId_ == record->petId) lastFriendId_ = 0;
    } else {
      record = &friends_[friendCount_++];
    }
    memset(record, 0, sizeof(*record));
    record->petId = packet.petId;
    if (isNew) *isNew = true;
  }

  record->generation = packet.generation;
  record->character = packet.character;
  record->catalogId =
      packet.catalogId < kCharacterCatalogCount
          ? packet.catalogId
          : catalogIdForCharacterKind(static_cast<CharacterKind>(packet.character));
  record->visits = clampAdd(record->visits, 1);
  record->relation = static_cast<uint8_t>(relationForVisits(record->visits));
  return record;
}

RelationLevel EchoPetModel::relationForVisits(uint8_t visits) const {
  if (visits >= 24) return RelationLevel::kPartner;
  if (visits >= 16) return RelationLevel::kBestFriend;
  if (visits >= 10) return RelationLevel::kGoodFriend;
  if (visits >= 6) return RelationLevel::kFriend;
  if (visits >= 3) return RelationLevel::kBuddy;
  if (visits >= 1) return RelationLevel::kAcquaintance;
  return RelationLevel::kNone;
}

RelationLevel EchoPetModel::bestRelation() const {
  RelationLevel best = RelationLevel::kNone;
  for (uint8_t i = 0; i < friendCount_; i++) {
    if (friends_[i].relation > static_cast<uint8_t>(best)) {
      best = static_cast<RelationLevel>(friends_[i].relation);
    }
  }
  return best;
}

bool EchoPetModel::chooseGift(GiftKind& kind, uint8_t& id, uint8_t& amount) {
  for (uint8_t pass = 0; pass < 2; pass++) {
    const CatalogKind target =
        pass == 0 ? CatalogKind::kItem : CatalogKind::kFood;
    for (uint8_t i = 0; i < kCatalogItemCount; i++) {
      if (catalogEntry(i).kind != target) {
        continue;
      }
      if (debitCatalogGift(i, false)) {
        kind = GiftKind::kCatalogItem;
        id = i;
        amount = 1;
        return true;
      }
    }
  }

  if (points_ >= 50) {
    points_ -= 50;
    kind = GiftKind::kPoints;
    id = 0;
    amount = 5;
    return true;
  }
  kind = GiftKind::kNone;
  id = 0;
  amount = 0;
  return false;
}

bool EchoPetModel::ownsLinkGameItem(LinkGameKind game) const {
  return linkGameOwnedInBits(catalogOwned_, game);
}

GrowthRoute EchoPetModel::chooseRoute() const {
  return routeForGrowthState();
}

CharacterKind EchoPetModel::characterFor(Stage stage, GrowthRoute route) const {
  switch (stage) {
    case Stage::kEgg:
      return CharacterKind::kShell;
    case Stage::kBaby:
      return CharacterKind::kBaby;
    case Stage::kChild:
      return CharacterKind::kSprout;
    case Stage::kTeen:
    case Stage::kAdult:
    case Stage::kParentCare:
      switch (route) {
        case GrowthRoute::kAthlete:
          return CharacterKind::kBolt;
        case GrowthRoute::kScholar:
          return CharacterKind::kQuill;
        case GrowthRoute::kSocial:
          return CharacterKind::kBuddy;
        case GrowthRoute::kDreamer:
          return CharacterKind::kDream;
        case GrowthRoute::kRascal:
          return CharacterKind::kRascal;
        case GrowthRoute::kBalanced:
          return CharacterKind::kSprout;
      }
      break;
    case Stage::kElder:
      return CharacterKind::kSage;
  }
  return CharacterKind::kSprout;
}

uint8_t EchoPetModel::chooseCharacterCatalogId(Stage stage,
                                               GrowthRoute /*route*/) {
  if (stage == Stage::kAdult &&
      (growthFlags_ & kGrowthFlagOyajitchiLineage)) {
    for (uint8_t i = 0; i < kCharacterCatalogCount; i++) {
      if (strcmp(characterCatalogEntry(i).name, "Oyajitchi") == 0) {
        return i;
      }
    }
  }

  const Stage lookupStage =
      stage == Stage::kParentCare ? Stage::kAdult : stage;
  const uint8_t generationMask = characterGenerationMask(generation_);
  const uint8_t tierMask =
      characterTierMaskForStage(lookupStage, generation_, growthTier_,
                                adultTier_);
  const uint8_t genderMask = characterGenderMask(gender_);
  uint8_t count = 0;
  for (uint8_t i = 0; i < kCharacterCatalogCount; i++) {
    const CharacterCatalogEntry& entry = characterCatalogEntry(i);
    if (characterSourcePoolMatches(entry, lookupStage, generationMask,
                                   tierMask, genderMask)) {
      count++;
    }
  }

  if (count) {
    const uint8_t target = nextRand(count);
    uint8_t seen = 0;
    for (uint8_t i = 0; i < kCharacterCatalogCount; i++) {
      const CharacterCatalogEntry& entry = characterCatalogEntry(i);
      if (characterSourcePoolMatches(entry, lookupStage, generationMask,
                                     tierMask, genderMask)) {
        if (seen == target) return i;
        seen++;
      }
    }
  }

  count = 0;
  for (uint8_t i = 0; i < kCharacterCatalogCount; i++) {
    if (characterCatalogEntry(i).stage == lookupStage) count++;
  }
  if (!count) return 0;

  const uint8_t target = nextRand(count);
  uint8_t seen = 0;
  for (uint8_t i = 0; i < kCharacterCatalogCount; i++) {
    if (characterCatalogEntry(i).stage == lookupStage) {
      if (seen == target) return i;
      seen++;
    }
  }
  return 0;
}

uint8_t EchoPetModel::unlockedGameCount() const {
  for (const GameUnlockRule& rule : kGameUnlockRules) {
    if (rule.stage == stage_) return rule.count;
  }
  return 0;
}

uint8_t EchoPetModel::nextRand(uint8_t limit) {
  rngState_ = rngState_ * 1664525UL + 1013904223UL;
  if (limit == 0) return 0;
  return static_cast<uint8_t>((rngState_ >> 24) % limit);
}

bool EchoPetModel::isBedtime() const {
  const uint16_t minute = static_cast<uint16_t>(clockMinutes_ % 1440UL);
  const SleepWindowRule& window = sleepWindowForStage(stage_);
  return minute >= window.sleepStartMinute || minute < window.wakeMinute;
}

bool EchoPetModel::isAdultLike() const {
  return stage_ == Stage::kAdult || stage_ == Stage::kElder;
}

void EchoPetModel::makeDefaultNames() {
  snprintf(petName_, sizeof(petName_), "E%04lu",
           static_cast<unsigned long>(petId_ % 10000UL));
  snprintf(nickname_, sizeof(nickname_), "P%04u",
           static_cast<unsigned>(generation_ % 10000U));
}

const char* actionLabel(Action action) {
  switch (action) {
    case Action::kHealth:
      return "HLTH";
    case Action::kMeal:
      return "MEAL";
    case Action::kSnack:
      return "SNK";
    case Action::kGame:
      return "GAME";
    case Action::kShop:
      return "SHOP";
    case Action::kItem:
      return "ITEM";
    case Action::kToilet:
      return "TOIL";
    case Action::kMedicine:
      return "MED";
    case Action::kLights:
      return "LITE";
    case Action::kDiscipline:
      return "DISC";
    case Action::kPraise:
      return "PRA";
    case Action::kVisit:
      return "VISIT";
    case Action::kFriendList:
      return "FRND";
    case Action::kPresent:
      return "GIFT";
    case Action::kLinkGame:
      return "LINK";
    case Action::kFamily:
      return "FAM";
    case Action::kDonate:
      return "DON";
    case Action::kPassword:
      return "CODE";
    case Action::kSound:
      return "SND";
    case Action::kReset:
      return "RST";
    case Action::kCount:
      return "";
  }
  return "";
}

const char* stageLabel(Stage stage) {
  switch (stage) {
    case Stage::kEgg:
      return "EGG";
    case Stage::kBaby:
      return "BABY";
    case Stage::kChild:
      return "CHILD";
    case Stage::kTeen:
      return "TEEN";
    case Stage::kAdult:
      return "ADULT";
    case Stage::kParentCare:
      return "PARENT";
    case Stage::kElder:
      return "ELDER";
  }
  return "";
}

const char* routeLabel(GrowthRoute route) {
  switch (route) {
    case GrowthRoute::kBalanced:
      return "BAL";
    case GrowthRoute::kAthlete:
      return "RUN";
    case GrowthRoute::kScholar:
      return "WISE";
    case GrowthRoute::kSocial:
      return "PAL";
    case GrowthRoute::kDreamer:
      return "DREAM";
    case GrowthRoute::kRascal:
      return "WILD";
  }
  return "";
}

const char* characterLabel(CharacterKind character) {
  switch (character) {
    case CharacterKind::kShell:
      return "Shell";
    case CharacterKind::kBaby:
      return "Baby";
    case CharacterKind::kSprout:
      return "Sprout";
    case CharacterKind::kBolt:
      return "Bolt";
    case CharacterKind::kQuill:
      return "Quill";
    case CharacterKind::kBuddy:
      return "Buddy";
    case CharacterKind::kDream:
      return "Dream";
    case CharacterKind::kRascal:
      return "Rascal";
    case CharacterKind::kSage:
      return "Sage";
  }
  return "";
}

const char* foodLabel(FoodKind food) {
  switch (food) {
    case FoodKind::kScone:
      return "SCONE";
    case FoodKind::kSushi:
      return "SUSHI";
    case FoodKind::kBread:
      return "BREAD";
    case FoodKind::kCereal:
      return "CEREAL";
    case FoodKind::kCone:
      return "CONE";
    case FoodKind::kPudding:
      return "PUDDING";
    case FoodKind::kTart:
      return "TART";
    case FoodKind::kApple:
      return "APPLE";
    case FoodKind::kCount:
      return "";
  }
  return "";
}

const char* itemLabel(ItemKind item) {
  switch (item) {
    case ItemKind::kToy:
      return "TOY";
    case ItemKind::kBook:
      return "BOOK";
    case ItemKind::kCharm:
      return "CHARM";
    case ItemKind::kBall:
      return "BALL";
    case ItemKind::kBlocks:
      return "BLOCK";
    case ItemKind::kRope:
      return "ROPE";
    case ItemKind::kTrumpet:
      return "HORN";
    case ItemKind::kTicket:
      return "TICKET";
    case ItemKind::kCount:
      return "";
  }
  return "";
}

LinkGameKind linkGameFromIndex(uint8_t index) {
  switch (index % kLinkGameCount) {
    case 0:
      return LinkGameKind::kGotchiPoint;
    case 1:
      return LinkGameKind::kBall;
    case 2:
      return LinkGameKind::kRcCar;
    case 3:
      return LinkGameKind::kRope;
    case 4:
      return LinkGameKind::kBuildingBlock;
    case 5:
      return LinkGameKind::kBalloon;
    default:
      return LinkGameKind::kTrumpet;
  }
}

const char* linkGameLabel(LinkGameKind game) {
  switch (game) {
    case LinkGameKind::kGotchiPoint:
      return "G.POINT";
    case LinkGameKind::kBall:
      return "BALL";
    case LinkGameKind::kRcCar:
      return "RC CAR";
    case LinkGameKind::kRope:
      return "ROPE";
    case LinkGameKind::kBuildingBlock:
      return "BLDG";
    case LinkGameKind::kBalloon:
      return "BALLOON";
    case LinkGameKind::kTrumpet:
      return "TRUMPET";
    case LinkGameKind::kNone:
      break;
  }
  return "";
}

const char* linkGameRequiredLabel(LinkGameKind game) {
  switch (game) {
    case LinkGameKind::kGotchiPoint:
      return "READY";
    case LinkGameKind::kBall:
      return "NEED BALL";
    case LinkGameKind::kRcCar:
      return "NEED RC";
    case LinkGameKind::kRope:
      return "NEED ROPE";
    case LinkGameKind::kBuildingBlock:
      return "NEED BLOCK";
    case LinkGameKind::kBalloon:
      return "NEED BALN";
    case LinkGameKind::kTrumpet:
      return "NEED HORN";
    case LinkGameKind::kNone:
      break;
  }
  return "NO GAME";
}

bool linkGameAvailable(const uint8_t* catalogOwned, LinkGameKind game) {
  return linkGameOwnedInBits(catalogOwned, game);
}

const char* miniGameLabel(MiniGameKind game) {
  switch (game) {
    case MiniGameKind::kNone:
      return "";
    case MiniGameKind::kGet:
      return "GET";
    case MiniGameKind::kBump:
      return "BUMP";
    case MiniGameKind::kFlag:
      return "FLAG";
    case MiniGameKind::kHeading:
      return "HEAD";
    case MiniGameKind::kMemory:
      return "MEM";
    case MiniGameKind::kSprint:
      return "RUN";
    case MiniGameKind::kHoops:
      return "HOOP";
  }
  return "";
}

const char* gamePromptLabel(GamePrompt prompt) {
  switch (prompt) {
    case GamePrompt::kNone:
      return "";
    case GamePrompt::kCatch:
      return "CATCH";
    case GamePrompt::kDodge:
      return "DODGE";
    case GamePrompt::kBump:
      return "BUMP";
    case GamePrompt::kWait:
      return "WAIT";
    case GamePrompt::kLeft:
      return "LEFT";
    case GamePrompt::kRight:
      return "RIGHT";
    case GamePrompt::kBoth:
      return "BOTH";
    case GamePrompt::kHit:
      return "HIT";
    case GamePrompt::kCopy:
      return "COPY";
    case GamePrompt::kPress:
      return "PRESS";
    case GamePrompt::kPush:
      return "PUSH";
    case GamePrompt::kShoot:
      return "SHOOT";
  }
  return "";
}

const char* relationLabel(RelationLevel relation) {
  switch (relation) {
    case RelationLevel::kNone:
      return "NONE";
    case RelationLevel::kAcquaintance:
      return "HELLO";
    case RelationLevel::kBuddy:
      return "BUDDY";
    case RelationLevel::kFriend:
      return "FRIEND";
    case RelationLevel::kGoodFriend:
      return "GOOD";
    case RelationLevel::kBestFriend:
      return "BEST";
    case RelationLevel::kPartner:
      return "PARTNER";
  }
  return "";
}

const char* noticeLabel(Notice notice) {
  switch (notice) {
    case Notice::kNone:
      return "";
    case Notice::kBorn:
      return "A new friend hatched.";
    case Notice::kStatus:
      return "Status view.";
    case Notice::kMeal:
      return "Meal time.";
    case Notice::kSnack:
      return "A sweet treat.";
    case Notice::kFull:
      return "No more food.";
    case Notice::kGameStart:
      return "Mini game!";
    case Notice::kGameGood:
      return "Nice move.";
    case Notice::kGameMiss:
      return "Missed.";
    case Notice::kGameWin:
      return "Great game.";
    case Notice::kGameLose:
      return "Try again.";
    case Notice::kGameTired:
      return "Too tired to play.";
    case Notice::kPoints:
      return "Points gained.";
    case Notice::kBought:
      return "Bought item.";
    case Notice::kNoPoints:
      return "Need points.";
    case Notice::kInventoryFull:
      return "Inventory full.";
    case Notice::kItemUsed:
      return "Item used.";
    case Notice::kNoItem:
      return "No item.";
    case Notice::kPasswordOk:
      return "Code unlocked.";
    case Notice::kPasswordDone:
      return "Codes complete.";
    case Notice::kPasswordBad:
      return "Wrong password.";
    case Notice::kGift:
      return "Gift received.";
    case Notice::kFriendReady:
      return "Link packet ready.";
    case Notice::kFriendVisit:
      return "Friend visited.";
    case Notice::kFriendPresent:
      return "Present received.";
    case Notice::kFriendGame:
      return "Link game done.";
    case Notice::kFriendFull:
      return "Friend list full.";
    case Notice::kFriendDeleted:
      return "Friend deleted.";
    case Notice::kPartner:
      return "Partner bond.";
    case Notice::kLoveRejected:
      return "Love rejected.";
    case Notice::kBaby:
      return "A baby arrived.";
    case Notice::kParentLeft:
      return "Parent left.";
    case Notice::kMatchmaker:
      return "Matchmaker came.";
    case Notice::kFamily:
      return "Family view.";
    case Notice::kAnniversary:
      return "Calendar event.";
    case Notice::kBirthday:
      return "Birthday event.";
    case Notice::kSouvenir:
      return "Souvenir gained.";
    case Notice::kDonate:
      return "Donation sent.";
    case Notice::kClean:
      return "All clean.";
    case Notice::kNoMess:
      return "No cleanup needed.";
    case Notice::kMedicine:
      return "Feeling better.";
    case Notice::kNeedMoreMedicine:
      return "Needs more meds.";
    case Notice::kNoMedicine:
      return "No medicine needed.";
    case Notice::kSickRefuse:
      return "Too sick.";
    case Notice::kTrain:
      return "Time-out worked.";
    case Notice::kPraise:
      return "Praised.";
    case Notice::kWrongDiscipline:
      return "Wrong care.";
    case Notice::kLightsOn:
      return "Lights on.";
    case Notice::kLightsOff:
      return "Lights off.";
    case Notice::kSoundOn:
      return "Sound on.";
    case Notice::kSoundOff:
      return "Sound off.";
    case Notice::kSleeping:
      return "Sleeping now.";
    case Notice::kNeedsCare:
      return "Needs care.";
    case Notice::kGrew:
      return "It grew.";
    case Notice::kPassed:
      return "A memory remains.";
    case Notice::kResetReady:
      return "Reset only after pass.";
  }
  return "";
}

uint32_t checksumSaveBytes(const PetSave& save, size_t size) {
  if (size > sizeof(PetSave)) {
    size = sizeof(PetSave);
  }
  PetSave copy = save;
  copy.checksum = 0;
  const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&copy);
  uint32_t hash = 2166136261UL;
  for (size_t i = 0; i < size; i++) {
    hash ^= bytes[i];
    hash *= 16777619UL;
  }
  return hash;
}

uint32_t checksumSave(const PetSave& save) {
  return checksumSaveBytes(save, sizeof(PetSave));
}

uint16_t checksumFriendPacket(const FriendPacket& packet) {
  FriendPacket copy = packet;
  copy.checksum = 0;
  const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&copy);
  uint16_t hash = 0xBEEF;
  for (size_t i = 0; i < sizeof(FriendPacket); i++) {
    hash ^= bytes[i];
    hash = static_cast<uint16_t>((hash << 5) | (hash >> 11));
    hash = static_cast<uint16_t>(hash + 0x31);
  }
  return hash;
}

bool validateFriendPacket(const FriendPacket& packet) {
  return packet.magic == kFriendMagic && packet.version == kFriendVersion &&
         validLinkKind(packet.kind) && validStage(packet.stage) &&
         validCharacter(packet.character) && validRelation(packet.relation) &&
         validGrowthTier(packet.growthTier) &&
         validAdultTier(packet.adultTier) && validGender(packet.gender) &&
         (packet.kind != static_cast<uint8_t>(LinkKind::kGame) ||
          validLinkGameKind(packet.gameKind)) &&
         packet.catalogId < kCharacterCatalogCount &&
         checksumFriendPacket(packet) == packet.checksum;
}

}  // namespace echopet
