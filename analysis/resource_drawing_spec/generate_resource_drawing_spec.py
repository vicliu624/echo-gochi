from __future__ import annotations

import csv
import json
from collections import Counter
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
OUT_DIR = ROOT / "analysis" / "resource_drawing_spec"

CATALOG_MANIFEST = (
    ROOT
    / "analysis"
    / "resource_replacement_manifest"
    / "catalog_replacement_manifest.json"
)
SOUVENIR_MANIFEST = (
    ROOT
    / "analysis"
    / "resource_replacement_manifest"
    / "souvenir_replacement_manifest.json"
)

CATALOG_JSON = OUT_DIR / "catalog_art_drawing_spec.json"
CATALOG_CSV = OUT_DIR / "catalog_art_drawing_spec.csv"
SOUVENIR_JSON = OUT_DIR / "souvenir_art_drawing_spec.json"
SOUVENIR_CSV = OUT_DIR / "souvenir_art_drawing_spec.csv"
REPORT = OUT_DIR / "resource_drawing_spec.md"


GENERIC_STOP_WORDS = {
    "A",
    "AN",
    "THE",
    "OF",
    "AND",
    "WITH",
    "FOR",
    "1",
    "2",
    "3",
    "4",
    "5",
}


ICON_FAMILIES = {
    "RiceBall": "meal/bowl silhouette",
    "Bowl": "meal/bowl silhouette",
    "Bread": "bread or cake block silhouette",
    "Cereal": "bowl with small garnish marks",
    "Cone": "cone/tapered snack silhouette",
    "Cup": "cup or container silhouette",
    "Cake": "layered cake silhouette",
    "Fruit": "round fruit silhouette with leaf/stem",
    "Turkey": "large roast or holiday food silhouette",
    "LayerCake": "multi-layer celebration cake silhouette",
    "Yogurt": "labeled cup/container silhouette",
    "Box": "box/gift silhouette",
    "Book": "open or closed book silhouette",
    "Charm": "small hanging charm silhouette",
    "Ball": "round ball silhouette with panel marks",
    "Blocks": "stacked building blocks silhouette",
    "Rope": "looped rope silhouette",
    "Trumpet": "trumpet/horn silhouette",
    "Ticket": "rectangular ticket silhouette",
    "Tv": "TV/screen silhouette",
    "Weights": "barbell/weights silhouette",
    "Wig": "hair/wig silhouette",
    "Wings": "paired wing silhouette",
    "Doll": "small doll/person silhouette",
    "Makeup": "make-up compact/mirror silhouette",
    "Shaver": "shaver/razor silhouette",
    "Drink": "bottle/drink silhouette",
    "Pencil": "diagonal pencil silhouette",
    "Glasses": "glasses silhouette",
    "Plant": "plant/pot silhouette",
    "Shovel": "shovel silhouette",
    "Chest": "treasure chest silhouette",
    "Lamp": "lamp silhouette",
    "FishingPole": "fishing pole silhouette",
    "Skis": "paired ski silhouette",
    "PalmTree": "palm tree silhouette",
    "Surfboard": "surfboard silhouette",
    "Panda": "bear/plush silhouette",
    "Maracas": "paired maracas silhouette",
    "Balloon": "balloon silhouette",
    "RcCar": "small car silhouette",
    "BoomBox": "boombox silhouette",
    "MusicDisc": "disc silhouette",
    "Costume": "character costume silhouette",
}


SOUVENIR_FAMILIES = {
    "growth": "hatch/baby growth memory",
    "nurture_food": "food or feeding memory",
    "nurture_toilet": "cleaning/toilet memory",
    "nurture_medicine": "medicine/recovery memory",
    "nurture_lights": "lights/sleep memory",
    "nurture_training": "discipline/training memory",
    "games": "game prize or medal memory",
    "shop_items": "shop item memory",
    "password_secret": "password/secret code memory",
    "connect": "friend/connection memory",
    "love": "love/partner memory",
    "family": "family/parent/baby memory",
    "calendar": "calendar or anniversary memory",
    "passaway": "pass-away memorial memory",
}


