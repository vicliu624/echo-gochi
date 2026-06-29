# Tamagotchi Connection V3 LCD Grid Spec

Last updated: 2026-06-29

This file is the compact-display scale contract for EchoPet's
Tamagotchi-Connection-style renderer. It exists to prevent the GAT562 128x64
OLED path from drifting into arbitrary sprite scaling, text substitutions, or
thumbnail art being treated as gameplay art.

## Source Baseline

- Public Connection/V3 references list the visible LCD resolution as `32x30`.
- The official Connection how-to GIF `img_breed_03.gif` is cached under
  `analysis/official_reference/raw/howto/` and is treated as the cleanup-motion
  grid probe input.
- `analysis/v3_lcd_grid/generate_v3_lcd_grid_probe.py` reconstructs candidate
  native grids from that GIF and compares black/white reconstruction error.
- Current probe result for `img_breed_03.gif`:
  - `32x30`: average mismatch `0.019253`, max `0.035540`.
  - `64x32`: average mismatch `0.048370`, max `0.078592`.

The product baseline is therefore:

```text
Connection/V3 visible logical LCD: 32 x 30
GAT562 compact projection:        64 x 60
Projection scale:                 2x nearest-neighbor
```

`64x32` controller-level or other-model information is not allowed to redefine
the GAT562 playfield unless a future source proves the visible gameplay frame is
actually 64x32.

## GAT562 Projection Rules

- Compact scene art must be drawn as native logical pixels, then projected with
  integer 2x nearest-neighbor scaling.
- The projected V3 canvas is `64x60` and is centered in the GAT562 main region.
- The left function area remains available for navigation, but compact mode must
  not draw left-area left/right frame lines.
- Compact mode must not draw a `96x64` main frame around the right scene; that
  frame creates a false boundary and makes the left menu feel boxed-in.
- Text hints, action labels, and completion labels must stay out of the compact
  center playfield unless the original V3 frame itself contains such pixels.

## Art Boundary

The following are different objects and must not be mixed:

- `native gameplay frame`: a `32x30` logical LCD frame or a sprite placed within
  that frame.
- `runtime projection`: the `64x60` 2x rendering of that native frame on the
  GAT562 OLED.
- `catalog portrait`: dense list/family/friend artwork.
- `generated 24x24 idle thumbnail`: a larger review/proof representation, not
  the compact V3 gameplay truth.

For GAT562 compact gameplay, `24x24` idle thumbnails must not be promoted to
main-scene truth. If a character source is only available as a `16x16` native
row, compact gameplay projects that row at 2x inside the V3 canvas.

## Toilet Cleanup Contract

The compact cleanup path is a native-frame animation:

- Draw the official-how-to-derived `32x30` pet-plus-poop start frame.
- Project it at 2x into the `64x60` V3 canvas.
- Move a vertical checker clear-wall across the same V3 canvas.
- Clip the native frame as the wall reaches it; do not spawn a separate large
  pet, do not slide the poop into the pet, and do not let the model's already
  cleared `messCount` make the poop disappear before the wall reaches it.
- The compact cleanup sequence runs for 44 logical frames. At 100 ms per frame
  on SSD1306/GAT562, the wall is visible for roughly 4.4 seconds.

Generated proof images:

- `analysis/v3_lcd_grid/breed03_grid_candidates_contact.png`
- `analysis/screen_simulator/out/compact_toilet_v3_2x_phases_x4.png`
