# Hardware map source summary

This file mirrors the definitions in `include/board_pins.h`.

## Main board

- MCU: nRF52840
- Display: GDEM0122T61 / SSD1681 E-Paper, 176x192
- LoRa: S62F / SX1262
- Flash: ZD25WQ32CEIGR
- Battery ADC through divider
- Peripheral 3.3V controlled by RT9080_EN

## KeyShield

Shared I2C:

| Signal | nRF52840 GPIO | Arduino pin number |
|---|---:|---:|
| SDA | P1.04 | 36 |
| SCL | P1.02 | 34 |

Expected devices:

| Device | Function | Address |
|---|---|---:|
| ES8311 | audio codec | 0x18 |
| AW21009 | keyboard backlight | 0x20 |
| TCA8418 | keyboard scanner | 0x34 |
| AW86224 | haptic driver | 0x58 |
| ICM20948 | IMU extension, optional | 0x68 |

TCA8418 INT: P1.03 / Arduino pin 35.

## Key map

| Row | Col0 | Col1 | Col2 | Col3 |
|---|---|---|---|---|
| 0 | Yes | * | 0 | # |
| 1 | No | 7 | 8 | 9 |
| 2 | Down | 4 | 5 | 6 |
| 3 | Center | 1 | 2 | 3 |
| 4 | Up | Esc | Home | Mail |