LABEL_VISUAL_HINTS = {
    "APPLE": "round fruit or pie with stem/leaf cue",
    "BANANA": "curved crescent fruit cue",
    "BBQ": "meat/plate cue, not a generic bowl",
    "BEEF": "bowl with beef topping cue",
    "CHEESE": "wedge or cake block cue",
    "CHERRY": "paired round fruit with stem cue",
    "CHOCOLATE": "segmented bar cue",
    "COOKIE": "round cookie with dot chips",
    "CORN": "cob with kernel rows",
    "DOG": "bun/sausage silhouette when paired with HOT or CORN",
    "CUPCAKE": "cup plus frosting top",
    "CURRY": "plate/bowl with sauce mound",
    "DONUT": "ring with center hole",
    "FRIES": "fries box with vertical strips",
    "HAMBURGER": "stacked bun/patty layers",
    "HOT": "bun/sausage silhouette when paired with DOG",
    "MELON": "round melon with stripe cue",
    "NOODLE": "bowl with noodle line cue",
    "OMELET": "oval plate/egg cue",
    "PASTA": "plate with fork/noodle line cue",
    "PEAR": "pear body with small stem",
    "PINEAPPLE": "pineapple body with crown",
    "ROLL": "rolled cake spiral cue",
    "SANDWICH": "triangular sandwich cue",
    "SOUP": "bowl with steam cue",
    "STEAK": "oval steak with bone or grill mark cue",
    "SUSHI": "rice block plus topping cue",
    "TACO": "folded shell cue",
    "TURKEY": "holiday roast cue",
    "WATERMELON": "wedge with seed marks",
    "YOGURT": "small cup/container cue",
    "ACTION": "small figure/person cue",
    "BALL": "round ball cue",
    "BALLOON": "balloon plus string cue",
    "BLOCK": "stacked block cue",
    "BOOM": "boombox speaker cue",
    "BOX": "box/gift cue",
    "BUBBLES": "soap bubbles cue",
    "CAMERA": "camera body/lens cue",
    "CAR": "car body and wheel cue",
    "CHEST": "treasure chest cue",
    "DISC": "disc/ring cue",
    "DOLL": "small doll cue",
    "DRINK": "bottle/can cue",
    "FISHING": "pole/line cue",
    "FLOWER": "flower/pot cue",
    "GLASSES": "two lens cue",
    "HAIR": "comb/hair gel container cue",
    "LAMP": "lamp shade/base cue",
    "MAKE": "compact/mirror cue",
    "MIRROR": "mirror cue",
    "PANDA": "bear/plush face cue",
    "PENCIL": "diagonal pencil cue",
    "PLANT": "sprout/pot cue",
    "RC": "car body and wheel cue",
    "ROPE": "looped rope cue",
    "SHAVER": "shaver cue",
    "SHOVEL": "shovel cue",
    "SKIS": "paired skis cue",
    "SURFBOARD": "long board cue",
    "TICKET": "ticket rectangle cue",
    "TRUMPET": "trumpet/horn cue",
    "TV": "screen cue",
    "WEIGHTS": "barbell cue",
    "WIG": "hair cap cue",
    "WINGS": "paired wings cue",
}


