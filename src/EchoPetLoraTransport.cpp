#include "EchoPetLoraTransport.h"

#include <string.h>

#if defined(ECHOPET_ENABLE_LORA) && ECHOPET_ENABLE_LORA
#include <SPI.h>

#ifndef ECHOPET_RADIOLIB_HEADER
#define ECHOPET_RADIOLIB_HEADER <RadioLib.h>
#endif

#include ECHOPET_RADIOLIB_HEADER
#include "echopet_target_config.h"
#endif

namespace echopet {

#if defined(ECHOPET_ENABLE_LORA) && ECHOPET_ENABLE_LORA
namespace {

constexpr float kFrequencyMhz = 868.6f;
constexpr float kBandwidthKhz = 125.0f;
constexpr uint8_t kSpreadingFactor = 9;
constexpr uint8_t kCodingRate = 6;
constexpr uint8_t kSyncWord = RADIOLIB_SX126X_SYNC_WORD_PRIVATE;
constexpr int8_t kOutputPowerDbm = ECHOPET_LORA_OUTPUT_POWER_DBM;
constexpr float kCurrentLimitMa = 140.0f;
constexpr uint16_t kPreambleLength = 16;

SPIClass loraSpi(NRF_SPIM3, ECHOPET_LORA_MISO, ECHOPET_LORA_SCLK,
                 ECHOPET_LORA_MOSI);
Module loraModule(ECHOPET_LORA_CS, ECHOPET_LORA_DIO1, ECHOPET_LORA_RST,
                  ECHOPET_LORA_BUSY, loraSpi);
SX1262 loraRadio(&loraModule);

volatile bool packetReady = false;
bool initialized = false;

void onPacketReady() {
  packetReady = true;
}

void setRfSwitch(bool transmit) {
#if ECHOPET_LORA_HAS_MANUAL_RF_SWITCH
  digitalWrite(ECHOPET_LORA_RF_SWITCH_TX, transmit ? HIGH : LOW);
  digitalWrite(ECHOPET_LORA_RF_SWITCH_RX, transmit ? LOW : HIGH);
#else
  (void)transmit;
#endif
}

bool startReceive() {
  setRfSwitch(false);
  return loraRadio.startReceive() == RADIOLIB_ERR_NONE;
}

bool applyParameters() {
  return loraRadio.setFrequency(kFrequencyMhz) == RADIOLIB_ERR_NONE &&
         loraRadio.setBandwidth(kBandwidthKhz) == RADIOLIB_ERR_NONE &&
         loraRadio.setSpreadingFactor(kSpreadingFactor) == RADIOLIB_ERR_NONE &&
         loraRadio.setCodingRate(kCodingRate) == RADIOLIB_ERR_NONE &&
         loraRadio.setSyncWord(kSyncWord) == RADIOLIB_ERR_NONE &&
         loraRadio.setOutputPower(kOutputPowerDbm) == RADIOLIB_ERR_NONE &&
         loraRadio.setCurrentLimit(kCurrentLimitMa) == RADIOLIB_ERR_NONE &&
         loraRadio.setPreambleLength(kPreambleLength) == RADIOLIB_ERR_NONE &&
         loraRadio.setCRC(true) == RADIOLIB_ERR_NONE;
}

int16_t beginRadio() {
  return loraRadio.begin(kFrequencyMhz, kBandwidthKhz, kSpreadingFactor,
                         kCodingRate, kSyncWord, kOutputPowerDbm,
                         kPreambleLength, ECHOPET_LORA_TCXO_VOLTAGE);
}

}  // namespace
#endif

bool beginLoraTransport() {
#if defined(ECHOPET_ENABLE_LORA) && ECHOPET_ENABLE_LORA
  if (initialized) return true;
#if ECHOPET_LORA_POWER_EN >= 0
  pinMode(ECHOPET_LORA_POWER_EN, OUTPUT);
  digitalWrite(ECHOPET_LORA_POWER_EN, HIGH);
  delay(10);
#endif
#if ECHOPET_LORA_HAS_MANUAL_RF_SWITCH
  pinMode(ECHOPET_LORA_RF_SWITCH_TX, OUTPUT);
  pinMode(ECHOPET_LORA_RF_SWITCH_RX, OUTPUT);
#endif
  setRfSwitch(false);
  loraSpi.begin();
  int16_t state = beginRadio();
  if (state != RADIOLIB_ERR_NONE || !applyParameters()) return false;
  loraRadio.setDio1Action(onPacketReady);
  initialized = startReceive();
  return initialized;
#else
  return false;
#endif
}

bool sendLoraFriendPacket(const FriendPacket& packet) {
#if defined(ECHOPET_ENABLE_LORA) && ECHOPET_ENABLE_LORA
  if (!initialized && !beginLoraTransport()) return false;
  if (!validateFriendPacket(packet)) return false;
  setRfSwitch(true);
  const int16_t state =
      loraRadio.transmit(reinterpret_cast<const uint8_t*>(&packet),
                         sizeof(packet));
  packetReady = false;
  startReceive();
  return state == RADIOLIB_ERR_NONE;
#else
  (void)packet;
  return false;
#endif
}

bool pollLoraFriendPacket(FriendPacket& packet) {
#if defined(ECHOPET_ENABLE_LORA) && ECHOPET_ENABLE_LORA
  if (!initialized || !packetReady) return false;
  packetReady = false;
  uint8_t buffer[sizeof(FriendPacket)] = {};
  size_t length = loraRadio.getPacketLength();
  if (length != sizeof(FriendPacket)) {
    startReceive();
    return false;
  }
  const int16_t state = loraRadio.readData(buffer, sizeof(buffer));
  startReceive();
  if (state != RADIOLIB_ERR_NONE) return false;
  memcpy(&packet, buffer, sizeof(packet));
  return validateFriendPacket(packet);
#else
  (void)packet;
  return false;
#endif
}

}  // namespace echopet
