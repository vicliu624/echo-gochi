#include "EchoPetDisplay.h"

#include <stdio.h>
#include <string.h>

#include "EchoPetMenuIconResources.h"
#include "EchoPetResources128.h"
#include "EchoPetResources64.h"
#include "EchoPetCatalog.h"
#include "EchoPetCatalogVisuals.h"
#include "EchoPetCharacterCatalog.h"
#include "EchoPetCharacterVisuals.h"
#include "EchoPetSprites.h"

namespace echopet {

namespace {

const ScreenResources& resourcesFor(EchoPetDisplayDevice& display) {
  if (display.width() == 128 && display.height() == 64) {
    return kEchoPetResources64;
  }
  return kEchoPetResources128;
}

void drawText(EchoPetDisplayDevice& display, int16_t x, int16_t y,
              const char* text, uint8_t size = 1) {
  display.setTextSize(size);
  display.setTextColor(EPD_BLACK);
  display.setCursor(x, y);
  display.print(text);
}

uint16_t textPixelWidth(const char* text, uint8_t size) {
  return static_cast<uint16_t>(strlen(text) * 6U * size);
}

void drawCenteredLine(EchoPetDisplayDevice& display, const ScreenResources& r,
                      int16_t y, const char* text, uint8_t size) {
  const uint16_t width = textPixelWidth(text, size);
  int16_t x = r.mainX;
  if (width < r.mainW) {
    x += static_cast<int16_t>((r.mainW - width) / 2);
  }
  drawText(display, x, y, text, size);
}

const char* shortAttentionLabel(AttentionReason reason) {
  switch (reason) {
    case AttentionReason::kHungry:
      return "HUNGRY";
    case AttentionReason::kSad:
      return "SAD";
    case AttentionReason::kDirty:
      return "CLEAN";
    case AttentionReason::kSick:
      return "SICK";
    case AttentionReason::kNaughty:
    case AttentionReason::kPraise:
      return "TRAIN";
    case AttentionReason::kNone:
      return "";
  }
  return "";
}

const char* shortRouteLabel(GrowthRoute route) {
  switch (route) {
    case GrowthRoute::kBalanced:
      return "BAL";
    case GrowthRoute::kAthlete:
      return "ATH";
    case GrowthRoute::kScholar:
      return "SCH";
    case GrowthRoute::kSocial:
      return "SOC";
    case GrowthRoute::kDreamer:
      return "DRM";
    case GrowthRoute::kRascal:
      return "BAD";
  }
  return "BAL";
}

const char* shortNoticeLabel(const Snapshot& pet) {
  switch (pet.notice) {
    case Notice::kNone:
      return shortAttentionLabel(pet.attentionReason);
    case Notice::kBorn:
      return "BORN";
    case Notice::kStatus:
      return "STATUS";
    case Notice::kMeal:
      return "MEAL";
    case Notice::kSnack:
      return "SNACK";
    case Notice::kFull:
      return "FULL";
    case Notice::kGameStart:
      return "GAME";
    case Notice::kGameGood:
    case Notice::kGameWin:
      return "GOOD";
    case Notice::kGameMiss:
      return "MISS";
    case Notice::kGameLose:
      return "LOSE";
    case Notice::kGameTired:
      return "TIRED";
    case Notice::kPoints:
      return "POINTS";
    case Notice::kBought:
      return "BOUGHT";
    case Notice::kNoPoints:
      return "NO PTS";
    case Notice::kInventoryFull:
      return "FULL";
    case Notice::kItemUsed:
      return "ITEM";
    case Notice::kNoItem:
      return "NO ITEM";
    case Notice::kPasswordOk:
      return "CODE OK";
    case Notice::kPasswordDone:
      return "CODES";
    case Notice::kPasswordBad:
      return "BAD CODE";
    case Notice::kGift:
    case Notice::kFriendPresent:
      return "GIFT";
    case Notice::kFriendReady:
      return "READY";
    case Notice::kFriendVisit:
      return "VISIT";
    case Notice::kFriendGame:
      return "LINK";
    case Notice::kFriendFull:
      return "FULL";
    case Notice::kFriendDeleted:
      return "DELETED";
    case Notice::kPartner:
      return "PARTNER";
    case Notice::kLoveRejected:
      return "REJECT";
    case Notice::kBaby:
      return "BABY";
    case Notice::kParentLeft:
      return "PARENT";
    case Notice::kMatchmaker:
      return "MATCH";
    case Notice::kFamily:
      return "FAMILY";
    case Notice::kAnniversary:
      return "MEMORY";
    case Notice::kBirthday:
      return "BIRTHDAY";
    case Notice::kSouvenir:
      return "MEMORY";
    case Notice::kDonate:
      return "DONATE";
    case Notice::kClean:
      return "CLEAN";
    case Notice::kNoMess:
      return "NO MESS";
    case Notice::kMedicine:
      return "MEDS";
    case Notice::kNeedMoreMedicine:
      return "MORE MED";
    case Notice::kNoMedicine:
      return "NO MED";
    case Notice::kSickRefuse:
      return "SICK";
    case Notice::kTrain:
      return "TIMEOUT";
    case Notice::kPraise:
      return "PRAISE";
    case Notice::kWrongDiscipline:
      return "WRONG";
    case Notice::kLightsOn:
      return "LIGHT ON";
    case Notice::kLightsOff:
      return "LIGHT OFF";
    case Notice::kSoundOn:
      return "SND ON";
    case Notice::kSoundOff:
      return "SND OFF";
    case Notice::kSleeping:
      return "SLEEP";
    case Notice::kNeedsCare:
      return "CARE";
    case Notice::kGrew:
      return "GREW";
    case Notice::kPassed:
      return "MEMORY";
    case Notice::kResetReady:
      return "RESET";
  }
  return "";
}

bool isShopResultNotice(Notice notice) {
  return notice == Notice::kBought || notice == Notice::kNoPoints ||
         notice == Notice::kInventoryFull || notice == Notice::kNoItem;
}

SpriteFrame shopResultFrame(Notice notice) {
  switch (notice) {
    case Notice::kBought:
      return SpriteFrame::kShopBuyOk;
    case Notice::kNoPoints:
      return SpriteFrame::kShopBuyNoMoney;
    case Notice::kInventoryFull:
      return SpriteFrame::kShopBuyFull;
    case Notice::kNoItem:
      return SpriteFrame::kShopSoldOut;
    default:
      return SpriteFrame::kShopItemPreview;
  }
}

const char* shopResultLabel(Notice notice) {
  switch (notice) {
    case Notice::kBought:
      return "THANKS";
    case Notice::kNoPoints:
      return "NO POINTS";
    case Notice::kInventoryFull:
      return "FULL";
    case Notice::kNoItem:
      return "SOLD OUT";
    default:
      return nullptr;
  }
}

void drawTopInfoLine(EchoPetDisplayDevice& display, const ScreenResources& r,
                     uint8_t selectedMenuIndex, const UiState& ui) {
  if (r.compactText) {
    return;
  }
  const char* text =
      ui.mode == UiMode::kHome ? menuLabelAt(selectedMenuIndex)
                               : uiModeLabel(ui.mode);
  drawCenteredLine(display, r, 4, text, 2);
}

const char* controlHintFor(UiMode mode, bool compact) {
  if (compact) {
    switch (mode) {
      case UiMode::kHome:
        return "A> BOK C?";
      case UiMode::kPassword:
        return "A+ BOK C<";
      case UiMode::kLinkStandby:
        return "B TX C<";
      case UiMode::kLinkResult:
        return "B/C<";
      case UiMode::kResetConfirm:
        return "BOK C<";
      case UiMode::kSpriteProof:
        return "A/B> C<";
      case UiMode::kClockSet:
        return "A+ BOK C<";
      case UiMode::kSetup:
        return "A+ BOK";
      case UiMode::kVisitLink:
      case UiMode::kPresent:
      case UiMode::kLinkGame:
      case UiMode::kFoodMenu:
      case UiMode::kActivityMenu:
      case UiMode::kConnectionMenu:
      case UiMode::kDisciplineMenu:
      case UiMode::kMeal:
      case UiMode::kSnack:
      case UiMode::kGame:
      case UiMode::kShop:
      case UiMode::kItem:
      case UiMode::kPoint:
      case UiMode::kFriends:
      case UiMode::kFamily:
      case UiMode::kSouvenirs:
      case UiMode::kFriendDeleteConfirm:
      case UiMode::kToilet:
      case UiMode::kMedicine:
      case UiMode::kLights:
      case UiMode::kDiscipline:
      case UiMode::kHealth:
        return "A> BOK C<";
    }
  }

  switch (mode) {
    case UiMode::kHome:
      return "A ICON B OK C STATUS";
    case UiMode::kHealth:
      return "A PAGE B PAGE C BACK";
    case UiMode::kPassword:
      return "A DIGIT B OK C BACK";
    case UiMode::kLinkStandby:
      return "B SEND C STOP";
    case UiMode::kLinkResult:
      return "B/C BACK";
    case UiMode::kResetConfirm:
      return "B RESET C BACK";
    case UiMode::kSpriteProof:
      return "A/B FRAME C BACK";
    case UiMode::kClockSet:
      return "A CHANGE B OK C BACK";
    case UiMode::kSetup:
      return "A CHANGE B OK";
    case UiMode::kFriends:
      return "A NEXT B NEXT C BACK";
    case UiMode::kFriendDeleteConfirm:
      return "B DELETE C NO";
    case UiMode::kVisitLink:
      return "B VISIT C BACK";
    case UiMode::kPresent:
      return "A GIFT B SEND C BACK";
    case UiMode::kLinkGame:
      return "A GAME B SEND C BACK";
    case UiMode::kFoodMenu:
    case UiMode::kActivityMenu:
    case UiMode::kConnectionMenu:
    case UiMode::kDisciplineMenu:
    case UiMode::kMeal:
    case UiMode::kSnack:
    case UiMode::kGame:
    case UiMode::kShop:
    case UiMode::kItem:
    case UiMode::kPoint:
    case UiMode::kFamily:
    case UiMode::kSouvenirs:
    case UiMode::kToilet:
    case UiMode::kMedicine:
    case UiMode::kLights:
    case UiMode::kDiscipline:
      return "A NEXT B OK C BACK";
  }
  return "";
}

void drawBottomInfoLine(EchoPetDisplayDevice& display, const ScreenResources& r,
                        const Snapshot& pet, const UiState& ui) {
  if (r.compactText) {
    return;
  }
  if (ui.mode != UiMode::kHome && pet.notice == Notice::kStatus) {
    return;
  }
  const char* text = shortNoticeLabel(pet);
  if (!text || !text[0]) {
    text = controlHintFor(ui.mode, false);
    if (!text || !text[0]) {
      return;
    }
    drawCenteredLine(display, r, r.mainY + r.mainH + 8, text, 1);
    return;
  }
  drawCenteredLine(display, r, r.mainY + r.mainH + 4, text, 2);
}

void drawCompactInfoLine(EchoPetDisplayDevice& display, const ScreenResources& r,
                         const Snapshot& pet, const UiState& ui) {
  (void)display;
  (void)r;
  (void)pet;
  (void)ui;
}

void drawPackedBitmap(EchoPetDisplayDevice& display, const uint8_t* bitmap,
                      uint8_t width, uint8_t height, int16_t x, int16_t y,
                      uint16_t color) {
  const uint8_t rowBytes = static_cast<uint8_t>((width + 7U) / 8U);
  for (uint8_t row = 0; row < height; ++row) {
    for (uint8_t col = 0; col < width; ++col) {
      const uint8_t bits =
          pgm_read_byte(bitmap + static_cast<uint16_t>(row) * rowBytes +
                        (col >> 3));
      if ((bits & (0x80U >> (col & 0x07))) == 0) {
        continue;
      }
      display.drawPixel(x + col, y + row, color);
    }
  }
}

void drawMenuIcon(EchoPetDisplayDevice& display, const ScreenResources& r,
                  uint8_t iconIndex, int16_t x, int16_t y, uint16_t color) {
  const uint8_t i = iconIndex % 10;
  if (r.compactText) {
    drawPackedBitmap(display, kEchoPetMenuIcons64Compact[i],
                     kEchoPetMenuIconCompactSide, kEchoPetMenuIconCompactSide,
                     x, y, color);
  } else {
    drawPackedBitmap(display, kEchoPetMenuIcons128Large[i],
                     kEchoPetMenuIconLargeSide, kEchoPetMenuIconLargeSide, x, y,
                     color);
  }
}

void drawMenuColumn(EchoPetDisplayDevice& display, const ScreenResources& r,
                    uint8_t selectedMenuIndex, bool right,
                    const Snapshot& pet, uint8_t animationPhase) {
  if (r.compactText) {
    if (right) {
      return;
    }
    const uint8_t menuCount = menuActionCount();
    const uint8_t visible = 2;
    const uint8_t rowH = static_cast<uint8_t>(r.leftH / visible);
    display.drawRect(r.leftX, r.leftY, 32, r.leftH, EPD_BLACK);
    uint8_t start = 0;
    if (selectedMenuIndex >= visible) {
      start = static_cast<uint8_t>(selectedMenuIndex - visible + 1);
    }
    if (start + visible > menuCount) {
      start = static_cast<uint8_t>(menuCount > visible ? menuCount - visible : 0);
    }

    for (uint8_t i = 0; i < visible; ++i) {
      const uint8_t menuIndex = static_cast<uint8_t>(start + i);
      if (menuIndex >= menuCount) {
        break;
      }
      const int16_t rowY = r.leftY + static_cast<int16_t>(i) * rowH;
      const uint8_t iconIndex = menuIconAt(menuIndex);
      const bool selected = menuIndex == selectedMenuIndex;
      const bool attentionPulse =
          iconIndex == 4 && pet.attention && ((animationPhase & 0x01) == 0);
      if (selected) {
        const int16_t cy = rowY + rowH / 2;
        display.fillTriangle(r.leftX + 3, cy - 5, r.leftX + 3, cy + 5,
                             r.leftX + 8, cy, EPD_BLACK);
      }
      const int16_t iconX = r.leftX + 9;
      const int16_t iconY =
          rowY + static_cast<int16_t>((rowH - kEchoPetMenuIconCompactSide) / 2);
      drawMenuIcon(display, r, iconIndex, iconX, iconY, EPD_BLACK);
      if (attentionPulse) {
        display.drawRect(iconX - 1, iconY - 1, kEchoPetMenuIconCompactSide + 2,
                         kEchoPetMenuIconCompactSide + 2, EPD_BLACK);
      }
    }
    return;
  }

  const int16_t x = right ? r.rightX : r.leftX;
  const int16_t y = right ? r.rightY : r.leftY;
  const uint16_t h = right ? r.rightH : r.leftH;
  const uint8_t iconSize = kEchoPetMenuIconLargeSide;
  const uint8_t visible = 5;
  const uint8_t menuCount = menuActionCount();
  const uint8_t split = menuActionSplit();
  const uint8_t start = right ? split : 0;
  const uint8_t end = right ? menuCount : split;
  const uint8_t count = end - start;

  display.drawRect(x, y, 32, h, EPD_BLACK);
  for (uint8_t i = 0; i < visible; i++) {
    if (i >= count) {
      break;
    }
    const uint16_t rowH = h / visible;
    const int16_t rowY =
        y + i * rowH +
        (r.compactText ? 1 : static_cast<int16_t>((rowH - iconSize) / 2));
    const int16_t iconX = x + (32 - iconSize) / 2;
    const uint8_t menuIndex = start + i;
    const uint8_t iconIndex = menuIconAt(menuIndex);
    const bool selected = menuIndex == selectedMenuIndex;
    const bool attentionPulse =
        iconIndex == 4 && pet.attention && ((animationPhase & 0x01) == 0);
    if (selected) {
      const uint8_t highlightH = r.compactText ? rowH : iconSize + 4;
      const int16_t highlightX = r.compactText ? x + 2 : x + 1;
      const uint8_t highlightW = r.compactText ? 28 : 30;
      display.fillRect(highlightX, r.compactText ? rowY : rowY - 2,
                       highlightW, highlightH,
                       EPD_BLACK);
      display.setTextColor(EPD_WHITE);
    } else {
      display.setTextColor(EPD_BLACK);
    }
    drawMenuIcon(display, r, iconIndex, iconX, rowY,
                 selected ? EPD_WHITE : EPD_BLACK);
    if (attentionPulse) {
      const int16_t bx = r.compactText ? x + 3 : x + 1;
      const int16_t by = r.compactText ? rowY : rowY - 3;
      const int16_t bw = r.compactText ? 26 : 30;
      const int16_t bh = r.compactText ? rowH : iconSize + 6;
      display.drawRect(bx, by, bw, bh, selected ? EPD_WHITE : EPD_BLACK);
      if (by > 5) {
        display.drawLine(bx + 3, by - 2, bx + 7, by - 5,
                         selected ? EPD_WHITE : EPD_BLACK);
        display.drawLine(bx + bw - 4, by - 2, bx + bw - 8, by - 5,
                         selected ? EPD_WHITE : EPD_BLACK);
      }
    }
    display.setTextSize(1);
  }
  display.setTextColor(EPD_BLACK);
}

void drawMainFrame(EchoPetDisplayDevice& display, const ScreenResources& r) {
  if (r.compactText) {
    return;
  }
  display.drawRect(r.mainX, r.mainY, r.mainW, r.mainH, EPD_BLACK);
}

struct V3Canvas {
  int16_t x;
  int16_t y;
  uint8_t scale;
  uint8_t nativeW;
  uint8_t nativeH;
  uint16_t screenW;
  uint16_t screenH;
};

V3Canvas v3CanvasFor(const ScreenResources& r) {
  constexpr uint8_t kNativeW = 32;
  constexpr uint8_t kNativeH = 30;
  constexpr uint8_t kScale = 2;
  const uint16_t screenW = kNativeW * kScale;
  const uint16_t screenH = kNativeH * kScale;
  return {
      static_cast<int16_t>(r.mainX + (static_cast<int16_t>(r.mainW) -
                                      static_cast<int16_t>(screenW)) /
                                         2),
      static_cast<int16_t>(r.mainY + (static_cast<int16_t>(r.mainH) -
                                      static_cast<int16_t>(screenH)) /
                                         2),
      kScale,
      kNativeW,
      kNativeH,
      screenW,
      screenH,
  };
}

void drawCharacterTraitOverlay(EchoPetDisplayDevice& display, int16_t x, int16_t y,
                               int16_t width, int16_t height, uint8_t scale,
                               const Snapshot& pet,
                               const CharacterCatalogEntry& character,
                               uint8_t animationPhase) {
  if (pet.stage == Stage::kEgg || pet.mood == Mood::kAsleep ||
      pet.mood == Mood::kPassed) {
    return;
  }
  if ((character.visualTraits & 0x80) != 0) {
    return;
  }

  const uint8_t topTrait = (character.visualTraits >> 5) & 0x07;
  const uint8_t sideTrait = (character.visualTraits >> 2) & 0x07;
  const uint8_t faceTrait = character.visualTraits & 0x03;
  const int16_t cx = x + width / 2;
  const int16_t topY = y + scale;
  const int16_t headW = width > 8 * scale ? width - 2 * scale : width;
  const int16_t left = x + (width - headW) / 2;
  const int16_t right = left + headW - scale;
  const int16_t midY = y + height / 2;
  const int16_t eyeY = y + height / 3;
  const int16_t footY = y + height - scale;
  const bool blink = (animationPhase & 0x03) == 1;

  switch (topTrait) {
    case 1:
      display.fillTriangle(cx - 3 * scale, topY, cx, topY - 4 * scale,
                           cx + 3 * scale, topY, EPD_BLACK);
      break;
    case 2:
      display.drawLine(cx - 5 * scale, topY, cx - 2 * scale,
                       topY - 4 * scale, EPD_BLACK);
      display.drawLine(cx + 5 * scale, topY, cx + 2 * scale,
                       topY - 4 * scale, EPD_BLACK);
      break;
    case 3:
      display.drawRect(cx - 5 * scale, topY - 4 * scale, 10 * scale,
                       4 * scale, EPD_BLACK);
      break;
    case 4:
      display.drawCircle(cx - 4 * scale, topY - 2 * scale, 3 * scale,
                         EPD_BLACK);
      display.drawCircle(cx + 4 * scale, topY - 2 * scale, 3 * scale,
                         EPD_BLACK);
      break;
    case 5:
      display.drawLine(cx, topY, cx, topY - 6 * scale, EPD_BLACK);
      display.drawLine(cx, topY - 6 * scale, cx + 4 * scale,
                       topY - 8 * scale, EPD_BLACK);
      break;
    case 6:
      display.fillRect(cx - 4 * scale, topY - 2 * scale, 8 * scale,
                       2 * scale, EPD_BLACK);
      break;
    case 7:
      display.drawTriangle(cx - 5 * scale, topY, cx - 1 * scale,
                           topY - 5 * scale, cx + 2 * scale, topY,
                           EPD_BLACK);
      break;
    default:
      break;
  }

  switch (sideTrait) {
    case 1:
      display.drawCircle(left - 2 * scale, midY - 2 * scale, 3 * scale,
                         EPD_BLACK);
      display.drawCircle(right + 2 * scale, midY - 2 * scale, 3 * scale,
                         EPD_BLACK);
      break;
    case 2:
      display.drawLine(left, midY - 4 * scale, left - 5 * scale, midY,
                       EPD_BLACK);
      display.drawLine(right, midY - 4 * scale, right + 5 * scale, midY,
                       EPD_BLACK);
      break;
    case 3:
      display.fillRect(left - 3 * scale, midY - 2 * scale, 3 * scale,
                       4 * scale, EPD_BLACK);
      display.fillRect(right + scale, midY - 2 * scale, 3 * scale,
                       4 * scale, EPD_BLACK);
      break;
    case 4:
      display.drawLine(left, midY, left - 5 * scale, midY - 5 * scale,
                       EPD_BLACK);
      display.drawLine(right, midY, right + 5 * scale, midY - 5 * scale,
                       EPD_BLACK);
      break;
    case 5:
      display.drawRect(left - 3 * scale, midY - scale, 3 * scale, 6 * scale,
                       EPD_BLACK);
      display.drawRect(right + scale, midY - scale, 3 * scale, 6 * scale,
                       EPD_BLACK);
      break;
    case 6:
      display.drawLine(left, midY + 2 * scale, left - 5 * scale,
                       midY + 5 * scale, EPD_BLACK);
      display.drawLine(right, midY + 2 * scale, right + 5 * scale,
                       midY + 5 * scale, EPD_BLACK);
      break;
    case 7:
      display.drawCircle(left - scale, midY + 3 * scale, 2 * scale,
                         EPD_BLACK);
      display.drawCircle(right + scale, midY + 3 * scale, 2 * scale,
                         EPD_BLACK);
      break;
    default:
      break;
  }

  if (faceTrait == 1) {
    if (blink) {
      display.drawLine(cx - 5 * scale, eyeY, cx - 2 * scale, eyeY,
                       EPD_BLACK);
      display.drawLine(cx + 2 * scale, eyeY, cx + 5 * scale, eyeY,
                       EPD_BLACK);
    } else {
      display.fillRect(cx - 5 * scale, eyeY - scale, 2 * scale, 2 * scale,
                       EPD_BLACK);
      display.fillRect(cx + 3 * scale, eyeY - scale, 2 * scale, 2 * scale,
                       EPD_BLACK);
    }
  } else if (faceTrait == 2) {
    display.drawLine(cx - 4 * scale, eyeY - scale, cx - 2 * scale,
                     eyeY + scale, EPD_BLACK);
    display.drawLine(cx + 4 * scale, eyeY - scale, cx + 2 * scale,
                     eyeY + scale, EPD_BLACK);
    display.drawLine(cx - 3 * scale, midY + 2 * scale, cx + 3 * scale,
                     midY + 2 * scale, EPD_BLACK);
  } else if (faceTrait == 3) {
    display.fillCircle(cx - 4 * scale, eyeY, scale, EPD_BLACK);
    display.fillCircle(cx + 4 * scale, eyeY, scale, EPD_BLACK);
    display.drawCircle(cx, midY + 2 * scale, 2 * scale, EPD_BLACK);
  }

  if ((character.sourceSlot & 0x01) != 0) {
    display.fillRect(cx - 5 * scale, footY, 3 * scale, scale, EPD_BLACK);
    display.fillRect(cx + 2 * scale, footY, 3 * scale, scale, EPD_BLACK);
  } else {
    display.drawLine(cx - 5 * scale, footY, cx - 2 * scale, footY,
                     EPD_BLACK);
    display.drawLine(cx + 2 * scale, footY, cx + 5 * scale, footY,
                     EPD_BLACK);
  }
}

struct IdleMotion {
  int8_t x;
  int8_t y;
};

IdleMotion connectionIdleMotion(const Snapshot& pet,
                                const CharacterCatalogEntry& character,
                                uint8_t phase) {
  const uint8_t p = phase & 0x0F;
  if (pet.stage == Stage::kEgg) {
    const int8_t x[4] = {-1, 0, 1, 0};
    return {x[phase & 0x03], static_cast<int8_t>((phase & 0x01) ? -1 : 0)};
  }

  if (pet.stage == Stage::kElder ||
      (pet.stage == Stage::kAdult &&
       pet.adultTier == static_cast<uint8_t>(AdultTier::kFrail))) {
    const int8_t x[16] = {-8, -5, -2, 0, 2, 5, 8, 5,
                          2,  0, -2, -5, -8, -5, -2, 0};
    const int8_t y[16] = {0, 1, 2, 1, 0, 1, 2, 1,
                          0, 1, 2, 1, 0, 1, 2, 1};
    return {x[p], y[p]};
  }

  if (pet.stage == Stage::kAdult &&
      pet.adultTier == static_cast<uint8_t>(AdultTier::kSerious)) {
    const int8_t x[16] = {-10, -7, -4, -1, 0, 0, 1, 4,
                           7, 10,  7,  4, 1, 0, -1, -4};
    const int8_t y[16] = {0, 0, 0, -2, -5, -2, 0, 0,
                          0, 0, 0, -2, -5, -2, 0, 0};
    return {x[p], y[p]};
  }

  if (pet.stage == Stage::kAdult &&
      (pet.adultTier == static_cast<uint8_t>(AdultTier::kNaughty) ||
       character.route == GrowthRoute::kRascal)) {
    const int8_t x[16] = {-20, -14, -8, -2, 6, 14, 20, 14,
                           8,   2, -6, -14, -20, -14, -8, -2};
    const int8_t y[16] = {0, -4, -8, -4, 0, -4, -8, -4,
                          0, -4, -8, -4, 0, -4, -8, -4};
    return {x[p], y[p]};
  }

  const int8_t x[16] = {-18, -12, -6, 0, 6, 12, 18, 12,
                         6,   0, -6, -12, -18, -12, -6, 0};
  const int8_t y[16] = {0, -3, -6, -3, 0, -3, -6, -3,
                        0, -3, -6, -3, 0, -3, -6, -3};
  return {x[p], y[p]};
}

void drawCharacter(EchoPetDisplayDevice& display, const ScreenResources& r,
                   const Snapshot& pet, uint8_t animationPhase) {
  const SpriteFrame frame = selectSpriteFrame(pet, animationPhase);
  const SpriteFrameInfo info = spriteFrameInfo(frame);
  if (info.width == 0 || info.height == 0) {
    return;
  }

  const CharacterCatalogEntry& character =
      characterCatalogEntry(pet.characterCatalogId);
  const uint8_t scale = r.spriteScale;
  const bool isEgg = pet.stage == Stage::kEgg;
  const bool idleNotice = pet.notice == Notice::kNone ||
                          pet.notice == Notice::kBorn ||
                          pet.notice == Notice::kStatus;
  const bool familyVisual =
      !isEgg && idleNotice && pet.mood != Mood::kAsleep &&
      pet.mood != Mood::kSick && pet.mood != Mood::kPassed;
  const bool compactV3Visual = familyVisual && r.compactText;
  const uint8_t familyScale =
      familyVisual ? static_cast<uint8_t>(r.compactText ? 2U : scale * 2U)
                   : scale;
  const uint8_t familySide =
      compactV3Visual ? kCharacterVisualSide : kCharacterIdleVisualSide;
  const int16_t width =
      familyVisual ? familySide * familyScale
                   : info.width * scale;
  const int16_t height =
      familyVisual ? familySide * familyScale
                   : info.height * scale;
  const bool canIdleMove = !isEgg && pet.mood != Mood::kAsleep &&
                           pet.mood != Mood::kPassed;
  const IdleMotion idle =
      (isEgg || canIdleMove) ? connectionIdleMotion(pet, character,
                                                    animationPhase)
                             : IdleMotion{0, 0};
  const uint8_t motionScale = compactV3Visual ? 2 : scale;
  const int16_t walkX = idle.x * motionScale;
  const int16_t bobY = idle.y * motionScale;
  const V3Canvas canvas = v3CanvasFor(r);
  const int16_t sceneX = compactV3Visual ? canvas.x : r.mainX;
  const int16_t sceneY = compactV3Visual ? canvas.y : r.mainY;
  const int16_t sceneW = compactV3Visual ? canvas.screenW : r.mainW;
  const int16_t sceneH = compactV3Visual ? canvas.screenH : r.mainH;

  int16_t x = sceneX + (sceneW - width) / 2 + walkX;
  const int16_t minX = compactV3Visual ? sceneX : r.mainX + 1;
  const int16_t maxX =
      compactV3Visual ? sceneX + sceneW - width : r.mainX + r.mainW - width - 1;
  if (maxX >= minX) {
    if (x < minX) x = minX;
    if (x > maxX) x = maxX;
  }

  const int16_t floorMargin =
      compactV3Visual ? 6 : ((familyVisual && r.compactText)
                                 ? 13
                                 : (r.compactText ? 11 : 16));
  int16_t y = sceneY + sceneH - height - floorMargin + bobY;
  const int16_t minY =
      compactV3Visual
          ? sceneY
          : r.mainY + ((familyVisual && r.compactText) ? 3
                                                       : (r.compactText ? 10 : 6));
  const int16_t maxY =
      compactV3Visual ? sceneY + sceneH - height : r.mainY + r.mainH - height - 1;
  if (maxY >= minY) {
    if (y < minY) y = minY;
    if (y > maxY) y = maxY;
  }
  if (familyVisual) {
    if (compactV3Visual) {
      drawCharacterCatalogBitmap(display, x, y, pet.characterCatalogId,
                                 animationPhase, familyScale);
    } else {
      drawCharacterCatalogIdleBitmap(display, x, y, pet.characterCatalogId,
                                     animationPhase, familyScale);
    }
  } else {
    drawSpriteFrame(display, frame, x, y, scale);
  }
  drawCharacterTraitOverlay(display, x, y, width, height, scale, pet,
                            character, animationPhase);
}

int16_t messXForIndex(const ScreenResources& r, uint8_t index) {
  const int16_t rightInset = r.compactText ? 13 : 20;
  const int16_t step = r.compactText ? 10 : 13;
  return r.mainX + r.mainW - rightInset - index * step;
}

int16_t messYForIndex(const ScreenResources& r, uint8_t index) {
  return r.mainY + r.mainH - (r.compactText ? 14 : 20) +
         (index & 0x01) * 2;
}

void drawMessWithStink(EchoPetDisplayDevice& display, int16_t x, int16_t y,
                       uint8_t phase) {
  drawMessIcon(display, x, y, 1);
  const int16_t leftX = x + 5 + ((phase & 0x01) ? 1 : 0);
  const int16_t rightX = x + 11 - ((phase & 0x01) ? 1 : 0);
  const int16_t top = y - 7;
  display.drawPixel(leftX, top + 1, EPD_BLACK);
  display.drawPixel(leftX + 1, top + 2, EPD_BLACK);
  display.drawPixel(leftX, top + 3, EPD_BLACK);
  display.drawPixel(rightX, top, EPD_BLACK);
  display.drawPixel(rightX - 1, top + 1, EPD_BLACK);
  display.drawPixel(rightX, top + 2, EPD_BLACK);
}

void drawMesses(EchoPetDisplayDevice& display, const ScreenResources& r,
                uint8_t count, uint8_t phase, int16_t clearBeforeX) {
  for (uint8_t i = 0; i < count && i < 4; i++) {
    const int16_t x = messXForIndex(r, i);
    if (x + 8 <= clearBeforeX) {
      continue;
    }
    drawMessWithStink(display, x, messYForIndex(r, i), phase + i);
  }
}

void drawMess(EchoPetDisplayDevice& display, const ScreenResources& r,
              const Snapshot& pet, uint8_t phase) {
  drawMesses(display, r, pet.messCount, phase, -32768);
}

void drawPageTitle(EchoPetDisplayDevice& display, const ScreenResources& r,
                   const char* title) {
  if (r.compactText) {
    return;
  }
  const uint8_t size = r.compactText ? 1 : 2;
  drawText(display, r.mainX + 4, r.mainY + 5, title, size);
}

void drawPageLine(EchoPetDisplayDevice& display, const ScreenResources& r,
                  uint8_t row, const char* text) {
  const int16_t y = r.mainY + 20 + row * (r.compactText ? 9 : 14);
  drawText(display, r.mainX + 5, y, text);
}

void drawChoiceMenu(EchoPetDisplayDevice& display, const ScreenResources& r,
                    const char* title, const char* const* labels,
                    uint8_t count, uint8_t cursor) {
  drawPageTitle(display, r, title);
  const uint8_t rows = r.compactText ? 3 : 4;
  uint8_t start = 0;
  if (cursor >= rows) start = cursor - rows + 1;
  const int16_t rowStep = r.compactText ? 10 : 18;
  const int16_t y0 = r.mainY + (r.compactText ? 20 : 34);
  for (uint8_t row = 0; row < rows; row++) {
    const uint8_t index = start + row;
    if (index >= count) break;
    const int16_t y = y0 + row * rowStep;
    if (index == cursor) {
      display.fillTriangle(r.mainX + 5, y + 3, r.mainX + 5, y + 10,
                           r.mainX + 11, y + 6, EPD_BLACK);
    }
    drawText(display, r.mainX + 16, y, labels[index], r.compactText ? 1 : 2);
  }
}

uint8_t pipsFor(uint8_t value) {
  if (value > 100) value = 100;
  uint8_t pips = static_cast<uint8_t>((value + 24) / 25);
  return pips > 4 ? 4 : pips;
}

void drawSmallHeart(EchoPetDisplayDevice& display, int16_t x, int16_t y,
                    bool filled) {
  if (filled) {
    display.fillRect(x + 1, y, 2, 2, EPD_BLACK);
    display.fillRect(x + 5, y, 2, 2, EPD_BLACK);
    display.fillRect(x, y + 2, 8, 2, EPD_BLACK);
    display.fillRect(x + 1, y + 4, 6, 1, EPD_BLACK);
    display.fillRect(x + 2, y + 5, 4, 1, EPD_BLACK);
    display.fillRect(x + 3, y + 6, 2, 1, EPD_BLACK);
  } else {
    display.drawRect(x + 1, y, 2, 2, EPD_BLACK);
    display.drawRect(x + 5, y, 2, 2, EPD_BLACK);
    display.drawRect(x, y + 2, 8, 2, EPD_BLACK);
    display.drawPixel(x + 2, y + 5, EPD_BLACK);
    display.drawPixel(x + 5, y + 5, EPD_BLACK);
    display.drawPixel(x + 3, y + 6, EPD_BLACK);
    display.drawPixel(x + 4, y + 6, EPD_BLACK);
  }
}

void drawPipRowSpaced(EchoPetDisplayDevice& display, int16_t x, int16_t y,
                      uint8_t value, uint8_t step) {
  const uint8_t pips = pipsFor(value);
  for (uint8_t i = 0; i < 4; i++) {
    drawSmallHeart(display, x + i * step, y, i < pips);
  }
}

void drawBowlIcon(EchoPetDisplayDevice& display, int16_t x, int16_t y,
                  uint8_t scale) {
  display.fillTriangle(x, y + 5 * scale, x + 16 * scale, y + 5 * scale,
                       x + 8 * scale, y + 12 * scale, EPD_BLACK);
  display.drawRect(x + 2 * scale, y + 2 * scale, 12 * scale, 4 * scale,
                   EPD_BLACK);
  display.fillRect(x + 5 * scale, y, 6 * scale, 2 * scale, EPD_BLACK);
}

void drawFoodItem(EchoPetDisplayDevice& display, int16_t x, int16_t y,
                  uint8_t kind, uint8_t phase, uint8_t scale) {
  drawCatalogFoodBitmap(display, x, y, kind % kFoodKindCount, phase, scale);
}

void drawItemIcon(EchoPetDisplayDevice& display, int16_t x, int16_t y,
                  ItemKind item, uint8_t phase, uint8_t scale) {
  drawCatalogItemBitmap(display, x, y, static_cast<uint8_t>(item), phase,
                        scale);
}

void drawSparkle(EchoPetDisplayDevice& display, int16_t x, int16_t y,
                 uint8_t phase) {
  if ((phase & 0x01) == 0) {
    display.drawLine(x - 3, y, x + 3, y, EPD_BLACK);
    display.drawLine(x, y - 3, x, y + 3, EPD_BLACK);
  } else {
    display.drawPixel(x, y - 2, EPD_BLACK);
    display.drawPixel(x + 2, y, EPD_BLACK);
    display.drawPixel(x, y + 2, EPD_BLACK);
    display.drawPixel(x - 2, y, EPD_BLACK);
  }
}

bool drawCatalogFoodSpecialIcon(EchoPetDisplayDevice& display, int16_t x,
                                int16_t y, uint8_t icon, uint8_t phase,
                                uint8_t scale) {
  if (icon >= kCatalogFoodVisualCount) return false;
  drawCatalogFoodBitmap(display, x, y, icon, phase, scale);
  return true;
}

bool drawCatalogItemSpecialIcon(EchoPetDisplayDevice& display, int16_t x,
                                int16_t y, uint8_t icon, uint8_t phase,
                                uint8_t scale) {
  if (icon >= kCatalogItemVisualCount) return false;
  drawCatalogItemBitmap(display, x, y, icon, phase, scale);
  return true;
}

void drawCatalogTraitOverlay(EchoPetDisplayDevice& display, int16_t x, int16_t y,
                             const CatalogEntry& entry, uint8_t phase,
                             uint8_t scale) {
  const uint8_t topTrait = (entry.visualTraits >> 5) & 0x07;
  const uint8_t sideTrait = (entry.visualTraits >> 2) & 0x07;
  const uint8_t markTrait = entry.visualTraits & 0x03;
  const int16_t s = scale;
  const int16_t left = x;
  const int16_t top = y;
  const int16_t right = x + kCatalogVisualSide * s;
  const int16_t bottom = y + kCatalogVisualSide * s;
  const bool pulse = (phase & 0x01) != 0;

  switch (topTrait) {
    case 1:
      display.drawLine(left + 2 * s, top, left + 8 * s, top, EPD_BLACK);
      display.drawLine(left + 3 * s, top + s, left + 7 * s, top + s,
                       EPD_BLACK);
      break;
    case 2:
      display.drawRect(left + 3 * s, top, 5 * s, 3 * s, EPD_BLACK);
      break;
    case 3:
      display.fillTriangle(left + 4 * s, top, left + 8 * s, top,
                           left + 6 * s, top - 3 * s, EPD_BLACK);
      break;
    case 4:
      display.drawCircle(left + 5 * s, top + s, 2 * s, EPD_BLACK);
      break;
    default:
      break;
  }

  switch (sideTrait) {
    case 1:
      display.drawLine(left, top + 6 * s, left - 2 * s, top + 8 * s,
                       EPD_BLACK);
      break;
    case 2:
      display.drawLine(right, top + 6 * s, right + 2 * s, top + 8 * s,
                       EPD_BLACK);
      break;
    case 3:
      display.drawLine(left, top + 5 * s, left - 2 * s, top + 7 * s,
                       EPD_BLACK);
      display.drawLine(right, top + 5 * s, right + 2 * s, top + 7 * s,
                       EPD_BLACK);
      break;
    case 4:
      display.fillRect(left + s, bottom - 2 * s, 4 * s, s, EPD_BLACK);
      break;
    case 5:
      display.fillRect(right - 5 * s, bottom - 2 * s, 4 * s, s, EPD_BLACK);
      break;
    case 6:
      display.drawCircle(right - 2 * s, top + 3 * s, s, EPD_BLACK);
      break;
    case 7:
      display.drawCircle(left + 2 * s, top + 3 * s, s, EPD_BLACK);
      display.drawCircle(right - 2 * s, top + 3 * s, s, EPD_BLACK);
      break;
    default:
      break;
  }

  switch (markTrait) {
    case 1:
      display.fillCircle(left + 3 * s, bottom - 3 * s, s, EPD_BLACK);
      break;
    case 2:
      display.drawLine(right - 5 * s, bottom - 3 * s, right - 2 * s,
                       bottom - 6 * s, EPD_BLACK);
      display.drawLine(right - 2 * s, bottom - 6 * s, right, bottom - 4 * s,
                       EPD_BLACK);
      break;
    case 3:
      drawSparkle(display, right - 3 * s, bottom - 4 * s,
                  static_cast<uint8_t>(phase + entry.visualTraits));
      break;
    default:
      break;
  }

  if ((entry.flags & kCatalogFlagSecret) != 0) {
    display.drawRect(right - 4 * s, top, 3 * s, 4 * s, EPD_BLACK);
    display.drawLine(right - 3 * s, top, right - 3 * s, top - 2 * s,
                     EPD_BLACK);
  }
  if ((entry.flags & kCatalogFlagReusable) != 0 && pulse) {
    display.drawLine(left + 2 * s, bottom, left + 6 * s, bottom, EPD_BLACK);
    display.drawLine(left + 6 * s, bottom, left + 5 * s, bottom - 2 * s,
                     EPD_BLACK);
  }
  if (entry.kind == CatalogKind::kSouvenir) {
    display.drawRect(left + 7 * s, bottom - 4 * s, 4 * s, 4 * s, EPD_BLACK);
  }
}

void drawCatalogIcon(EchoPetDisplayDevice& display, int16_t x, int16_t y,
                     const CatalogEntry& entry, uint8_t catalogIndex,
                     uint8_t phase, uint8_t scale) {
  if (catalogIndex < kCatalogItemCount) {
    drawCatalogEntryBitmap(display, x, y, catalogIndex, phase, scale);
    return;
  }
  if (entry.kind == CatalogKind::kFood) {
    if (entry.icon >= kFoodKindCount &&
        drawCatalogFoodSpecialIcon(display, x, y, entry.icon, phase, scale)) {
      drawCatalogTraitOverlay(display, x, y, entry, phase, scale);
      return;
    }
    drawFoodItem(display, x, y, entry.icon % kFoodKindCount, phase, scale);
    drawCatalogTraitOverlay(display, x, y, entry, phase, scale);
    return;
  }
  if (entry.kind == CatalogKind::kItem) {
    if (entry.icon >= kItemKindCount &&
        drawCatalogItemSpecialIcon(display, x, y, entry.icon, phase, scale)) {
      drawCatalogTraitOverlay(display, x, y, entry, phase, scale);
      return;
    }
    drawItemIcon(display, x, y,
                 static_cast<ItemKind>(entry.icon % kItemKindCount), phase,
                 scale);
    drawCatalogTraitOverlay(display, x, y, entry, phase, scale);
    return;
  }
  drawCatalogSouvenirBitmap(display, x, y, entry.behavior, phase, scale);
  drawCatalogTraitOverlay(display, x, y, entry, phase, scale);
}

void drawSouvenirIcon(EchoPetDisplayDevice& display, int16_t x, int16_t y,
                      uint8_t index, bool found, uint8_t phase,
                      uint8_t scale) {
  const int16_t s = scale;
  const CatalogEntry entry = {
      CatalogKind::kSouvenir,
      index,
      0,
      static_cast<uint8_t>(index & 0x07),
      0,
      8,
      kCatalogUseMemory,
      catalogSouvenirVisualTraits(index),
  };
  drawCatalogSouvenirBitmap(display, x, y, index, phase, scale);
  if (found) {
    display.drawLine(x + 3 * s, y + 17 * s, x + 13 * s, y + 17 * s,
                     EPD_BLACK);
    drawSparkle(display, x + 14 * s, y + 2 * s,
                static_cast<uint8_t>(phase + index));
  } else {
    display.drawLine(x + 3 * s, y + 13 * s, x + 13 * s, y + 3 * s,
                     EPD_BLACK);
    drawText(display, x + 5 * s, y + 5 * s, "?", scale);
  }
  drawCatalogTraitOverlay(display, x, y, entry, phase, scale);
}

void drawCatalogUseScene(EchoPetDisplayDevice& display, const ScreenResources& r,
                         const CatalogEntry& entry, uint8_t catalogIndex,
                         uint8_t phase) {
  const uint8_t scale = r.compactText ? 1 : 2;
  const int16_t x = r.mainX + r.mainW - (r.compactText ? 24 : 50);
  const int16_t y = r.mainY + (r.compactText ? 16 : 34);
  switch (entry.useScene) {
    case kCatalogUseMeal:
    case kCatalogUseSnack:
      drawCatalogIcon(display, x, y, entry, catalogIndex, phase, scale);
      if (entry.useScene == kCatalogUseSnack) {
        drawSmallHeart(display, x + 17 * scale, y + 1 * scale,
                       (phase & 1) != 0);
      }
      break;
    case kCatalogUseStudy:
      drawCatalogIcon(display, x, y, entry, catalogIndex, phase, scale);
      display.drawLine(x + 2 * scale, y + 17 * scale, x + 18 * scale,
                       y + 17 * scale, EPD_BLACK);
      break;
    case kCatalogUseMusic:
      drawCatalogIcon(display, x, y, entry, catalogIndex, phase, scale);
      display.fillCircle(x + 18 * scale, y + 2 * scale, scale, EPD_BLACK);
      display.drawLine(x + 19 * scale, y + 2 * scale, x + 19 * scale,
                       y + 8 * scale, EPD_BLACK);
      break;
    case kCatalogUseTravel:
      drawCatalogIcon(display, x, y, entry, catalogIndex, phase, scale);
      display.drawLine(x - 2 * scale, y + 17 * scale, x + 22 * scale,
                       y + 17 * scale, EPD_BLACK);
      display.drawTriangle(x + 11 * scale, y + 2 * scale, x + 17 * scale,
                           y + 7 * scale, x + 11 * scale, y + 12 * scale,
                           EPD_BLACK);
      break;
    case kCatalogUseMemory:
      display.drawRect(x + 1 * scale, y + 2 * scale, 18 * scale, 13 * scale,
                       EPD_BLACK);
      display.drawLine(x + 4 * scale, y + 5 * scale, x + 16 * scale,
                       y + 5 * scale, EPD_BLACK);
      drawSparkle(display, x + 10 * scale, y + 10 * scale, phase);
      break;
    case kCatalogUsePlay:
    default:
      drawCatalogIcon(display, x, y, entry, catalogIndex, phase, scale);
      drawSparkle(display, x + 20 * scale, y + 3 * scale, phase);
      break;
  }
}

void drawSmallFace(EchoPetDisplayDevice& display, int16_t x, int16_t y,
                   uint8_t phase) {
  display.drawCircle(x + 8, y + 8, 8, EPD_BLACK);
  display.fillCircle(x + 5, y + 7, 1, EPD_BLACK);
  display.fillCircle(x + 11, y + 7, 1, EPD_BLACK);
  if ((phase & 0x01) == 0) {
    display.drawLine(x + 5, y + 12, x + 11, y + 12, EPD_BLACK);
  } else {
    display.drawCircle(x + 8, y + 11, 3, EPD_BLACK);
  }
}

uint8_t catalogIdForFriendAvatar(const FriendRecord& record) {
  const CharacterKind kind =
      record.character <= static_cast<uint8_t>(CharacterKind::kSage)
          ? static_cast<CharacterKind>(record.character)
          : CharacterKind::kBaby;
  if (record.catalogId < kCharacterCatalogCount &&
      characterCatalogEntry(record.catalogId).archetype == kind) {
    return record.catalogId;
  }
  for (uint8_t i = 0; i < kCharacterCatalogCount; i++) {
    if (characterCatalogEntry(i).archetype == kind) return i;
  }
  return 0;
}

uint8_t familyRouteValue(const FamilyRecord& record) {
  return record.route & kFamilyRecordRouteMask;
}

uint8_t familyParentCatalogId(const FamilyRecord& record) {
  return record.catalogId < kCharacterCatalogCount ? record.catalogId : 0;
}

bool familyHasPartnerCatalog(const FamilyRecord& record) {
  return (record.route & kFamilyRecordPartnerCatalogValid) != 0 &&
         record.character < kCharacterCatalogCount;
}

bool familyAncestryValid(const FamilyAncestryRecord& record) {
  return (record.flags & kFamilyAncestryValid) != 0 &&
         record.parentCatalogIdA < kCharacterCatalogCount &&
         record.babyCatalogId < kCharacterCatalogCount;
}

bool familyAncestryHasPartnerCatalog(const FamilyAncestryRecord& record) {
  return familyAncestryValid(record) &&
         (record.flags & kFamilyAncestryPartnerCatalogValid) != 0 &&
         record.parentCatalogIdB < kCharacterCatalogCount;
}

void drawUnknownAvatar(EchoPetDisplayDevice& display, int16_t x, int16_t y,
                       uint8_t scale) {
  const uint8_t s = scale == 0 ? 1 : scale;
  const int16_t w = 16 * s;
  const int16_t cx = x + 8 * s;
  const int16_t cy = y + 8 * s;
  display.drawRect(x + 2 * s, y + 2 * s, 12 * s, 12 * s, EPD_BLACK);
  display.drawLine(x + 4 * s, y + 4 * s, x + w - 4 * s, y + w - 4 * s,
                   EPD_BLACK);
  display.drawLine(x + w - 4 * s, y + 4 * s, x + 4 * s, y + w - 4 * s,
                   EPD_BLACK);
  display.fillCircle(cx - 3 * s, cy - 2 * s, s, EPD_BLACK);
  display.fillCircle(cx + 3 * s, cy - 2 * s, s, EPD_BLACK);
}

void drawCatalogAvatar(EchoPetDisplayDevice& display, int16_t x, int16_t y,
                       uint8_t catalogId, uint8_t phase, uint8_t scale) {
  const uint8_t s = scale == 0 ? 1 : scale;
  const CharacterCatalogEntry& character = characterCatalogEntry(catalogId);
  const int16_t cx = x + 8 * s;
  const int16_t cy = y + 8 * s;
  const int16_t top = y + 2 * s;
  const int16_t left = x + 2 * s;
  const int16_t right = x + 14 * s;
  const uint8_t topTrait = (character.visualTraits >> 5) & 0x07;
  const uint8_t sideTrait = (character.visualTraits >> 2) & 0x07;
  const uint8_t faceTrait = character.visualTraits & 0x03;
  const bool blink = (phase & 0x03) == 1;

  drawCharacterCatalogBitmap(display, x, y, catalogId, phase, s);
  if ((character.visualTraits & 0x80) != 0) {
    return;
  }

  switch (topTrait) {
    case 1:
      display.fillTriangle(cx - 3 * s, top, cx, top - 3 * s, cx + 3 * s,
                           top, EPD_BLACK);
      break;
    case 2:
      display.drawLine(cx - 4 * s, top, cx - s, top - 3 * s, EPD_BLACK);
      display.drawLine(cx + 4 * s, top, cx + s, top - 3 * s, EPD_BLACK);
      break;
    case 3:
      display.drawRect(cx - 4 * s, top - 3 * s, 8 * s, 3 * s, EPD_BLACK);
      break;
    case 4:
      display.drawCircle(cx - 3 * s, top - s, 2 * s, EPD_BLACK);
      display.drawCircle(cx + 3 * s, top - s, 2 * s, EPD_BLACK);
      break;
    default:
      break;
  }

  if (sideTrait & 0x01) {
    display.drawLine(left, cy, left - 2 * s, cy - 3 * s, EPD_BLACK);
    display.drawLine(right, cy, right + 2 * s, cy - 3 * s, EPD_BLACK);
  }
  if (sideTrait & 0x02) {
    display.fillRect(left - s, cy + 2 * s, s, 3 * s, EPD_BLACK);
    display.fillRect(right, cy + 2 * s, s, 3 * s, EPD_BLACK);
  }

  const int16_t eyeY = y + 7 * s;
  if (blink) {
    display.drawLine(cx - 4 * s, eyeY, cx - 2 * s, eyeY, EPD_BLACK);
    display.drawLine(cx + 2 * s, eyeY, cx + 4 * s, eyeY, EPD_BLACK);
  } else if (faceTrait == 2) {
    display.drawLine(cx - 4 * s, eyeY - s, cx - 2 * s, eyeY + s,
                     EPD_BLACK);
    display.drawLine(cx + 4 * s, eyeY - s, cx + 2 * s, eyeY + s,
                     EPD_BLACK);
  } else {
    display.fillCircle(cx - 3 * s, eyeY, s, EPD_BLACK);
    display.fillCircle(cx + 3 * s, eyeY, s, EPD_BLACK);
  }

  if (faceTrait == 3) {
    display.drawCircle(cx, y + 11 * s, 2 * s, EPD_BLACK);
  } else if ((phase & 0x01) != 0) {
    display.drawLine(cx - 3 * s, y + 11 * s, cx, y + 12 * s, EPD_BLACK);
    display.drawLine(cx, y + 12 * s, cx + 3 * s, y + 11 * s, EPD_BLACK);
  } else {
    display.drawLine(cx - 3 * s, y + 11 * s, cx + 3 * s, y + 11 * s,
                     EPD_BLACK);
  }
}

void drawSceneFrame(EchoPetDisplayDevice& display, const ScreenResources& r,
                    SpriteFrame frame, int8_t xBias = 0,
                    int8_t yBias = 0) {
  const SpriteFrameInfo info = spriteFrameInfo(frame);
  if (info.width == 0 || info.height == 0) {
    return;
  }

  const uint8_t scale = r.spriteScale;
  const int16_t width = info.width * scale;
  const int16_t height = info.height * scale;
  int16_t x = r.mainX + (r.mainW - width) / 2 + xBias * scale;
  const int16_t minX = r.mainX + 1;
  const int16_t maxX = r.mainX + r.mainW - width - 1;
  if (maxX >= minX) {
    if (x < minX) x = minX;
    if (x > maxX) x = maxX;
  }

  const int16_t floorMargin = r.compactText ? 10 : 14;
  int16_t y = r.mainY + r.mainH - height - floorMargin + yBias * scale;
  const int16_t minY = r.mainY + 2;
  const int16_t maxY = r.mainY + r.mainH - height - 1;
  if (maxY >= minY) {
    if (y < minY) y = minY;
    if (y > maxY) y = maxY;
  }
  drawSpriteFrame(display, frame, x, y, scale);
}

void drawScenePet(EchoPetDisplayDevice& display, const ScreenResources& r,
                  const Snapshot& pet, uint8_t phase) {
  drawCharacter(display, r, pet, phase);
}

void drawHealthPage(EchoPetDisplayDevice& display, const ScreenResources& r,
                    const Snapshot& pet, uint8_t page, uint8_t phase) {
  const int16_t x = r.mainX + (r.compactText ? 5 : 12);
  const int16_t y = r.mainY + (r.compactText ? 10 : 16);
  const uint8_t rowGap = r.compactText ? 12 : 23;
  const uint8_t meterX = r.compactText ? 17 : 48;
  const uint8_t iconScale = r.compactText ? 1 : 2;
  const uint8_t pipStep = r.compactText ? 10 : 12;
  char buffer[40];

  switch (page % 6) {
    case 0:
      drawBowlIcon(display, x, y, iconScale);
      drawPipRowSpaced(display, x + meterX, y + 3, pet.hunger, pipStep);
      drawSmallHeart(display, x + 4, y + rowGap + 3, true);
      drawPipRowSpaced(display, x + meterX, y + rowGap + 3, pet.happiness,
                       pipStep);
      break;
    case 1:
      display.drawRect(x, y + 3, 21, 12, EPD_BLACK);
      display.drawLine(x + 3, y + 9, x + 18, y + 9, EPD_BLACK);
      drawPipRowSpaced(display, x + meterX, y + 3, pet.discipline, pipStep);
      display.drawRect(x, y + rowGap + 3, 22, 14, EPD_BLACK);
      display.fillRect(x + 3, y + rowGap + 7, 16, 6, EPD_BLACK);
      drawPipRowSpaced(display, x + meterX, y + rowGap + 3, pet.energy,
                       pipStep);
      break;
    case 2:
      drawScenePet(display, r, pet, phase);
      snprintf(buffer, sizeof(buffer), "AGE %lud",
               static_cast<unsigned long>(pet.ageMinutes / 1440UL));
      drawPageLine(display, r, 0, buffer);
      snprintf(buffer, sizeof(buffer), "WT %u", pet.weight);
      drawPageLine(display, r, 1, buffer);
      break;
    case 3:
      snprintf(buffer, sizeof(buffer), "%s %s", pet.petName,
               pet.gender == Gender::kBoy ? "BOY" : "GIRL");
      drawPageLine(display, r, 0, buffer);
      snprintf(buffer, sizeof(buffer), "GEN %u", pet.generation);
      drawPageLine(display, r, 1, buffer);
      snprintf(buffer, sizeof(buffer), "%u GP", pet.points);
      drawPageLine(display, r, 2, buffer);
      break;
    case 4:
      display.drawRect(x + 2, y + 4, 16, 8, EPD_BLACK);
      drawPipRowSpaced(display, x + meterX, y + 3, pet.hygiene, pipStep);
      snprintf(buffer, sizeof(buffer), "POOP %u", pet.messCount);
      drawPageLine(display, r, 1, buffer);
      if (pet.sickness) {
        drawPageLine(display, r, 2, "SKULL");
      } else if (pet.toothache) {
        drawPageLine(display, r, 2, "TOOTH");
      } else {
        drawPageLine(display, r, 2, "OK");
      }
      break;
    case 5:
      drawSmallFace(display, x + 2, y + 4, phase);
      drawPipRowSpaced(display, x + meterX, y + 8, pet.friendship, pipStep);
      snprintf(buffer, sizeof(buffer), "FRIEND %u", pet.friendCount);
      drawPageLine(display, r, 1, buffer);
      snprintf(buffer, sizeof(buffer), "%s",
               characterCatalogLabel(pet.characterCatalogId));
      drawPageLine(display, r, 2, buffer);
      if (!r.compactText) {
        snprintf(buffer, sizeof(buffer), "RT %s C%u", shortRouteLabel(pet.route),
                 pet.careScore);
        drawPageLine(display, r, 3, buffer);
      }
      break;
  }
}

void drawFoodPage(EchoPetDisplayDevice& display, const ScreenResources& r,
                  const Snapshot& pet, const UiState& ui, bool snack,
                  uint8_t phase) {
  char buffer[32];
  const uint8_t i =
      snack ? static_cast<uint8_t>(kDefaultMealCount +
                                   (ui.cursor % kDefaultSnackCount))
            : static_cast<uint8_t>(ui.cursor % kDefaultMealCount);
  const uint8_t s = r.compactText ? 1 : 2;
  const bool refused = pet.notice == Notice::kFull;
  const bool eating =
      pet.notice == (snack ? Notice::kSnack : Notice::kMeal) ||
      pet.notice == Notice::kMeal || pet.notice == Notice::kSnack;
  const uint8_t stage = refused ? 0 : (eating ? (phase & 0x03) : 0);
  const int16_t foodX = r.mainX + (r.compactText ? 6 : 18);
  const int16_t foodY = r.mainY + (r.compactText ? 12 : 22);
  const int16_t plateX = foodX - 2 * s;
  const int16_t plateY = foodY + 15 * s;
  const int16_t travelX = stage == 1 ? 8 * s : (stage >= 2 ? 15 * s : 0);
  const int16_t bobY = stage == 1 ? -2 * s : 0;

  auto drawFoodPlate = [&]() {
    display.drawRoundRect(plateX, plateY, 24 * s, 8 * s, 3 * s, EPD_BLACK);
    if (stage < 2 || refused) {
      drawFoodItem(display, foodX + travelX, foodY + bobY, i, phase, s);
    }
    if (stage == 2) {
      for (uint8_t crumb = 0; crumb < 5; crumb++) {
        const int16_t cx = plateX + (5 + crumb * 3) * s;
        const int16_t cy = plateY + ((crumb & 1) ? 1 : 4) * s;
        display.fillRect(cx, cy, s, s, EPD_BLACK);
      }
    } else if (stage == 3) {
      display.drawLine(plateX + 5 * s, plateY + 4 * s, plateX + 10 * s,
                       plateY + 7 * s, EPD_BLACK);
      display.drawLine(plateX + 10 * s, plateY + 7 * s, plateX + 19 * s,
                       plateY + 1 * s, EPD_BLACK);
    }
    if (refused) {
      display.drawLine(plateX, plateY, plateX + 24 * s, plateY + 8 * s,
                       EPD_BLACK);
      display.drawLine(plateX + 24 * s, plateY, plateX, plateY + 8 * s,
                       EPD_BLACK);
    }
  };

  auto drawFoodStageBadge = [&]() {
    const int16_t badgeX = r.mainX + r.mainW - 18 * s;
    const int16_t badgeY = r.mainY + (r.compactText ? 16 : 28);
    display.drawRect(badgeX, badgeY, 13 * s, 13 * s, EPD_BLACK);
    if (refused) {
      display.drawLine(badgeX + 3 * s, badgeY + 3 * s, badgeX + 10 * s,
                       badgeY + 10 * s, EPD_BLACK);
      display.drawLine(badgeX + 10 * s, badgeY + 3 * s, badgeX + 3 * s,
                       badgeY + 10 * s, EPD_BLACK);
    } else if (stage == 3) {
      drawSmallHeart(display, badgeX + 2 * s, badgeY + 2 * s, true);
    } else if (stage == 2) {
      display.fillRect(badgeX + 4 * s, badgeY + 4 * s, 2 * s, 2 * s,
                       EPD_BLACK);
      display.fillRect(badgeX + 7 * s, badgeY + 7 * s, 2 * s, 2 * s,
                       EPD_BLACK);
      display.fillRect(badgeX + 9 * s, badgeY + 4 * s, 2 * s, 2 * s,
                       EPD_BLACK);
    } else {
      display.drawLine(badgeX + 4 * s, badgeY + 3 * s, badgeX + 4 * s,
                       badgeY + 10 * s, EPD_BLACK);
      display.drawLine(badgeX + 8 * s, badgeY + 3 * s, badgeX + 8 * s,
                       badgeY + 10 * s, EPD_BLACK);
      display.drawLine(badgeX + 4 * s, badgeY + 6 * s, badgeX + 8 * s,
                       badgeY + 6 * s, EPD_BLACK);
    }
  };

  snprintf(buffer, sizeof(buffer), "%s", foodLabel(static_cast<FoodKind>(i)));
  drawCenteredLine(display, r, r.mainY + (r.compactText ? 2 : 8), buffer, 1);
  drawCenteredLine(display, r, r.mainY + r.mainH - (r.compactText ? 10 : 18),
                   snack ? "SNACK" : "MEAL", 1);
  drawFoodPlate();
  drawFoodStageBadge();
  if (snack) {
    drawSmallHeart(display, foodX + (r.compactText ? 22 : 48),
                   foodY + (r.compactText ? 4 : 8), (phase & 1) != 0);
  }
  SpriteFrame frame = snack ? SpriteFrame::kEatSnack0 : SpriteFrame::kEatMeal0;
  if (refused) {
    frame = SpriteFrame::kFoodRefuse;
  } else {
    switch (stage) {
      case 1:
        frame = snack ? SpriteFrame::kEatSnack1 : SpriteFrame::kEatMeal1;
        break;
      case 2:
        frame = SpriteFrame::kFoodCrumbs;
        break;
      case 3:
        frame = SpriteFrame::kFoodDone;
        break;
      default:
        break;
    }
  }
  drawSceneFrame(display, r, frame, r.compactText ? 10 : 16, 0);
}

void drawItemPage(EchoPetDisplayDevice& display, const ScreenResources& r,
                  const Snapshot& pet, const UiState& ui) {
  char buffer[40];
  const uint8_t i = ui.cursor % kCatalogItemCount;
  const CatalogEntry entry = catalogEntry(i);
  const bool used = pet.notice == Notice::kItemUsed;
  if (used) {
    drawSceneFrame(display, r, SpriteFrame::kItemPlay,
                   r.compactText ? -7 : -10, 0);
    drawCatalogUseScene(display, r, entry, i, ui.cursor);
  } else {
    const uint8_t iconScale = r.compactText ? 1 : 2;
    const int16_t iconSize = kCatalogVisualSide * iconScale;
    const int16_t iconX = r.mainX + (r.mainW - iconSize) / 2;
    const int16_t iconY = r.mainY + (r.mainH - iconSize) / 2;
    drawCatalogIcon(display, iconX, iconY, entry, i, ui.cursor, iconScale);
  }
  catalogItemLabel(i, buffer, sizeof(buffer));
  drawCenteredLine(display, r, r.mainY + (r.compactText ? 2 : 8), buffer, 1);
  const bool owned = catalogBitTest(pet.catalogOwned, i);
  if (owned && pet.catalogStock[i] > 0) {
    snprintf(buffer, sizeof(buffer), "x%u %u/%u",
             static_cast<unsigned>(pet.catalogStock[i]),
             static_cast<unsigned>(i + 1),
             static_cast<unsigned>(kCatalogItemCount));
  } else {
    snprintf(buffer, sizeof(buffer), "%s %u/%u", owned ? "OWN" : "----",
             static_cast<unsigned>(i + 1),
             static_cast<unsigned>(kCatalogItemCount));
  }
  drawCenteredLine(display, r, r.mainY + r.mainH - (r.compactText ? 10 : 18),
                   buffer, 1);
}

void drawScoreDots(EchoPetDisplayDevice& display, int16_t x, int16_t y,
                   uint8_t score);
uint8_t visualProgress(uint8_t value, uint8_t limit, uint8_t span);
uint8_t visualScoreDots(uint8_t score, uint8_t scoreLimit);

void drawGameSelectPage(EchoPetDisplayDevice& display, const ScreenResources& r,
                        const Snapshot& pet, const UiState& ui,
                        bool linkGame, uint8_t phase) {
  const uint8_t count =
      linkGame ? kLinkGameCount : (pet.unlockedGames ? pet.unlockedGames : 1);
  const uint8_t i = ui.cursor % count;
  const int16_t cx = r.mainX + r.mainW / 2;
  const int16_t cy = r.mainY + r.mainH / 2;
  if (linkGame) {
    const LinkGameKind game = linkGameFromIndex(i);
    const bool ready = linkGameAvailable(pet.catalogOwned, game);
    drawCenteredLine(display, r, r.mainY + (r.compactText ? 2 : 8),
                     linkGameLabel(game), 1);
    display.drawRect(cx - 42, cy - 22, 28, 38, EPD_BLACK);
    display.drawRect(cx + 14, cy - 22, 28, 38, EPD_BLACK);
    drawSmallFace(display, cx - 36, cy - 10, phase);
    drawSmallFace(display, cx + 20, cy - 10, phase + 1);
    const uint8_t arc = (phase & 0x03) * 4;
    display.drawCircle(cx, cy - 5, 6 + arc, EPD_BLACK);
    display.drawCircle(cx, cy - 5, 12 + arc, EPD_BLACK);
    drawCenteredLine(display, r, r.mainY + r.mainH - (r.compactText ? 10 : 18),
                     ready ? "READY" : linkGameRequiredLabel(game), 1);
  } else {
    const int16_t ballX = cx - 34 + ((phase + i) & 0x03) * 18;
    display.drawRect(cx - 45, cy - 18, 8, 36, EPD_BLACK);
    display.drawRect(cx + 37, cy - 18, 8, 36, EPD_BLACK);
    display.fillCircle(ballX, cy - 8 + ((phase & 1) ? 8 : 0), 6, EPD_BLACK);
    drawScoreDots(display, cx - 31, cy + 28, pet.bestGameScore > 5 ? 5
                                                                   : pet.bestGameScore);
  }
}

void drawShopPage(EchoPetDisplayDevice& display, const ScreenResources& r,
                  const Snapshot& pet, const UiState& ui) {
  char buffer[40];
  const uint8_t slot = ui.cursor % kShopSlotCount;
  const uint8_t i =
      catalogShopIndex(slot, pet.month, pet.day, pet.birthdayMonth,
                       pet.birthdayDay, pet.clockMinutes);
  const CatalogEntry entry = catalogEntry(i);
  const bool vendor = catalogShopIsVendorVisit(pet.clockMinutes);
  const bool sale = catalogShopIsSale(pet.day, pet.clockMinutes);
  const uint16_t price = catalogShopPrice(i, pet.day, pet.clockMinutes);
  const bool resultVisible = isShopResultNotice(pet.notice);
  const uint8_t boothScale = 1;
  const uint8_t itemScale = r.compactText ? 1 : 2;
  const int16_t boothW = 56;
  const int16_t boothX = r.mainX + (r.mainW - boothW) / 2;
  const int16_t boothY = r.mainY + (r.compactText ? 15 : 56);
  const int16_t clerkX = r.mainX + (r.compactText ? 3 : 18);
  const int16_t clerkY = boothY + (r.compactText ? 11 : 22);
  const int16_t iconX = r.mainX + r.mainW - (r.compactText ? 22 : 48);
  const int16_t iconY = boothY + (r.compactText ? 10 : 18);
  drawSpriteFrame(display, SpriteFrame::kShopBooth, boothX, boothY, boothScale);
  SpriteFrame keeperFrame =
      sale || vendor ? SpriteFrame::kShopkeeperHappy : SpriteFrame::kShopkeeperIdle;
  if (ui.secretCode || pet.notice == Notice::kNoPoints ||
      pet.notice == Notice::kInventoryFull || pet.notice == Notice::kNoItem) {
    keeperFrame = SpriteFrame::kShopkeeperSurprise;
  } else if (pet.notice == Notice::kBought) {
    keeperFrame = SpriteFrame::kShopkeeperHappy;
  }
  drawSpriteFrame(display, keeperFrame, clerkX, clerkY, 1);
  if (ui.secretCode) {
    drawSparkle(display, clerkX + (r.compactText ? 17 : 28),
                clerkY + (r.compactText ? 2 : 4), ui.cursor);
  }
  if (resultVisible) {
    drawSpriteFrame(display, shopResultFrame(pet.notice), iconX, iconY,
                    itemScale);
  } else {
    drawCatalogIcon(display, iconX, iconY, entry, i, ui.cursor, itemScale);
  }
  catalogItemLabel(i, buffer, sizeof(buffer));
  drawCenteredLine(display, r, r.mainY + (r.compactText ? 2 : 8), buffer, 1);
  const char* resultLabel = shopResultLabel(pet.notice);
  if (resultLabel) {
    snprintf(buffer, sizeof(buffer), "%s", resultLabel);
  } else if (ui.secretCode) {
    snprintf(buffer, sizeof(buffer), "SECRET?");
  } else if (ui.entry[0] > 0) {
    snprintf(buffer, sizeof(buffer), "A x%u",
             static_cast<unsigned>(ui.entry[0]));
  } else if (vendor) {
    snprintf(buffer, sizeof(buffer), r.compactText ? "%uP CART" : "%uPT CART",
             static_cast<unsigned>(price));
  } else if (sale) {
    snprintf(buffer, sizeof(buffer), r.compactText ? "%uP SALE" : "%uPT SALE",
             static_cast<unsigned>(price));
  } else {
    if (r.compactText) {
      snprintf(buffer, sizeof(buffer), "%uP %u/%u",
               static_cast<unsigned>(price),
               static_cast<unsigned>(slot + 1),
               static_cast<unsigned>(kShopSlotCount));
    } else {
      snprintf(buffer, sizeof(buffer), "%uPT S%u/%u",
               static_cast<unsigned>(price),
               static_cast<unsigned>(slot + 1),
               static_cast<unsigned>(kShopSlotCount));
    }
  }
  drawCenteredLine(display, r, r.mainY + r.mainH - (r.compactText ? 10 : 18),
                   buffer, 1);
}

void drawFriendPage(EchoPetDisplayDevice& display, const ScreenResources& r,
                    const Snapshot& pet, const UiState& ui, uint8_t phase) {
  char buffer[40];
  const int16_t cx = r.mainX + r.mainW / 2;
  const int16_t cy = r.mainY + r.mainH / 2;
  const uint8_t avatarScale = r.compactText ? 1 : 2;
  const int16_t avatarSize = 16 * avatarScale;
  const uint8_t count = pet.friendCount;
  drawCenteredLine(display, r, r.mainY + (r.compactText ? 2 : 8), "FRIEND", 1);

  if (r.compactText) {
    const int16_t boxY = r.mainY + 14;
    const int16_t leftBoxX = r.mainX + 5;
    const int16_t rightBoxX = r.mainX + 35;
    const int16_t boxW = 24;
    const int16_t boxH = 22;
    display.drawRect(leftBoxX, boxY, boxW, boxH, EPD_BLACK);
    display.drawRect(rightBoxX, boxY, boxW, boxH, EPD_BLACK);
    drawCatalogAvatar(display, leftBoxX + (boxW - avatarSize) / 2,
                      boxY + (boxH - avatarSize) / 2, pet.characterCatalogId,
                      phase, avatarScale);
    if (count == 0) {
      display.drawCircle(rightBoxX + boxW / 2, boxY + boxH / 2, 7, EPD_BLACK);
      display.drawLine(rightBoxX + 6, boxY + boxH - 5, rightBoxX + boxW - 6,
                       boxY + 5, EPD_BLACK);
      drawCenteredLine(display, r, r.mainY + 43, "NO FRIEND", 1);
      return;
    }

    const uint8_t i = ui.cursor % count;
    const FriendRecord& f = pet.friends[i];
    drawCatalogAvatar(display, rightBoxX + (boxW - avatarSize) / 2,
                      boxY + (boxH - avatarSize) / 2,
                      catalogIdForFriendAvatar(f), phase + ui.cursor + 1,
                      avatarScale);
    drawSmallHeart(display, cx - 4, boxY + boxH - 3, (phase & 1) != 0);
    snprintf(buffer, sizeof(buffer), "#%u %s", static_cast<unsigned>(i + 1),
             relationLabel(static_cast<RelationLevel>(f.relation)));
    drawCenteredLine(display, r, r.mainY + 40, buffer, 1);
    return;
  }

  display.drawRect(cx - 40, cy - 28, 34, 44, EPD_BLACK);
  display.drawRect(cx + 6, cy - 28, 34, 44, EPD_BLACK);
  drawCatalogAvatar(display, cx - 40 + (34 - avatarSize) / 2,
                    cy - 28 + (44 - avatarSize) / 2, pet.characterCatalogId,
                    phase, avatarScale);
  if (count == 0) {
    display.drawCircle(cx + 23, cy - 6, 8, EPD_BLACK);
    display.drawLine(cx + 16, cy + 1, cx + 30, cy - 13, EPD_BLACK);
    drawCenteredLine(display, r, r.mainY + r.mainH - (r.compactText ? 21 : 33),
                     "NO FRIEND", 1);
  } else {
    const uint8_t i = ui.cursor % count;
    const FriendRecord& f = pet.friends[i];
    drawCatalogAvatar(display, cx + 6 + (34 - avatarSize) / 2,
                      cy - 28 + (44 - avatarSize) / 2,
                      catalogIdForFriendAvatar(f), phase + ui.cursor + 1,
                      avatarScale);
    drawSmallHeart(display, cx - 4, cy + 22, (phase & 1) != 0);
    snprintf(buffer, sizeof(buffer), "#%u %s", static_cast<unsigned>(i + 1),
             relationLabel(static_cast<RelationLevel>(f.relation)));
    drawCenteredLine(display, r, r.mainY + r.mainH - (r.compactText ? 28 : 42),
                     buffer, 1);
    snprintf(buffer, sizeof(buffer), "V%u G%u",
             static_cast<unsigned>(f.visits), static_cast<unsigned>(f.gifts));
    drawCenteredLine(display, r, r.mainY + r.mainH - (r.compactText ? 19 : 30),
                     buffer, 1);
  }
}

void drawFriendDeletePage(EchoPetDisplayDevice& display, const ScreenResources& r,
                          const Snapshot& pet, const UiState& ui) {
  char buffer[40];
  drawPageTitle(display, r, "DELETE?");
  if (pet.friendCount == 0) {
    drawPageLine(display, r, 0, "No friends");
    return;
  }
  const uint8_t i = ui.cursor % pet.friendCount;
  const FriendRecord& f = pet.friends[i];
  snprintf(buffer, sizeof(buffer), "#%u %s", static_cast<unsigned>(i + 1),
           relationLabel(static_cast<RelationLevel>(f.relation)));
  drawPageLine(display, r, 0, buffer);
  drawPageLine(display, r, 1, "B yes C no");
}

void drawFamilyAlbumCard(EchoPetDisplayDevice& display, const ScreenResources& r,
                         const FamilyRecord* family,
                         const FamilyAncestryRecord* ancestry,
                         uint8_t phase) {
  const uint8_t scale = r.compactText ? 1 : 2;
  const int16_t cardX = r.mainX + (r.compactText ? 4 : 14);
  const int16_t cardY = r.mainY + (r.compactText ? 14 : 26);
  const int16_t cardW = r.mainW - (r.compactText ? 8 : 28);
  const int16_t cardH = r.compactText ? 36 : 76;
  const int16_t cx = r.mainX + r.mainW / 2;
  const int16_t parentY = cardY + (r.compactText ? 9 : 18);
  const int16_t childY = cardY + cardH - (r.compactText ? 11 : 22);
  const int16_t leftX = cx - (r.compactText ? 18 : 32);
  const int16_t rightX = cx + (r.compactText ? 10 : 22);

  display.drawRect(cardX, cardY, cardW, cardH, EPD_BLACK);
  display.drawLine(cardX + 3 * scale, cardY + 4 * scale,
                   cardX + 8 * scale, cardY + 1 * scale, EPD_BLACK);
  display.drawLine(cardX + cardW - 3 * scale, cardY + 4 * scale,
                   cardX + cardW - 8 * scale, cardY + 1 * scale, EPD_BLACK);

  if (!family) {
    drawUnknownAvatar(display, leftX, parentY - 6 * scale, scale);
    drawUnknownAvatar(display, rightX, parentY - 6 * scale, scale);
    drawSmallHeart(display, cx - 4 * scale, childY - 2 * scale,
                   (phase & 1) != 0);
    return;
  }

  const bool hasAncestry = ancestry && familyAncestryValid(*ancestry);
  const uint8_t parentA =
      hasAncestry ? ancestry->parentCatalogIdA : familyParentCatalogId(*family);
  drawCatalogAvatar(display, leftX, parentY - 6 * scale,
                    parentA, phase, scale);
  if (hasAncestry && familyAncestryHasPartnerCatalog(*ancestry)) {
    drawCatalogAvatar(display, rightX, parentY - 6 * scale,
                      ancestry->parentCatalogIdB, phase + 1, scale);
  } else if (familyHasPartnerCatalog(*family)) {
    drawCatalogAvatar(display, rightX, parentY - 6 * scale,
                      family->character, phase + 1, scale);
  } else {
    drawUnknownAvatar(display, rightX, parentY - 6 * scale, scale);
  }
  drawSmallHeart(display, cx - 4 * scale, parentY + 8 * scale,
                 (phase & 1) != 0);
  display.drawLine(leftX + 7 * scale, parentY + 8 * scale,
                   cx - 2 * scale, childY - 5 * scale, EPD_BLACK);
  display.drawLine(rightX - 1 * scale, parentY + 8 * scale,
                   cx + 2 * scale, childY - 5 * scale, EPD_BLACK);
  const uint8_t babyCatalog =
      hasAncestry ? ancestry->babyCatalogId
                  : static_cast<uint8_t>((family->generation & 0x01) ? 0 : 1);
  drawCatalogAvatar(display, cx - 8, childY - 8,
                    babyCatalog, phase + 1, 1);
  const uint8_t pips = family->careScore > 5 ? 5 : family->careScore;
  for (uint8_t i = 0; i < 5; i++) {
    const int16_t px = cardX + 6 * scale + i * 4 * scale;
    const int16_t py = cardY + cardH - 5 * scale;
    if (i < pips) {
      display.fillRect(px, py, 2 * scale, 2 * scale, EPD_BLACK);
    } else {
      display.drawRect(px, py, 2 * scale, 2 * scale, EPD_BLACK);
    }
  }
}

void drawFamilyPage(EchoPetDisplayDevice& display, const ScreenResources& r,
                    const Snapshot& pet, const UiState& ui, uint8_t phase) {
  char buffer[48];
  drawCenteredLine(display, r, r.mainY + (r.compactText ? 2 : 8), "FAMILY",
                   1);
  const uint8_t i = ui.cursor % kFamilyHistoryCount;
  const FamilyRecord& f = pet.family[i];
  if (f.petId == 0) {
    drawFamilyAlbumCard(display, r, nullptr, nullptr, phase);
    drawCenteredLine(display, r, r.mainY + r.mainH - (r.compactText ? 10 : 18),
                     "NO RECORD", 1);
    return;
  }
  const FamilyAncestryRecord& ancestry = pet.familyAncestry[i];
  const FamilyAncestryRecord* ancestryPtr =
      familyAncestryValid(ancestry) ? &ancestry : nullptr;
  drawFamilyAlbumCard(display, r, &f, ancestryPtr, phase);
  if (ancestryPtr) {
    char partner[5];
    const unsigned parentA =
        static_cast<unsigned>(ancestry.parentCatalogIdA) + 1U;
    const unsigned baby = static_cast<unsigned>(ancestry.babyCatalogId) + 1U;
    if (familyAncestryHasPartnerCatalog(ancestry)) {
      snprintf(partner, sizeof(partner), "%u",
               static_cast<unsigned>(ancestry.parentCatalogIdB) + 1U);
    } else {
      snprintf(partner, sizeof(partner), "?");
    }
    if (r.compactText) {
      snprintf(buffer, sizeof(buffer), "G%u %u+%s>%u",
               f.generation, parentA, partner, baby);
    } else {
      snprintf(buffer, sizeof(buffer), "G%u %u+%s>%u T%u%s",
               f.generation, parentA, partner, baby,
               static_cast<unsigned>(ancestry.childGrowthTier),
               (ancestry.flags & kFamilyAncestryOyajitchiLineage) ? " OY"
                                                                  : "");
    }
  } else {
    snprintf(buffer, sizeof(buffer), "G%u %s C%u", f.generation,
             shortRouteLabel(static_cast<GrowthRoute>(familyRouteValue(f))),
             f.careScore);
  }
  drawCenteredLine(display, r, r.mainY + r.mainH - (r.compactText ? 10 : 18),
                   buffer, 1);
}

void drawSouvenirPage(EchoPetDisplayDevice& display, const ScreenResources& r,
                      const Snapshot& pet, const UiState& ui) {
  char buffer[40];
  drawPageTitle(display, r, "MEMORY");
  const uint8_t i = ui.cursor % kCatalogSouvenirCount;
  catalogSouvenirLabel(i, buffer, sizeof(buffer));
  if (r.compactText) {
    buffer[8] = '\0';
  }
  drawPageLine(display, r, 0, buffer);
  if (r.compactText) {
    snprintf(buffer, sizeof(buffer), "%s %u/%u",
             catalogBitTest(pet.souvenirOwned, i) ? "FND" : "---",
             static_cast<unsigned>(pet.souvenirOwnedCount),
             static_cast<unsigned>(kCatalogSouvenirCount));
  } else {
    snprintf(buffer, sizeof(buffer), "%s %u/%u",
             catalogBitTest(pet.souvenirOwned, i) ? "FOUND" : "----",
             static_cast<unsigned>(pet.souvenirOwnedCount),
             static_cast<unsigned>(kCatalogSouvenirCount));
  }
  drawPageLine(display, r, 1, buffer);
  const bool found = catalogBitTest(pet.souvenirOwned, i);
  const uint8_t iconScale = r.compactText ? 1 : 2;
  const int16_t iconSize = kCatalogVisualSide * iconScale;
  drawSouvenirIcon(display, r.mainX + (r.mainW - iconSize) / 2,
                   r.mainY + (r.compactText ? 37 : 58), i, found, ui.cursor,
                   iconScale);
}

void drawPasswordPage(EchoPetDisplayDevice& display, const ScreenResources& r,
                      const UiState& ui) {
  drawPageTitle(display, r, ui.secretCode ? "SECRET" : "PASSWORD");
  const uint8_t count = ui.secretCode ? 8 : 10;
  const uint8_t step = r.compactText ? (ui.secretCode ? 7 : 6) : 11;
  const int16_t x0 = r.mainX + (r.compactText ? 3 : 8);
  const int16_t y0 = r.mainY + (r.compactText ? 24 : 44);
  for (uint8_t i = 0; i < count; i++) {
    const int16_t x = x0 + i * step;
    char digit[2] = {
        ui.secretCode ? static_cast<char>('A' + (ui.entry[i] % 3))
                      : static_cast<char>('0' + (ui.entry[i] % 10)),
        '\0'};
    if (i == ui.page) {
      display.fillRect(x - 1, y0 - 2, r.compactText ? 7 : 10,
                       r.compactText ? 10 : 14, EPD_BLACK);
      display.setTextColor(EPD_WHITE);
      display.setCursor(x, y0);
      display.print(digit);
      display.setTextColor(EPD_BLACK);
    } else {
      drawText(display, x, y0, digit, r.compactText ? 1 : 2);
    }
    if (!r.compactText && !ui.secretCode && i == 4) {
      drawText(display, x + step, y0, "-", 2);
    }
  }
  if (!r.compactText) {
    drawCenteredLine(display, r, r.mainY + r.mainH - 22,
                     ui.secretCode ? "A/B/C CODE" : "A DIGIT  B OK", 1);
  }
}

char setupChar(uint8_t index) {
  index %= 36;
  if (index < 26) return static_cast<char>('A' + index);
  return static_cast<char>('0' + (index - 26));
}

void drawSetupField(EchoPetDisplayDevice& display, int16_t x, int16_t y,
                    const char* text, bool selected, uint8_t size) {
  const uint16_t w = textPixelWidth(text, size);
  const uint8_t h = size == 2 ? 15 : 9;
  if (selected) {
    display.fillRect(x - 2, y - 2, w + 4, h, EPD_BLACK);
    display.setTextColor(EPD_WHITE);
    display.setTextSize(size);
    display.setCursor(x, y);
    display.print(text);
    display.setTextColor(EPD_BLACK);
  } else {
    drawText(display, x, y, text, size);
  }
}

void drawClockSetPage(EchoPetDisplayDevice& display, const ScreenResources& r,
                      const UiState& ui, uint8_t phase) {
  char buffer[8];
  const uint8_t size = r.compactText ? 1 : 2;
  const int16_t y = r.mainY + (r.compactText ? 27 : 55);
  const int16_t hourX = r.mainX + (r.compactText ? 8 : 28);
  const int16_t colonX = r.mainX + (r.compactText ? 24 : 58);
  const int16_t minuteX = r.mainX + (r.compactText ? 32 : 72);

  drawPageTitle(display, r, "CLOCK");
  snprintf(buffer, sizeof(buffer), "%02u", ui.entry[0] % 24);
  drawSetupField(display, hourX, y, buffer, ui.cursor == 0, size);
  drawText(display, colonX, y, (phase & 1) ? ":" : " ", size);
  snprintf(buffer, sizeof(buffer), "%02u", ui.entry[1] % 60);
  drawSetupField(display, minuteX, y, buffer, ui.cursor == 1, size);
  drawCenteredLine(display, r, r.mainY + r.mainH - (r.compactText ? 10 : 18),
                   "PAUSED", 1);
}

void drawSetupName(EchoPetDisplayDevice& display, const ScreenResources& r,
                   const UiState& ui, const char* title) {
  char name[kNameChars + 1];
  for (uint8_t i = 0; i < kNameChars; i++) {
    name[i] = setupChar(ui.entry[i]);
  }
  name[kNameChars] = '\0';
  drawPageTitle(display, r, title);
  const uint8_t size = r.compactText ? 1 : 2;
  const int16_t step = r.compactText ? 10 : 18;
  const int16_t y = r.mainY + (r.compactText ? 28 : 55);
  const int16_t x0 =
      r.mainX + (r.mainW - static_cast<int16_t>(kNameChars * step)) / 2 + 2;
  char one[2] = {'A', '\0'};
  for (uint8_t i = 0; i < kNameChars; i++) {
    one[0] = name[i];
    drawSetupField(display, x0 + i * step, y, one, ui.cursor == i, size);
  }
}

void drawSetupPage(EchoPetDisplayDevice& display, const ScreenResources& r,
                   const UiState& ui, uint8_t phase) {
  char buffer[16];
  switch (ui.page) {
    case 0:
      drawPageTitle(display, r, "CLOCK");
      snprintf(buffer, sizeof(buffer), "%02u", ui.entry[0]);
      drawSetupField(display, r.mainX + (r.compactText ? 7 : 18),
                     r.mainY + (r.compactText ? 25 : 52), buffer,
                     ui.cursor == 0, r.compactText ? 1 : 2);
      drawText(display, r.mainX + (r.compactText ? 21 : 44),
               r.mainY + (r.compactText ? 25 : 52), "/",
               r.compactText ? 1 : 2);
      snprintf(buffer, sizeof(buffer), "%02u", ui.entry[1]);
      drawSetupField(display, r.mainX + (r.compactText ? 29 : 56),
                     r.mainY + (r.compactText ? 25 : 52), buffer,
                     ui.cursor == 1, r.compactText ? 1 : 2);
      snprintf(buffer, sizeof(buffer), "%02u", ui.entry[2]);
      drawSetupField(display, r.mainX + (r.compactText ? 7 : 18),
                     r.mainY + (r.compactText ? 42 : 82), buffer,
                     ui.cursor == 2, r.compactText ? 1 : 2);
      drawText(display, r.mainX + (r.compactText ? 21 : 44),
               r.mainY + (r.compactText ? 42 : 82), ":",
               r.compactText ? 1 : 2);
      snprintf(buffer, sizeof(buffer), "%02u", ui.entry[3]);
      drawSetupField(display, r.mainX + (r.compactText ? 29 : 56),
                     r.mainY + (r.compactText ? 42 : 82), buffer,
                     ui.cursor == 3, r.compactText ? 1 : 2);
      break;
    case 1:
      drawPageTitle(display, r, "BIRTHDAY");
      snprintf(buffer, sizeof(buffer), "%02u", ui.entry[0]);
      drawSetupField(display, r.mainX + (r.compactText ? 11 : 32),
                     r.mainY + (r.compactText ? 33 : 64), buffer,
                     ui.cursor == 0, r.compactText ? 1 : 2);
      drawText(display, r.mainX + (r.compactText ? 25 : 58),
               r.mainY + (r.compactText ? 33 : 64), "/",
               r.compactText ? 1 : 2);
      snprintf(buffer, sizeof(buffer), "%02u", ui.entry[1]);
      drawSetupField(display, r.mainX + (r.compactText ? 33 : 70),
                     r.mainY + (r.compactText ? 33 : 64), buffer,
                     ui.cursor == 1, r.compactText ? 1 : 2);
      break;
    case 2:
      drawSetupName(display, r, ui, "USER");
      break;
    case 3:
      drawPageTitle(display, r, "BOY/GIRL");
      drawCenteredLine(display, r, r.mainY + (r.compactText ? 32 : 62),
                       ui.entry[0] ? "GIRL" : "BOY", r.compactText ? 1 : 2);
      break;
    case 4:
      drawSetupName(display, r, ui, "NAME");
      break;
    default:
      drawPageTitle(display, r, "HATCH");
      switch (phase & 0x03) {
        case 1:
          drawSceneFrame(display, r, SpriteFrame::kEggCrack0, 0, 0);
          break;
        case 2:
          drawSceneFrame(display, r, SpriteFrame::kEggCrack1, 0, 0);
          break;
        case 3:
          drawSceneFrame(display, r, SpriteFrame::kEggHatch, 0, 0);
          break;
        default:
          drawSceneFrame(display, r, SpriteFrame::kEgg0, 0, 0);
          break;
      }
      if (!r.compactText) {
        drawCenteredLine(display, r, r.mainY + r.mainH - 18, "READY", 2);
      }
      break;
  }
}

void drawPresentPage(EchoPetDisplayDevice& display, const ScreenResources& r,
                     const Snapshot& pet, const UiState& ui) {
  const uint8_t c = ui.cursor;
  const int16_t cx = r.mainX + r.mainW / 2;
  const int16_t cy = r.mainY + r.mainH / 2;
  char buffer[40];
  const uint8_t scale = r.compactText ? 1 : 2;
  const int16_t iconHalf = r.compactText ? 8 : 16;

  display.drawRect(cx - (r.compactText ? 20 : 32),
                   cy - (r.compactText ? 15 : 24),
                   r.compactText ? 40 : 64, r.compactText ? 30 : 48,
                   EPD_BLACK);
  display.drawLine(cx, cy - (r.compactText ? 18 : 28), cx,
                   cy + (r.compactText ? 18 : 28), EPD_BLACK);
  display.drawLine(cx - (r.compactText ? 24 : 36), cy,
                   cx + (r.compactText ? 24 : 36), cy, EPD_BLACK);

  if (c < kCatalogItemCount) {
    const CatalogEntry entry = catalogEntry(c);
    catalogItemLabel(c, buffer, sizeof(buffer));
    drawCenteredLine(display, r, r.mainY + (r.compactText ? 2 : 8), buffer, 1);
    drawCatalogIcon(display, cx - iconHalf, cy - iconHalf, entry, c,
                    ui.cursor, scale);

    const bool owned = catalogBitTest(pet.catalogOwned, c);
    if (entry.kind == CatalogKind::kSouvenir) {
      snprintf(buffer, sizeof(buffer), "NO SEND %u/%u",
               static_cast<unsigned>(c + 1),
               static_cast<unsigned>(kCatalogItemCount));
    } else if (pet.catalogStock[c] > 0) {
      snprintf(buffer, sizeof(buffer), "x%u %u/%u",
               static_cast<unsigned>(pet.catalogStock[c]),
               static_cast<unsigned>(c + 1),
               static_cast<unsigned>(kCatalogItemCount));
    } else {
      snprintf(buffer, sizeof(buffer), "%s %u/%u", owned ? "OWN" : "----",
               static_cast<unsigned>(c + 1),
               static_cast<unsigned>(kCatalogItemCount));
    }
  } else {
    drawCenteredLine(display, r, r.mainY + (r.compactText ? 2 : 8), "PRESENT",
                     1);
    drawCenteredLine(display, r, cy - 4, "PTS", r.compactText ? 1 : 2);
    snprintf(buffer, sizeof(buffer), "POINTS %u", static_cast<unsigned>(pet.points));
  }
  drawCenteredLine(display, r, r.mainY + r.mainH - (r.compactText ? 10 : 18),
                   buffer, 1);
}

void drawVisitLinkPage(EchoPetDisplayDevice& display, const ScreenResources& r,
                       const Snapshot& pet, uint8_t phase) {
  (void)pet;
  const int16_t cx = r.mainX + r.mainW / 2;
  const int16_t cy = r.mainY + r.mainH / 2;
  display.drawRect(cx - 42, cy - 24, 28, 40, EPD_BLACK);
  display.drawRect(cx + 14, cy - 24, 28, 40, EPD_BLACK);
  drawSmallFace(display, cx - 36, cy - 12, phase);
  drawSmallFace(display, cx + 20, cy - 12, phase + 1);
  const uint8_t arc = (phase & 0x03) * 3;
  display.drawCircle(cx, cy - 8, 6 + arc, EPD_BLACK);
  display.drawCircle(cx, cy - 8, 12 + arc, EPD_BLACK);
  if (!r.compactText) {
    drawCenteredLine(display, r, r.mainY + r.mainH - 24, "B VISIT", 1);
  }
}

const char* linkKindLabel(uint8_t kind) {
  switch (kind) {
    case static_cast<uint8_t>(LinkKind::kVisit):
      return "VISIT";
    case static_cast<uint8_t>(LinkKind::kPresent):
      return "PRESENT";
    case static_cast<uint8_t>(LinkKind::kGame):
      return "GAME";
    case static_cast<uint8_t>(LinkKind::kLove):
      return "LOVE";
  }
  return "LINK";
}

const char* linkStatusLabel(uint8_t status) {
  switch (status) {
    case 1:
      return "SENT";
    case 2:
      return "RECEIVED";
    case 3:
      return "TIMEOUT";
    case 4:
      return "CANCEL";
    case 5:
      return "FAILED";
    default:
      return "WAIT";
  }
}

void drawLinkStandbyPage(EchoPetDisplayDevice& display, const ScreenResources& r,
                         const UiState& ui, uint8_t phase) {
  const int16_t cx = r.mainX + r.mainW / 2;
  const int16_t cy = r.mainY + r.mainH / 2;
  drawCenteredLine(display, r, r.mainY + (r.compactText ? 3 : 9),
                   linkKindLabel(ui.entry[0]), r.compactText ? 1 : 2);
  drawSceneFrame(display, r,
                 (phase & 1) ? SpriteFrame::kLinkReceive
                             : SpriteFrame::kLinkSend,
                 0, 0);
  const uint8_t arc = (phase & 0x03) * 4;
  display.drawCircle(cx, cy - 6, 7 + arc, EPD_BLACK);
  display.drawCircle(cx, cy - 6, 13 + arc, EPD_BLACK);
  drawCenteredLine(display, r, r.mainY + r.mainH - (r.compactText ? 10 : 18),
                   "B SEND  C STOP", 1);
}

const char* linkResultLabel(uint8_t kind, uint8_t status, const Snapshot& pet) {
  if (status == 2) {
    if (kind == static_cast<uint8_t>(LinkKind::kLove)) {
      return pet.stage == Stage::kParentCare ? "BABY" : "PARTNER";
    }
    if (kind == static_cast<uint8_t>(LinkKind::kPresent)) return "GIFT";
    if (kind == static_cast<uint8_t>(LinkKind::kGame)) return "RESULT";
    return "VISIT";
  }
  if (status == 5 && kind == static_cast<uint8_t>(LinkKind::kLove)) {
    return "REJECT";
  }
  return linkStatusLabel(status);
}

void drawGiftBox(EchoPetDisplayDevice& display, int16_t cx, int16_t cy,
                 bool open) {
  display.drawRect(cx - 14, cy - 6, 28, 20, EPD_BLACK);
  display.fillRect(cx - 2, cy - 6, 4, 20, EPD_BLACK);
  display.fillRect(cx - 16, cy, 32, 3, EPD_BLACK);
  if (open) {
    display.drawLine(cx - 14, cy - 6, cx - 24, cy - 16, EPD_BLACK);
    display.drawLine(cx + 14, cy - 6, cx + 24, cy - 16, EPD_BLACK);
  } else {
    display.drawRect(cx - 10, cy - 14, 20, 8, EPD_BLACK);
  }
}

void drawLinkResultPage(EchoPetDisplayDevice& display, const ScreenResources& r,
                        const Snapshot& pet, const UiState& ui,
                        uint8_t phase) {
  const int16_t cx = r.mainX + r.mainW / 2;
  const int16_t cy = r.mainY + r.mainH / 2;
  const uint8_t kind = ui.entry[0];
  const uint8_t status = ui.entry[2];
  const bool ok = status == 1 || status == 2;
  drawCenteredLine(display, r, r.mainY + (r.compactText ? 3 : 9),
                   linkKindLabel(kind), r.compactText ? 1 : 2);

  if (kind == static_cast<uint8_t>(LinkKind::kPresent) && ok) {
    drawGiftBox(display, cx, cy - (r.compactText ? 2 : 6), status == 2);
    drawSceneFrame(display, r, status == 2 ? SpriteFrame::kJoy
                                           : SpriteFrame::kLinkSend,
                   r.compactText ? 8 : 13, 0);
  } else if (kind == static_cast<uint8_t>(LinkKind::kGame) && ok) {
    drawSmallFace(display, cx - (r.compactText ? 30 : 42),
                  cy - (r.compactText ? 8 : 16), phase);
    drawSmallFace(display, cx + (r.compactText ? 14 : 26),
                  cy - (r.compactText ? 8 : 16), phase + 1);
    display.drawRect(cx - 9, cy + (r.compactText ? 10 : 20), 18, 12,
                     EPD_BLACK);
    display.fillRect(cx - 5, cy + (r.compactText ? 13 : 23), 10, 5,
                     EPD_BLACK);
  } else if (kind == static_cast<uint8_t>(LinkKind::kLove) && ok) {
    if (status == 2 && pet.stage == Stage::kParentCare) {
      drawSmallFace(display, cx - (r.compactText ? 27 : 40),
                    cy - (r.compactText ? 10 : 18), phase);
      drawSmallFace(display, cx + (r.compactText ? 11 : 24),
                    cy - (r.compactText ? 10 : 18), phase + 1);
      drawSmallHeart(display, cx - 5, cy + (r.compactText ? 4 : 10), true);
      display.drawLine(cx - 13, cy + (r.compactText ? 12 : 22), cx,
                       cy + (r.compactText ? 22 : 36), EPD_BLACK);
      display.drawLine(cx + 13, cy + (r.compactText ? 12 : 22), cx,
                       cy + (r.compactText ? 22 : 36), EPD_BLACK);
      drawSceneFrame(display, r, SpriteFrame::kFriend, r.compactText ? 9 : 16,
                     1);
    } else {
      drawSceneFrame(display, r,
                     (phase & 1) ? SpriteFrame::kLinkReceive
                                 : SpriteFrame::kLinkSend,
                     0, 0);
      drawSmallHeart(display, cx + (r.compactText ? 18 : 31),
                     cy + (r.compactText ? 9 : 17), true);
      drawSmallHeart(display, cx - (r.compactText ? 27 : 39),
                     cy + (r.compactText ? 8 : 16), (phase & 1) != 0);
    }
  } else if (ok) {
    drawSceneFrame(display, r,
                   (phase & 1) ? SpriteFrame::kLinkReceive
                               : SpriteFrame::kLinkSend,
                   0, 0);
    drawSmallHeart(display, cx + (r.compactText ? 18 : 31),
                   cy + (r.compactText ? 9 : 17), (phase & 1) != 0);
  } else {
    drawSceneFrame(display, r, SpriteFrame::kGameLose, 0, 0);
    display.drawCircle(cx, cy - (r.compactText ? 12 : 22),
                       r.compactText ? 8 : 13, EPD_BLACK);
    display.drawLine(cx - 6, cy - (r.compactText ? 18 : 29), cx + 6,
                     cy - (r.compactText ? 8 : 16), EPD_BLACK);
    display.drawLine(cx + 6, cy - (r.compactText ? 18 : 29), cx - 6,
                     cy - (r.compactText ? 8 : 16), EPD_BLACK);
  }
  drawCenteredLine(display, r, r.mainY + r.mainH - (r.compactText ? 10 : 18),
                   linkResultLabel(kind, status, pet), r.compactText ? 1 : 2);
}

void drawPointPage(EchoPetDisplayDevice& display, const ScreenResources& r,
                   const Snapshot& pet, uint8_t phase) {
  char buffer[32];
  const uint8_t scale = r.compactText ? 1 : 2;
  const int16_t cx = r.mainX + r.mainW / 2;
  const int16_t top = r.mainY + (r.compactText ? 33 : 52);
  const int16_t palaceW = 24 * scale;
  const int16_t palaceH = 14 * scale;
  const uint16_t donationGoal = 1000;
  const uint16_t progress =
      pet.donations >= donationGoal ? donationGoal : pet.donations;
  const int16_t barX = r.mainX + (r.compactText ? 6 : 16);
  const int16_t barY = r.mainY + r.mainH - (r.compactText ? 14 : 24);
  const int16_t barW = r.mainW - (r.compactText ? 12 : 32);
  const int16_t barFill =
      static_cast<int16_t>(((barW - 2) * static_cast<uint32_t>(progress)) /
                           donationGoal);
  drawPageTitle(display, r, "POINT");
  snprintf(buffer, sizeof(buffer), "%u GOTCHI", pet.points);
  drawPageLine(display, r, 0, buffer);
  snprintf(buffer, sizeof(buffer), "D %u/%u", pet.donations, donationGoal);
  drawPageLine(display, r, 1, buffer);

  display.drawRect(cx - palaceW / 2, top + 4 * scale, palaceW, palaceH,
                   EPD_BLACK);
  display.drawTriangle(cx - 11 * scale, top + 4 * scale, cx - 6 * scale,
                       top - 2 * scale, cx - 1 * scale, top + 4 * scale,
                       EPD_BLACK);
  display.drawTriangle(cx - 1 * scale, top + 4 * scale, cx + 5 * scale,
                       top - 3 * scale, cx + 11 * scale, top + 4 * scale,
                       EPD_BLACK);
  display.fillRect(cx - 4 * scale, top + 12 * scale, 8 * scale, 6 * scale,
                   EPD_BLACK);
  drawSmallFace(display, cx - 4 * scale, top + 6 * scale,
                static_cast<uint8_t>(pet.donations / 100));

  const uint8_t coinCount =
      pet.points >= 500 ? 5 : static_cast<uint8_t>((pet.points + 99) / 100);
  for (uint8_t i = 0; i < coinCount; i++) {
    const int16_t coinX = r.mainX + 6 * scale + i * 3 * scale;
    const int16_t coinY = top + 17 * scale - i * scale;
    display.drawCircle(coinX, coinY, 2 * scale, EPD_BLACK);
  }
  if (pet.notice == Notice::kDonate) {
    drawSparkle(display, cx + 14 * scale, top + 2 * scale, phase);
  } else if (pet.notice == Notice::kNoPoints) {
    display.drawLine(r.mainX + 4 * scale, top + 20 * scale,
                     r.mainX + 23 * scale, top + 2 * scale, EPD_BLACK);
  } else if (pet.donations >= donationGoal) {
    drawSmallHeart(display, cx + 13 * scale, top + 1 * scale, true);
  }

  display.drawRect(barX, barY, barW, 5, EPD_BLACK);
  if (barFill > 0) {
    display.fillRect(barX + 1, barY + 1, barFill, 3, EPD_BLACK);
  }
  drawCenteredLine(display, r, r.mainY + r.mainH - (r.compactText ? 8 : 14),
                   "B DONATE 100", 1);
}

void drawActionStateLabel(EchoPetDisplayDevice& display, const ScreenResources& r,
                          const Snapshot& pet, const char* fallback) {
  if (r.compactText) {
    return;
  }
  const char* text = shortNoticeLabel(pet);
  if (!text || !text[0]) {
    text = fallback;
  }
  if (text && text[0]) {
    drawCenteredLine(display, r, r.mainY + (r.compactText ? 2 : 6), text, 1);
  }
}

constexpr uint8_t kToiletCleanupFrames = 44;
constexpr uint32_t kToiletPetRows[] = {
    0b0000000111111000000000, 0b0000011000000110000000,
    0b0000100000000001000000, 0b0001000000000000100000,
    0b1111000000000000110000, 0b0001000010010000110000,
    0b1111000000000000110000, 0b0001000000000000100000,
    0b0001100001100001100000, 0b0000110001100011000000,
    0b0000011111111100000000, 0b0000001000010000000000,
    0b0000010000001000000000, 0b0000100000000100000000,
    0b0000000000000000000000, 0b0000000000000000000000,
};
constexpr uint32_t kToiletPoopRows[] = {
    0b0000100000, 0b0001110000, 0b0011111000, 0b0111111100,
    0b1111111110, 0b1111111111, 0b0111111110, 0b0011111100,
    0b0000000000, 0b0000000000,
};

constexpr uint32_t kToiletCleanupNativeStartRows[30] PROGMEM = {
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x0003E000, 0x00041000, 0x003A2800, 0x00400400,
    0x00380400, 0x00400400, 0x00380400, 0x00085280, 0x00085244,
    0x00082292, 0x00040434, 0x0002E858, 0x0002A8BC, 0x000110FC,
};

bool insideMainScene(const ScreenResources& r, int16_t x, int16_t y) {
  if (r.compactText) {
    const V3Canvas canvas = v3CanvasFor(r);
    return x >= canvas.x &&
           x < canvas.x + static_cast<int16_t>(canvas.screenW) &&
           y >= canvas.y &&
           y < canvas.y + static_cast<int16_t>(canvas.screenH);
  }
  return x > r.mainX && x < r.mainX + static_cast<int16_t>(r.mainW) - 1 &&
         y > r.mainY && y < r.mainY + static_cast<int16_t>(r.mainH) - 1;
}

void drawPixelInMain(EchoPetDisplayDevice& display, const ScreenResources& r,
                     int16_t x, int16_t y, uint16_t color) {
  if (insideMainScene(r, x, y)) {
    display.drawPixel(x, y, color);
  }
}

void fillRectInMain(EchoPetDisplayDevice& display, const ScreenResources& r,
                    int16_t x, int16_t y, int16_t w, int16_t h,
                    uint16_t color) {
  for (int16_t yy = 0; yy < h; yy++) {
    for (int16_t xx = 0; xx < w; xx++) {
      drawPixelInMain(display, r, x + xx, y + yy, color);
    }
  }
}

void drawScaledPixelBefore(EchoPetDisplayDevice& display,
                           const ScreenResources& r, int16_t x, int16_t y,
                           uint8_t scale, int16_t clipRightX) {
  for (uint8_t yy = 0; yy < scale; yy++) {
    for (uint8_t xx = 0; xx < scale; xx++) {
      const int16_t px = x + xx;
      if (px >= clipRightX) continue;
      drawPixelInMain(display, r, px, y + yy, EPD_BLACK);
    }
  }
}

void drawMonoBitmapBefore(EchoPetDisplayDevice& display,
                          const ScreenResources& r, int16_t x, int16_t y,
                          const uint32_t* rows, uint8_t width, uint8_t height,
                          uint8_t scale, int16_t clipRightX) {
  for (uint8_t row = 0; row < height; row++) {
    const uint32_t bits = rows[row];
    for (uint8_t col = 0; col < width; col++) {
      if ((bits & (1UL << (width - 1 - col))) == 0) {
        continue;
      }
      drawScaledPixelBefore(display, r, x + col * scale, y + row * scale,
                            scale, clipRightX);
    }
  }
}

void drawV3NativeRowsBefore(EchoPetDisplayDevice& display,
                            const ScreenResources& r, const uint32_t* rows,
                            int16_t clipRightX) {
  const V3Canvas canvas = v3CanvasFor(r);
  for (uint8_t row = 0; row < canvas.nativeH; row++) {
    const uint32_t bits = pgm_read_dword(&rows[row]);
    for (uint8_t col = 0; col < canvas.nativeW; col++) {
      if ((bits & (1UL << (canvas.nativeW - 1 - col))) == 0) {
        continue;
      }
      drawScaledPixelBefore(display, r,
                            canvas.x + col * canvas.scale,
                            canvas.y + row * canvas.scale, canvas.scale,
                            clipRightX);
    }
  }
}

void drawToiletPetBefore(EchoPetDisplayDevice& display,
                         const ScreenResources& r, int16_t x, int16_t y,
                         uint8_t scale, int16_t clipRightX) {
  drawMonoBitmapBefore(display, r, x, y, kToiletPetRows, 22, 16, scale,
                       clipRightX);
}

void drawToiletPoopBefore(EchoPetDisplayDevice& display,
                          const ScreenResources& r, int16_t x, int16_t y,
                          uint8_t scale, int16_t clipRightX, uint8_t phase) {
  drawMonoBitmapBefore(display, r, x, y, kToiletPoopRows, 10, 10, scale,
                       clipRightX);
  const int16_t wiggle = (phase & 0x01) ? scale : 0;
  drawScaledPixelBefore(display, r, x + 2 * scale + wiggle, y - 5 * scale,
                        scale, clipRightX);
  drawScaledPixelBefore(display, r, x + 3 * scale + wiggle, y - 4 * scale,
                        scale, clipRightX);
  drawScaledPixelBefore(display, r, x + 2 * scale + wiggle, y - 3 * scale,
                        scale, clipRightX);
  drawScaledPixelBefore(display, r, x + 8 * scale - wiggle, y - 6 * scale,
                        scale, clipRightX);
  drawScaledPixelBefore(display, r, x + 7 * scale - wiggle, y - 5 * scale,
                        scale, clipRightX);
  drawScaledPixelBefore(display, r, x + 8 * scale - wiggle, y - 4 * scale,
                        scale, clipRightX);
}

void drawToiletCleanupObjects(EchoPetDisplayDevice& display,
                              const ScreenResources& r, const Snapshot& pet,
                              uint8_t cleanupMessCount,
                              int16_t clipRightX, uint8_t phase) {
  if (r.compactText) {
    drawV3NativeRowsBefore(display, r, kToiletCleanupNativeStartRows,
                           clipRightX);
    return;
  }
  const uint8_t s = r.compactText ? 1 : 2;
  const int16_t petX = r.mainX + (r.compactText ? 7 : 16);
  const int16_t petY =
      r.mainY + r.mainH - 16 * s - (r.compactText ? 12 : 22);
  drawToiletPetBefore(display, r, petX, petY, s, clipRightX);

  const uint8_t messes = cleanupMessCount ? cleanupMessCount
                         : pet.messCount ? pet.messCount
                                         : 1;
  const int16_t poopBaseX =
      r.mainX + r.mainW - (r.compactText ? 24 : 48);
  const int16_t poopBaseY =
      r.mainY + r.mainH - 10 * s - (r.compactText ? 13 : 24);
  for (uint8_t i = 0; i < messes && i < 2; i++) {
    drawToiletPoopBefore(display, r, poopBaseX - i * 8 * s,
                         poopBaseY + i * 2 * s, s, clipRightX, phase + i);
  }
}

void drawToiletCleanupDottedWall(EchoPetDisplayDevice& display,
                                 const ScreenResources& r, int16_t x,
                                 uint8_t phase) {
  const V3Canvas canvas = v3CanvasFor(r);
  const uint8_t s = r.compactText ? canvas.scale : 2;
  const int16_t top = r.compactText ? canvas.y : r.mainY + 1;
  const int16_t bottom =
      r.compactText ? canvas.y + canvas.screenH : r.mainY + r.mainH - 1;
  for (int16_t y = top + ((phase & 0x01) ? 2 * s : 0); y < bottom;
       y += 7 * s) {
    fillRectInMain(display, r, x, y, s, 2 * s, EPD_BLACK);
  }
}

void drawToiletCleanupWall(EchoPetDisplayDevice& display,
                           const ScreenResources& r, int16_t x,
                           int16_t width, uint8_t phase) {
  const V3Canvas canvas = v3CanvasFor(r);
  const int16_t cell = r.compactText ? canvas.scale : 4;
  const int16_t top = r.compactText ? canvas.y : r.mainY + 1;
  const int16_t bottom =
      r.compactText ? canvas.y + canvas.screenH : r.mainY + r.mainH - 1;
  for (int16_t yy = top; yy < bottom; yy += cell) {
    for (int16_t xx = x; xx < x + width; xx += cell) {
      const int16_t gx = static_cast<int16_t>((xx - x) / cell);
      const int16_t gy = static_cast<int16_t>((yy - top) / cell);
      if (((gx + gy + phase) & 0x01) != 0) {
        continue;
      }
      fillRectInMain(display, r, xx, yy, cell, cell, EPD_BLACK);
    }
  }
}

void drawToiletScene(EchoPetDisplayDevice& display, const ScreenResources& r,
                     const Snapshot& pet, const UiState& ui, uint8_t phase) {
  const uint8_t cleanupMessCount = ui.entry[0];
  const bool hasMess = cleanupMessCount != 0 || pet.messCount != 0 ||
                       pet.notice == Notice::kClean;
  drawActionStateLabel(display, r, pet, hasMess ? "CLEAN" : "OK");

  if (!hasMess) {
    drawSceneFrame(display, r, SpriteFrame::kToiletNoMess,
                   r.compactText ? -6 : -8, 0);
    return;
  }

  const uint8_t stage =
      phase < kToiletCleanupFrames ? phase : kToiletCleanupFrames - 1;
  const V3Canvas canvas = v3CanvasFor(r);
  const int16_t mainRight =
      r.compactText ? canvas.x + static_cast<int16_t>(canvas.screenW)
                    : r.mainX + static_cast<int16_t>(r.mainW) - 1;
  if (stage == 0) {
    drawToiletCleanupObjects(display, r, pet, cleanupMessCount, mainRight,
                             phase);
    return;
  }

  const uint8_t s = r.compactText ? canvas.scale : 2;
  const int16_t wallW = r.compactText ? 8 : 24;
  if (stage == 1) {
    drawToiletCleanupObjects(display, r, pet, cleanupMessCount, mainRight,
                             phase);
    drawToiletCleanupDottedWall(display, r, mainRight - 2 * s, phase);
    return;
  }

  const uint8_t wallStage = stage - 2;
  constexpr uint8_t kWallFrames = kToiletCleanupFrames - 6;
  const uint8_t capped =
      wallStage < kWallFrames ? wallStage : kWallFrames;
  const int16_t travel =
      (r.compactText ? static_cast<int16_t>(canvas.screenW)
                     : static_cast<int16_t>(r.mainW)) +
      wallW + 2 * s;
  const int16_t progress =
      static_cast<int16_t>((static_cast<uint32_t>(travel) * capped) /
                           kWallFrames);
  const int16_t wallX =
      (r.compactText ? canvas.x + static_cast<int16_t>(canvas.screenW)
                     : r.mainX + static_cast<int16_t>(r.mainW)) -
      progress;
  drawToiletCleanupObjects(display, r, pet, cleanupMessCount, wallX, phase);
  drawToiletCleanupWall(display, r, wallX, wallW, phase);
}

constexpr uint8_t kSpriteProofFrameCount = kSpriteFrameCount;

const char* spriteProofLabel(uint8_t index) {
  switch (static_cast<SpriteFrame>(index % kSpriteProofFrameCount)) {
    case SpriteFrame::kEgg0:
      return "EGG0";
    case SpriteFrame::kEgg1:
      return "EGG1";
    case SpriteFrame::kEggCrack0:
      return "EGG CR0";
    case SpriteFrame::kEggCrack1:
      return "EGG CR1";
    case SpriteFrame::kEggHatch:
      return "EGG HATCH";
    case SpriteFrame::kBaby0:
      return "BABY0";
    case SpriteFrame::kBaby1:
      return "BABY1";
    case SpriteFrame::kChild0:
      return "CHILD0";
    case SpriteFrame::kChild1:
      return "CHILD1";
    case SpriteFrame::kTeen0:
      return "TEEN0";
    case SpriteFrame::kTeen1:
      return "TEEN1";
    case SpriteFrame::kAdult0:
      return "ADULT0";
    case SpriteFrame::kAdult1:
      return "ADULT1";
    case SpriteFrame::kElder0:
      return "ELDER0";
    case SpriteFrame::kElder1:
      return "ELDER1";
    case SpriteFrame::kSleep:
      return "SLEEP";
    case SpriteFrame::kSick:
      return "SICK";
    case SpriteFrame::kJoy:
      return "JOY";
    case SpriteFrame::kSad:
      return "SAD";
    case SpriteFrame::kFriend:
      return "FRIEND";
    case SpriteFrame::kAthlete:
      return "ATHLETE";
    case SpriteFrame::kScholar:
      return "SCHOLAR";
    case SpriteFrame::kDream:
      return "DREAM";
    case SpriteFrame::kRascal:
      return "RASCAL";
    case SpriteFrame::kEatMeal0:
      return "MEAL0";
    case SpriteFrame::kEatMeal1:
      return "MEAL1";
    case SpriteFrame::kEatSnack0:
      return "SNACK0";
    case SpriteFrame::kEatSnack1:
      return "SNACK1";
    case SpriteFrame::kFoodCrumbs:
      return "FOOD CRMB";
    case SpriteFrame::kFoodRefuse:
      return "FOOD NO";
    case SpriteFrame::kFoodDone:
      return "FOOD DONE";
    case SpriteFrame::kToilet0:
      return "TOILET0";
    case SpriteFrame::kToilet1:
      return "TOILET1";
    case SpriteFrame::kToiletMess:
      return "MESS";
    case SpriteFrame::kToiletCleanupWall0:
      return "WALL0";
    case SpriteFrame::kToiletCleanupWall1:
      return "WALL1";
    case SpriteFrame::kToiletDone:
      return "TOIL DONE";
    case SpriteFrame::kToiletNoMess:
      return "NO MESS";
    case SpriteFrame::kMedicine0:
      return "MED0";
    case SpriteFrame::kMedicine1:
      return "MED1";
    case SpriteFrame::kMedicineSickSkull:
      return "MED SKULL";
    case SpriteFrame::kMedicineSickTooth:
      return "MED TOOTH";
    case SpriteFrame::kMedicineDose0:
      return "DOSE0";
    case SpriteFrame::kMedicineDose1:
      return "DOSE1";
    case SpriteFrame::kMedicineRecover:
      return "RECOVER";
    case SpriteFrame::kMedicineRefuse:
      return "MED NO";
    case SpriteFrame::kLightsOn:
      return "LIGHT ON";
    case SpriteFrame::kLightsOff:
      return "LIGHTS";
    case SpriteFrame::kLightsSelectorOn:
      return "LIGHT SEL ON";
    case SpriteFrame::kLightsSelectorOff:
      return "LIGHT SEL OFF";
    case SpriteFrame::kLightsWake:
      return "LIGHT WAKE";
    case SpriteFrame::kLightsInvalid:
      return "LIGHT NO";
    case SpriteFrame::kDisciplineTimeout:
      return "TIMEOUT";
    case SpriteFrame::kDisciplinePraise:
      return "PRAISE";
    case SpriteFrame::kDisciplineInvalid:
      return "DISC NO";
    case SpriteFrame::kAttentionCall:
      return "ATTN";
    case SpriteFrame::kAttentionMissed:
      return "MISS CALL";
    case SpriteFrame::kItemPlay:
      return "ITEM";
    case SpriteFrame::kShopBooth:
      return "SHOP BOOTH";
    case SpriteFrame::kShopkeeperIdle:
      return "SHOP IDLE";
    case SpriteFrame::kShopkeeperSurprise:
      return "SHOP WOW";
    case SpriteFrame::kShopkeeperHappy:
      return "SHOP OK";
    case SpriteFrame::kShopItemPreview:
      return "SHOP ITEM";
    case SpriteFrame::kShopBuyOk:
      return "BUY OK";
    case SpriteFrame::kShopBuyNoMoney:
      return "NO POINTS";
    case SpriteFrame::kShopBuyFull:
      return "BUY FULL";
    case SpriteFrame::kShopSoldOut:
      return "SOLD OUT";
    case SpriteFrame::kLinkSend:
      return "LINKTX";
    case SpriteFrame::kLinkReceive:
      return "LINKRX";
    case SpriteFrame::kLovePartner:
      return "PARTNER";
    case SpriteFrame::kLoveBaby:
      return "BABY";
    case SpriteFrame::kParentDepart:
      return "PARENT";
    case SpriteFrame::kGameWin:
      return "WIN";
    case SpriteFrame::kGameLose:
      return "LOSE";
    case SpriteFrame::kGameGetNote:
      return "GET NOTE";
    case SpriteFrame::kGameGetBad:
      return "GET BAD";
    case SpriteFrame::kGameGetCatch:
      return "GET OK";
    case SpriteFrame::kGameGetMiss:
      return "GET MISS";
    case SpriteFrame::kGameBumpMeter:
      return "BUMP MTR";
    case SpriteFrame::kGameBumpPush:
      return "BUMP PUSH";
    case SpriteFrame::kGameBumpFall:
      return "BUMP FALL";
    case SpriteFrame::kGameFlagLeft:
      return "FLAG L";
    case SpriteFrame::kGameFlagRight:
      return "FLAG R";
    case SpriteFrame::kGameFlagBoth:
      return "FLAG BOTH";
    case SpriteFrame::kGameFlagGood:
      return "FLAG OK";
    case SpriteFrame::kGameFlagMiss:
      return "FLAG MISS";
    case SpriteFrame::kGameHeadingBall:
      return "HEAD BALL";
    case SpriteFrame::kGameHeadingHit:
      return "HEAD HIT";
    case SpriteFrame::kGameHeadingMiss:
      return "HEAD MISS";
    case SpriteFrame::kGameMemoryReveal:
      return "MEM SHOW";
    case SpriteFrame::kGameMemoryCursor:
      return "MEM CUR";
    case SpriteFrame::kGameMemoryGood:
      return "MEM OK";
    case SpriteFrame::kGameMemoryWrong:
      return "MEM WRONG";
    case SpriteFrame::kGameSprintRunner0:
      return "RUN0";
    case SpriteFrame::kGameSprintRunner1:
      return "RUN1";
    case SpriteFrame::kGameSprintFinish:
      return "RUN FIN";
    case SpriteFrame::kGameHoopsHoop:
      return "HOOP";
    case SpriteFrame::kGameHoopsShoot:
      return "HOOP SHOT";
    case SpriteFrame::kGameHoopsMade:
      return "HOOP OK";
    case SpriteFrame::kGameHoopsMiss:
      return "HOOP MISS";
    case SpriteFrame::kPassed:
      return "PASSED";
  }
  return "FRAME";
}

SpriteFrame spriteProofFrame(uint8_t index) {
  return static_cast<SpriteFrame>(index % kSpriteProofFrameCount);
}

void drawSpriteProofPage(EchoPetDisplayDevice& display, const ScreenResources& r,
                         const UiState& ui) {
  const uint8_t index = ui.cursor % kSpriteProofFrameCount;
  const SpriteFrame frame = spriteProofFrame(index);
  const SpriteFrameInfo info = spriteFrameInfo(frame);
  char label[24];
  snprintf(label, sizeof(label), "%02u %s", static_cast<unsigned>(index),
           spriteProofLabel(index));
  drawCenteredLine(display, r, r.mainY + (r.compactText ? 3 : 8), label, 1);
  if (info.width == 0 || info.height == 0) {
    drawCenteredLine(display, r, r.mainY + r.mainH / 2, "EMPTY", 1);
    return;
  }

  const uint8_t scale = r.compactText ? 1 : 2;
  const int16_t width = info.width * scale;
  const int16_t height = info.height * scale;
  const int16_t x = r.mainX + (r.mainW - width) / 2;
  const int16_t y = r.mainY + (r.mainH - height) / 2 + (r.compactText ? 5 : 8);
  display.drawRect(x - 2, y - 2, width + 4, height + 4, EPD_BLACK);
  drawSpriteFrame(display, frame, x, y, scale);
}

void drawMedicineScene(EchoPetDisplayDevice& display, const ScreenResources& r,
                       const Snapshot& pet, uint8_t phase) {
  auto medicineFrame = [&]() {
    const bool stillSick = pet.sickness || pet.toothache;
    if (pet.notice == Notice::kNoMedicine) {
      return SpriteFrame::kMedicineRefuse;
    }
    if (pet.notice == Notice::kMedicine && !stillSick) {
      switch (phase & 0x03) {
        case 0:
          return SpriteFrame::kMedicineDose0;
        case 1:
          return SpriteFrame::kMedicineDose1;
        case 2:
          return SpriteFrame::kMedicineRecover;
        default:
          return SpriteFrame::kMedicine1;
      }
    }
    if (pet.notice == Notice::kNeedMoreMedicine || stillSick) {
      switch (phase & 0x03) {
        case 0:
          return pet.toothache ? SpriteFrame::kMedicineSickTooth
                               : SpriteFrame::kMedicineSickSkull;
        case 1:
          return SpriteFrame::kMedicineDose0;
        case 2:
          return SpriteFrame::kMedicineDose1;
        default:
          return pet.notice == Notice::kNeedMoreMedicine
                     ? SpriteFrame::kMedicine0
                     : SpriteFrame::kMedicineRecover;
      }
    }
    return SpriteFrame::kMedicineRefuse;
  };

  auto medicineLabel = [&]() -> const char* {
    if (pet.notice == Notice::kNoMedicine) return "NO MED";
    if (pet.notice == Notice::kNeedMoreMedicine) return "MORE";
    if (pet.notice == Notice::kMedicine && !(pet.sickness || pet.toothache)) {
      return "CURED";
    }
    if (pet.toothache) return "TOOTH";
    if (pet.sickness) return "SICK";
    return "NO MED";
  };

  auto drawMedicineBottle = [&](int16_t x, int16_t y, uint8_t s,
                                bool crossed) {
    display.drawRect(x + 5 * s, y, 10 * s, 5 * s, EPD_BLACK);
    display.drawRect(x + 2 * s, y + 5 * s, 16 * s, 20 * s, EPD_BLACK);
    display.drawLine(x + 8 * s, y + 11 * s, x + 12 * s, y + 11 * s,
                     EPD_BLACK);
    display.drawLine(x + 10 * s, y + 9 * s, x + 10 * s, y + 13 * s,
                     EPD_BLACK);
    if ((phase & 0x01) != 0 && !crossed) {
      display.fillRect(x + 3 * s, y + 17 * s, 14 * s, 3 * s, EPD_BLACK);
    }
    if (crossed) {
      display.drawLine(x, y + 2 * s, x + 20 * s, y + 28 * s, EPD_BLACK);
      display.drawLine(x + 20 * s, y + 2 * s, x, y + 28 * s, EPD_BLACK);
    }
  };

  auto drawMedicineBadge = [&](int16_t x, int16_t y, uint8_t s) {
    const bool cured =
        pet.notice == Notice::kMedicine && !(pet.sickness || pet.toothache);
    const bool refused = pet.notice == Notice::kNoMedicine;
    display.drawRect(x, y, 18 * s, 18 * s, EPD_BLACK);
    if (refused) {
      display.drawLine(x + 3 * s, y + 3 * s, x + 15 * s, y + 15 * s,
                       EPD_BLACK);
      display.drawLine(x + 15 * s, y + 3 * s, x + 3 * s, y + 15 * s,
                       EPD_BLACK);
      return;
    }
    if (cured) {
      drawSmallHeart(display, x + 5 * s, y + 5 * s, true);
      return;
    }
    if (pet.toothache) {
      display.drawLine(x + 6 * s, y + 4 * s, x + 12 * s, y + 4 * s,
                       EPD_BLACK);
      display.drawLine(x + 4 * s, y + 6 * s, x + 4 * s, y + 14 * s,
                       EPD_BLACK);
      display.drawLine(x + 14 * s, y + 6 * s, x + 14 * s, y + 14 * s,
                       EPD_BLACK);
      display.drawLine(x + 4 * s, y + 14 * s, x + 8 * s, y + 10 * s,
                       EPD_BLACK);
      display.drawLine(x + 14 * s, y + 14 * s, x + 10 * s, y + 10 * s,
                       EPD_BLACK);
      display.drawPixel(x + 8 * s, y + 8 * s, EPD_BLACK);
      return;
    }
    display.drawCircle(x + 9 * s, y + 8 * s, 6 * s, EPD_BLACK);
    display.fillRect(x + 6 * s, y + 7 * s, 2 * s, 2 * s, EPD_BLACK);
    display.fillRect(x + 11 * s, y + 7 * s, 2 * s, 2 * s, EPD_BLACK);
    display.drawRect(x + 7 * s, y + 12 * s, 5 * s, 3 * s, EPD_BLACK);
  };

  auto drawDoseTrail = [&](int16_t x, int16_t y, uint8_t s) {
    const uint8_t step = phase & 0x03;
    for (uint8_t i = 0; i < 3; i++) {
      const int16_t px = x + i * 7 * s;
      if (i <= step && pet.notice != Notice::kNoMedicine) {
        display.fillRoundRect(px, y + (i & 1) * s, 5 * s, 3 * s, s,
                              EPD_BLACK);
      } else {
        display.drawRoundRect(px, y + (i & 1) * s, 5 * s, 3 * s, s,
                              EPD_BLACK);
      }
    }
  };

  const int16_t x = r.mainX + (r.compactText ? 6 : 16);
  const int16_t y = r.mainY + (r.compactText ? 10 : 18);
  const uint8_t s = r.compactText ? 1 : 2;
  const bool refused = pet.notice == Notice::kNoMedicine;
  const bool cured =
      pet.notice == Notice::kMedicine && !(pet.sickness || pet.toothache);
  drawActionStateLabel(display, r, pet, medicineLabel());
  drawMedicineBottle(x, y + 7 * s, s, refused);
  drawMedicineBadge(r.mainX + r.mainW - 24 * s, y + 8 * s, s);
  drawDoseTrail(x + 1 * s, y + 34 * s, s);
  if (cured) {
    drawSparkle(display, r.mainX + r.mainW - 24 * s, y + 1 * s, phase);
  } else if (pet.notice == Notice::kNeedMoreMedicine) {
    drawSparkle(display, r.mainX + r.mainW - 22 * s, y + 2 * s, phase);
    drawSparkle(display, r.mainX + r.mainW - 13 * s, y + 13 * s, phase + 1);
  }
  drawSceneFrame(display, r, medicineFrame(), r.compactText ? 10 : 16, 0);
}

void drawLightsScene(EchoPetDisplayDevice& display, const ScreenResources& r,
                     const Snapshot& pet, uint8_t phase) {
  const int16_t cx = r.mainX + r.mainW / 2;
  const int16_t y = r.mainY + (r.compactText ? 9 : 18);
  const uint8_t s = r.compactText ? 1 : 2;
  auto drawLightSelector = [&](bool off, bool invalid) {
    const int16_t x = r.mainX + r.mainW - 23 * s;
    const int16_t top = r.mainY + (r.compactText ? 18 : 30);
    display.drawRect(x, top, 18 * s, 9 * s, EPD_BLACK);
    display.drawRect(x, top + 11 * s, 18 * s, 9 * s, EPD_BLACK);
    display.fillRect(x + 2 * s, top + (off ? 13 : 2) * s, 4 * s, 5 * s,
                     EPD_BLACK);
    if (!r.compactText) {
      drawText(display, x + 8 * s, top + 1 * s, "ON", 1);
      drawText(display, x + 8 * s, top + 12 * s, "OFF", 1);
    } else {
      display.drawLine(x + 8, top + 3, x + 14, top + 3, EPD_BLACK);
      display.drawLine(x + 8, top + 15, x + 14, top + 15, EPD_BLACK);
    }
    if (invalid) {
      display.drawLine(x - 1, top - 1, x + 19 * s, top + 21 * s,
                       EPD_BLACK);
      display.drawLine(x + 19 * s, top - 1, x - 1, top + 21 * s,
                       EPD_BLACK);
    }
  };

  auto drawDarkRoomPattern = [&]() {
    const int16_t roomX = r.mainX + 5 * s;
    const int16_t roomY = r.mainY + (r.compactText ? 15 : 27);
    const int16_t roomW = r.mainW - 34 * s;
    const int16_t roomH = r.compactText ? 24 : 48;
    display.drawRect(roomX, roomY, roomW, roomH, EPD_BLACK);
    for (int16_t dx = 4 * s; dx < roomW - 2 * s; dx += 7 * s) {
      display.drawLine(roomX + dx, roomY + 2 * s, roomX + dx - 4 * s,
                       roomY + roomH - 3 * s, EPD_BLACK);
    }
    display.drawCircle(roomX + 11 * s, roomY + 9 * s, 6 * s, EPD_BLACK);
    display.drawCircle(roomX + 14 * s, roomY + 7 * s, 6 * s, EPD_WHITE);
    for (uint8_t i = 0; i < 4; i++) {
      const int16_t sx = roomX + (10 + i * 5) * s;
      const int16_t sy = roomY + (5 + ((phase + i) & 0x03) * 4) * s;
      display.drawPixel(sx, sy, EPD_BLACK);
      if (!r.compactText) display.drawPixel(sx + 1, sy, EPD_BLACK);
    }
  };

  drawActionStateLabel(display, r, pet, pet.lightsOff ? "LIGHT OFF" : "LIGHT ON");
  if (pet.lightsOff) {
    drawDarkRoomPattern();
    drawLightSelector(true, false);
    display.setCursor(cx - 2, y + ((phase & 1) ? 0 : 4));
    display.print("Z");
  } else {
    display.drawCircle(cx, y + 16, 11 * s, EPD_BLACK);
    display.fillCircle(cx, y + 16, 3 * s, EPD_BLACK);
    for (uint8_t i = 0; i < 6; i++) {
      const int16_t dx = (i % 3 - 1) * 20;
      const int16_t dy = (i / 3 == 0 ? -1 : 1) * (18 + ((phase + i) & 1) * 4);
      display.drawLine(cx, y + 16, cx + dx, y + 16 + dy, EPD_BLACK);
    }
    drawLightSelector(false,
                      pet.notice == Notice::kLightsOn && !pet.lightsOff);
  }
  SpriteFrame frame =
      pet.lightsOff ? SpriteFrame::kLightsSelectorOff
                    : SpriteFrame::kLightsSelectorOn;
  if (pet.notice == Notice::kLightsOff) {
    frame = (phase & 1) ? SpriteFrame::kLightsOff
                        : SpriteFrame::kLightsSelectorOff;
  } else if (pet.notice == Notice::kLightsOn && pet.mood == Mood::kAsleep) {
    frame = SpriteFrame::kLightsWake;
  } else if (pet.notice == Notice::kLightsOn && !pet.lightsOff) {
    frame = SpriteFrame::kLightsInvalid;
  }
  drawSceneFrame(display, r, frame, r.compactText ? 4 : 8, 0);
}

void drawDisciplineScene(EchoPetDisplayDevice& display, const ScreenResources& r,
                         const Snapshot& pet, uint8_t phase) {
  const int16_t x = r.mainX + (r.compactText ? 6 : 14);
  const int16_t y = r.mainY + (r.compactText ? 12 : 22);
  const bool activeCall =
      pet.notice == Notice::kNeedsCare || pet.attention != 0;
  const bool missedOrInvalid = pet.notice == Notice::kWrongDiscipline;
  drawActionStateLabel(display, r, pet,
                       activeCall ? shortAttentionLabel(pet.attentionReason)
                                  : (pet.notice == Notice::kPraise ? "PRAISE"
                                                                   : "TIMEOUT"));
  display.drawRect(x, y + 8, 36, 26, EPD_BLACK);
  display.drawLine(x + 6, y + 20, x + 30, y + 20, EPD_BLACK);
  display.fillCircle(x + 12 + ((phase & 1) ? 8 : 0), y + 15, 3, EPD_BLACK);
  display.fillCircle(x + 24, y + 15, 3, EPD_BLACK);
  if (missedOrInvalid) {
    display.drawLine(x + 10, y + 25, x + 30, y + 12, EPD_BLACK);
    display.drawLine(x + 30, y + 25, x + 10, y + 12, EPD_BLACK);
  } else if (activeCall) {
    display.drawLine(x + 13, y + 26, x + 18, y + 22, EPD_BLACK);
    display.drawLine(x + 18, y + 22, x + 25, y + 26, EPD_BLACK);
    drawSmallHeart(display, x + 15, y + 2, (phase & 1) != 0);
  } else if (pet.notice == Notice::kPraise) {
    drawSmallHeart(display, x + 15, y + 24, true);
  } else {
    display.drawLine(x + 15, y + 26, x + 26, y + 26, EPD_BLACK);
  }
  display.drawRect(r.mainX + r.mainW - 34, y + 4, 18, 32, EPD_BLACK);
  display.drawLine(r.mainX + r.mainW - 25, y, r.mainX + r.mainW - 25, y + 4,
                   EPD_BLACK);
  SpriteFrame frame = SpriteFrame::kDisciplineTimeout;
  if (activeCall) {
    frame = SpriteFrame::kAttentionCall;
  } else if (pet.notice == Notice::kPraise) {
    frame = SpriteFrame::kDisciplinePraise;
  } else if (missedOrInvalid) {
    frame = (phase & 1) ? SpriteFrame::kDisciplineInvalid
                        : SpriteFrame::kAttentionMissed;
  }
  drawSceneFrame(display, r, frame, r.compactText ? 9 : 15, 0);
}

int16_t v3Px(const V3Canvas& canvas, int16_t x) {
  return canvas.x + x * canvas.scale;
}

int16_t v3Py(const V3Canvas& canvas, int16_t y) {
  return canvas.y + y * canvas.scale;
}

void v3Fill(EchoPetDisplayDevice& display, const V3Canvas& canvas, int16_t x,
            int16_t y, int16_t w, int16_t h) {
  display.fillRect(v3Px(canvas, x), v3Py(canvas, y), w * canvas.scale,
                   h * canvas.scale, EPD_BLACK);
}

void v3Rect(EchoPetDisplayDevice& display, const V3Canvas& canvas, int16_t x,
            int16_t y, int16_t w, int16_t h) {
  display.drawRect(v3Px(canvas, x), v3Py(canvas, y), w * canvas.scale,
                   h * canvas.scale, EPD_BLACK);
}

void v3HLine(EchoPetDisplayDevice& display, const V3Canvas& canvas, int16_t x,
             int16_t y, int16_t w) {
  v3Fill(display, canvas, x, y, w, 1);
}

void v3VLine(EchoPetDisplayDevice& display, const V3Canvas& canvas, int16_t x,
             int16_t y, int16_t h) {
  v3Fill(display, canvas, x, y, 1, h);
}

void v3Text(EchoPetDisplayDevice& display, const V3Canvas& canvas, int16_t x,
            int16_t y, const char* text) {
  drawText(display, v3Px(canvas, x), v3Py(canvas, y), text, 1);
}

void v3CenteredText(EchoPetDisplayDevice& display, const V3Canvas& canvas,
                    int16_t y, const char* text) {
  const uint16_t w = textPixelWidth(text, 1);
  drawText(display, canvas.x + (canvas.screenW - static_cast<int16_t>(w)) / 2,
           v3Py(canvas, y), text, 1);
}

void drawV3Heart(EchoPetDisplayDevice& display, const V3Canvas& canvas,
                 int16_t x, int16_t y, bool filled) {
  if (filled) {
    v3Fill(display, canvas, x + 1, y, 2, 1);
    v3Fill(display, canvas, x + 4, y, 2, 1);
    v3Fill(display, canvas, x, y + 1, 7, 2);
    v3Fill(display, canvas, x + 1, y + 3, 5, 1);
    v3Fill(display, canvas, x + 2, y + 4, 3, 1);
    v3Fill(display, canvas, x + 3, y + 5, 1, 1);
  } else {
    v3Rect(display, canvas, x + 1, y, 2, 1);
    v3Rect(display, canvas, x + 4, y, 2, 1);
    v3HLine(display, canvas, x, y + 1, 7);
    v3VLine(display, canvas, x, y + 2, 1);
    v3VLine(display, canvas, x + 6, y + 2, 1);
    v3HLine(display, canvas, x + 1, y + 3, 5);
    v3Fill(display, canvas, x + 2, y + 4, 1, 1);
    v3Fill(display, canvas, x + 4, y + 4, 1, 1);
    v3Fill(display, canvas, x + 3, y + 5, 1, 1);
  }
}

void drawV3MiniPet(EchoPetDisplayDevice& display, const V3Canvas& canvas,
                   int16_t x, int16_t y, uint8_t phase, bool happy) {
  const int16_t bob = (phase & 1) ? -1 : 0;
  v3HLine(display, canvas, x + 3, y + bob, 8);
  v3HLine(display, canvas, x + 1, y + 1 + bob, 12);
  v3VLine(display, canvas, x, y + 3 + bob, 7);
  v3VLine(display, canvas, x + 13, y + 3 + bob, 7);
  v3HLine(display, canvas, x + 1, y + 10 + bob, 12);
  v3Fill(display, canvas, x + 4, y + 5 + bob, 2, 2);
  v3Fill(display, canvas, x + 9, y + 5 + bob, 2, 2);
  if (happy) {
    v3HLine(display, canvas, x + 5, y + 8 + bob, 4);
    v3Fill(display, canvas, x + 4, y + 7 + bob, 1, 1);
    v3Fill(display, canvas, x + 9, y + 7 + bob, 1, 1);
  } else {
    v3HLine(display, canvas, x + 5, y + 9 + bob, 5);
  }
  v3Fill(display, canvas, x + 2, y + 11 + bob, 2, 1);
  v3Fill(display, canvas, x + 10, y + 11 + bob, 2, 1);
}

void drawV3CatalogPet(EchoPetDisplayDevice& display, const V3Canvas& canvas,
                      const Snapshot& pet, int16_t x, int16_t y,
                      uint8_t phase, uint8_t scale = 1) {
  drawCharacterCatalogBitmap(display, v3Px(canvas, x), v3Py(canvas, y),
                             pet.characterCatalogId, phase, scale);
}

void drawV3Poop(EchoPetDisplayDevice& display, const V3Canvas& canvas, int16_t x,
                int16_t y, uint8_t phase) {
  v3Fill(display, canvas, x + 4, y, 2, 1);
  v3Fill(display, canvas, x + 3, y + 1, 4, 1);
  v3Fill(display, canvas, x + 2, y + 2, 6, 2);
  v3Fill(display, canvas, x + 1, y + 4, 8, 1);
  v3Fill(display, canvas, x, y + 5, 10, 2);
  v3Fill(display, canvas, x + 2, y + 7, 6, 1);
  const int16_t drift = (phase & 1) ? 1 : 0;
  v3Fill(display, canvas, x + 2 + drift, y - 3, 1, 1);
  v3Fill(display, canvas, x + 3 + drift, y - 2, 1, 1);
  v3Fill(display, canvas, x + 7 - drift, y - 4, 1, 1);
  v3Fill(display, canvas, x + 6 - drift, y - 3, 1, 1);
}

void drawV3FoodProp(EchoPetDisplayDevice& display, const V3Canvas& canvas,
                    int16_t x, int16_t y, bool snack) {
  if (snack) {
    v3Fill(display, canvas, x + 3, y, 4, 1);
    v3Fill(display, canvas, x + 1, y + 1, 8, 2);
    v3Fill(display, canvas, x, y + 3, 10, 4);
    v3Fill(display, canvas, x + 2, y + 7, 6, 1);
    v3Fill(display, canvas, x + 3, y + 4, 1, 1);
    v3Fill(display, canvas, x + 6, y + 4, 1, 1);
  } else {
    v3Rect(display, canvas, x, y + 4, 12, 5);
    v3HLine(display, canvas, x + 2, y + 3, 8);
    v3Fill(display, canvas, x + 3, y + 1, 6, 2);
  }
}

void drawV3GameProp(EchoPetDisplayDevice& display, const V3Canvas& canvas,
                    int16_t x, int16_t y, uint8_t phase) {
  v3Fill(display, canvas, x + ((phase & 1) ? 2 : 0), y, 4, 4);
  v3HLine(display, canvas, x + 8, y + 7, 12);
  v3VLine(display, canvas, x + 14, y + 2, 8);
  v3HLine(display, canvas, x + 13, y + 2, 5);
  v3HLine(display, canvas, x + 13, y + 10, 5);
}

void drawV3ConnectSignal(EchoPetDisplayDevice& display, const V3Canvas& canvas,
                         int16_t x, int16_t y, uint8_t phase) {
  drawV3Heart(display, canvas, x, y + 6, true);
  const int16_t pulse = phase & 1;
  v3Fill(display, canvas, x + 8, y + 4 - pulse, 1, 1);
  v3Fill(display, canvas, x + 10, y + 2 - pulse, 1, 2);
  v3Fill(display, canvas, x + 12, y - pulse, 1, 3);
}

void drawV3MedicineBag(EchoPetDisplayDevice& display, const V3Canvas& canvas,
                       int16_t x, int16_t y) {
  v3Rect(display, canvas, x + 2, y + 4, 14, 10);
  v3Rect(display, canvas, x + 6, y + 1, 6, 4);
  v3HLine(display, canvas, x + 6, y + 9, 6);
  v3VLine(display, canvas, x + 9, y + 6, 6);
}

void drawV3Lamp(EchoPetDisplayDevice& display, const V3Canvas& canvas,
                int16_t x, int16_t y) {
  v3HLine(display, canvas, x + 3, y, 10);
  v3VLine(display, canvas, x + 2, y + 1, 6);
  v3VLine(display, canvas, x + 13, y + 1, 6);
  v3HLine(display, canvas, x + 4, y + 7, 8);
  v3VLine(display, canvas, x + 8, y + 8, 7);
  v3HLine(display, canvas, x + 4, y + 15, 9);
}

void drawV3OpenBook(EchoPetDisplayDevice& display, const V3Canvas& canvas,
                    int16_t x, int16_t y) {
  v3Rect(display, canvas, x, y + 2, 11, 14);
  v3Rect(display, canvas, x + 11, y + 2, 11, 14);
  v3VLine(display, canvas, x + 11, y, 18);
  v3HLine(display, canvas, x + 3, y + 6, 5);
  v3HLine(display, canvas, x + 14, y + 6, 5);
  v3HLine(display, canvas, x + 3, y + 10, 5);
  v3HLine(display, canvas, x + 14, y + 10, 5);
}

void drawV3HealthScene(EchoPetDisplayDevice& display, const ScreenResources& r,
                       const Snapshot& pet, uint8_t phase) {
  const V3Canvas c = v3CanvasFor(r);
  v3Text(display, c, 0, 0, "HUNGRY");
  v3Text(display, c, 0, 15, "HAPPY");
  for (uint8_t i = 0; i < 4; ++i) {
    drawV3Heart(display, c, 2 + i * 7, 8, i < pipsFor(pet.hunger));
    drawV3Heart(display, c, 2 + i * 7, 23, i < pipsFor(pet.happiness));
  }
  if (pet.attention && (phase & 1)) {
    v3Fill(display, c, 29, 2, 2, 2);
    v3VLine(display, c, 30, 5, 5);
  }
}

void drawV3FoodScene(EchoPetDisplayDevice& display, const ScreenResources& r,
                     const Snapshot& pet, const UiState& ui, uint8_t phase) {
  const V3Canvas c = v3CanvasFor(r);
  const bool snack = ui.mode == UiMode::kSnack ||
                     (ui.mode == UiMode::kFoodMenu && (ui.cursor & 1));
  const bool eating = ui.mode == UiMode::kMeal || ui.mode == UiMode::kSnack ||
                      pet.notice == Notice::kMeal || pet.notice == Notice::kSnack;
  if (!eating) {
    v3Fill(display, c, 2, snack ? 18 : 4, 3, 3);
    v3Text(display, c, 8, 3, "MEAL");
    v3Text(display, c, 8, 18, "SNACK");
    drawV3FoodProp(display, c, 23, snack ? 18 : 4, snack);
    return;
  }
  drawV3CatalogPet(display, c, pet, 16, 11 + ((phase & 1) ? -1 : 0), phase, 1);
  if ((phase & 3) < 2) {
    drawV3FoodProp(display, c, 2 + ((phase & 1) ? 4 : 0), 16, snack);
  } else {
    v3Fill(display, c, 5, 22, 1, 1);
    v3Fill(display, c, 8, 23, 1, 1);
    v3Fill(display, c, 11, 22, 1, 1);
    drawV3Heart(display, c, 4, 4, true);
  }
}

void drawV3GameScene(EchoPetDisplayDevice& display, const ScreenResources& r,
                     const Snapshot& pet, const UiState& ui, uint8_t phase) {
  const V3Canvas c = v3CanvasFor(r);
  const MiniGameKind game =
      pet.miniGame == MiniGameKind::kNone
          ? static_cast<MiniGameKind>((ui.cursor % 7) + 1)
          : pet.miniGame;
  switch (game) {
    case MiniGameKind::kGet:
      v3Text(display, c, 3, 0, "GET");
      v3Fill(display, c, 7 + (phase & 3) * 5, 8 + (phase & 3), 2, 2);
      v3Rect(display, c, 10 + (pet.gameCursor % 3) * 5, 23, 8, 4);
      break;
    case MiniGameKind::kBump:
      v3Text(display, c, 2, 0, "PUSH");
      drawV3MiniPet(display, c, 1, 10, phase, true);
      drawV3MiniPet(display, c, 18, 10, phase + 1, false);
      v3Rect(display, c, 4, 25, 24, 3);
      v3Fill(display, c, 5, 26, 5 + (phase & 3) * 5, 1);
      break;
    case MiniGameKind::kFlag:
      v3Text(display, c, 2, 0, "FLAG");
      drawV3MiniPet(display, c, 10, 13, phase, true);
      v3VLine(display, c, 4, 7, 14);
      v3Rect(display, c, 5, 7, 7, 4);
      v3VLine(display, c, 25, 7, 14);
      v3Fill(display, c, 18, 7, 7, 4);
      break;
    case MiniGameKind::kHeading:
      v3Text(display, c, 0, 0, "HEAD");
      v3Fill(display, c, 14 + ((phase & 1) ? 4 : -4), 7 + (phase & 3) * 4, 3,
             3);
      drawV3MiniPet(display, c, 9, 19, phase, true);
      break;
    case MiniGameKind::kMemory:
      v3Text(display, c, 0, 0, "MEMORY");
      for (uint8_t i = 0; i < 3; ++i) {
        v3Rect(display, c, 4 + i * 9, 12, 6, 6);
        if (i == (phase % 3)) v3Fill(display, c, 6 + i * 9, 14, 2, 2);
      }
      break;
    case MiniGameKind::kSprint:
      v3Text(display, c, 1, 0, "SPRINT");
      v3HLine(display, c, 1, 25, 30);
      v3VLine(display, c, 27, 17, 9);
      drawV3MiniPet(display, c, 2 + (phase & 3) * 5, 14, phase, true);
      break;
    case MiniGameKind::kHoops:
      v3Text(display, c, 2, 0, "HOOPS");
      v3Rect(display, c, 23, 10, 7, 6);
      v3VLine(display, c, 29, 16, 10);
      v3Fill(display, c, 7 + (phase & 3) * 4, 10 - (phase & 1), 3, 3);
      drawV3MiniPet(display, c, 4, 17, phase, true);
      break;
    case MiniGameKind::kNone:
      drawV3GameProp(display, c, 5, 9, phase);
      break;
  }
  if (pet.notice == Notice::kGameGood || pet.notice == Notice::kGameWin) {
    v3Text(display, c, 3, 22, "GOOD");
  }
}

void drawV3ConnectScene(EchoPetDisplayDevice& display, const ScreenResources& r,
                        const Snapshot& pet, const UiState& ui,
                        uint8_t phase) {
  const V3Canvas c = v3CanvasFor(r);
  const bool standby = ui.mode == UiMode::kLinkStandby;
  const bool result = ui.mode == UiMode::kLinkResult;
  if (standby) {
    v3CenteredText(display, c, 0, "STAND BY");
  } else if (result && ui.entry[2] >= 3) {
    v3CenteredText(display, c, 0, "FAILED");
  } else {
    v3CenteredText(display, c, 0, "CONNECT");
  }
  drawV3CatalogPet(display, c, pet, 2, 13 + ((phase & 1) ? -1 : 0), phase, 1);
  drawV3MiniPet(display, c, 20, 13 + ((phase & 1) ? 0 : -1), phase + 1,
                result ? ui.entry[2] < 3 : true);
  if (ui.mode == UiMode::kPresent ||
      ui.entry[0] == static_cast<uint8_t>(LinkKind::kPresent)) {
    v3Rect(display, c, 13, 18, 6, 6);
    v3HLine(display, c, 12, 20, 8);
    v3VLine(display, c, 16, 17, 8);
  } else if (ui.entry[0] == static_cast<uint8_t>(LinkKind::kLove)) {
    drawV3Heart(display, c, 13, 16, true);
  } else if (ui.mode == UiMode::kLinkGame ||
             ui.entry[0] == static_cast<uint8_t>(LinkKind::kGame)) {
    drawV3GameProp(display, c, 8, 18, phase);
  } else {
    drawV3ConnectSignal(display, c, 13, 12, phase);
  }
}

void drawV3CareScene(EchoPetDisplayDevice& display, const ScreenResources& r,
                     const Snapshot& pet, uint8_t phase) {
  const V3Canvas c = v3CanvasFor(r);
  drawV3CatalogPet(display, c, pet, 8, 13 + ((phase & 1) ? -1 : 0), phase, 1);
  v3Fill(display, c, 22, 5, 2, 2);
  v3VLine(display, c, 23, 8, 7);
  v3Fill(display, c, 23, 18, 2, 2);
  if (pet.attention || (phase & 1)) {
    v3Fill(display, c, 4, 8, 5, 1);
    v3Fill(display, c, 3, 9, 1, 2);
    v3Fill(display, c, 9, 9, 1, 2);
  }
  if (pet.messCount != 0) {
    drawV3Poop(display, c, 23, 21, phase);
  }
}

void drawV3DisciplineScene(EchoPetDisplayDevice& display,
                           const ScreenResources& r, const Snapshot& pet,
                           uint8_t phase) {
  const V3Canvas c = v3CanvasFor(r);
  const bool praise = pet.notice == Notice::kPraise;
  drawV3CatalogPet(display, c, pet, 15, 13 + ((phase & 1) ? -1 : 0), phase, 1);
  if (praise) {
    drawV3Heart(display, c, 5, 9, true);
    drawV3Heart(display, c, 8, 15, (phase & 1) != 0);
  } else {
    v3Fill(display, c, 2, 10, 8, 8);
    v3Fill(display, c, 10, 13, 5, 2);
    v3Fill(display, c, 4, 19, 3, 1);
    v3Fill(display, c, 24, 6, 2, 2);
    v3Fill(display, c, 27, 4, 1, 3);
  }
}

void drawV3MedicineScene(EchoPetDisplayDevice& display,
                         const ScreenResources& r, const Snapshot& pet,
                         uint8_t phase) {
  const V3Canvas c = v3CanvasFor(r);
  drawV3MedicineBag(display, c, 1, 12);
  drawV3CatalogPet(display, c, pet, 16, 13 + ((phase & 1) ? -1 : 0), phase, 1);
  if (pet.toothache) {
    v3Rect(display, c, 20, 4, 6, 7);
    v3HLine(display, c, 21, 10, 4);
  } else {
    v3Fill(display, c, 21, 5, 5, 5);
    v3Fill(display, c, 23, 10, 1, 3);
  }
  if (phase & 1) {
    v3Fill(display, c, 12, 17, 2, 1);
    v3Fill(display, c, 14, 16, 2, 1);
  }
}

void drawV3LightsScene(EchoPetDisplayDevice& display, const ScreenResources& r,
                       const Snapshot& pet, uint8_t phase) {
  const V3Canvas c = v3CanvasFor(r);
  if (pet.lightsOff) {
    v3Fill(display, c, 0, 0, 32, 30);
    display.setTextColor(EPD_WHITE);
    drawText(display, v3Px(c, 20), v3Py(c, 3), "Z", 1);
    display.setTextColor(EPD_BLACK);
    return;
  }
  drawV3Lamp(display, c, 2, 5);
  drawV3CatalogPet(display, c, pet, 17, 13 + ((phase & 1) ? -1 : 0), phase, 1);
  v3Rect(display, c, 22, 4, 7, 5);
  v3Rect(display, c, 22, 11, 7, 5);
  v3Fill(display, c, 24, (phase & 1) ? 12 : 5, 3, 3);
}

void drawV3FriendScene(EchoPetDisplayDevice& display, const ScreenResources& r,
                       const Snapshot& pet, const UiState& ui, uint8_t phase) {
  const V3Canvas c = v3CanvasFor(r);
  drawV3OpenBook(display, c, 5, 5);
  if (pet.friendCount == 0) {
    v3CenteredText(display, c, 22, "NO FRIEND");
    return;
  }
  const uint8_t i = ui.cursor % pet.friendCount;
  drawCatalogAvatar(display, v3Px(c, 3), v3Py(c, 12), pet.characterCatalogId,
                    phase, 1);
  drawCatalogAvatar(display, v3Px(c, 20), v3Py(c, 12),
                    catalogIdForFriendAvatar(pet.friends[i]), phase + 1, 1);
  drawV3Heart(display, c, 13, 19, (phase & 1) != 0);
}

bool drawCompactV3FunctionScreen(EchoPetDisplayDevice& display,
                                 const ScreenResources& r,
                                 uint8_t selectedMenuIndex,
                                 const Snapshot& pet, const UiState& ui,
                                 uint8_t phase) {
  if (!r.compactText ||
      (ui.mode == UiMode::kHome && pet.miniGame == MiniGameKind::kNone)) {
    return false;
  }

  switch (selectedMenuIndex) {
    case 0:
      drawV3HealthScene(display, r, pet, phase);
      return true;
    case 1:
      drawV3FoodScene(display, r, pet, ui, phase);
      return true;
    case 2:
      drawToiletScene(display, r, pet, ui, phase);
      return true;
    case 3:
      drawV3GameScene(display, r, pet, ui, phase);
      return true;
    case 4:
      drawV3ConnectScene(display, r, pet, ui, phase);
      return true;
    case 5:
      drawV3CareScene(display, r, pet, phase);
      return true;
    case 6:
      drawV3DisciplineScene(display, r, pet, phase);
      return true;
    case 7:
      drawV3MedicineScene(display, r, pet, phase);
      return true;
    case 8:
      drawV3LightsScene(display, r, pet, phase);
      return true;
    case 9:
      drawV3FriendScene(display, r, pet, ui, phase);
      return true;
  }

  switch (ui.mode) {
    case UiMode::kConnectionMenu:
    case UiMode::kVisitLink:
    case UiMode::kPresent:
    case UiMode::kLinkGame:
    case UiMode::kLinkStandby:
    case UiMode::kLinkResult:
      drawV3ConnectScene(display, r, pet, ui, phase);
      return true;
    case UiMode::kDisciplineMenu:
    case UiMode::kDiscipline:
      drawV3DisciplineScene(display, r, pet, phase);
      return true;
    case UiMode::kMedicine:
      drawV3MedicineScene(display, r, pet, phase);
      return true;
    case UiMode::kLights:
      drawV3LightsScene(display, r, pet, phase);
      return true;
    case UiMode::kFriends:
      drawV3FriendScene(display, r, pet, ui, phase);
      return true;
    default:
      return false;
  }
}

void drawPageScreen(EchoPetDisplayDevice& display, const ScreenResources& r,
                    const Snapshot& pet, const UiState& ui,
                    uint8_t animationPhase) {
  switch (ui.mode) {
    case UiMode::kHealth:
      drawHealthPage(display, r, pet, ui.page, animationPhase);
      break;
    case UiMode::kFoodMenu: {
      static const char* const labels[] = {"MEAL", "SNACK"};
      drawChoiceMenu(display, r, "FOOD", labels, 2, ui.cursor);
      break;
    }
    case UiMode::kMeal:
      drawFoodPage(display, r, pet, ui, false, animationPhase);
      break;
    case UiMode::kSnack:
      drawFoodPage(display, r, pet, ui, true, animationPhase);
      break;
    case UiMode::kActivityMenu: {
      static const char* const labels[] = {
          "GAME", "ITEM", "SHOP", "PASSWORD", "SOUV", "POINT"};
      drawChoiceMenu(display, r, "ACTIVITY", labels, 6, ui.cursor);
      break;
    }
    case UiMode::kGame:
      drawGameSelectPage(display, r, pet, ui, false, animationPhase);
      break;
    case UiMode::kShop:
      drawShopPage(display, r, pet, ui);
      break;
    case UiMode::kItem:
      drawItemPage(display, r, pet, ui);
      break;
    case UiMode::kPoint:
      drawPointPage(display, r, pet, animationPhase);
      break;
    case UiMode::kConnectionMenu: {
      static const char* const labels[] = {"GAME", "PRESENT", "VISIT", "LOVE"};
      drawChoiceMenu(display, r, "CONNECT", labels, 4, ui.cursor);
      break;
    }
    case UiMode::kVisitLink:
      drawVisitLinkPage(display, r, pet, animationPhase);
      break;
    case UiMode::kFriends:
      drawFriendPage(display, r, pet, ui, animationPhase);
      break;
    case UiMode::kFamily:
      drawFamilyPage(display, r, pet, ui, animationPhase);
      break;
    case UiMode::kSouvenirs:
      drawSouvenirPage(display, r, pet, ui);
      break;
    case UiMode::kPassword:
      drawPasswordPage(display, r, ui);
      break;
    case UiMode::kLinkStandby:
      drawLinkStandbyPage(display, r, ui, animationPhase);
      break;
    case UiMode::kLinkResult:
      drawLinkResultPage(display, r, pet, ui, animationPhase);
      break;
    case UiMode::kDisciplineMenu: {
      static const char* const labels[] = {"TIMEOUT", "PRAISE"};
      drawChoiceMenu(display, r, "DISCIPLINE", labels, 2, ui.cursor);
      break;
    }
    case UiMode::kFriendDeleteConfirm:
      drawFriendDeletePage(display, r, pet, ui);
      break;
    case UiMode::kPresent:
      drawPresentPage(display, r, pet, ui);
      break;
    case UiMode::kLinkGame:
      drawGameSelectPage(display, r, pet, ui, true, animationPhase);
      break;
    case UiMode::kToilet:
      drawToiletScene(display, r, pet, ui, animationPhase);
      break;
    case UiMode::kMedicine:
      drawMedicineScene(display, r, pet, animationPhase);
      break;
    case UiMode::kLights:
      drawLightsScene(display, r, pet, animationPhase);
      break;
    case UiMode::kDiscipline:
      drawDisciplineScene(display, r, pet, animationPhase);
      break;
    case UiMode::kResetConfirm:
      drawPageTitle(display, r, "RESET");
      drawPageLine(display, r, 0, "Only after pass");
      drawPageLine(display, r, 1, "B confirms");
      break;
    case UiMode::kSpriteProof:
      drawSpriteProofPage(display, r, ui);
      break;
    case UiMode::kClockSet:
      drawClockSetPage(display, r, ui, animationPhase);
      break;
    case UiMode::kSetup:
      drawSetupPage(display, r, ui, animationPhase);
      break;
    case UiMode::kHome:
      break;
  }
}

void drawScoreDots(EchoPetDisplayDevice& display, int16_t x, int16_t y,
                   uint8_t score) {
  for (uint8_t i = 0; i < 5; i++) {
    display.drawRect(x + i * 13, y, 9, 9, EPD_BLACK);
    if (i < score) display.fillRect(x + i * 13 + 2, y + 2, 5, 5, EPD_BLACK);
  }
}

uint8_t visualProgress(uint8_t value, uint8_t limit, uint8_t span) {
  if (limit <= 1 || span == 0) return 0;
  if (value >= limit) value = static_cast<uint8_t>(limit - 1);
  return static_cast<uint8_t>(
      (static_cast<uint16_t>(value) * span) / (limit - 1));
}

uint8_t visualScoreDots(uint8_t score, uint8_t scoreLimit) {
  if (scoreLimit == 0) return 0;
  if (score >= scoreLimit) return 5;
  return static_cast<uint8_t>(
      (static_cast<uint16_t>(score) * 5U + scoreLimit / 2U) / scoreLimit);
}

int16_t gameLaneX(const ScreenResources& r, uint8_t lane) {
  if (lane > 2) lane = 2;
  return r.mainX + r.mainW / 4 + lane * (r.mainW / 4);
}

void drawFlagSignal(EchoPetDisplayDevice& display, int16_t x, int16_t y,
                    bool blackFlag, bool fake) {
  const int16_t poleH = fake ? 10 : 18;
  const int16_t flagY = fake ? y + 4 : y;
  const int16_t flagH = fake ? 5 : 8;
  display.drawLine(x, y, x, y + poleH, EPD_BLACK);
  if (blackFlag) {
    display.fillRect(x + 1, flagY, 12, flagH, EPD_BLACK);
  } else {
    display.drawRect(x + 1, flagY, 12, flagH, EPD_BLACK);
  }
}

const char* gameControlHint(MiniGameKind game) {
  switch (game) {
    case MiniGameKind::kGet:
      return "A< C>";
    case MiniGameKind::kBump:
      return "ANY PUSH";
    case MiniGameKind::kFlag:
      return "A:B AC C:W";
    case MiniGameKind::kHeading:
      return "A< B C>";
    case MiniGameKind::kMemory:
      return "A B C COPY";
    case MiniGameKind::kSprint:
      return "ANY TAP";
    case MiniGameKind::kHoops:
      return "A< BSHOT C>";
    case MiniGameKind::kNone:
      return "";
  }
  return "";
}

void drawGamePromptBar(EchoPetDisplayDevice& display, const ScreenResources& r,
                       const Snapshot& pet) {
  char buffer[24];
  const int16_t y = r.mainY + r.mainH - (r.compactText ? 11 : 17);
  const char* hint = gameControlHint(pet.miniGame);
  const uint8_t roundLimit = pet.gameRoundLimit ? pet.gameRoundLimit : 1;
  if (!r.compactText && hint[0] != '\0') {
    drawCenteredLine(display, r, y - (r.compactText ? 9 : 12), hint, 1);
  }
  if (r.compactText && (roundLimit >= 100 || pet.gameScoreLimit >= 100)) {
    snprintf(buffer, sizeof(buffer), "%u S%u",
             static_cast<unsigned>(pet.gameRound + 1),
             static_cast<unsigned>(pet.gameScore));
  } else {
    snprintf(buffer, sizeof(buffer), "%s %u/%u S%u",
             gamePromptLabel(pet.gamePrompt),
             static_cast<unsigned>(pet.gameRound + 1),
             static_cast<unsigned>(roundLimit),
             static_cast<unsigned>(pet.gameScore));
  }
  drawCenteredLine(display, r, y, buffer, 1);
}

void drawGameSprite(EchoPetDisplayDevice& display, const ScreenResources& r,
                    SpriteFrame frame, int16_t centerX, int16_t centerY) {
  const SpriteFrameInfo info = spriteFrameInfo(frame);
  if (info.width == 0 || info.height == 0) {
    return;
  }
  const uint8_t scale = 1;
  const int16_t width = info.width * scale;
  const int16_t height = info.height * scale;
  int16_t x = centerX - width / 2;
  int16_t y = centerY - height / 2;
  if (x < r.mainX + 1) x = r.mainX + 1;
  if (x + width > r.mainX + r.mainW - 1) {
    x = r.mainX + r.mainW - width - 1;
  }
  if (y < r.mainY + 14) y = r.mainY + 14;
  if (y + height > r.mainY + r.mainH - 12) {
    y = r.mainY + r.mainH - height - 12;
  }
  drawSpriteFrame(display, frame, x, y, scale);
}

void drawGameScreen(EchoPetDisplayDevice& display, const ScreenResources& r,
                    const Snapshot& pet) {
  char buffer[24];
  const uint8_t roundLimit = pet.gameRoundLimit ? pet.gameRoundLimit : 1;
  const uint8_t scoreLimit = pet.gameScoreLimit ? pet.gameScoreLimit : 1;
  const uint8_t titleSize = r.compactText ? 1 : 2;
  drawText(display, r.mainX + 6, r.mainY + 8, miniGameLabel(pet.miniGame),
           titleSize);
  if (!r.compactText || roundLimit < 100) {
    snprintf(buffer, sizeof(buffer), "%u/%u",
             static_cast<unsigned>(pet.gameRound + 1),
             static_cast<unsigned>(roundLimit));
    drawText(display, r.mainX + r.mainW - (roundLimit >= 10 ? 36 : 24),
             r.mainY + 8, buffer);
  }
  const int16_t top = r.mainY + (r.compactText ? 20 : 34);
  const int16_t bottom = r.mainY + r.mainH - (r.compactText ? 18 : 30);
  const int16_t cx = r.mainX + r.mainW / 2;

  switch (pet.miniGame) {
    case MiniGameKind::kGet: {
      const int16_t itemX = gameLaneX(r, pet.gameTarget);
      const uint8_t fall =
          visualProgress(pet.gameRound, roundLimit,
                         static_cast<uint8_t>(bottom > top + 20
                                                  ? bottom - top - 20
                                                  : 0));
      if (pet.gameHazard) {
        drawMessIcon(display, itemX - 5, top + 3, 1);
      } else {
        display.fillCircle(itemX, top + 7 + fall, 5, EPD_BLACK);
        if (pet.gameExpectedYes > 1) {
          display.fillCircle(itemX + 8, top + 5 + fall, 4, EPD_BLACK);
        }
      }
      const int16_t bucketX = gameLaneX(r, pet.gameCursor) - 11;
      display.drawRect(bucketX, bottom - 7, 22, 10, EPD_BLACK);
      display.drawLine(bucketX, bottom - 7, bucketX + 11, bottom - 16,
                       EPD_BLACK);
      display.drawLine(bucketX + 22, bottom - 7, bucketX + 11, bottom - 16,
                       EPD_BLACK);
      drawGameSprite(display, r,
                     pet.gameHazard ? SpriteFrame::kGameGetBad
                                    : SpriteFrame::kGameGetNote,
                     cx, top + 28);
      break;
    }
    case MiniGameKind::kBump:
      drawSmallFace(display, cx - 34, top + 6, pet.gameRound);
      drawSmallFace(display, cx + 20, top + 6, pet.gameRound + 1);
      display.drawLine(cx - 5, top + 16, cx + 5, top + 16, EPD_BLACK);
      display.drawRect(r.mainX + 12, bottom - 16, r.mainW - 24, 8, EPD_BLACK);
      display.fillRect(r.mainX + 14, bottom - 14,
                       (r.mainW - 28) * pet.gameCursor / 100, 4, EPD_BLACK);
      drawGameSprite(display, r,
                     (pet.gameCursor > 70) ? SpriteFrame::kGameBumpPush
                                           : SpriteFrame::kGameBumpMeter,
                     cx, top + 35);
      break;
    case MiniGameKind::kFlag: {
      const bool both =
          pet.gameExpectedYes == static_cast<uint8_t>(GameButton::kAC);
      const bool black =
          pet.gameExpectedYes == static_cast<uint8_t>(GameButton::kA);
      const bool singleLeft = pet.gameTarget == 0;
      const bool fake = pet.gameHazard != 0;
      drawSmallFace(display, cx - 4, top + 13, pet.gameRound);
      if (both) {
        drawFlagSignal(display, cx - 34, top + 6, true, fake);
        drawFlagSignal(display, cx + 26, top + 6, false, fake);
      } else {
        drawFlagSignal(display, singleLeft ? cx - 34 : cx + 26, top + 6,
                       black, fake);
      }
      drawGameSprite(display, r,
                     both
                         ? SpriteFrame::kGameFlagBoth
                         : (singleLeft ? SpriteFrame::kGameFlagLeft
                                 : SpriteFrame::kGameFlagRight),
                     cx, top + 36);
      break;
    }
    case MiniGameKind::kHeading:
      display.fillCircle(
          gameLaneX(r, pet.gameTarget),
          top + 8 +
              visualProgress(pet.gameRound, roundLimit,
                             static_cast<uint8_t>(bottom > top + 22
                                                      ? bottom - top - 22
                                                      : 0)),
          5, EPD_BLACK);
      display.drawCircle(gameLaneX(r, pet.gameCursor), bottom - 10, 9,
                         EPD_BLACK);
      display.fillRect(gameLaneX(r, pet.gameCursor) - 5, bottom - 8, 10, 4,
                       EPD_BLACK);
      drawGameSprite(display, r, SpriteFrame::kGameHeadingBall, cx, top + 35);
      break;
    case MiniGameKind::kMemory: {
      const uint8_t sequenceLen = pet.gameTarget ? pet.gameTarget : 1;
      uint8_t windowStart = pet.gameCursor >= 3
                                ? static_cast<uint8_t>(pet.gameCursor - 3)
                                : 0;
      if (sequenceLen > 4 && windowStart + 4 > sequenceLen) {
        windowStart = static_cast<uint8_t>(sequenceLen - 4);
      }
      const uint8_t visibleCount = sequenceLen < 4 ? sequenceLen : 4;
      for (uint8_t i = 0; i < visibleCount; i++) {
        const uint8_t index = static_cast<uint8_t>(windowStart + i);
        const uint8_t code =
            static_cast<uint8_t>((pet.gameHazard >> (index * 2)) & 0x03);
        const int16_t x = gameLaneX(r, code < 3 ? code : 1);
        display.drawRect(x - 8, top + i * 10, 16, 8, EPD_BLACK);
        if (index == pet.gameCursor) {
          display.fillRect(x - 5, top + i * 10 + 2, 10, 4, EPD_BLACK);
        }
      }
      drawGameSprite(display, r,
                     pet.gamePrompt == GamePrompt::kWait
                         ? SpriteFrame::kGameMemoryReveal
                         : SpriteFrame::kGameMemoryCursor,
                     cx, top + 42);
      break;
    }
    case MiniGameKind::kSprint:
      display.drawLine(r.mainX + 12, bottom - 8, r.mainX + r.mainW - 12,
                       bottom - 8, EPD_BLACK);
      for (uint8_t i = 0; i < 5; i++) {
        display.drawLine(r.mainX + 15 + i * ((r.mainW - 30) / 4), bottom - 13,
                         r.mainX + 15 + i * ((r.mainW - 30) / 4), bottom - 3,
                         EPD_BLACK);
      }
      drawSmallFace(display, r.mainX + 12 + pet.gameCursor * ((r.mainW - 30) / 5),
                    top + 18, pet.gameRound);
      drawGameSprite(display, r,
                     (pet.gameRound & 1) ? SpriteFrame::kGameSprintRunner1
                                         : SpriteFrame::kGameSprintRunner0,
                     cx, top + 36);
      break;
    case MiniGameKind::kHoops:
      display.drawCircle(cx + 26, top + 20, 11, EPD_BLACK);
      display.drawLine(cx + 37, top + 20, cx + 37, bottom - 6, EPD_BLACK);
      display.fillCircle(cx - 32 + pet.gameCursor * 16, top + 24, 5,
                         EPD_BLACK);
      for (uint8_t i = 0; i < 3; i++) {
        display.drawRect(cx - 24 + i * 16, bottom - 15, 10, 8, EPD_BLACK);
        if (i == pet.gameTarget) {
          display.fillRect(cx - 22 + i * 16, bottom - 13, 6, 4, EPD_BLACK);
        }
      }
      drawGameSprite(display, r,
                     pet.gamePrompt == GamePrompt::kWait
                         ? SpriteFrame::kGameHoopsHoop
                         : SpriteFrame::kGameHoopsShoot,
                     cx - 4, top + 38);
      break;
    case MiniGameKind::kNone:
      break;
  }
  if (!r.compactText) {
    const uint8_t visibleScore =
        visualScoreDots(pet.gameScore, scoreLimit);
    drawScoreDots(display, r.mainX + 31, r.mainY + 95, visibleScore);
  }
  drawGamePromptBar(display, r, pet);
}

}  // namespace

void drawEchoPet(EchoPetDisplayDevice& display, const Snapshot& pet,
                 uint8_t selectedMenuIndex, const UiState& ui,
                 uint8_t animationPhase) {
  const ScreenResources& r = resourcesFor(display);
  display.fillScreen(EPD_WHITE);
  drawMenuColumn(display, r, selectedMenuIndex, false, pet, animationPhase);
  drawMenuColumn(display, r, selectedMenuIndex, true, pet, animationPhase);
  drawMainFrame(display, r);
  drawTopInfoLine(display, r, selectedMenuIndex, ui);

  if (drawCompactV3FunctionScreen(display, r, selectedMenuIndex, pet, ui,
                                  animationPhase)) {
    // Compact V3 scenes own the whole 32x30 projected playfield.
  } else if (pet.miniGame != MiniGameKind::kNone) {
    drawGameScreen(display, r, pet);
  } else if (ui.mode != UiMode::kHome) {
    drawPageScreen(display, r, pet, ui, animationPhase);
  } else {
    drawCharacter(display, r, pet, animationPhase);
    drawMess(display, r, pet, animationPhase);
  }
  if (r.compactText) {
    drawCompactInfoLine(display, r, pet, ui);
  } else {
    drawBottomInfoLine(display, r, pet, ui);
  }
}

#ifdef HOST_SCREEN_SIMULATOR
void drawEchoPetCatalogVisualProof(EchoPetDisplayDevice& display, bool souvenirs) {
  display.fillScreen(EPD_WHITE);
  drawText(display, 4, 4, souvenirs ? "CATALOG SOUVENIR VISUAL PROOF"
                                    : "CATALOG ITEM VISUAL PROOF");

  const uint8_t count = souvenirs ? kCatalogSouvenirCount : kCatalogItemCount;
  const uint8_t cols = souvenirs ? 8 : 8;
  const int16_t cellW = display.width() / cols;
  const int16_t cellH = 34;
  char label[40];
  char shortLabel[10];

  for (uint8_t i = 0; i < count; ++i) {
    const int16_t col = i % cols;
    const int16_t row = i / cols;
    const int16_t x = col * cellW;
    const int16_t y = 20 + row * cellH;
    display.drawRect(x, y, cellW, cellH, EPD_BLACK);
    snprintf(label, sizeof(label), "%03u", static_cast<unsigned>(i));
    drawText(display, x + 2, y + 2, label);

    if (souvenirs) {
      catalogSouvenirLabel(i, label, sizeof(label));
      drawSouvenirIcon(display, x + 4, y + 14, i, true, i, 1);
    } else {
      const CatalogEntry entry = catalogEntry(i);
      catalogItemLabel(i, label, sizeof(label));
      drawCatalogIcon(display, x + 4, y + 14, entry, i, i, 1);
    }

    memset(shortLabel, 0, sizeof(shortLabel));
    strncpy(shortLabel, label, sizeof(shortLabel) - 1);
    drawText(display, x + 24, y + 15, shortLabel);
  }
}

void drawEchoPetCharacterRosterProof(EchoPetDisplayDevice& display) {
  display.fillScreen(EPD_WHITE);
  drawText(display, 4, 4, "CHARACTER ROSTER VISUAL PROOF");

  const uint8_t cols = 5;
  const int16_t cellW = display.width() / cols;
  const int16_t cellH = 44;
  char line[40];
  char name[15];

  for (uint8_t i = 0; i < kCharacterCatalogCount; ++i) {
    const CharacterCatalogEntry& character = characterCatalogEntry(i);
    const int16_t col = i % cols;
    const int16_t row = i / cols;
    const int16_t x = col * cellW;
    const int16_t y = 20 + row * cellH;
    display.drawRect(x, y, cellW, cellH, EPD_BLACK);
    snprintf(line, sizeof(line), "%02u F%u P%u.%u",
             static_cast<unsigned>(i),
             static_cast<unsigned>(character.frameFamily),
             static_cast<unsigned>(character.sourcePage),
             static_cast<unsigned>(character.sourceSlot));
    drawText(display, x + 2, y + 2, line);
    drawCatalogAvatar(display, x + 4, y + 16, i, i, 1);

    memset(name, 0, sizeof(name));
    strncpy(name, character.name, sizeof(name) - 1);
    drawText(display, x + 24, y + 15, name);
  }
}

void drawEchoPetMenuIconProof(EchoPetDisplayDevice& display) {
  display.fillScreen(EPD_WHITE);
  drawText(display, 4, 4, "FIXED MENU ICON RESOURCE PROOF");
  drawText(display, 4, 24, "COMPACT 24x24");
  drawText(display, 4, 92, "LARGE 30x30");

  for (uint8_t i = 0; i < menuActionCount(); ++i) {
    const int16_t x = 4 + i * 63;
    const uint8_t iconIndex = menuIconAt(i);
    display.drawRect(x, 38, 56, 48, EPD_BLACK);
    drawText(display, x + 2, 40, menuLabelAt(i));
    drawMenuIcon(display, kEchoPetResources64, iconIndex, x + 16, 58,
                 EPD_BLACK);

    display.drawRect(x, 104, 56, 58, EPD_BLACK);
    drawText(display, x + 2, 106, menuLabelAt(i));
    drawMenuIcon(display, kEchoPetResources128, iconIndex, x + 13, 126,
                 EPD_BLACK);
  }
}
#endif

}  // namespace echopet
