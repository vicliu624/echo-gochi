/*
 * EchoPet - an original virtual pet for T-Echo-Lite nRF52840.
 *
 * The mechanics are inspired by the handheld virtual-pet genre, but the
 * character, art, state model, and code are original to this project.
 */

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <string.h>

#include "EchoPetDisplayDevice.h"
#include "EchoPetDisplay.h"
#include "EchoPetInput.h"
#include "EchoPetLink.h"
#include "EchoPetLoraTransport.h"
#include "EchoPetModel.h"
#include "EchoPetSave.h"
#include "EchoPetUi.h"
#include "echopet_target_config.h"

using echopet::Action;
using echopet::ConnectButtonEvent;
using echopet::EchoPetModel;
using echopet::FoodKind;
using echopet::FriendPacket;
using echopet::GameButton;
using echopet::Gender;
using echopet::GiftKind;
using echopet::ItemKind;
using echopet::LinkKind;
using echopet::LinkGameKind;
using echopet::MenuSlotRole;
using echopet::MiniGameKind;
using echopet::PetSave;
using echopet::UiMode;
using echopet::UiState;

constexpr uint32_t kAutosaveMs = 120000;
#if ECHOPET_TARGET_DISPLAY_SSD1306
constexpr uint32_t kIdleAnimationMs = 480;
constexpr uint32_t kToiletSceneAnimationMs = 100;
constexpr uint32_t kToiletCleanNoticeMinVisibleMs = 4600;
#else
constexpr uint32_t kIdleAnimationMs = 2200;
constexpr uint32_t kToiletSceneAnimationMs = 420;
constexpr uint32_t kToiletCleanNoticeMinVisibleMs = 4200;
#endif
constexpr uint32_t kSceneAnimationMs = 360;
constexpr uint32_t kFoodSceneAnimationMs = 500;
constexpr uint32_t kMedicineSceneAnimationMs = 500;
constexpr uint32_t kDisciplineSceneAnimationMs = 500;
constexpr uint32_t kLightsSceneAnimationMs = 500;
constexpr uint32_t kFamilySceneAnimationMs = 520;
constexpr uint32_t kLinkSceneAnimationMs = 360;
constexpr uint32_t kCatalogSceneAnimationMs = 420;
constexpr uint32_t kDisplaySubmitGuardMs = 50;
constexpr uint32_t kNoticeMinVisibleMs = 2600;
constexpr uint32_t kLinkStandbyTimeoutMs = 30000;
constexpr uint32_t kShopSecretTapWindowMs = 1200;
constexpr uint32_t kShopSecretInitialPauseWindowMs = 3000;
constexpr uint32_t kShopSecretSurpriseMs = 650;
constexpr uint32_t kFlagChordWindowMs = 140;
constexpr uint8_t kShopSecretTapGoal = 4;
constexpr uint8_t kShopSecretCodeLength = 8;
constexpr uint32_t kFrameBufferBytes =
    static_cast<uint32_t>(SCREEN_WIDTH) * static_cast<uint32_t>(SCREEN_HEIGHT) /
    8U;

enum LinkUiStatus : uint8_t {
  kLinkUiWaiting,
  kLinkUiSent,
  kLinkUiReceived,
  kLinkUiTimeout,
  kLinkUiCanceled,
  kLinkUiFailed,
};

#if ECHOPET_TARGET_DISPLAY_EPD
SPIClass screenSpi(NRF_SPIM1, SCREEN_MISO, SCREEN_SCLK, SCREEN_MOSI);
EchoPetDisplayDevice display(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_DC, SCREEN_RST,
                             SCREEN_CS, SCREEN_SRAM_CS, SCREEN_BUSY, &screenSpi,
                             8000000);
#elif ECHOPET_TARGET_DISPLAY_SSD1306
EchoPetDisplayDevice display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
#else
#error "Unsupported EchoPet display target"
#endif

EchoPetModel pet;
UiState ui;
uint8_t selectedMenuIndex = 0;
uint32_t lastSaveMs = 0;
uint32_t lastAnimationMs = 0;
uint32_t lastInputMs = 0;
uint32_t linkSessionStartMs = 0;
uint8_t animationPhase = 0;
bool dirty = true;
uint8_t previousFrame[kFrameBufferBytes];
bool previousFrameValid = false;
bool partialRefreshBaseMapReady = false;
uint32_t lastDisplaySubmitMs = 0;
bool noticeClearPending = false;
uint32_t noticeVisibleSinceMs = 0;
echopet::Notice noticeClearTarget = echopet::Notice::kNone;
uint8_t shopSecretTapCount = 0;
uint32_t lastShopSecretTapMs = 0;
uint32_t shopSecretSurpriseStartMs = 0;
bool flagChordPending = false;
GameButton flagChordButton = GameButton::kA;
uint32_t flagChordStartedMs = 0;

struct SetupDraft {
  uint8_t month = 1;
  uint8_t day = 1;
  uint8_t hour = 9;
  uint8_t minute = 0;
  uint8_t birthdayMonth = 6;
  uint8_t birthdayDay = 16;
  char userName[echopet::kNameChars + 1] = {'U', 'S', 'E', 'R', '1', '\0'};
  char petName[echopet::kNameChars + 1] = {'M', 'A', 'M', 'E', '1', '\0'};
  uint8_t gender = 0;
  uint8_t page = 0;
  uint8_t field = 0;
};

SetupDraft setupDraft;
bool setupInProgress = false;

static void beginDisplay() {
#if ECHOPET_TARGET_DISPLAY_SSD1306
  Wire.setPins(ECHOPET_OLED_I2C_SDA, ECHOPET_OLED_I2C_SCL);
  Wire.begin();
  Wire.setClock(400000);
  if (!display.begin(SSD1306_SWITCHCAPVCC, ECHOPET_OLED_I2C_ADDRESS, true,
                     false)) {
    Serial.println("EchoPet display begin failed");
  }
  display.setTextWrap(false);
  display.clearDisplay();
  display.display();
#else
  display.begin();
  display.setRotation(1);
  display.setTextWrap(false);
  display.fillScreen(EPD_WHITE);
  display.clearBuffer();
#endif
  previousFrameValid = false;
  partialRefreshBaseMapReady = false;
  lastDisplaySubmitMs = 0;
}

