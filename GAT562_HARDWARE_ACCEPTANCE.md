# GAT562 Hardware Acceptance Log

This file records what has actually been observed on the physical GAT562 Mesh
EVB Pro. Do not mark GAT562 hardware rows as done without an entry here.

The current GAT562 projection is intentionally narrow: EchoPet still sees the
same A/B/C virtual-pet input contract, while the board layer maps GAT562
hardware to that contract.

## Current Flash Record

| Date | Build | Port | Method | Result | Notes |
| --- | --- | --- | --- | --- | --- |
| 2026-06-29 18:14 +08:00 | `EchoPet_GAT562_LoRa` current build | `COM52` bootloader, returned as `COM51` app serial | Direct `adafruit-nrfutil` serial DFU | PASS | Fixed the remaining instant-disappear cleanup bug by capturing the pre-cleanup `messCount` into the Toilet scene UI state before `EchoPetModel::apply(Action::kToilet)` clears the model. `drawToiletScene()` now renders poop from that scene snapshot until the right-to-left checker clear-wall clips it away, instead of guessing from the already-clean model snapshot. First automatic PlatformIO upload touched `COM51` but `nrfutil` reported `No data received on serial port`; the board stayed in bootloader as `COM52`. A direct serial DFU to `COM52` then uploaded `.pio\build\EchoPet_GAT562_LoRa\firmware.zip`, activated firmware, and reported `Device programmed.` The board returned as application serial `COM51`. Build size: RAM 13080/248832, Flash 246400/815104. User visual confirmation on the actual OLED is still pending. |
| 2026-06-29 17:58 +08:00 | `EchoPet_GAT562_LoRa` current build | Touched `COM51`, uploaded on `COM52` bootloader | `pio run -e EchoPet_GAT562_LoRa -t upload` | PASS | Fixed the wrong cleanup landing page shown in the user photo: after `Notice::kClean` clears, `UiMode::kToilet` now exits to the home playfield instead of falling through to `NO MESS`. `Action::kToilet` now starts cleanup only when `messCount > 0`; low hygiene alone no longer masquerades as poop cleanup. Build size: RAM 13080/248832, Flash 246352/815104. PlatformIO auto-detected app serial `COM51`, reset into bootloader `COM52`, uploaded `.pio\build\EchoPet_GAT562_LoRa\firmware.zip`, activated firmware, and reported `Device programmed.` User visual confirmation on the actual OLED is still pending. |
| 2026-06-29 17:45 +08:00 | `EchoPet_GAT562_LoRa` current build | Touched `COM51`, uploaded on `COM52` bootloader | `pio run -e EchoPet_GAT562_LoRa -t upload` | PASS | Rebuilt after aligning source proof-frame naming with the accepted cleanup-wall contract (`WALL0`/`WALL1`, `CleanupWall` internals instead of historical flush/sweep wording). Build size unchanged: RAM 13080/248832, Flash 246416/815104. PlatformIO auto-detected app serial `COM51`, reset into bootloader `COM52`, uploaded `.pio\build\EchoPet_GAT562_LoRa\firmware.zip`, activated firmware, and reported `Device programmed.` User visual confirmation on the actual OLED is still pending. |
| 2026-06-29 17:36 +08:00 | `EchoPet_GAT562_LoRa` current build | `COM52` bootloader, returned as `COM51` app serial | Direct `adafruit-nrfutil` serial DFU | PASS | User confirmed `analysis\gif_audit\contact_Clean_up_the_poop.png` as the correct cleanup contract: a right-side vertical checker clear-wall travels right-to-left and removes pet plus poop together. Added `CONNECT_ANIMATION_SPEC.md`, replaced the prior water/flush-wave projection with a clipped 40-frame cleanup wall sequence, reset scene animation phase on entry, extended `Notice::kClean` visibility, and changed GAT562 toilet pacing to 100 ms per logical frame. Idle motion now uses a 16-phase Connection-style stage/tier movement table and GAT562 OLED idle pacing is 480 ms. Simulator proofs: `analysis\screen_simulator\out\compact_toilet_cleanup_wall_contact_x4.png` and `analysis\screen_simulator\out\compact_home_idle_connection_contact_x4.png`. Build size: RAM 13080/248832, Flash 246416/815104. Direct DFU reported `Device programmed.` and the board returned as application serial `COM51`. User visual confirmation on the actual OLED is still pending. |
| 2026-06-29 16:54 +08:00 | `EchoPet_GAT562_LoRa` superseded intermediate build | `COM52` bootloader, returned as `COM51` app serial | Direct `adafruit-nrfutil` serial DFU | PASS, SUPERSEDED | User reported that the poop-cleaning animation was obviously wrong and unlike the official/original behavior. This checkpoint replaced the broom/sweeping scene but still interpreted the reference too loosely as a left-to-right water/flush wave. Do not use this row as the current animation contract; the 17:36 row above supersedes it with the right-to-left checker clear-wall spec. `analysis\screen_simulator\out\compact_toilet_x4.png` belongs to this superseded build. Build size then was RAM 13080/248832, Flash 242584/815104. Direct DFU used `.pio\build\EchoPet_GAT562_LoRa\firmware.zip` on bootloader `COM52` and reported `Device programmed.` The board returned as application serial `COM51`. |
| 2026-06-29 16:34 +08:00 | `EchoPet_GAT562_LoRa` current build | `COM52` bootloader, returned as `COM51` app serial | Direct `adafruit-nrfutil` serial DFU | PASS | User observed that the GAT562 center character looked too large, mainly in the center area. Confirmed the target already uses `kEchoPetResources64` (`spriteScale = 1`, compact text enabled). Fixed the separate catalog idle draw path so compact 128x64 home-scene characters stay at 24x24 instead of being doubled to 48x48. Build size: RAM 13080/248832, Flash 242152/815104. Direct DFU used `.pio\build\EchoPet_GAT562_LoRa\firmware.zip` on bootloader `COM52` and reported `Device programmed.` The board returned as application serial `COM51`. Screen appearance still needs user confirmation after this flash. |
| 2026-06-29 16:23 +08:00 | `EchoPet_GAT562_LoRa` current build | `COM52` bootloader, returned to app serial after activation | Direct `adafruit-nrfutil` serial DFU | PASS | User observed that the center area was too cluttered: compact main scenes showed labels such as `HATCH` and button hints such as `A+ BOK`, unlike the Bandai original. Rebuilt `EchoPet_GAT562_LoRa` after removing compact in-frame page titles, compact control hints, and compact action-state labels from the center frame. Build size: RAM 13080/248832, Flash 242136/815104. Direct DFU used `.pio\build\EchoPet_GAT562_LoRa\firmware.zip` on bootloader `COM52` and reported `Device programmed.` A second DFU write after source cleanup reported the same result. Screen appearance still needs user confirmation after this flash. |
| 2026-06-29 16:13 +08:00 | `EchoPet_GAT562_LoRa` current build | `COM52` bootloader, returned as `COM51` app serial | Direct `adafruit-nrfutil` serial DFU | PASS | User explicitly allowed flashing after plugging in the GAT562. At flash time the environment was still named `EchoPet_GAT562`; it was renamed immediately afterward to `EchoPet_GAT562_LoRa` because the target is LoRa-enabled. `pio run -e EchoPet_GAT562 -t upload --upload-port COM51` twice touched app serial `COM51` and detected bootloader `COM52`, but `nrfutil` missed the open window. After the device stabilized in bootloader mode as `COM52` (`VID:PID=239A:002A`), direct `C:\Users\vicliu\.platformio\penv\Scripts\python.exe C:\Users\vicliu\.platformio\packages\tool-adafruit-nrfutil\adafruit-nrfutil.py dfu serial -p COM52 -b 115200 --singlebank -pkg .pio\build\EchoPet_GAT562\firmware.zip` reported `Device programmed.` The board then returned as application serial `COM51` (`VID:PID=239A:8029`). Build size: RAM 13080/248832, Flash 242400/815104. This proves build and flash only; screen, input, and LoRa behavior still need visual/interactive confirmation. |

