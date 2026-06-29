from __future__ import annotations

import binascii
import csv
import math
import re
import struct
import zlib
from dataclasses import dataclass
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
SOURCE_ROOT = ROOT / "src" if (ROOT / "src").exists() else ROOT
OUT_DIR = ROOT / "analysis" / "sprite_render_audit"
SPRITES_H = SOURCE_ROOT / "EchoPetSprites.h"
SPRITES_CPP = SOURCE_ROOT / "EchoPetSprites.cpp"
DISPLAY_CPP = SOURCE_ROOT / "EchoPetDisplay.cpp"

AUDIT_MD = OUT_DIR / "SPRITE_RENDER_AUDIT.md"
METRICS_CSV = OUT_DIR / "sprite_frame_metrics.csv"
CONTACT_SHEET = OUT_DIR / "sprite_contact_sheet.png"
MAMETCHI_MOTION = OUT_DIR / "sprite_mametchi_motion.png"
EGG_HATCH = OUT_DIR / "sprite_egg_hatch.png"
CARE_SCENES = OUT_DIR / "sprite_care_scenes.png"
GAME_SCENES = OUT_DIR / "sprite_game_scenes.png"

WHITE = (255, 255, 255)
BLACK = (0, 0, 0)
GRID = (216, 216, 216)
LIGHT = (238, 238, 238)
WARN = (255, 220, 220)


@dataclass(frozen=True)
class Asset:
    name: str
    width: int
    height: int
    stride: int
    data: list[int]


@dataclass(frozen=True)
class Part:
    x: int
    y: int
    asset_id: str
    flags: int


@dataclass(frozen=True)
class Frame:
    name: str
    width: int
    height: int
    parts_id: str
    parts: list[Part]


@dataclass
class RenderedFrame:
    sprite: str
    label: str
    frame_name: str
    width: int
    height: int
    parts: list[Part]
    pixels: set[tuple[int, int]]
    bbox: tuple[int, int, int, int] | None
    out_of_nominal_bounds: bool


def read(path: Path) -> str:
    return path.read_text(encoding="utf-8")


def strip_line_comments(text: str) -> str:
    return re.sub(r"//.*", "", text)


def parse_int(raw: str) -> int:
    return int(raw.strip(), 0)


def parse_enum_names(text: str, enum_name: str) -> list[str]:
    match = re.search(rf"enum(?:\s+class)?\s+{enum_name}[^{{]*\{{(.*?)\}};", text, re.S)
    if not match:
        return []
    names: list[str] = []
    value = 0
    for raw_line in match.group(1).splitlines():
        line = raw_line.split("//", 1)[0].strip().rstrip(",")
        if not line:
            continue
        token = line.split("=", 1)[0].strip()
        if not token:
            continue
        if "=" in line:
            value = parse_int(line.split("=", 1)[1])
        names.append(token)
        value += 1
    return names


def parse_byte_arrays(text: str) -> dict[str, list[int]]:
    arrays: dict[str, list[int]] = {}
    pattern = re.compile(
        r"const\s+uint8_t\s+(k\w+)\[\]\s+PROGMEM\s*=\s*\{(.*?)\};",
        re.S,
    )
    for name, body in pattern.findall(text):
        clean = strip_line_comments(body)
        values = [parse_int(token) for token in re.findall(r"0x[0-9A-Fa-f]+|\b\d+\b", clean)]
        arrays[name] = values
    return arrays


def between(text: str, start: str, end: str) -> str:
    start_index = text.find(start)
    if start_index < 0:
        return ""
    end_index = text.find(end, start_index + len(start))
    if end_index < 0:
        return text[start_index:]
    return text[start_index:end_index]


def parse_bitmap_assets(text: str, asset_ids: list[str], arrays: dict[str, list[int]]) -> dict[str, Asset]:
    match = re.search(r"const\s+BitmapAsset\s+kBitmapAssets\[\]\s*=\s*\{(.*?)\};", text, re.S)
    if not match:
        return {}
    entries = re.findall(r"\{\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*,\s*(k\w+)\s*\}", match.group(1))
    assets: dict[str, Asset] = {}
    for index, (width, height, stride, data_name) in enumerate(entries):
        if index >= len(asset_ids):
            continue
        assets[asset_ids[index]] = Asset(
            name=data_name,
            width=int(width),
            height=int(height),
            stride=int(stride),
            data=arrays.get(data_name, []),
        )
    return assets


