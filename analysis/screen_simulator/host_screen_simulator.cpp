#include "EchoPetDisplay.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace {

using echopet::Action;
using echopet::AttentionReason;
using echopet::CharacterKind;
using echopet::FoodKind;
using echopet::GamePrompt;
using echopet::Gender;
using echopet::GrowthRoute;
using echopet::ItemKind;
using echopet::LifeState;
using echopet::LinkKind;
using echopet::MiniGameKind;
using echopet::Mood;
using echopet::Notice;
using echopet::RelationLevel;
using echopet::Snapshot;
using echopet::Stage;
using echopet::UiMode;
using echopet::UiState;

struct Profile {
  const char* name;
  int16_t width;
  int16_t height;
};

struct Scenario {
  const char* name;
  UiMode mode;
  Action selectedAction;
  int8_t selectedMenuIndex;
  uint8_t cursor;
  uint8_t page;
  void (*tweak)(Snapshot&);
};

void noTweak(Snapshot&) {}

void gameGetTweak(Snapshot& pet) {
  pet.miniGame = MiniGameKind::kGet;
  pet.gamePrompt = GamePrompt::kCatch;
  pet.gameRound = 3;
  pet.gameScore = 2;
  pet.gameRoundLimit = 8;
  pet.gameScoreLimit = 8;
  pet.gameCursor = 1;
  pet.gameTarget = 2;
  pet.notice = Notice::kGameGood;
}

void gameFlagTweak(Snapshot& pet) {
  pet.miniGame = MiniGameKind::kFlag;
  pet.gamePrompt = GamePrompt::kLeft;
  pet.gameRound = 5;
  pet.gameScore = 4;
  pet.gameRoundLimit = 8;
  pet.gameScoreLimit = 8;
  pet.gameCursor = 0;
  pet.gameTarget = 1;
  pet.notice = Notice::kGameStart;
}

void dirtyTweak(Snapshot& pet) {
  pet.messCount = 2;
  pet.hygiene = 36;
  pet.notice = Notice::kClean;
}

void sickTweak(Snapshot& pet) {
  pet.sickness = 1;
  pet.toothache = 1;
  pet.mood = Mood::kSick;
  pet.notice = Notice::kMedicine;
}

void lightsTweak(Snapshot& pet) {
  pet.lightsOff = 1;
  pet.mood = Mood::kAsleep;
  pet.notice = Notice::kLightsOff;
}

void careTweak(Snapshot& pet) {
  pet.attention = 1;
  pet.attentionReason = AttentionReason::kHungry;
  pet.notice = Notice::kNeedsCare;
}

void relationTweak(Snapshot& pet) {
  pet.lastLinkKind = LinkKind::kVisit;
  pet.bestRelation = RelationLevel::kGoodFriend;
  pet.notice = Notice::kFriendVisit;
}

void setName(char (&target)[echopet::kNameChars + 1], const char* value) {
  std::memset(target, 0, echopet::kNameChars + 1);
  std::strncpy(target, value, echopet::kNameChars);
}

