#include "EchoPetUi.h"

#include <string.h>

#include "EchoPetSprites.h"

namespace echopet {

namespace {

constexpr Action kConnectMenuActions[] = {
    Action::kHealth,     Action::kMeal,       Action::kToilet,
    Action::kGame,       Action::kLinkGame,   Action::kHealth,
    Action::kDiscipline, Action::kMedicine,   Action::kLights,
    Action::kFriendList,
};

constexpr uint8_t kConnectMenuIcons[] = {
    0, 1, 2, 9, 3,
    4, 5, 6, 7, 8,
};

constexpr MenuSlotRole kConnectMenuRoles[] = {
    MenuSlotRole::kHealth,     MenuSlotRole::kFood,
    MenuSlotRole::kToilet,     MenuSlotRole::kGame,
    MenuSlotRole::kConnect,    MenuSlotRole::kCareCall,
    MenuSlotRole::kDiscipline, MenuSlotRole::kMedicine,
    MenuSlotRole::kLights,     MenuSlotRole::kFriend,
};

const char* const kConnectMenuLabels[] = {
    "HEALTH", "FOOD", "TOILET", "GAME", "CONNECT",
    "CARE", "TRAIN", "MEDS", "LIGHTS", "FRIEND",
};

constexpr uint8_t kConnectMenuActionCount =
    sizeof(kConnectMenuActions) / sizeof(kConnectMenuActions[0]);
constexpr uint8_t kConnectMenuSplit = 5;
constexpr uint8_t kSpriteProofFrameCount = kSpriteFrameCount;

}  // namespace

UiMode modeForAction(Action action) {
  switch (action) {
    case Action::kHealth:
      return UiMode::kHealth;
    case Action::kMeal:
      return UiMode::kFoodMenu;
    case Action::kSnack:
      return UiMode::kFoodMenu;
    case Action::kGame:
      return UiMode::kActivityMenu;
    case Action::kShop:
      return UiMode::kShop;
    case Action::kItem:
      return UiMode::kItem;
    case Action::kVisit:
      return UiMode::kVisitLink;
    case Action::kFriendList:
      return UiMode::kFriends;
    case Action::kPresent:
      return UiMode::kPresent;
    case Action::kLinkGame:
      return UiMode::kConnectionMenu;
    case Action::kFamily:
      return UiMode::kFamily;
    case Action::kPassword:
      return UiMode::kPassword;
    case Action::kToilet:
      return UiMode::kToilet;
    case Action::kMedicine:
      return UiMode::kMedicine;
    case Action::kLights:
      return UiMode::kLights;
    case Action::kDiscipline:
    case Action::kPraise:
      return UiMode::kDisciplineMenu;
    case Action::kReset:
      return UiMode::kResetConfirm;
    default:
      return UiMode::kHome;
  }
}

bool actionHasPage(Action action) {
  return modeForAction(action) != UiMode::kHome;
}

uint8_t menuActionCount() {
  return kConnectMenuActionCount;
}

uint8_t menuActionSplit() {
  return kConnectMenuSplit;
}

Action menuActionAt(uint8_t index) {
  if (index >= kConnectMenuActionCount) {
    index = 0;
  }
  return kConnectMenuActions[index];
}

MenuSlotRole menuRoleAt(uint8_t index) {
  if (index >= kConnectMenuActionCount) {
    index = 0;
  }
  return kConnectMenuRoles[index];
}

uint8_t menuActionIndex(Action action) {
  for (uint8_t i = 0; i < kConnectMenuActionCount; i++) {
    if (kConnectMenuActions[i] == action) {
      return i;
    }
  }
  return 0;
}

uint8_t menuIconAt(uint8_t index) {
  if (index >= kConnectMenuActionCount) {
    index = 0;
  }
  return kConnectMenuIcons[index];
}

const char* menuLabelAt(uint8_t index) {
  if (index >= kConnectMenuActionCount) {
    index = 0;
  }
  return kConnectMenuLabels[index];
}

void uiEnter(UiState& ui, Action action) {
  ui.mode = modeForAction(action);
  ui.cursor = 0;
  ui.page = 0;
  ui.secretCode = false;
  memset(ui.entry, 0, sizeof(ui.entry));
}

void uiExit(UiState& ui) {
  ui.mode = UiMode::kHome;
  ui.cursor = 0;
  ui.page = 0;
  ui.secretCode = false;
  memset(ui.entry, 0, sizeof(ui.entry));
}

uint8_t uiItemCount(UiMode mode, const Snapshot& pet) {
  (void)pet;
  switch (mode) {
    case UiMode::kHealth:
      return 6;
    case UiMode::kFoodMenu:
      return 2;
    case UiMode::kMeal:
      return kDefaultMealCount;
    case UiMode::kSnack:
      return kDefaultSnackCount;
    case UiMode::kActivityMenu:
      return 6;
    case UiMode::kGame:
      return pet.unlockedGames ? pet.unlockedGames : 1;
    case UiMode::kLinkGame:
      return kLinkGameCount;
    case UiMode::kShop:
      return kShopSlotCount;
    case UiMode::kItem:
      return kCatalogItemCount;
    case UiMode::kPoint:
      return 1;
    case UiMode::kConnectionMenu:
      return 4;
    case UiMode::kVisitLink:
      return 1;
    case UiMode::kFriends:
      return pet.friendCount ? pet.friendCount : 1;
    case UiMode::kFamily:
      return kFamilyHistoryCount;
    case UiMode::kSouvenirs:
      return kCatalogSouvenirCount;
    case UiMode::kPassword:
      return 10;
    case UiMode::kPresent:
      return kCatalogItemCount + 1;
    case UiMode::kLinkStandby:
    case UiMode::kLinkResult:
      return 1;
    case UiMode::kDisciplineMenu:
      return 2;
    case UiMode::kToilet:
    case UiMode::kMedicine:
    case UiMode::kLights:
    case UiMode::kDiscipline:
    case UiMode::kFriendDeleteConfirm:
    case UiMode::kResetConfirm:
      return 1;
    case UiMode::kSpriteProof:
      return kSpriteProofFrameCount;
    case UiMode::kClockSet:
      return 2;
    case UiMode::kSetup:
      return 1;
    case UiMode::kHome:
      return 1;
  }
  return 1;
}

void uiNext(UiState& ui, const Snapshot& pet) {
  if (ui.mode == UiMode::kPassword) {
    ui.cursor++;
    if (ui.cursor >= 10) ui.cursor = 0;
    if (ui.page < sizeof(ui.entry)) {
      ui.entry[ui.page] = ui.cursor;
    }
    return;
  }

  const uint8_t count = uiItemCount(ui.mode, pet);
  if (count == 0) {
    ui.cursor = 0;
    return;
  }
  ui.cursor++;
  if (ui.cursor >= count) ui.cursor = 0;
  ui.page = ui.cursor;
}

const char* uiModeLabel(UiMode mode) {
  switch (mode) {
    case UiMode::kHome:
      return "HOME";
    case UiMode::kHealth:
      return "HEALTH";
    case UiMode::kFoodMenu:
      return "FOOD";
    case UiMode::kMeal:
      return "MEAL";
    case UiMode::kSnack:
      return "SNACK";
    case UiMode::kActivityMenu:
      return "ACTIVITY";
    case UiMode::kGame:
      return "GAME";
    case UiMode::kShop:
      return "SHOP";
    case UiMode::kItem:
      return "ITEM";
    case UiMode::kPoint:
      return "POINT";
    case UiMode::kConnectionMenu:
      return "CONNECT";
    case UiMode::kVisitLink:
      return "VISIT";
    case UiMode::kFriends:
      return "FRIEND";
    case UiMode::kFamily:
      return "FAMILY";
    case UiMode::kSouvenirs:
      return "MEMORY";
    case UiMode::kPresent:
      return "PRESENT";
    case UiMode::kLinkGame:
      return "LINKGAME";
    case UiMode::kLinkStandby:
      return "STANDBY";
    case UiMode::kLinkResult:
      return "LINK";
    case UiMode::kPassword:
      return "CODE";
    case UiMode::kDisciplineMenu:
      return "DISCIPLINE";
    case UiMode::kToilet:
      return "TOILET";
    case UiMode::kMedicine:
      return "MEDS";
    case UiMode::kLights:
      return "LIGHTS";
    case UiMode::kDiscipline:
      return "TRAIN";
    case UiMode::kFriendDeleteConfirm:
      return "DELFRIEND";
    case UiMode::kResetConfirm:
      return "RESET";
    case UiMode::kSpriteProof:
      return "SPRITE";
    case UiMode::kClockSet:
      return "CLOCK";
    case UiMode::kSetup:
      return "SETUP";
  }
  return "";
}

}  // namespace echopet