def parse_flags(raw: str) -> int:
    raw = raw.strip()
    if raw in {"", "0"}:
        return 0
    flags = 0
    if "kPartFlipX" in raw:
        flags |= 0x01
    numbers = re.findall(r"0x[0-9A-Fa-f]+|\b\d+\b", raw)
    for number in numbers:
        flags |= int(number, 0)
    return flags


def parse_frame_parts(text: str) -> dict[str, list[Part]]:
    parts_by_id: dict[str, list[Part]] = {}
    pattern = re.compile(
        r"const\s+FramePart\s+(k\w+Parts)\[\]\s+PROGMEM\s*=\s*\{(.*?)\};",
        re.S,
    )
    for name, body in pattern.findall(text):
        clean = strip_line_comments(body)
        entries = re.findall(
            r"\{\s*(-?\d+)\s*,\s*(-?\d+)\s*,\s*(B_\w+)\s*,\s*([^{}]+?)\s*\}",
            clean,
        )
        parts_by_id[name] = [
            Part(x=int(x), y=int(y), asset_id=asset_id, flags=parse_flags(flags))
            for x, y, asset_id, flags in entries
        ]
    return parts_by_id


def parse_composed_frames(text: str, parts_by_id: dict[str, list[Part]]) -> dict[str, Frame]:
    frames: dict[str, Frame] = {}
    pattern = re.compile(
        r"const\s+ComposedFrame\s+(k\w+)\s*=\s*\{\s*(\d+)\s*,\s*(\d+)\s*,\s*(k\w+Parts)\s*,",
        re.S,
    )
    for name, width, height, parts_id in pattern.findall(text):
        frames[name] = Frame(
            name=name,
            width=int(width),
            height=int(height),
            parts_id=parts_id,
            parts=parts_by_id.get(parts_id, []),
        )
    return frames


def parse_connect_frame_map(text: str) -> dict[str, str]:
    block = between(text, "const ComposedFrame* connectFrame", "const Cell* frameCells")
    mapping: dict[str, str] = {}
    pending_cases: list[str] = []
    for line in block.splitlines():
        for case in re.findall(r"case\s+SpriteFrame::(k\w+)\s*:", line):
            pending_cases.append(case)
        match = re.search(r"return\s+&(k\w+)\s*;", line)
        if match:
            frame_name = match.group(1)
            for case in pending_cases:
                mapping[case] = frame_name
            pending_cases.clear()
    return mapping


def parse_proof_labels(text: str) -> dict[str, str]:
    block = between(text, "const char* spriteProofLabel", "SpriteFrame spriteProofFrame")
    labels: dict[str, str] = {}
    pending_cases: list[str] = []
    for line in block.splitlines():
        for case in re.findall(r"case\s+SpriteFrame::(k\w+)\s*:", line):
            pending_cases.append(case)
        match = re.search(r'return\s+"([^"]*)"\s*;', line)
        if match:
            label = match.group(1)
            for case in pending_cases:
                labels[case] = label
            pending_cases.clear()
    return labels


def asset_pixel(asset: Asset, x: int, y: int, flags: int) -> bool:
    source_x = asset.width - 1 - x if flags & 0x01 else x
    index = y * asset.stride + source_x // 8
    if index < 0 or index >= len(asset.data):
        return False
    bits = asset.data[index]
    return (bits & (0x80 >> (source_x & 0x07))) != 0


def render_frame(sprite: str, frame: Frame, assets: dict[str, Asset], label: str) -> RenderedFrame:
    pixels: set[tuple[int, int]] = set()
    for part in frame.parts:
        asset = assets.get(part.asset_id)
        if not asset:
            continue
        for y in range(asset.height):
            for x in range(asset.width):
                if asset_pixel(asset, x, y, part.flags):
                    pixels.add((part.x + x, part.y + y))

    if pixels:
        xs = [x for x, _ in pixels]
        ys = [y for _, y in pixels]
        bbox = (min(xs), min(ys), max(xs), max(ys))
        out_of_nominal = bbox[0] < 0 or bbox[1] < 0 or bbox[2] >= frame.width or bbox[3] >= frame.height
    else:
        bbox = None
        out_of_nominal = False

    return RenderedFrame(
        sprite=sprite,
        label=label,
        frame_name=frame.name,
        width=frame.width,
        height=frame.height,
        parts=frame.parts,
        pixels=pixels,
        bbox=bbox,
        out_of_nominal_bounds=out_of_nominal,
    )