static void powerOnBoard() {
#if ECHOPET_PERIPHERAL_POWER_EN >= 0
  pinMode(ECHOPET_PERIPHERAL_POWER_EN, OUTPUT);
  digitalWrite(ECHOPET_PERIPHERAL_POWER_EN, HIGH);
#endif

#if ECHOPET_TARGET_DISPLAY_EPD
  pinMode(SCREEN_BS1, OUTPUT);
  digitalWrite(SCREEN_BS1, LOW);
#endif

#if ECHOPET_STATUS_LED >= 0
  pinMode(ECHOPET_STATUS_LED, OUTPUT);
  digitalWrite(ECHOPET_STATUS_LED,
               ECHOPET_STATUS_LED_ACTIVE_HIGH ? HIGH : LOW);
#endif
}

static void savePet() {
  PetSave save;
  pet.exportSave(save);
  if (echopet::writeSave(save)) {
    lastSaveMs = millis();
    Serial.println("EchoPet save OK");
  } else {
    Serial.println("EchoPet save failed");
  }
}

static bool eventToGameButton(ConnectButtonEvent event, GameButton& button) {
  switch (event) {
    case ConnectButtonEvent::kA:
    case ConnectButtonEvent::kALong:
      button = GameButton::kA;
      return true;
    case ConnectButtonEvent::kB:
    case ConnectButtonEvent::kBLong:
      button = GameButton::kB;
      return true;
    case ConnectButtonEvent::kC:
    case ConnectButtonEvent::kCLong:
      button = GameButton::kC;
      return true;
    case ConnectButtonEvent::kNone:
      return false;
  }
  return false;
}

static bool isFlagChordSide(GameButton button) {
  return button == GameButton::kA || button == GameButton::kC;
}

static void submitGameButton(GameButton button, uint32_t nowMs) {
  pet.gameInput(button, nowMs);
  dirty = true;
  if (!pet.isGameActive()) {
    savePet();
  }
}

static bool flushFlagChord(uint32_t nowMs) {
  if (!flagChordPending) {
    return false;
  }
  const GameButton button = flagChordButton;
  flagChordPending = false;
  submitGameButton(button, nowMs);
  return true;
}

static void clearFlagChord() {
  flagChordPending = false;
}

static void resetShopSecretGesture() {
  shopSecretTapCount = 0;
  lastShopSecretTapMs = 0;
  shopSecretSurpriseStartMs = 0;
}

static void cancelShopSecretGesture() {
  resetShopSecretGesture();
  ui.secretCode = false;
  ui.entry[0] = 0;
}

static char setupCharAt(uint8_t index) {
  index %= 36;
  if (index < 26) return static_cast<char>('A' + index);
  return static_cast<char>('0' + (index - 26));
}

static uint8_t setupCharIndex(char c) {
  if (c >= 'a' && c <= 'z') c = static_cast<char>(c - 'a' + 'A');
  if (c >= 'A' && c <= 'Z') return static_cast<uint8_t>(c - 'A');
  if (c >= '0' && c <= '9') return static_cast<uint8_t>(26 + c - '0');
  return 0;
}

static uint8_t setupDaysInMonth(uint8_t month) {
  static const uint8_t kDays[] = {
      31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,
  };
  if (month < 1) month = 1;
  if (month > 12) month = 12;
  return kDays[month - 1];
}

static uint8_t setupFieldCount(uint8_t page) {
  switch (page) {
    case 0:
      return 4;  // month, day, hour, minute
    case 1:
      return 2;  // birthday month/day
    case 2:
      return echopet::kNameChars;
    case 3:
      return 1;  // gender reveal
    case 4:
      return echopet::kNameChars;
    default:
      return 1;  // hatch confirmation
  }
}

static void syncSetupUi() {
  ui.mode = UiMode::kSetup;
  ui.page = setupDraft.page;
  ui.cursor = setupDraft.field;
  memset(ui.entry, 0, sizeof(ui.entry));
  switch (setupDraft.page) {
    case 0:
      ui.entry[0] = setupDraft.month;
      ui.entry[1] = setupDraft.day;
      ui.entry[2] = setupDraft.hour;
      ui.entry[3] = setupDraft.minute;
      break;
    case 1:
      ui.entry[0] = setupDraft.birthdayMonth;
      ui.entry[1] = setupDraft.birthdayDay;
      break;
    case 2:
      for (uint8_t i = 0; i < echopet::kNameChars; i++) {
        ui.entry[i] = setupCharIndex(setupDraft.userName[i]);
      }
      break;
    case 3:
      ui.entry[0] = setupDraft.gender;
      break;
    case 4:
      for (uint8_t i = 0; i < echopet::kNameChars; i++) {
        ui.entry[i] = setupCharIndex(setupDraft.petName[i]);
      }
      break;
    default:
      break;
  }
}

static void enterSetupFlow() {
  setupDraft = SetupDraft();
  setupDraft.gender = static_cast<uint8_t>((micros() ^ millis()) & 0x01);
  setupInProgress = true;
  syncSetupUi();
}

