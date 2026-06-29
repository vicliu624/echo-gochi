#pragma once

#include <stdint.h>

#include "EchoPetCatalog.h"

#ifndef ECHOPET_MINUTE_MS
#define ECHOPET_MINUTE_MS 60000UL
#endif

namespace echopet {

constexpr uint8_t kNameChars = 5;
constexpr uint8_t kDefaultMealCount = 4;
constexpr uint8_t kDefaultSnackCount = 4;
constexpr uint8_t kFoodKindCount = kDefaultMealCount + kDefaultSnackCount;
constexpr uint8_t kItemKindCount = 8;
constexpr uint8_t kSouvenirCount = 16;
constexpr uint8_t kMaxFriends = 45;
constexpr uint8_t kFamilyHistoryCount = 4;
constexpr uint8_t kCatalogIdUnknown = 0xFF;
constexpr uint8_t kFamilyRecordPartnerCatalogValid = 0x80;
constexpr uint8_t kFamilyRecordRouteMask = 0x7F;
constexpr uint8_t kFamilyAncestryValid = 0x01;
constexpr uint8_t kFamilyAncestryPartnerCatalogValid = 0x02;
constexpr uint8_t kFamilyAncestryOyajitchiLineage = 0x04;
constexpr uint8_t kMemoryFlagTamaDrinkGuard = 0x04;
constexpr uint8_t kMemoryFlagFriendSeen = 0x08;
constexpr uint8_t kMemoryFlagSouvenirSeen = 0x10;
constexpr uint8_t kMemoryFlagHoneyPending = 0x20;
constexpr uint8_t kMemoryFlagNyatchiCostume = 0x40;
constexpr uint8_t kMemoryFlagHohotchiCostume = 0x80;
constexpr uint8_t kMemoryFlagCostumeMask =
    kMemoryFlagNyatchiCostume | kMemoryFlagHohotchiCostume;

enum class Stage : uint8_t {
  kEgg,
  kBaby,
  kChild,
  kTeen,
  kAdult,
  kParentCare,
  kElder,
};

enum class LifeState : uint8_t {
  kAlive,
  kPassed,
};

enum class Gender : uint8_t {
  kBoy,
  kGirl,
};

enum class Mood : uint8_t {
  kGreat,
  kOkay,
  kSad,
  kSick,
  kAsleep,
  kPassed,
};

enum class GrowthRoute : uint8_t {
  kBalanced,
  kAthlete,
  kScholar,
  kSocial,
  kDreamer,
  kRascal,
};

enum class CareMistakeClass : uint8_t {
  kPhysical,
  kMental,
};

enum class GrowthTier : uint8_t {
  kFirstGeneration,
  kTierOne,
  kTierTwo,
  kTierThree,
  kTierFour,
};

enum class AdultTier : uint8_t {
  kSerious,
  kNormal,
  kNaughty,
  kFrail,
  kSpecial,
};

enum class CharacterKind : uint8_t {
  kShell,
  kBaby,
  kSprout,
  kBolt,
  kQuill,
  kBuddy,
  kDream,
  kRascal,
  kSage,
};

enum class Action : uint8_t {
  kHealth,
  kMeal,
  kSnack,
  kGame,
  kShop,
  kItem,
  kToilet,
  kMedicine,
  kLights,
  kDiscipline,
  kPraise,
  kVisit,
  kFriendList,
  kPresent,
  kLinkGame,
  kFamily,
  kDonate,
  kPassword,
  kSound,
  kReset,
  kCount,
};

enum class MiniGameKind : uint8_t {
  kNone,
  kGet,
  kBump,
  kFlag,
  kHeading,
  kMemory,
  kSprint,
  kHoops,
};

enum class LinkGameKind : uint8_t {
  kNone,
  kGotchiPoint,
  kBall,
  kRcCar,
  kRope,
  kBuildingBlock,
  kBalloon,
  kTrumpet,
};

constexpr uint8_t kLinkGameCount = 7;

enum class GameButton : uint8_t {
  kA,
  kB,
  kC,
  kAC,
};

enum class GamePrompt : uint8_t {
  kNone,
  kCatch,
  kDodge,
  kBump,
  kWait,
  kLeft,
  kRight,
  kBoth,
  kHit,
  kCopy,
  kPress,
  kPush,
  kShoot,
};

enum class FoodKind : uint8_t {
  kScone,
  kSushi,
  kBread,
  kCereal,
  kCone,
  kPudding,
  kTart,
  kApple,
  kCount,
};

enum class ItemKind : uint8_t {
  kToy,
  kBook,
  kCharm,
  kBall,
  kBlocks,
  kRope,
  kTrumpet,
  kTicket,
  kCount,
};

enum class LinkKind : uint8_t {
  kVisit,
  kPresent,
  kGame,
  kLove,
};

enum class GiftKind : uint8_t {
  kNone,
  kFood,
  kItem,
  kPoints,
  kCatalogItem,
};

enum class RelationLevel : uint8_t {
  kNone,
  kAcquaintance,
  kBuddy,
  kFriend,
  kGoodFriend,
  kBestFriend,
  kPartner,
};

enum class AttentionReason : uint8_t {
  kNone,
  kHungry,
  kSad,
  kDirty,
  kSick,
  kNaughty,
  kPraise,
};

enum class Notice : uint8_t {
  kNone,
  kBorn,
  kStatus,
  kMeal,
  kSnack,
  kFull,
  kGameStart,
  kGameGood,
  kGameMiss,
  kGameWin,
  kGameLose,
  kGameTired,
  kPoints,
  kBought,
  kNoPoints,
  kInventoryFull,
  kItemUsed,
  kNoItem,
  kPasswordOk,
  kPasswordDone,
  kPasswordBad,
  kGift,
  kFriendReady,
  kFriendVisit,
  kFriendPresent,
  kFriendGame,
  kFriendFull,
  kFriendDeleted,
  kPartner,
  kLoveRejected,
  kBaby,
  kParentLeft,
  kMatchmaker,
  kFamily,
  kAnniversary,
  kBirthday,
  kSouvenir,
  kDonate,
  kClean,
  kNoMess,
  kMedicine,
  kNeedMoreMedicine,
  kNoMedicine,
  kSickRefuse,
  kTrain,
  kPraise,
  kWrongDiscipline,
  kLightsOn,
  kLightsOff,
  kSoundOn,
  kSoundOff,
  kSleeping,
  kNeedsCare,
  kGrew,
  kPassed,
  kResetReady,
};

#pragma pack(push, 1)
struct FriendRecord {
  uint32_t petId;
  uint16_t generation;
  uint8_t character;
  uint8_t relation;
  uint8_t visits;
  uint8_t gifts;
  uint8_t catalogId;
};

struct FamilyRecord {
  uint32_t petId;
  uint16_t generation;
  // When route has kFamilyRecordPartnerCatalogValid, character stores the
  // partner catalog id; otherwise it stores the legacy parent archetype.
  uint8_t character;
  uint8_t catalogId;
  uint8_t route;
  uint8_t careScore;
};

struct FamilyAncestryRecord {
  uint8_t flags;
  uint8_t parentCatalogIdA;
  uint8_t parentCatalogIdB;
  uint8_t babyCatalogId;
  uint8_t parentAdultTierA;
  uint8_t parentAdultTierB;
  uint8_t childGrowthTier;
};
#pragma pack(pop)

struct Snapshot {
  uint32_t ageMinutes;
  uint32_t clockMinutes;
  uint32_t petId;
  uint32_t lastFriendId;
  uint16_t generation;
  uint16_t weight;
  uint16_t careMistakes;
  uint16_t points;
  uint16_t donations;
  uint16_t souvenirMask;
  uint8_t catalogOwned[kCatalogItemBitBytes];
  uint8_t catalogStock[kCatalogItemCount];
  uint8_t souvenirOwned[kCatalogSouvenirBitBytes];
  uint8_t catalogOwnedCount;
  uint8_t souvenirOwnedCount;
  uint8_t careScore;
  uint8_t playScore;
  uint8_t socialScore;
  uint8_t disciplineScore;
  uint8_t snackScore;
  uint8_t physicalCareMistakes;
  uint8_t mentalCareMistakes;
  uint8_t growthTier;
  uint8_t adultTier;
  uint8_t parentAdultTierA;
  uint8_t parentAdultTierB;
  uint8_t bornFromUnhealthyParents;
  uint8_t growthFlags;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t birthdayMonth;
  uint8_t birthdayDay;
  uint8_t hunger;
  uint8_t happiness;
  uint8_t energy;
  uint8_t hygiene;
  uint8_t discipline;
  uint8_t friendship;
  uint8_t messCount;
  uint8_t sickness;
  uint8_t toothache;
  uint8_t attention;
  uint8_t lightsOff;
  uint8_t soundOn;
  uint8_t foodCounts[kFoodKindCount];
  uint8_t itemCounts[kItemKindCount];
  uint8_t gameWins;
  uint8_t gamePlays;
  uint8_t gameStreak;
  uint8_t bestGameScore;
  uint8_t friendCount;
  uint8_t friendVisits;
  uint8_t memoryFlags;
  uint8_t gameRound;
  uint8_t gameScore;
  uint8_t gameRoundLimit;
  uint8_t gameScoreLimit;
  uint8_t gameExpectedYes;
  uint8_t gameCursor;
  uint8_t gameTarget;
  uint32_t gameHazard;
  uint8_t unlockedGames;
  char petName[kNameChars + 1];
  char nickname[kNameChars + 1];
  Gender gender;
  LifeState lifeState;
  GrowthRoute route;
  CharacterKind character;
  uint8_t characterCatalogId;
  Stage stage;
  Mood mood;
  MiniGameKind miniGame;
  GamePrompt gamePrompt;
  LinkKind lastLinkKind;
  RelationLevel bestRelation;
  AttentionReason attentionReason;
  Notice notice;
  FriendRecord friends[kMaxFriends];
  FamilyRecord family[kFamilyHistoryCount];
  FamilyAncestryRecord familyAncestry[kFamilyHistoryCount];
};

#pragma pack(push, 1)
struct FriendPacket {
  uint16_t magic;
  uint8_t version;
  uint8_t kind;
  uint32_t petId;
  uint16_t generation;
  uint16_t ageDays;
  uint8_t stage;
  uint8_t character;
  uint8_t friendship;
  uint8_t relation;
  uint8_t sequence;
  uint8_t giftKind;
  uint8_t giftId;
  uint8_t giftAmount;
  uint8_t gameKind;
  uint8_t gameScore;
  uint8_t growthTier;
  uint8_t adultTier;
  uint8_t gender;
  uint8_t catalogId;
  uint16_t checksum;
};

struct PetSave {
  uint32_t magic;
  uint16_t version;
  uint16_t size;
  uint32_t checksum;
  uint32_t ageMinutes;
  uint32_t clockMinutes;
  uint32_t rngState;
  uint32_t petId;
  uint32_t lastFriendId;
  uint16_t generation;
  uint16_t parentCareMinutes;
  uint16_t lastAnniversaryDay;
  uint16_t weight;
  uint16_t careMistakes;
  uint16_t points;
  uint16_t donations;
  uint16_t souvenirMask;
  uint8_t catalogOwned[kCatalogItemBitBytes];
  uint8_t souvenirOwned[kCatalogSouvenirBitBytes];
  uint8_t month;
  uint8_t day;
  uint8_t birthdayMonth;
  uint8_t birthdayDay;
  uint8_t hunger;
  uint8_t happiness;
  uint8_t energy;
  uint8_t hygiene;
  uint8_t discipline;
  uint8_t friendship;
  uint8_t messCount;
  uint8_t sickness;
  uint8_t toothache;
  uint8_t attentionMinutes;
  uint8_t attention;
  uint8_t attentionReason;
  uint8_t lightsOff;
  uint8_t soundOn;
  uint8_t stage;
  uint8_t lifeState;
  uint8_t gender;
  uint8_t route;
  uint8_t character;
  uint8_t characterCatalogId;
  uint8_t foodCounts[kFoodKindCount];
  uint8_t itemCounts[kItemKindCount];
  uint8_t gameWins;
  uint8_t gamePlays;
  uint8_t gameStreak;
  uint8_t bestGameScore;
  uint8_t friendVisits;
  uint8_t memoryFlags;
  uint8_t careScore;
  uint8_t playScore;
  uint8_t socialScore;
  uint8_t disciplineScore;
  uint8_t snackScore;
  uint8_t lastGameKind;
  uint8_t linkSequence;
  uint8_t friendCount;
  char petName[kNameChars + 1];
  char nickname[kNameChars + 1];
  FriendRecord friends[kMaxFriends];
  FamilyRecord family[kFamilyHistoryCount];
  uint8_t physicalCareMistakes;
  uint8_t mentalCareMistakes;
  uint8_t growthTier;
  uint8_t adultTier;
  uint8_t parentAdultTierA;
  uint8_t parentAdultTierB;
  uint8_t bornFromUnhealthyParents;
  uint8_t growthFlags;
  uint8_t sweetSnackStreak;
  uint8_t parentCatalogIdA;
  uint8_t parentCatalogIdB;
  uint16_t lastSweetSnackMinute;
  uint8_t stageSicknesses;
  uint8_t catalogStock[kCatalogItemCount];
  FamilyAncestryRecord familyAncestry[kFamilyHistoryCount];
};
#pragma pack(pop)

class EchoPetModel {
 public:
  void begin(uint32_t nowMs);
  void reset(uint32_t seed, uint32_t nowMs);
  void configureSetup(uint8_t month, uint8_t day, uint8_t hour, uint8_t minute,
                      uint8_t birthdayMonth, uint8_t birthdayDay,
                      const char* userName, const char* petName,
                      Gender gender);
  bool setClock(uint8_t hour, uint8_t minute);
  bool load(const PetSave& save, uint32_t nowMs);
  void exportSave(PetSave& save) const;

