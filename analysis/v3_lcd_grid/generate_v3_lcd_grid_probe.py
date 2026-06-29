#!/usr/bin/env python3
from __future__ import annotations

import json
from pathlib import Path

from PIL import Image, ImageDraw, ImageSequence


ROOT = Path(__file__).resolve().parents[2]
SOURCE = (
    ROOT
    / "analysis"
    / "official_reference"
    / "raw"
    / "howto"
    / "images__howto__breed__img_breed_03.gif"
)
OUT = ROOT / "analysis" / "v3_lcd_grid"

CANDIDATE_GRIDS = ((32, 30), (64, 32), (64, 30), (48, 32), (48, 30))
SELECTED_FRAMES = (0, 1, 2, 3, 4, 5, 8, 10, 12, 16, 20, 24, 28, 32, 36, 40, 43)


def is_ink(pixel: tuple[int, int, int]) -> bool:
    r, g, b = pixel
    return r < 128 and g < 128 and b < 128


def extract_grid(frame: Image.Image, grid_w: int, grid_h: int) -> list[list[int]]:
    rgb = frame.convert("RGB")
    rows: list[list[int]] = []
    for grid_y in range(grid_h):
        y0 = round(grid_y * rgb.height / grid_h)
        y1 = max(y0 + 1, round((grid_y + 1) * rgb.height / grid_h))
        row: list[int] = []
        for grid_x in range(grid_w):
            x0 = round(grid_x * rgb.width / grid_w)
            x1 = max(x0 + 1, round((grid_x + 1) * rgb.width / grid_w))
            total = 0
            ink = 0
            for y in range(y0, y1):
                for x in range(x0, x1):
                    total += 1
                    if is_ink(rgb.getpixel((x, y))):
                        ink += 1
            row.append(1 if ink * 2 >= total else 0)
        rows.append(row)
    return rows


def upscale_to_source(rows: list[list[int]], width: int, height: int) -> Image.Image:
    grid_h = len(rows)
    grid_w = len(rows[0])
    image = Image.new("RGB", (width, height), "white")
    pixels = image.load()
    for y in range(height):
        grid_y = min(grid_h - 1, int(y * grid_h / height))
        for x in range(width):
            grid_x = min(grid_w - 1, int(x * grid_w / width))
            if rows[grid_y][grid_x]:
                pixels[x, y] = (0, 0, 0)
    return image


def mismatch_ratio(source: Image.Image, reconstructed: Image.Image) -> float:
    src = source.convert("RGB")
    rec = reconstructed.convert("RGB")
    mismatches = 0
    for y in range(src.height):
        for x in range(src.width):
            if is_ink(src.getpixel((x, y))) != is_ink(rec.getpixel((x, y))):
                mismatches += 1
    return mismatches / float(src.width * src.height)


def ink_bbox(rows: list[list[int]]) -> list[int] | None:
    points = [(x, y) for y, row in enumerate(rows) for x, value in enumerate(row) if value]
    if not points:
        return None
    xs = [x for x, _ in points]
    ys = [y for _, y in points]
    return [min(xs), min(ys), max(xs) + 1, max(ys) + 1]


def render_grid(rows: list[list[int]], scale: int, pad: int = 4) -> Image.Image:
    grid_h = len(rows)
    grid_w = len(rows[0])
    image = Image.new("RGB", (grid_w * scale + 2 * pad, grid_h * scale + 2 * pad), "white")
    draw = ImageDraw.Draw(image)
    for y, row in enumerate(rows):
        for x, value in enumerate(row):
            if value:
                draw.rectangle(
                    (
                        pad + x * scale,
                        pad + y * scale,
                        pad + (x + 1) * scale - 1,
                        pad + (y + 1) * scale - 1,
                    ),
                    fill="black",
                )
    draw.rectangle((pad - 1, pad - 1, pad + grid_w * scale, pad + grid_h * scale), outline=(220, 220, 220))
    return image


def write_contact_sheet(frames: list[Image.Image]) -> None:
    cell_w = 220
    cell_h = 260
    sheet = Image.new("RGB", (len(SELECTED_FRAMES) * cell_w, 2 * cell_h), "white")
    draw = ImageDraw.Draw(sheet)
    for col, frame_index in enumerate(SELECTED_FRAMES):
        frame = frames[frame_index]
        for row, (grid_w, grid_h, scale) in enumerate(((32, 30, 6), (64, 32, 3))):
            native = extract_grid(frame, grid_w, grid_h)
            tile = render_grid(native, scale=scale)
            x = col * cell_w + (cell_w - tile.width) // 2
            y = row * cell_h + 30
            sheet.paste(tile, (x, y))
            draw.text((col * cell_w + 4, row * cell_h + 4), f"f{frame_index} {grid_w}x{grid_h}", fill="black")
    sheet.save(OUT / "breed03_grid_candidates_contact.png")


def main() -> None:
    OUT.mkdir(parents=True, exist_ok=True)
    frames = [frame.copy().convert("RGB") for frame in ImageSequence.Iterator(Image.open(SOURCE))]
    metrics: dict[str, object] = {
        "source": str(SOURCE.relative_to(ROOT)),
        "source_size": list(frames[0].size),
        "frame_count": len(frames),
        "candidate_grids": {},
        "conclusion": (
            "32x30 is the native visible LCD-grid candidate for Connection/V3-style "
            "how-to animation frames; 64x32 is treated as a controller/other-model "
            "candidate, not the compact GAT562 playfield projection."
        ),
    }

    candidate_metrics: dict[str, object] = {}
    for grid_w, grid_h in CANDIDATE_GRIDS:
        errors: list[float] = []
        bboxes: list[list[int] | None] = []
        for frame in frames:
            rows = extract_grid(frame, grid_w, grid_h)
            reconstructed = upscale_to_source(rows, frame.width, frame.height)
            errors.append(mismatch_ratio(frame, reconstructed))
            bboxes.append(ink_bbox(rows))
        first_bbox = next((bbox for bbox in bboxes if bbox), None)
        candidate_metrics[f"{grid_w}x{grid_h}"] = {
            "avg_mismatch": sum(errors) / len(errors),
            "max_mismatch": max(errors),
            "first_nonblank_bbox": first_bbox,
        }

    metrics["candidate_grids"] = candidate_metrics
    (OUT / "breed03_grid_metrics.json").write_text(json.dumps(metrics, indent=2), encoding="utf-8")
    write_contact_sheet(frames)
    print(json.dumps(metrics, indent=2))


if __name__ == "__main__":
    main()
