# V3 LCD Grid Probe

This folder contains analysis-only proof material for the compact GAT562
Tamagotchi Connection V3 LCD projection.

Runtime firmware does not load these files. The firmware keeps C/C++ bitmap
rows and procedural drawing paths.

## Files

- `generate_v3_lcd_grid_probe.py`
  - Reconstructs candidate native grids from the official cached
    `img_breed_03.gif` cleanup animation.
  - Compares black/white reconstruction error for `32x30`, `64x32`, `64x30`,
    `48x32`, and `48x30`.
- `breed03_grid_metrics.json`
  - Machine-readable probe output. Current result favors `32x30` for visible
    Connection/V3 gameplay frames.
- `breed03_grid_candidates_contact.png`
  - Human-review contact sheet comparing `32x30` and `64x32` reconstructions
    across selected cleanup frames.
