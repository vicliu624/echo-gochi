#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import pathlib
import re
import struct
import sys
import zlib
from dataclasses import dataclass
from typing import Iterable


ROOT = pathlib.Path(__file__).resolve().parents[2]
SOURCE_ROOT = ROOT / "src" if (ROOT / "src").exists() else ROOT
OUT_DIR = ROOT / "analysis" / "screen_simulator" / "out"
REPORT_PATH = ROOT / "analysis" / "visual_alignment" / "visual_alignment_report.md"


SCENARIOS = (
    "home",
    "health",
    "food_menu",
    "meal",
    "activity_menu",
    "shop",
    "item",
    "game_get",
    "game_flag",
    "friends",
    "family",
    "souvenirs",
    "toilet",
    "medicine",
    "lights",
    "sprite_proof",
    "setup",
)


PROFILES = {
    "compact": {
        "base": (128, 64),
        "left": (0, 0, 32, 64),
        "main": (32, 0, 64, 64),
        "right": (96, 0, 32, 64),
    },
    "large": {
        "base": (192, 176),
        "left": (0, 0, 32, 176),
        "top": (32, 0, 128, 24),
        "main": (32, 24, 128, 128),
        "bottom": (32, 152, 128, 24),
        "right": (160, 0, 32, 176),
    },
}


@dataclass
class PngImage:
    width: int
    height: int
    pixels: bytes
    channels: int

    def is_black(self, x: int, y: int) -> bool:
        offset = (y * self.width + x) * self.channels
        sample = self.pixels[offset : offset + self.channels]
        return bool(sample) and sample[0] < 128 and sample[1] < 128 and sample[2] < 128

    def black_ratio(self, region: tuple[int, int, int, int]) -> float:
        x, y, w, h = region
        if w <= 0 or h <= 0:
            return 0.0
        total = w * h
        black = 0
        for yy in range(y, y + h):
            for xx in range(x, x + w):
                if self.is_black(xx, yy):
                    black += 1
        return black / total


@dataclass
class Check:
    name: str
    status: str
    detail: str