static void incrementSetupField() {
  switch (setupDraft.page) {
    case 0:
      if (setupDraft.field == 0) {
        setupDraft.month = setupDraft.month >= 12 ? 1 : setupDraft.month + 1;
        if (setupDraft.day > setupDaysInMonth(setupDraft.month)) {
          setupDraft.day = setupDaysInMonth(setupDraft.month);
        }
      } else if (setupDraft.field == 1) {
        const uint8_t maxDay = setupDaysInMonth(setupDraft.month);
        setupDraft.day = setupDraft.day >= maxDay ? 1 : setupDraft.day + 1;
      } else if (setupDraft.field == 2) {
        setupDraft.hour = (setupDraft.hour + 1) % 24;
      } else {
        setupDraft.minute = (setupDraft.minute + 1) % 60;
      }
      break;
    case 1:
      if (setupDraft.field == 0) {
        setupDraft.birthdayMonth =
            setupDraft.birthdayMonth >= 12 ? 1 : setupDraft.birthdayMonth + 1;
        if (setupDraft.birthdayDay >
            setupDaysInMonth(setupDraft.birthdayMonth)) {
          setupDraft.birthdayDay =
              setupDaysInMonth(setupDraft.birthdayMonth);
        }
      } else {
        const uint8_t maxDay = setupDaysInMonth(setupDraft.birthdayMonth);
        setupDraft.birthdayDay =
            setupDraft.birthdayDay >= maxDay ? 1 : setupDraft.birthdayDay + 1;
      }
      break;
    case 2:
      setupDraft.userName[setupDraft.field] =
          setupCharAt(setupCharIndex(setupDraft.userName[setupDraft.field]) + 1);
      break;
    case 3:
      break;
    case 4:
      setupDraft.petName[setupDraft.field] =
          setupCharAt(setupCharIndex(setupDraft.petName[setupDraft.field]) + 1);
      break;
    default:
      break;
  }
  syncSetupUi();
}

static void advanceSetupField() {
  if (setupDraft.page >= 5) {
    pet.configureSetup(setupDraft.month, setupDraft.day, setupDraft.hour,
                       setupDraft.minute, setupDraft.birthdayMonth,
                       setupDraft.birthdayDay, setupDraft.userName,
                       setupDraft.petName,
                       setupDraft.gender ? Gender::kGirl : Gender::kBoy);
    setupInProgress = false;
    echopet::uiExit(ui);
    savePet();
    return;
  }

  setupDraft.field++;
  if (setupDraft.field >= setupFieldCount(setupDraft.page)) {
    setupDraft.page++;
    setupDraft.field = 0;
  }
  syncSetupUi();
}

static void backSetupField() {
  if (setupDraft.field > 0) {
    setupDraft.field--;
  } else if (setupDraft.page > 0) {
    setupDraft.page--;
    setupDraft.field = setupFieldCount(setupDraft.page) - 1;
  }
  syncSetupUi();
}

static void handleSetupInput(ConnectButtonEvent event) {
  switch (event) {
    case ConnectButtonEvent::kA:
    case ConnectButtonEvent::kALong:
      incrementSetupField();
      dirty = true;
      break;
    case ConnectButtonEvent::kB:
    case ConnectButtonEvent::kBLong:
      advanceSetupField();
      dirty = true;
      break;
    case ConnectButtonEvent::kC:
    case ConnectButtonEvent::kCLong:
      backSetupField();
      dirty = true;
      break;
    case ConnectButtonEvent::kNone:
      break;
  }
}

static bool emitFriendPacket(const FriendPacket& packet) {
  char line[echopet::kFriendLineBufferSize];
  if (echopet::encodeFriendPacketHex(packet, line, sizeof(line))) {
    Serial.println(line);
    echopet::sendLoraFriendPacket(packet);
    return true;
  }
  return false;
}

static bool printFriendPacket(LinkKind kind, bool updateNotice = true) {
  FriendPacket packet;
  if (!pet.prepareFriendPacket(kind, packet, updateNotice)) {
    return false;
  }
  return emitFriendPacket(packet);
}

static bool printGiftPacket(const UiState& state, bool updateNotice = true) {
  FriendPacket packet;
  bool ok = false;
  if (state.cursor < echopet::kCatalogItemCount) {
    ok = pet.prepareCatalogGiftPacket(state.cursor, packet, updateNotice);
  } else {
    ok = pet.prepareGiftPacket(GiftKind::kPoints, 0, 5, packet, updateNotice);
  }
  if (!ok) {
    return false;
  }
  return emitFriendPacket(packet);
}

static bool printLinkGamePacket(const UiState& state,
                                bool updateNotice = true) {
  FriendPacket packet;
  const LinkGameKind game = echopet::linkGameFromIndex(state.cursor);
  if (!pet.prepareLinkGamePacket(game, packet, updateNotice)) {
    return false;
  }
  return emitFriendPacket(packet);
}

static bool emitLinkPacketForSelection(LinkKind kind, uint8_t selectionIndex,
                                       bool reply) {
  UiState selection = ui;
  selection.cursor = selectionIndex;
  switch (kind) {
    case LinkKind::kVisit:
      return printFriendPacket(LinkKind::kVisit, !reply);
    case LinkKind::kPresent:
      if (reply) {
        if (printGiftPacket(selection, false)) {
          return true;
        }
        return printFriendPacket(LinkKind::kPresent, false);
      }
      return printGiftPacket(selection, true);
    case LinkKind::kGame:
      if (reply) {
        if (printLinkGamePacket(selection, false)) {
          return true;
        }
        return printFriendPacket(LinkKind::kGame, false);
      }
      return printLinkGamePacket(selection, true);
    case LinkKind::kLove:
      if (reply) {
        FriendPacket packet;
        return pet.prepareLinkReplyPacket(LinkKind::kLove, packet) &&
               emitFriendPacket(packet);
      }
      return printFriendPacket(LinkKind::kLove);
  }
  return false;
}

