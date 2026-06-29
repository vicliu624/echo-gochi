# EchoPet Handoff

Last updated: 2026-06-29

This document is the working handoff for continuing EchoPet inside
`C:\Users\vicliu\Projects\t-echo-lite-minimal-pio`.

It is not a parity-complete claim. The current firmware builds, but
Tamagotchi Connection behavior, visual art, and hardware proof are not all
closed yet.

## Current Boundary

- Active project: `C:\Users\vicliu\Projects\t-echo-lite-minimal-pio`
- Local reference/resource bundle:
  `C:\Users\vicliu\Projects\t-echo-lite-minimal-pio\analysis\echopet_reference_resources`
- Legacy external resource workspace:
  `C:\Users\vicliu\Projects\pet`
- Supported target boards:
  - T-Echo-Lite nRF52840, 176x192 SSD1681 e-paper.
  - GAT562 Mesh EVB Pro, 128x64 SSD1306 OLED.
- Target communication route: keep the same visible IR/Connection behavior,
  but carry it over LoRa.
- Do not flash unless the user explicitly asks for flashing.
- Runtime visual resources must be C/C++ source bitmap arrays and composed
  frame tables. Do not add runtime PNG/JPG/GIF/BMP assets or runtime file-load
  dependencies for animation, character, item, icon, or menu art.
- Animation contracts that have been explicitly locked during hardware review
  are recorded in `CONNECT_ANIMATION_SPEC.md`.
- Official/reference raster files may exist only under `analysis/`, including
  `analysis\echopet_reference_resources`, as proof and comparison material.
- Two supported layout families are still part of the requirement:
  - 128x64 compact: center 64x64 main scene, left/right 32x64 fixed menu bands.
  - 176x192 hardware intent: center 128x128 main scene, 32-pixel side menu
    bands, with top/bottom utility lines kept to one line each.

## Reference Websites And Source IDs

The canonical source ledger is `CONNECT_SOURCE_LEDGER.md`. The list below is
duplicated here so future work can resume without hunting for the websites.

Confidence levels:

- A: official Bandai page/manual, or direct T-Echo-Lite hardware evidence.
- B: cross-checked community/wiki/gameplay research with useful provenance.
- C: single community claim, historical release cross-check, image chart, or
  low-confidence substitute. Do not promote C-level values to final parity
  without better evidence.
- D: current EchoPet implementation fact only; not a source of truth.

### Official Tamagotchi / Bandai Sources

