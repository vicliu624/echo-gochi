from __future__ import annotations

import json
import re
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


ROOT = Path(__file__).resolve().parent
PROJECT_ROOT = ROOT.parents[1]
SOURCE_ROOT = PROJECT_ROOT / "src" if (PROJECT_ROOT / "src").exists() else PROJECT_ROOT
RAW = ROOT / "raw" / "howto"
GENERATED = ROOT / "generated"
SIM_OUT = PROJECT_ROOT / "analysis" / "screen_simulator" / "out"
SOURCE = SOURCE_ROOT / "EchoPetCatalogVisuals.cpp"
OFFICIAL_ITEM_ICON_PC = RAW / "images__howto__item__img_icon_02_pc.png"

SOUVENIR_NAMES = [
    "FIRST HATCH",
    "BABY STEP",
    "FIRST MEAL",
    "FIRST SNACK",
    "CLEAN ROOM",
    "GOOD MEDS",
    "LIGHTS OFF",
    "TRAINING",
    "FIRST GAME",
    "SKIS",
    "PALM TREE",
    "SURFBOARD",
    "PANDA BEAR",
    "MARACAS",
    "SPRINT RIB",
    "HOOP MEDAL",
    "FIRST SHOP",
    "BEST BUY",
    "SECRET CODE",
    "PASSWORD",
    "DONATION",
    "GIFT BOX",
    "VISIT TAG",
    "FRIEND BADGE",
    "BEST FRIEND",
    "PARTNER",
    "BABY PHOTO",
    "PARENT NOTE",
    "GEN PHOTO",
    "BIRTHDAY",
    "ANNIV 1",
    "ANNIV 2",
    "ANNIV 3",
    "ANNIV 4",
    "ANNIV 5",
    "CARE GOOD",
    "CARE HARD",
    "SICK DAY",
    "TOOTH FIX",
    "NO MESS",
    "FULL HEART",
    "FULL FOOD",
    "FULL TRAIN",
    "HEAVY WIN",
    "LIGHT SLEEP",
    "WAKE SMILE",
    "TOY PLAY",
    "BOOK DAY",
    "MUSIC DAY",
    "TRAVEL PASS",
    "TICKET",
    "CHARM",
    "BLOCK SET",
    "ROPE SKIP",
    "TRUMPET",
    "BALL PLAY",
    "SHOPPER",
    "COLLECTOR",
    "FAMILY TREE",
    "OLD FRIEND",
    "LEGACY",
    "MEMORY DAY",
    "NEW GEN",
    "ALBUM",
]

SOUVENIR_SEMANTIC_SOURCES = {
    "growth": {
        "files": [
            "images__howto__growth__img_growth_01.gif",
            "images__howto__growth__img_growth_02.gif",
        ],
        "rows": [0, 1],
        "note": "egg, baby, growth-stage presentation",
    },
    "nurture_food": {
        "files": ["images__howto__breed__img_breed_02.gif"],
        "rows": [2, 3, 41],
        "note": "meal/snack feeding and food care",
    },
    "nurture_toilet": {
        "files": ["images__howto__breed__img_breed_03.gif"],
        "rows": [4, 39],
        "note": "mess and toilet cleanup",
    },
    "nurture_training": {
        "files": ["images__main__howto__breed__img_breed_04.gif"],
        "rows": [7, 35, 36, 42],
        "note": "discipline and care-quality reactions",
    },
    "nurture_medicine": {
        "files": ["images__howto__breed__img_breed_05.gif"],
        "rows": [5, 37, 38],
        "note": "sickness, medicine, recovery",
    },
    "nurture_lights": {
        "files": ["images__main__howto__breed__img_breed_06.gif"],
        "rows": [6, 44, 45],
        "note": "lights, sleep, wake behavior",
    },
    "games": {
        "files": [
            "images__howto__game__img_game_01.gif",
            "images__howto__game__img_game_06.gif",
            "images__howto__game__img_game_07.gif",
            "images__main__howto__game__img_game_05.gif",
        ],
        "rows": [8, 14, 15, 43, 61],
        "note": "game play, win, memory/medal semantics",
    },
    "shop_items": {
        "files": [
            "images__howto__item__img_item_03_pc.gif",
            "images__howto__item__img_icon_02_pc.png",
        ],
        "rows": [
            9,
            10,
            11,
            12,
            13,
            16,
            17,
            21,
            46,
            47,
            48,
            49,
            50,
            51,
            52,
            53,
            54,
            55,
            56,
            57,
        ],
        "note": "shop, item, prize, toy, ticket, ball, trumpet references",
    },
    "password_secret": {
        "files": [
            "images__main__howto__special__img_special_01.gif",
            "images__howto__special__img_special_02.gif",
        ],
        "rows": [18, 19, 20, 29, 30, 31, 32, 33, 34],
        "note": "password, secret-code, date/celebration reward context",
    },
    "connect": {
        "files": [
            "images__howto__connect__img_connect_01.gif",
            "images__howto__connect__img_connect_02.gif",
            "images__howto__connect__img_connect_04.gif",
            "images__howto__connect__img_connect_05.gif",
            "images__howto__connect__img_connect_07.gif",
        ],
        "rows": [22, 23, 24, 25, 26, 27, 28, 58, 59, 60, 62, 63],
        "note": "visit, present, friendship, love, partner, baby, generation",
    },
}

