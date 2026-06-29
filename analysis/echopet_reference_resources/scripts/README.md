# EchoPet Official Reference Assets

This folder is the analysis-only home for official Tamagotchi Connection visual
references.

Runtime firmware must not load files from this folder. Firmware graphics remain
C/C++ bitmap arrays and composed frame tables.

## Sources

- `raw/manuals/bandai_connection_2024_instruction_manual.pdf`
  - Source URL: `https://www.bandai.com/amfile/file/download/file/242/product/1297931/`
  - Use: official fixed-menu icon semantics, manual callouts, button mapping,
    and behavior wording.
- `raw/howto/*`
  - Source page: `https://tamagotchi-official.com/gb/series/connection/howto/`
  - Use: official how-to GIF/image reference for behavior animation, games,
    growth, shop, items, connection, and character presentation.
- `raw/character/*`
  - Source page: `https://tamagotchi-official.com/gb/series/connection/character/`
  - Use: analysis-only official character thumbnails and source-page HTML for
    the 50-row runtime character catalog proof.

## Generated Proofs

- `generated/official_howto_gif_contact_sheet.png`
  - Contact sheet of sampled frames from official how-to GIFs.
- `generated/manual_pages/page-*.png`
  - Rendered instruction manual pages for analysis.
- `generated/official_manual_fixed_menu_device_callouts.png`
  - Official manual crop showing the ten fixed menu callouts on the device.
- `generated/official_manual_fixed_menu_center_callouts.png`
  - Official manual crop showing the same fixed-menu semantics and attention
    behavior.
- `generated/fixed_menu_official_tiles_contact_sheet.png`
  - Per-slot reference crops for the ten fixed menu icons, ordered to match
    `EchoPetMenuIconResources.cpp`.
- `generated/fixed_menu_official_tiles_manifest.json`
  - Crop-coordinate manifest for regenerating the per-slot reference tiles.
- `generated/fixed_menu_generated_source_contact_sheet.png`
  - Preview of the 12x12 and 30x30 source bitmaps generated from the official
    per-slot tile crops.
- `generated/fixed_menu_compact_official_source_contact_sheet.png`
  - Preview of the compact 12x12 fixed-menu rows and their analysis-only
    official source route. The compact rows are derived from the same official
    device/menu tile crops as the 30x30 rows so both sizes keep one visual
    concept per slot.
- `generated/fixed_menu_source_policy_contact_sheet.png`
  - Per-slot proof that the compact 12x12 and large 30x30 fixed menu icons
    share the same official device/menu tile source.
- `generated/fixed_menu_source_policy.json`
  - Machine-readable source-policy record for the ten fixed menu slots.
- `generated/fixed_menu_official_alignment_proof.png`
  - Side-by-side proof comparing official fixed-menu references with the current
    EchoPet C/C++ source icons and the unified 12x12/30x30 source policy.
- `generated/catalog_item_official_assets_contact_sheet.png`
  - Official item-section image/GIF samples from the Connection how-to page.
- `generated/catalog_item_official_pc_gif_frames.png`
  - All frames from the official item PC GIF, reduced into a contact sheet for
    shape and animation-style review.
- `generated/catalog_item_official_detected_tiles_contact_sheet.png`
  - Deduplicated LCD item tile candidates detected across all frames of the
    official item PC GIF. Current coverage is 52 candidate tiles. This is a
    style/semantic sample set, not a full 160-row item atlas, and candidates
    are not promoted to catalog rows until their label is unambiguous.
- `generated/catalog_item_reference_tiles/*.png`
  - Raw and black/white crops for the detected official item LCD reference tile
    candidates, plus 16x16 row data in
    `catalog_item_official_reference_manifest.json` for future C++ row
    replacement work.
- `generated/catalog_item_official_alignment_proof.png`
  - Side-by-side proof comparing the official item references with the current
    EchoPet catalog item source proof.
- `generated/catalog_item_official_source_overrides_contact_sheet.png`
  - Preview of the catalog rows whose runtime C/C++ 16x16 item bitmaps are
    generated from clearly mapped official item how-to icons: PENCIL, CAP,
    SHOVEL, BALL, BALLOON, and TRUMPET. Ambiguous official icons such as the PC
    and rabbit are intentionally not mapped to catalog rows.
- `generated/catalog_item_source_semantic_status_contact_sheet.png`
  - The current 152 non-souvenir catalog C/C++ bitmap rows with per-row official
    semantic status: official-source C++ override or section-level official
    food/item/shop scene reference. This is the row-local replacement map for
    future item art.
