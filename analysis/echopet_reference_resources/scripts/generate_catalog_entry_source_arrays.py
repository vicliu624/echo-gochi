from __future__ import annotations

import re
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


ROOT = Path(__file__).resolve().parents[2]
SOURCE_ROOT = ROOT / "src" if (ROOT / "src").exists() else ROOT
PROOF = ROOT / "analysis" / "screen_simulator" / "out" / "catalog_items_proof_x2.png"
OUT = ROOT / "analysis" / "screen_simulator" / "out"
SOURCE = SOURCE_ROOT / "EchoPetCatalogVisuals.cpp"
OFFICIAL_ITEM_ICON_PC = (
    ROOT / "analysis" / "official_reference" / "raw" / "howto" /
    "images__howto__item__img_icon_02_pc.png"
)
OFFICIAL_ITEM_ICON_SHOVEL = (
    ROOT / "analysis" / "official_reference" / "raw" / "howto" /
    "images__howto__item__img_icon_01.png"
)
OFFICIAL_OVERRIDE_PREVIEW = (
    ROOT / "analysis" / "official_reference" / "generated" /
    "catalog_item_official_source_overrides_contact_sheet.png"
)

OFFICIAL_ICON_OVERRIDES = [
    {
        "label": "PENCIL",
        "source_file": OFFICIAL_ITEM_ICON_PC,
        "source_box": (481, 25, 562, 110),
        "catalog_indices": [48, 112],
    },
    {
        "label": "CAP",
        "source_file": OFFICIAL_ITEM_ICON_PC,
        "source_box": (321, 21, 430, 107),
        "catalog_indices": [58, 122],
    },
    {
        "label": "SHOVEL",
        "source_file": OFFICIAL_ITEM_ICON_SHOVEL,
        "source_box": (0, 0, 110, 127),
        "catalog_indices": [54, 118],
    },
    {
        "label": "BALL",
        "source_file": OFFICIAL_ITEM_ICON_PC,
        "source_box": (615, 27, 695, 105),
        "catalog_indices": [60],
    },
    {
        "label": "BALLOON",
        "source_file": OFFICIAL_ITEM_ICON_PC,
        "source_box": (922, 4, 1000, 127),
        "catalog_indices": [56, 63, 120],
    },
    {
        "label": "TRUMPET",
        "source_file": OFFICIAL_ITEM_ICON_PC,
        "source_box": (748, 3, 856, 114),
        "catalog_indices": [84, 141],
    },
]


def font(size: int = 12) -> ImageFont.ImageFont:
    try:
        return ImageFont.truetype("DejaVuSansMono.ttf", size)
    except OSError:
        return ImageFont.load_default()


def is_black(pixel: tuple[int, ...]) -> bool:
    r, g, b = pixel[:3]
    return r < 128 and g < 128 and b < 128


def extract_icon_rows(image: Image.Image, index: int) -> list[int]:
    scale = 2
    cell_w = 80 * scale
    cell_h = 34 * scale
    cols = 8
    col = index % cols
    row = index // cols
    x0 = col * cell_w + 4 * scale
    y0 = 20 * scale + row * cell_h + 14 * scale
    crop = image.crop((x0, y0, x0 + 16 * scale, y0 + 16 * scale)).convert("RGB")
    rows: list[int] = []
    for y in range(16):
        value = 0
        for x in range(16):
            found = False
            for yy in range(y * scale, y * scale + scale):
                for xx in range(x * scale, x * scale + scale):
                    if is_black(crop.getpixel((xx, yy))):
                        found = True
            if found:
                value |= 0x8000 >> x
        rows.append(value)
    return rows


def rows_to_image(rows: list[int]) -> Image.Image:
    image = Image.new("RGB", (16, 16), "white")
    for y, row in enumerate(rows):
        for x in range(16):
            if (row & (0x8000 >> x)) != 0:
                image.putpixel((x, y), (0, 0, 0))
    return image


def official_icon_rows(source: Image.Image, box: tuple[int, int, int, int]) -> list[int]:
    crop = source.crop(box).convert("RGBA")
    xs: list[int] = []
    ys: list[int] = []
    for y in range(crop.height):
        for x in range(crop.width):
            r, g, b, a = crop.getpixel((x, y))
            if a > 16 and not (r < 5 and g < 5 and b < 5):
                xs.append(x)
                ys.append(y)
    if not xs:
        return [0 for _ in range(16)]

    crop = crop.crop((min(xs), min(ys), max(xs) + 1, max(ys) + 1))
    scale = min(14 / crop.width, 14 / crop.height)
    draw_w = max(1, round(crop.width * scale))
    draw_h = max(1, round(crop.height * scale))
    resized = crop.resize((draw_w, draw_h), Image.Resampling.LANCZOS)
    rows = [0 for _ in range(16)]
    ox = (16 - draw_w) // 2
    oy = (16 - draw_h) // 2

    for y in range(draw_h):
        for x in range(draw_w):
            r, g, b, a = resized.getpixel((x, y))
            if a <= 32:
                continue
            luminance = (r * 299 + g * 587 + b * 114) // 1000
            if luminance < 150:
                rows[oy + y] |= 0x8000 >> (ox + x)
    return rows