  bool tick(uint32_t nowMs);
  bool apply(Action action);
  bool feed(FoodKind food);
  bool snack(FoodKind food);
  bool useItem(ItemKind item);
  bool useCatalogItem(uint8_t index);
  bool buyShopSlot(uint8_t slot);
  bool enterPassword(const uint8_t digits[10]);
  bool enterSecretCode(const uint8_t symbols[8]);
  bool startGame(MiniGameKind game);
  bool isGameActive() const;
  bool gameInput(GameButton button, uint32_t nowMs);
  bool prepareFriendPacket(LinkKind kind, FriendPacket& packet,
                           bool updateNotice = true);
  bool prepareLinkReplyPacket(LinkKind kind, FriendPacket& packet);
  bool prepareGiftPacket(GiftKind kind, uint8_t id, uint8_t amount,
                         FriendPacket& packet, bool updateNotice = true);
  bool prepareCatalogGiftPacket(uint8_t index, FriendPacket& packet,
                                bool updateNotice = true);
  bool prepareLinkGamePacket(LinkGameKind game, FriendPacket& packet,
                             bool updateNotice = true);
  bool receiveFriendPacket(const FriendPacket& packet);
  bool deleteFriend(uint8_t index);
  Snapshot snapshot() const;
  void clearNotice();