def new_image(width: int, height: int, color: tuple[int, int, int] = WHITE) -> list[list[tuple[int, int, int]]]:
    return [[color for _ in range(width)] for _ in range(height)]


def set_pixel(image: list[list[tuple[int, int, int]]], x: int, y: int, color: tuple[int, int, int]) -> None:
    if 0 <= y < len(image) and 0 <= x < len(image[0]):
        image[y][x] = color


def fill_rect(
    image: list[list[tuple[int, int, int]]],
    x: int,
    y: int,
    width: int,
    height: int,
    color: tuple[int, int, int],
) -> None:
    for row in range(y, y + height):
        if row < 0 or row >= len(image):
            continue
        for col in range(x, x + width):
            if 0 <= col < len(image[0]):
                image[row][col] = color


def draw_rect(
    image: list[list[tuple[int, int, int]]],
    x: int,
    y: int,
    width: int,
    height: int,
    color: tuple[int, int, int],
) -> None:
    for col in range(x, x + width):
        set_pixel(image, col, y, color)
        set_pixel(image, col, y + height - 1, color)
    for row in range(y, y + height):
        set_pixel(image, x, row, color)
        set_pixel(image, x + width - 1, row, color)


FONT = {
    " ": ["000", "000", "000", "000", "000"],
    "-": ["000", "000", "111", "000", "000"],
    "/": ["001", "001", "010", "100", "100"],
    "0": ["111", "101", "101", "101", "111"],
    "1": ["010", "110", "010", "010", "111"],
    "2": ["111", "001", "111", "100", "111"],
    "3": ["111", "001", "111", "001", "111"],
    "4": ["101", "101", "111", "001", "001"],
    "5": ["111", "100", "111", "001", "111"],
    "6": ["111", "100", "111", "101", "111"],
    "7": ["111", "001", "010", "010", "010"],
    "8": ["111", "101", "111", "101", "111"],
    "9": ["111", "101", "111", "001", "111"],
    "A": ["010", "101", "111", "101", "101"],
    "B": ["110", "101", "110", "101", "110"],
    "C": ["111", "100", "100", "100", "111"],
    "D": ["110", "101", "101", "101", "110"],
    "E": ["111", "100", "110", "100", "111"],
    "F": ["111", "100", "110", "100", "100"],
    "G": ["111", "100", "101", "101", "111"],
    "H": ["101", "101", "111", "101", "101"],
    "I": ["111", "010", "010", "010", "111"],
    "J": ["001", "001", "001", "101", "111"],
    "K": ["101", "101", "110", "101", "101"],
    "L": ["100", "100", "100", "100", "111"],
    "M": ["101", "111", "111", "101", "101"],
    "N": ["101", "111", "111", "111", "101"],
    "O": ["111", "101", "101", "101", "111"],
    "P": ["111", "101", "111", "100", "100"],
    "Q": ["111", "101", "101", "111", "001"],
    "R": ["111", "101", "111", "110", "101"],
    "S": ["111", "100", "111", "001", "111"],
    "T": ["111", "010", "010", "010", "010"],
    "U": ["101", "101", "101", "101", "111"],
    "V": ["101", "101", "101", "101", "010"],
    "W": ["101", "101", "111", "111", "101"],
    "X": ["101", "101", "010", "101", "101"],
    "Y": ["101", "101", "010", "010", "010"],
    "Z": ["111", "001", "010", "100", "111"],
}


def draw_text(
    image: list[list[tuple[int, int, int]]],
    x: int,
    y: int,
    text: str,
    color: tuple[int, int, int] = BLACK,
    scale: int = 2,
) -> None:
    cursor = x
    for char in text.upper():
        glyph = FONT.get(char, FONT[" "])
        for gy, row in enumerate(glyph):
            for gx, bit in enumerate(row):
                if bit == "1":
                    fill_rect(image, cursor + gx * scale, y + gy * scale, scale, scale, color)
        cursor += 4 * scale


