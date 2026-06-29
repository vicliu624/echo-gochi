#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <Wire.h>
#include "board_pins.h"

using namespace techo;

volatile bool boot_pressed = false;
volatile bool keyshield_irq_seen = false;

void onBootFalling() { boot_pressed = true; }
void onTca8418Falling() { keyshield_irq_seen = true; }

static void printPin(const char* name, int pin) {
  Serial.print("  ");
  Serial.print(name);
  Serial.print(" = ");
  Serial.println(pin);
}

static void enableExternal3v3() {
  pinMode(RT9080_EN, OUTPUT);
  digitalWrite(RT9080_EN, HIGH);
  delay(200);
}

static float readBatteryVoltage() {
  pinMode(BATTERY_MEASUREMENT_CONTROL, OUTPUT);
  digitalWrite(BATTERY_MEASUREMENT_CONTROL, HIGH);
  pinMode(BATTERY_ADC_DATA, INPUT);
  analogReference(AR_INTERNAL_3_0);
  analogReadResolution(BATTERY_ADC_BITS);

  uint32_t sum = 0;
  for (int i = 0; i < 16; ++i) {
    sum += analogRead(BATTERY_ADC_DATA);
    delay(2);
  }
  const float raw = sum / 16.0f;
  const float max_count = (1UL << BATTERY_ADC_BITS) - 1;
  const float adc_mv = raw * (BATTERY_ADC_REFERENCE_MV / max_count);
  return (adc_mv / 1000.0f) * BATTERY_DIVIDER_RATIO;
}

static bool probeI2c(uint8_t addr) {
  Wire.beginTransmission(addr);
  return Wire.endTransmission() == 0;
}

static void scanI2c() {
  Serial.println();
  Serial.println("I2C scan on KeyShield/shared bus:");
  Serial.print("  SDA="); Serial.print(IIC_1_SDA);
  Serial.print(" SCL="); Serial.println(IIC_1_SCL);

  int found = 0;
  for (uint8_t addr = 1; addr < 0x7f; ++addr) {
    if (probeI2c(addr)) {
      Serial.print("  found 0x");
      if (addr < 0x10) Serial.print('0');
      Serial.println(addr, HEX);
      ++found;
    }
  }
  if (found == 0) {
    Serial.println("  no I2C device found; check KeyShield power, connector, SDA/SCL.");
  }

  Serial.println("Expected devices:");
  for (const auto& dev : EXPECTED_I2C_DEVICES) {
    const bool ok = probeI2c(dev.address);
    Serial.print("  ");
    Serial.print(dev.name);
    Serial.print(" @0x");
    if (dev.address < 0x10) Serial.print('0');
    Serial.print(dev.address, HEX);
    Serial.println(ok ? " OK" : " missing");
  }
}

