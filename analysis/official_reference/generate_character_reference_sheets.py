from __future__ import annotations

import json
import re
import time
import urllib.parse
import urllib.request
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


ROOT = Path(__file__).resolve().parent
PROJECT_ROOT = ROOT.parents[1]
SOURCE_ROOT = PROJECT_ROOT / "src" if (PROJECT_ROOT / "src").exists() else PROJECT_ROOT
RAW = ROOT / "raw" / "character"
GENERATED = ROOT / "generated"
SIM_OUT = PROJECT_ROOT / "analysis" / "screen_simulator" / "out"
CATALOG = SOURCE_ROOT / "EchoPetCharacterCatalog.cpp"

BASE_URL = "https://tamagotchi-official.com/gb/series/connection/character/"


def font(size: int = 12) -> ImageFont.ImageFont:
    try:
        return ImageFont.truetype("DejaVuSansMono.ttf", size)
    except OSError:
        return ImageFont.load_default()


def catalog_names() -> list[str]:
    text = CATALOG.read_text(encoding="utf-8")
    return re.findall(r'CHARACTER_ROW(?:_TRAITS)?\("([^"]+)"', text)


def fetch(url: str) -> bytes:
    request = urllib.request.Request(
        url,
        headers={
            "User-Agent": "EchoPet official-reference audit/1.0",
        },
    )
    with urllib.request.urlopen(request, timeout=20) as response:
        return response.read()


def find_character_image(page_html: str, page_url: str) -> tuple[str, str]:
    match = re.search(
        r'<img\s+src="([^"]+)"\s+alt="([^"]+)"\s+class="c-card__thumb--full"',
        page_html,
    )
    if not match:
        raise ValueError("could not find official character image")
    src, alt = match.groups()
    joined = urllib.parse.urljoin(page_url, src)
    parsed = urllib.parse.urlsplit(joined)
    quoted_path = urllib.parse.quote(parsed.path)
    quoted = urllib.parse.urlunsplit(
        (parsed.scheme, parsed.netloc, quoted_path, parsed.query, parsed.fragment)
    )
    return quoted, alt


def safe_name(value: str) -> str:
    return re.sub(r"[^A-Za-z0-9_.-]+", "_", value).strip("_")


def ensure_character_assets(names: list[str]) -> list[dict[str, object]]:
    RAW.mkdir(parents=True, exist_ok=True)
    records: list[dict[str, object]] = []
    for index, name in enumerate(names):
        page_id = 201 + index
        page_url = f"{BASE_URL}{page_id}/"
        html_path = RAW / f"{page_id:03d}_{safe_name(name)}.html"
        if html_path.exists():
            html = html_path.read_text(encoding="utf-8")
        else:
            html = fetch(page_url).decode("utf-8", errors="replace")
            html_path.write_text(html, encoding="utf-8")
            time.sleep(0.08)

        image_url, alt = find_character_image(html, page_url)
        suffix = Path(urllib.parse.urlparse(image_url).path).suffix or ".jpg"
        image_name = f"{page_id:03d}_{safe_name(name)}{suffix.lower()}"
        image_path = RAW / image_name
        if not image_path.exists():
            image_path.write_bytes(fetch(image_url))
            time.sleep(0.08)

        with Image.open(image_path) as image:
            size = image.size
        records.append(
            {
                "index": index,
                "page_id": page_id,
                "catalog_name": name,
                "page_url": page_url,
                "official_image_url": image_url,
                "official_alt": alt,
                "local_html": str(html_path.relative_to(ROOT)),
                "local_image": str(image_path.relative_to(ROOT)),
                "image_size": size,
                "runtime_boundary": (
                    "analysis-only official reference; firmware runtime must use "
                    "C/C++ bitmap rows or rights-cleared replacement art"
                ),
            }
        )
    return records


def fit_image(image: Image.Image, box: tuple[int, int]) -> Image.Image:
    copy = image.convert("RGBA")
    copy.thumbnail(box, Image.Resampling.LANCZOS)
    return copy