## Hardware Projection

| Surface | GAT562 hardware | EchoPet event | Status | Evidence |
| --- | --- | --- | --- | --- |
| Display | 128x64 SSD1306 OLED on I2C SDA 13, SCL 14, address `0x3C` | Existing compact `128x64` resource family | CODED, FLASHED, HW OPEN | Needs user observation that boot/setup/home screens are visible and not clipped. |
| Center-frame guidance | Compact page labels, button hints, and action-state labels | Omitted from 128x64 center frame | CODED, FLASHED, HW OPEN | Needs user confirmation that the center scene is visually cleaner and closer to the original toy screen. |
| Center character scale | Compact catalog idle bitmap | 24x24 home-scene character on 128x64, not doubled to 48x48 | CODED, FLASHED, HW OPEN | Needs user confirmation that the GAT562 center character no longer dominates the 64x64 main region. |
| Toilet cleanup animation | Connection cleanup-wall contract | Right-side vertical checker clear-wall travels right-to-left and removes pet plus poop together; no broom, water wave, heart finish, or center text | SPEC LOCKED, CODED, SIM-PREVIEWED, FLASHED, HW OPEN | Compact simulator proof: `analysis\screen_simulator\out\compact_toilet_cleanup_wall_contact_x4.png`. The scene now snapshots pre-cleanup `messCount` so poop remains visible until the wall clips it away. Needs user confirmation on OLED. |
| Toilet cleanup completion | Scene landing behavior | After `Notice::kClean` clears, UI returns to the home playfield; it must not fall through to the `NO MESS` page. `Action::kToilet` only triggers cleanup when `messCount > 0`. | CODED, BUILT, FLASHED, HW OPEN | Added after user photo showed the cleanup path landing on the wrong no-mess/menu page. Needs user confirmation on OLED. |
| Home idle animation | Connection-style stage/tier movement | 16-phase horizontal hop/fall loop; normal jumps around, naughty jumps larger, serious is restrained, frail/senior falls instead of hopping | CODED-SUBSTITUTE, SIM-PREVIEWED, FLASHED, HW OPEN | Compact simulator proof: `analysis\screen_simulator\out\compact_home_idle_connection_contact_x4.png`. Exact per-character flip fidelity remains open. |
| Select | Joystick left, pin 30, active-low pullup | `A` / select | CODED, FLASHED, HW OPEN | Needs press-once confirmation. |
| Confirm | Joystick up, pin 28, active-low pullup | `B` / confirm | CODED, FLASHED, HW OPEN | Needs press-once confirmation. |
| Cancel | Joystick right, pin 31, active-low pullup | `C` / cancel | CODED, FLASHED, HW OPEN | Needs press-once confirmation. |
| A fallback | Primary button, pin 9, active-low pullup | `A` / select | CODED, FLASHED, HW OPEN | Needs fallback-button confirmation. |
| B fallback | Secondary button, pin 12, active-low pullup | `B` / confirm | CODED, FLASHED, HW OPEN | Needs fallback-button confirmation. |
| LoRa | SX1262 SPI SCK 43, MISO 45, MOSI 44, CS 42, DIO1 47, BUSY 46, RESET 38, POWER_EN 37, DIO2 RF switch, DIO3 TCXO 1.8 V | Same `FriendPacket` link contract as T-Echo-Lite LoRa | CODED, FLASHED, HW OPEN | Needs two-device send/receive proof. |

