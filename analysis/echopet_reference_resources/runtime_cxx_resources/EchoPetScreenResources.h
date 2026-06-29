#pragma once

#include <stdint.h>

namespace echopet {

struct ScreenResources {
  uint16_t canvasWidth;
  uint16_t canvasHeight;
  int16_t leftX;
  int16_t leftY;
  uint16_t leftW;
  uint16_t leftH;
  int16_t mainX;
  int16_t mainY;
  uint16_t mainW;
  uint16_t mainH;
  int16_t rightX;
  int16_t rightY;
  uint16_t rightW;
  uint16_t rightH;
  uint8_t spriteScale;
  uint8_t compactText;
};

}  // namespace echopet