static bool acceptIncomingFriendPacket(const FriendPacket& packet,
                                       const char* carrier) {
  const bool standby = ui.mode == UiMode::kLinkStandby;
  const bool awaitingReply =
      ui.mode == UiMode::kLinkResult && ui.entry[2] == kLinkUiSent &&
      (millis() - linkSessionStartMs) < kLinkStandbyTimeoutMs;
  if (!standby && !awaitingReply) {
    Serial.print("EchoPet ");
    Serial.print(carrier);
    Serial.println(" friend packet ignored outside LINK receive window");
    return false;
  }
  if (packet.kind != ui.entry[0]) {
    Serial.print("EchoPet ");
    Serial.print(carrier);
    Serial.println(" friend packet rejected for mismatched LINK kind");
    enterLinkResult(kLinkUiFailed);
    dirty = true;
    return false;
  }
  if (pet.receiveFriendPacket(packet)) {
    Serial.print("EchoPet ");
    Serial.print(carrier);
    Serial.println(" friend packet accepted");
    if (standby) {
      const LinkKind kind = static_cast<LinkKind>(ui.entry[0]);
      uint8_t replySelection = ui.entry[1];
      if (kind == LinkKind::kGame && packet.gameKind > 0) {
        replySelection = packet.gameKind - 1;
      }
      if (emitLinkPacketForSelection(kind, replySelection, true)) {
        Serial.print("EchoPet ");
        Serial.print(carrier);
        Serial.println(" link reply emitted");
      }
    }
    enterLinkResult(kLinkUiReceived);
    dirty = true;
    return true;
  }
  enterLinkResult(kLinkUiFailed);
  dirty = true;
  return false;
}

static bool readLoraLink() {
  FriendPacket packet;
  if (echopet::pollLoraFriendPacket(packet)) {
    return acceptIncomingFriendPacket(packet, "LoRa");
  }
  return false;
}

static bool readSerialLink() {
  static char line[echopet::kFriendLineBufferSize];
  static uint8_t length = 0;
  bool accepted = false;

  while (Serial.available() > 0) {
    const char c = static_cast<char>(Serial.read());
    if (c == '\r') {
      continue;
    }
    if (c == '\n') {
      line[length] = '\0';
      FriendPacket packet;
      if (echopet::decodeFriendPacketHex(line, packet)) {
        accepted = acceptIncomingFriendPacket(packet, "Serial") || accepted;
      }
      length = 0;
      continue;
    }
    if (length < sizeof(line) - 1) {
      line[length++] = c;
    } else {
      length = 0;
    }
  }
  return accepted;
}

static void loadOrCreatePet() {
  PetSave save;
  if (echopet::loadSave(save) && pet.load(save, millis())) {
    setupInProgress = false;
    Serial.println("EchoPet save loaded");
    return;
  }

  const uint32_t seed = micros() ^ 0xEC2026UL;
  pet.reset(seed, millis());
  enterSetupFlow();
  Serial.println("EchoPet setup required for new save");
}

static const uint8_t* currentDisplayFrame() {
#if ECHOPET_TARGET_DISPLAY_SSD1306
  return display.getBuffer();
#else
  return display.getBlackBuffer();
#endif
}

static void rememberFrame(const uint8_t* frame) {
  if (!frame) {
    previousFrameValid = false;
    return;
  }
  memcpy(previousFrame, frame, kFrameBufferBytes);
  previousFrameValid = true;
}

static void armNoticeClear(const echopet::Snapshot& snapshot) {
  if (snapshot.notice == echopet::Notice::kNone) {
    return;
  }
  if (!noticeClearPending || noticeClearTarget != snapshot.notice) {
    noticeClearTarget = snapshot.notice;
    noticeVisibleSinceMs = millis();
  }
  noticeClearPending = true;
}

static bool frameChanged(const uint8_t* frame) {
  return frame && previousFrameValid &&
         memcmp(frame, previousFrame, kFrameBufferBytes) != 0;
}

static void waitForDisplaySubmitInterval() {
  if (lastDisplaySubmitMs == 0) {
    return;
  }
  const uint32_t elapsed = millis() - lastDisplaySubmitMs;
  if (elapsed < kDisplaySubmitGuardMs) {
    delay(kDisplaySubmitGuardMs - elapsed);
  }
}

static void performFullRefresh(const uint8_t* frame) {
  waitForDisplaySubmitInterval();
#if ECHOPET_TARGET_DISPLAY_SSD1306
  display.display();
#else
  display.display(Adafruit_EPD::Update_Mode::FULL_REFRESH, true);
#endif
  rememberFrame(frame);
  partialRefreshBaseMapReady = false;
  lastDisplaySubmitMs = millis();
}

static bool shouldRunIdleAnimation(uint32_t nowMs) {
  (void)nowMs;
  if (pet.isGameActive()) {
    return true;
  }
  if (ui.mode != UiMode::kHome || setupInProgress) {
    return true;
  }

  return true;
}

static uint32_t gameAnimationIntervalMs(MiniGameKind game) {
  switch (game) {
    case MiniGameKind::kHeading:
    case MiniGameKind::kHoops:
    case MiniGameKind::kBump:
    case MiniGameKind::kSprint:
      return 250;
    case MiniGameKind::kGet:
    case MiniGameKind::kFlag:
      return 260;
    case MiniGameKind::kMemory:
      return 300;
    case MiniGameKind::kNone:
      return kSceneAnimationMs;
  }
  return kSceneAnimationMs;
}

