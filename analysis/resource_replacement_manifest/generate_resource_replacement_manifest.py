#!/usr/bin/env python3
from __future__ import annotations

import csv
import json
import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
SRC = ROOT / "src"
OUT = ROOT / "analysis" / "resource_replacement_manifest"

CATALOG_CPP = SRC / "EchoPetCatalog.cpp"
CATALOG_H = SRC / "EchoPetCatalog.h"
CATALOG_VISUALS_CPP = SRC / "EchoPetCatalogVisuals.cpp"
CHARACTER_CATALOG_CPP = SRC / "EchoPetCharacterCatalog.cpp"
CHARACTER_CATALOG_H = SRC / "EchoPetCharacterCatalog.h"
CHARACTER_VISUALS_CPP = SRC / "EchoPetCharacterVisuals.cpp"
CATALOG_OFFICIAL = (
    ROOT
    / "analysis"
    / "official_reference"
    / "generated"
    / "catalog_item_official_reference_manifest.json"
)
SOUVENIR_OFFICIAL = (
    ROOT
    / "analysis"
    / "official_reference"
    / "generated"
    / "souvenir_official_reference_manifest.json"
)
CHARACTER_STATUS = (
    ROOT
    / "analysis"
    / "official_reference"
    / "generated"
    / "character_source_alignment_status.json"
)

CHARACTER_JSON = OUT / "character_replacement_manifest.json"
CHARACTER_CSV = OUT / "character_replacement_manifest.csv"
CATALOG_JSON = OUT / "catalog_replacement_manifest.json"
CATALOG_CSV = OUT / "catalog_replacement_manifest.csv"
SOUVENIR_JSON = OUT / "souvenir_replacement_manifest.json"
SOUVENIR_CSV = OUT / "souvenir_replacement_manifest.csv"
REPORT = OUT / "resource_replacement_manifest.md"


KIND_NAMES = {
    "CatalogKind::kFood": "food",
    "CatalogKind::kItem": "item",
    "CatalogKind::kSouvenir": "souvenir",
}

FLAG_NAMES = [
    (0x01, "snack"),
    (0x02, "travel"),
    (0x04, "adult_only"),
    (0x08, "reusable"),
    (0x10, "child_plus"),
    (0x20, "teen_plus"),
    (0x80, "secret"),
]

USE_SCENE_NAMES = {
    0: "none",
    1: "meal",
    2: "snack",
    3: "play",
    4: "study",
    5: "music",
    6: "travel",
    7: "memory",
}

STAGE_NAMES = {
    "Stage::kEgg": "egg",
    "Stage::kBaby": "baby",
    "Stage::kChild": "child",
    "Stage::kTeen": "teen",
    "Stage::kAdult": "adult",
    "Stage::kElder": "elder",
}

GROWTH_ROUTE_NAMES = {
    "GrowthRoute::kBalanced": "balanced",
    "GrowthRoute::kAthlete": "athlete",
    "GrowthRoute::kDreamer": "dreamer",
    "GrowthRoute::kRascal": "rascal",
    "GrowthRoute::kScholar": "scholar",
    "GrowthRoute::kSocial": "social",
}

CHARACTER_KIND_NAMES = {
    "CharacterKind::kBaby": "baby",
    "CharacterKind::kSprout": "sprout",
    "CharacterKind::kBuddy": "buddy",
    "CharacterKind::kQuill": "quill",
    "CharacterKind::kBolt": "bolt",
    "CharacterKind::kDream": "dream",
    "CharacterKind::kRascal": "rascal",
    "CharacterKind::kSage": "sage",
}

CHARACTER_GENERATION_BITS = [
    (0x01, "first"),
    (0x02, "odd"),
    (0x04, "even"),
]

CHARACTER_TIER_BITS = [
    (0x01, "one"),
    (0x02, "two"),
    (0x04, "three"),
    (0x08, "four"),
    (0x10, "special"),
]

CHARACTER_GENDER_BITS = [
    (0x01, "boy"),
    (0x02, "girl"),
]


def read(path: Path) -> str:
    return path.read_text(encoding="utf-8")


