#pragma once

#include <stdint.h>

#include "Adafruit_EPD.h"
#include "EchoPetModel.h"

namespace echopet {

enum class SpriteFrame : uint8_t {
  kEgg0,
  kEgg1,
  kEggCrack0,
  kEggCrack1,
  kEggHatch,
  kBaby0,
  kBaby1,
  kChild0,
  kChild1,
  kTeen0,
  kTeen1,
  kAdult0,
  kAdult1,
  kElder0,
  kElder1,
  kSleep,
  kSick,
  kJoy,
  kSad,
  kFriend,
  kAthlete,
  kScholar,
  kDream,
  kRascal,
  kEatMeal0,
  kEatMeal1,
  kEatSnack0,
  kEatSnack1,
  kFoodCrumbs,
  kFoodRefuse,
  kFoodDone,
  kToilet0,
  kToilet1,
  kToiletMess,
  kToiletSweep0,
  kToiletSweep1,
  kToiletDone,
  kToiletNoMess,
  kMedicine0,
  kMedicine1,
  kMedicineSickSkull,
  kMedicineSickTooth,
  kMedicineDose0,
  kMedicineDose1,
  kMedicineRecover,
  kMedicineRefuse,
  kLightsOn,
  kLightsOff,
  kLightsSelectorOn,
  kLightsSelectorOff,
  kLightsWake,
  kLightsInvalid,
  kDisciplineTimeout,
  kDisciplinePraise,
  kDisciplineInvalid,
  kAttentionCall,
  kAttentionMissed,
  kItemPlay,
  kShopBooth,
  kShopkeeperIdle,
  kShopkeeperSurprise,
  kShopkeeperHappy,
  kShopItemPreview,
  kShopBuyOk,
  kShopBuyNoMoney,
  kShopBuyFull,
  kShopSoldOut,
  kLinkSend,
  kLinkReceive,
  kLovePartner,
  kLoveBaby,
  kParentDepart,
  kGameWin,
  kGameLose,
  kGameGetNote,
  kGameGetBad,
  kGameGetCatch,
  kGameGetMiss,
  kGameBumpMeter,
  kGameBumpPush,
  kGameBumpFall,
  kGameFlagLeft,
  kGameFlagRight,
  kGameFlagBoth,
  kGameFlagGood,
  kGameFlagMiss,
  kGameHeadingBall,
  kGameHeadingHit,
  kGameHeadingMiss,
  kGameMemoryReveal,
  kGameMemoryCursor,
  kGameMemoryGood,
  kGameMemoryWrong,
  kGameSprintRunner0,
  kGameSprintRunner1,
  kGameSprintFinish,
  kGameHoopsHoop,
  kGameHoopsShoot,
  kGameHoopsMade,
  kGameHoopsMiss,
  kPassed,
};

constexpr uint8_t kSpriteFrameCount =
    static_cast<uint8_t>(SpriteFrame::kPassed) + 1;

struct SpriteFrameInfo {
  uint8_t width;
  uint8_t height;
};

void drawTile(Adafruit_SSD1681& display, uint8_t tile, int16_t x, int16_t y,
              uint8_t scale);
void drawMessIcon(Adafruit_SSD1681& display, int16_t x, int16_t y,
                  uint8_t scale);
void drawSpriteFrame(Adafruit_SSD1681& display, SpriteFrame frame, int16_t x,
                     int16_t y, uint8_t scale);
SpriteFrameInfo spriteFrameInfo(SpriteFrame frame);
SpriteFrame selectSpriteFrame(const Snapshot& pet, uint8_t phase);

}  // namespace echopet
