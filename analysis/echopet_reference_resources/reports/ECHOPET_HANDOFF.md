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
- Target board: T-Echo-Lite nRF52840.
- Target communication route: keep the same visible IR/Connection behavior,
  but carry it over LoRa.
- Do not flash unless the user explicitly asks for flashing.
- Runtime visual resources must be C/C++ source bitmap arrays and composed
  frame tables. Do not add runtime PNG/JPG/GIF/BMP assets or runtime file-load
  dependencies for animation, character, item, icon, or menu art.
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
| `CURL-2024-EVENTS` | B/C | [Curlour Connection 2024 events](https://curlour.neocities.org/en/tmgc/cone2/eventos) | Seasonal/date events, birthday, New Year, winter, Easter, back-to-school, Halloween, Thanksgiving, Christmas, Santaclautchi timing, seasonal shop-food dates. |
| `CURL-ENTAMA-CARE` | C | [Curlour Entama requirements](https://curlour.neocities.org/en/tmgc/entama/requirements) | Cross-version Hungry/Happy/poop timing table. This is only a low-confidence substitute while exact Connection 2024 timing is unavailable. |
| `TAMATALK-BUMP-V3` | B/C | [TamaTalk V3 Bump/Heading guide](https://www.tamatalk.com/threads/guide-to-ver-3-bump-heading.56907/), [Bump score chart image](https://imgur.com/a/eim3rkW) | V3 Bump/Heading guide and Bump weight-band score chart; useful substitute until 2024-specific Bump/Sprint probability proof exists. |
| `TAMATALK-MEMORY-V3` | C | [TamaTalk V3 games guide](https://www.tamatalk.com/threads/guide-for-v3-games.53259/) | Historical V3 Memory/Sprint play notes. Kept as cross-check only; 2024-specific game notes override it where available. |
| `TAMATOWN-V3-GROWTH` | C | [TamaTown V3 growth chart](https://tamatown.com/guides/tamagotchi-connection-v3-growth-chart/) | V3 growth chart provenance. Image-centric and not directly parseable yet. |

### Local Reference And Hardware Evidence

| Source ID | Level | Location | Used for |
| --- | --- | --- | --- |
| `LOCAL-GIF` | A for user intent, not official behavior | `analysis\echopet_reference_resources\concept_design_gifs` copied from `C:\Users\vicliu\Projects\pet` | User-provided concept GIFs and animation phase expectations. Use them for scene contracts and motion intent, not official behavior truth. |
| `HW-TECHO` | A once observed | `T_ECHO_LITE_HARDWARE_ACCEPTANCE.md` | T-Echo-Lite refresh, controls, sprite proof, and two-device LoRa acceptance. Rows are not final until real hardware evidence is recorded. |
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
```

Latest verified result:

| Environment | Status | RAM | Flash |
| --- | --- | ---: | ---: |
| `EchoPet` | PASS | 15556 / 248832, 6.3% | 209452 / 815104, 25.7% |
| `EchoPet_LoRa` | PASS | 16520 / 248832, 6.6% | 242592 / 815104, 29.8% |

Generated firmware files:

- `.pio\build\EchoPet\firmware.elf`
- `.pio\build\EchoPet\firmware.hex`
- `.pio\build\EchoPet\firmware.zip`
- `.pio\build\EchoPet_LoRa\firmware.elf`
- `.pio\build\EchoPet_LoRa\firmware.hex`
- `.pio\build\EchoPet_LoRa\firmware.zip`

## Current Audit Snapshot

Primary reports:

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

Important next technical check:

- Compare the current EPD/T-Echo-Lite refresh path against
  `C:\Users\vicliu\Projects\trail-mate`.
- Do this before adding more animation complexity if whitening/blur still
  appears on hardware.
- Keep refresh policy simple enough for the panel:
  - avoid invisible periodic full refreshes that users notice;
  - avoid overusing partial refresh if it whitens the screen;
  - record every accepted strategy in `T_ECHO_LITE_HARDWARE_ACCEPTANCE.md`.

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
- Toilet sweep/removal visibility.
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

- Audit current display driver/update path.
- Compare with `C:\Users\vicliu\Projects\trail-mate`.
- Identify whether current full/partial refresh scheduling causes whitening.
- Simplify the policy if needed.
- Add a short note to `T_ECHO_LITE_HARDWARE_ACCEPTANCE.md` describing the
  accepted refresh strategy.
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
- Do not parallelize PlatformIO builds for `EchoPet` and `EchoPet_LoRa`.
- Keep source evidence, runtime implementation, hardware proof, and final art
  as separate concepts in every checklist update.

## If You Resume From Here

Start with this concrete sequence:

1. Set cwd to `C:\Users\vicliu\Projects\t-echo-lite-minimal-pio`.
2. Run `pio run -e EchoPet` and `pio run -e EchoPet_LoRa`.
3. Read:
   - `analysis\completion_audit\completion_audit.md`
   - `analysis\visual_alignment\visual_alignment_report.md`
   - `T_ECHO_LITE_HARDWARE_ACCEPTANCE.md`
4. If hardware is available and flashing is allowed, burn down P1/P2 hardware
   refresh and button rows first.
5. If hardware is not available, work on P3 final C++ bitmap art rows or P4
   source-exact table replacement, but keep all substitute rows labeled.