def load_json(path: Path) -> dict:
    return json.loads(read(path))


def find_brace_body(text: str, anchor: str) -> str:
    start = text.index(anchor)
    brace = text.index("{", start)
    depth = 0
    in_string = False
    escaped = False
    for i in range(brace, len(text)):
        ch = text[i]
        if in_string:
            if escaped:
                escaped = False
            elif ch == "\\":
                escaped = True
            elif ch == '"':
                in_string = False
            continue
        if ch == '"':
            in_string = True
            continue
        if ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                return text[brace + 1 : i]
    raise ValueError(f"could not find body for {anchor}")


def split_top_level(text: str, delimiter: str = ",") -> list[str]:
    parts: list[str] = []
    start = 0
    paren_depth = 0
    brace_depth = 0
    in_string = False
    escaped = False
    for i, ch in enumerate(text):
        if in_string:
            if escaped:
                escaped = False
            elif ch == "\\":
                escaped = True
            elif ch == '"':
                in_string = False
            continue
        if ch == '"':
            in_string = True
            continue
        if ch == "(":
            paren_depth += 1
        elif ch == ")":
            paren_depth -= 1
        elif ch == "{":
            brace_depth += 1
        elif ch == "}":
            brace_depth -= 1
        elif ch == delimiter and paren_depth == 0 and brace_depth == 0:
            value = text[start:i].strip()
            if value:
                parts.append(value)
            start = i + 1
    value = text[start:].strip()
    if value:
        parts.append(value)
    return parts


def split_initializer_entries(body: str) -> list[str]:
    entries: list[str] = []
    depth = 0
    start = -1
    in_string = False
    escaped = False
    for i, ch in enumerate(body):
        if in_string:
            if escaped:
                escaped = False
            elif ch == "\\":
                escaped = True
            elif ch == '"':
                in_string = False
            continue
        if ch == '"':
            in_string = True
            continue
        if ch == "{":
            if depth == 0:
                start = i + 1
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0 and start >= 0:
                entries.append(body[start:i].strip())
                start = -1
    return entries


def normalize_expr(expr: str) -> str:
    result = expr.strip()
    result = re.sub(r"static_cast<[^>]+>", "", result)
    result = re.sub(r"(?<=\d)U\b", "", result)
    result = result.replace("\n", " ")
    result = re.sub(r"\s+", " ", result)
    return result.strip()


def load_constants() -> dict[str, int]:
    constants: dict[str, int] = {}
    text = (
        read(CATALOG_H)
        + "\n"
        + read(CHARACTER_CATALOG_H)
        + "\n"
        + read(CATALOG_CPP)
        + "\n"
        + read(CHARACTER_CATALOG_CPP)
    )
    for match in re.finditer(r"constexpr\s+uint\d+_t\s+(\w+)\s*=\s*([^;]+);", text):
        name, expr = match.groups()
        try:
            constants[name] = int(eval(normalize_expr(expr), {}, constants))
        except Exception:
            continue
    return constants


def eval_expr(expr: str, constants: dict[str, int]) -> int | None:
    value = normalize_expr(expr)
    try:
        return int(eval(value, {}, constants))
    except Exception:
        return None


def string_value(expr: str) -> str:
    value = expr.strip()
    if len(value) >= 2 and value[0] == '"' and value[-1] == '"':
        return value[1:-1]
    return value


def kind_value(expr: str) -> str:
    return KIND_NAMES.get(expr.strip(), expr.strip())


def decode_flags(value: int | None) -> list[str]:
    if value is None:
        return []
    return [name for bit, name in FLAG_NAMES if value & bit]


def decode_bits(value: int | None, bits: list[tuple[int, str]]) -> list[str]:
    if value is None:
        return []
    return [name for bit, name in bits if value & bit]


