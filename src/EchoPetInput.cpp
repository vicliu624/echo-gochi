#include "EchoPetInput.h"

#include <Arduino.h>
#include <Wire.h>

#include "echopet_target_config.h"

namespace echopet {
namespace {

constexpr uint32_t kLongPressMs = 800;
constexpr uint32_t kDoubleClickWindowMs = 360;

enum ButtonIndex : uint8_t {
  kButtonA,
  kButtonB,
  kButtonC,
  kButtonCount,
};

ConnectButtonEvent buttonEvent(uint8_t index, bool longPress) {
  switch (index) {
    case kButtonA:
      return longPress ? ConnectButtonEvent::kALong : ConnectButtonEvent::kA;
    case kButtonB:
      return longPress ? ConnectButtonEvent::kBLong : ConnectButtonEvent::kB;
    case kButtonC:
      return longPress ? ConnectButtonEvent::kCLong : ConnectButtonEvent::kC;
    default:
      return ConnectButtonEvent::kNone;
  }
}

#if defined(ECHOPET_TARGET_GAT562) && ECHOPET_TARGET_GAT562

struct DebouncedButton {
  bool sampled = false;
  bool stable = false;
  bool longSent = false;
  uint32_t changedAtMs = 0;
  uint32_t downAtMs = 0;
};

DebouncedButton buttons[kButtonCount];

bool readActiveLowPin(int pin) {
  return pin >= 0 && digitalRead(pin) == LOW;
}

bool readMappedButton(uint8_t index) {
  switch (index) {
    case kButtonA:
      return readActiveLowPin(ECHOPET_BUTTON_A_PIN) ||
             readActiveLowPin(ECHOPET_BUTTON_A_FALLBACK_PIN);
    case kButtonB:
      return readActiveLowPin(ECHOPET_BUTTON_B_PIN) ||
             readActiveLowPin(ECHOPET_BUTTON_B_FALLBACK_PIN);
    case kButtonC:
      return readActiveLowPin(ECHOPET_BUTTON_C_PIN);
    default:
      return false;
  }
}

void beginInputPin(int pin) {
  if (pin < 0) return;
  pinMode(pin, ECHOPET_BUTTON_NEED_PULLUP ? INPUT_PULLUP : INPUT);
}

ConnectButtonEvent updateDebouncedButton(uint8_t index, bool sampled,
                                         uint32_t nowMs) {
  DebouncedButton& state = buttons[index];
  if (sampled != state.sampled) {
    state.sampled = sampled;
    state.changedAtMs = nowMs;
  }

  if (state.stable == state.sampled ||
      (nowMs - state.changedAtMs) < ECHOPET_BUTTON_DEBOUNCE_MS) {
    return ConnectButtonEvent::kNone;
  }

  state.stable = state.sampled;
  if (state.stable) {
    state.downAtMs = nowMs;
    state.longSent = false;
    return ConnectButtonEvent::kNone;
  }

  const bool alreadySentLong = state.longSent;
  const bool longPress =
      !alreadySentLong && (nowMs - state.downAtMs) >= kLongPressMs;
  state.longSent = false;
  return alreadySentLong ? ConnectButtonEvent::kNone
                         : buttonEvent(index, longPress);
}

ConnectButtonEvent pollGat562Input() {
  const uint32_t nowMs = millis();
  for (uint8_t index = 0; index < kButtonCount; index++) {
    const ConnectButtonEvent event =
        updateDebouncedButton(index, readMappedButton(index), nowMs);
    if (event != ConnectButtonEvent::kNone) {
      return event;
    }
  }

  for (uint8_t index = 0; index < kButtonCount; index++) {
    DebouncedButton& state = buttons[index];
    if (state.stable && !state.longSent &&
        (nowMs - state.downAtMs) >= kLongPressMs) {
      state.longSent = true;
      return buttonEvent(index, true);
    }
  }
  return ConnectButtonEvent::kNone;
}

#else

constexpr uint8_t kTca8418Address = 0x34;
constexpr uint8_t kRegCfg = 0x01;
constexpr uint8_t kRegIntStat = 0x02;
constexpr uint8_t kRegKeyLckEc = 0x03;
constexpr uint8_t kRegKeyEventA = 0x04;
constexpr uint8_t kRegKpGpio1 = 0x1D;
constexpr uint8_t kRegKpGpio2 = 0x1E;
constexpr uint8_t kRegKpGpio3 = 0x1F;
constexpr uint8_t kRegDebounceDisable1 = 0x29;
constexpr uint8_t kRegDebounceDisable2 = 0x2A;
constexpr uint8_t kRegDebounceDisable3 = 0x2B;

constexpr uint8_t kCfgInterruptPulse = 0x10;
constexpr uint8_t kCfgFifoOverflowInterrupt = 0x08;
constexpr uint8_t kCfgKeyEventInterrupt = 0x01;
constexpr uint8_t kTcaKeyReleasedMask = 0x80;

constexpr uint8_t kKeyEsc = 42;
constexpr uint8_t kKeyHome = 43;
constexpr uint8_t kKeyMail = 44;

bool tcaReady = false;
bool tcaDown[kButtonCount] = {};
bool tcaLongSent[kButtonCount] = {};
uint32_t tcaDownAt[kButtonCount] = {};

bool writeTca(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(kTca8418Address);
  Wire.write(reg);
  Wire.write(value);
  return Wire.endTransmission() == 0;
}

bool readTca(uint8_t reg, uint8_t& value) {
  Wire.beginTransmission(kTca8418Address);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom(kTca8418Address, static_cast<uint8_t>(1)) != 1) {
    return false;
  }
  value = Wire.read();
  return true;
}

void flushTcaFifo() {
  uint8_t count = 0;
  if (!readTca(kRegKeyLckEc, count)) return;
  count &= 0x0F;
  while (count--) {
    uint8_t ignored = 0;
    if (!readTca(kRegKeyEventA, ignored)) break;
  }
  writeTca(kRegIntStat, 0x1F);
}

int8_t indexForKey(uint8_t key) {
  switch (key) {
    case kKeyEsc:
      return kButtonA;
    case kKeyHome:
      return kButtonB;
    case kKeyMail:
      return kButtonC;
    default:
      return -1;
  }
}

ConnectButtonEvent pollTcaInput() {
  if (!tcaReady) return ConnectButtonEvent::kNone;

  uint8_t count = 0;
  if (!readTca(kRegKeyLckEc, count)) {
    tcaReady = false;
    return ConnectButtonEvent::kNone;
  }
  count &= 0x0F;
  const bool hadEvents = count > 0;

  ConnectButtonEvent result = ConnectButtonEvent::kNone;
  while (count--) {
    uint8_t raw = 0;
    if (!readTca(kRegKeyEventA, raw) || raw == 0) break;

    const bool pressed = (raw & kTcaKeyReleasedMask) == 0;
    const int8_t index = indexForKey(raw & 0x7F);
    if (index < 0) continue;

    if (pressed) {
      tcaDown[index] = true;
      tcaLongSent[index] = false;
      tcaDownAt[index] = millis();
    } else if (tcaDown[index] && result == ConnectButtonEvent::kNone) {
      const bool alreadySentLong = tcaLongSent[index];
      const bool longPress = !alreadySentLong &&
                             (millis() - tcaDownAt[index]) >= kLongPressMs;
      tcaDown[index] = false;
      tcaLongSent[index] = false;
      if (!alreadySentLong) {
        result = buttonEvent(static_cast<uint8_t>(index), longPress);
      }
    } else {
      tcaDown[index] = false;
      tcaLongSent[index] = false;
    }
  }
  if (hadEvents) {
    writeTca(kRegIntStat, 0x1F);
  }

  if (result == ConnectButtonEvent::kNone) {
    const uint32_t now = millis();
    for (uint8_t index = 0; index < kButtonCount; index++) {
      if (tcaDown[index] && !tcaLongSent[index] &&
          (now - tcaDownAt[index]) >= kLongPressMs) {
        tcaLongSent[index] = true;
        result = buttonEvent(index, true);
        break;
      }
    }
  }

  return result;
}

ConnectButtonEvent pollBootFallback() {
  static bool previousDown = false;
  static bool longSent = false;
  static uint32_t downStartedMs = 0;
  static uint32_t lastUpMs = 0;
  static uint8_t shortClickCount = 0;

  const uint32_t now = millis();
  const bool down = digitalRead(ECHOPET_BOOT_BUTTON_PIN) == LOW;

  if (down && !previousDown) {
    downStartedMs = now;
    longSent = false;
  }

  if (down && previousDown && !longSent &&
      (now - downStartedMs) >= kLongPressMs) {
    shortClickCount = 0;
    longSent = true;
    return ConnectButtonEvent::kBLong;
  }

  if (!down && previousDown) {
    const uint32_t heldMs = now - downStartedMs;
    if (longSent || heldMs >= kLongPressMs) {
      shortClickCount = 0;
      previousDown = down;
      longSent = false;
      return ConnectButtonEvent::kNone;
    }
    shortClickCount++;
    lastUpMs = now;
  }

  previousDown = down;

  if (!down && shortClickCount > 0 &&
      (now - lastUpMs) > kDoubleClickWindowMs) {
    const uint8_t clicks = shortClickCount;
    shortClickCount = 0;
    return clicks >= 2 ? ConnectButtonEvent::kC : ConnectButtonEvent::kA;
  }

  return ConnectButtonEvent::kNone;
}

#endif

}  // namespace