Snapshot seedSnapshot() {
  Snapshot pet{};
  pet.ageMinutes = 42UL * 60UL + 18UL;
  pet.clockMinutes = 10UL * 60UL + 24UL;
  pet.petId = 0xEC0F2026UL;
  pet.lastFriendId = 0x1206ABCDUL;
  pet.generation = 2;
  pet.weight = 32;
  pet.careMistakes = 1;
  pet.points = 987;
  pet.donations = 250;
  pet.careScore = 82;
  pet.playScore = 74;
  pet.socialScore = 66;
  pet.disciplineScore = 71;
  pet.snackScore = 24;
  pet.physicalCareMistakes = 1;
  pet.mentalCareMistakes = 0;
  pet.growthTier = 2;
  pet.adultTier = 1;
  pet.month = 6;
  pet.day = 20;
  pet.hour = 10;
  pet.minute = 24;
  pet.birthdayMonth = 6;
  pet.birthdayDay = 20;
  pet.hunger = 86;
  pet.happiness = 78;
  pet.energy = 84;
  pet.hygiene = 72;
  pet.discipline = 67;
  pet.friendship = 58;
  pet.messCount = 1;
  pet.soundOn = 1;
  pet.unlockedGames = 0x7F;
  pet.gameWins = 9;
  pet.gamePlays = 15;
  pet.gameStreak = 3;
  pet.bestGameScore = 7;
  pet.friendCount = 3;
  pet.friendVisits = 6;
  pet.gameHazard = 0x2A55U;
  setName(pet.petName, "MAME");
  setName(pet.nickname, "ECHO");
  pet.gender = Gender::kBoy;
  pet.lifeState = LifeState::kAlive;
  pet.route = GrowthRoute::kBalanced;
  pet.character = CharacterKind::kBuddy;
  pet.characterCatalogId = 6;
  pet.stage = Stage::kAdult;
  pet.mood = Mood::kGreat;
  pet.miniGame = MiniGameKind::kNone;
  pet.gamePrompt = GamePrompt::kNone;
  pet.lastLinkKind = LinkKind::kVisit;
  pet.bestRelation = RelationLevel::kFriend;
  pet.attentionReason = AttentionReason::kNone;
  pet.notice = Notice::kStatus;

  for (uint8_t i = 0; i < echopet::kFoodKindCount; ++i) {
    pet.foodCounts[i] = static_cast<uint8_t>(1 + (i % 3));
  }
  for (uint8_t i = 0; i < echopet::kItemKindCount; ++i) {
    pet.itemCounts[i] = static_cast<uint8_t>(i % 2);
  }
  for (uint8_t i = 0; i < 24; ++i) {
    echopet::catalogBitSet(pet.catalogOwned, i);
    pet.catalogStock[i] = static_cast<uint8_t>(1 + (i % 4));
  }
  for (uint8_t i = 0; i < 10; ++i) {
    echopet::catalogBitSet(pet.souvenirOwned, i);
  }
  pet.catalogOwnedCount =
      echopet::catalogBitCount(pet.catalogOwned, echopet::kCatalogItemCount);
  pet.souvenirOwnedCount = echopet::catalogBitCount(
      pet.souvenirOwned, echopet::kCatalogSouvenirCount);

  pet.friends[0] = {0x01020304UL, 1, static_cast<uint8_t>(CharacterKind::kSage),
                    static_cast<uint8_t>(RelationLevel::kFriend), 4, 2, 8};
  pet.friends[1] = {0x12131415UL, 3,
                    static_cast<uint8_t>(CharacterKind::kDream),
                    static_cast<uint8_t>(RelationLevel::kGoodFriend), 8, 5,
                    9};
  pet.friends[2] = {0x22232425UL, 4,
                    static_cast<uint8_t>(CharacterKind::kRascal),
                    static_cast<uint8_t>(RelationLevel::kBestFriend), 12, 6,
                    10};

  pet.family[0] = {0xABC00001UL, 1, static_cast<uint8_t>(CharacterKind::kBuddy),
                   6, static_cast<uint8_t>(GrowthRoute::kBalanced), 82};
  pet.family[1] = {0xABC00002UL, 1, static_cast<uint8_t>(CharacterKind::kDream),
                   7, static_cast<uint8_t>(GrowthRoute::kDreamer), 74};
  return pet;
}

bool matches(const std::vector<std::string>& filters, const char* name) {
  if (filters.empty()) return true;
  for (const std::string& filter : filters) {
    if (filter == name) return true;
  }
  return false;
}

std::string joinPath(const std::string& dir, const std::string& leaf) {
  if (dir.empty()) return leaf;
  const char last = dir[dir.size() - 1];
  if (last == '/' || last == '\\') return dir + leaf;
  return dir + "/" + leaf;
}

}  // namespace