static uint32_t animationIntervalMs() {
  const echopet::Snapshot snapshot = pet.snapshot();
  if (snapshot.miniGame != MiniGameKind::kNone) {
    return gameAnimationIntervalMs(snapshot.miniGame);
  }
  switch (snapshot.notice) {
    case echopet::Notice::kMeal:
    case echopet::Notice::kSnack:
      return kFoodSceneAnimationMs;
    case echopet::Notice::kClean:
      return kToiletSceneAnimationMs;
    case echopet::Notice::kMedicine:
    case echopet::Notice::kNeedMoreMedicine:
      return kMedicineSceneAnimationMs;
    case echopet::Notice::kTrain:
    case echopet::Notice::kPraise:
      return kDisciplineSceneAnimationMs;
    case echopet::Notice::kLightsOn:
    case echopet::Notice::kLightsOff:
      return kLightsSceneAnimationMs;
    default:
      break;
  }
  switch (ui.mode) {
    case UiMode::kFoodMenu:
    case UiMode::kMeal:
    case UiMode::kSnack:
      return kFoodSceneAnimationMs;
    case UiMode::kToilet:
      return kToiletSceneAnimationMs;
    case UiMode::kMedicine:
      return kMedicineSceneAnimationMs;
    case UiMode::kDisciplineMenu:
    case UiMode::kDiscipline:
      return kDisciplineSceneAnimationMs;
    case UiMode::kLights:
      return kLightsSceneAnimationMs;
    case UiMode::kFriends:
    case UiMode::kFamily:
      return kFamilySceneAnimationMs;
    case UiMode::kShop:
    case UiMode::kPassword:
    case UiMode::kItem:
    case UiMode::kSouvenirs:
    case UiMode::kPoint:
      return kCatalogSceneAnimationMs;
    case UiMode::kVisitLink:
    case UiMode::kPresent:
    case UiMode::kLinkGame:
    case UiMode::kLinkStandby:
    case UiMode::kLinkResult:
      return kLinkSceneAnimationMs;
    case UiMode::kSetup:
      return kSceneAnimationMs;
    default:
      return kIdleAnimationMs;
  }
}

static bool handleActiveGameInput(ConnectButtonEvent event, uint32_t nowMs) {
  const echopet::Snapshot snapshot = pet.snapshot();
  if (snapshot.miniGame != MiniGameKind::kFlag) {
    clearFlagChord();
    GameButton button = GameButton::kA;
    if (!eventToGameButton(event, button)) {
      return false;
    }
    submitGameButton(button, nowMs);
    return true;
  }

  if (event == ConnectButtonEvent::kNone) {
    if (flagChordPending &&
        (nowMs - flagChordStartedMs) >= kFlagChordWindowMs) {
      return flushFlagChord(nowMs);
    }
    return false;
  }

  GameButton button = GameButton::kA;
  if (!eventToGameButton(event, button)) {
    return false;
  }

  const bool waitingForBoth =
      snapshot.gameHazard == 0 &&
      snapshot.gameExpectedYes == static_cast<uint8_t>(GameButton::kAC);
  if (!waitingForBoth || !isFlagChordSide(button)) {
    if (flagChordPending) {
      return flushFlagChord(nowMs);
    }
    submitGameButton(button, nowMs);
    return true;
  }

  if (!flagChordPending) {
    flagChordPending = true;
    flagChordButton = button;
    flagChordStartedMs = nowMs;
    return false;
  }

  if (button != flagChordButton &&
      (nowMs - flagChordStartedMs) <= kFlagChordWindowMs) {
    flagChordPending = false;
    submitGameButton(GameButton::kAC, nowMs);
    return true;
  }

  if ((nowMs - flagChordStartedMs) >= kFlagChordWindowMs) {
    return flushFlagChord(nowMs);
  }
  return false;
}

static void refreshDisplay(bool fullRefresh) {
  const uint8_t* frame = currentDisplayFrame();
  if (fullRefresh || !previousFrameValid) {
    performFullRefresh(frame);
    return;
  }

  if (!frameChanged(frame)) {
    return;
  }

  waitForDisplaySubmitInterval();
#if ECHOPET_TARGET_DISPLAY_SSD1306
  display.display();
#else
  if (!partialRefreshBaseMapReady) {
    display.setRAMValueBaseMap(Adafruit_EPD::Update_Mode::FAST_REFRESH);
    partialRefreshBaseMapReady = true;
  }
  display.display(Adafruit_EPD::Update_Mode::PARTIAL_REFRESH, true);
#endif
  rememberFrame(frame);
  lastDisplaySubmitMs = millis();
}

static void pumpDisplayRefresh() {
  // Refresh is now synchronous, matching the LilyGo-proven trail-mate path.
}

static uint32_t noticeMinimumVisibleMs(echopet::Notice notice) {
  return notice == echopet::Notice::kClean ? kToiletCleanNoticeMinVisibleMs
                                           : kNoticeMinVisibleMs;
}

static void resetAnimationPhase() {
  animationPhase = 0;
  lastAnimationMs = 0;
}

static void maybeClearRenderedNotice(uint32_t nowMs) {
  if (!noticeClearPending) {
    return;
  }
  if ((nowMs - noticeVisibleSinceMs) <
      noticeMinimumVisibleMs(noticeClearTarget)) {
    return;
  }
  const echopet::Snapshot snapshot = pet.snapshot();
  if (snapshot.notice == noticeClearTarget) {
    pet.clearNotice();
    if (ui.mode == UiMode::kToilet &&
        (noticeClearTarget == echopet::Notice::kClean ||
         noticeClearTarget == echopet::Notice::kNoMess)) {
      echopet::uiExit(ui);
      resetAnimationPhase();
    }
    dirty = true;
  }
  noticeClearPending = false;
  noticeClearTarget = echopet::Notice::kNone;
}

static void render(bool fullRefresh = false) {
  const echopet::Snapshot snapshot = pet.snapshot();
  echopet::drawEchoPet(display, snapshot, selectedMenuIndex, ui, animationPhase);
  animationPhase++;
  refreshDisplay(fullRefresh);
  armNoticeClear(snapshot);
  lastAnimationMs = millis();
  dirty = false;
}

static void selectNextAction() {
  uint8_t next = selectedMenuIndex + 1;
  if (next >= echopet::menuActionCount()) {
    next = 0;
  }
  selectedMenuIndex = next;
  dirty = true;
}

static Action selectedMenuAction() {
  if (echopet::menuRoleAt(selectedMenuIndex) == MenuSlotRole::kCareCall) {
    const echopet::Snapshot snapshot = pet.snapshot();
    switch (snapshot.attentionReason) {
      case echopet::AttentionReason::kDirty:
        return Action::kToilet;
      case echopet::AttentionReason::kSick:
        return Action::kMedicine;
      case echopet::AttentionReason::kNaughty:
      case echopet::AttentionReason::kPraise:
        return Action::kDiscipline;
      case echopet::AttentionReason::kHungry:
      case echopet::AttentionReason::kSad:
      case echopet::AttentionReason::kNone:
        return Action::kHealth;
    }
  }
  return echopet::menuActionAt(selectedMenuIndex);
}

