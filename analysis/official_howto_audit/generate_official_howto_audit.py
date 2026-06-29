#!/usr/bin/env python3
"""Generate metadata-only audit data for the official Connection how-to page.

The official page is source evidence for player-visible feature groups and
animation presence, but its images are not redistributed by this repository.
This script fetches the images in memory, records dimensions/frame timing/motion
metrics, and writes only non-pixel metadata.
"""

from __future__ import annotations

import csv
import hashlib
import html.parser
import io
import json
import os
import pathlib
import re
import sys
import urllib.parse
import urllib.request
from dataclasses import dataclass
from typing import Iterable

from PIL import Image, ImageChops, ImageSequence


HOWTO_URL = "https://tamagotchi-official.com/gb/series/connection/howto/"
OUTPUT_DIR = pathlib.Path(__file__).resolve().parent
ROOT_DIR = OUTPUT_DIR.parents[1]


@dataclass
class ImageRef:
    src: str
    alt: str
    ordinal: int


class ImgParser(html.parser.HTMLParser):
    def __init__(self) -> None:
        super().__init__()
        self.images: list[ImageRef] = []
        self._ordinal = 0

    def handle_starttag(self, tag: str, attrs: list[tuple[str, str | None]]) -> None:
        if tag.lower() != "img":
            return
        attr = {key.lower(): value or "" for key, value in attrs}
        src = attr.get("src") or attr.get("data-src")
        if not src:
            return
        alt = attr.get("alt", "")
        self._ordinal += 1
        self.images.append(ImageRef(src=src, alt=alt, ordinal=self._ordinal))


def group_for_url(url: str) -> str:
    path = urllib.parse.urlparse(url).path.lower()
    if "/howto/breed/" in path or "/main/howto/breed/" in path:
        return "nurture"
    if "/howto/growth/" in path or "/main/howto/growth/" in path:
        return "growth"
    if "/howto/game/" in path or "/main/howto/game/" in path:
        return "game"
    if "/howto/item/" in path or "/main/howto/item/" in path:
        return "item_shop"
    if "/howto/special/" in path or "/main/howto/special/" in path:
        return "password_secret"
    if "/howto/connect/" in path or "/main/howto/connect/" in path:
        return "connect"
    if "/main/howto/" in path:
        return "section_title"
    return "chrome"


def contract_hint(url: str) -> str:
    name = pathlib.PurePosixPath(urllib.parse.urlparse(url).path).name.lower()
    path = urllib.parse.urlparse(url).path.lower()
    if "/breed/" in path:
        if "breed_01" in name:
            return "status/check menu visual"
        if "breed_02" in name:
            return "food.select/bite/crumbs"
        if "breed_03" in name:
            return "toilet.mess/sweep/done"
        if "breed_04" in name:
            return "discipline.prompt/react"
        if "breed_05" in name:
            return "medicine.sick/dose/recover"
        if "breed_06" in name:
            return "lights.selector/sleep/dark"
        return "nurture.menu.icon"
    if "/growth/" in path:
        if "chart" in name:
            return "character.route.chart"
        return "egg/baby/child/teen/adult/passaway"
    if "/game/" in path:
        mapping = {
            "game_01": "game.get.*",
            "game_02": "game.bump.*",
            "game_03": "game.flag.*",
            "game_04": "game.heading.*",
            "game_05": "game.memory.*",
            "game_06": "game.sprint.*",
            "game_07": "game.hoops.*",
        }
        for token, hint in mapping.items():
            if token in name:
                return hint
        return "game.menu/reward"
    if "/item/" in path:
        if "item_03" in name:
            return "shop.booth/keeper/item"
        return "item.use/collection"
    if "/special/" in path:
        if "special_02" in name:
            return "shop.secret-code.entry"
        return "password.reward.entry"
    if "/connect/" in path:
        mapping = {
            "connect_01": "connection.visit",
            "connect_02": "connection.present",
            "connect_03": "connection.game",
            "connect_04": "connection.love",
            "connect_05": "connection.next_generation",
            "connect_06": "connection.result",
            "connect_07": "connection.friendship",
        }
        for token, hint in mapping.items():
            if token in name:
                return hint
        return "connection.transport/menu"
    return ""


def fetch(url: str) -> bytes:
    request = urllib.request.Request(
        url,
        headers={
            "User-Agent": (
                "Mozilla/5.0 EchoPet metadata audit; no redistribution of images"
            )
        },
    )
    with urllib.request.urlopen(request, timeout=30) as response:
        return response.read()


def bbox_tuple(bbox: tuple[int, int, int, int] | None) -> list[int] | None:
    if bbox is None:
        return None
    return [int(v) for v in bbox]


