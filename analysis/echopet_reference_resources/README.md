# EchoPet reference resource bundle

This folder is the project-local visual/reference archive used to compare
EchoPet against the Tamagotchi Connection target. It lives under `analysis/`
on purpose: acquired official assets, generated proof sheets, host previews,
manifests, reports, concept GIF mirrors, and copied C/C++ resource snapshots
are available beside the firmware project, but they remain non-runtime
analysis material.

Source firmware project:

```text
C:\Users\vicliu\Projects\t-echo-lite-minimal-pio
```

Original external mirror, retained only as historical input:

```text
C:\Users\vicliu\Projects\pet\echopet_reference_resources
```

## Folders

Current file counts at the time this bundle was imported into the firmware
project:

```text
official_raw/            151
official_generated/      160
firmware_previews/        39
concept_design_gifs/      15
hardware_acceptance_matrix/  3
replacement_manifests/     7
resource_drawing_spec/     5
runtime_cxx_resources/    13
reports/                  23
scripts/                  20
source_blocker_matrix/     3
source_evidence_probe/     3
```

```text
official_raw/
```

Official website/manual resources cached for analysis. These include how-to
GIF/PNG assets, character reference images, and manual/reference files. They are
comparison/reference material only.

```text
official_generated/
```

Generated official-reference proof sheets, extracted tile sheets, JSON
manifests, character/contact sheets, catalog item status sheets, souvenir
status sheets, and fixed-menu alignment proofs.
The catalog item official reference manifest now includes 52 deduplicated LCD
tile candidates scanned across all frames of the official item PC GIF. These
tiles are comparison candidates only; the 12 unambiguous official-source
catalog rows remain the only catalog rows promoted to runtime C++ overrides.

```text
firmware_previews/
```

Current EchoPet host-rendered previews from the real firmware drawing path:
compact/large screen previews, catalog proof sheets, character roster proof,
and fixed menu proof.

```text
concept_design_gifs/
```

Mirror of the GIF/PNG concept files originally kept under
`C:\Users\vicliu\Projects\pet`. This local mirror makes the firmware project a
single comparison entry point.

```text
hardware_acceptance_matrix/
```

Machine-readable hardware proof matrix generated from
`T_ECHO_LITE_HARDWARE_ACCEPTANCE.md`: JSON, CSV, and Markdown rows for the 55
remaining physical acceptance cases. These rows separate source readiness from
real T-Echo-Lite evidence and include proof routes for buttons, refresh/game
scenes, sprite proof, and two-device LoRa flows.

```text
replacement_manifests/
```

Machine-readable replacement contracts for character, catalog item, and
souvenir art. The JSON/CSV files list every stable row id, label, runtime C++
bitmap symbol, dimensions, current source status, and which rows still need
accepted final art. Character rows include both 16x16 portrait and 24x24 idle
bitmap symbols.

```text
resource_drawing_spec/
```

Row-local drawing briefs for the remaining catalog and souvenir art work. The
JSON/CSV/Markdown files define every 16x16 catalog item and souvenir row by
stable row id, semantic object/family, runtime C++ bitmap symbol, preservation
rules, and acceptance checks. This is a drawing/replacement contract, not a
claim that the substitute art has reached final visual parity.

```text
runtime_cxx_resources/
```

Runtime C/C++ bitmap/resource source files copied from `src/`. These are the
actual firmware resource truth: sprites, character rows, catalog rows, fixed
menu icons, and 64/128 layout resources.

```text
source_blocker_matrix/
```

Machine-readable matrix for the remaining source-gated and substitute rows.
The JSON/CSV/Markdown files list each blocker by stable ID, backlog row,
domain, blocker type, current substitute, evidence required, candidate source
IDs, and the code/data table that must change when exact evidence is found.

```text
source_evidence_probe/
```

Machine-readable probe that checks sourced Connection facts against current
code tables and constants. This is a code-evidence report, not a final parity
claim; rows may still remain substitute or hardware-gated.

```text
reports/
```

Current visual/resource/completion reports and source ledgers.
Start with `completion_audit.md`, `visual_alignment_report.md`, and
`SOURCE_AND_HARDWARE_BLOCKERS.md` when checking what is closed versus still
blocked by missing final art or hardware proof.
`ECHOPET_MIGRATION_BOUNDARY.md` records the split between active firmware
source, C/C++ runtime resources, firmware-local analysis files, and this local
human comparison archive.

```text
scripts/
```

The scripts that generated the official-reference proofs and C/C++ resource
tables.

```text
RESOURCE_INVENTORY.json
RESOURCE_INVENTORY.csv
```

Generated inventory files for this bundle. Each row records relative path, file
size, last write time, and SHA-256 so later resource changes can be compared
without guessing.

## Boundary

Official PNG/JPG/GIF/PDF resources are reference material. EchoPet firmware must
not load them at runtime. Runtime visual resources remain C/C++ bitmap arrays in
the copied source files under `runtime_cxx_resources/` and in the active
firmware project.

The concept GIFs originally placed in `C:\Users\vicliu\Projects\pet` are
mirrored under `concept_design_gifs/`. The originals were not deleted or moved.

## Current Visual Gate Snapshot

At the time this bundle was copied, source-side visual alignment had closed:

- fixed menu icon source-side alignment,
- 50-row character source-side alignment,
- runtime C/C++ resource boundary.

Still open:

- final 160-row catalog item art,
- final 64-row souvenir/memory art,
- hardware proof for e-paper refresh, ghosting, physical keys, and real-device
  readability.
- the structured hardware matrix currently has 55 TODO acceptance rows, all
  source-ready and still awaiting real photo/video proof.
- the drawing spec now covers all 160 catalog rows and all 64 souvenir rows,
  while 148 catalog rows and 62 souvenir rows still await accepted final art.
- the source blocker matrix now tracks 21 source-gated/substitute rows that
  require exact external evidence or accepted replacement art before final
  parity can be claimed.
