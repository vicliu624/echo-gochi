Last updated: 2026-06-29

# EchoPet Connection Animation Spec

This file records animation contracts that are now explicit product
specification. It is intentionally small: these rows constrain firmware motion
language and prevent future code from replacing a sourced animation with a
generic effect.

## Source Levels

- `OFF-HOWTO`: official Bandai Tamagotchi Connection how-to page and referenced
  raster/GIF assets.
- `LOCAL-ACCEPTED`: user-confirmed visual contract captured in this repository
  under `analysis/`.
- `CURL-CONNECTION-IDLE`: Curlour's Connection / Plus idle-animation analysis,
  used for motion categories and tier differences, not as runtime art.
- `V3-LCD-GRID`: `V3_LCD_GRID_SPEC.md`; Connection/V3 visible logical LCD is
  treated as 32x30, projected to GAT562 as 64x60 with 2x nearest-neighbor
  scaling.

## Toilet Cleanup

Status: SPEC LOCKED, CODED-SUBSTITUTE, GAT562 BUILD PASS / FLASH BLOCKED.

Canonical references:

- `OFF-HOWTO`: `img_breed_03.gif`, recorded by
  `analysis/official_howto_audit/summary.csv` as
  `toilet.mess/sweep/done`, 213x200, 44 frames, 2640 ms.
- `LOCAL-ACCEPTED`: `analysis/gif_audit/contact_Clean_up_the_poop.png`,
  user-confirmed on 2026-06-29 as the correct cleanup animation contract.
- `V3-LCD-GRID`: `analysis/v3_lcd_grid/breed03_grid_metrics.json` and
  `analysis/v3_lcd_grid/breed03_grid_candidates_contact.png`; current probe
  records `32x30` as the visible-grid baseline for the official cleanup GIF.

Required behavior:

- The scene starts with the pet on the left/middle and the poop on the right.
- A vertical black/white checker clear-wall appears at the right side.
- The wall travels right-to-left across the playfield.
- The pet and poop stay at their scene positions until the wall reaches them;
  they are clipped/erased by the wall, not pushed toward each other or moved
  independently.
- The pet and poop are removed by the wall; neither object remains as a
  separate final success icon.
- The wall continues off the left side, leaving the center playfield empty.
- Compact 128x64 mode must not add center text such as `CLEAN`, `OK`, button
  hints, or completion labels.
- GAT562 compact mode renders cleanup in a 32x30 native V3 logical frame, then
  projects that frame at 2x into a 64x60 OLED canvas. It must not assemble the
  cleanup by drawing a separate oversized pet sprite over the main scene.

Forbidden substitutions:

- Do not use a broom, brush, hand, sparkle-only cleanup, or generic water wave.
- Do not clean only the poop while leaving the pet standing still.
- Do not translate the poop toward the pet or make it look swallowed/eaten.
- Do not end with a happy/heart completion frame as the main cleanup contract.
- Do not make the animation a one-frame state change.
- Do not use the generated 24x24 idle thumbnail or normal home-scene character
  renderer as the cleanup pet.

Firmware projection:

- `drawToiletScene()` draws a clipped 44-frame logical cleanup sequence.
- `Action::kToilet` enters the cleanup-wall sequence only when `messCount > 0`;
  low hygiene alone is not allowed to masquerade as poop cleanup.
- The UI captures the pre-cleanup `messCount` for the scene before the model
  clears it, so poop remains visible until the clear-wall removes it.
- The cleanup wall is the only moving cleaner; pet and poop scene coordinates
  remain fixed while the wall clips pixels from right to left.
- On SSD1306/GAT562, `kToiletSceneAnimationMs` is 100 ms, producing a roughly
  4.4-second cleanup instead of a flash.
- On e-paper targets, scene pacing remains slower to avoid over-refreshing.
- `Notice::kClean` has an extended visible window so the cleanup sequence is not
  cleared before the wall leaves.
- When `Notice::kClean` clears, the UI exits the Toilet scene and returns to
  the home playfield instead of falling through to the `NO MESS` page.
- In compact mode, `drawToiletScene()` uses the official-how-to-derived 32x30
  start frame rows and clips that native frame with the wall. The older
  hand-assembled 22x16 pet / 10x10 poop helper path is retained only for larger
  non-compact projection paths.

Proof outputs:

- `analysis/screen_simulator/out/compact_toilet_cleanup_wall_contact_x4.png`
  shows the current 128x64 logical phases.
- `analysis/screen_simulator/out/compact_toilet_v3_2x_phases_x4.png` shows the
  current GAT562 32x30-to-64x60 2x phases.
- `GAT562_HARDWARE_ACCEPTANCE.md` records the 2026-06-29 17:36 +08:00 flash.

## Idle Motion

Status: SPEC PARTIAL, CODED-SUBSTITUTE, GAT562 FLASHED.

Canonical reference:

- `CURL-CONNECTION-IDLE`: Connection / Plus idle animations are categorized by
  life stage and adult tier. The important constraints are:
  baby/toddler motion does not rely on horizontal flips; teen animation uses
  side-facing/flipped motion; adult animations are tiered; Serious jumps once
  near the middle, Normal jumps around more like younger characters, Naughty
  has larger jumps, and Frail/Senior does not jump upward but falls/weakly
  moves instead.

Required behavior:

- Home idle must not be a tiny in-place bob.
- The pet must visibly travel across the center playfield in a regular loop.
- The loop must include airborne and grounded phases.
- Adult tier changes must affect motion:
  Serious is more centered and restrained, Normal moves broadly, Naughty jumps
  larger, Frail/Senior sags/falls instead of hopping upward.

Firmware projection:

- `drawCharacter()` now uses a 16-phase Connection-style idle motion table.
- GAT562/OLED advances idle at 480 ms per logical phase.
- E-paper targets retain slower idle pacing to protect readability and refresh
  behavior.
- Current firmware does not yet implement true horizontal flip of catalog idle
  bitmaps; this remains a future parity task for teen/adult direction fidelity.

Proof outputs:

- `analysis/screen_simulator/out/compact_home_idle_connection_contact_x4.png`
  shows the current 128x64 logical idle phases.
