# EchoPet host screen simulator

This simulator renders the existing EchoPet firmware UI on a desktop machine.
It does not reimplement the UI in HTML/GIF. Instead, it compiles the real
`drawEchoPet(...)` path and the real resource files against a small host-side
`Adafruit_SSD1681` replacement that records pixels into a framebuffer and writes
PNG files.

## Run

From `C:\Users\vicliu\Projects\t-echo-lite-minimal-pio`:

```powershell
python analysis\screen_simulator\run_simulator.py
```

If Windows routes `python` to the Microsoft Store alias, use an installed
interpreter directly:

```powershell
& "C:\Users\vicliu\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe" analysis\screen_simulator\run_simulator.py
```

Generated previews are written to:

```text
analysis\screen_simulator\out
```

Useful variants:

```powershell
python analysis\screen_simulator\run_simulator.py --profile large
python analysis\screen_simulator\run_simulator.py --profile compact
python analysis\screen_simulator\run_simulator.py --scenario home --scenario health
python analysis\screen_simulator\run_simulator.py --scenario catalog_items
python analysis\screen_simulator\run_simulator.py --scenario catalog_souvenirs
python analysis\screen_simulator\run_simulator.py --scenario character_roster
python analysis\screen_simulator\run_simulator.py --scenario fixed_icons
python analysis\screen_simulator\run_simulator.py --scale 6
python analysis\screen_simulator\run_simulator.py --list
```

## What is covered

- `large`: 192x176 canvas, which selects the 128-style EchoPet resources.
- `compact`: 128x64 canvas, which selects the compact EchoPet resources.
- The preview uses `EchoPetDisplay.cpp`, `EchoPetSprites.cpp`, both resource
  profiles, catalog visual bitmap rows, character visual bitmap rows, UI helpers, catalog helpers,
  character catalog helpers, and model label/catalog bit helpers.
- `catalog_items_proof_x*.png` and `catalog_souvenirs_proof_x*.png` are
  source-resource proof sheets, not actual screen layouts. They exist to make
  item shapes and the 64 per-memory souvenir bitmaps easy to inspect.
- `generate_catalog_entry_source_arrays.py` can snapshot the current
  `catalog_items_proof_x2.png` into the firmware's 160-row
  `kCatalogEntryBitmaps` C/C++ source table. This makes each catalog row a
  directly replaceable runtime bitmap instead of only sharing the older 41 item
  icon families.
- `character_roster_proof_x*.png` renders all 50 character catalog rows through
  the same per-catalog bitmap and trait route used by the main, friends, and
  family screens.
- `fixed_menu_icons_proof_x*.png` renders the ten fixed menu icons from the
  compact 12x12 C/C++ resource arrays and the large 30x30 C/C++ resource arrays
  in `EchoPetMenuIconResources.cpp`.
- `../official_reference/generated/fixed_menu_official_alignment_proof.png`
  places the current fixed-menu proof beside official manual callouts. That
  official-reference proof is the acceptance surface for replacing or approving
  the source icon rows.
- The fake display implements the drawing calls currently used by
  `EchoPetDisplay.cpp`: pixels, lines, rectangles, rounded rectangles, circles,
  triangles, fill calls, cursor/text rendering, and PNG export.

## Boundary

This is a source-side preview. It verifies layout, sprite composition, text, and
mode-specific drawing logic against the current C/C++ UI path. It is not a full
hardware proof for e-paper waveform behavior, refresh timing, ghosting, SPI
transport, or board rotation setup.
