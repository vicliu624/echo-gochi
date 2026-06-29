from __future__ import annotations

import json
import re
from hashlib import sha1
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


ROOT = Path(__file__).resolve().parent
PROJECT_ROOT = ROOT.parents[1]
SOURCE_ROOT = PROJECT_ROOT / "src" if (PROJECT_ROOT / "src").exists() else PROJECT_ROOT
RAW = ROOT / "raw" / "howto"
GENERATED = ROOT / "generated"
SIM_OUT = PROJECT_ROOT / "analysis" / "screen_simulator" / "out"
CATALOG_SOURCE = SOURCE_ROOT / "EchoPetCatalog.cpp"
VISUAL_SOURCE = SOURCE_ROOT / "EchoPetCatalogVisuals.cpp"


ITEM_ASSETS = (
    "images__howto__item__img_icon_01.png",
    "images__howto__item__img_icon_02_pc.png",
    "images__howto__item__img_item_01.gif",
    "images__howto__item__img_item_02.gif",
    "images__howto__item__img_item_03_pc.gif",
)

DETECTED_TILE_DIR = GENERATED / "catalog_item_reference_tiles"
OFFICIAL_SOURCE_OVERRIDE_PREVIEW = (
    GENERATED / "catalog_item_official_source_overrides_contact_sheet.png"
)
OFFICIAL_SOURCE_OVERRIDE_ROWS = [
    {"catalog_index": 48, "label": "PENCIL"},
    {"catalog_index": 112, "label": "PENCIL"},
    {"catalog_index": 58, "label": "CAP"},
    {"catalog_index": 122, "label": "CAP"},
    {"catalog_index": 54, "label": "SHOVEL"},
    {"catalog_index": 118, "label": "SHOVEL"},
    {"catalog_index": 60, "label": "BALL"},
    {"catalog_index": 56, "label": "BALLOON"},
    {"catalog_index": 63, "label": "BALLOON"},
    {"catalog_index": 120, "label": "BALLOON"},
    {"catalog_index": 84, "label": "TRUMPET"},
    {"catalog_index": 141, "label": "TRUMPET"},
]


def font(size: int = 12) -> ImageFont.ImageFont:
    try:
        return ImageFont.truetype("DejaVuSansMono.ttf", size)
    except OSError:
        return ImageFont.load_default()


def fit_image(image: Image.Image, box: tuple[int, int]) -> Image.Image:
    copy = image.convert("RGBA")
    copy.thumbnail(box, Image.Resampling.NEAREST)
    return copy