## Acceptance Checklist

| ID | Scenario | Expected result | Status | Evidence |
| --- | --- | --- | --- | --- |
| `GAT562-BOOT-001` | Power on or reset after `EchoPet_GAT562_LoRa` flash | OLED shows a clean EchoPet setup/home frame using the 128x64 compact layout | HW OPEN | Awaiting user observation. |
| `GAT562-DISPLAY-001` | Navigate through home, menu, setup/name, health, food, item, game, and sprite-proof pages | Text and sprites fit the 128x64 OLED without clipping or stale pixels | HW OPEN | Awaiting user observation. |
| `GAT562-DISPLAY-002` | Observe the home-scene center character on the GAT562 OLED | The character uses the compact 24x24 idle art and leaves clear breathing room inside the 64x64 center frame | HW OPEN | Awaiting user observation. |
| `GAT562-DISPLAY-003` | Trigger Toilet cleanup while poop is present | A right-side vertical checker clear-wall travels right-to-left, pushing/removing pet and poop together; no broom, water wave, heart finish, or center text appears; after the animation clears, the UI returns to the home playfield instead of showing the no-mess page | HW OPEN | Awaiting user observation. |
| `GAT562-DISPLAY-004` | Leave the pet on the home screen without operating buttons | The pet performs a regular Connection-style horizontal hop/fall loop rather than a tiny in-place bob | HW OPEN | Awaiting user observation. |
| `GAT562-JOY-001` | Press joystick left once on home/menu | Emits select/A once and advances/cycles as expected | HW OPEN | Awaiting user observation. |
| `GAT562-JOY-002` | Press joystick up once on menu/choice pages | Emits confirm/B once and enters/confirms the selected action | HW OPEN | Awaiting user observation. |
| `GAT562-JOY-003` | Press joystick right once in a submenu | Emits cancel/C once and exits/backtracks as expected | HW OPEN | Awaiting user observation. |
| `GAT562-JOY-004` | Hold joystick left/up/right past the long-press threshold, then release | Long event fires once; release does not emit an extra short event | HW OPEN | Awaiting user observation. |
| `GAT562-BTN-001` | Press primary button once | Acts as A/select fallback | HW OPEN | Awaiting user observation. |
| `GAT562-BTN-002` | Press secondary button once | Acts as B/confirm fallback | HW OPEN | Awaiting user observation. |
| `GAT562-LORA-001` | Start a Visit/Present/Game/Love link between GAT562 and another EchoPet LoRa device | Same visible result as the IR-visible `FriendPacket` contract projection | HW OPEN | Needs two-device proof. |

## Latest Verified Builds

Commands:

```powershell
cd C:\Users\vicliu\Projects\t-echo-lite-minimal-pio
pio run -e EchoPet_GAT562_LoRa
```

Latest result:

| Environment | Status | RAM | Flash |
| --- | --- | ---: | ---: |
| `EchoPet_GAT562_LoRa` | PASS | 13080 / 248832, 5.3% | 246400 / 815104, 30.2% |

Generated firmware files:

- `.pio\build\EchoPet_GAT562_LoRa\firmware.elf`
- `.pio\build\EchoPet_GAT562_LoRa\firmware.hex`
- `.pio\build\EchoPet_GAT562_LoRa\firmware.zip`

## Notes For Next Validation

- The board returned as application serial `COM51` after the successful DFU.
- If a future PlatformIO upload misses the bootloader window, first get the
  board into stable bootloader serial (`VID:PID=239A:002A`), then run direct
  `adafruit-nrfutil` against that COM port.
- Keep "build", "flash", and "hardware behavior observed" separate. The
  current state is build PASS and flash PASS; screen/input/LoRa behavior is
  still pending real observations.
