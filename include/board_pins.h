#pragma once
#include <Arduino.h>

#define TECHO_PINNUM(port, pin) ((port) * 32 + (pin))

namespace techo {

struct I2cDeviceInfo { const char* name; uint8_t address; };
struct KeyMapEntry { const char* text; uint8_t row; uint8_t col; uint8_t tca8418_index; };

// Board identity
static constexpr const char* BOARD_NAME = "LILYGO T-Echo-Lite nRF52840";
static constexpr const char* KEYSHIELD_NAME = "T-Echo-Lite-KeyShield";
static constexpr uint32_t MCU_CLOCK_HZ = 64000000UL;
static constexpr uint32_t MCU_RAM_BYTES_APPROX = 256UL * 1024UL;
static constexpr uint32_t MCU_FLASH_BYTES_APPROX = 1024UL * 1024UL;

// External 1x4p
static constexpr uint8_t EXT_1X4P_1_IO_0_25 = TECHO_PINNUM(0, 25);
static constexpr uint8_t EXT_1X4P_1_IO_0_23 = TECHO_PINNUM(0, 23);
static constexpr uint8_t EXT_1X4P_2_IO_1_2  = TECHO_PINNUM(1, 2);
static constexpr uint8_t EXT_1X4P_2_IO_1_4  = TECHO_PINNUM(1, 4);

// External 1x7p
static constexpr uint8_t EXT_1X7P_IO_1_13 = TECHO_PINNUM(1, 13);
static constexpr uint8_t EXT_1X7P_IO_1_15 = TECHO_PINNUM(1, 15);
static constexpr uint8_t EXT_1X7P_IO_0_29 = TECHO_PINNUM(0, 29);
static constexpr uint8_t EXT_1X7P_IO_1_10 = TECHO_PINNUM(1, 10);
static constexpr uint8_t EXT_1X7P_IO_1_11 = TECHO_PINNUM(1, 11);

// External 2x5p
static constexpr uint8_t EXT_2X5P_1_IO_1_13 = TECHO_PINNUM(1, 13);
static constexpr uint8_t EXT_2X5P_1_IO_1_15 = TECHO_PINNUM(1, 15);
static constexpr uint8_t EXT_2X5P_1_IO_1_3  = TECHO_PINNUM(1, 3);
static constexpr uint8_t EXT_2X5P_1_IO_0_16 = TECHO_PINNUM(0, 16);
static constexpr uint8_t EXT_2X5P_1_IO_0_29 = TECHO_PINNUM(0, 29);
static constexpr uint8_t EXT_2X5P_1_IO_1_10 = TECHO_PINNUM(1, 10);
static constexpr uint8_t EXT_2X5P_1_IO_1_11 = TECHO_PINNUM(1, 11);
static constexpr uint8_t EXT_2X5P_2_IO_1_6  = TECHO_PINNUM(1, 6);
static constexpr uint8_t EXT_2X5P_2_IO_0_25 = TECHO_PINNUM(0, 25);
static constexpr uint8_t EXT_2X5P_2_IO_1_2  = TECHO_PINNUM(1, 2);
static constexpr uint8_t EXT_2X5P_2_IO_0_10 = TECHO_PINNUM(0, 10);
static constexpr uint8_t EXT_2X5P_2_IO_0_23 = TECHO_PINNUM(0, 23);
static constexpr uint8_t EXT_2X5P_2_IO_0_9  = TECHO_PINNUM(0, 9);
static constexpr uint8_t EXT_2X5P_2_IO_1_4  = TECHO_PINNUM(1, 4);

// Shared I2C bus
static constexpr uint8_t IIC_1_SDA = TECHO_PINNUM(1, 4);
static constexpr uint8_t IIC_1_SCL = TECHO_PINNUM(1, 2);

// External SPI flash: ZD25WQ32CEIGR
static constexpr uint8_t ZD25WQ32C_CS   = TECHO_PINNUM(0, 12);
static constexpr uint8_t ZD25WQ32C_SCLK = TECHO_PINNUM(0, 4);
static constexpr uint8_t ZD25WQ32C_MOSI = TECHO_PINNUM(0, 6);
static constexpr uint8_t ZD25WQ32C_MISO = TECHO_PINNUM(0, 8);
static constexpr uint8_t ZD25WQ32C_IO0  = TECHO_PINNUM(0, 6);
static constexpr uint8_t ZD25WQ32C_IO1  = TECHO_PINNUM(0, 8);
static constexpr uint8_t ZD25WQ32C_IO2  = TECHO_PINNUM(1, 9);
static constexpr uint8_t ZD25WQ32C_IO3  = TECHO_PINNUM(0, 26);

// LEDs
static constexpr uint8_t LED_1 = TECHO_PINNUM(1, 7);
static constexpr uint8_t LED_2 = TECHO_PINNUM(1, 5);
static constexpr uint8_t LED_3 = TECHO_PINNUM(1, 14);
static constexpr uint8_t LED_ON_LEVEL = LOW;
static constexpr uint8_t LED_OFF_LEVEL = HIGH;

// GDEM0122T61 / SSD1681 E-Paper, 176x192
static constexpr uint16_t SCREEN_WIDTH = 176;
static constexpr uint16_t SCREEN_HEIGHT = 192;
static constexpr uint8_t SCREEN_BS1  = TECHO_PINNUM(1, 12);
static constexpr uint8_t SCREEN_BUSY = TECHO_PINNUM(0, 3);
static constexpr uint8_t SCREEN_RST  = TECHO_PINNUM(0, 28);
static constexpr uint8_t SCREEN_DC   = TECHO_PINNUM(0, 21);
static constexpr uint8_t SCREEN_CS   = TECHO_PINNUM(0, 22);
static constexpr uint8_t SCREEN_SCLK = TECHO_PINNUM(0, 19);
static constexpr uint8_t SCREEN_MOSI = TECHO_PINNUM(0, 20);
static constexpr int8_t  SCREEN_MISO = -1;
static constexpr int8_t  SCREEN_SRAM_CS = -1;

// LoRa S62F / SX1262
static constexpr uint8_t SX1262_CS     = TECHO_PINNUM(0, 11);
static constexpr uint8_t SX1262_RST    = TECHO_PINNUM(0, 7);
static constexpr uint8_t SX1262_SCLK   = TECHO_PINNUM(0, 13);
static constexpr uint8_t SX1262_MOSI   = TECHO_PINNUM(0, 15);
static constexpr uint8_t SX1262_MISO   = TECHO_PINNUM(0, 17);
static constexpr uint8_t SX1262_BUSY   = TECHO_PINNUM(0, 14);
static constexpr uint8_t SX1262_INT    = TECHO_PINNUM(1, 8);
static constexpr uint8_t SX1262_DIO1   = TECHO_PINNUM(1, 8);
static constexpr uint8_t SX1262_DIO2   = TECHO_PINNUM(0, 5);
static constexpr uint8_t SX1262_RF_VC1 = TECHO_PINNUM(0, 27);
static constexpr uint8_t SX1262_RF_VC2 = TECHO_PINNUM(1, 1);

// Boot button
static constexpr uint8_t NRF52840_BOOT = TECHO_PINNUM(0, 24);

// Battery
static constexpr uint8_t BATTERY_MEASUREMENT_CONTROL = TECHO_PINNUM(0, 31);
static constexpr uint8_t BATTERY_ADC_DATA = TECHO_PINNUM(0, 2);
static constexpr float BATTERY_ADC_REFERENCE_MV = 3000.0f;
static constexpr float BATTERY_DIVIDER_RATIO = 2.0f;
static constexpr uint8_t BATTERY_ADC_BITS = 12;

// RT9080 external peripheral 3.3V enable
static constexpr uint8_t RT9080_EN = TECHO_PINNUM(0, 30);

// GPS L76K extension
static constexpr uint8_t GPS_WAKE_UP   = EXT_1X7P_IO_1_13;
static constexpr uint8_t GPS_1PPS      = EXT_1X7P_IO_1_15;
static constexpr uint8_t GPS_UART_TX   = EXT_1X7P_IO_0_29;
static constexpr uint8_t GPS_UART_RX   = EXT_1X7P_IO_1_10;
static constexpr uint8_t GPS_RT9080_EN = EXT_1X7P_IO_1_11;

// ICM20948 extension
static constexpr uint8_t ICM20948_ADDRESS = 0x68;
static constexpr uint8_t ICM20948_SDA = IIC_1_SDA;
static constexpr uint8_t ICM20948_SCL = IIC_1_SCL;
static constexpr uint8_t ICM20948_INT = TECHO_PINNUM(0, 16);

// KeyShield / TCA8418 keyboard scanner
static constexpr uint8_t TCA8418_SDA = IIC_1_SDA;
static constexpr uint8_t TCA8418_SCL = IIC_1_SCL;
static constexpr uint8_t TCA8418_INT = EXT_2X5P_1_IO_1_3;
static constexpr uint8_t TCA8418_I2C_ADDRESS = 0x34;
static constexpr uint8_t TCA8418_KEYPAD_SCAN_WIDTH = 4;
static constexpr uint8_t TCA8418_KEYPAD_SCAN_HEIGHT = 5;
static constexpr KeyMapEntry KEYSHIELD_KEYS[] = {
  {"Yes",    0, 0,  1}, {"*",      0, 1,  2}, {"0",    0, 2,  3}, {"#",    0, 3,  4},
  {"No",     1, 0, 11}, {"7",      1, 1, 12}, {"8",    1, 2, 13}, {"9",    1, 3, 14},
  {"Down",   2, 0, 21}, {"4",      2, 1, 22}, {"5",    2, 2, 23}, {"6",    2, 3, 24},
  {"Center", 3, 0, 31}, {"1",      3, 1, 32}, {"2",    3, 2, 33}, {"3",    3, 3, 34},
  {"Up",     4, 0, 41}, {"Esc",    4, 1, 42}, {"Home", 4, 2, 43}, {"Mail", 4, 3, 44},
};

// KeyShield / ES8311 audio codec
static constexpr uint8_t ES8311_SDA = IIC_1_SDA;
static constexpr uint8_t ES8311_SCL = IIC_1_SCL;
static constexpr uint8_t ES8311_I2C_ADDRESS = 0x18;
static constexpr uint8_t ES8311_ADC_DATA = EXT_2X5P_2_IO_0_23;
static constexpr uint8_t ES8311_DAC_DATA = EXT_2X5P_2_IO_1_6;
static constexpr uint8_t ES8311_BCLK     = EXT_2X5P_2_IO_0_10;
static constexpr uint8_t ES8311_MCLK     = EXT_2X5P_2_IO_0_9;
static constexpr uint8_t ES8311_WS_LRCK  = EXT_2X5P_2_IO_0_25;

// KeyShield / AW86224 haptic
static constexpr uint8_t AW86224_SDA = IIC_1_SDA;
static constexpr uint8_t AW86224_SCL = IIC_1_SCL;
static constexpr uint8_t AW86224_I2C_ADDRESS = 0x58;

// KeyShield / AW21009 backlight
static constexpr uint8_t AW21009_SDA = IIC_1_SDA;
static constexpr uint8_t AW21009_SCL = IIC_1_SCL;
static constexpr uint8_t AW21009_I2C_ADDRESS = 0x20;
static constexpr uint16_t AW21009_MAX_BRIGHTNESS = 4095;

static constexpr I2cDeviceInfo EXPECTED_I2C_DEVICES[] = {
  {"ES8311 audio codec", ES8311_I2C_ADDRESS},
  {"AW21009 keyboard backlight", AW21009_I2C_ADDRESS},
  {"TCA8418 keyboard scanner", TCA8418_I2C_ADDRESS},
  {"AW86224 haptic driver", AW86224_I2C_ADDRESS},
  {"ICM20948 IMU extension", ICM20948_ADDRESS},
};

} // namespace techo