def framed_cell(title: str, image: Image.Image, size: tuple[int, int]) -> Image.Image:
    w, h = size
    label_h = 18
    cell = Image.new("RGB", (w, h), "white")
    draw = ImageDraw.Draw(cell)
    draw.rectangle((0, 0, w - 1, h - 1), outline="black")
    draw.text((4, 2), title[: max(1, (w - 8) // 6)], fill="black", font=font(10))
    fitted = fit_image(image, (w - 8, h - label_h - 8))
    x = (w - fitted.width) // 2
    y = label_h + (h - label_h - fitted.height) // 2
    cell.paste(fitted.convert("RGB"), (x, y))
    return cell


def gif_frames(path: Path) -> list[Image.Image]:
    image = Image.open(path)
    frames: list[Image.Image] = []
    for i in range(getattr(image, "n_frames", 1)):
        image.seek(i)
        frames.append(image.convert("RGBA"))
    return frames


def dark_pixel(pixel: tuple[int, int, int]) -> bool:
    r, g, b = pixel
    return r < 120 and g < 120 and b < 120


def detect_reference_tiles(frame: Image.Image) -> list[tuple[int, int, int, int, int]]:
    """Detect representative LCD item clusters from the official PC GIF.

    The official frame is not a neatly separated atlas: many icons are broken
    into detached strokes. A small dilation groups strokes into reviewable
    reference tiles while still avoiding one giant merged row.
    """
    from collections import deque

    rgb = frame.convert("RGB")
    w, h = rgb.size
    black = [[False for _ in range(w)] for _ in range(h)]
    for y in range(h):
        for x in range(w):
            if dark_pixel(rgb.getpixel((x, y))):
                black[y][x] = True

    radius = 2
    mask = [[False for _ in range(w)] for _ in range(h)]
    for y in range(h):
        for x, value in enumerate(black[y]):
            if not value:
                continue
            for yy in range(max(0, y - radius), min(h, y + radius + 1)):
                for xx in range(max(0, x - radius), min(w, x + radius + 1)):
                    mask[yy][xx] = True

    seen = [[False for _ in range(w)] for _ in range(h)]
    components: list[tuple[int, int, int, int, int]] = []
    for y in range(h):
        for x in range(w):
            if not mask[y][x] or seen[y][x]:
                continue
            queue = deque([(x, y)])
            seen[y][x] = True
            xs: list[int] = []
            ys: list[int] = []
            while queue:
                cx, cy = queue.popleft()
                xs.append(cx)
                ys.append(cy)
                for nx, ny in ((cx + 1, cy), (cx - 1, cy), (cx, cy + 1), (cx, cy - 1)):
                    if 0 <= nx < w and 0 <= ny < h and mask[ny][nx] and not seen[ny][nx]:
                        seen[ny][nx] = True
                        queue.append((nx, ny))

            x0, y0, x1, y1 = min(xs), min(ys), max(xs) + 1, max(ys) + 1
            ink = sum(
                1
                for yy in range(y0, y1)
                for xx in range(x0, x1)
                if black[yy][xx]
            )
            width = x1 - x0
            height = y1 - y0
            if ink > 20 and 8 <= width <= 90 and 8 <= height <= 90:
                components.append((x0, y0, x1, y1, ink))

    return sorted(components, key=lambda item: (item[1], item[0]))


def crop_to_bw(image: Image.Image) -> Image.Image:
    rgb = image.convert("RGB")
    out = Image.new("1", rgb.size, 1)
    for y in range(rgb.height):
        for x in range(rgb.width):
            if dark_pixel(rgb.getpixel((x, y))):
                out.putpixel((x, y), 0)
    return out.convert("RGB")


def tile_signature(image: Image.Image) -> tuple[str, list[int]]:
    rgb = image.convert("RGB")
    xs: list[int] = []
    ys: list[int] = []
    for y in range(rgb.height):
        for x in range(rgb.width):
            if dark_pixel(rgb.getpixel((x, y))):
                xs.append(x)
                ys.append(y)
    if not xs:
        rows = [0 for _ in range(16)]
        return sha1(bytes(32)).hexdigest(), rows

    crop = rgb.crop((min(xs), min(ys), max(xs) + 1, max(ys) + 1))
    crop.thumbnail((16, 16), Image.Resampling.NEAREST)
    normalized = Image.new("RGB", (16, 16), "white")
    ox = (16 - crop.width) // 2
    oy = (16 - crop.height) // 2
    normalized.paste(crop, (ox, oy))

    rows: list[int] = []
    for y in range(16):
        value = 0
        for x in range(16):
            if dark_pixel(normalized.getpixel((x, y))):
                value |= 0x8000 >> x
        rows.append(value)
    payload = b"".join(row.to_bytes(2, "big") for row in rows)
    return sha1(payload).hexdigest(), rows


def rows_to_image(rows: list[int]) -> Image.Image:
    image = Image.new("RGB", (16, 16), "white")
    for y, row in enumerate(rows):
        for x in range(16):
            if (row & (0x8000 >> x)) != 0:
                image.putpixel((x, y), (0, 0, 0))
    return image


def parse_catalog_entries() -> list[dict[str, object]]:
    text = CATALOG_SOURCE.read_text(encoding="utf-8")
    entries: list[dict[str, object]] = []
    for match in re.finditer(
        r"\{(\d+),\s*\"([^\"]+)\",\s*CatalogKind::k([^,]+),",
        text,
    ):
        entries.append(
            {
                "index": int(match.group(1)),
                "name": match.group(2),
                "kind": match.group(3),
            }
        )
    entries.sort(key=lambda item: int(item["index"]))
    return entries


def load_catalog_bitmap_rows() -> list[list[int]]:
    text = VISUAL_SOURCE.read_text(encoding="utf-8")
    block = re.search(
        r"const uint16_t kCatalogEntryBitmaps\[kCatalogItemCount\]\[kCatalogVisualSide\] PROGMEM = \{(.*?)\n\};",
        text,
        re.S,
    )
    if not block:
        return []
    rows: list[list[int]] = []
    for match in re.finditer(
        r"\{\s*//\s*(\d{3})\n(.*?)\n\s*\},",
        block.group(1),
        re.S,
    ):
        values = [
            int(token, 0)
            for token in re.findall(r"0x[0-9A-Fa-f]+|0b[01]+", match.group(2))
        ]
        if len(values) == 16:
            rows.append(values)
    return rows


def catalog_status(index: int, kind: str) -> tuple[str, str]:
    override = next(
        (row for row in OFFICIAL_SOURCE_OVERRIDE_ROWS if row["catalog_index"] == index),
        None,
    )
    if override is not None:
        return "official-source C++ override", str(override["label"])
    if kind == "Food":
        return "official section scene only", "food/snack behavior reference"
    return "official section scene only", "item/shop behavior reference"


def save_catalog_semantic_status_sheet() -> tuple[Path, list[dict[str, object]]]:
    entries = parse_catalog_entries()
    bitmap_rows = load_catalog_bitmap_rows()
    if len(entries) != 152:
        raise SystemExit(f"expected 152 non-souvenir catalog entries, found {len(entries)}")
    if len(bitmap_rows) < 152:
        raise SystemExit(f"expected at least 152 catalog bitmap rows, found {len(bitmap_rows)}")

    override_indices = {
        int(row["catalog_index"]) for row in OFFICIAL_SOURCE_OVERRIDE_ROWS
    }
    cell_w = 160
    cell_h = 68
    cols = 4
    sheet_rows = (len(entries) + cols - 1) // cols
    sheet = Image.new("RGB", (cell_w * cols, 28 + cell_h * sheet_rows), "white")
    draw = ImageDraw.Draw(sheet)
    draw.text(
        (4, 4),
        "152 CATALOG C++ SOURCE ICONS WITH OFFICIAL SEMANTIC STATUS",
        fill="black",
        font=font(13),
    )

    records: list[dict[str, object]] = []
    for pos, entry in enumerate(entries):
        index = int(entry["index"])
        name = str(entry["name"])
        kind = str(entry["kind"])
        status, source = catalog_status(index, kind)
        x = (pos % cols) * cell_w
        y = 28 + (pos // cols) * cell_h
        draw.rectangle((x, y, x + cell_w - 1, y + cell_h - 1), outline="black")
        draw.text((x + 3, y + 3), f"{index:03d} {name}"[:25], fill="black", font=font(9))
        draw.text((x + 3, y + 15), kind, fill="black", font=font(8))
        draw.text((x + 3, y + 26), status[:30], fill="black", font=font(8))
        draw.text((x + 3, y + 37), source[:30], fill="black", font=font(8))
        icon = rows_to_image(bitmap_rows[index]).resize((24, 24), Image.Resampling.NEAREST)
        sheet.paste(icon, (x + cell_w - 30, y + 40))
        records.append(
            {
                "index": index,
                "name": name,
                "kind": kind,
                "status": status,
                "source": source,
                "coverage": (
                    "official-source C++ bitmap"
                    if index in override_indices
                    else "source-backed substitute with official section scene reference"
                ),
            }
        )

    out = GENERATED / "catalog_item_source_semantic_status_contact_sheet.png"
    sheet.save(out)
    return out, records


def save_detected_tile_sheet() -> tuple[Path, list[dict[str, object]]]:
    for old in DETECTED_TILE_DIR.glob("*.png"):
        old.unlink()
    DETECTED_TILE_DIR.mkdir(parents=True, exist_ok=True)

    source = RAW / "images__howto__item__img_item_03_pc.gif"
    frames = gif_frames(source)
    by_signature: dict[str, dict[str, object]] = {}
    tile_payloads: list[dict[str, object]] = []

    for frame_index, frame in enumerate(frames):
        for component in detect_reference_tiles(frame):
            x0, y0, x1, y1, ink = component
            pad = 3
            crop_box = (
                max(0, x0 - pad),
                max(0, y0 - pad),
                min(frame.width, x1 + pad),
                min(frame.height, y1 + pad),
            )
            raw_crop = frame.crop(crop_box).convert("RGBA")
            bw_crop = crop_to_bw(raw_crop)
            signature, rows16 = tile_signature(bw_crop)
            existing = by_signature.get(signature)
            if existing is not None:
                existing["seen_frames"].append(frame_index)
                existing["seen_count"] = int(existing["seen_count"]) + 1
                continue
            record = {
                "signature": signature,
                "source_file": source.name,
                "source_frame": frame_index,
                "bbox": [x0, y0, x1, y1],
                "crop_box": list(crop_box),
                "ink_pixels": ink,
                "rows16": rows16,
                "raw_crop_image": raw_crop,
                "bw_crop_image": bw_crop,
                "seen_frames": [frame_index],
                "seen_count": 1,
            }
            by_signature[signature] = record
            tile_payloads.append(record)

    tile_payloads.sort(
        key=lambda item: (
            int(item["source_frame"]),
            int(item["bbox"][1]),
            int(item["bbox"][0]),
        )
    )
    tile_records: list[dict[str, object]] = []

    cell_w = 92
    cell_h = 76
    cols = 6
    rows = (len(tile_payloads) + cols - 1) // cols
    sheet = Image.new("RGB", (cell_w * cols, 26 + cell_h * rows), "white")
    draw = ImageDraw.Draw(sheet)
    draw.text(
        (4, 4),
        "DETECTED OFFICIAL ITEM LCD REFERENCE TILES ACROSS ALL FRAMES",
        fill="black",
        font=font(14),
    )

    for i, payload in enumerate(tile_payloads):
        raw_name = f"{i:02d}_official_item_raw.png"
        bw_name = f"{i:02d}_official_item_bw.png"
        payload["raw_crop_image"].save(DETECTED_TILE_DIR / raw_name)
        payload["bw_crop_image"].save(DETECTED_TILE_DIR / bw_name)

        cell = framed_cell(
            f"ref {i:02d} f{payload['source_frame']}",
            payload["bw_crop_image"],
            (cell_w, cell_h),
        )
        sheet.paste(cell, ((i % cols) * cell_w, 26 + (i // cols) * cell_h))
        tile_records.append(
            {
                "index": i,
                "signature": payload["signature"],
                "source_file": payload["source_file"],
                "source_frame": payload["source_frame"],
                "seen_frames": payload["seen_frames"],
                "seen_count": payload["seen_count"],
                "bbox": payload["bbox"],
                "crop_box": payload["crop_box"],
                "ink_pixels": payload["ink_pixels"],
                "rows16": payload["rows16"],
                "raw_crop": str((DETECTED_TILE_DIR / raw_name).relative_to(ROOT)),
                "bw_crop": str((DETECTED_TILE_DIR / bw_name).relative_to(ROOT)),
                "coverage": (
                    "deduplicated official item LCD reference tile candidate; "
                    "not promoted to a catalog row until its label is unambiguous"
                ),
            }
        )

    out = GENERATED / "catalog_item_official_detected_tiles_contact_sheet.png"
    sheet.save(out)
    return out, tile_records


def save_item_asset_sheet(asset_info: list[dict[str, object]]) -> Path:
    cell_w = 250
    cell_h = 98
    rows: list[Image.Image] = []
    for filename in ITEM_ASSETS:
        path = RAW / filename
        if not path.exists():
            continue
        if path.suffix.lower() == ".gif":
            frames = gif_frames(path)
            samples = [
                frames[0],
                frames[len(frames) // 2],
                frames[-1],
            ]
            title_base = filename.replace("images__howto__item__", "")
            cells = [
                framed_cell(f"{title_base} f{idx}", frame, (cell_w, cell_h))
                for idx, frame in zip((0, len(frames) // 2, len(frames) - 1), samples)
            ]
            row = Image.new("RGB", (cell_w * 3, cell_h), "white")
            for i, cell in enumerate(cells):
                row.paste(cell, (i * cell_w, 0))
            rows.append(row)
            asset_info.append(
                {
                    "file": filename,
                    "kind": "gif",
                    "size": Image.open(path).size,
                    "frames": len(frames),
                    "role": "official item animation reference",
                }
            )
        else:
            image = Image.open(path).convert("RGBA")
            row = framed_cell(
                filename.replace("images__howto__item__", ""),
                image,
                (cell_w * 3, cell_h),
            )
            rows.append(row)
            asset_info.append(
                {
                    "file": filename,
                    "kind": "image",
                    "size": image.size,
                    "frames": 1,
                    "role": "official item semantic/icon reference",
                }
            )

    sheet_h = sum(row.height for row in rows) + 28
    sheet = Image.new("RGB", (cell_w * 3, sheet_h), "white")
    draw = ImageDraw.Draw(sheet)
    draw.text((4, 4), "OFFICIAL ITEM REFERENCE ASSETS", fill="black", font=font(14))
    y = 28
    for row in rows:
        sheet.paste(row, (0, y))
        y += row.height

    out = GENERATED / "catalog_item_official_assets_contact_sheet.png"
    sheet.save(out)
    return out


def save_pc_gif_frames() -> Path:
    path = RAW / "images__howto__item__img_item_03_pc.gif"
    frames = gif_frames(path)
    cell_w = 250
    cell_h = 86
    cols = 4
    rows = (len(frames) + cols - 1) // cols
    sheet = Image.new("RGB", (cell_w * cols, 26 + cell_h * rows), "white")
    draw = ImageDraw.Draw(sheet)
    draw.text(
        (4, 4),
        "OFFICIAL ITEM PC GIF FRAME CONTACT SHEET",
        fill="black",
        font=font(14),
    )
    for i, frame in enumerate(frames):
        cell = framed_cell(f"frame {i:02d}", frame, (cell_w, cell_h))
        x = (i % cols) * cell_w
        y = 26 + (i // cols) * cell_h
        sheet.paste(cell, (x, y))

    out = GENERATED / "catalog_item_official_pc_gif_frames.png"
    sheet.save(out)
    return out


def save_alignment_proof(
    asset_sheet: Path,
    pc_frames: Path,
    detected_tiles: Path,
    semantic_status: Path,
) -> Path:
    current = SIM_OUT / "catalog_items_proof_x2.png"
    parts: list[tuple[str, Image.Image]] = [
        ("official item asset samples", Image.open(asset_sheet).convert("RGBA")),
        ("official item pc gif frames", Image.open(pc_frames).convert("RGBA")),
        ("detected official LCD reference tiles", Image.open(detected_tiles).convert("RGBA")),
    ]
    if OFFICIAL_SOURCE_OVERRIDE_PREVIEW.exists():
        parts.append((
            "official item source overrides: mapped to C++ catalog rows",
            Image.open(OFFICIAL_SOURCE_OVERRIDE_PREVIEW).convert("RGBA"),
        ))
    parts.append((
        "current C++ catalog rows with official semantic status",
        Image.open(semantic_status).convert("RGBA"),
    ))
    if current.exists():
        parts.append(("current EchoPet catalog source proof", Image.open(current).convert("RGBA")))

    width = 1000
    title_h = 24
    prepared: list[tuple[str, Image.Image]] = []
    for title, image in parts:
        fitted = fit_image(image, (width - 8, 620))
        prepared.append((title, fitted))

    height = 28 + sum(title_h + image.height + 10 for _, image in prepared)
    sheet = Image.new("RGB", (width, height), "white")
    draw = ImageDraw.Draw(sheet)
    draw.text(
        (4, 4),
        "CATALOG ITEM OFFICIAL VS SOURCE ALIGNMENT PROOF",
        fill="black",
        font=font(14),
    )
    y = 28
    for title, image in prepared:
        draw.rectangle((0, y, width - 1, y + title_h + image.height + 7), outline="black")
        draw.text((4, y + 4), title, fill="black", font=font(12))
        sheet.paste(image.convert("RGB"), ((width - image.width) // 2, y + title_h))
        y += title_h + image.height + 10

    out = GENERATED / "catalog_item_official_alignment_proof.png"
    sheet.save(out)
    return out


def main() -> None:
    GENERATED.mkdir(parents=True, exist_ok=True)
    missing = [name for name in ITEM_ASSETS if not (RAW / name).exists()]
    if missing:
        raise SystemExit(f"missing official item assets: {', '.join(missing)}")

    asset_info: list[dict[str, object]] = []
    asset_sheet = save_item_asset_sheet(asset_info)
    pc_frames = save_pc_gif_frames()
    detected_tiles, tile_records = save_detected_tile_sheet()
    semantic_status, semantic_rows = save_catalog_semantic_status_sheet()
    alignment = save_alignment_proof(
        asset_sheet, pc_frames, detected_tiles, semantic_status
    )

    manifest = {
        "source_page": "https://tamagotchi-official.com/gb/series/connection/howto/",
        "runtime_boundary": (
            "Official PNG/GIF files are analysis-only references. Firmware "
            "resources remain C/C++ bitmap arrays and composed frame tables."
        ),
        "assets": asset_info,
        "generated": {
            "asset_contact_sheet": str(asset_sheet.relative_to(ROOT)),
            "pc_gif_frames": str(pc_frames.relative_to(ROOT)),
            "detected_lcd_tiles": str(detected_tiles.relative_to(ROOT)),
            "source_semantic_status_contact_sheet": str(semantic_status.relative_to(ROOT)),
            "alignment_proof": str(alignment.relative_to(ROOT)),
        },
        "detected_lcd_tile_count": len(tile_records),
        "detected_lcd_tiles": tile_records,
        "official_source_override_count": (
            len(OFFICIAL_SOURCE_OVERRIDE_ROWS)
            if OFFICIAL_SOURCE_OVERRIDE_PREVIEW.exists()
            else 0
        ),
        "official_source_overrides": (
            OFFICIAL_SOURCE_OVERRIDE_ROWS
            if OFFICIAL_SOURCE_OVERRIDE_PREVIEW.exists()
            else []
        ),
        "official_source_override_preview": (
            str(OFFICIAL_SOURCE_OVERRIDE_PREVIEW.relative_to(ROOT))
            if OFFICIAL_SOURCE_OVERRIDE_PREVIEW.exists()
            else None
        ),
        "catalog_row_status_count": len(semantic_rows),
        "catalog_row_status": semantic_rows,
        "coverage_note": (
            "These official how-to assets prove item semantics and animation "
            "style. Twelve clearly mapped catalog rows now have official-source "
            "C/C++ bitmap overrides, but the reference set still does not "
            "provide one accepted source bitmap for every one of the 160 "
            "EchoPet catalog rows."
        ),
    }
    manifest_path = GENERATED / "catalog_item_official_reference_manifest.json"
    manifest_path.write_text(json.dumps(manifest, indent=2), encoding="utf-8")

    print(f"wrote {asset_sheet}")
    print(f"wrote {pc_frames}")
    print(f"wrote {alignment}")
    print(f"wrote {manifest_path}")


if __name__ == "__main__":
    main()


