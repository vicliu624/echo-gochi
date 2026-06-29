#pragma once

#include <stdint.h>

#include "Adafruit_EPD.h"

namespace echopet {

constexpr uint8_t kCatalogVisualSide = 16;
constexpr uint8_t kCatalogFoodVisualCount = 11;
constexpr uint8_t kCatalogItemVisualCount = 41;
constexpr uint8_t kCatalogSouvenirVisualCount = 16;
constexpr uint8_t kCatalogSouvenirMemoryVisualCount = 64;

void drawCatalogFoodBitmap(Adafruit_SSD1681& display, int16_t x, int16_t y,
                           uint8_t icon, uint8_t phase, uint8_t scale);
void drawCatalogItemBitmap(Adafruit_SSD1681& display, int16_t x, int16_t y,
                           uint8_t icon, uint8_t phase, uint8_t scale);
void drawCatalogEntryBitmap(Adafruit_SSD1681& display, int16_t x, int16_t y,
                            uint8_t catalogIndex, uint8_t phase,
                            uint8_t scale);
void drawCatalogSouvenirBitmap(Adafruit_SSD1681& display, int16_t x,
                               int16_t y, uint8_t index, uint8_t phase,
                               uint8_t scale);

}  // namespace echopet
