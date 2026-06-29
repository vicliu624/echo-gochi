# EchoPet PlatformIO Firmware

This project is now the standalone PlatformIO home for EchoPet on supported
nRF52840 handheld targets:

- LILYGO T-Echo-Lite nRF52840 + T-Echo-Lite-KeyShield, 176x192 SSD1681 e-paper.
- GAT562 Mesh EVB Pro, 128x64 SSD1306 OLED.

The old minimal hardware probe is preserved at:

```text
docs/minimal_probe/main.cpp
```

The active firmware entry is:

```text
src/EchoPet.ino
```

## Layout

```text
src/                         EchoPet firmware source and C/C++ bitmap assets
include/                     target pin/config and display compatibility headers
boards/                      vendored PlatformIO board definitions
variants/                    vendored Arduino nRF52 variants
lib/Adafruit_EPD-4.5.5/      local patched EPD driver with partial-refresh APIs
lib/Adafruit_BusIO-1.16.1/   local BusIO version required by the patched EPD
analysis/                    host simulators, official-reference proofs, audits,
                             and local non-runtime reference resource bundle
docs/echopet/                copied EchoPet specs, parity checklists, ledgers
docs/minimal_probe/          previous minimal hardware probe source
```

Runtime pet art and animation resources are kept as C/C++ arrays under `src/`.
PNG/JPG/GIF/BMP files are allowed only under `analysis/` as reference or proof
artifacts.

The local human comparison archive is:

```text
analysis\echopet_reference_resources
```

That folder mirrors official/reference assets, generated proof sheets, host
previews, manifests, reports, scripts, concept GIFs, and copied C/C++ resource
snapshots so visual review has one stable location inside the firmware project.
It is still analysis-only material. Runtime pet art and animation must remain
C/C++ arrays under `src/`.

## Build

Default build, with LoRa enabled:

```powershell
pio run
```

Explicit environments:

```powershell
pio run -e EchoPet_LoRa
pio run -e EchoPet
pio run -e EchoPet_GAT562_LoRa
```

Successful builds produce:

```text
.pio/build/EchoPet_LoRa/firmware.zip
.pio/build/EchoPet_LoRa/firmware.hex
.pio/build/EchoPet/firmware.zip
.pio/build/EchoPet/firmware.hex
.pio/build/EchoPet_GAT562_LoRa/firmware.zip
.pio/build/EchoPet_GAT562_LoRa/firmware.hex
```

## Upload

Serial bootloader / nrfutil:

```powershell
pio run -e EchoPet_LoRa -t upload
pio run -e EchoPet_GAT562_LoRa -t upload
```

Do not upload from automation unless the device state and port are confirmed.

For GAT562, if PlatformIO touches the application port but misses the short
bootloader window, wait for the stable bootloader serial port
(`VID:PID=239A:002A`) and run direct `adafruit-nrfutil` against that port.

## Host visual proof

Use the bundled or system Python interpreter:

```powershell
python analysis/screen_simulator/run_simulator.py
python analysis/visual_alignment/generate_visual_alignment_gate.py
python analysis/runtime_resource_audit/generate_runtime_resource_audit.py
python analysis/completion_audit/generate_completion_audit.py
```

Generated previews are written to:

```text
analysis/screen_simulator/out
```

## Hardware boundary

Host previews prove source-side layout, resource routing, and C/C++ asset
composition. They do not prove e-paper whitening, ghosting, partial-refresh
cadence, button feel, or two-device LoRa behavior. Those remain hardware
acceptance tasks tracked in `T_ECHO_LITE_HARDWARE_ACCEPTANCE.md` and
`GAT562_HARDWARE_ACCEPTANCE.md`.