static void printHardwareMap() {
  Serial.println();
  Serial.println("=== T-Echo-Lite + KeyShield hardware map ===");
  Serial.println(BOARD_NAME);
  Serial.println(KEYSHIELD_NAME);

  Serial.println("\nPower / buttons / battery:");
  printPin("RT9080_EN", RT9080_EN);
  printPin("NRF52840_BOOT", NRF52840_BOOT);
  printPin("BATTERY_MEASUREMENT_CONTROL", BATTERY_MEASUREMENT_CONTROL);
  printPin("BATTERY_ADC_DATA", BATTERY_ADC_DATA);

  Serial.println("\nLEDs:");
  printPin("LED_1", LED_1);
  printPin("LED_2", LED_2);
  printPin("LED_3", LED_3);

  Serial.println("\nE-Paper display:");
  Serial.print("  size = "); Serial.print(SCREEN_WIDTH); Serial.print("x"); Serial.println(SCREEN_HEIGHT);
  printPin("SCREEN_BS1", SCREEN_BS1);
  printPin("SCREEN_BUSY", SCREEN_BUSY);
  printPin("SCREEN_RST", SCREEN_RST);
  printPin("SCREEN_DC", SCREEN_DC);
  printPin("SCREEN_CS", SCREEN_CS);
  printPin("SCREEN_SCLK", SCREEN_SCLK);
  printPin("SCREEN_MOSI", SCREEN_MOSI);

  Serial.println("\nSX1262 LoRa:");
  printPin("SX1262_CS", SX1262_CS);
  printPin("SX1262_RST", SX1262_RST);
  printPin("SX1262_SCLK", SX1262_SCLK);
  printPin("SX1262_MOSI", SX1262_MOSI);
  printPin("SX1262_MISO", SX1262_MISO);
  printPin("SX1262_BUSY", SX1262_BUSY);
  printPin("SX1262_DIO1/INT", SX1262_DIO1);
  printPin("SX1262_DIO2", SX1262_DIO2);
  printPin("SX1262_RF_VC1", SX1262_RF_VC1);
  printPin("SX1262_RF_VC2", SX1262_RF_VC2);

  Serial.println("\nExternal SPI Flash ZD25WQ32C:");
  printPin("FLASH_CS", ZD25WQ32C_CS);
  printPin("FLASH_SCLK", ZD25WQ32C_SCLK);
  printPin("FLASH_MOSI/IO0", ZD25WQ32C_MOSI);
  printPin("FLASH_MISO/IO1", ZD25WQ32C_MISO);
  printPin("FLASH_IO2", ZD25WQ32C_IO2);
  printPin("FLASH_IO3", ZD25WQ32C_IO3);

  Serial.println("\nKeyShield shared I2C:");
  printPin("IIC_1_SDA", IIC_1_SDA);
  printPin("IIC_1_SCL", IIC_1_SCL);
  printPin("TCA8418_INT", TCA8418_INT);

  Serial.println("\nKeyShield ES8311 I2S:");
  printPin("ES8311_ADC_DATA", ES8311_ADC_DATA);
  printPin("ES8311_DAC_DATA", ES8311_DAC_DATA);
  printPin("ES8311_BCLK", ES8311_BCLK);
  printPin("ES8311_MCLK", ES8311_MCLK);
  printPin("ES8311_WS_LRCK", ES8311_WS_LRCK);

  Serial.println("\nGPS extension:");
  printPin("GPS_WAKE_UP", GPS_WAKE_UP);
  printPin("GPS_1PPS", GPS_1PPS);
  printPin("GPS_UART_TX", GPS_UART_TX);
  printPin("GPS_UART_RX", GPS_UART_RX);
  printPin("GPS_RT9080_EN", GPS_RT9080_EN);

  Serial.println("\nICM20948 extension:");
  printPin("ICM20948_INT", ICM20948_INT);

  Serial.println("\nKeyShield 4x5 key map:");
  for (const auto& key : KEYSHIELD_KEYS) {
    Serial.print("  row "); Serial.print(key.row);
    Serial.print(" col "); Serial.print(key.col);
    Serial.print(" index "); Serial.print(key.tca8418_index);
    Serial.print(" -> "); Serial.println(key.text);
  }
}

void setup() {
  Serial.begin(115200);
  delay(1200);
  Serial.println("\nBooting t-echo-lite-minimal-pio-fixed");

  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  digitalWrite(LED_1, LED_OFF_LEVEL);
  digitalWrite(LED_2, LED_OFF_LEVEL);

  enableExternal3v3();

  pinMode(NRF52840_BOOT, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(NRF52840_BOOT), onBootFalling, FALLING);

  pinMode(TCA8418_INT, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(TCA8418_INT), onTca8418Falling, FALLING);

  Wire.setPins(IIC_1_SDA, IIC_1_SCL);
  Wire.begin();
  Wire.setClock(100000);

  printHardwareMap();
  scanI2c();
}

void loop() {
  static uint32_t last = 0;
  static bool led = false;

  if (boot_pressed) {
    boot_pressed = false;
    Serial.println("BOOT falling edge detected");
  }

  if (keyshield_irq_seen) {
    keyshield_irq_seen = false;
    Serial.println("TCA8418 INT falling edge detected; TCA8418 driver is not included in this minimal project.");
  }

  const uint32_t now = millis();
  if (now - last > 2000) {
    last = now;
    led = !led;
    digitalWrite(LED_1, led ? LED_ON_LEVEL : LED_OFF_LEVEL);
    Serial.print("battery ~= ");
    Serial.print(readBatteryVoltage(), 3);
    Serial.println(" V");
  }
}
