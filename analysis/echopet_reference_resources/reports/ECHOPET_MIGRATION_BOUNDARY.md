# EchoPet migration boundary

EchoPet was migrated out of:

```text
C:\Users\vicliu\Projects\T-Echo-Lite\examples\T-Echo-Lite\EchoPet
```

into:

```text
C:\Users\vicliu\Projects\t-echo-lite-minimal-pio
```

## Source of truth

- Firmware runtime source: `src/EchoPet*.ino`, `src/EchoPet*.cpp`, and
  `src/EchoPet*.h`.
- Runtime visual resources: C/C++ bitmap arrays and composed-frame tables under
  `src/`.
- T-Echo-Lite board shell: `boards/`, `variants/`, `include/`, and
  `platformio.ini`.
- Local display dependencies: `lib/Adafruit_EPD-4.5.5/` and
  `lib/Adafruit_BusIO-1.16.1/`.
- Firmware-local analysis/proof scripts and generated build validation:
  `analysis/`.
- Human comparison/reference archive:
  `analysis/echopet_reference_resources`.
- Historical minimal probe: `docs/minimal_probe/main.cpp`.

## Rules for future work

- New EchoPet development happens in this project, not in the old examples
  directory.
- Runtime animation and art assets must remain C/C++ source arrays; image files
  are analysis/reference material only.
- Official visual references, generated proof sheets, local concept GIFs,
  manifests, reports, scripts, and copied C/C++ snapshots are centralized under
  `analysis/echopet_reference_resources` for manual comparison. They remain
  analysis/reference material only, not runtime firmware resources.
- Generated proof PNGs stay under `analysis/`.
- Hardware-only claims stay open until proven on T-Echo-Lite hardware.

## Migration validation

The migrated project has been validated with:

```powershell
pio run -e EchoPet_LoRa
pio run -e EchoPet
python analysis/screen_simulator/run_simulator.py
python analysis/visual_alignment/generate_visual_alignment_gate.py
python analysis/runtime_resource_audit/generate_runtime_resource_audit.py
python analysis/completion_audit/generate_completion_audit.py
```
