from __future__ import annotations

import json
from collections import deque
from pathlib import Path

from PIL import Image, ImageDraw


ROOT = Path(__file__).resolve().parents[2]
SOURCE_ROOT = ROOT / "src" if (ROOT / "src").exists() else ROOT
REF = ROOT / "analysis" / "official_reference"
GENERATED = REF / "generated"
TILE_DIR = GENERATED / "fixed_menu_reference_tiles"
MANIFEST = GENERATED / "fixed_menu_official_tiles_manifest.json"
OUT_CPP = SOURCE_ROOT / "EchoPetMenuIconResources.cpp"


def black_mask(image: Image.Image) -> list[list[bool]]:
    gray = image.convert("L")
    return [[gray.getpixel((x, y)) < 128 for x in range(gray.width)]
            for y in range(gray.height)]


def clean_mask(mask: list[list[bool]]) -> list[list[bool]]:
    h = len(mask)
    w = len(mask[0])
    seen = [[False for _ in range(w)] for _ in range(h)]
    out = [[False for _ in range(w)] for _ in range(h)]
    dirs = ((1, 0), (-1, 0), (0, 1), (0, -1))

    for y in range(h):
        for x in range(w):
            if not mask[y][x] or seen[y][x]:
                continue
            q: deque[tuple[int, int]] = deque([(x, y)])
            seen[y][x] = True
            pixels: list[tuple[int, int]] = []
            touches_edge = False
            while q:
                px, py = q.popleft()
                pixels.append((px, py))
                if px == 0 or py == 0 or px == w - 1 or py == h - 1:
                    touches_edge = True
                for dx, dy in dirs:
                    nx = px + dx
                    ny = py + dy
                    if 0 <= nx < w and 0 <= ny < h and mask[ny][nx] and not seen[ny][nx]:
                        seen[ny][nx] = True
                        q.append((nx, ny))
            if touches_edge or len(pixels) < 2:
                continue
            for px, py in pixels:
                out[py][px] = True
    return out


def bbox(mask: list[list[bool]]) -> tuple[int, int, int, int]:
    xs: list[int] = []
    ys: list[int] = []
    for y, row in enumerate(mask):
        for x, value in enumerate(row):
            if value:
                xs.append(x)
                ys.append(y)
    if not xs:
        return (0, 0, 1, 1)
    return (min(xs), min(ys), max(xs) + 1, max(ys) + 1)


def mask_to_image(mask: list[list[bool]]) -> Image.Image:
    h = len(mask)
    w = len(mask[0])
    image = Image.new("1", (w, h), 1)
    pixels = image.load()
    for y in range(h):
        for x in range(w):
            if mask[y][x]:
                pixels[x, y] = 0
    return image