def image_metrics(content: bytes) -> dict[str, object]:
    with Image.open(io.BytesIO(content)) as image:
        width, height = image.size
        frame_hashes: list[str] = []
        durations: list[int] = []
        diff_bboxes: list[list[int] | None] = []
        content_bboxes: list[list[int] | None] = []
        previous = None
        for frame in ImageSequence.Iterator(image):
            rgba = frame.convert("RGBA")
            frame_hashes.append(hashlib.sha1(rgba.tobytes()).hexdigest())
            durations.append(int(frame.info.get("duration", 0)))

            alpha = rgba.getchannel("A")
            content_bboxes.append(bbox_tuple(alpha.getbbox()))
            if previous is None:
                diff_bboxes.append(None)
            else:
                diff = ImageChops.difference(previous, rgba)
                diff_bboxes.append(bbox_tuple(diff.getbbox()))
            previous = rgba

        frame_count = len(frame_hashes) or 1
        duration_total = sum(durations)
        non_null_diffs = [bbox for bbox in diff_bboxes if bbox is not None]
        return {
            "format": image.format,
            "width": width,
            "height": height,
            "frame_count": frame_count,
            "unique_frame_count": len(set(frame_hashes)),
            "duration_ms": duration_total,
            "frame_durations_ms": durations,
            "content_bboxes": content_bboxes,
            "changed_bboxes": diff_bboxes,
            "changed_frame_count": len(non_null_diffs),
        }


def write_json(path: pathlib.Path, data: object) -> None:
    path.write_text(json.dumps(data, indent=2, ensure_ascii=False) + "\n",
                    encoding="utf-8")


def main() -> int:
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    page = fetch(HOWTO_URL).decode("utf-8", errors="replace")
    parser = ImgParser()
    parser.feed(page)

    seen: set[str] = set()
    records: list[dict[str, object]] = []
    for image_ref in parser.images:
        url = urllib.parse.urljoin(HOWTO_URL, image_ref.src)
        if url in seen:
            continue
        seen.add(url)
        record: dict[str, object] = {
            "ordinal": image_ref.ordinal,
            "url": url,
            "path": urllib.parse.urlparse(url).path,
            "file": pathlib.PurePosixPath(urllib.parse.urlparse(url).path).name,
            "group": group_for_url(url),
            "contract_hint": contract_hint(url),
            "alt": image_ref.alt,
        }
        try:
            content = fetch(url)
            record.update(image_metrics(content))
        except Exception as exc:  # keep the audit useful when one asset fails
            record["error"] = str(exc)
        records.append(record)

    records.sort(key=lambda item: int(item["ordinal"]))
    write_json(OUTPUT_DIR / "official_howto_asset_metrics.json", records)

    csv_path = OUTPUT_DIR / "summary.csv"
    with csv_path.open("w", encoding="utf-8", newline="") as handle:
        writer = csv.writer(handle)
        writer.writerow([
            "ordinal",
            "group",
            "file",
            "format",
            "size",
            "frames",
            "unique",
            "duration_ms",
            "changed_frames",
            "contract_hint",
            "url",
        ])
        for r in records:
            writer.writerow([
                r.get("ordinal", ""),
                r.get("group", ""),
                r.get("file", ""),
                r.get("format", ""),
                f"{r.get('width', '')}x{r.get('height', '')}",
                r.get("frame_count", ""),
                r.get("unique_frame_count", ""),
                r.get("duration_ms", ""),
                r.get("changed_frame_count", ""),
                r.get("contract_hint", ""),
                r.get("url", ""),
            ])

    md_lines = [
        "# Official How-To Visual Metadata Audit",
        "",
        f"Source: `{HOWTO_URL}`",
        "",
        "This audit intentionally stores metadata only. It does not copy, embed,",
        "or redistribute official image pixels. Metrics are used to define",
        "required original/right-cleared EchoPet frame families.",
        "",
        "## Summary",
        "",
        "| Group | Assets | Animated | Max frames | Total duration ms |",
        "| --- | ---: | ---: | ---: | ---: |",
    ]
    groups = sorted({str(r.get("group", "")) for r in records})
    for group in groups:
        group_records = [r for r in records if r.get("group") == group]
        animated = [r for r in group_records if int(r.get("frame_count", 1)) > 1]
        max_frames = max((int(r.get("frame_count", 1)) for r in group_records),
                         default=0)
        total_duration = sum(int(r.get("duration_ms", 0)) for r in group_records)
        md_lines.append(
            f"| `{group}` | {len(group_records)} | {len(animated)} | "
            f"{max_frames} | {total_duration} |"
        )

    md_lines.extend([
        "",
        "## Asset Rows",
        "",
        "| Group | File | Size | Frames | Duration | Contract hint |",
        "| --- | --- | --- | ---: | ---: | --- |",
    ])
    for r in records:
        size = f"{r.get('width', '?')}x{r.get('height', '?')}"
        md_lines.append(
            f"| `{r.get('group', '')}` | `{r.get('file', '')}` | {size} | "
            f"{r.get('frame_count', '')} | {r.get('duration_ms', '')} | "
            f"`{r.get('contract_hint', '')}` |"
        )

    md_lines.extend([
        "",
        "## Contract Consequences",
        "",
        "- Official how-to assets confirm that action screens need staged visuals,",
        "  not static text pages.",
        "- The firmware must keep official assets out of the binary unless a",
        "  rights-cleared asset pack is supplied.",
        "- Resource authors can use the `contract_hint`, frame count, and timing",
        "  metadata to draw replacement 1-bit sprites for 64x64 and 128x128",
        "  playfields.",
        "- Hardware acceptance still requires T-Echo-Lite photos/videos because",
        "  metadata does not prove e-paper readability, ghosting, or button pace.",
        "",
    ])
    (OUTPUT_DIR / "OFFICIAL_HOWTO_VISUAL_AUDIT.md").write_text(
        "\n".join(md_lines), encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