def save_official_contact_sheet(records: list[dict[str, object]]) -> Path:
    cell_w = 150
    cell_h = 170
    cols = 10
    rows = (len(records) + cols - 1) // cols
    sheet = Image.new("RGB", (cell_w * cols, 28 + cell_h * rows), "white")
    draw = ImageDraw.Draw(sheet)
    draw.text(
        (4, 4),
        "OFFICIAL CHARACTER REFERENCE CONTACT SHEET",
        fill="black",
        font=font(14),
    )
    for record in records:
        i = int(record["index"])
        image_path = ROOT / str(record["local_image"])
        image = Image.open(image_path).convert("RGBA")
        fitted = fit_image(image, (cell_w - 12, cell_h - 44))
        x = (i % cols) * cell_w
        y = 28 + (i // cols) * cell_h
        draw.rectangle((x, y, x + cell_w - 1, y + cell_h - 1), outline="black")
        title = f'{i:02d} {record["page_id"]} {record["catalog_name"]}'
        draw.text((x + 4, y + 4), title[:23], fill="black", font=font(10))
        draw.text((x + 4, y + 18), str(record["official_alt"])[:23], fill="black", font=font(10))
        sheet.paste(fitted.convert("RGB"), (x + (cell_w - fitted.width) // 2, y + 36))

    out = GENERATED / "character_official_reference_contact_sheet.png"
    sheet.save(out)
    return out


def save_alignment_proof(official_sheet: Path) -> Path:
    current = SIM_OUT / "character_roster_proof_x2.png"
    source_status = GENERATED / "character_source_alignment_status_contact_sheet.png"
    parts: list[tuple[str, Image.Image]] = [
        ("official character reference thumbnails", Image.open(official_sheet).convert("RGBA")),
    ]
    if source_status.exists():
        parts.append(
            (
                "row-local official / 16x16 C++ / 24x24 C++ status",
                Image.open(source_status).convert("RGBA"),
            )
        )
    if current.exists():
        parts.append(("current EchoPet 50-row C/C++ source proof", Image.open(current).convert("RGBA")))

    width = 1400
    title_h = 24
    prepared: list[tuple[str, Image.Image]] = []
    for title, image in parts:
        fitted = fit_image(image, (width - 8, 760))
        prepared.append((title, fitted))

    height = 28 + sum(title_h + image.height + 10 for _, image in prepared)
    sheet = Image.new("RGB", (width, height), "white")
    draw = ImageDraw.Draw(sheet)
    draw.text(
        (4, 4),
        "CHARACTER OFFICIAL VS SOURCE ALIGNMENT PROOF",
        fill="black",
        font=font(14),
    )
    y = 28
    for title, image in prepared:
        draw.rectangle((0, y, width - 1, y + title_h + image.height + 7), outline="black")
        draw.text((4, y + 4), title, fill="black", font=font(12))
        sheet.paste(image.convert("RGB"), ((width - image.width) // 2, y + title_h))
        y += title_h + image.height + 10

    out = GENERATED / "character_official_alignment_proof.png"
    sheet.save(out)
    return out


def main() -> None:
    GENERATED.mkdir(parents=True, exist_ok=True)
    names = catalog_names()
    if len(names) != 50:
        raise SystemExit(f"expected 50 character rows, found {len(names)}")
    records = ensure_character_assets(names)
    official_sheet = save_official_contact_sheet(records)
    alignment = save_alignment_proof(official_sheet)
    generated: dict[str, str] = {
        "official_contact_sheet": str(official_sheet.relative_to(ROOT)),
        "alignment_proof": str(alignment.relative_to(ROOT)),
    }
    source_status = GENERATED / "character_source_alignment_status_contact_sheet.png"
    source_status_json = GENERATED / "character_source_alignment_status.json"
    if source_status.exists():
        generated["source_alignment_status_contact_sheet"] = str(
            source_status.relative_to(ROOT)
        )
    if source_status_json.exists():
        generated["source_alignment_status"] = str(
            source_status_json.relative_to(ROOT)
        )
    manifest = {
        "source_page": BASE_URL,
        "runtime_boundary": (
            "Official character images and page HTML are analysis-only references. "
            "Firmware resources remain C/C++ bitmap arrays or rights-cleared "
            "replacement art."
        ),
        "character_count": len(records),
        "generated": generated,
        "characters": records,
        "coverage_note": (
            "This closes the official-reference proof surface for the 50-row "
            "character catalog. It does not close official-look runtime art; "
            "the firmware still needs human acceptance of the generated 16x16 "
            "and 24x24 C++ bitmap rows."
        ),
    }
    manifest_path = GENERATED / "character_official_reference_manifest.json"
    manifest_path.write_text(json.dumps(manifest, indent=2), encoding="utf-8")
    print(f"wrote {official_sheet}")
    print(f"wrote {alignment}")
    print(f"wrote {manifest_path}")


if __name__ == "__main__":
    main()