 private:
  void resetPetBody(uint32_t seed, bool preserveSocial);
  void tickOnePetMinute();
  void advanceClockOneMinute();
  void updateGrowth();
  void updateMood();
  void recordCareMistake(CareMistakeClass type);
  bool reverseCareMistake(CareMistakeClass type);
  void clearAttentionFor(Action action);
  void resetStageCareMistakes();
  void applyGrowthTransition(Stage previous, Stage next);
  GrowthRoute routeForGrowthState() const;
  void callForCare();
  void checkCalendarEvents();
  void checkMatchmaker();
  void checkDeath();
  void becomeSick(uint8_t severity);
  void startBaby(AdultTier partnerAdultTier = AdultTier::kNormal,
                 bool oyajitchiLineage = false,
                 uint8_t partnerCatalogId = kCatalogIdUnknown);
  void parentLeaves();
  void addFamilyRecord(uint8_t babyCatalogId, GrowthTier childGrowthTier,
                       bool oyajitchiLineage);
  void clampStats();
  void startMiniGame(MiniGameKind requested);
  void setupGameRound();
  void completeMiniGameRound(bool correct, uint8_t gain, uint32_t nowMs);
  bool tickMiniGame(uint32_t nowMs);
  void finishMiniGame();
  bool buyShopStock();
  bool buyCatalogItem(uint8_t index);
  void awardFood(FoodKind food, uint8_t amount);
  void awardItem(ItemKind item, uint8_t amount);
  void awardCatalogItem(uint8_t index);
  void awardSouvenir(uint8_t index);
  bool catalogGiftAvailable(uint8_t index) const;
  bool debitCatalogGift(uint8_t index, bool updateNotice);
  bool receiveCatalogGift(uint8_t index, bool& inventoryFull);
  bool consumeCatalogItem(uint8_t index, const CatalogEntry& entry);
  void clearCatalogStockIfEmpty(uint8_t index, const CatalogEntry& entry);
  void rebuildCatalogStockFromOwned();
  uint16_t catalogStockTotal(CatalogKind kind) const;
  bool useBestItem();
  bool useSpecificItem(ItemKind item, bool consume = true);
  bool useFood(FoodKind food, bool snack, bool consumeInventory = false);
  void applyCatalogFoodTaste(uint8_t catalogIndex);
  FriendRecord* findFriend(uint32_t petId);
  FriendRecord* addOrUpdateFriend(const FriendPacket& packet, bool* isNew);
  RelationLevel relationForVisits(uint8_t visits) const;
  RelationLevel bestRelation() const;
  bool chooseGift(GiftKind& kind, uint8_t& id, uint8_t& amount);
  bool ownsLinkGameItem(LinkGameKind game) const;
  GrowthRoute chooseRoute() const;
  CharacterKind characterFor(Stage stage, GrowthRoute route) const;
  uint8_t chooseCharacterCatalogId(Stage stage, GrowthRoute route);
  uint8_t unlockedGameCount() const;
  uint8_t nextRand(uint8_t limit);
  bool isBedtime() const;
  bool isAdultLike() const;
  void makeDefaultNames();
  void fillFriendPacket(LinkKind kind, FriendPacket& packet);