def fit_source_to_canvas(source: Image.Image, target_side: int,
                         max_ink_side: int) -> Image.Image:
    mask = clean_mask(black_mask(source))
    left, top, right, bottom = bbox(mask)
    cropped = mask_to_image(mask).crop((left, top, right, bottom))
    cw, ch = cropped.size
    if cw <= 0 or ch <= 0:
        return Image.new("1", (target_side, target_side), 1)
    if cw >= ch:
        draw_w = max_ink_side
        draw_h = max(1, round(ch * max_ink_side / cw))
    else:
        draw_h = max_ink_side
        draw_w = max(1, round(cw * max_ink_side / ch))
    resized = cropped.resize((draw_w, draw_h), Image.Resampling.NEAREST)
    canvas = Image.new("1", (target_side, target_side), 1)
    canvas.paste(resized, ((target_side - draw_w) // 2,
                           (target_side - draw_h) // 2))
    return canvas


def pack_bitmap(image: Image.Image, width: int) -> list[int]:
    row_bytes = (width + 7) // 8
    gray = image.convert("L")
    values: list[int] = []
    for y in range(image.height):
        for byte_index in range(row_bytes):
            value = 0
            for bit in range(8):
                x = byte_index * 8 + bit
                if x < width and gray.getpixel((x, y)) < 128:
                    value |= 0x80 >> bit
            values.append(value)
    return values


def format_bytes(values: list[int], indent: str) -> str:
    lines: list[str] = []
    for index in range(0, len(values), 8):
        chunk = values[index:index + 8]
        suffix = "," if index + 8 < len(values) else ""
        lines.append(indent + ", ".join(f"0x{value:02X}" for value in chunk) + suffix)
    return "\n".join(lines)


def source_contact_sheet(slots: list[dict], compact: list[Image.Image],
                         large: list[Image.Image]) -> None:
    cell_w = 112
    sheet = Image.new("RGB", (cell_w * len(slots), 250), "white")
    draw = ImageDraw.Draw(sheet)
    for i, slot in enumerate(slots):
        x = i * cell_w
        draw.rectangle((x, 0, x + cell_w - 1, 249), outline="black")
        draw.text((x + 4, 4), slot["local_id"], fill="black")
        draw.text((x + 4, 18), slot["official_label"][:16], fill="black")
        c = compact[i].resize((60, 60), Image.Resampling.NEAREST).convert("RGB")
        l = large[i].resize((90, 90), Image.Resampling.NEAREST).convert("RGB")
        sheet.paste(c, (x + 26, 42))
        sheet.paste(l, (x + 11, 142))
    out = GENERATED / "fixed_menu_generated_source_contact_sheet.png"
    sheet.save(out)


def compact_source_contact_sheet(slots: list[dict],
                                 compact: list[Image.Image]) -> None:
    cell_w = 112
    sheet = Image.new("RGB", (cell_w * len(slots), 134), "white")
    draw = ImageDraw.Draw(sheet)
    draw.text(
        (4, 4),
        "COMPACT FIXED MENU OFFICIAL-SOURCE 24x24 ROWS",
        fill="black",
    )
    for i, slot in enumerate(slots):
        x = i * cell_w
        draw.rectangle((x, 24, x + cell_w - 1, 133), outline="black")
        draw.text((x + 4, 28), slot["local_id"], fill="black")
        draw.text((x + 4, 42), "device tile", fill="black")
        icon = compact[i].resize((60, 60), Image.Resampling.NEAREST).convert("RGB")
        sheet.paste(icon, (x + 26, 66))
    out = GENERATED / "fixed_menu_compact_official_source_contact_sheet.png"
    sheet.save(out)


def source_policy_contact_sheet(slots: list[dict], compact: list[Image.Image],
                                large: list[Image.Image]) -> None:
    cell_w = 124
    sheet = Image.new("RGB", (cell_w * len(slots), 210), "white")
    draw = ImageDraw.Draw(sheet)
    draw.text(
        (4, 4),
        "FIXED MENU SOURCE POLICY: 24x24 AND 30x30 SHARE DEVICE-TILE SOURCE",
        fill="black",
    )
    policy_rows: list[dict[str, object]] = []
    for i, slot in enumerate(slots):
        x = i * cell_w
        draw.rectangle((x, 24, x + cell_w - 1, 209), outline="black")
        draw.text((x + 4, 28), slot["local_id"], fill="black")
        draw.text((x + 4, 42), "compact: device", fill="black")
        draw.text((x + 4, 56), "large: device", fill="black")
        compact_icon = compact[i].resize((54, 54), Image.Resampling.NEAREST).convert("RGB")
        large_icon = large[i].resize((72, 72), Image.Resampling.NEAREST).convert("RGB")
        sheet.paste(compact_icon, (x + 8, 86))
        sheet.paste(large_icon, (x + 46, 118))
        draw.text((x + 8, 144), "24", fill="black")
        draw.text((x + 46, 194), "30", fill="black")
        policy_rows.append(
            {
                "index": int(slot["index"]),
                "local_id": slot["local_id"],
                "official_label": slot["official_label"],
                "compact_source": "official_device_tile",
                "large_source": "official_device_tile",
                "shared_source": True,
                "runtime_boundary": (
                    "Official raster tile is analysis-only. Firmware stores "
                    "the derived icon as C/C++ bitmap bytes."
                ),
            }
        )
    out = GENERATED / "fixed_menu_source_policy_contact_sheet.png"
    out_json = GENERATED / "fixed_menu_source_policy.json"
    sheet.save(out)
    out_json.write_text(
        json.dumps(
            {
                "source_manifest": str(MANIFEST.relative_to(REF)),
                "runtime_boundary": (
                    "Official fixed-menu references are analysis-only. Runtime "
                    "icons are C/C++ byte arrays in EchoPetMenuIconResources.cpp."
                ),
                "policy": (
                    "The EchoPet side rails model the external fixed menu icon "
                    "semantics, so compact 24x24 and large 30x30 icons are "
                    "derived from the same official device/menu tile per slot."
                ),
                "slots": policy_rows,
            },
            indent=2,
        ),
        encoding="utf-8",
    )


def main() -> None:
    manifest = json.loads(MANIFEST.read_text(encoding="utf-8"))
    slots = manifest["slots"]
    compact_images: list[Image.Image] = []
    large_images: list[Image.Image] = []
    for slot in slots:
        source = Image.open(TILE_DIR / Path(slot["bw_tile"]).name).convert("RGB")
        compact_images.append(fit_source_to_canvas(source, 24, 22))
        large_images.append(fit_source_to_canvas(source, 30, 24))

    compact_bytes = [pack_bitmap(image, 24) for image in compact_images]
    large_bytes = [pack_bitmap(image, 30) for image in large_images]

    lines: list[str] = [
        '#include "EchoPetMenuIconResources.h"',
        "",
        "namespace echopet {",
        "",
        "const EchoPetMenuIconSemantic kEchoPetMenuIconSemantics[kEchoPetMenuIconCount] =",
        "    {",
    ]
    for slot in slots:
        lines.append(
            f'        {{"{slot["local_id"]}", "{slot["official_label"]}"}},')
    lines.extend([
        "};",
        "",
        "const uint8_t kEchoPetMenuIcons64Compact[kEchoPetMenuIconCount]",
        "                                        [kEchoPetMenuIconCompactBytes] = {",
    ])
    for slot, values in zip(slots, compact_bytes):
        lines.append(f"    {{// {slot['official_label']}")
        lines.append(format_bytes(values, "     "))
        lines.append("    },")
    lines.extend([
        "};",
        "",
        "const uint8_t kEchoPetMenuIcons128Large[kEchoPetMenuIconCount]",
        "                                       [kEchoPetMenuIconLargeBytes] = {",
    ])
    for slot, values in zip(slots, large_bytes):
        lines.append(f"    {{// {slot['official_label']}")
        lines.append(format_bytes(values, "     "))
        lines.append("    },")
    lines.extend([
        "};",
        "",
        "}  // namespace echopet",
    ])

    OUT_CPP.write_text("\n".join(lines) + "\n", encoding="utf-8")
    source_contact_sheet(slots, compact_images, large_images)
    compact_source_contact_sheet(slots, compact_images)
    source_policy_contact_sheet(slots, compact_images, large_images)
    print(f"wrote {OUT_CPP}")


if __name__ == "__main__":
    main()