bool beginEchoPetInput() {
#if defined(ECHOPET_TARGET_GAT562) && ECHOPET_TARGET_GAT562
  beginInputPin(ECHOPET_BUTTON_A_PIN);
  beginInputPin(ECHOPET_BUTTON_B_PIN);
  beginInputPin(ECHOPET_BUTTON_C_PIN);
  beginInputPin(ECHOPET_BUTTON_A_FALLBACK_PIN);
  beginInputPin(ECHOPET_BUTTON_B_FALLBACK_PIN);
  const uint32_t nowMs = millis();
  for (uint8_t index = 0; index < kButtonCount; index++) {
    buttons[index].sampled = readMappedButton(index);
    buttons[index].stable = buttons[index].sampled;
    buttons[index].changedAtMs = nowMs;
    buttons[index].downAtMs = nowMs;
    buttons[index].longSent = false;
  }
  return false;
#else
  pinMode(ECHOPET_BOOT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(ECHOPET_KEYSIELD_INT, INPUT_PULLUP);
  Wire.setPins(ECHOPET_KEYSIELD_I2C_SDA, ECHOPET_KEYSIELD_I2C_SCL);
  Wire.begin();
  delay(5);

  bool ok = true;
  ok &= writeTca(kRegKpGpio1, 0x1F);  // ROW0-ROW4.
  ok &= writeTca(kRegKpGpio2, 0x0F);  // COL0-COL3.
  ok &= writeTca(kRegKpGpio3, 0x00);
  ok &= writeTca(kRegDebounceDisable1, 0x00);
  ok &= writeTca(kRegDebounceDisable2, 0x00);
  ok &= writeTca(kRegDebounceDisable3, 0x00);
  ok &= writeTca(kRegCfg, kCfgInterruptPulse | kCfgFifoOverflowInterrupt |
                              kCfgKeyEventInterrupt);
  ok &= writeTca(kRegIntStat, 0x1F);
  tcaReady = ok;
  if (tcaReady) flushTcaFifo();
  return tcaReady;
#endif
}

bool echoPetInputUsesKeyShield() {
#if defined(ECHOPET_TARGET_GAT562) && ECHOPET_TARGET_GAT562
  return false;
#else
  return tcaReady;
#endif
}

ConnectButtonEvent pollEchoPetInput() {
#if defined(ECHOPET_TARGET_GAT562) && ECHOPET_TARGET_GAT562
  return pollGat562Input();
#else
  const ConnectButtonEvent event = pollTcaInput();
  return event != ConnectButtonEvent::kNone ? event : pollBootFallback();
#endif
}

const char* connectButtonLabel(ConnectButtonEvent event) {
  switch (event) {
    case ConnectButtonEvent::kA:
#if defined(ECHOPET_TARGET_GAT562) && ECHOPET_TARGET_GAT562
      return "SELECT";
#else
      return "A";
#endif
    case ConnectButtonEvent::kB:
#if defined(ECHOPET_TARGET_GAT562) && ECHOPET_TARGET_GAT562
      return "CONFIRM";
#else
      return "B";
#endif
    case ConnectButtonEvent::kC:
#if defined(ECHOPET_TARGET_GAT562) && ECHOPET_TARGET_GAT562
      return "CANCEL";
#else
      return "C";
#endif
    case ConnectButtonEvent::kALong:
#if defined(ECHOPET_TARGET_GAT562) && ECHOPET_TARGET_GAT562
      return "SELECT HOLD";
#else
      return "A HOLD";
#endif
    case ConnectButtonEvent::kBLong:
#if defined(ECHOPET_TARGET_GAT562) && ECHOPET_TARGET_GAT562
      return "CONFIRM HOLD";
#else
      return "B HOLD";
#endif
    case ConnectButtonEvent::kCLong:
#if defined(ECHOPET_TARGET_GAT562) && ECHOPET_TARGET_GAT562
      return "CANCEL HOLD";
#else
      return "C HOLD";
#endif
    case ConnectButtonEvent::kNone:
      return "";
  }
  return "";
}

}  // namespace echopet