def draw_rendered_frame(
    image: list[list[tuple[int, int, int]]],
    rendered: RenderedFrame,
    x: int,
    y: int,
    scale: int,
    pad: int,
) -> None:
    nominal_x = x + pad * scale
    nominal_y = y + pad * scale
    fill_rect(
        image,
        nominal_x,
        nominal_y,
        rendered.width * scale,
        rendered.height * scale,
        WHITE,
    )
    draw_rect(
        image,
        nominal_x,
        nominal_y,
        rendered.width * scale + 1,
        rendered.height * scale + 1,
        GRID,
    )
    if rendered.out_of_nominal_bounds:
        draw_rect(
            image,
            nominal_x - 2,
            nominal_y - 2,
            rendered.width * scale + 5,
            rendered.height * scale + 5,
            (200, 0, 0),
        )
    for px, py in rendered.pixels:
        fill_rect(
            image,
            x + (pad + px) * scale,
            y + (pad + py) * scale,
            scale,
            scale,
            BLACK,
        )


def write_png(path: Path, image: list[list[tuple[int, int, int]]]) -> None:
    height = len(image)
    width = len(image[0]) if height else 0
    raw = bytearray()
    for row in image:
        raw.append(0)
        for r, g, b in row:
            raw.extend((r, g, b))

    def chunk(kind: bytes, data: bytes) -> bytes:
        return (
            struct.pack(">I", len(data))
            + kind
            + data
            + struct.pack(">I", binascii.crc32(kind + data) & 0xFFFFFFFF)
        )

    png = bytearray(b"\x89PNG\r\n\x1a\n")
    png.extend(chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0)))
    png.extend(chunk(b"IDAT", zlib.compress(bytes(raw), 9)))
    png.extend(chunk(b"IEND", b""))
    path.write_bytes(bytes(png))


def contact_sheet(path: Path, title: str, frames: list[RenderedFrame], columns: int = 6) -> None:
    if not frames:
        return
    scale = 3
    pad = 5
    cell_w = 132
    cell_h = 142
    rows = math.ceil(len(frames) / columns)
    title_h = 28
    image = new_image(columns * cell_w + 1, title_h + rows * cell_h + 1, WHITE)
    draw_text(image, 8, 8, title[:56], BLACK, 2)
    for index, rendered in enumerate(frames):
        row = index // columns
        col = index % columns
        x = col * cell_w
        y = title_h + row * cell_h
        fill_rect(image, x, y, cell_w, cell_h, WARN if rendered.out_of_nominal_bounds else WHITE)
        draw_rect(image, x, y, cell_w + 1, cell_h + 1, GRID)
        label = rendered.label or rendered.sprite.replace("k", "", 1)
        draw_text(image, x + 6, y + 6, label[:14], BLACK, 2)
        draw_text(image, x + 6, y + 22, rendered.frame_name.replace("kFrame", "")[:14], BLACK, 1)
        draw_rendered_frame(image, rendered, x + 14, y + 34, scale, pad)
    write_png(path, image)


def frame_summary(rendered: RenderedFrame) -> dict[str, str | int]:
    bbox = rendered.bbox
    if bbox:
        bbox_text = f"{bbox[0]},{bbox[1]},{bbox[2]},{bbox[3]}"
    else:
        bbox_text = ""
    part_ids = " ".join(part.asset_id for part in rendered.parts)
    return {
        "sprite": rendered.sprite,
        "label": rendered.label,
        "frame": rendered.frame_name,
        "width": rendered.width,
        "height": rendered.height,
        "part_count": len(rendered.parts),
        "bbox": bbox_text,
        "out_of_nominal_bounds": "yes" if rendered.out_of_nominal_bounds else "no",
        "part_assets": part_ids,
    }


def part_positions(frame: Frame, asset_id: str) -> list[tuple[int, int, int]]:
    return [(part.x, part.y, part.flags) for part in frame.parts if part.asset_id == asset_id]