static bool isImmediateSceneAction(Action action) {
  return action == Action::kToilet || action == Action::kMedicine ||
         action == Action::kLights;
}

static void enterUiMode(UiMode mode) {
  resetShopSecretGesture();
  ui.mode = mode;
  ui.cursor = 0;
  ui.page = 0;
  ui.secretCode = false;
  memset(ui.entry, 0, sizeof(ui.entry));
  resetAnimationPhase();
}

static void enterClockSetMode() {
  const echopet::Snapshot snapshot = pet.snapshot();
  enterUiMode(UiMode::kClockSet);
  ui.entry[0] = snapshot.hour % 24;
  ui.entry[1] = snapshot.minute % 60;
}

static void incrementClockSetField() {
  if (ui.cursor == 0) {
    ui.entry[0] = (ui.entry[0] + 1) % 24;
  } else {
    ui.entry[1] = (ui.entry[1] + 1) % 60;
  }
  ui.page = ui.cursor;
}

static bool confirmClockSetField() {
  if (ui.cursor == 0) {
    ui.cursor = 1;
    ui.page = 1;
    return false;
  }
  const bool changed = pet.setClock(ui.entry[0], ui.entry[1]);
  echopet::uiExit(ui);
  return changed;
}

static bool handleShopSecretTap(uint32_t nowMs) {
  if (ui.mode != UiMode::kShop) {
    resetShopSecretGesture();
    return false;
  }
  if (ui.secretCode) {
    return true;
  }
  const uint32_t window =
      shopSecretTapCount <= 1 ? kShopSecretInitialPauseWindowMs
                              : kShopSecretTapWindowMs;
  if (shopSecretTapCount > 0 && (nowMs - lastShopSecretTapMs) > window) {
    shopSecretTapCount = 0;
  }
  lastShopSecretTapMs = nowMs;
  if (shopSecretTapCount < kShopSecretTapGoal) {
    shopSecretTapCount++;
  }
  ui.entry[0] = shopSecretTapCount;
  if (shopSecretTapCount >= kShopSecretTapGoal) {
    ui.secretCode = true;
    shopSecretSurpriseStartMs = nowMs;
    return true;
  }
  return false;
}

static void updateShopSecretTransition(uint32_t nowMs) {
  if (ui.mode != UiMode::kShop) {
    return;
  }
  if (!ui.secretCode) {
    const uint32_t window =
        shopSecretTapCount <= 1 ? kShopSecretInitialPauseWindowMs
                                : kShopSecretTapWindowMs;
    if (shopSecretTapCount > 0 &&
        (nowMs - lastShopSecretTapMs) > window) {
      resetShopSecretGesture();
      ui.entry[0] = 0;
      dirty = true;
    }
    return;
  }
  if ((nowMs - shopSecretSurpriseStartMs) < kShopSecretSurpriseMs) {
    return;
  }
  enterUiMode(UiMode::kPassword);
  ui.secretCode = true;
  ui.cursor = 0;
  ui.page = 0;
  memset(ui.entry, 0, sizeof(ui.entry));
  dirty = true;
}

static void appendSecretCodeSymbol(uint8_t symbol) {
  if (ui.mode != UiMode::kPassword || !ui.secretCode) {
    return;
  }
  if (ui.page >= kShopSecretCodeLength) {
    ui.page = 0;
  }
  ui.entry[ui.page] = symbol % 3;
  if (ui.page + 1 < kShopSecretCodeLength) {
    ui.page++;
    ui.cursor = 0;
    dirty = true;
    return;
  }

  const bool changed = pet.enterSecretCode(ui.entry);
  ui.secretCode = false;
  ui.page = 0;
  ui.cursor = 0;
  memset(ui.entry, 0, sizeof(ui.entry));
  if (changed) {
    savePet();
  }
  dirty = true;
}

static void enterLinkStandby(LinkKind kind, uint8_t selection) {
  ui.mode = UiMode::kLinkStandby;
  ui.cursor = 0;
  ui.page = 0;
  memset(ui.entry, 0, sizeof(ui.entry));
  ui.entry[0] = static_cast<uint8_t>(kind);
  ui.entry[1] = selection;
  ui.entry[2] = kLinkUiWaiting;
  linkSessionStartMs = millis();
}

static void enterLinkResult(LinkUiStatus status) {
  ui.mode = UiMode::kLinkResult;
  ui.cursor = 0;
  ui.page = 0;
  ui.entry[2] = status;
  linkSessionStartMs = millis();
}

static bool sendLinkSessionPacket() {
  const LinkKind kind = static_cast<LinkKind>(ui.entry[0]);
  const bool ok = emitLinkPacketForSelection(kind, ui.entry[1], false);
  enterLinkResult(ok ? kLinkUiSent : kLinkUiFailed);
  return ok;
}

static void activateSelectedAction() {
  const Action selectedAction = selectedMenuAction();
  if (echopet::menuRoleAt(selectedMenuIndex) == MenuSlotRole::kCareCall) {
    selectedMenuIndex = echopet::menuActionIndex(selectedAction);
  }
  if (isImmediateSceneAction(selectedAction)) {
    const uint8_t toiletMessCountBefore =
        selectedAction == Action::kToilet ? pet.snapshot().messCount : 0;
    const bool changed = pet.apply(selectedAction);
    echopet::uiEnter(ui, selectedAction);
    if (selectedAction == Action::kToilet && changed) {
      ui.entry[0] = toiletMessCountBefore;
    }
    resetAnimationPhase();
    if (changed) {
      savePet();
    }
    dirty = true;
    return;
  }

  if (echopet::actionHasPage(selectedAction)) {
    echopet::uiEnter(ui, selectedAction);
    resetAnimationPhase();
    dirty = true;
    return;
  }

  pet.apply(selectedAction);
  savePet();
  dirty = true;
}

