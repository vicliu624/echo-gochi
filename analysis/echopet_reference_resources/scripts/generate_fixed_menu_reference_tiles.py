from __future__ import annotations

import json
from pathlib import Path

from PIL import Image, ImageDraw


ROOT = Path(__file__).resolve().parent
GENERATED = ROOT / "generated"
TILE_DIR = GENERATED / "fixed_menu_reference_tiles"
SOURCE = GENERATED / "official_manual_fixed_menu_device_callouts.png"

# Coordinates are in official_manual_fixed_menu_device_callouts.png space.
# The source is a rendered Bandai instruction-manual crop, not a runtime asset.
SLOTS = [
    {"id": "HEALTH", "official": "Health Meter", "center": (324, 142)},
    {"id": "FOOD", "official": "Food", "center": (359, 142)},
    {"id": "TOILET", "official": "Toilet", "center": (394, 142)},
    {"id": "CONNECT", "official": "Connection/Communication", "center": (462, 142)},
    {"id": "CARE", "official": "Attention", "center": (465, 286)},
    {"id": "TRAIN", "official": "Discipline", "center": (322, 286)},
    {"id": "MEDS", "official": "Medicine", "center": (359, 286)},
    {"id": "LIGHTS", "official": "Lights", "center": (397, 286)},
    {"id": "FRIEND", "official": "Friend List", "center": (431, 286)},
    {"id": "GAME", "official": "Games/Activity", "center": (429, 142)},
]


def threshold_tile(tile: Image.Image) -> Image.Image:
    gray = tile.convert("L")
    out = Image.new("1", tile.size, 1)
    pix = out.load()
    for y in range(tile.height):
        for x in range(tile.width):
            if gray.getpixel((x, y)) < 170:
                pix[x, y] = 0
    return out.convert("RGB")


def crop_tile(source: Image.Image, center: tuple[int, int], size: int) -> Image.Image:
    cx, cy = center
    half = size // 2
    return source.crop((cx - half, cy - half, cx + half, cy + half))


def main() -> None:
    TILE_DIR.mkdir(parents=True, exist_ok=True)
    for stale in TILE_DIR.glob("*_official_*.png"):
        stale.unlink()
    source = Image.open(SOURCE).convert("RGB")
    tile_size = 24
    scale = 5
    contact_w = 10 * 112
    contact_h = 176
    contact = Image.new("RGB", (contact_w, contact_h), "white")
    draw = ImageDraw.Draw(contact)

    manifest = {
        "source": str(SOURCE.relative_to(ROOT)),
        "tile_size": tile_size,
        "runtime_boundary": "These PNG tiles are analysis-only reference crops; firmware still uses C/C++ bitmap arrays.",
        "slots": [],
    }

    for index, slot in enumerate(SLOTS):
        tile = crop_tile(source, slot["center"], tile_size)
        bw = threshold_tile(tile)
        raw_name = f"{index:02d}_{slot['id'].lower()}_official_raw.png"
        bw_name = f"{index:02d}_{slot['id'].lower()}_official_bw.png"
        tile.save(TILE_DIR / raw_name)
        bw.save(TILE_DIR / bw_name)

        x = index * 112
        draw.rectangle((x, 0, x + 111, contact_h - 1), outline="black")
        draw.text((x + 4, 4), slot["id"], fill="black")
        draw.text((x + 4, 18), slot["official"][:16], fill="black")
        enlarged = bw.resize((tile_size * scale, tile_size * scale),
                             Image.Resampling.NEAREST)
        contact.paste(enlarged, (x + (112 - enlarged.width) // 2, 38))
        manifest["slots"].append({
            "index": index,
            "local_id": slot["id"],
            "official_label": slot["official"],
            "center": slot["center"],
            "raw_tile": f"fixed_menu_reference_tiles/{raw_name}",
            "bw_tile": f"fixed_menu_reference_tiles/{bw_name}",
        })

    out = GENERATED / "fixed_menu_official_tiles_contact_sheet.png"
    contact.save(out)
    (GENERATED / "fixed_menu_official_tiles_manifest.json").write_text(
        json.dumps(manifest, indent=2), encoding="utf-8")
    print(f"wrote {out}")


if __name__ == "__main__":
    main()