  uint32_t ageMinutes_ = 0;
  uint32_t clockMinutes_ = 9 * 60;
  uint32_t rngState_ = 0xC0FFEE12UL;
  uint32_t petId_ = 0;
  uint32_t lastFriendId_ = 0;
  uint32_t lastMs_ = 0;
  uint32_t minuteRemainderMs_ = 0;

  uint16_t generation_ = 1;
  uint16_t parentCareMinutes_ = 0;
  uint16_t lastAnniversaryDay_ = 0xFFFF;
  uint16_t weight_ = 5;
  uint16_t careMistakes_ = 0;
  uint16_t points_ = 300;
  uint16_t donations_ = 0;
  uint16_t souvenirMask_ = 0;
  uint8_t catalogOwned_[kCatalogItemBitBytes] = {};
  uint8_t catalogStock_[kCatalogItemCount] = {};
  uint8_t souvenirOwned_[kCatalogSouvenirBitBytes] = {};
  uint8_t month_ = 1;
  uint8_t day_ = 1;
  uint8_t birthdayMonth_ = 6;
  uint8_t birthdayDay_ = 16;
  uint8_t hunger_ = 78;
  uint8_t happiness_ = 72;
  uint8_t energy_ = 80;
  uint8_t hygiene_ = 90;
  uint8_t discipline_ = 12;
  uint8_t friendship_ = 20;
  uint8_t messCount_ = 0;
  uint8_t sickness_ = 0;
  uint8_t toothache_ = 0;
  uint8_t stageSicknesses_ = 0;
  uint8_t attention_ = 0;
  uint8_t attentionMinutes_ = 0;
  AttentionReason attentionReason_ = AttentionReason::kNone;
  uint8_t lightsOff_ = 0;
  uint8_t soundOn_ = 1;
  uint8_t foodCounts_[kFoodKindCount] = {};
  uint8_t itemCounts_[kItemKindCount] = {0, 0, 0, 0, 0, 0, 0, 0};
  uint8_t gameWins_ = 0;
  uint8_t gamePlays_ = 0;
  uint8_t gameStreak_ = 0;
  uint8_t bestGameScore_ = 0;
  uint8_t friendVisits_ = 0;
  uint8_t memoryFlags_ = 0;
  uint8_t careScore_ = 0;
  uint8_t playScore_ = 0;
  uint8_t socialScore_ = 0;
  uint8_t disciplineScore_ = 0;
  uint8_t snackScore_ = 0;
  uint8_t sweetSnackStreak_ = 0;
  uint16_t lastSweetSnackMinute_ = 0xFFFF;
  uint8_t physicalCareMistakes_ = 0;
  uint8_t mentalCareMistakes_ = 0;
  GrowthTier growthTier_ = GrowthTier::kFirstGeneration;
  AdultTier adultTier_ = AdultTier::kNormal;
  AdultTier parentAdultTierA_ = AdultTier::kNormal;
  AdultTier parentAdultTierB_ = AdultTier::kNormal;
  uint8_t parentCatalogIdA_ = kCatalogIdUnknown;
  uint8_t parentCatalogIdB_ = kCatalogIdUnknown;
  uint8_t bornFromUnhealthyParents_ = 0;
  uint8_t growthFlags_ = 0;
  uint8_t lastGameKind_ = 0;
  uint8_t linkSequence_ = 0;
  uint8_t friendCount_ = 0;
  uint8_t gameRound_ = 0;
  uint8_t gameScore_ = 0;
  uint8_t gameExpectedYes_ = 0;
  uint8_t gameCursor_ = 1;
  uint8_t gameTarget_ = 1;
  uint32_t gameHazard_ = 0;
  uint32_t gameRoundStartMs_ = 0;
  char petName_[kNameChars + 1] = {'E', 'C', 'H', 'O', '1', '\0'};
  char nickname_[kNameChars + 1] = {'P', 'A', 'L', '0', '1', '\0'};
  FriendRecord friends_[kMaxFriends] = {};
  FamilyRecord family_[kFamilyHistoryCount] = {};
  FamilyAncestryRecord familyAncestry_[kFamilyHistoryCount] = {};
  Gender gender_ = Gender::kBoy;
  LifeState lifeState_ = LifeState::kAlive;
  GrowthRoute route_ = GrowthRoute::kBalanced;
  CharacterKind character_ = CharacterKind::kShell;
  uint8_t characterCatalogId_ = 0;
  Stage stage_ = Stage::kEgg;
  Mood mood_ = Mood::kOkay;
  MiniGameKind miniGame_ = MiniGameKind::kNone;
  GamePrompt gamePrompt_ = GamePrompt::kNone;
  LinkKind lastLinkKind_ = LinkKind::kVisit;
  Notice notice_ = Notice::kBorn;
};

const char* actionLabel(Action action);
const char* stageLabel(Stage stage);
const char* routeLabel(GrowthRoute route);
const char* characterLabel(CharacterKind character);
const char* foodLabel(FoodKind food);
const char* itemLabel(ItemKind item);
const char* miniGameLabel(MiniGameKind game);
LinkGameKind linkGameFromIndex(uint8_t index);
const char* linkGameLabel(LinkGameKind game);
const char* linkGameRequiredLabel(LinkGameKind game);
bool linkGameAvailable(const uint8_t* catalogOwned, LinkGameKind game);
const char* gamePromptLabel(GamePrompt prompt);
const char* relationLabel(RelationLevel relation);
const char* noticeLabel(Notice notice);
uint32_t checksumSave(const PetSave& save);
uint16_t checksumFriendPacket(const FriendPacket& packet);
bool validateFriendPacket(const FriendPacket& packet);

}  // namespace echopet