def parse_segments(text: str, constants: dict[str, int]) -> list[dict]:
    body = find_brace_body(text, "kCatalogSegments[]")
    rows: list[dict] = []
    for entry in split_initializer_entries(body):
        parts = split_top_level(entry)
        if len(parts) != 13:
            raise ValueError(f"unexpected catalog segment field count: {entry}")
        rows.append(
            {
                "start": eval_expr(parts[0], constants),
                "end": eval_expr(parts[1], constants),
                "prefix": string_value(parts[2]),
                "kind": kind_value(parts[3]),
                "behavior_base": eval_expr(parts[4], constants),
                "behavior_modulo": eval_expr(parts[5], constants),
                "base_price": eval_expr(parts[6], constants),
                "price_base": eval_expr(parts[7], constants),
                "price_modulo": eval_expr(parts[8], constants),
                "price_step": eval_expr(parts[9], constants),
                "flags": eval_expr(parts[10], constants),
                "source_group": eval_expr(parts[11], constants),
                "use_scene": eval_expr(parts[12], constants),
            }
        )
    return rows


def parse_overrides(text: str, constants: dict[str, int]) -> dict[int, dict]:
    body = find_brace_body(text, "kCatalogOverrides[]")
    result: dict[int, dict] = {}
    for entry in split_initializer_entries(body):
        parts = split_top_level(entry)
        if len(parts) != 9:
            raise ValueError(f"unexpected catalog override field count: {entry}")
        index = eval_expr(parts[0], constants)
        if index is None:
            raise ValueError(f"override has no numeric index: {entry}")
        result[index] = {
            "index": index,
            "label": string_value(parts[1]),
            "kind": kind_value(parts[2]),
            "behavior": eval_expr(parts[3], constants),
            "price": eval_expr(parts[4], constants),
            "icon": eval_expr(parts[5], constants),
            "flags": eval_expr(parts[6], constants),
            "source_group": eval_expr(parts[7], constants),
            "use_scene": eval_expr(parts[8], constants),
            "entry_source": "override",
        }
    return result


def parse_item_icon_switch(text: str, constants: dict[str, int]) -> dict[int, int]:
    body = find_brace_body(text, "switch (index)")
    mapping: dict[int, int] = {}
    pending: list[int] = []
    for line in body.splitlines():
        stripped = line.strip()
        case = re.match(r"case\s+(\d+):", stripped)
        if case:
            pending.append(int(case.group(1)))
            continue
        ret = re.match(r"return\s+([^;]+);", stripped)
        if ret and pending:
            value = eval_expr(ret.group(1), constants)
            if value is not None:
                for index in pending:
                    mapping[index] = value
            pending = []
    return mapping


def parse_souvenir_names(text: str) -> list[str]:
    body = find_brace_body(text, "kSouvenirNames[kCatalogSouvenirCount]")
    return [string_value(part) for part in split_top_level(body)]


def parse_character_rows(text: str, constants: dict[str, int]) -> list[dict]:
    body = find_brace_body(text, "kCharacters[kCharacterCatalogCount]")
    rows: list[dict] = []
    pattern = re.compile(r"CHARACTER_ROW(?:_TRAITS)?\(")
    pos = 0
    while True:
        match = pattern.search(body, pos)
        if not match:
            break
        macro = (
            "CHARACTER_ROW_TRAITS"
            if match.group(0).startswith("CHARACTER_ROW_TRAITS")
            else "CHARACTER_ROW"
        )
        open_paren = match.end() - 1
        depth = 0
        in_string = False
        escaped = False
        end = open_paren
        for i in range(open_paren, len(body)):
            ch = body[i]
            if in_string:
                if escaped:
                    escaped = False
                elif ch == "\\":
                    escaped = True
                elif ch == '"':
                    in_string = False
                continue
            if ch == '"':
                in_string = True
                continue
            if ch == "(":
                depth += 1
            elif ch == ")":
                depth -= 1
                if depth == 0:
                    end = i
                    break
        args = split_top_level(body[open_paren + 1 : end])
        if macro == "CHARACTER_ROW" and len(args) != 11:
            raise ValueError(f"unexpected CHARACTER_ROW field count: {args}")
        if macro == "CHARACTER_ROW_TRAITS" and len(args) != 12:
            raise ValueError(
                f"unexpected CHARACTER_ROW_TRAITS field count: {args}"
            )
        care_band = eval_expr(args[4], constants)
        source_page = eval_expr(args[8], constants)
        source_slot = eval_expr(args[9], constants)
        if macro == "CHARACTER_ROW_TRAITS":
            visual_traits = eval_expr(args[11], constants)
        else:
            visual_traits = (
                0x80
                | (((source_page or 0) & 0x07) << 5)
                | (((source_slot or 0) & 0x07) << 2)
                | ((care_band or 0) & 0x03)
            )
        generation_mask = eval_expr(args[5], constants)
        tier_mask = eval_expr(args[6], constants)
        gender_mask = eval_expr(args[7], constants)
        rows.append(
            {
                "index": len(rows),
                "label": string_value(args[0]),
                "stage": STAGE_NAMES.get(args[1].strip(), args[1].strip()),
                "route": GROWTH_ROUTE_NAMES.get(
                    args[2].strip(), args[2].strip()
                ),
                "archetype": CHARACTER_KIND_NAMES.get(
                    args[3].strip(), args[3].strip()
                ),
                "care_band": care_band,
                "generation_mask": generation_mask,
                "generation_tags": decode_bits(
                    generation_mask, CHARACTER_GENERATION_BITS
                ),
                "tier_mask": tier_mask,
                "tier_tags": decode_bits(tier_mask, CHARACTER_TIER_BITS),
                "gender_mask": gender_mask,
                "gender_tags": decode_bits(gender_mask, CHARACTER_GENDER_BITS),
                "source_page": source_page,
                "source_slot": source_slot,
                "frame_family": eval_expr(args[10], constants),
                "visual_traits": visual_traits,
                "entry_source": macro.lower(),
            }
        )
        pos = end + 1
    return rows


