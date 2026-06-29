# EchoPet Runtime Resource Contract

This contract protects the firmware resource route for T-Echo-Lite.

## Runtime Rule

All runtime animation and icon resources must be represented as C/C++ source:

- `const uint8_t ...[] PROGMEM` bitmap arrays.
- `FramePart` / `ComposedFrame` records that compose reusable bitmap parts.
- Tile-cell tables and menu icon byte arrays in `.cpp` files.

The firmware must not depend on PNG, JPG, JPEG, GIF, BMP, or other decoded image
files at runtime. It must not load animation assets from LittleFS, SD, host
paths, or packaged raster files.

Offline GIF/image decomposition may only produce human-readable contracts:
motion labels, frame names, part lists, timing notes, and audit screenshots.
The implementation step is always a C/C++ translation into bitmap arrays,
tile tables, `SpriteFrame` rows, or composed-frame tables.

## Current Runtime Resource Files

- `EchoPetSprites.cpp`
- `EchoPetSprites.h`
- `EchoPetResources64.cpp`
- `EchoPetResources64.h`
- `EchoPetResources128.cpp`
- `EchoPetResources128.h`

These files are the firmware resource truth. They are intentionally written as
low-resource C/C++ bitmap and composition data so future pet designs can replace
the required frames without changing the display/runtime architecture.

## Non-Runtime Raster Files

PNG/JPG/GIF/BMP files under `analysis/` are generated audit or reference
artifacts only:

- GIF/contact-sheet analysis output.
- Host-side sprite render proof images.
- `analysis/echopet_reference_resources/`, which is the local copy of the
  human comparison/resource bundle formerly kept under
  `C:\Users\vicliu\Projects\pet\echopet_reference_resources`.

They may be included in reports to help humans inspect frame composition, but
they are not firmware assets and must not be referenced by runtime source.

## Replacement Rule For Future Pets

When a new pet design is added, every animation frame should be defined by a
named `SpriteFrame` row and backed by explicit C/C++ bitmap parts. Shared parts
such as eyes, mouth states, feet, hands, marks, props, and special expressions
should be reusable bitmap assets rather than copied whole-frame images.

The source-side guard for this contract is:

`analysis/runtime_resource_audit/generate_runtime_resource_audit.py`