- `generated/catalog_item_official_reference_manifest.json`
  - Machine-readable record of the official item assets, generated proofs, and
    current coverage boundary, including the 52 all-frame LCD tile candidates,
    12 unambiguous official-source catalog overrides, and the explicit rule
    that ambiguous candidates are not runtime truth.
- `generated/souvenir_official_candidate_assets_contact_sheet.png`
  - Official how-to animation/image candidates grouped by the 64 MEMORY/
    souvenir row semantics. These are scene/icon references, not a complete
    official souvenir atlas.
- `generated/souvenir_source_semantic_status_contact_sheet.png`
  - The current 64 C/C++ souvenir memory bitmap rows with per-row official
    semantic status: official scene reference, official-source C++ override, or
    no direct cached official row.
- `generated/souvenir_official_source_overrides_contact_sheet.png`
  - The unambiguous souvenir rows whose C/C++ bitmaps are generated from
    official item icons. Current coverage: TRUMPET and BALL PLAY.
- `generated/souvenir_official_alignment_proof.png`
  - Side-by-side proof comparing official souvenir/memory semantic candidates,
    current C/C++ source rows, official-source overrides, and the current
    simulator souvenir proof.
- `generated/souvenir_official_reference_manifest.json`
  - Machine-readable record of souvenir semantic candidates, 64 row statuses,
    official-source overrides, and the remaining official atlas gap.
- `generated/character_official_reference_contact_sheet.png`
  - Official character thumbnails for the 50 catalog rows, ordered by the
    runtime character catalog.
- `generated/character_official_alignment_proof.png`
  - Side-by-side proof comparing official character references with the current
    EchoPet row-local official/source status sheet and current 50-row C/C++
    source-rendered character proof.
- `generated/character_official_reference_manifest.json`
  - Machine-readable record of official character page URLs, alt text, local
    analysis-only references, and current runtime-art boundary.
- `generated/character_generated_source_contact_sheet.png`
  - Preview of the 50 C/C++ 16x16 character bitmap rows generated from the
    analysis-only official references.
- `generated/character_idle_generated_source_contact_sheet.png`
  - Preview of the 50 C/C++ 24x24 idle/main-scene character bitmap rows
    generated from the same analysis-only official references. These rows are
    used for larger main-screen character placement while the 16x16 rows remain
    available for compact catalog/friend/family portrait contexts.
- `generated/character_source_alignment_status_contact_sheet.png`
  - Per-character review sheet showing the official reference thumbnail, the
    generated 16x16 C++ portrait row, and the generated 24x24 C++ idle row in
    the same cell. This is the row-local visual acceptance surface for future
    character replacement art.
- `generated/character_source_alignment_status.json`
  - Machine-readable ink/bounding-box metrics and review flags for the 50
    generated character rows.

## Generators

- `generate_fixed_menu_reference_tiles.py`
  - Regenerates the per-slot official reference tile crops from the rendered
    instruction-manual page crop.
- `generate_menu_icon_source_arrays.py`
  - Converts analysis-only fixed-menu references into
    `EchoPetMenuIconResources.cpp`. Both the compact 12x12 rows and large
    30x30 rows use the same official device/menu tile crop for each slot. The
    generated firmware source remains C/C++ byte arrays, not runtime image
    files.
- `generate_menu_icon_alignment.py`
  - Regenerates the side-by-side official-vs-source proof sheet.
- `generate_catalog_item_reference_sheets.py`
  - Regenerates official item reference contact sheets and the item alignment
    proof. It does not generate firmware resources; the proof exists to compare
    current C/C++ source bitmaps against official semantics and animation style.
- `generate_souvenir_reference_sheets.py`
  - Regenerates official MEMORY/souvenir semantic proof sheets, writes the
    64-row status manifest, and maintains the generated C/C++ souvenir bitmap
    table. Only unambiguous object rows are converted into official-source C++
    overrides; other official GIF/PNG files remain analysis-only scene
    references.
- `generate_character_reference_sheets.py`
  - Downloads/caches the 50 official character reference pages/images under
    `raw/character`, then regenerates official-vs-source character proof sheets.
    It does not generate firmware resources; runtime character art remains
    C/C++ source rows or future rights-cleared replacements.
- `generate_character_source_arrays.py`
  - Converts the analysis-only official character references into the firmware's
    50-row `kCharacterCatalogBitmaps` and `kCharacterIdleBitmaps` C/C++ source
    tables and writes generated source contact sheets plus the row-local
    official/16x16/24x24 status sheet. The firmware still does not load image
    files at runtime.

## Boundary

Official raster assets in this directory are references only. They are allowed
to inform hand-authored C/C++ bitmaps, proof sheets, and human acceptance, but
they must not become firmware runtime dependencies or be packaged as runtime
resources.