def catalog_segment_for(index: int, segments: list[dict]) -> dict | None:
    for segment in segments:
        if segment["start"] <= index < segment["end"]:
            return segment
    return None


def build_catalog_entry(
    index: int,
    segments: list[dict],
    overrides: dict[int, dict],
    icon_switch: dict[int, int],
) -> dict:
    if index in overrides:
        entry = dict(overrides[index])
    else:
        segment = catalog_segment_for(index, segments)
        if not segment:
            entry = {
                "index": index,
                "label": "ITEM---",
                "kind": "item",
                "behavior": 0,
                "price": 100,
                "icon": 0,
                "flags": 0,
                "source_group": 0,
                "use_scene": 0,
                "entry_source": "default",
            }
        else:
            behavior = (index - segment["behavior_base"]) % segment["behavior_modulo"]
            price_step_index = (
                (index - segment["price_base"]) % segment["price_modulo"]
                if segment["price_modulo"]
                else 0
            )
            entry = {
                "index": index,
                "label": f"{segment['prefix']}{index + 1:03d}",
                "kind": segment["kind"],
                "behavior": behavior,
                "price": segment["base_price"] + price_step_index * segment["price_step"],
                "icon": behavior,
                "flags": segment["flags"],
                "source_group": segment["source_group"],
                "use_scene": segment["use_scene"],
                "entry_source": "segment",
            }
    if entry["kind"] == "item" and index in icon_switch:
        entry["icon"] = icon_switch[index]
    return entry


def icon_name_maps(constants: dict[str, int]) -> dict[int, str]:
    names: dict[int, str] = {}
    for name, value in constants.items():
        if name.startswith("kCatalogItemIcon"):
            names[value] = name.replace("kCatalogItemIcon", "")
        elif name.startswith("kCatalogFoodIcon"):
            names[value] = name.replace("kCatalogFoodIcon", "")
    return names


def count_bitmap_entries(text: str, anchor: str) -> int:
    body = find_brace_body(text, anchor)
    return len(split_initializer_entries(body))


