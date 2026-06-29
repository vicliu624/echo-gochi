from __future__ import annotations

import json
import re
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


ROOT = Path(__file__).resolve().parent
PROJECT_ROOT = ROOT.parents[1]
SOURCE_ROOT = PROJECT_ROOT / "src" if (PROJECT_ROOT / "src").exists() else PROJECT_ROOT
GENERATED = ROOT / "generated"
MANIFEST = GENERATED / "character_official_reference_manifest.json"
SOURCE = SOURCE_ROOT / "EchoPetCharacterVisuals.cpp"


def font(size: int = 12) -> ImageFont.ImageFont:
    try:
        return ImageFont.truetype("DejaVuSansMono.ttf", size)
    except OSError:
        return ImageFont.load_default()


def dark(pixel: tuple[int, ...]) -> bool:
    if len(pixel) >= 4 and pixel[3] < 64:
        return False
    r, g, b = pixel[:3]
    return r < 170 and g < 170 and b < 170


def official_to_size(path: Path, side: int) -> tuple[list[int], Image.Image]:
    source = Image.open(path).convert("RGBA")
    pixels = source.load()
    xs: list[int] = []
    ys: list[int] = []
    for y in range(source.height):
        for x in range(source.width):
            if dark(pixels[x, y]):
                xs.append(x)
                ys.append(y)
    if not xs:
        return [0] * side, Image.new("RGB", (side, side), "white")

    pad = max(2, min(source.width, source.height) // 32)
    x0 = max(0, min(xs) - pad)
    y0 = max(0, min(ys) - pad)
    x1 = min(source.width, max(xs) + 1 + pad)
    y1 = min(source.height, max(ys) + 1 + pad)
    crop = source.crop((x0, y0, x1, y1))

    max_side = side - 1
    scale = min(max_side / crop.width, max_side / crop.height)
    new_size = (
        max(1, int(round(crop.width * scale))),
        max(1, int(round(crop.height * scale))),
    )
    resized = crop.resize(new_size, Image.Resampling.NEAREST).convert("RGBA")
    canvas = Image.new("RGBA", (side, side), "white")
    ox = (side - resized.width) // 2
    oy = side - resized.height - 1
    if oy < 0:
        oy = 0
    canvas.alpha_composite(resized, (ox, oy))

    rows: list[int] = []
    rgb = canvas.convert("RGBA")
    for y in range(side):
        value = 0
        for x in range(side):
            if dark(rgb.getpixel((x, y))):
                if side <= 16:
                    value |= 0x8000 >> x
                else:
                    value |= 0x800000 >> x
        rows.append(value)

    proof = Image.new("RGB", (side, side), "white")
    for y, row in enumerate(rows):
        for x in range(side):
            mask = (0x8000 >> x) if side <= 16 else (0x800000 >> x)
            if (row & mask) != 0:
                proof.putpixel((x, y), (0, 0, 0))
    return rows, proof


def fit_image(image: Image.Image, box: tuple[int, int]) -> Image.Image:
    copy = image.convert("RGBA")
    copy.thumbnail(box, Image.Resampling.LANCZOS)
    return copy


def proof_ink_metrics(proof: Image.Image) -> dict[str, object]:
    xs: list[int] = []
    ys: list[int] = []
    for y in range(proof.height):
        for x in range(proof.width):
            if dark(proof.getpixel((x, y))):
                xs.append(x)
                ys.append(y)
    if not xs:
        return {
            "ink_pixels": 0,
            "ink_ratio": 0.0,
            "bbox": None,
        }
    bbox = [
        min(xs),
        min(ys),
        max(xs) - min(xs) + 1,
        max(ys) - min(ys) + 1,
    ]
    return {
        "ink_pixels": len(xs),
        "ink_ratio": round(len(xs) / (proof.width * proof.height), 4),
        "bbox": bbox,
    }


def review_flags(metrics16: dict[str, object],
                 metrics24: dict[str, object]) -> list[str]:
    flags: list[str] = []
    ink16 = int(metrics16["ink_pixels"])
    ink24 = int(metrics24["ink_pixels"])
    bbox16 = metrics16["bbox"]
    bbox24 = metrics24["bbox"]
    if ink16 < 12:
        flags.append("16 sparse")
    if ink24 < 24:
        flags.append("24 sparse")
    if ink16 > 126:
        flags.append("16 dense")
    if ink24 > 288:
        flags.append("24 dense")
    if isinstance(bbox16, list) and (bbox16[2] < 7 or bbox16[3] < 7):
        flags.append("16 tiny")
    if isinstance(bbox24, list) and (bbox24[2] < 11 or bbox24[3] < 11):
        flags.append("24 tiny")
    return flags


def save_source_alignment_status(records: list[dict[str, object]],
                                 proofs16: list[Image.Image],
                                 proofs24: list[Image.Image]) -> tuple[Path, Path]:
    cell_w = 180
    cell_h = 94
    cols = 5
    rows = (len(records) + cols - 1) // cols
    sheet = Image.new("RGB", (cell_w * cols, 30 + cell_h * rows), "white")
    draw = ImageDraw.Draw(sheet)
    draw.text(
        (4, 4),
        "CHARACTER SOURCE ALIGNMENT STATUS: OFFICIAL / 16x16 C++ / 24x24 C++",
        fill="black",
        font=font(13),
    )

    status_rows: list[dict[str, object]] = []
    for i, record in enumerate(records):
        proof16 = proofs16[i]
        proof24 = proofs24[i]
        metrics16 = proof_ink_metrics(proof16)
        metrics24 = proof_ink_metrics(proof24)
        flags = review_flags(metrics16, metrics24)
        status = "review" if flags else "official-derived"
        status_rows.append(
            {
                "index": int(record["index"]),
                "page_id": int(record["page_id"]),
                "catalog_name": str(record["catalog_name"]),
                "official_image": str(record["local_image"]),
                "status": status,
                "flags": flags,
                "bitmap16": metrics16,
                "bitmap24": metrics24,
            }
        )

        x = (i % cols) * cell_w
        y = 30 + (i // cols) * cell_h
        draw.rectangle((x, y, x + cell_w - 1, y + cell_h - 1), outline="black")
        title = f'{i:02d} {record["catalog_name"]}'
        draw.text((x + 4, y + 4), title[:24], fill="black", font=font(9))
        draw.text((x + 4, y + 17), status.upper()[:24], fill="black", font=font(8))

        official = Image.open(ROOT / str(record["local_image"])).convert("RGBA")
        official_thumb = fit_image(official, (42, 42))
        sheet.paste(official_thumb.convert("RGB"), (x + 4, y + 38))
        sheet.paste(
            proof16.resize((32, 32), Image.Resampling.NEAREST),
            (x + 56, y + 42),
        )
        sheet.paste(
            proof24.resize((36, 36), Image.Resampling.NEAREST),
            (x + 100, y + 40),
        )
        draw.text((x + 53, y + 76), "16", fill="black", font=font(8))
        draw.text((x + 101, y + 76), "24", fill="black", font=font(8))
        metric = f'i16 {metrics16["ink_pixels"]} i24 {metrics24["ink_pixels"]}'
        draw.text((x + 4, y + 82), metric[:28], fill="black", font=font(8))
        if flags:
            draw.text((x + 138, y + 39), ",".join(flags)[:8],
                      fill="black", font=font(7))

    out = GENERATED / "character_source_alignment_status_contact_sheet.png"
    sheet.save(out)

    status_path = GENERATED / "character_source_alignment_status.json"
    status_path.write_text(
        json.dumps(
            {
                "source_manifest": str(MANIFEST.relative_to(ROOT)),
                "runtime_boundary": (
                    "Official raster files are analysis-only references. The "
                    "firmware character visuals are generated C/C++ bitmap rows."
                ),
                "character_count": len(status_rows),
                "generated": {
                    "source_alignment_status_contact_sheet": str(
                        out.relative_to(ROOT)
                    )
                },
                "characters": status_rows,
            },
            indent=2,
        ),
        encoding="utf-8",
    )
    return out, status_path


def write_preview(records: list[dict[str, object]], proofs: list[Image.Image],
                  out_name: str, title: str, preview_side: int) -> Path:
    cell_w = 88
    cell_h = 64
    cols = 10
    rows = (len(proofs) + cols - 1) // cols
    sheet = Image.new("RGB", (cell_w * cols, 28 + cell_h * rows), "white")
    draw = ImageDraw.Draw(sheet)
    draw.text(
        (4, 4),
        title,
        fill="black",
        font=font(13),
    )
    for i, proof in enumerate(proofs):
        x = (i % cols) * cell_w
        y = 28 + (i // cols) * cell_h
        draw.rectangle((x, y, x + cell_w - 1, y + cell_h - 1), outline="black")
        label = f'{i:02d} {records[i]["catalog_name"]}'
        draw.text((x + 3, y + 3), label[:13], fill="black", font=font(9))
        large = proof.resize((preview_side, preview_side), Image.Resampling.NEAREST)
        sheet.paste(large, (x + (cell_w - preview_side) // 2, y + 22))
    out = GENERATED / out_name
    sheet.save(out)
    return out


def format_rows_16(records: list[dict[str, object]], all_rows: list[list[int]]) -> str:
    lines: list[str] = [
        "const uint16_t kCharacterCatalogBitmaps[kCharacterVisualCatalogCount]",
        "                                        [kCharacterVisualSide] PROGMEM = {",
        "    // Generated by analysis/official_reference/generate_character_source_arrays.py.",
        "    // Official raster files remain analysis-only references; firmware stores",
        "    // the derived runtime representation as C/C++ source bitmap rows.",
    ]
    for record, rows in zip(records, all_rows):
        index = int(record["index"])
        name = str(record["catalog_name"])
        page_id = int(record["page_id"])
        lines.append(f"    {{  // {index:02d} {name} official page {page_id}")
        for start in range(0, 16, 4):
            values = ", ".join(f"0x{row:04X}" for row in rows[start : start + 4])
            suffix = "," if start < 12 else ""
            lines.append(f"     {values}{suffix}")
        lines.append("    },")
    lines.append("};")
    return "\n".join(lines)


def format_rows_24(records: list[dict[str, object]], all_rows: list[list[int]]) -> str:
    lines: list[str] = [
        "const uint32_t kCharacterIdleBitmaps[kCharacterVisualCatalogCount]",
        "                                    [kCharacterIdleVisualSide] PROGMEM = {",
        "    // Generated by analysis/official_reference/generate_character_source_arrays.py.",
        "    // Official raster files remain analysis-only references; firmware stores",
        "    // the derived runtime representation as C/C++ source bitmap rows.",
    ]
    for record, rows in zip(records, all_rows):
        index = int(record["index"])
        name = str(record["catalog_name"])
        page_id = int(record["page_id"])
        lines.append(f"    {{  // {index:02d} {name} official page {page_id}")
        for start in range(0, 24, 4):
            values = ", ".join(f"0x{row:06X}UL" for row in rows[start : start + 4])
            suffix = "," if start < 20 else ""
            lines.append(f"     {values}{suffix}")
        lines.append("    },")
    lines.append("};")
    return "\n".join(lines)


def replace_catalog_block(new_block: str, kind: str) -> None:
    text = SOURCE.read_text(encoding="utf-8")
    if kind == "catalog16":
        pattern = re.compile(
            r"const uint16_t kCharacterCatalogBitmaps\[kCharacterVisualCatalogCount\]\s*"
            r"\[kCharacterVisualSide\] PROGMEM = \{.*?\n\};",
            re.DOTALL,
        )
    elif kind == "idle24":
        pattern = re.compile(
            r"const uint32_t kCharacterIdleBitmaps\[kCharacterVisualCatalogCount\]\s*"
            r"\[kCharacterIdleVisualSide\] PROGMEM = \{.*?\n\};",
            re.DOTALL,
        )
    else:
        raise ValueError(kind)
    updated, count = pattern.subn(new_block, text, count=1)
    if count != 1:
        raise SystemExit(f"could not replace {kind} block")
    SOURCE.write_text(updated, encoding="utf-8")


def main() -> None:
    if not MANIFEST.exists():
        raise SystemExit(
            "character official manifest is missing; run "
            "generate_character_reference_sheets.py first"
        )
    data = json.loads(MANIFEST.read_text(encoding="utf-8"))
    records = data["characters"]
    all_rows_16: list[list[int]] = []
    proofs_16: list[Image.Image] = []
    all_rows_24: list[list[int]] = []
    proofs_24: list[Image.Image] = []
    for record in records:
        image_path = ROOT / str(record["local_image"])
        rows16, proof16 = official_to_size(image_path, 16)
        rows24, proof24 = official_to_size(image_path, 24)
        all_rows_16.append(rows16)
        proofs_16.append(proof16)
        all_rows_24.append(rows24)
        proofs_24.append(proof24)
    preview16 = write_preview(
        records,
        proofs_16,
        "character_generated_source_contact_sheet.png",
        "GENERATED CHARACTER 16x16 AVATAR C++ BITMAPS FROM OFFICIAL REFERENCES",
        32,
    )
    preview24 = write_preview(
        records,
        proofs_24,
        "character_idle_generated_source_contact_sheet.png",
        "GENERATED CHARACTER 24x24 IDLE C++ BITMAPS FROM OFFICIAL REFERENCES",
        36,
    )
    status_sheet, status_path = save_source_alignment_status(
        records, proofs_16, proofs_24
    )
    replace_catalog_block(format_rows_16(records, all_rows_16), "catalog16")
    replace_catalog_block(format_rows_24(records, all_rows_24), "idle24")
    print(f"wrote {preview16}")
    print(f"wrote {preview24}")
    print(f"wrote {status_sheet}")
    print(f"wrote {status_path}")
    print(f"updated {SOURCE}")


if __name__ == "__main__":
    main()