OFFICIAL_SOUVENIR_ICON_OVERRIDES = [
    {
        "label": "TRUMPET",
        "source_file": "images__howto__item__img_icon_02_pc.png",
        "source_box": (748, 3, 856, 114),
        "souvenir_indices": [54],
    },
    {
        "label": "BALL",
        "source_file": "images__howto__item__img_icon_02_pc.png",
        "source_box": (615, 27, 695, 105),
        "souvenir_indices": [55],
    },
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
    label_h = 28
    cell = Image.new("RGB", (w, h), "white")
    draw = ImageDraw.Draw(cell)
    draw.rectangle((0, 0, w - 1, h - 1), outline="black")
    draw.text((4, 3), title[: max(1, (w - 8) // 6)], fill="black", font=font(10))
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


def representative_frames(path: Path) -> list[tuple[int, Image.Image]]:
    if path.suffix.lower() != ".gif":
        image = Image.open(path).convert("RGBA")
        return [(0, image)]
    frames = gif_frames(path)
    if len(frames) <= 3:
        return list(enumerate(frames))
    picks = [0, len(frames) // 2, len(frames) - 1]
    return [(i, frames[i]) for i in picks]


def is_black(pixel: tuple[int, ...]) -> bool:
    r, g, b = pixel[:3]
    return r < 128 and g < 128 and b < 128


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


def load_souvenir_rows() -> list[tuple[str, list[int]]]:
    text = SOURCE.read_text(encoding="utf-8")
    block_match = re.search(
        r"const uint16_t kSouvenirMemoryBitmaps\[kCatalogSouvenirMemoryVisualCount\]\s*"
        r"\[kCatalogVisualSide\] PROGMEM = \{(.*?)\n\};",
        text,
        re.S,
    )
    if not block_match:
        raise SystemExit("could not find kSouvenirMemoryBitmaps block")

    entries: list[tuple[int, str, list[int]]] = []
    for match in re.finditer(
        r"\{\s*//\s*(\d{2}) ([^\n]+)\n(.*?)\n\s*\},",
        block_match.group(1),
        re.S,
    ):
        index = int(match.group(1))
        label = match.group(2).strip()
        values: list[int] = []
        for token in re.findall(r"0x[0-9A-Fa-f]+|0b[01]+", match.group(3)):
            values.append(int(token, 0))
        if len(values) != 16:
            raise SystemExit(f"souvenir row {index:02d} has {len(values)} rows")
        entries.append((index, label, values))

    entries.sort(key=lambda item: item[0])
    if len(entries) != len(SOUVENIR_NAMES):
        raise SystemExit(
            f"expected {len(SOUVENIR_NAMES)} souvenir rows, found {len(entries)}"
        )
    return [(label, values) for _, label, values in entries]


def format_souvenir_block(entries: list[tuple[str, list[int]]]) -> str:
    lines = [
        "// Generated souvenir memory bitmaps are maintained by",
        "// analysis/official_reference/generate_souvenir_reference_sheets.py.",
        "const uint16_t kSouvenirMemoryBitmaps[kCatalogSouvenirMemoryVisualCount]",
        "                                       [kCatalogVisualSide] PROGMEM = {",
    ]
    for index, (label, rows) in enumerate(entries):
        lines.append(f"    {{  // {index:02d} {label}")
        for start in range(0, 16, 4):
            values = ", ".join(f"0x{row:04X}" for row in rows[start : start + 4])
            suffix = "," if start < 12 else ","
            lines.append(f"     {values}{suffix}")
        lines.append("    },")
    lines.append("};")
    return "\n".join(lines)


def replace_souvenir_block(entries: list[tuple[str, list[int]]]) -> None:
    text = SOURCE.read_text(encoding="utf-8")
    pattern = re.compile(
        r"(?:// Generated souvenir memory bitmaps are maintained by\n"
        r"// analysis/official_reference/generate_souvenir_reference_sheets.py\.\n)?"
        r"const uint16_t kSouvenirMemoryBitmaps\[kCatalogSouvenirMemoryVisualCount\]\s*"
        r"\[kCatalogVisualSide\] PROGMEM = \{.*?\n\};",
        re.S,
    )
    updated, count = pattern.subn(format_souvenir_block(entries), text, count=1)
    if count != 1:
        raise SystemExit("could not replace kSouvenirMemoryBitmaps block")
    SOURCE.write_text(updated, encoding="utf-8")


def apply_official_overrides(entries: list[tuple[str, list[int]]]) -> list[dict[str, object]]:
    if not OFFICIAL_ITEM_ICON_PC.exists():
        return []
    source = Image.open(OFFICIAL_ITEM_ICON_PC).convert("RGBA")
    records: list[dict[str, object]] = []
    for override in OFFICIAL_SOUVENIR_ICON_OVERRIDES:
        source_path = RAW / str(override["source_file"])
        if not source_path.exists():
            continue
        rows = official_icon_rows(source, override["source_box"])
        for index in override["souvenir_indices"]:
            label = entries[index][0]
            entries[index] = (label, rows)
            records.append(
                {
                    "label": override["label"],
                    "souvenir_index": index,
                    "souvenir_name": label,
                    "source_file": override["source_file"],
                    "source_box": list(override["source_box"]),
                    "coverage": "official-source C++ 16x16 override",
                }
            )
    return records


def save_official_override_preview(records: list[dict[str, object]]) -> Path | None:
    if not records:
        return None
    cell_w = 92
    cell_h = 66
    sheet = Image.new("RGB", (cell_w * len(records), cell_h), "white")
    draw = ImageDraw.Draw(sheet)
    source = Image.open(OFFICIAL_ITEM_ICON_PC).convert("RGBA")
    for i, record in enumerate(records):
        x = i * cell_w
        rows = official_icon_rows(source, tuple(record["source_box"]))
        draw.rectangle((x, 0, x + cell_w - 1, cell_h - 1), outline="black")
        draw.text((x + 3, 3), f"{record['souvenir_index']:02d}", fill="black", font=font(9))
        draw.text((x + 3, 15), str(record["label"])[:11], fill="black", font=font(9))
        icon = rows_to_image(rows).resize((32, 32), Image.Resampling.NEAREST)
        sheet.paste(icon, (x + (cell_w - 32) // 2, 31))
    out = GENERATED / "souvenir_official_source_overrides_contact_sheet.png"
    sheet.save(out)
    return out


def save_candidate_sheet() -> tuple[Path, list[dict[str, object]]]:
    cell_w = 226
    cell_h = 92
    rows: list[Image.Image] = []
    asset_records: list[dict[str, object]] = []
    for source_id, info in SOUVENIR_SEMANTIC_SOURCES.items():
        for filename in info["files"]:
            path = RAW / filename
            if not path.exists():
                continue
            frames = representative_frames(path)
            row = Image.new("RGB", (cell_w * 3, cell_h), "white")
            for cell_index, (frame_index, frame) in enumerate(frames[:3]):
                title = f"{source_id} f{frame_index}"
                row.paste(framed_cell(title, frame, (cell_w, cell_h)), (cell_index * cell_w, 0))
            rows.append(row)
            image = Image.open(path)
            asset_records.append(
                {
                    "source_id": source_id,
                    "file": filename,
                    "size": list(image.size),
                    "frames": getattr(image, "n_frames", 1),
                    "souvenir_indices": info["rows"],
                    "souvenir_names": [SOUVENIR_NAMES[i] for i in info["rows"]],
                    "coverage": "official scene/icon semantic reference, not a full per-souvenir atlas",
                    "note": info["note"],
                }
            )

    height = 28 + len(rows) * cell_h
    sheet = Image.new("RGB", (cell_w * 3, height), "white")
    draw = ImageDraw.Draw(sheet)
    draw.text(
        (4, 4),
        "OFFICIAL SOUVENIR/MEMORY SEMANTIC CANDIDATE SOURCES",
        fill="black",
        font=font(13),
    )
    y = 28
    for row in rows:
        sheet.paste(row, (0, y))
        y += cell_h
    out = GENERATED / "souvenir_official_candidate_assets_contact_sheet.png"
    sheet.save(out)
    return out, asset_records


def semantic_source_for(index: int) -> tuple[str, str]:
    hits = []
    for source_id, info in SOUVENIR_SEMANTIC_SOURCES.items():
        if index in info["rows"]:
            hits.append(source_id)
    if not hits:
        return "none", "no direct official semantic candidate in cached how-to assets"
    return "+".join(hits), "official semantic scene/icon reference exists"


def save_per_memory_semantic_sheet(entries: list[tuple[str, list[int]]], records: list[dict[str, object]]) -> Path:
    override_indices = {int(record["souvenir_index"]) for record in records}
    cell_w = 158
    cell_h = 76
    cols = 4
    sheet = Image.new("RGB", (cell_w * cols, 28 + cell_h * 16), "white")
    draw = ImageDraw.Draw(sheet)
    draw.text(
        (4, 4),
        "64 SOUVENIR C++ SOURCE ICONS WITH OFFICIAL SEMANTIC STATUS",
        fill="black",
        font=font(13),
    )
    for index, (label, rows) in enumerate(entries):
        x = (index % cols) * cell_w
        y = 28 + (index // cols) * cell_h
        source_id, status = semantic_source_for(index)
        if index in override_indices:
            status = "official-source C++ override"
        draw.rectangle((x, y, x + cell_w - 1, y + cell_h - 1), outline="black")
        draw.text((x + 3, y + 3), f"{index:02d} {label}"[:24], fill="black", font=font(9))
        draw.text((x + 3, y + 15), source_id[:24], fill="black", font=font(8))
        draw.text((x + 3, y + 26), status[:28], fill="black", font=font(8))
        icon = rows_to_image(rows).resize((32, 32), Image.Resampling.NEAREST)
        sheet.paste(icon, (x + 6, y + 40))
    out = GENERATED / "souvenir_source_semantic_status_contact_sheet.png"
    sheet.save(out)
    return out


def save_alignment_proof(candidate_sheet: Path, status_sheet: Path, override_sheet: Path | None) -> Path:
    current = SIM_OUT / "catalog_souvenirs_proof_x2.png"
    parts: list[tuple[str, Image.Image]] = [
        ("official semantic candidate sources", Image.open(candidate_sheet).convert("RGBA")),
        ("current C++ souvenir rows with official semantic status", Image.open(status_sheet).convert("RGBA")),
    ]
    if override_sheet is not None:
        parts.append(("official-source souvenir overrides", Image.open(override_sheet).convert("RGBA")))
    if current.exists():
        parts.append(("current EchoPet souvenir proof", Image.open(current).convert("RGBA")))

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
        "SOUVENIR OFFICIAL SEMANTIC VS SOURCE ALIGNMENT PROOF",
        fill="black",
        font=font(13),
    )
    y = 28
    for title, image in prepared:
        draw.rectangle((0, y, width - 1, y + title_h + image.height + 7), outline="black")
        draw.text((4, y + 4), title, fill="black", font=font(12))
        sheet.paste(image.convert("RGB"), ((width - image.width) // 2, y + title_h))
        y += title_h + image.height + 10

    out = GENERATED / "souvenir_official_alignment_proof.png"
    sheet.save(out)
    return out


def write_manifest(
    asset_records: list[dict[str, object]],
    override_records: list[dict[str, object]],
    candidate_sheet: Path,
    status_sheet: Path,
    override_sheet: Path | None,
    alignment: Path,
) -> Path:
    rows = []
    override_by_index = {
        int(record["souvenir_index"]): record for record in override_records
    }
    for index, name in enumerate(SOUVENIR_NAMES):
        source_id, status = semantic_source_for(index)
        coverage = "official semantic reference"
        if index in override_by_index:
            status = "official-source C++ override"
            coverage = "official-source C++ bitmap"
        elif source_id == "none":
            coverage = "source-backed substitute; no direct cached official row"
        rows.append(
            {
                "index": index,
                "name": name,
                "semantic_source": source_id,
                "status": status,
                "coverage": coverage,
            }
        )

    manifest = {
        "source_page": "https://tamagotchi-official.com/gb/series/connection/howto/",
        "runtime_boundary": (
            "Official PNG/GIF files are analysis-only references. Firmware "
            "resources remain C/C++ bitmap arrays and composed frame tables."
        ),
        "coverage_note": (
            "The cached official how-to assets provide behavior and semantic "
            "scene references for souvenir-like memories, but they do not form "
            "a complete 64-row official souvenir atlas. Only unambiguous object "
            "rows are converted into official-source C++ overrides."
        ),
        "generated": {
            "candidate_contact_sheet": str(candidate_sheet.relative_to(ROOT)),
            "source_semantic_status_contact_sheet": str(status_sheet.relative_to(ROOT)),
            "official_source_override_contact_sheet": (
                str(override_sheet.relative_to(ROOT)) if override_sheet else None
            ),
            "alignment_proof": str(alignment.relative_to(ROOT)),
        },
        "official_source_override_count": len(override_records),
        "official_source_overrides": override_records,
        "assets": asset_records,
        "rows": rows,
    }
    out = GENERATED / "souvenir_official_reference_manifest.json"
    out.write_text(json.dumps(manifest, indent=2), encoding="utf-8")
    return out


def main() -> None:
    GENERATED.mkdir(parents=True, exist_ok=True)
    missing = sorted(
        {
            filename
            for info in SOUVENIR_SEMANTIC_SOURCES.values()
            for filename in info["files"]
            if not (RAW / filename).exists()
        }
    )
    if missing:
        raise SystemExit(f"missing official souvenir semantic assets: {', '.join(missing)}")

    entries = load_souvenir_rows()
    override_records = apply_official_overrides(entries)
    replace_souvenir_block(entries)

    candidate_sheet, asset_records = save_candidate_sheet()
    override_sheet = save_official_override_preview(override_records)
    status_sheet = save_per_memory_semantic_sheet(entries, override_records)
    alignment = save_alignment_proof(candidate_sheet, status_sheet, override_sheet)
    manifest = write_manifest(
        asset_records,
        override_records,
        candidate_sheet,
        status_sheet,
        override_sheet,
        alignment,
    )

    print(f"updated {SOURCE}")
    print(f"wrote {candidate_sheet}")
    print(f"wrote {status_sheet}")
    if override_sheet is not None:
        print(f"wrote {override_sheet}")
    print(f"wrote {alignment}")
    print(f"wrote {manifest}")


if __name__ == "__main__":
    main()