def build_catalog_manifest() -> dict:
    catalog_text = read(CATALOG_CPP)
    visuals_text = read(CATALOG_VISUALS_CPP)
    constants = load_constants()
    segments = parse_segments(catalog_text, constants)
    overrides = parse_overrides(catalog_text, constants)
    icon_switch = parse_item_icon_switch(catalog_text, constants)
    icons = icon_name_maps(constants)
    official = load_json(CATALOG_OFFICIAL)
    row_status = {
        int(row["index"]): row for row in official.get("catalog_row_status", [])
    }
    official_override = {
        int(row["catalog_index"]): row
        for row in official.get("official_source_overrides", [])
    }
    runtime_rows = count_bitmap_entries(visuals_text, "kCatalogEntryBitmaps")

    rows: list[dict] = []
    for index in range(constants["kCatalogItemCount"]):
        entry = build_catalog_entry(index, segments, overrides, icon_switch)
        status_row = row_status.get(index, {})
        official_row = official_override.get(index)
        art_status = (
            "official_source_override"
            if official_row
            else status_row.get("status", "generated runtime row")
        )
        row = {
            "index": index,
            "label": entry["label"],
            "kind": entry["kind"],
            "price": entry["price"],
            "behavior": entry["behavior"],
            "icon": entry["icon"],
            "icon_name": icons.get(entry["icon"], f"icon_{entry['icon']}"),
            "flags": entry["flags"],
            "flag_names": decode_flags(entry["flags"]),
            "source_group": entry["source_group"],
            "use_scene": entry["use_scene"],
            "use_scene_name": USE_SCENE_NAMES.get(entry["use_scene"], "unknown"),
            "visual_traits": index,
            "runtime_bitmap_symbol": f"kCatalogEntryBitmaps[{index}]",
            "runtime_dimensions_px": [16, 16],
            "runtime_encoding": "16 uint16_t rows, high bit is leftmost pixel",
            "entry_source": entry["entry_source"],
            "art_status": art_status,
            "official_status": status_row.get("status", ""),
            "official_source": status_row.get("source", ""),
            "official_coverage": (
                official_row.get("label", "") if official_row else status_row.get("coverage", "")
            ),
            "replacement_contract": (
                "Replace only this 16x16 C++ row in EchoPetCatalogVisuals.cpp "
                "or regenerate it with the manifest row id preserved. Do not "
                "add runtime PNG/JPG/GIF loading."
            ),
        }
        rows.append(row)

    counts: dict[str, int] = {}
    for row in rows:
        counts[row["art_status"]] = counts.get(row["art_status"], 0) + 1

    return {
        "generated_by": "analysis/resource_replacement_manifest/generate_resource_replacement_manifest.py",
        "runtime_boundary": "Firmware visual resources remain C/C++ bitmap arrays. Raster files are reference/proof artifacts only.",
        "source_files": [
            "src/EchoPetCatalog.cpp",
            "src/EchoPetCatalog.h",
            "src/EchoPetCatalogVisuals.cpp",
            "analysis/official_reference/generated/catalog_item_official_reference_manifest.json",
        ],
        "row_count": len(rows),
        "expected_runtime_rows": constants["kCatalogItemCount"],
        "detected_runtime_rows": runtime_rows,
        "runtime_row_match": runtime_rows == constants["kCatalogItemCount"],
        "runtime_dimensions_px": [16, 16],
        "official_candidate_tile_count": official.get("detected_lcd_tile_count", 0),
        "official_candidate_tile_rule": (
            "Detected official LCD tiles are candidate references only. They "
            "must not be promoted to a catalog row unless the row label is "
            "unambiguous and the C++ bitmap row is regenerated with the stable "
            "catalog index preserved."
        ),
        "art_status_counts": counts,
        "rows": rows,
    }