def read_png(path: pathlib.Path) -> PngImage:
    data = path.read_bytes()
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        raise ValueError(f"{path} is not a PNG")

    offset = 8
    width = height = bit_depth = color_type = None
    idat = bytearray()
    while offset < len(data):
        length = struct.unpack(">I", data[offset : offset + 4])[0]
        chunk_type = data[offset + 4 : offset + 8]
        chunk = data[offset + 8 : offset + 8 + length]
        offset += 12 + length
        if chunk_type == b"IHDR":
            width, height, bit_depth, color_type, _, _, _ = struct.unpack(
                ">IIBBBBB", chunk
            )
        elif chunk_type == b"IDAT":
            idat.extend(chunk)
        elif chunk_type == b"IEND":
            break

    if width is None or height is None or bit_depth is None or color_type is None:
        raise ValueError(f"{path} has no IHDR")
    if bit_depth != 8 or color_type not in (0, 2, 6):
        raise ValueError(
            f"{path} uses unsupported PNG format bit_depth={bit_depth} "
            f"color_type={color_type}"
        )

    source_channels = {0: 1, 2: 3, 6: 4}[color_type]
    stride = width * source_channels
    raw = zlib.decompress(bytes(idat))
    previous = bytearray(stride)
    rgb = bytearray(width * height * 3)
    raw_offset = 0
    out_offset = 0
    for _ in range(height):
        filter_type = raw[raw_offset]
        raw_offset += 1
        scan = bytearray(raw[raw_offset : raw_offset + stride])
        raw_offset += stride
        recon = bytearray(stride)
        for i, value in enumerate(scan):
            left = recon[i - source_channels] if i >= source_channels else 0
            up = previous[i]
            upper_left = previous[i - source_channels] if i >= source_channels else 0
            if filter_type == 0:
                recon[i] = value
            elif filter_type == 1:
                recon[i] = (value + left) & 0xFF
            elif filter_type == 2:
                recon[i] = (value + up) & 0xFF
            elif filter_type == 3:
                recon[i] = (value + ((left + up) // 2)) & 0xFF
            elif filter_type == 4:
                recon[i] = (value + paeth(left, up, upper_left)) & 0xFF
            else:
                raise ValueError(f"{path} uses unsupported PNG filter {filter_type}")
        previous = recon
        for x in range(width):
            pixel = recon[x * source_channels : (x + 1) * source_channels]
            if color_type == 0:
                rgb[out_offset : out_offset + 3] = pixel * 3
            else:
                rgb[out_offset : out_offset + 3] = pixel[:3]
            out_offset += 3
    return PngImage(width, height, bytes(rgb), 3)


def paeth(a: int, b: int, c: int) -> int:
    p = a + b - c
    pa = abs(p - a)
    pb = abs(p - b)
    pc = abs(p - c)
    if pa <= pb and pa <= pc:
        return a
    if pb <= pc:
        return b
    return c


def latest_png(pattern: str) -> pathlib.Path | None:
    matches = sorted(OUT_DIR.glob(pattern), key=lambda p: p.stat().st_mtime)
    return matches[-1] if matches else None


def scale_region(region: tuple[int, int, int, int], scale: int) -> tuple[int, int, int, int]:
    x, y, w, h = region
    return x * scale, y * scale, w * scale, h * scale


def check_preview(profile: str, scenario: str) -> list[Check]:
    result: list[Check] = []
    path = latest_png(f"{profile}_{scenario}_x*.png")
    prefix = f"{profile}/{scenario}"
    if path is None:
        return [Check(prefix, "FAIL", "missing simulator preview")]
    try:
        image = read_png(path)
    except Exception as exc:  # pragma: no cover - diagnostic path
        return [Check(prefix, "FAIL", f"could not read PNG: {exc}")]

    base_w, base_h = PROFILES[profile]["base"]
    if image.width % base_w != 0 or image.height % base_h != 0:
        result.append(
            Check(
                f"{prefix} dimensions",
                "FAIL",
                f"{path.name} is {image.width}x{image.height}, not a scaled "
                f"{base_w}x{base_h} frame",
            )
        )
        return result

    scale_x = image.width // base_w
    scale_y = image.height // base_h
    if scale_x != scale_y:
        result.append(
            Check(
                f"{prefix} scale",
                "FAIL",
                f"non-uniform scale {scale_x}x{scale_y}",
            )
        )
        return result

    scale = scale_x
    result.append(
        Check(
            f"{prefix} dimensions",
            "PASS",
            f"{path.name} matches {base_w}x{base_h} at x{scale}",
        )
    )

    for region_name in ("left", "main", "right"):
        region = scale_region(PROFILES[profile][region_name], scale)
        ratio = image.black_ratio(region)
        if region_name == "main":
            ok = 0.003 <= ratio <= 0.65
        else:
            ok = 0.006 <= ratio <= 0.75
        result.append(
            Check(
                f"{prefix} {region_name} ink",
                "PASS" if ok else "FAIL",
                f"black pixel ratio {ratio:.3f}",
            )
        )

    if profile == "large":
        for region_name in ("top", "bottom"):
            region = scale_region(PROFILES[profile][region_name], scale)
            ratio = image.black_ratio(region)
            ok = ratio <= 0.45
            result.append(
                Check(
                    f"{prefix} {region_name} band density",
                    "PASS" if ok else "FAIL",
                    f"black pixel ratio {ratio:.3f}; should remain a single utility line",
                )
            )
    return result


def count_runtime_bitmap_families() -> dict[str, int]:
    source = (SOURCE_ROOT / "EchoPetCatalogVisuals.cpp").read_text(encoding="utf-8")
    character_source = (SOURCE_ROOT / "EchoPetCharacterVisuals.cpp").read_text(encoding="utf-8")
    catalog_block = re.search(
        r"kCharacterCatalogBitmaps\[.*?\] PROGMEM = \{(.*?)\n\};",
        character_source,
        re.S,
    )
    idle_block = re.search(
        r"kCharacterIdleBitmaps\[.*?\] PROGMEM = \{(.*?)\n\};",
        character_source,
        re.S,
    )
    return {
        "food": len(set(re.findall(r"const uint16_t (kFood[A-Za-z0-9_]+)\[16\]", source))),
        "item": len(set(re.findall(r"const uint16_t (kItem[A-Za-z0-9_]+)\[16\]", source))),
        "catalog_entry": len(re.findall(r"\{\s*// \d{3}\n", source)),
        "souvenir": len(set(re.findall(r"const uint16_t (kSouvenir[A-Za-z0-9_]+)\[16\]", source))),
        "souvenir_memory": len(re.findall(r"\{\s*// \d{2} [A-Z0-9 ]+", source)),
        "character": len(set(re.findall(r"const uint16_t (kCharacter[A-Za-z0-9_]+)\[16\]", character_source))),
        "character_catalog": len(re.findall(r"\{\s*// \d{2} [A-Za-z0-9]+",
                                             catalog_block.group(1) if catalog_block else "")),
        "character_idle": len(re.findall(r"\{\s*// \d{2} [A-Za-z0-9]+",
                                          idle_block.group(1) if idle_block else "")),
    }


def source_checks() -> list[Check]:
    checks: list[Check] = []
    catalog_visual_source = (SOURCE_ROOT / "EchoPetCatalogVisuals.cpp").read_text(
        encoding="utf-8"
    )
    runtime_report = ROOT / "analysis" / "runtime_resource_audit" / "runtime_resource_audit.md"
    runtime_text = runtime_report.read_text(encoding="utf-8") if runtime_report.exists() else ""
    checks.append(
        Check(
            "runtime resource route",
            "PASS" if "Overall status: `PASS`" in runtime_text else "FAIL",
            "runtime audit must reject PNG/JPG/GIF/BMP as firmware resources",
        )
    )

    sprite_audit = ROOT / "analysis" / "sprite_render_audit" / "SPRITE_RENDER_AUDIT.md"
    sprite_text = sprite_audit.read_text(encoding="utf-8") if sprite_audit.exists() else ""
    checks.append(
        Check(
            "sprite composed-frame proof",
            "PASS"
            if (
                "missing `connectframe()` mappings: 0" in sprite_text.lower()
                and "missing composed frame definitions: 0" in sprite_text.lower()
                and "frames with pixels outside their nominal frame: 0"
                in sprite_text.lower()
            )
            else "OPEN",
            "host audit should prove every SpriteFrame enum renders from source tables",
        )
    )

    families = count_runtime_bitmap_families()
    checks.append(
        Check(
            "catalog food visual basis",
            "PASS" if families["food"] >= 10 else "OPEN",
            f"{families['food']} source bitmap families; enough for current food screens, not final official art",
        )
    )
    icon_resources = (SOURCE_ROOT / "EchoPetMenuIconResources.cpp").read_text(
        encoding="utf-8")
    display = (SOURCE_ROOT / "EchoPetDisplay.cpp").read_text(encoding="utf-8")
    dual_size_icons = (
        "kEchoPetMenuIconSemantics" in icon_resources
        and "kEchoPetMenuIcons128Large" in icon_resources
        and "kEchoPetMenuIcons64Compact" in icon_resources
        and "Health Meter" in icon_resources
        and "Connection/Communication" in icon_resources
        and "drawPackedBitmap(display, kEchoPetMenuIcons64Compact[i], 12, 12"
        in display
        and "drawPackedBitmap(display, kEchoPetMenuIcons128Large[i], 30, 30"
        in display
    )
    checks.append(
        Check(
            "fixed menu icon dual-size semantic route",
            "PASS" if dual_size_icons else "FAIL",
            "compact 12x12 and large 30x30 icons must both be C/C++ source bitmaps selected by the same menu action order",
        )
    )
    official_ref = ROOT / "analysis" / "official_reference"
    official_alignment = (
        (official_ref / "official_sources_manifest.json").exists()
        and (official_ref / "generated" /
             "fixed_menu_official_alignment_proof.png").exists()
        and (official_ref / "generated" /
             "fixed_menu_official_tiles_contact_sheet.png").exists()
        and (official_ref / "generated" /
             "fixed_menu_compact_official_source_contact_sheet.png").exists()
        and (official_ref / "generated" /
             "fixed_menu_source_policy_contact_sheet.png").exists()
        and (official_ref / "generated" /
             "fixed_menu_source_policy.json").exists()
        and (official_ref / "generated" /
             "fixed_menu_official_tiles_manifest.json").exists()
        and (official_ref / "raw" / "manuals" /
             "bandai_connection_2024_instruction_manual.pdf").exists()
    )
    checks.append(
        Check(
            "fixed menu official reference proof",
            "PASS" if official_alignment else "OPEN",
            "analysis-only official manual references, per-slot fixed-menu tile crops, unified 12x12/30x30 source-policy proof, and current EchoPet source-icon proof must exist",
        )
    )
    official_source_generation = (
        (official_ref / "generate_menu_icon_source_arrays.py").exists()
        and (official_ref / "generated" /
             "fixed_menu_generated_source_contact_sheet.png").exists()
        and (official_ref / "generated" /
             "fixed_menu_compact_official_source_contact_sheet.png").exists()
        and (official_ref / "generated" /
             "fixed_menu_source_policy_contact_sheet.png").exists()
        and (official_ref / "generated" /
             "fixed_menu_source_policy.json").exists()
        and "Health Meter" in icon_resources
        and "Games/Activity" in icon_resources
    )
    checks.append(
        Check(
            "fixed menu official-tile source generation",
            "PASS" if official_source_generation else "OPEN",
            "current fixed-menu C/C++ arrays must be generated from analysis-only official device tile crops, with compact and large rows sharing the same slot source and previewed in source contact sheets",
        )
    )
    fixed_menu_alignment_manifest = official_ref / "generated" / (
        "fixed_menu_official_alignment_manifest.json"
    )
    fixed_menu_aligned = False
    fixed_menu_detail = (
        "ten-slot icon set is source-backed and present on both rails; final "
        "human acceptance for Connection-like recognizability remains open"
    )
    if fixed_menu_alignment_manifest.exists():
        try:
            fixed_menu_manifest = json.loads(
                fixed_menu_alignment_manifest.read_text(encoding="utf-8")
            )
            fixed_menu_slots = fixed_menu_manifest.get("slots", [])
            fixed_menu_aligned = (
                fixed_menu_manifest.get("status") == "PASS"
                and len(fixed_menu_slots) == 10
                and all(
                    slot.get("local") and slot.get("official")
                    for slot in fixed_menu_slots
                )
                and official_source_generation
                and dual_size_icons
            )
            if fixed_menu_aligned:
                fixed_menu_detail = (
                    "ten fixed-menu slots have official manual semantics, "
                    "analysis-only official tile crops, shared 12x12/30x30 "
                    "source policy, and runtime C/C++ bitmap rows"
                )
        except Exception as exc:  # pragma: no cover - diagnostic path
            fixed_menu_detail = f"fixed menu alignment manifest unreadable: {exc}"
    checks.append(
        Check(
            "catalog item visual basis",
            "PASS" if families["item"] >= 32 else "OPEN",
            f"{families['item']} source bitmap families for 160 catalog rows; "
            "replacement hooks exist and major item classes route to C++ bitmap families",
        )
    )
    catalog_item_manifest_path = (
        official_ref / "generated" / "catalog_item_official_reference_manifest.json"
    )
    catalog_item_reference = (
        (official_ref / "generate_catalog_item_reference_sheets.py").exists()
        and (official_ref / "generated" /
             "catalog_item_official_assets_contact_sheet.png").exists()
        and (official_ref / "generated" /
             "catalog_item_official_pc_gif_frames.png").exists()
        and (official_ref / "generated" /
             "catalog_item_official_detected_tiles_contact_sheet.png").exists()
        and (official_ref / "generated" /
             "catalog_item_official_source_overrides_contact_sheet.png").exists()
        and (official_ref / "generated" /
             "catalog_item_source_semantic_status_contact_sheet.png").exists()
        and (official_ref / "generated" /
             "catalog_item_official_alignment_proof.png").exists()
        and catalog_item_manifest_path.exists()
    )
    catalog_item_detail = (
        "official item how-to assets are separated under "
        "analysis/official_reference, with explicit source-override and "
        "152-row semantic-status proof compared against the current C/C++ "
        "catalog item proof"
    )
    try:
        catalog_item_manifest = (
            json.loads(catalog_item_manifest_path.read_text(encoding="utf-8"))
            if catalog_item_manifest_path.exists()
            else {}
        )
        candidate_tiles = int(catalog_item_manifest.get("detected_lcd_tile_count", 0))
        override_rows = int(catalog_item_manifest.get("official_source_override_count", 0))
        catalog_item_reference = (
            catalog_item_reference and candidate_tiles >= 50 and override_rows == 12
        )
        if catalog_item_reference:
            catalog_item_detail = (
                "official item how-to assets are separated under "
                "analysis/official_reference; all PC-GIF frames are scanned into "
                f"{candidate_tiles} deduplicated LCD tile candidates, 12 "
                "unambiguous C++ source overrides stay mapped, and 152-row "
                "semantic status is compared against the current catalog proof"
            )
    except Exception as exc:  # pragma: no cover - diagnostic path
        catalog_item_reference = False
        catalog_item_detail = f"catalog item official manifest unreadable: {exc}"
    checks.append(
        Check(
            "catalog item official reference proof",
            "PASS" if catalog_item_reference else "OPEN",
            catalog_item_detail,
        )
    )
    display_source = (SOURCE_ROOT / "EchoPetDisplay.cpp").read_text(encoding="utf-8")
    catalog_entry_source_table = (
        families["catalog_entry"] >= 160
        and (ROOT / "analysis" / "screen_simulator" /
             "generate_catalog_entry_source_arrays.py").exists()
        and (OUT_DIR / "catalog_entry_generated_source_contact_sheet.png").exists()
        and "drawCatalogEntryBitmap(display, x, y, catalogIndex" in display_source
    )
    checks.append(
        Check(
            "catalog item per-row source table",
            "PASS" if catalog_entry_source_table else "OPEN",
            f"{families['catalog_entry']} explicit C/C++ catalog-entry bitmap rows; catalog previews route by catalog index instead of only the shared icon family",
        )
    )
    drawing_spec_dir = ROOT / "analysis" / "resource_drawing_spec"
    catalog_drawing_spec = drawing_spec_dir / "catalog_art_drawing_spec.json"
    souvenir_drawing_spec = drawing_spec_dir / "souvenir_art_drawing_spec.json"
    drawing_spec_report = drawing_spec_dir / "resource_drawing_spec.md"
    drawing_spec_ok = False
    drawing_spec_detail = (
        "row-local drawing specs for 160 catalog rows and 64 souvenir rows "
        "must define the object, semantic family, runtime symbol, and C++ "
        "bitmap acceptance rule"
    )
    try:
        catalog_spec = (
            json.loads(catalog_drawing_spec.read_text(encoding="utf-8"))
            if catalog_drawing_spec.exists()
            else {}
        )
        souvenir_spec = (
            json.loads(souvenir_drawing_spec.read_text(encoding="utf-8"))
            if souvenir_drawing_spec.exists()
            else {}
        )
        drawing_spec_ok = (
            drawing_spec_report.exists()
            and catalog_spec.get("row_count") == 160
            and souvenir_spec.get("row_count") == 64
            and catalog_spec.get("runtime_row_match") is True
            and souvenir_spec.get("runtime_row_match") is True
            and catalog_spec.get("replacement_state_counts", {}).get(
                "needs_final_art"
            )
            == 148
            and souvenir_spec.get("replacement_state_counts", {}).get(
                "needs_final_art"
            )
            == 62
        )
        if drawing_spec_ok:
            drawing_spec_detail = (
                "resource drawing spec defines 160 catalog and 64 souvenir "
                "row-local briefs, runtime bitmap symbols, preservation rules, "
                "and final-art acceptance checks without moving raster assets "
                "into firmware"
            )
    except Exception as exc:  # pragma: no cover - diagnostic path
        drawing_spec_detail = f"resource drawing spec unreadable: {exc}"
    checks.append(
        Check(
            "catalog/souvenir row-local drawing spec",
            "PASS" if drawing_spec_ok else "OPEN",
            drawing_spec_detail,
        )
    )
    checks.append(
        Check(
            "catalog item final per-row art",
            "OPEN",
            "160 catalog rows now have explicit source rows, 12 non-souvenir "
            "rows are official-source overrides, and the official item PC GIF "
            "is available as a deduplicated candidate tile atlas; most rows "
            "still come from the previous low-resource proof rather than "
            "accepted official-like item art",
        )
    )
    checks.append(
        Check(
            "souvenir visual basis",
            "PASS" if families["souvenir"] >= 16 else "OPEN",
            f"{families['souvenir']} source bitmap families remain as fallback/base memory silhouettes",
        )
    )
    checks.append(
        Check(
            "souvenir per-memory visual basis",
            "PASS" if families["souvenir_memory"] >= 64 else "OPEN",
            f"{families['souvenir_memory']} source memory bitmap rows; souvenir screens route by memory index",
        )
    )
    souvenir_reference = (
        (official_ref / "generate_souvenir_reference_sheets.py").exists()
        and (official_ref / "generated" /
             "souvenir_official_candidate_assets_contact_sheet.png").exists()
        and (official_ref / "generated" /
             "souvenir_source_semantic_status_contact_sheet.png").exists()
        and (official_ref / "generated" /
             "souvenir_official_source_overrides_contact_sheet.png").exists()
        and (official_ref / "generated" /
             "souvenir_official_alignment_proof.png").exists()
        and (official_ref / "generated" /
             "souvenir_official_reference_manifest.json").exists()
        and "Generated souvenir memory bitmaps are maintained by"
        in catalog_visual_source
    )
    checks.append(
        Check(
            "souvenir official semantic reference proof",
            "PASS" if souvenir_reference else "OPEN",
            "official how-to scene/icon candidates, current C++ 64-row status, and official-source souvenir overrides must be isolated under analysis/official_reference",
        )
    )
    checks.append(
        Check(
            "souvenir official-look acceptance",
            "OPEN",
            "64 memory rows now have source-backed C++ bitmaps, official semantic reference status, and two unambiguous official-source object overrides; final human acceptance against Connection-like souvenir recognizability remains open",
        )
    )
    checks.append(
        Check(
            "character visual family basis",
            "PASS" if families["character"] >= 9 else "OPEN",
            f"{families['character']} source bitmap families remain as fallback silhouettes",
        )
    )
    checks.append(
        Check(
            "character catalog per-row visual basis",
            "PASS" if families["character_catalog"] >= 50 else "OPEN",
            f"{families['character_catalog']} source 16x16 catalog portrait rows; friend/family/catalog portraits route by catalog id",
        )
    )
    checks.append(
        Check(
            "character idle per-row visual basis",
            "PASS" if families["character_idle"] >= 50 else "OPEN",
            f"{families['character_idle']} source 24x24 idle rows; main-scene portraits route by catalog id",
        )
    )
    character_reference = (
        (official_ref / "generate_character_reference_sheets.py").exists()
        and (official_ref / "generated" /
             "character_official_reference_contact_sheet.png").exists()
        and (official_ref / "generated" /
             "character_source_alignment_status_contact_sheet.png").exists()
        and (official_ref / "generated" /
             "character_source_alignment_status.json").exists()
        and (official_ref / "generated" /
             "character_official_alignment_proof.png").exists()
        and (official_ref / "generated" /
             "character_official_reference_manifest.json").exists()
    )
    checks.append(
        Check(
            "character official reference proof",
            "PASS" if character_reference else "OPEN",
            "official 50-row character references are isolated under analysis/official_reference and compared row-locally against the generated 16x16 and 24x24 C/C++ character rows",
        )
    )
    character_source = (SOURCE_ROOT / "EchoPetCharacterVisuals.cpp").read_text(
        encoding="utf-8")
    character_source_generated = (
        (official_ref / "generate_character_source_arrays.py").exists()
        and (official_ref / "generated" /
             "character_generated_source_contact_sheet.png").exists()
        and (official_ref / "generated" /
             "character_idle_generated_source_contact_sheet.png").exists()
        and (official_ref / "generated" /
             "character_source_alignment_status_contact_sheet.png").exists()
        and (official_ref / "generated" /
             "character_source_alignment_status.json").exists()
        and "Generated by analysis/official_reference/generate_character_source_arrays.py"
        in character_source
    )
    checks.append(
        Check(
            "character official-derived source arrays",
            "PASS" if character_source_generated else "OPEN",
            "50 runtime character rows are C/C++ 16x16 portrait and 24x24 idle arrays generated from analysis-only official references, with a row-local official/source status sheet; firmware does not load image files",
        )
    )
    character_status_path = (
        official_ref / "generated" / "character_source_alignment_status.json"
    )
    character_rows_source_accepted = False
    character_acceptance_detail = (
        "50 catalog rows now use official-derived C/C++ bitmap rows; final "
        "acceptance for 16x16 portrait detail loss, 24x24 idle recognizability, "
        "and hardware readability remains open"
    )
    if character_status_path.exists():
        try:
            character_status = json.loads(
                character_status_path.read_text(encoding="utf-8")
            )
            character_rows = character_status.get("characters", [])
            character_rows_source_accepted = (
                character_source_generated
                and len(character_rows) == 50
                and all(
                    row.get("status") == "official-derived"
                    and not row.get("flags")
                    for row in character_rows
                )
            )
            if character_rows_source_accepted:
                character_acceptance_detail = (
                    "50 character rows are official-derived C/C++ 16x16 "
                    "portrait and 24x24 idle bitmaps with zero review flags; "
                    "hardware readability remains under the hardware proof row"
                )
        except Exception as exc:  # pragma: no cover - diagnostic path
            character_acceptance_detail = (
                f"character source alignment status unreadable: {exc}"
            )
    checks.append(
        Check(
            "character roster official-look acceptance",
            "PASS" if character_rows_source_accepted else "OPEN",
            character_acceptance_detail,
        )
    )
    checks.append(
        Check(
            "fixed menu icon recognizability",
            "PASS" if fixed_menu_aligned else "OPEN",
            fixed_menu_detail,
        )
    )
    checks.append(
        Check(
            "hardware visual proof",
            "HW",
            "simulator cannot prove whitening, ghosting, partial refresh cadence, or physical key feel",
        )
    )
    return checks


def proof_checks() -> list[Check]:
    checks: list[Check] = []
    for name in (
        "catalog_items_proof",
        "catalog_souvenirs_proof",
        "character_roster_proof",
        "fixed_menu_icons_proof",
    ):
        path = latest_png(f"{name}_x*.png")
        if path is None:
            checks.append(Check(name, "FAIL", "missing proof sheet"))
            continue
        try:
            image = read_png(path)
        except Exception as exc:  # pragma: no cover - diagnostic path
            checks.append(Check(name, "FAIL", f"could not read PNG: {exc}"))
            continue
        ratio = image.black_ratio((0, 0, image.width, image.height))
        checks.append(
            Check(
                name,
                "PASS" if 0.003 <= ratio <= 0.65 else "FAIL",
                f"{path.name} {image.width}x{image.height}, black pixel ratio {ratio:.3f}",
            )
        )
    return checks


def summarize(checks: Iterable[Check]) -> str:
    statuses = {check.status for check in checks}
    if "FAIL" in statuses:
        return "FAIL"
    if "OPEN" in statuses or "HW" in statuses:
        return "OPEN"
    return "PASS"


def render_report(checks: list[Check]) -> str:
    summary = summarize(checks)
    counts = {status: sum(1 for check in checks if check.status == status) for status in ("PASS", "OPEN", "HW", "FAIL")}
    lines = [
        "# EchoPet Visual Alignment Gate Report",
        "",
        "Generated by `analysis/visual_alignment/generate_visual_alignment_gate.py`.",
        "",
        "## Summary",
        "",
        f"- Overall visual gate status: `{summary}`.",
        f"- PASS checks: `{counts['PASS']}`.",
        f"- OPEN checks: `{counts['OPEN']}`.",
        f"- HW checks: `{counts['HW']}`.",
        f"- FAIL checks: `{counts['FAIL']}`.",
        "",
        "Interpretation:",
        "",
        "- `PASS`: source-side layout/resource evidence is present.",
        "- `OPEN`: current implementation is visible but not final visual alignment.",
        "- `HW`: cannot be closed without T-Echo-Lite observation.",
        "- `FAIL`: generated previews or source/resource gates are broken.",
        "",
        "## Checks",
        "",
        "| Check | Status | Detail |",
        "| --- | --- | --- |",
    ]
    for check in checks:
        detail = check.detail.replace("|", "\\|")
        lines.append(f"| {check.name} | `{check.status}` | {detail} |")

    lines.extend(
        [
            "",
            "## Current Visual Gate Meaning",
            "",
            "The current simulator output is useful as a layout proof, but the project is",
            "not allowed to claim official-grade visual parity while the catalog item,",
            "souvenir, and hardware proof",
            "rows remain `OPEN` or `HW`.",
            "",
            "Runtime resources remain C/C++ source data. Preview PNGs and proof sheets",
            "are generated artifacts under `analysis/` only.",
            "",
        ]
    )
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Generate EchoPet visual alignment gate report."
    )
    parser.add_argument("--fail-on-open", action="store_true")
    args = parser.parse_args()

    checks: list[Check] = []
    for profile in PROFILES:
        for scenario in SCENARIOS:
            checks.extend(check_preview(profile, scenario))
    checks.extend(proof_checks())
    checks.extend(source_checks())

    REPORT_PATH.parent.mkdir(parents=True, exist_ok=True)
    REPORT_PATH.write_text(render_report(checks), encoding="utf-8")
    summary = summarize(checks)
    print(f"wrote {REPORT_PATH}")
    print(f"visual gate status: {summary}")
    if summary == "FAIL":
        return 1
    if args.fail_on_open and summary != "PASS":
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main())