int main(int argc, char** argv) {
  std::string outDir = "analysis/screen_simulator/out";
  std::string profileFilter = "all";
  std::vector<std::string> scenarioFilters;
  int scale = 4;
  int phase = 2;
  bool phaseExplicit = false;

  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--out" && i + 1 < argc) {
      outDir = argv[++i];
    } else if (arg == "--profile" && i + 1 < argc) {
      profileFilter = argv[++i];
    } else if (arg == "--scenario" && i + 1 < argc) {
      scenarioFilters.push_back(argv[++i]);
    } else if (arg == "--scale" && i + 1 < argc) {
      scale = std::atoi(argv[++i]);
    } else if (arg == "--phase" && i + 1 < argc) {
      phase = std::atoi(argv[++i]);
      phaseExplicit = true;
    } else if (arg == "--list") {
      std::printf("profiles: large compact\n");
      std::printf("scenarios: home health food_menu meal activity_menu "
                  "connection_menu care discipline_menu shop item game_get "
                  "game_flag friends family souvenirs toilet medicine lights "
                  "sprite_proof setup catalog_items "
                  "catalog_souvenirs character_roster fixed_icons "
                  "care_toilet\n");
      return 0;
    }
  }

  if (scale < 1) scale = 1;

  const Profile profiles[] = {
      {"large", 192, 176},
      {"compact", 128, 64},
  };
  const Scenario scenarios[] = {
      {"home", UiMode::kHome, Action::kHealth, -1, 0, 0, noTweak},
      {"health", UiMode::kHealth, Action::kHealth, 0, 0, 0, noTweak},
      {"food_menu", UiMode::kFoodMenu, Action::kMeal, 1, 1, 0, noTweak},
      {"meal", UiMode::kMeal, Action::kMeal, 1, 2, 0, noTweak},
      {"activity_menu", UiMode::kActivityMenu, Action::kGame, 3, 2, 0,
       noTweak},
      {"connection_menu", UiMode::kConnectionMenu, Action::kLinkGame, 4, 2, 0,
       relationTweak},
      {"care", UiMode::kHealth, Action::kHealth, 5, 0, 0, careTweak},
      {"discipline_menu", UiMode::kDisciplineMenu, Action::kDiscipline, 6, 1,
       0, careTweak},
      {"shop", UiMode::kShop, Action::kGame, 3, 1, 0, noTweak},
      {"item", UiMode::kItem, Action::kGame, 3, 8, 0, noTweak},
      {"game_get", UiMode::kGame, Action::kGame, 3, 0, 0, gameGetTweak},
      {"game_flag", UiMode::kGame, Action::kGame, 3, 2, 0, gameFlagTweak},
      {"friends", UiMode::kFriends, Action::kFriendList, 9, 1, 0,
       relationTweak},
      {"family", UiMode::kFamily, Action::kFriendList, 9, 0, 0, noTweak},
      {"souvenirs", UiMode::kSouvenirs, Action::kFriendList, 9, 4, 0,
       noTweak},
      {"toilet", UiMode::kToilet, Action::kToilet, 2, 0, 0, dirtyTweak},
      {"care_toilet", UiMode::kToilet, Action::kHealth, 5, 0, 0,
       dirtyTweak},
      {"medicine", UiMode::kMedicine, Action::kMedicine, 7, 0, 0, sickTweak},
      {"lights", UiMode::kLights, Action::kLights, 8, 0, 0, lightsTweak},
      {"sprite_proof", UiMode::kSpriteProof, Action::kHealth, -1, 5, 0,
       noTweak},
      {"setup", UiMode::kSetup, Action::kSound, -1, 1, 0, noTweak},
  };

  int failures = 0;
  int writes = 0;
  for (const Profile& profile : profiles) {
    if (profileFilter != "all" && profileFilter != profile.name) continue;
    for (const Scenario& scenario : scenarios) {
      if (!matches(scenarioFilters, scenario.name)) continue;
      Snapshot pet = seedSnapshot();
      scenario.tweak(pet);
      UiState ui{};
      ui.mode = scenario.mode;
      ui.cursor = scenario.cursor;
      ui.page = scenario.page;
      Adafruit_SSD1681 display(profile.width, profile.height);
      const uint8_t selectedMenu =
          scenario.selectedMenuIndex >= 0
              ? static_cast<uint8_t>(scenario.selectedMenuIndex)
              : echopet::menuActionIndex(scenario.selectedAction);
      echopet::drawEchoPet(display, pet, selectedMenu, ui,
                           static_cast<uint8_t>(phase));
      const std::string leaf = std::string(profile.name) + "_" + scenario.name +
                               (phaseExplicit
                                    ? "_p" + std::to_string(phase)
                                    : "") +
                               "_x" + std::to_string(scale) + ".png";
      const std::string path = joinPath(outDir, leaf);
      if (!display.writePng(path.c_str(), scale)) {
        std::fprintf(stderr, "failed to write %s\n", path.c_str());
        ++failures;
      } else {
        std::printf("wrote %s\n", path.c_str());
        ++writes;
      }
    }
  }

  if (matches(scenarioFilters, "catalog_items")) {
    Adafruit_SSD1681 display(640, 720);
    echopet::drawEchoPetCatalogVisualProof(display, false);
    const int proofScale = scale > 2 ? 2 : scale;
    const std::string path = joinPath(outDir, "catalog_items_proof_x" +
                                                 std::to_string(proofScale) +
                                                 ".png");
    if (!display.writePng(path.c_str(), proofScale)) {
      std::fprintf(stderr, "failed to write %s\n", path.c_str());
      ++failures;
    } else {
      std::printf("wrote %s\n", path.c_str());
      ++writes;
    }
  }

  if (matches(scenarioFilters, "catalog_souvenirs")) {
    Adafruit_SSD1681 display(640, 320);
    echopet::drawEchoPetCatalogVisualProof(display, true);
    const int proofScale = scale > 2 ? 2 : scale;
    const std::string path = joinPath(outDir, "catalog_souvenirs_proof_x" +
                                                 std::to_string(proofScale) +
                                                 ".png");
    if (!display.writePng(path.c_str(), proofScale)) {
      std::fprintf(stderr, "failed to write %s\n", path.c_str());
      ++failures;
    } else {
      std::printf("wrote %s\n", path.c_str());
      ++writes;
    }
  }

  if (matches(scenarioFilters, "character_roster")) {
    Adafruit_SSD1681 display(640, 470);
    echopet::drawEchoPetCharacterRosterProof(display);
    const int proofScale = scale > 2 ? 2 : scale;
    const std::string path =
        joinPath(outDir, "character_roster_proof_x" +
                             std::to_string(proofScale) + ".png");
    if (!display.writePng(path.c_str(), proofScale)) {
      std::fprintf(stderr, "failed to write %s\n", path.c_str());
      ++failures;
    } else {
      std::printf("wrote %s\n", path.c_str());
      ++writes;
    }
  }

  if (matches(scenarioFilters, "fixed_icons")) {
    Adafruit_SSD1681 display(640, 170);
    echopet::drawEchoPetMenuIconProof(display);
    const int proofScale = scale > 2 ? 2 : scale;
    const std::string path =
        joinPath(outDir, "fixed_menu_icons_proof_x" +
                             std::to_string(proofScale) + ".png");
    if (!display.writePng(path.c_str(), proofScale)) {
      std::fprintf(stderr, "failed to write %s\n", path.c_str());
      ++failures;
    } else {
      std::printf("wrote %s\n", path.c_str());
      ++writes;
    }
  }

  std::printf("screen simulator wrote %d PNG(s)\n", writes);
  return failures == 0 ? 0 : 1;
}