def analyze(rendered_by_sprite: dict[str, RenderedFrame], frames: dict[str, Frame]) -> list[tuple[str, str, str]]:
    checks: list[tuple[str, str, str]] = []

    missing = [name for name, rendered in rendered_by_sprite.items() if not rendered.pixels]
    checks.append(("all mapped frames draw black pixels", "PASS" if not missing else "FAIL", ", ".join(missing)))

    out_of_bounds = [name for name, rendered in rendered_by_sprite.items() if rendered.out_of_nominal_bounds]
    checks.append(("all composed pixels stay inside nominal frame", "PASS" if not out_of_bounds else "WARN", ", ".join(out_of_bounds)))

    egg_failures: list[str] = []
    for sprite in ["kEgg0", "kEgg1", "kEggCrack0", "kEggCrack1", "kEggHatch"]:
        rendered = rendered_by_sprite.get(sprite)
        if not rendered:
            egg_failures.append(f"{sprite}:missing")
            continue
        face_parts = [
            part
            for part in rendered.parts
            if part.asset_id in {
                "B_EGG_EYE",
                "B_EGG_MOUTH_SMILE",
                "B_EGG_MOUTH_SAD",
                "B_EGG_MOUTH_OPEN",
                "B_EGG_CRACK",
            }
        ]
        for part in face_parts:
            if not (0 <= part.x <= 31 and 0 <= part.y <= 31):
                egg_failures.append(f"{sprite}:{part.asset_id}@{part.x},{part.y}")
    checks.append(("egg face/crack anchors are inside the 32x32 shell frame", "PASS" if not egg_failures else "FAIL", ", ".join(egg_failures)))

    stand0 = frames.get("kFrameStand0")
    stand1 = frames.get("kFrameStand1")
    if stand0 and stand1:
        adult_ok = (
            len(part_positions(stand0, "B_EYE_OPEN")) == 2
            and len(part_positions(stand0, "B_MOUTH_SMILE")) == 1
            and len(part_positions(stand0, "B_FOOT")) == 2
            and len(part_positions(stand1, "B_EYE_OPEN")) == 2
            and len(part_positions(stand1, "B_MOUTH_SMILE")) == 1
            and len(part_positions(stand1, "B_FOOT")) == 2
        )
        details = (
            f"stand0 feet={part_positions(stand0, 'B_FOOT')} "
            f"stand1 feet={part_positions(stand1, 'B_FOOT')}"
        )
        checks.append(("adult idle uses separate eyes, mouth, and mirrored feet", "PASS" if adult_ok else "FAIL", details))
        foot_motion = part_positions(stand0, "B_FOOT") != part_positions(stand1, "B_FOOT")
        checks.append(("adult idle foot anchors move between kAdult0 and kAdult1", "PASS" if foot_motion else "FAIL", details))
    else:
        checks.append(("adult idle composed frames exist", "FAIL", "kFrameStand0/kFrameStand1 missing"))

    return checks


def write_metrics(rows: list[dict[str, str | int]]) -> None:
    if not rows:
        return
    with METRICS_CSV.open("w", encoding="utf-8", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=list(rows[0].keys()))
        writer.writeheader()
        writer.writerows(rows)