def apply_official_icon_overrides(all_rows: list[list[int]]) -> list[dict]:
    records: list[dict] = []
    for override in OFFICIAL_ICON_OVERRIDES:
        source_file = override["source_file"]
        if not source_file.exists():
            continue
        source = Image.open(source_file).convert("RGBA")
        rows = official_icon_rows(source, override["source_box"])
        for catalog_index in override["catalog_indices"]:
            all_rows[catalog_index] = rows
            records.append({
                "label": override["label"],
                "catalog_index": catalog_index,
                "source_file": source_file.name,
                "source_box": override["source_box"],
            })
    return records


def save_official_override_preview(records: list[dict]) -> None:
    if not records:
        return

    OFFICIAL_OVERRIDE_PREVIEW.parent.mkdir(parents=True, exist_ok=True)
    cell_w = 84
    cell_h = 62
    sheet = Image.new("RGB", (cell_w * len(records), cell_h), "white")
    draw = ImageDraw.Draw(sheet)
    for i, record in enumerate(records):
        x = i * cell_w
        source_file = (
            ROOT / "analysis" / "official_reference" / "raw" / "howto" /
            record["source_file"]
        )
        rows = official_icon_rows(
            Image.open(source_file).convert("RGBA"),
            tuple(record["source_box"]),
        )
        draw.rectangle((x, 0, x + cell_w - 1, cell_h - 1), outline="black")
        draw.text((x + 3, 3), f"{record['catalog_index']:03d}", fill="black",
                  font=font(9))
        draw.text((x + 3, 14), record["label"][:10], fill="black", font=font(9))
        icon = rows_to_image(rows).resize((32, 32), Image.Resampling.NEAREST)
        sheet.paste(icon, (x + (cell_w - 32) // 2, 28))
    sheet.save(OFFICIAL_OVERRIDE_PREVIEW)


def save_preview(all_rows: list[list[int]]) -> Path:
    cell_w = 64
    cell_h = 48
    cols = 16
    rows = (len(all_rows) + cols - 1) // cols
    sheet = Image.new("RGB", (cell_w * cols, 28 + cell_h * rows), "white")
    draw = ImageDraw.Draw(sheet)
    draw.text(
        (4, 4),
        "GENERATED 160-ROW CATALOG ENTRY SOURCE BITMAPS",
        fill="black",
        font=font(13),
    )
    for i, bitmap_rows in enumerate(all_rows):
        x = (i % cols) * cell_w
        y = 28 + (i // cols) * cell_h
        draw.rectangle((x, y, x + cell_w - 1, y + cell_h - 1), outline="black")
        draw.text((x + 3, y + 3), f"{i:03d}", fill="black", font=font(9))
        large = rows_to_image(bitmap_rows).resize((28, 28), Image.Resampling.NEAREST)
        sheet.paste(large, (x + (cell_w - 28) // 2, y + 17))
    out = OUT / "catalog_entry_generated_source_contact_sheet.png"
    sheet.save(out)
    return out


def format_block(all_rows: list[list[int]]) -> str:
    lines = [
        "// Generated catalog-entry bitmaps are inserted below by",
        "// analysis/screen_simulator/generate_catalog_entry_source_arrays.py.",
        "const uint16_t kCatalogEntryBitmaps[kCatalogItemCount][kCatalogVisualSide] PROGMEM = {",
    ]
    for index, rows in enumerate(all_rows):
        lines.append(f"    {{  // {index:03d}")
        for start in range(0, 16, 4):
            values = ", ".join(f"0x{row:04X}" for row in rows[start : start + 4])
            suffix = "," if start < 12 else ""
            lines.append(f"     {values}{suffix}")
        lines.append("    },")
    lines.append("};")
    return "\n".join(lines)


def replace_block(block: str) -> None:
    text = SOURCE.read_text(encoding="utf-8")
    pattern = re.compile(
        r"// Generated catalog-entry bitmaps are inserted below by\n"
        r"// analysis/screen_simulator/generate_catalog_entry_source_arrays.py\.\n"
        r"const uint16_t kCatalogEntryBitmaps\[kCatalogItemCount\]\[kCatalogVisualSide\] PROGMEM = \{.*?\n\};",
        re.DOTALL,
    )
    updated, count = pattern.subn(block, text, count=1)
    if count != 1:
        raise SystemExit("could not replace kCatalogEntryBitmaps block")
    SOURCE.write_text(updated, encoding="utf-8")


def main() -> None:
    if not PROOF.exists():
        raise SystemExit(
            "catalog proof is missing; run analysis/screen_simulator/run_simulator.py first"
        )
    image = Image.open(PROOF).convert("RGB")
    all_rows = [extract_icon_rows(image, index) for index in range(160)]
    override_records = apply_official_icon_overrides(all_rows)
    save_official_override_preview(override_records)
    preview = save_preview(all_rows)
    replace_block(format_block(all_rows))
    print(f"wrote {preview}")
    print(f"updated {SOURCE}")


if __name__ == "__main__":
    main()

