#pragma once

#include "echopet_target_config.h"

#if ECHOPET_TARGET_DISPLAY_SSD1306
#include <Adafruit_SSD1306.h>

using EchoPetDisplayDevice = Adafruit_SSD1306;

#ifndef EPD_BLACK
#define EPD_BLACK SSD1306_WHITE
#endif
#ifndef EPD_WHITE
#define EPD_WHITE SSD1306_BLACK
#endif

#else
#include "Adafruit_EPD.h"

using EchoPetDisplayDevice = Adafruit_SSD1681;
#endif
