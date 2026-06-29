# EchoPet Source And Hardware Blockers

This file keeps the remaining non-code blockers separate from ordinary
implementation work. It exists to prevent the checklist from silently treating
substitute data, guessed art, or unobserved hardware behavior as Connection
parity.

## Current Specification Boundary

- Firmware runtime resources are C/C++ bitmap arrays and tables under `src/`.
  PNG/JPG/GIF files are analysis or reference material only.
- `analysis/official_reference/` and `analysis/screen_simulator/` are proof and
  comparison projections, not runtime resource paths.
- `analysis/echopet_reference_resources` is the project-local human comparison
  workspace. It mirrors official-cache material, generated proof sheets,
  firmware previews, concept GIFs, runtime C++ resource files, scripts, and
  reports for visual review while remaining outside the runtime firmware path.

## Source-Side Closed Visual Rows

| Row | Evidence | Remaining blocker |
| --- | --- | --- |
| 10 fixed menu icons | `analysis/official_reference/generated/fixed_menu_official_alignment_manifest.json` reports `PASS`; 12x12 and 30x30 C++ rows share the same official device/menu tile source per slot. | T-Echo-Lite hardware flow/readability proof. |
| 50 character rows | `analysis/official_reference/generated/character_source_alignment_status.json` reports 50 `official-derived` rows and zero review flags; generated 16x16 portrait and 24x24 idle rows are in C++ source. `analysis/resource_replacement_manifest/character_replacement_manifest.json` exposes the stable replacement row IDs and bitmap symbols. | T-Echo-Lite readability proof and any future user-drawn replacement pack review. |

## Source Or Rights-Cleared Art Blockers

| Blocker | Why it cannot be closed safely by code alone | Current substitute/evidence |
| --- | --- | --- |
| Catalog item final per-row art | The official how-to surface confirms a 150+ item scope and exposes representative item imagery, but the cached public page is not a complete rights-cleared 150+ row atlas. Unmapped rows must be drawn or supplied with explicit acceptance. | 160 catalog rows exist, all with C++ preview rows and semantics; 12 unambiguous non-souvenir rows currently have official-source overrides. |
| Souvenir official-look final art | Public reference material gives only partial candidate scenes/icons, not a complete 64-row souvenir atlas. | 64 memory/souvenir rows exist with C++ bitmap rows and semantic status; TRUMPET and BALL PLAY are official-source overrides. |
| Exact item animation timing and reward branches | Current sources identify many behaviors but not every probability, reward amount, and frame timing. | Explicit substitute `CatalogRandomRule` rows exist and remain marked substitute. |

The row-local replacement surface is generated under
`analysis/resource_replacement_manifest/`. It proves that the current C++
truth has 160 catalog bitmap rows and 64 souvenir bitmap rows, then exposes
the remaining accepted-art workload as 148 catalog rows and 62 souvenir rows.
Future replacement art must preserve those row IDs and the 16x16 C++ bitmap
encoding instead of adding runtime PNG/JPG/GIF loading.

The row-local drawing brief is generated under
`analysis/resource_drawing_spec/`. It closes the "what exactly should be drawn"
specification surface for all 160 catalog rows and 64 souvenir rows by naming
the semantic object/family, runtime C++ symbol, encoding contract, and
recognizability acceptance rule. It does not close the blocker itself: 148
catalog rows and 62 souvenir rows still need accepted final art or a
rights-cleared replacement pack.

## Exact Behavior Table Blockers

The executable source-evidence list is generated at
`analysis/source_blocker_matrix/`. It currently tracks 21 source or acceptance
rows across care timing, sickness/medicine, death/pass-away, shop/catalog,
game rewards, growth/family probability, visual art, and hardware-only proof.
Each row records the blocker type, current substitute, exact evidence required,
candidate source IDs, and the code/data table that must change once evidence
is found. Current blocker types are `exact_source`, `final_art`,
`hardware_only`, `mixed_art_hardware`, `mixed_source_art`,
`mixed_source_hardware`, and `multi_source`.

| Blocker | Current implementation state |
| --- | --- |
| Death/pass-away thresholds | Neglect/sickness direction and fourth-sickness-in-stage rule are modeled; exact Grim Gotchi threshold remains unsourced. |
| Poop and stat decay windows | Stage-shaped explicit tables exist; some timing rows are cross-version/substitute. |
| Shop sale frequency and seasonal collision priority | Sale pricing, restock times, vendor windows, birthday/holiday substitutions exist; exact frequency/collision priority remains unsourced. |
| Game low-score reward curves and probability details | Top prizes, game lengths, and main mechanics are modeled; exact low-score scaling and some Bump/Sprint curves remain unsourced. |
| Love/partner/baby exact two-device timing | Carrier-independent packet behavior is implemented; exact real-device timing and screens need two-device proof. |

## Hardware Blockers

| Blocker | Required proof |
| --- | --- |
| E-paper refresh/whitening/ghosting | Long-running T-Echo-Lite observation across idle, care scenes, menus, games, and item pages. |
| Physical button feel | Esc/Home/Email short press, long press, release-after-hold, and high-input screens. |
| Seven games at e-paper speed | Hardware video/photo proof that Get, Bump, Flag, Heading, Memory, Sprint, and Hoops are readable and responsive. |
| Two-device LoRa equivalence | Visit, Present, Game, and Love flows must be observed on two T-Echo-Lite LoRa builds using the same visible contract as the IR spec. |
| Sprite proof on both layouts | 128x64 and 176x192 hardware captures must verify anchors, clipping, and selected frame families. |

The executable hardware proof list is generated at
`analysis/hardware_acceptance_matrix/`. It contains 55 stable acceptance IDs:
7 physical-button rows, 18 refresh/game/scene rows, 21 sprite-proof frame
families, and 9 two-device LoRa rows. All 55 are currently source-ready but
still require real photo/video evidence before they can become hardware PASS.
