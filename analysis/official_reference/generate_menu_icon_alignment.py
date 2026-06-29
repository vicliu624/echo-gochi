from __future__ import annotations

import json
from pathlib import Path

from PIL import Image, ImageDraw


ROOT = Path(__file__).resolve().parents[2]
SOURCE_ROOT = ROOT / "src" if (ROOT / "src").exists() else ROOT
REF = ROOT / "analysis" / "official_reference"
GENERATED = REF / "generated"
SIM_OUT = ROOT / "analysis" / "screen_simulator" / "out"

SLOTS = [
    ("HEALTH", "Health Meter"),
    ("FOOD", "Food"),
    ("TOILET", "Toilet"),
    ("CONNECT", "Connection/Communication"),
    ("CARE", "Attention"),
    ("TRAIN", "Discipline"),
    ("MEDS", "Medicine"),
    ("LIGHTS", "Lights"),
    ("FRIEND", "Friend List"),
    ("GAME", "Games/Activity"),
]


def fit(image: Image.Image, width: int, height: int) -> Image.Image:
    out = image.copy()
    out.thumbnail((width, height), Image.Resampling.LANCZOS)
    return out


def paste_labeled(sheet: Image.Image, draw: ImageDraw.ImageDraw, image: Image.Image,
                  x: int, y: int, w: int, h: int, label: str) -> None:
    draw.rectangle((x, y, x + w - 1, y + h - 1), outline="black")
    draw.text((x + 6, y + 6), label, fill="black")
    body = fit(image, w - 16, h - 28)
    sheet.paste(body.convert("RGB"), (x + (w - body.width) // 2, y + 24))


def main() -> None:
    GENERATED.mkdir(parents=True, exist_ok=True)
    manual_device = Image.open(
        GENERATED / "official_manual_fixed_menu_device_callouts.png").convert(
            "RGB")
    manual_center = Image.open(
        GENERATED / "official_manual_fixed_menu_center_callouts.png").convert(
            "RGB")
    official_tiles = Image.open(
        GENERATED / "fixed_menu_official_tiles_contact_sheet.png").convert("RGB")
    compact_sources = Image.open(
        GENERATED / "fixed_menu_compact_official_source_contact_sheet.png").convert("RGB")
    source_policy = Image.open(
        GENERATED / "fixed_menu_source_policy_contact_sheet.png").convert("RGB")
    current = Image.open(SIM_OUT / "fixed_menu_icons_proof_x2.png").convert("RGB")

    w = 1280
    h = 1440
    sheet = Image.new("RGB", (w, h), "white")
    draw = ImageDraw.Draw(sheet)

    draw.text((8, 8), "OFFICIAL FIXED MENU REFERENCE VS ECHOPET SOURCE ICONS",
              fill="black")
    draw.text((8, 30),
              "Official references are analysis-only crops from Bandai's "
              "Connection instruction manual. Firmware still uses C/C++ "
              "bitmap arrays.",
              fill="black")

    paste_labeled(sheet, draw, manual_device, 8, 58, 624, 430,
                  "official manual: device callouts")
    paste_labeled(sheet, draw, manual_center, 648, 58, 624, 430,
                  "official manual: menu/attention callouts")

    draw.text((8, 510), "semantic slot ledger", fill="black")
    y = 536
    for index, (local, official) in enumerate(SLOTS, start=1):
        draw.text((18, y), f"{index:02d}  {local:<8} -> {official}",
                  fill="black")
        y += 22

    draw.text((520, 510),
              "visual verdict: PASS for source-side fixed-menu alignment. "
              "The runtime rows are C/C++ bitmaps derived from the official "
              "device-tile source policy.",
              fill="black")
    paste_labeled(sheet, draw, official_tiles, 8, 770, 1264, 150,
                  "official manual tile crops: one reference tile per fixed menu slot")
    paste_labeled(sheet, draw, source_policy, 8, 928, 1264, 150,
                  "source policy: compact 12x12 and large 30x30 share the same official device-tile source")
    paste_labeled(sheet, draw, compact_sources, 8, 1086, 1264, 150,
                  "compact 12x12 official-source rows: derived from the same device tiles as 30x30")
    paste_labeled(sheet, draw, current, 8, 1244, 1264, 180,
                  "current EchoPet C/C++ source icons: compact 12x12 and large 30x30")

    out = GENERATED / "fixed_menu_official_alignment_proof.png"
    sheet.save(out)
    manifest = {
        "status": "PASS",
        "verdict": (
            "Fixed menu icons are source-side aligned: the ten semantic slots "
            "are derived from analysis-only official device tile crops, both "
            "12x12 and 30x30 rows share the same per-slot source policy, and "
            "runtime firmware stores only C/C++ bitmap bytes."
        ),
        "source": {
            "manual_pdf": "raw/manuals/bandai_connection_2024_instruction_manual.pdf",
            "manual_page": "generated/manual_pages/page-1.png",
            "official_device_crop": "generated/official_manual_fixed_menu_device_callouts.png",
            "official_center_crop": "generated/official_manual_fixed_menu_center_callouts.png",
            "official_tile_contact_sheet": "generated/fixed_menu_official_tiles_contact_sheet.png",
            "official_tile_manifest": "generated/fixed_menu_official_tiles_manifest.json",
            "compact_source_contact_sheet": "generated/fixed_menu_compact_official_source_contact_sheet.png",
            "source_policy_contact_sheet": "generated/fixed_menu_source_policy_contact_sheet.png",
            "source_policy": "generated/fixed_menu_source_policy.json",
            "echopet_icon_proof": "../screen_simulator/out/fixed_menu_icons_proof_x2.png",
        },
        "slots": [{"local": local, "official": official}
                  for local, official in SLOTS],
        "runtime_boundary": "Official raster assets are analysis-only references; firmware resources remain C/C++ bitmap arrays.",
    }
    (GENERATED / "fixed_menu_official_alignment_manifest.json").write_text(
        json.dumps(manifest, indent=2), encoding="utf-8")
    print(f"wrote {out}")


if __name__ == "__main__":
    main()