static void showStatus() {
  selectedMenuIndex = 0;
  echopet::uiEnter(ui, Action::kHealth);
  resetAnimationPhase();
  dirty = true;
}

static void activateUiSelection() {
  bool changed = false;
  switch (ui.mode) {
    case UiMode::kHealth:
      echopet::uiNext(ui, pet.snapshot());
      break;
    case UiMode::kFoodMenu:
      enterUiMode(ui.cursor == 0 ? UiMode::kMeal : UiMode::kSnack);
      break;
    case UiMode::kMeal:
      changed = pet.feed(static_cast<FoodKind>(
          ui.cursor % echopet::kDefaultMealCount));
      break;
    case UiMode::kSnack:
      changed = pet.snack(static_cast<FoodKind>(
          echopet::kDefaultMealCount +
          (ui.cursor % echopet::kDefaultSnackCount)));
      break;
    case UiMode::kActivityMenu:
      switch (ui.cursor % 6) {
        case 0:
          enterUiMode(UiMode::kGame);
          break;
        case 1:
          enterUiMode(UiMode::kItem);
          break;
        case 2:
          enterUiMode(UiMode::kShop);
          break;
        case 3:
          enterUiMode(UiMode::kPassword);
          break;
        case 4:
          enterUiMode(UiMode::kSouvenirs);
          break;
        default:
          enterUiMode(UiMode::kPoint);
          break;
      }
      break;
    case UiMode::kGame:
      changed = pet.startGame(static_cast<MiniGameKind>((ui.cursor % 7) + 1));
      if (pet.isGameActive()) echopet::uiExit(ui);
      break;
    case UiMode::kShop:
      changed = pet.buyShopSlot(ui.cursor % echopet::kShopSlotCount);
      break;
    case UiMode::kItem:
      changed = pet.useCatalogItem(ui.cursor % echopet::kCatalogItemCount);
      break;
    case UiMode::kPoint:
      changed = pet.apply(Action::kDonate);
      break;
    case UiMode::kConnectionMenu:
      switch (ui.cursor % 4) {
        case 0:
          enterUiMode(UiMode::kLinkGame);
          break;
        case 1:
          enterUiMode(UiMode::kPresent);
          break;
        case 2:
          enterUiMode(UiMode::kVisitLink);
          break;
        default:
          enterLinkStandby(LinkKind::kLove, 0);
          break;
      }
      break;
    case UiMode::kVisitLink:
      enterLinkStandby(LinkKind::kVisit, 0);
      break;
    case UiMode::kFriends:
      echopet::uiNext(ui, pet.snapshot());
      break;
    case UiMode::kFriendDeleteConfirm:
      changed = pet.deleteFriend(ui.cursor);
      ui.mode = UiMode::kFriends;
      ui.cursor = 0;
      ui.page = 0;
      break;
    case UiMode::kFamily:
      changed = pet.apply(Action::kFamily);
      break;
    case UiMode::kSouvenirs:
      break;
    case UiMode::kPresent:
      enterLinkStandby(LinkKind::kPresent, ui.cursor);
      break;
    case UiMode::kLinkGame:
      enterLinkStandby(LinkKind::kGame, ui.cursor);
      break;
    case UiMode::kLinkStandby:
      changed = sendLinkSessionPacket();
      break;
    case UiMode::kLinkResult:
      echopet::uiExit(ui);
      break;
    case UiMode::kPassword:
      if (ui.page < sizeof(ui.entry)) {
        ui.entry[ui.page] = ui.cursor;
      }
      if (ui.page + 1 < static_cast<uint8_t>(sizeof(ui.entry))) {
        ui.page++;
        ui.cursor = ui.entry[ui.page];
      } else {
        changed = pet.enterPassword(ui.entry);
        ui.page = 0;
        ui.cursor = 0;
        ui.secretCode = false;
        memset(ui.entry, 0, sizeof(ui.entry));
      }
      break;
    case UiMode::kDisciplineMenu:
      changed = pet.apply(ui.cursor == 0 ? Action::kDiscipline
                                         : Action::kPraise);
      enterUiMode(UiMode::kDiscipline);
      break;
    case UiMode::kToilet:
    case UiMode::kMedicine:
    case UiMode::kLights:
    case UiMode::kDiscipline: {
      changed = false;
      break;
    }
    case UiMode::kResetConfirm:
      changed = pet.apply(Action::kReset);
      if (changed) echopet::uiExit(ui);
      break;
    case UiMode::kSpriteProof:
      echopet::uiNext(ui, pet.snapshot());
      break;
    case UiMode::kClockSet:
      changed = confirmClockSetField();
      break;
    case UiMode::kSetup:
      break;
    case UiMode::kHome:
      break;
  }
  if (changed) {
    savePet();
  }
  dirty = true;
}

