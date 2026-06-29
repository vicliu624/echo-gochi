#pragma once

#include <stdint.h>

#include "Adafruit_EPD.h"

namespace echopet {

constexpr uint8_t kCharacterVisualFamilyCount = 9;
constexpr uint8_t kCharacterVisualCatalogCount = 50;
constexpr uint8_t kCharacterVisualSide = 16;
constexpr uint8_t kCharacterIdleVisualSide = 24;

void drawCharacterFamilyBitmap(Adafruit_SSD1681& display, int16_t x,
                               int16_t y, uint8_t family, uint8_t phase,
                               uint8_t scale);
void drawCharacterCatalogBitmap(Adafruit_SSD1681& display, int16_t x,
                                int16_t y, uint8_t catalogId, uint8_t phase,
                                uint8_t scale);
void drawCharacterCatalogIdleBitmap(Adafruit_SSD1681& display, int16_t x,
                                    int16_t y, uint8_t catalogId,
                                    uint8_t phase, uint8_t scale);

}  // namespace echopet