def build_souvenir_manifest() -> dict:
    catalog_text = read(CATALOG_CPP)
    visuals_text = read(CATALOG_VISUALS_CPP)
    constants = load_constants()
    names = parse_souvenir_names(catalog_text)
    official = load_json(SOUVENIR_OFFICIAL)
    row_status = {int(row["index"]): row for row in official.get("rows", [])}
    official_override = {
        int(row["souvenir_index"]): row
        for row in official.get("official_source_overrides", [])
    }
    runtime_rows = count_bitmap_entries(visuals_text, "kSouvenirMemoryBitmaps")

    rows: list[dict] = []
    for index, name in enumerate(names):
        status_row = row_status.get(index, {})
        official_row = official_override.get(index)
        art_status = (
            "official_source_override"
            if official_row
            else status_row.get("status", "semantic_reference_only")
        )
        rows.append(
            {
                "index": index,
                "label": name,
                "runtime_bitmap_symbol": f"kSouvenirMemoryBitmaps[{index}]",
                "runtime_dimensions_px": [16, 16],
                "runtime_encoding": "16 uint16_t rows, high bit is leftmost pixel",
                "visual_traits": index,
                "art_status": art_status,
                "semantic_source": status_row.get("semantic_source", ""),
                "official_coverage": (
                    official_row.get("coverage", "")
                    if official_row
                    else status_row.get("coverage", "")
                ),
                "replacement_contract": (
                    "Replace only this 16x16 C++ memory row in "
                    "EchoPetCatalogVisuals.cpp or regenerate it with the "
                    "manifest row id preserved. Do not add runtime PNG/JPG/GIF "
                    "loading."
                ),
            }
        )

    counts: dict[str, int] = {}
    for row in rows:
        counts[row["art_status"]] = counts.get(row["art_status"], 0) + 1

    return {
        "generated_by": "analysis/resource_replacement_manifest/generate_resource_replacement_manifest.py",
        "runtime_boundary": "Firmware visual resources remain C/C++ bitmap arrays. Raster files are reference/proof artifacts only.",
        "source_files": [
            "src/EchoPetCatalog.cpp",
            "src/EchoPetCatalogVisuals.cpp",
            "analysis/official_reference/generated/souvenir_official_reference_manifest.json",
        ],
        "row_count": len(rows),
        "expected_runtime_rows": constants["kCatalogSouvenirCount"],
        "detected_runtime_rows": runtime_rows,
        "runtime_row_match": runtime_rows == constants["kCatalogSouvenirCount"],
        "runtime_dimensions_px": [16, 16],
        "art_status_counts": counts,
        "rows": rows,
    }


def build_character_manifest() -> dict:
    character_text = read(CHARACTER_CATALOG_CPP)
    visuals_text = read(CHARACTER_VISUALS_CPP)
    constants = load_constants()
    rows = parse_character_rows(character_text, constants)
    status_data = load_json(CHARACTER_STATUS)
    status_rows = {
        int(row["index"]): row for row in status_data.get("characters", [])
    }
    portrait_rows = count_bitmap_entries(visuals_text, "kCharacterCatalogBitmaps")
    idle_rows = count_bitmap_entries(visuals_text, "kCharacterIdleBitmaps")
    for row in rows:
        status = status_rows.get(row["index"], {})
        flags = status.get("flags", [])
        row.update(
            {
                "page_id": status.get("page_id", ""),
                "official_image": status.get("official_image", ""),
                "art_status": status.get("status", "unknown"),
                "review_flags": flags,
                "portrait_bitmap_symbol": (
                    f"kCharacterCatalogBitmaps[{row['index']}]"
                ),
                "portrait_dimensions_px": [16, 16],
                "portrait_encoding": (
                    "16 uint16_t rows, high bit is leftmost pixel"
                ),
                "idle_bitmap_symbol": f"kCharacterIdleBitmaps[{row['index']}]",
                "idle_dimensions_px": [24, 24],
                "idle_encoding": "24 uint32_t rows, high bit is leftmost pixel",
                "replacement_contract": (
                    "Replace the paired 16x16 portrait and 24x24 idle C++ "
                    "rows with the catalog row id preserved. Keep gameplay "
                    "catalog metadata stable; do not add runtime PNG/JPG/GIF "
                    "loading."
                ),
            }
        )

    counts: dict[str, int] = {}
    review_flag_count = 0
    for row in rows:
        counts[row["art_status"]] = counts.get(row["art_status"], 0) + 1
        review_flag_count += len(row["review_flags"])

    return {
        "generated_by": "analysis/resource_replacement_manifest/generate_resource_replacement_manifest.py",
        "runtime_boundary": "Firmware visual resources remain C/C++ bitmap arrays. Raster files are reference/proof artifacts only.",
        "source_files": [
            "src/EchoPetCharacterCatalog.cpp",
            "src/EchoPetCharacterCatalog.h",
            "src/EchoPetCharacterVisuals.cpp",
            "analysis/official_reference/generated/character_source_alignment_status.json",
        ],
        "row_count": len(rows),
        "expected_runtime_rows": constants["kCharacterCatalogCount"],
        "detected_portrait_rows": portrait_rows,
        "detected_idle_rows": idle_rows,
        "runtime_row_match": (
            portrait_rows == constants["kCharacterCatalogCount"]
            and idle_rows == constants["kCharacterCatalogCount"]
        ),
        "portrait_dimensions_px": [16, 16],
        "idle_dimensions_px": [24, 24],
        "art_status_counts": counts,
        "review_flag_count": review_flag_count,
        "rows": rows,
    }