static void updateLinkStandby() {
  if (ui.mode == UiMode::kLinkStandby &&
      (millis() - linkSessionStartMs) >= kLinkStandbyTimeoutMs) {
    enterLinkResult(kLinkUiTimeout);
    dirty = true;
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("EchoPet boot");

  powerOnBoard();
  const bool keyShield = echopet::beginEchoPetInput();
#if defined(ECHOPET_TARGET_GAT562) && ECHOPET_TARGET_GAT562
  (void)keyShield;
  Serial.println("EchoPet input: GAT562 joystick L/U/R");
#else
  Serial.println(keyShield ? "EchoPet input: Esc/Home/Mail"
                           : "EchoPet input: BOOT fallback");
#endif
  beginDisplay();

  loadOrCreatePet();
  echopet::beginLoraTransport();
  pet.begin(millis());
  lastInputMs = millis();
  render(true);
}

void loop() {
  const uint32_t nowMs = millis();
  const ConnectButtonEvent event = echopet::pollEchoPetInput();
  if (event != ConnectButtonEvent::kNone) {
    lastInputMs = nowMs;
    Serial.print("EchoPet button ");
    Serial.println(echopet::connectButtonLabel(event));
  }
  if (ui.mode == UiMode::kSetup || setupInProgress) {
    handleSetupInput(event);
    if (!dirty && (nowMs - lastAnimationMs) >= animationIntervalMs()) {
      dirty = true;
    }
    if (dirty) {
      render();
    }
    pumpDisplayRefresh();
    maybeClearRenderedNotice(nowMs);
    delay(20);
    return;
  }

  if (pet.isGameActive()) {
    handleActiveGameInput(event, nowMs);
  } else if (ui.mode != UiMode::kHome) {
    if (ui.mode == UiMode::kClockSet) {
      switch (event) {
        case ConnectButtonEvent::kA:
        case ConnectButtonEvent::kALong:
          incrementClockSetField();
          dirty = true;
          break;
        case ConnectButtonEvent::kB:
        case ConnectButtonEvent::kBLong:
          if (confirmClockSetField()) {
            savePet();
          }
          dirty = true;
          break;
        case ConnectButtonEvent::kC:
        case ConnectButtonEvent::kCLong:
          echopet::uiExit(ui);
          dirty = true;
          break;
        case ConnectButtonEvent::kNone:
          break;
      }
    } else if (ui.mode == UiMode::kPassword && ui.secretCode) {
      switch (event) {
        case ConnectButtonEvent::kA:
        case ConnectButtonEvent::kALong:
          appendSecretCodeSymbol(0);
          break;
        case ConnectButtonEvent::kB:
        case ConnectButtonEvent::kBLong:
          appendSecretCodeSymbol(1);
          break;
        case ConnectButtonEvent::kC:
          appendSecretCodeSymbol(2);
          break;
        case ConnectButtonEvent::kCLong:
          ui.secretCode = false;
          ui.page = 0;
          ui.cursor = 0;
          memset(ui.entry, 0, sizeof(ui.entry));
          echopet::uiExit(ui);
          dirty = true;
          break;
        case ConnectButtonEvent::kNone:
          break;
      }
    } else {
    switch (event) {
      case ConnectButtonEvent::kA:
      case ConnectButtonEvent::kALong:
        if (ui.mode == UiMode::kShop) {
          if (!handleShopSecretTap(nowMs)) {
            echopet::uiNext(ui, pet.snapshot());
          }
        } else if (ui.mode != UiMode::kFriendDeleteConfirm) {
          resetShopSecretGesture();
          echopet::uiNext(ui, pet.snapshot());
        }
        dirty = true;
        break;
      case ConnectButtonEvent::kC:
        if (ui.mode == UiMode::kShop) {
          cancelShopSecretGesture();
        }
        if (ui.mode == UiMode::kLinkStandby) {
          enterLinkResult(kLinkUiCanceled);
        } else if (ui.mode == UiMode::kLinkResult) {
          echopet::uiExit(ui);
        } else if (ui.mode == UiMode::kFriendDeleteConfirm) {
          ui.mode = UiMode::kFriends;
          ui.page = ui.cursor;
        } else {
          echopet::uiExit(ui);
        }
        dirty = true;
        break;
      case ConnectButtonEvent::kCLong:
        if (ui.mode == UiMode::kShop) {
          cancelShopSecretGesture();
        }
        if (ui.mode == UiMode::kLinkStandby) {
          enterLinkResult(kLinkUiCanceled);
        } else if (ui.mode == UiMode::kLinkResult) {
          echopet::uiExit(ui);
        } else if (ui.mode == UiMode::kFriends && pet.snapshot().friendCount > 0) {
          ui.mode = UiMode::kFriendDeleteConfirm;
          ui.page = ui.cursor;
        } else if (ui.mode == UiMode::kFriendDeleteConfirm) {
          ui.mode = UiMode::kFriends;
          ui.page = ui.cursor;
        } else {
          echopet::uiExit(ui);
        }
        dirty = true;
        break;
      case ConnectButtonEvent::kB:
      case ConnectButtonEvent::kBLong:
        if (ui.mode == UiMode::kShop) {
          cancelShopSecretGesture();
        }
        activateUiSelection();
        break;
      case ConnectButtonEvent::kNone:
        break;
    }
    }
  } else {
    switch (event) {
      case ConnectButtonEvent::kA:
      case ConnectButtonEvent::kALong:
        selectNextAction();
        break;
      case ConnectButtonEvent::kB:
        activateSelectedAction();
        break;
      case ConnectButtonEvent::kBLong:
        if (pet.snapshot().lifeState == echopet::LifeState::kPassed) {
          enterUiMode(UiMode::kResetConfirm);
        } else {
          enterClockSetMode();
        }
        dirty = true;
        break;
      case ConnectButtonEvent::kC:
        showStatus();
        break;
      case ConnectButtonEvent::kCLong:
        enterUiMode(UiMode::kSpriteProof);
        dirty = true;
        break;
      case ConnectButtonEvent::kNone:
        break;
    }
  }

  if (readLoraLink() || readSerialLink()) {
    savePet();
    dirty = true;
  }

  updateLinkStandby();
  updateShopSecretTransition(nowMs);

  if (ui.mode != UiMode::kClockSet && pet.tick(nowMs)) {
    dirty = true;
  }

  if (!dirty && shouldRunIdleAnimation(nowMs) &&
      (nowMs - lastAnimationMs) >= animationIntervalMs()) {
    dirty = true;
  }

  if ((nowMs - lastSaveMs) > kAutosaveMs) {
    savePet();
  }

  if (dirty) {
    render();
  }
  pumpDisplayRefresh();
  maybeClearRenderedNotice(nowMs);

  delay(20);
}
