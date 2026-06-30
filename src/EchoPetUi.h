#pragma once

#include <stdint.h>

#include "EchoPetModel.h"

namespace echopet {

enum class UiMode : uint8_t {
  kHome,
  kHealth,
  kFoodMenu,
  kMeal,
  kSnack,
  kActivityMenu,
  kGame,
  kShop,
  kItem,
  kPoint,
  kConnectionMenu,
  kVisitLink,
  kFriends,
  kFamily,
  kSouvenirs,
  kPresent,
  kLinkGame,
  kLinkStandby,
  kLinkResult,
  kPassword,
  kDisciplineMenu,
  kToilet,
  kMedicine,
  kLights,
  kDiscipline,
  kFriendDeleteConfirm,
  kResetConfirm,
  kSpriteProof,
  kClockSet,
  kSetup,
};

enum class MenuSlotRole : uint8_t {
  kHealth,
  kFood,
  kToilet,
  kGame,
  kConnect,
  kCareCall,
  kDiscipline,
  kMedicine,
  kLights,
  kFriend,
};

struct UiState {
  UiMode mode = UiMode::kHome;
  uint8_t cursor = 0;
  uint8_t page = 0;
  uint8_t entry[10] = {};
  bool secretCode = false;
};

UiMode modeForAction(Action action);
bool actionHasPage(Action action);
uint8_t menuActionCount();
uint8_t menuActionSplit();
Action menuActionAt(uint8_t index);
uint8_t menuActionIndex(Action action);
MenuSlotRole menuRoleAt(uint8_t index);
uint8_t menuIconAt(uint8_t index);
const char* menuLabelAt(uint8_t index);
void uiEnter(UiState& ui, Action action);
void uiExit(UiState& ui);
void uiNext(UiState& ui, const Snapshot& pet);
uint8_t uiItemCount(UiMode mode, const Snapshot& pet);
const char* uiModeLabel(UiMode mode);

}  // namespace echopet