def write_csv(path: Path, rows: list[dict], fields: list[str]) -> None:
    with path.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(handle, fieldnames=fields, extrasaction="ignore")
        writer.writeheader()
        for row in rows:
            flat = dict(row)
            for key, value in list(flat.items()):
                if isinstance(value, list):
                    flat[key] = ";".join(str(item) for item in value)
            writer.writerow(flat)


def md_cell(value: object) -> str:
    return str(value).replace("|", "\\|")


def write_report(character: dict, catalog: dict, souvenir: dict) -> None:
    character_open = [
        row
        for row in character["rows"]
        if row["art_status"] != "official-derived" or row["review_flags"]
    ]
    catalog_open = [
        row
        for row in catalog["rows"]
        if row["art_status"] != "official_source_override"
    ]
    souvenir_open = [
        row
        for row in souvenir["rows"]
        if row["art_status"] != "official_source_override"
    ]
    lines = [
        "# EchoPet Resource Replacement Manifest",
        "",
        "Generated by `analysis/resource_replacement_manifest/generate_resource_replacement_manifest.py`.",
        "",
        "This manifest defines the stable replacement surface for catalog item",
        "and souvenir visuals. It does not move raster assets into firmware.",
        "Runtime resources remain C/C++ bitmap arrays.",
        "",
        "## Summary",
        "",
        f"- Character rows: `{character['row_count']}`; portrait rows detected: `{character['detected_portrait_rows']}`; idle rows detected: `{character['detected_idle_rows']}`; match: `{character['runtime_row_match']}`.",
        f"- Catalog rows: `{catalog['row_count']}`; runtime rows detected: `{catalog['detected_runtime_rows']}`; match: `{catalog['runtime_row_match']}`.",
        f"- Souvenir rows: `{souvenir['row_count']}`; runtime rows detected: `{souvenir['detected_runtime_rows']}`; match: `{souvenir['runtime_row_match']}`.",
        f"- Character official-derived rows: `{character['art_status_counts'].get('official-derived', 0)}`.",
        f"- Character rows with review flags or non-official-derived status: `{len(character_open)}`.",
        f"- Catalog official-source override rows: `{catalog['art_status_counts'].get('official_source_override', 0)}`.",
        f"- Souvenir official-source override rows: `{souvenir['art_status_counts'].get('official_source_override', 0)}`.",
        f"- Catalog rows still requiring accepted final art: `{len(catalog_open)}`.",
        f"- Souvenir rows still requiring accepted final art: `{len(souvenir_open)}`.",
        "",
        "## Replacement Contract",
        "",
        "- Character replacement packs must preserve the 50 catalog row IDs, the 16x16 portrait row, and the 24x24 idle row for each character.",
        "- Preserve row IDs. Gameplay, inventory, shop, password, and souvenir logic address rows by numeric index.",
        "- Preserve the `16x16` source bitmap shape: 16 `uint16_t` rows, high bit is the leftmost pixel.",
        "- Preserve the character idle `24x24` source bitmap shape: 24 `uint32_t` rows, high bit is the leftmost pixel.",
        "- Replace or regenerate C++ rows in `EchoPetCatalogVisuals.cpp`; do not add runtime image file loading.",
        "- Use official images only as analysis/reference material unless rights-cleared replacement assets are supplied.",
        "",
        "## Character Art Status",
        "",
        "| Status | Rows |",
        "| --- | ---: |",
    ]
    for status, count in sorted(character["art_status_counts"].items()):
        lines.append(f"| {md_cell(status)} | {count} |")

    lines += [
        "",
        "## Catalog Art Status",
        "",
        "| Status | Rows |",
        "| --- | ---: |",
    ]
    for status, count in sorted(catalog["art_status_counts"].items()):
        lines.append(f"| {md_cell(status)} | {count} |")

    lines += [
        "",
        "## Souvenir Art Status",
        "",
        "| Status | Rows |",
        "| --- | ---: |",
    ]
    for status, count in sorted(souvenir["art_status_counts"].items()):
        lines.append(f"| {md_cell(status)} | {count} |")

    lines += [
        "",
        "## First Open Catalog Rows",
        "",
        "| Row | Label | Kind | Current status | Use scene |",
        "| ---: | --- | --- | --- | --- |",
    ]
    for row in catalog_open[:24]:
        lines.append(
            f"| {row['index']} | {md_cell(row['label'])} | {row['kind']} | "
            f"{md_cell(row['art_status'])} | {md_cell(row['use_scene_name'])} |"
        )

    lines += [
        "",
        "## First Open Souvenir Rows",
        "",
        "| Row | Label | Current status | Semantic source |",
        "| ---: | --- | --- | --- |",
    ]
    for row in souvenir_open[:24]:
        lines.append(
            f"| {row['index']} | {md_cell(row['label'])} | "
            f"{md_cell(row['art_status'])} | {md_cell(row['semantic_source'])} |"
        )

    if character_open:
        lines += [
            "",
            "## Character Rows Needing Review",
            "",
            "| Row | Label | Current status | Flags |",
            "| ---: | --- | --- | --- |",
        ]
        for row in character_open[:24]:
            lines.append(
                f"| {row['index']} | {md_cell(row['label'])} | "
                f"{md_cell(row['art_status'])} | "
                f"{md_cell(';'.join(row['review_flags']))} |"
            )

    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> None:
    OUT.mkdir(parents=True, exist_ok=True)
    character = build_character_manifest()
    catalog = build_catalog_manifest()
    souvenir = build_souvenir_manifest()
    CHARACTER_JSON.write_text(
        json.dumps(character, indent=2) + "\n", encoding="utf-8")
    CATALOG_JSON.write_text(json.dumps(catalog, indent=2) + "\n", encoding="utf-8")
    SOUVENIR_JSON.write_text(json.dumps(souvenir, indent=2) + "\n", encoding="utf-8")
    write_csv(
        CHARACTER_CSV,
        character["rows"],
        [
            "index",
            "label",
            "stage",
            "route",
            "archetype",
            "care_band",
            "generation_mask",
            "generation_tags",
            "tier_mask",
            "tier_tags",
            "gender_mask",
            "gender_tags",
            "source_page",
            "source_slot",
            "page_id",
            "frame_family",
            "visual_traits",
            "entry_source",
            "art_status",
            "review_flags",
            "portrait_bitmap_symbol",
            "portrait_dimensions_px",
            "idle_bitmap_symbol",
            "idle_dimensions_px",
        ],
    )
    write_csv(
        CATALOG_CSV,
        catalog["rows"],
        [
            "index",
            "label",
            "kind",
            "price",
            "behavior",
            "icon",
            "icon_name",
            "flags",
            "flag_names",
            "source_group",
            "use_scene",
            "use_scene_name",
            "visual_traits",
            "runtime_bitmap_symbol",
            "runtime_dimensions_px",
            "entry_source",
            "art_status",
            "official_status",
            "official_source",
            "official_coverage",
        ],
    )
    write_csv(
        SOUVENIR_CSV,
        souvenir["rows"],
        [
            "index",
            "label",
            "runtime_bitmap_symbol",
            "runtime_dimensions_px",
            "visual_traits",
            "art_status",
            "semantic_source",
            "official_coverage",
        ],
    )
    write_report(character, catalog, souvenir)
    print(f"wrote {REPORT}")
    print(
        "character rows:",
        character["row_count"],
        "catalog rows:",
        catalog["row_count"],
        "souvenir rows:",
        souvenir["row_count"],
    )


if __name__ == "__main__":
    main()