| Source ID | Level | URL | Used for |
| --- | --- | --- | --- |
| `OFF-HOWTO` | A | [Tamagotchi Connection how-to](https://tamagotchi-official.com/gb/series/connection/howto/) | Official feature surface: care menus, 50+ characters, seven games, 150+ items, password, shop secret-code gesture, IR connection, Love/next generation, and official animation/reference material. Official raster files are reference/proof only, not runtime firmware assets. |
| `OFF-CHAR` | A | [Official character page](https://tamagotchi-official.com/gb/series/connection/character/) | Official character names, roster targets, visual references, page/slot IDs, and character proof sheets. Runtime character rows must remain C++ bitmap rows. |
| `OFF-MANUAL` | A | [Official toy manual page](https://tamagotchi-official.com/manual/toy/connection/) | Official controls, setup, health pages, manual flow, and menu behavior. The English PDF linked from this page is also part of this source ID. |
| `OFF-P1-MANUAL` | A | [Bandai Tamagotchi P1 1996 user guide scan/OCR](https://archive.org/details/bandai-tamagotchi-p1-1996) | Original duck/flush cleanup semantics: highlight the Duck icon and press B to flush the screen after a mess. Used as an official historical behavior constraint, not as runtime art. |
| `OFF-NEWS` | A | [Official Connection news/update page](https://tamagotchi-official.com/gb/series/connection/news/?c=update) | Official password item announcements and released password-item flow. |

### Tamagotchi Wiki / Fandom Sources

| Source ID | Level | URL | Used for |
| --- | --- | --- | --- |
| `WIKI-2024` | B | [Tamagotchi Connection (2024)](https://tamagotchi.fandom.com/wiki/Tamagotchi_Connection_%282024%29) | 2024 feature summary, shop, games, inventory caps, standard-game top-prize table, senior game availability. |
| `WIKI-ITEM-2024` | B | [Connection 2024 item list](https://tamagotchi.fandom.com/wiki/Tamagotchi_Connection_%282024%29/Item_list) | Food/item names, shop restocks, storage caps, prices, secret-code items, item restrictions, liked-by-all food notes. This is source data, not official art. |
| `WIKI-CODE-2024` | B | [Connection 2024 password and secret codes](https://tamagotchi.fandom.com/wiki/Tamagotchi_Connection_%282024%29/Password_and_Secret_Codes_list) | 2024 password values, shop A/B/C secret-code values, prices, duplicate notes. |
| `WIKI-CHAR-2024` | B | [Connection 2024 character list](https://tamagotchi.fandom.com/wiki/Tamagotchi_Connection_%282024%29/Character_list) | Character list, care tiers, care-mistake categories, tier-to-character mappings, growth routing. |
| `WIKI-KING-2024` | B | [Gotchi King](https://tamagotchi.fandom.com/wiki/Gotchi_King), [Super Unchikun](https://tamagotchi.fandom.com/wiki/Super_Unchikun) | King donation and Super Unchikun/no-mess cleanup behavior. |
| `WIKI-DEATH` | B/C | [Death](https://tamagotchi.fandom.com/wiki/Death) | General death/starvation/sickness rules across releases; used only to constrain substitute death behavior. Exact 2024 thresholds are still open. |
| `WIKI-SICKNESS` | B/C | [Sickness](https://tamagotchi.fandom.com/wiki/Sickness) | General sickness/toothache behavior; used for the fifteen-consecutive-sweet-snack toothache direction. Exact 2024 short-period duration remains open. |
| `WIKI-CARE` | B/C | [Care](https://tamagotchi.fandom.com/wiki/Care) | Hungry/Happy care concepts and hidden heart reserve model. Exact 2024 decay windows remain open. |
| `WIKI-GOTCHI-POINTS` | B | [Gotchi Points](https://tamagotchi.fandom.com/wiki/Gotchi_Points) | Gotchi Point wallet cap behavior, including the 9,999 cap used by V2/V3 and 2024-style behavior. |
| `WIKI-FAQ-2024` | B | [NightBladeSequel Connection 2024 FAQ](https://tamagotchi.fandom.com/wiki/User_blog:NightBladeSequel/Tamagotchi_Connection_2024_FAQ) | Care mistakes, life cycle, marriage/baby, pause behavior, growth/care cross-checks. |

### Community Guides And Historical Cross-Checks

| Source ID | Level | URL | Used for |
| --- | --- | --- | --- |
| `GG-GROWTH-2024` | B | [Gotchi Garden Connection 20th character guide](https://gotchi-garden.blogspot.com/p/connection-20th-anniversary-character.html) | Growth guide, care-mistake categories, tiers, lifespan; cross-check for wiki growth rules. |
| `GG-SPECIAL-2024` | B | [Gotchi Garden Connection 20th guide](https://gotchi-garden.blogspot.com/p/tamagotchi-connection-20th-anniversary.html) | Shop A/B/C special-code entry instructions and seven code values; cross-check for `WIKI-CODE-2024`. |
| `TAMA-PALACE-PAUSE-2024` | B | [Tama-Palace pause article](https://tamapalace.tumblr.com/post/755897449829351424/how-to-pause-a-tamagotchi-connection-20th) | 2024 pause behavior: no older-model pause feature; clock-set trick pauses growth while editing time. |
| `CURL-V1-BASICS` | C | [Curlour Plus/V1 basics](https://curlour.neocities.org/en/tmgc/plus/bases) | Legacy Plus/V1 button and clock/pause notes; used only to keep regional/version differences explicit. |
| `CURL-CONNECTION-IDLE` | B/C | [Curlour Connection / Plus idle animations](https://sa311.tumblr.com/post/666773897457401856/tamagotchi-connection-plus-series-idle) | Connection/Plus idle motion categories: baby/toddler no horizontal flip, teens use side/flipped motion, adults split into Serious/Normal/Naughty/Frail/Senior motion styles. Used for motion rules, not runtime art. |
| `CURL-2024-EVENTS` | B/C | [Curlour Connection 2024 events](https://curlour.neocities.org/en/tmgc/cone2/eventos) | Seasonal/date events, birthday, New Year, winter, Easter, back-to-school, Halloween, Thanksgiving, Christmas, Santaclautchi timing, seasonal shop-food dates. |
| `CURL-ENTAMA-CARE` | C | [Curlour Entama requirements](https://curlour.neocities.org/en/tmgc/entama/requirements) | Cross-version Hungry/Happy/poop timing table. This is only a low-confidence substitute while exact Connection 2024 timing is unavailable. |
| `TAMATALK-BUMP-V3` | B/C | [TamaTalk V3 Bump/Heading guide](https://www.tamatalk.com/threads/guide-to-ver-3-bump-heading.56907/), [Bump score chart image](https://imgur.com/a/eim3rkW) | V3 Bump/Heading guide and Bump weight-band score chart; useful substitute until 2024-specific Bump/Sprint probability proof exists. |
| `TAMATALK-MEMORY-V3` | C | [TamaTalk V3 games guide](https://www.tamatalk.com/threads/guide-for-v3-games.53259/) | Historical V3 Memory/Sprint play notes. Kept as cross-check only; 2024-specific game notes override it where available. |
| `TAMATOWN-V3-GROWTH` | C | [TamaTown V3 growth chart](https://tamatown.com/guides/tamagotchi-connection-v3-growth-chart/) | V3 growth chart provenance. Image-centric and not directly parseable yet. |

### Local Reference And Hardware Evidence

| Source ID | Level | Location | Used for |
| --- | --- | --- | --- |
| `LOCAL-GIF` | A for user intent, not official behavior | `analysis\echopet_reference_resources\concept_design_gifs` copied from `C:\Users\vicliu\Projects\pet` | User-provided concept GIFs and animation phase expectations. Use them for scene contracts and motion intent, not official behavior truth. |
| `LOCAL-POOP-GIF-AUDIT` | B visual cross-check, user-confirmed spec on 2026-06-29 | `analysis\gif_audit\contact_Clean_up_the_poop.png` | Local extracted cleanup animation frames. Confirms the accepted visual motion is a right-side vertical checker clear-wall traveling right-to-left and removing pet plus poop together, not a broom, generic water wave, sparkle-only cleanup, poop-only cleanup, or happy/heart finish. |
| `HW-TECHO` | A once observed | `T_ECHO_LITE_HARDWARE_ACCEPTANCE.md` | T-Echo-Lite refresh, controls, sprite proof, and two-device LoRa acceptance. Rows are not final until real hardware evidence is recorded. |
| `HW-GAT562` | A once observed | `GAT562_HARDWARE_ACCEPTANCE.md` | GAT562 128x64 OLED, joystick input projection, and LoRa acceptance. Build and flash are recorded; screen/input/LoRa rows remain open until real hardware observations are written down. |
| Trail-Mate reference | local implementation reference | `C:\Users\vicliu\Projects\trail-mate` | Compare T-Echo-Lite e-paper refresh and driver strategy before changing refresh policy further. |

## Build Status

Builds were verified sequentially, not in parallel. PlatformIO can race on the
temporary `.ino.cpp` conversion file if both environments are built at the same
time.

Commands:

```powershell
cd C:\Users\vicliu\Projects\t-echo-lite-minimal-pio
pio run -e EchoPet
pio run -e EchoPet_LoRa
pio run -e EchoPet_GAT562_LoRa
```

Latest verified result:

| Environment | Status | RAM | Flash |
| --- | --- | ---: | ---: |
| `EchoPet` | PASS | 15556 / 248832, 6.3% | 213924 / 815104, 26.2% |
| `EchoPet_LoRa` | PASS | 16528 / 248832, 6.6% | 247064 / 815104, 30.3% |
| `EchoPet_GAT562_LoRa` | PASS | 13080 / 248832, 5.3% | 246400 / 815104, 30.2% |

Latest GAT562 flash result:

- 2026-06-29 18:14 +08:00: `EchoPet_GAT562_LoRa` was rebuilt and installed
  after the user reported that poop still disappeared instantly. The fix
  captures the pre-cleanup `messCount` into the Toilet scene UI state before
  `EchoPetModel::apply(Action::kToilet)` clears the model, and
  `drawToiletScene()` renders poop from that scene snapshot until the
  right-to-left checker wall clips it away. First automatic PlatformIO upload
  touched `COM51` but `nrfutil` reported `No data received on serial port`; the
  board stayed in bootloader as `COM52`. Direct serial DFU to `COM52` then
  uploaded `.pio\build\EchoPet_GAT562_LoRa\firmware.zip`, activated firmware,
  and reported `Device programmed.` The board returned as app serial `COM51`.
  Build size: RAM 13080/248832, Flash 246400/815104. Actual OLED visual
  acceptance is pending user observation.
- 2026-06-29 17:58 +08:00: `EchoPet_GAT562_LoRa` was rebuilt and uploaded
  after the user photo showed the cleanup path landing on the wrong page. The
  fix is state-machine level: after `Notice::kClean` clears, `UiMode::kToilet`
  exits to the home playfield instead of falling through to `NO MESS`.
  `Action::kToilet` now starts cleanup only when `messCount > 0`; low hygiene
  alone no longer masquerades as poop cleanup. Build size: RAM 13080/248832,
  Flash 246352/815104. `pio run -e EchoPet_GAT562_LoRa -t upload`
  auto-detected app serial `COM51`, reset into bootloader `COM52`, uploaded
  `.pio\build\EchoPet_GAT562_LoRa\firmware.zip`, activated the firmware, and
  reported `Device programmed.` Actual OLED visual acceptance is pending user
  observation.
- 2026-06-29 17:45 +08:00: `EchoPet_GAT562_LoRa` was rebuilt and
  uploaded again after aligning source proof-frame naming with the accepted
  cleanup-wall contract (`WALL0`/`WALL1`, `CleanupWall` internals instead of
  historical flush/sweep wording). Build size was unchanged: RAM
  13080/248832, Flash 246416/815104. `pio run -e EchoPet_GAT562_LoRa -t upload`
  auto-detected app serial `COM51`, reset into bootloader `COM52`, uploaded
  `.pio\build\EchoPet_GAT562_LoRa\firmware.zip`, activated the firmware, and
  reported `Device programmed.` Actual OLED visual acceptance is pending user
  observation.
- 2026-06-29 17:36 +08:00: `EchoPet_GAT562_LoRa` was rebuilt and
  reinstalled after the user confirmed
  `analysis\gif_audit\contact_Clean_up_the_poop.png` as the correct cleanup
  animation: a right-side vertical checker clear-wall travels right-to-left and
  removes the pet and poop together. This contract is now recorded in
  `CONNECT_ANIMATION_SPEC.md`. Firmware now resets scene animation phase on
  entry, extends `Notice::kClean` visibility, draws a clipped 40-frame cleanup
  wall instead of the previous water/flush wave, and uses 100 ms toilet frames
  on the GAT562 OLED. Home idle now uses a 16-phase Connection-style motion
  table with adult-tier-specific Serious/Normal/Naughty/Frail behavior, and
  GAT562 idle pacing is 480 ms per logical frame. Compact simulator proofs:
  `analysis\screen_simulator\out\compact_toilet_cleanup_wall_contact_x4.png`
  and `analysis\screen_simulator\out\compact_home_idle_connection_contact_x4.png`.
  Build size: RAM 13080/248832, Flash 246416/815104. Direct DFU on bootloader
  `COM52` reported `Device programmed.`, and the board returned as app serial
  `COM51`. Actual OLED visual acceptance is pending user observation.
- 2026-06-29 16:54 +08:00: SUPERSEDED by the 17:36 cleanup-wall spec above.
  At this checkpoint, `EchoPet_GAT562_LoRa` was rebuilt and reinstalled on the
  plugged-in GAT562 after replacing the wrong broom/sweeping toilet cleanup
  scene with an intermediate flush/wipe projection. That intermediate build
  still interpreted the visual cross-check too loosely as a left-to-right
  water/flush wave. Do not use this row as the current animation contract.
  Compact simulator proof from that superseded build was regenerated at
  `analysis\screen_simulator\out\compact_toilet_x4.png`. Build size: RAM
  13080/248832, Flash 242584/815104. Direct DFU on bootloader `COM52`
  reported `Device programmed.`, and the board returned as app serial `COM51`.
  After source naming/ledger cleanup, the same GAT562 build was recompiled and
  reinstalled again at 2026-06-29 16:58 +08:00; DFU again reported
  `Device programmed.` and the board returned as `COM51`.
- 2026-06-29 16:34 +08:00: `EchoPet_GAT562_LoRa` was rebuilt and
  reinstalled on the plugged-in GAT562 after confirming that the target already
  selected `kEchoPetResources64`. The remaining center-frame size issue was the
  home-scene catalog idle character path: compact 128x64 resources had
  `spriteScale = 1`, but the catalog idle bitmap was still doubled to 48x48.
  Compact catalog idle art now stays at 24x24. Build size: RAM
  13080/248832, Flash 242152/815104. Direct DFU on bootloader `COM52`
  reported `Device programmed.`, and the board returned as app serial `COM51`.
- 2026-06-29 16:23 +08:00: `EchoPet_GAT562_LoRa` was rebuilt and
  reinstalled on the plugged-in GAT562. The 128x64 compact main scene no
  longer draws page titles, compact control hints, or compact action-state
  labels inside the center frame. This removes the observed `HATCH` and
  `A+ BOK` clutter from the OLED main area. Direct DFU on bootloader `COM52`
  reported `Device programmed.` A second DFU write after source cleanup
  reported the same result.
- 2026-06-29 16:13 +08:00: `EchoPet_GAT562_LoRa` was installed on the
  plugged-in GAT562 via direct `adafruit-nrfutil` serial DFU.
- At the moment of flashing, the PlatformIO environment was still named
  `EchoPet_GAT562`; it was renamed immediately afterward to
  `EchoPet_GAT562_LoRa` because the target is LoRa-enabled.
- PlatformIO's first two upload attempts touched application serial `COM51`
  and detected bootloader `COM52`, but missed the short bootloader open window.
- The successful path was direct DFU on stable bootloader `COM52`:
  `python.exe adafruit-nrfutil.py dfu serial -p COM52 -b 115200 --singlebank
  -pkg .pio\build\EchoPet_GAT562\firmware.zip`.
- The tool reported `Device programmed.` and the board returned as application
  serial `COM51` (`VID:PID=239A:8029`).
- This closes build/flash evidence only. Screen, joystick, and LoRa hardware
  behavior still need observations in `GAT562_HARDWARE_ACCEPTANCE.md`.

Generated firmware files:

- `.pio\build\EchoPet\firmware.elf`
- `.pio\build\EchoPet\firmware.hex`
- `.pio\build\EchoPet\firmware.zip`
- `.pio\build\EchoPet_LoRa\firmware.elf`
- `.pio\build\EchoPet_LoRa\firmware.hex`
- `.pio\build\EchoPet_LoRa\firmware.zip`
- `.pio\build\EchoPet_GAT562_LoRa\firmware.elf`
- `.pio\build\EchoPet_GAT562_LoRa\firmware.hex`
- `.pio\build\EchoPet_GAT562_LoRa\firmware.zip`

## Current Audit Snapshot

Primary reports:

- `CONNECT_ANIMATION_SPEC.md`
- `analysis\completion_audit\completion_audit.md`
- `analysis\visual_alignment\visual_alignment_report.md`
- `analysis\runtime_resource_audit\runtime_resource_audit.md`
- `analysis\source_blocker_matrix\source_blocker_matrix.md`
- `analysis\source_evidence_probe\source_evidence_probe.md`
- `T_ECHO_LITE_HARDWARE_ACCEPTANCE.md`

Current summary from the latest generated reports:

- Completion audit is not complete.
- Checklist open rows: 3, all hardware-gated.
- Task rows marked `TODO`: 0.
- Task rows marked `DOING`: 0.
- Task rows marked `HW`: 16.
- Task rows marked `CODED-SUBSTITUTE`: 10.
- Hardware acceptance matrix TODO rows: 55.
- Hardware TODO source readiness READY/PARTIAL/MISSING: 55 / 0 / 0.
- Hardware acceptance matrix groups:
  - button: 7
  - refresh: 18
  - sprite_proof: 21
  - two_device: 9
- Source blocker matrix rows: 21.
- Source evidence probe rows: 16:
  - PASS: 11
  - CODED-SUBSTITUTE: 5
- Visual alignment gate:
  - PASS: 194
  - OPEN: 2
  - HW: 1
  - FAIL: 0
- Runtime resource audit:
  - Overall: PASS
  - Runtime raster files outside `analysis/`: 0
  - Forbidden runtime image/file-load pattern hits: 0

## What Is Already Moved/Working

- EchoPet now lives in `t-echo-lite-minimal-pio` and builds there.
- Both non-LoRa and LoRa PlatformIO environments build.
- GAT562 has a dedicated `EchoPet_GAT562_LoRa` PlatformIO environment. It builds
  the same EchoPet model/UI/link code against the GAT562 board profile and a
  128x64 SSD1306 OLED target.
- GAT562 display code uses the existing compact `128x64` resource family:
  `drawEchoPet()` selects `kEchoPetResources64` when the display dimensions are
  `128x64`.
- GAT562 compact main scenes intentionally omit in-frame page labels, compact
  control hints, and compact action-state labels so the center area stays close
  to the original Tamagotchi-style toy screen rather than becoming a help UI.
- GAT562 compact home-scene catalog idle characters use the compact 24x24
  scale. The 128x64 resource family is still responsible for selecting the
  small layout; the draw path must not re-expand the center character to 48x48.
- The toilet cleanup scene now follows `CONNECT_ANIMATION_SPEC.md`: a
  right-side vertical checker clear-wall travels right-to-left and removes pet
  plus poop together. No broom, generic water wave, sparkle-only cleanup,
  poop-only cleanup, happy/heart finish, or center prompt is used.
- GAT562 input projects the joystick into the same EchoPet A/B/C semantic
  events: left = select/A, up = confirm/B, right = cancel/C. The primary and
  secondary hardware buttons are kept as A/B fallbacks.
- GAT562 LoRa uses the GAT562 SX1262 pin set, power-enable pin, DIO2 RF switch,
  and DIO3 TCXO voltage from `echopet_target_config.h` instead of the
  T-Echo-Lite RF switch pins.
- The LoRa build includes `RadioLib`.
- Fixed menu icons are now generated as C++ arrays with 12x12 and 30x30
  routes sharing the same menu semantics.
- Character visual rows are 50 official-derived runtime C++ rows:
  - 16x16 portrait rows for friend/family/catalog surfaces.
  - 24x24 idle rows for main-scene character rendering.
- Catalog and souvenir drawing routes are row-local rather than only generic
  icon-family routes.
- Runtime resource contract is currently clean: C++ bitmap arrays, no runtime
  PNG/JPG/GIF/BMP loading.
- The code-side refresh path has been compared with the local Trail-Mate
  T-Echo-Lite reference. EchoPet now initializes display through a dedicated
  `beginDisplay()` helper that clears the Adafruit EPD buffer before first
  full refresh, disables text wrap, resets local frame/base-map state, keeps
  synchronous whole-frame partial refresh, suppresses identical frames, and
  uses the Trail-Mate-style 50 ms submit guard.
- Shop/password/item/game/growth/link behavior has many source-shaped tables
  and probes, but several values remain substitute rather than exact.
- LoRa code path shares the same `FriendPacket` and `receiveFriendPacket()`
  behavior as the IR-visible contract projection.

## What Is Not Aligned Yet

### 1. Hardware Refresh And Physical Interaction

This is the biggest practical blocker because the user has already observed
whitening, blur, slow game feedback, and questionable partial refresh behavior
on real T-Echo-Lite hardware.

Open hardware rows include:

- Esc/Home/Email short press must each fire exactly once.
- Esc/Home/Email long press threshold must be measured.
- Release after long press must not emit an extra short press.
- Home long press should enter `CLOCK` edit pause from home.
- Email long press should enter sprite proof diagnostic path from home.
- Cold boot must show a clean frame.
- Idle refresh must not progressively whiten or blur.
- Menu cycling 50 presses must not cause intrusive full-screen flashes.
- Food, toilet, medicine, discipline, lights, shop, password, and all seven
  games must remain readable at real e-paper speed.
- Sprite proof families must be photographed/recorded on hardware.
- Compact and large layout profiles need physical readability proof.

Current code-side refresh strategy under test:

- Trail-Mate comparison completed on 2026-06-29.
- Startup clears the local Adafruit EPD framebuffer before the first full
  refresh.
- Changed frames use synchronous whole-frame partial refresh after one
  `FAST_REFRESH` base-map setup.
- Identical frames are suppressed.
- The application submit guard is 50 ms, matching the Trail-Mate T-Echo-Lite
  reference path.
- This is not hardware proof. Keep refresh policy simple enough for the panel:
  - avoid invisible periodic full refreshes that users notice;
  - avoid overusing partial refresh if it whitens the screen;
  - record every accepted strategy in `T_ECHO_LITE_HARDWARE_ACCEPTANCE.md`.

### 1b. GAT562 128x64 OLED Hardware Interaction

GAT562 compile and flash are done, but physical acceptance is still open.

Open GAT562 rows include:

- Cold boot must show a clean 128x64 OLED frame.
- Compact resources must be selected automatically and fit the 128x64 screen.
- The home-scene center character should read as compact 24x24 art, not a
  doubled 48x48 catalog idle sprite.
- Joystick left must act as select/A exactly once per press.
- Joystick up must act as confirm/B exactly once per press.
- Joystick right must act as cancel/C exactly once per press.
- Long-press behavior on left/up/right must not emit an extra short press on
  release.
- Primary and secondary buttons should remain usable A/B fallbacks.
- OLED frame submission must not flicker or leave stale pixels across menu,
  care-scene, setup, game, and sprite-proof screens.
- GAT562 SX1262 LoRa must be validated with either another GAT562 or a
  T-Echo-Lite LoRa build using the same `FriendPacket` link contract.

Record observations in `GAT562_HARDWARE_ACCEPTANCE.md`.

### 2. Final Visual Art

Visual simulator evidence is much better than before, but official-grade final
art is not closed.

Open visual alignment rows:

- Catalog item final per-row art:
  - 160 catalog rows exist.
  - 12 non-souvenir rows are official-source overrides.
  - 148 catalog rows still need accepted final art.
- Souvenir official-look acceptance:
  - 64 souvenir rows exist.
  - 2 rows are official-source overrides.
  - 62 souvenir rows still need accepted final art.
- Egg/crack/hatch art has code and proof routes, but still needs accepted
  final art plus real hardware readability proof.
- Hardware visual proof remains open because simulator output cannot prove
  whitening, ghosting, partial refresh cadence, or physical key feel.

Do not solve this by loading official PNG/GIF/JPG at runtime. The replacement
path is:

1. Use official/reference files only as analysis/reference material.
2. Convert or redraw into rights-cleared/user-accepted C++ bitmap rows.
3. Update the replacement manifest and drawing spec.
4. Regenerate preview/proof sheets.
5. Keep final acceptance explicit.

Relevant files:

- `analysis\resource_replacement_manifest\*`
- `analysis\resource_drawing_spec\*`
- `analysis\official_reference\*`
- `src\EchoPetCatalogVisuals.cpp`
- `src\EchoPetCharacterVisuals.cpp`
- `src\EchoPetMenuIconResources.cpp`
- `src\EchoPetSprites.cpp`

### 3. Source-Exact Behavior Still Marked Substitute

The following cannot be marked final until exact source evidence, hardware
evidence, or accepted final art exists.

From `analysis\source_blocker_matrix\source_blocker_matrix.md`:

| Area | Missing exact alignment |
| --- | --- |
| Toilet / poop timing | Exact Connection 2024 poop interval by life stage. |
| Medicine / sickness | Exact sickness probability, official toothache short-period length, and medicine cure/timing probabilities. |
| Death / pass-away | Exact 2024 Grim/pass-away thresholds and wording. |
| Shop | Exact random sale frequency, seasonal collision priority, and Ojitchi cart availability duration. |
| Games | Official low-score scaling, Bump/Sprint probability curves, and non-point stat effects. |
| Catalog / item random effects | Exact Plant/Shovel/Chest/Fishing Pole/Lamp probability tables, reward amounts, and item animation timing. |
| Growth/family | Exact same-pool character probability, matchmaker/senior overlap, transition ordering, and hardware proof. |
| Stat decay / events | Exact Connection 2024 decay windows, sleep windows, lifecycle timing, calendar cutscene wording/art, and probability/timing values. |
| Password/secret-code visuals | Behavior is coded, but final per-item art is open. |

Current source evidence probe:

- PASS rows:
  - shop secret gesture
  - shop windows
  - game top-prize table
  - game pacing targets
  - attention windows
  - inventory caps
  - default foods
  - wallet cap
  - password/secret tables
  - link-game requirements
  - item dependencies
- CODED-SUBSTITUTE rows:
  - shop sale cadence
  - toothache short-period duration
  - death/pass-away thresholds
  - King donation cutscene/art
  - random item effect probabilities

### 4. Two-Device LoRa / IR-Visible Contract

The code is structured so that LoRa carries the same visible contract as the
original IR-style connection behavior, but it has not been proven with two
T-Echo-Lite devices.

Open dual-device rows:

- Visit success and timeout.
- Present success.
- Present full-inventory refusal.
- Game success with both devices resolving the same visible outcome.
- Love reject.
- Love partner.
- Love baby / next generation.
- LoRa parity with the IR-visible contract.

Relevant files:

- `LINK_CONTRACT.md`
- `src\EchoPetModel.h`
- `src\EchoPetModel.cpp`
- `src\EchoPet.ino`
- `T_ECHO_LITE_HARDWARE_ACCEPTANCE.md`

### 5. User-Visible Interaction Completeness

Many menus are no longer just text, but hardware proof is still missing for
whether the visible feedback feels like a Tamagotchi Connection on the real
panel.

Priority interaction checks:

- Fixed icon selection/cycling with Esc/Home/Email.
- Food scene bite/result visibility.
- Toilet cleanup-wall/removal visibility.
- Medicine dose/recovery/refusal visibility.
- Discipline TIME OUT/PRAISE/invalid/missed visibility.
- Lights dark-room stability.
- Shop booth/shopkeeper/purchase/secret-code feedback.
- Password entry feedback.
- All seven games at real panel speed.

## Recommended Next Work Order

### P0. Rebase Your Workspace Around This Project

1. Set working directory to:

   ```powershell
   cd C:\Users\vicliu\Projects\t-echo-lite-minimal-pio
   ```

2. Confirm builds:

   ```powershell
   pio run -e EchoPet
   pio run -e EchoPet_LoRa
   ```

3. Do not run both PlatformIO environments in parallel.

4. Re-run the audits before starting major changes:

   ```powershell
   $py='C:\Users\vicliu\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe'
   if (!(Test-Path -LiteralPath $py)) { $py='python' }
   & $py analysis\runtime_resource_audit\generate_runtime_resource_audit.py
   & $py analysis\visual_alignment\generate_visual_alignment_gate.py
   & $py analysis\source_evidence_probe\generate_source_evidence_probe.py
   & $py analysis\source_blocker_matrix\generate_source_blocker_matrix.py
   & $py analysis\completion_audit\generate_completion_audit.py
   ```

### P1. Fix Or Prove The E-Paper Refresh Strategy

Do this before polishing more sprites, because bad refresh makes every sprite
look wrong.

Tasks:

- Code-side audit and Trail-Mate comparison are complete as of 2026-06-29.
- Install the current build only if the user explicitly allows flashing.
- Identify on real hardware whether the current full/partial refresh
  scheduling still causes whitening.
- If whitening/blur persists, collect evidence and simplify the policy further
  from the observed failure rather than adding animation complexity.
- Keep `T_ECHO_LITE_HARDWARE_ACCEPTANCE.md` as the accepted refresh-strategy
  and observation log.
- Re-test:
  - idle 5 minutes;
  - menu cycling 50 presses;
  - food/toilet/medicine scene loops;
  - all seven games.

Acceptance:

- No progressive whitening.
- No progressive blur.
- No user-visible surprise full refresh during normal short animations.
- Notice/result screens remain legible long enough.

### P2. Hardware Button And Sprite Proof Burn-Down

Tasks:

- Validate Esc/Home/Email short press.
- Validate Esc/Home/Email long press.
- Validate Home long press -> clock edit pause.
- Validate Email long press -> sprite proof.
- Validate release-after-hold suppression.
- Photograph or record every sprite proof family listed in
  `T_ECHO_LITE_HARDWARE_ACCEPTANCE.md`.

Acceptance:

- Fill the 55 TODO rows in `T_ECHO_LITE_HARDWARE_ACCEPTANCE.md` with real
  evidence notes.
- Hardware acceptance matrix TODO count should decrease after regenerating:

  ```powershell
  $py='C:\Users\vicliu\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe'
  if (!(Test-Path -LiteralPath $py)) { $py='python' }
  & $py analysis\hardware_acceptance_matrix\generate_hardware_acceptance_matrix.py
  & $py analysis\completion_audit\generate_completion_audit.py
  ```

### P3. Visual Final Art Pass

Tasks:

- Start with catalog item rows used by visible flows first:
  - password rewards,
  - secret-code rewards,
  - shop items,
  - Connection-game required items,
  - random-effect items.
- Then do souvenir rows.
- Keep every runtime asset as C++ bitmap data.
- Update:
  - `src\EchoPetCatalogVisuals.cpp`
  - `analysis\resource_replacement_manifest\*`
  - `analysis\resource_drawing_spec\*`
  - visual proof sheets.

Acceptance:

- Catalog rows needing final art goes from 148 toward 0.
- Souvenir rows needing final art goes from 62 toward 0.
- `analysis\visual_alignment\visual_alignment_report.md` loses its two
  `OPEN` rows.
- `analysis\runtime_resource_audit\runtime_resource_audit.md` remains PASS.

### P4. Source-Exact Table Replacement

Only replace a substitute value when source evidence is recorded.

Priority exact-source targets:

1. Game low-score rewards and Bump/Sprint probability curves.
2. Sickness/toothache short-period and cure probabilities.
3. Death/pass-away/Grim thresholds.
4. Poop/decay/sleep timing.
5. Shop sale frequency, seasonal collision priority, and Ojitchi cart duration.
6. Growth same-pool probability and matchmaker/senior transition ordering.

When changing any table:

- Update `CONNECT_SOURCE_LEDGER.md`.
- Update the code table/constants.
- Update or add source evidence probe rows.
- Regenerate source/blocker/completion reports.

### P5. Two-Device LoRa Validation

Do this after single-device refresh and controls are stable.

Tasks:

- Prepare two T-Echo-Lite devices with the same LoRa firmware.
- Validate Visit, Present, Game, and Love flows.
- Confirm both devices show compatible standby/send/receive/result states.
- Confirm timeout does not mutate pet state.
- Confirm full inventory refusal is visible and state-safe.
- Confirm Love partner/baby/next-generation flow.

Acceptance:

- `T_ECHO_LITE_HARDWARE_ACCEPTANCE.md` dual-device rows are filled.
- `LINK_CONTRACT.md` remains true.
- LoRa visible results match the IR-visible behavior contract.

### P6. Final Closure Audit

Final parity cannot be claimed until all of these are true:

- `analysis\completion_audit\completion_audit.md` has:
  - checklist open rows: 0
  - hardware acceptance TODO rows: 0
  - source blocker rows: 0, or all remaining rows are explicitly accepted by
    the user as non-goals
  - visual gate OPEN/HW/FAIL: 0
- `analysis\runtime_resource_audit\runtime_resource_audit.md` remains PASS.
- `pio run -e EchoPet` passes.
- `pio run -e EchoPet_LoRa` passes.
- `analysis\echopet_reference_resources` is present and up to date.

The old external mirror at
`C:\Users\vicliu\Projects\pet\echopet_reference_resources` has been copied into
this project. If that external mirror changes and needs to be re-imported, use:

```powershell
$src='C:\Users\vicliu\Projects\pet\echopet_reference_resources'
$dst='C:\Users\vicliu\Projects\t-echo-lite-minimal-pio\analysis\echopet_reference_resources'
New-Item -ItemType Directory -Force -Path $dst | Out-Null
Get-ChildItem -LiteralPath $src -Force | Copy-Item -Destination $dst -Recurse -Force
```

## Important Files

### Firmware

- `src\EchoPet.ino`
- `src\EchoPetModel.h`
- `src\EchoPetModel.cpp`
- `src\EchoPetDisplay.cpp`
- `src\EchoPetUi.cpp`
- `src\EchoPetSprites.cpp`
- `src\EchoPetCatalog.cpp`
- `src\EchoPetCatalog.h`
- `src\EchoPetCatalogVisuals.cpp`
- `src\EchoPetCharacterCatalog.cpp`
- `src\EchoPetCharacterVisuals.cpp`
- `src\EchoPetMenuIconResources.cpp`
- `src\EchoPetResources64.cpp`
- `src\EchoPetResources128.cpp`

### Contracts And Specs

- `README.md`
- `ECHOPET_MIGRATION_BOUNDARY.md`
- `RUNTIME_RESOURCE_CONTRACT.md`
- `VISUAL_REFERENCE_CONTRACT.md`
- `VISUAL_ALIGNMENT_GATE.md`
- `CONNECT_ALIGNMENT_CHECKLIST.md`
- `CONNECT_REMAINING_BACKLOG.md`
- `CONNECT_PARITY_TASKS.md`
- `CONNECT_SOURCE_LEDGER.md`
- `SOURCE_AND_HARDWARE_BLOCKERS.md`
- `LINK_CONTRACT.md`
- `T_ECHO_LITE_HARDWARE_ACCEPTANCE.md`
- `GAT562_HARDWARE_ACCEPTANCE.md`

### Generated Audits

- `analysis\completion_audit\completion_audit.md`
- `analysis\runtime_resource_audit\runtime_resource_audit.md`
- `analysis\visual_alignment\visual_alignment_report.md`
- `analysis\source_blocker_matrix\source_blocker_matrix.md`
- `analysis\source_evidence_probe\source_evidence_probe.md`
- `analysis\hardware_acceptance_matrix\hardware_acceptance_matrix.md`

## Current Anti-Drift Rules

- Do not treat "build passes" as "Connection parity complete".
- Do not treat simulator previews as hardware proof.
- Do not treat generated low-resource drawings as final official-look art.
- Do not treat substitute timing/probability constants as sourced truth.
- Do not add new runtime image-file assets.
- Do not flash unless explicitly asked.
- Do not parallelize PlatformIO builds for `EchoPet`, `EchoPet_LoRa`, and
  `EchoPet_GAT562_LoRa`.
- Keep source evidence, runtime implementation, hardware proof, and final art
  as separate concepts in every checklist update.

## If You Resume From Here

Start with this concrete sequence:

1. Set cwd to `C:\Users\vicliu\Projects\t-echo-lite-minimal-pio`.
2. Run `pio run -e EchoPet`, `pio run -e EchoPet_LoRa`, and
   `pio run -e EchoPet_GAT562_LoRa`.
3. Read:
   - `analysis\completion_audit\completion_audit.md`
   - `analysis\visual_alignment\visual_alignment_report.md`
   - `T_ECHO_LITE_HARDWARE_ACCEPTANCE.md`
   - `GAT562_HARDWARE_ACCEPTANCE.md`
4. If T-Echo-Lite hardware is available and flashing is explicitly allowed,
   install the current T-Echo-Lite build and burn down P1/P2 hardware refresh
   and button rows first.
5. If GAT562 hardware is available and flashing is explicitly allowed, install
   `EchoPet_GAT562_LoRa`. If PlatformIO misses the bootloader window, touch the
   application port, wait for stable bootloader `VID:PID=239A:002A`, then run
   direct `adafruit-nrfutil` against that bootloader COM port.
6. If hardware is not available, work on P3 final C++ bitmap art rows or P4
   source-exact table replacement, but keep all substitute rows labeled.