def read_json(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


def md_cell(value: object) -> str:
    return str(value).replace("|", "\\|")


def label_keywords(label: str) -> list[str]:
    words = [
        part
        for part in label.replace("-", " ").replace("/", " ").split()
        if part and part not in GENERIC_STOP_WORDS
    ]
    return words


def label_hint(label: str) -> str:
    hints: list[str] = []
    for word in label_keywords(label):
        hint = LABEL_VISUAL_HINTS.get(word)
        if hint and hint not in hints:
            hints.append(hint)
    return "; ".join(hints)


def catalog_family(row: dict) -> str:
    icon_name = str(row.get("icon_name", ""))
    use_scene = str(row.get("use_scene_name", "none"))
    kind = str(row.get("kind", ""))
    if icon_name in ICON_FAMILIES:
        return ICON_FAMILIES[icon_name]
    if kind == "food":
        if use_scene == "meal":
            return "meal silhouette with plate/bowl readability"
        if use_scene == "snack":
            return "snack silhouette with compact object readability"
        return "food silhouette"
    if kind == "souvenir":
        return "memory/souvenir silhouette"
    return "item silhouette"


def catalog_drawing_brief(row: dict) -> str:
    label = str(row["label"])
    family = catalog_family(row)
    hint = label_hint(label)
    if hint:
        return f"Draw `{label}` as a 16x16 {family}; key cue: {hint}."
    return f"Draw `{label}` as a 16x16 {family} matching its gameplay/use semantic."


def souvenir_drawing_brief(row: dict) -> str:
    label = str(row["label"])
    semantic = str(row.get("semantic_source", ""))
    family = SOUVENIR_FAMILIES.get(semantic, "memory/souvenir object")
    hint = label_hint(label)
    if hint:
        return f"Draw `{label}` as a 16x16 {family}; key cue: {hint}."
    return f"Draw `{label}` as a 16x16 {family} tied to the named memory."


def row_state(row: dict) -> str:
    return (
        "accepted_reference"
        if row.get("art_status") == "official_source_override"
        else "needs_final_art"
    )


def acceptance_rule(surface: str) -> str:
    return (
        f"{surface} row must remain recognizable at native 16x16, at x2 proof "
        "scale, and when placed in the compact 64x64 main scene; it must not "
        "depend on text labels, anti-aliased grayscale, or runtime image files."
    )


def common_contract(row: dict, surface: str) -> dict:
    return {
        "runtime_bitmap_symbol": row["runtime_bitmap_symbol"],
        "runtime_dimensions_px": [16, 16],
        "runtime_encoding": "16 uint16_t rows, high bit is leftmost pixel",
        "replacement_state": row_state(row),
        "must_preserve": [
            "numeric row index",
            "16x16 bitmap dimensions",
            "PROGMEM C/C++ array route",
            "high-bit-left row encoding",
            "no runtime PNG/JPG/GIF/BMP loading",
        ],
        "acceptance_rule": acceptance_rule(surface),
    }


def build_catalog_specs() -> dict:
    manifest = read_json(CATALOG_MANIFEST)
    rows = []
    for row in manifest["rows"]:
        spec = {
            "id": f"CAT-{int(row['index']):03d}",
            "index": row["index"],
            "label": row["label"],
            "kind": row["kind"],
            "use_scene": row["use_scene_name"],
            "icon_name": row["icon_name"],
            "flag_names": row["flag_names"],
            "visual_family": catalog_family(row),
            "drawing_brief": catalog_drawing_brief(row),
            "art_status": row["art_status"],
            "official_source": row.get("official_source", ""),
            "official_coverage": row.get("official_coverage", ""),
            "replacement_contract": row["replacement_contract"],
        }
        spec.update(common_contract(row, "catalog"))
        rows.append(spec)
    return wrap_specs("catalog", manifest, rows)


def build_souvenir_specs() -> dict:
    manifest = read_json(SOUVENIR_MANIFEST)
    rows = []
    for row in manifest["rows"]:
        spec = {
            "id": f"SOU-{int(row['index']):03d}",
            "index": row["index"],
            "label": row["label"],
            "semantic_source": row.get("semantic_source", ""),
            "visual_family": SOUVENIR_FAMILIES.get(
                row.get("semantic_source", ""), "memory/souvenir object"
            ),
            "drawing_brief": souvenir_drawing_brief(row),
            "art_status": row["art_status"],
            "official_coverage": row.get("official_coverage", ""),
            "replacement_contract": row["replacement_contract"],
        }
        spec.update(common_contract(row, "souvenir"))
        rows.append(spec)
    return wrap_specs("souvenir", manifest, rows)


def wrap_specs(surface: str, manifest: dict, rows: list[dict]) -> dict:
    state_counts = Counter(row["replacement_state"] for row in rows)
    status_counts = Counter(str(row["art_status"]) for row in rows)
    return {
        "generated_by": "analysis/resource_drawing_spec/generate_resource_drawing_spec.py",
        "surface": surface,
        "runtime_boundary": (
            "Official/reference raster assets are analysis-only. Replacement "
            "runtime art must be committed as C/C++ bitmap rows."
        ),
        "source_manifest": manifest.get("generated_by", ""),
        "row_count": len(rows),
        "runtime_row_match": manifest.get("runtime_row_match", False),
        "replacement_state_counts": dict(sorted(state_counts.items())),
        "art_status_counts": dict(sorted(status_counts.items())),
        "rows": rows,
    }


def write_csv(path: Path, rows: list[dict], extra_fields: list[str]) -> None:
    fields = [
        "id",
        "index",
        "label",
        *extra_fields,
        "visual_family",
        "drawing_brief",
        "replacement_state",
        "art_status",
        "runtime_bitmap_symbol",
        "runtime_encoding",
        "acceptance_rule",
        "official_coverage",
        "replacement_contract",
    ]
    with path.open("w", encoding="utf-8", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=fields)
        writer.writeheader()
        for row in rows:
            writer.writerow({field: row.get(field, "") for field in fields})


def write_report(catalog: dict, souvenir: dict) -> None:
    catalog_open = [
        row for row in catalog["rows"] if row["replacement_state"] != "accepted_reference"
    ]
    souvenir_open = [
        row for row in souvenir["rows"] if row["replacement_state"] != "accepted_reference"
    ]
    lines = [
        "# EchoPet Resource Drawing Specification",
        "",
        "Generated by `analysis/resource_drawing_spec/generate_resource_drawing_spec.py`.",
        "",
        "This is the row-local drawing contract for replacing catalog and",
        "souvenir visuals. It does not claim final art acceptance. It fixes the",
        "object to draw, bitmap encoding, acceptance rule, and runtime boundary",
        "for every replacement row.",
        "",
        "## Summary",
        "",
        f"- Catalog drawing rows: `{catalog['row_count']}`.",
        f"- Catalog rows needing final art: `{len(catalog_open)}`.",
        f"- Souvenir drawing rows: `{souvenir['row_count']}`.",
        f"- Souvenir rows needing final art: `{len(souvenir_open)}`.",
        f"- Catalog runtime row match: `{catalog['runtime_row_match']}`.",
        f"- Souvenir runtime row match: `{souvenir['runtime_row_match']}`.",
        "",
        "## Contract",
        "",
        "- Preserve the numeric row ID and C++ runtime row symbol.",
        "- Use 16 `uint16_t` rows per bitmap; the high bit is the leftmost pixel.",
        "- Keep official PNG/GIF/JPG resources as analysis references only.",
        "- A row is not visually accepted merely because it has a drawing brief.",
        "- Acceptance requires recognizability at native 16x16 and in the compact main scene.",
        "",
        "## Catalog Rows Needing Final Art",
        "",
        "| ID | Label | Family | Brief | Runtime symbol |",
        "| --- | --- | --- | --- | --- |",
    ]
    for row in catalog_open:
        lines.append(
            f"| `{row['id']}` | {md_cell(row['label'])} | "
            f"{md_cell(row['visual_family'])} | {md_cell(row['drawing_brief'])} | "
            f"`{md_cell(row['runtime_bitmap_symbol'])}` |"
        )
    lines += [
        "",
        "## Souvenir Rows Needing Final Art",
        "",
        "| ID | Label | Family | Brief | Runtime symbol |",
        "| --- | --- | --- | --- | --- |",
    ]
    for row in souvenir_open:
        lines.append(
            f"| `{row['id']}` | {md_cell(row['label'])} | "
            f"{md_cell(row['visual_family'])} | {md_cell(row['drawing_brief'])} | "
            f"`{md_cell(row['runtime_bitmap_symbol'])}` |"
        )
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> None:
    catalog = build_catalog_specs()
    souvenir = build_souvenir_specs()
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    CATALOG_JSON.write_text(json.dumps(catalog, indent=2) + "\n", encoding="utf-8")
    SOUVENIR_JSON.write_text(json.dumps(souvenir, indent=2) + "\n", encoding="utf-8")
    write_csv(
        CATALOG_CSV,
        catalog["rows"],
        ["kind", "use_scene", "icon_name", "flag_names", "official_source"],
    )
    write_csv(
        SOUVENIR_CSV,
        souvenir["rows"],
        ["semantic_source"],
    )
    write_report(catalog, souvenir)


if __name__ == "__main__":
    main()