def write_audit(
    sprite_names: list[str],
    rendered: list[RenderedFrame],
    missing_mapping: list[str],
    missing_composed: list[str],
    checks: list[tuple[str, str, str]],
) -> None:
    out_of_bounds = [frame.sprite for frame in rendered if frame.out_of_nominal_bounds]
    lines = [
        "# Firmware Sprite Render Audit",
        "",
        "This audit renders the firmware's current low-resource sprite parts from source.",
        "It is generated from `EchoPetSprites.cpp`, `EchoPetSprites.h`, and `EchoPetDisplay.cpp`.",
        "It does not copy or embed official Tamagotchi pixels.",
        "",
        "## Summary",
        "",
        f"- Sprite enum frames: {len(sprite_names)}",
        f"- Rendered composed frames: {len(rendered)}",
        f"- Missing `connectFrame()` mappings: {len(missing_mapping)}",
        f"- Missing composed frame definitions: {len(missing_composed)}",
        f"- Frames with pixels outside their nominal frame: {len(out_of_bounds)}",
        "",
        "## Generated Files",
        "",
        f"- `{CONTACT_SHEET.relative_to(ROOT)}`",
        f"- `{MAMETCHI_MOTION.relative_to(ROOT)}`",
        f"- `{EGG_HATCH.relative_to(ROOT)}`",
        f"- `{CARE_SCENES.relative_to(ROOT)}`",
        f"- `{GAME_SCENES.relative_to(ROOT)}`",
        f"- `{METRICS_CSV.relative_to(ROOT)}`",
        "",
        "## Structural Checks",
        "",
        "| Check | Status | Details |",
        "| --- | --- | --- |",
    ]
    for check, status, details in checks:
        lines.append(f"| {check} | {status} | {details or ''} |")
    lines.extend(["", "## Remaining Acceptance Boundary", ""])
    lines.extend(
        [
            "- These PNGs prove code-side sprite composition only.",
            "- Hardware proof is still required for e-paper refresh speed, whitening/ghosting, and button timing.",
            "- Official-look final art still requires original or rights-cleared replacement sprites.",
            "- If a user photo shows a visual bug, compare it first against this audit to decide whether the defect is source sprite composition or display refresh behavior.",
        ]
    )
    if missing_mapping:
        lines.extend(["", "## Missing Mapping", ""])
        lines.extend(f"- `{name}`" for name in missing_mapping)
    if missing_composed:
        lines.extend(["", "## Missing Composed Frame", ""])
        lines.extend(f"- `{name}`" for name in missing_composed)
    AUDIT_MD.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> None:
    OUT_DIR.mkdir(parents=True, exist_ok=True)

    header = read(SPRITES_H)
    sprites = read(SPRITES_CPP)
    display = read(DISPLAY_CPP)

    sprite_names = parse_enum_names(header, "SpriteFrame")
    asset_ids = parse_enum_names(sprites, "BitmapAssetId")
    arrays = parse_byte_arrays(sprites)
    assets = parse_bitmap_assets(sprites, asset_ids, arrays)
    parts_by_id = parse_frame_parts(sprites)
    frames = parse_composed_frames(sprites, parts_by_id)
    frame_map = parse_connect_frame_map(sprites)
    labels = parse_proof_labels(display)

    missing_mapping = [name for name in sprite_names if name not in frame_map]
    missing_composed = sorted({frame for frame in frame_map.values() if frame not in frames})

    rendered: list[RenderedFrame] = []
    rendered_by_sprite: dict[str, RenderedFrame] = {}
    for sprite in sprite_names:
        frame_name = frame_map.get(sprite)
        if not frame_name:
            continue
        frame = frames.get(frame_name)
        if not frame:
            continue
        item = render_frame(sprite, frame, assets, labels.get(sprite, sprite))
        rendered.append(item)
        rendered_by_sprite[sprite] = item

    contact_sheet(CONTACT_SHEET, "All Firmware Sprite Frames", rendered, columns=6)
    contact_sheet(
        MAMETCHI_MOTION,
        "Default Adult / Mametchi-Like Motion",
        [rendered_by_sprite[name] for name in [
            "kAdult0",
            "kAdult1",
            "kJoy",
            "kSad",
            "kSleep",
            "kSick",
            "kFriend",
            "kPassed",
        ] if name in rendered_by_sprite],
        columns=4,
    )
    contact_sheet(
        EGG_HATCH,
        "Egg / Crack / Hatch",
        [rendered_by_sprite[name] for name in [
            "kEgg0",
            "kEgg1",
            "kEggCrack0",
            "kEggCrack1",
            "kEggHatch",
        ] if name in rendered_by_sprite],
        columns=5,
    )
    care_names = [
        "kEatMeal0",
        "kEatMeal1",
        "kEatSnack0",
        "kEatSnack1",
        "kFoodCrumbs",
        "kFoodRefuse",
        "kFoodDone",
        "kToilet0",
        "kToilet1",
        "kToiletMess",
        "kToiletSweep0",
        "kToiletSweep1",
        "kToiletDone",
        "kMedicine0",
        "kMedicine1",
        "kMedicineSickSkull",
        "kMedicineSickTooth",
        "kMedicineDose0",
        "kMedicineDose1",
        "kMedicineRecover",
        "kMedicineRefuse",
        "kLightsOn",
        "kLightsOff",
        "kDisciplineTimeout",
        "kDisciplinePraise",
        "kAttentionCall",
    ]
    contact_sheet(
        CARE_SCENES,
        "Care / Food / Toilet / Medicine / Discipline",
        [rendered_by_sprite[name] for name in care_names if name in rendered_by_sprite],
        columns=6,
    )
    game_names = [name for name in sprite_names if name.startswith("kGame")]
    contact_sheet(
        GAME_SCENES,
        "Game Sprite Frames",
        [rendered_by_sprite[name] for name in game_names if name in rendered_by_sprite],
        columns=6,
    )

    rows = [frame_summary(item) for item in rendered]
    write_metrics(rows)
    checks = analyze(rendered_by_sprite, frames)
    write_audit(sprite_names, rendered, missing_mapping, missing_composed, checks)


if __name__ == "__main__":
    main()

