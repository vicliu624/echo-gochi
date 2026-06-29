from __future__ import annotations

import csv
import json
import re
from collections import Counter
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
OUT_DIR = ROOT / "analysis" / "source_blocker_matrix"

BACKLOG = ROOT / "CONNECT_REMAINING_BACKLOG.md"
SOURCE_LEDGER = ROOT / "CONNECT_SOURCE_LEDGER.md"

JSON_OUT = OUT_DIR / "source_blocker_matrix.json"
CSV_OUT = OUT_DIR / "source_blocker_matrix.csv"
MD_OUT = OUT_DIR / "source_blocker_matrix.md"


SOURCE_STATUS_PREFIXES = (
    "CODED-SUBSTITUTE",
    "BLOCKED-SOURCE",
    "CODED-PARTIAL",
)

SOURCE_GAP_PATTERNS = (
    "remain open",
    "remains open",
    "still need",
    "still needs",
    "source gap",
    "source-gated",
    "source gated",
    "needs source",
    "needs proof",
)

DOMAIN_RULES = [
    ("death_passaway", ("death", "pass-away", "Grim", "runaway")),
    ("care_timing", ("poop", "toilet", "decay", "Hungry", "Happy")),
    ("medicine_sickness", ("medicine", "sick", "toothache", "sickness")),
    ("shop_catalog", ("shop", "catalog", "item", "sale", "cart", "seasonal")),
    ("game_rewards", ("game", "reward", "Bump", "Sprint", "score")),
    ("growth_family", ("growth", "senior", "matchmaker", "parent", "baby")),
    ("visual_art", ("art", "visual", "sprite", "souvenir", "per-item")),
    ("hardware", ("hardware", "T-Echo-Lite", "refresh", "button")),
]

DOMAIN_OVERRIDES = {
    "P7.02": "visual_art",
    "P7.08": "growth_family",
    "P8.03c": "care_timing",
    "P8.03e": "care_timing",
    "P9.01": "source_data",
    "P9.05": "shop_catalog",
    "P9.06": "visual_art",
}

BLOCKER_TYPE_OVERRIDES = {
    "P1.02": "exact_source",
    "P1.04": "exact_source",
    "P2.06": "exact_source",
    "P2.07": "exact_source",
    "P3.04": "exact_source",
    "P4.02": "exact_source",
    "P7.02": "final_art",
    "P7.04": "mixed_art_hardware",
    "P7.08": "mixed_source_hardware",
    "P8.03a": "hardware_only",
    "P8.03b": "mixed_source_hardware",
    "P8.03c": "mixed_source_hardware",
    "P8.03d": "mixed_source_hardware",
    "P8.03e": "mixed_source_hardware",
    "P9.01": "multi_source",
    "P9.02": "mixed_source_hardware",
    "P9.03": "exact_source",
    "P9.04": "exact_source",
    "P9.05": "mixed_source_art",
    "P9.06": "final_art",
    "P9.07": "exact_source",
}

OPEN_NEED_OVERRIDES = {
    "P1.02": "Exact Connection 2024 poop interval by life stage.",
    "P1.04": (
        "Exact sickness probability, official toothache short-period length, "
        "and medicine cure/timing probabilities."
    ),
    "P2.06": "Exact 2024 Grim/pass-away threshold and wording.",
    "P2.07": (
        "Exact sourced pass-away thresholds to replace current elder, "
        "low-stat, and fourth-sickness substitute behavior."
    ),
    "P3.04": (
        "Exact sale date frequency, seasonal collision priority, and Ojitchi "
        "cart availability duration."
    ),
    "P4.02": (
        "Official low-score scaling, Bump/Sprint probability curves, and "
        "non-point stat effects."
    ),
    "P7.02": "Accepted final art for 148 catalog rows and 62 souvenir rows.",
    "P7.04": (
        "Accepted official-look egg/crack/hatch final art plus hardware proof."
    ),
    "P7.08": (
        "Exact same-pool character selection probability and hardware "
        "readability proof."
    ),
    "P8.03a": "Hardware readability proof for discipline scene pacing.",
    "P8.03b": "Medicine cure odds/timing plus hardware readability proof.",
    "P8.03c": "Food bite count/timing plus hardware readability proof.",
    "P8.03d": (
        "Exact mess interval, sickness effects, and hardware readability proof."
    ),
    "P8.03e": (
        "Exact sleep/nap behavior plus hardware whitening/ghosting proof."
    ),
    "P9.01": (
        "Ledger-level exact death, poop timing, sale frequency, probability "
        "curves, and final pixel-art evidence."
    ),
    "P9.02": (
        "Exact same-pool route probabilities, matchmaker/senior overlap, "
        "transition ordering, and hardware proof."
    ),
    "P9.03": "Exact 2024 death/pass-away thresholds and wording.",
    "P9.04": (
        "Exact Connection 2024 decay windows, sleep windows, lifecycle timing, "
        "toothache short-period length, calendar wording/art, and "
        "probability/timing values."
    ),
    "P9.05": (
        "Exact shop sale frequency, cart duration, seasonal collision priority, "
        "random probability/reward tables, item animation timing, and final "
        "item/souvenir art."
    ),
    "P9.06": "Accepted final per-item art for password/secret-code rewards.",
    "P9.07": "Official 2024 low-score table and Bump/Sprint probability curves.",
}

BLOCKER_TYPE_EVIDENCE = {
    "exact_source": (
        "official table/manual, cross-checked source, or hardware sample with "
        "exact value"
    ),
    "final_art": (
        "rights-cleared C++ bitmap replacement row or explicit user acceptance"
    ),
    "hardware_only": "T-Echo-Lite photo/video or two-device observation",
    "mixed_source_hardware": (
        "exact source value plus T-Echo-Lite photo/video or timing observation"
    ),
    "mixed_source_art": (
        "exact source value plus rights-cleared or user-accepted final art"
    ),
    "mixed_art_hardware": (
        "accepted final art plus T-Echo-Lite readability/refresh proof"
    ),
    "multi_source": (
        "multi-domain exact source evidence plus final art evidence where named"
    ),
}

BLOCKER_TYPE_GATES = {
    "exact_source": "do not mark final until the exact external value is sourced",
    "final_art": "do not mark final until replacement art is accepted",
    "hardware_only": "do not mark final until real T-Echo-Lite evidence is recorded",
    "mixed_source_hardware": (
        "do not mark final until both exact source and hardware evidence exist"
    ),
    "mixed_source_art": (
        "do not mark final until both exact source and final art evidence exist"
    ),
    "mixed_art_hardware": (
        "do not mark final until both final art and hardware evidence exist"
    ),
    "multi_source": (
        "do not mark final until every named source/art evidence stream is closed"
    ),
}

IMPLEMENTATION_HINTS = {
    "P1.02": ["kStageCareTimingRules", "kStatDecayRules", "checkPoop"],
    "P1.04": [
        "kSicknessTriggerRules",
        "kSweetSnackToothacheStreak",
        "kSweetSnackShortPeriodMinutes",
        "stageSicknesses_",
    ],
    "P2.06": ["kPassAwayRules", "stageSicknesses_"],
    "P2.07": ["checkDeath", "kPassAwayRules", "stageSicknesses_"],
    "P3.04": ["kShopRestockWindows", "kShopSaleRules", "kShopVendorWindows"],
    "P4.02": ["kGameRewardRules", "kBumpPrizeChart", "kGamePacingRules"],
    "P5.07": ["LinkGameKind", "prepareLinkGamePacket", "receiveFriendPacket"],
    "P7.02": [
        "kCatalogEntryBitmaps",
        "kSouvenirMemoryBitmaps",
        "catalog_art_drawing_spec",
    ],
    "P7.04": ["kEgg0", "kEggCrack0", "kEggHatch"],
    "P7.08": ["kCharacterCatalogBitmaps", "kCharacterIdleBitmaps"],
    "P8.03a": ["kDisciplineSceneAnimationMs", "drawDisciplineScene"],
    "P8.03b": ["kMedicineSceneAnimationMs", "drawMedicineScene"],
    "P8.03c": ["kFoodSceneAnimationMs", "drawFoodPage"],
    "P8.03d": ["kToiletSceneAnimationMs", "drawToiletScene"],
    "P8.03e": ["kLightsSceneAnimationMs", "drawLightsScene"],
    "P9.01": ["CONNECT_SOURCE_LEDGER.md"],
    "P9.02": [
        "kGrowthScheduleRules",
        "kAdultGrowthRules",
        "matchmakerNoticeReady",
        "characterTierMaskForStage",
    ],
    "P9.03": ["kPassAwayRules", "checkDeath"],
    "P9.04": [
        "kStageCareTimingRules",
        "kSicknessTriggerRules",
        "kAttentionTimingRules",
        "kSleepWindowRules",
    ],
    "P9.05": [
        "kCatalogOverrides",
        "kCatalogRandomRules",
        "kShopSaleRules",
        "kShopVendorWindows",
    ],
    "P9.06": ["kPasswordRewards", "kSecretCodeRewards"],
    "P9.07": ["kGameRewardRules", "kBumpPrizeChart", "kGameScorePrizePercent"],
}

DOMAIN_SOURCE_CANDIDATES = {
    "care_timing": [
        "OFF-MANUAL",
        "WIKI-CARE",
        "WIKI-FAQ-2024",
        "CURL-ENTAMA-CARE",
        "HW-TECHO",
    ],
    "medicine_sickness": [
        "OFF-MANUAL",
        "WIKI-SICKNESS",
        "WIKI-DEATH",
        "WIKI-FAQ-2024",
        "HW-TECHO",
    ],
    "death_passaway": [
        "WIKI-DEATH",
        "WIKI-FAQ-2024",
        "GG-GROWTH-2024",
        "HW-TECHO",
    ],
    "shop_catalog": [
        "OFF-HOWTO",
        "OFF-NEWS",
        "WIKI-2024",
        "WIKI-ITEM-2024",
        "WIKI-CODE-2024",
        "GG-SPECIAL-2024",
        "CURL-2024-EVENTS",
    ],
    "game_rewards": [
        "OFF-HOWTO",
        "OFF-MANUAL",
        "WIKI-2024",
        "TAMATALK-BUMP-V3",
        "TAMATALK-MEMORY-V3",
        "HW-TECHO",
    ],
    "growth_family": [
        "WIKI-CHAR-2024",
        "WIKI-FAQ-2024",
        "GG-GROWTH-2024",
        "HW-TECHO",
    ],
    "visual_art": [
        "OFF-HOWTO",
        "OFF-CHAR",
        "LOCAL-GIF",
        "resource_drawing_spec",
        "user-accepted replacement pack",
    ],
    "hardware": ["HW-TECHO"],
    "source_data": [
        "OFF-HOWTO",
        "OFF-MANUAL",
        "OFF-CHAR",
        "WIKI-2024",
        "WIKI-ITEM-2024",
        "WIKI-CHAR-2024",
        "WIKI-FAQ-2024",
        "GG-GROWTH-2024",
        "HW-TECHO",
    ],
}


def read(path: Path) -> str:
    return path.read_text(encoding="utf-8")


def md_cell(value: object) -> str:
    return str(value).replace("|", "\\|")


def parse_cells(line: str) -> list[str]:
    return [cell.strip() for cell in line.strip().strip("|").split("|")]


def is_separator(cells: list[str]) -> bool:
    return bool(cells) and all(set(cell.replace(":", "").strip()) <= {"-"} for cell in cells)


def status_has_source_gap(status: str) -> bool:
    if status.startswith(SOURCE_STATUS_PREFIXES):
        return True
    return any(pattern in status for pattern in SOURCE_GAP_PATTERNS)


def domain_for(backlog_id: str, task: str, status: str) -> str:
    if backlog_id in DOMAIN_OVERRIDES:
        return DOMAIN_OVERRIDES[backlog_id]
    haystack = f"{task} {status}"
    for domain, tokens in DOMAIN_RULES:
        if any(token in haystack for token in tokens):
            return domain
    return "source_data"


def blocker_type_for(backlog_id: str, domain: str, status: str) -> str:
    if backlog_id in BLOCKER_TYPE_OVERRIDES:
        return BLOCKER_TYPE_OVERRIDES[backlog_id]
    status_words = set(re.findall(r"[A-Za-z-]+", status.lower()))
    if domain == "hardware":
        return "hardware_only"
    if domain == "visual_art" or "art" in status_words or "pixel" in status_words:
        return "final_art"
    if "hardware" in status.lower():
        return "mixed_source_hardware"
    return "exact_source"


def evidence_class_for(blocker_type: str) -> str:
    return BLOCKER_TYPE_EVIDENCE.get(
        blocker_type,
        "official source or cross-checked community source with exact value",
    )


def acceptance_gate_for(blocker_type: str, status: str) -> str:
    gate = BLOCKER_TYPE_GATES.get(
        blocker_type,
        "source evidence required before promoting to DONE",
    )
    if status.startswith("CODED-SUBSTITUTE"):
        return f"{gate}; current substitute must remain labeled substitute"
    if status.startswith("CODED-PARTIAL"):
        return f"{gate}; current partial row must remain open"
    return gate


def extract_open_need(backlog_id: str, status: str) -> str:
    if backlog_id in OPEN_NEED_OVERRIDES:
        return OPEN_NEED_OVERRIDES[backlog_id]
    candidates: list[str] = []
    patterns = [
        r"(Exact[^.]+(?:missing|open|substitute|unconfirmed|need[^.]*|remain[^.]*)\.?)",
        r"(exact[^.]+(?:missing|open|substitute|unconfirmed|need[^.]*|remain[^.]*)\.?)",
        r"((?:[^.]*remain open[^.]*)\.?)",
        r"((?:[^.]*remains open[^.]*)\.?)",
        r"((?:[^.]*still need[^.]*)\.?)",
        r"((?:[^.]*still needs[^.]*)\.?)",
        r"((?:[^.]*source-gated[^.]*)\.?)",
    ]
    for pattern in patterns:
        for match in re.finditer(pattern, status):
            text = re.sub(r"\s+", " ", match.group(1)).strip()
            if text and text not in candidates:
                candidates.append(text)
    if candidates:
        return " ".join(candidates)
    if "while " in status:
        tail = status.rsplit("while ", 1)[-1].strip()
        if tail:
            return tail
    if ":" in status:
        return status.split(":", 1)[1].strip()
    return status


def parse_backlog_rows() -> list[dict[str, object]]:
    rows: list[dict[str, object]] = []
    current_section = ""
    for line_number, line in enumerate(read(BACKLOG).splitlines(), start=1):
        if line.startswith("## "):
            current_section = line[3:].strip()
            continue
        if not line.startswith("|"):
            continue
        cells = parse_cells(line)
        if len(cells) < 4 or is_separator(cells) or cells[0] == "ID":
            continue
        backlog_id = cells[0].strip("`")
        task = cells[1]
        gate = cells[2]
        status = cells[3]
        if not status_has_source_gap(status):
            continue
        domain = domain_for(backlog_id, task, status)
        blocker_type = blocker_type_for(backlog_id, domain, status)
        acceptance_gate = acceptance_gate_for(blocker_type, status)
        rows.append(
            {
                "id": f"SRC-{len(rows) + 1:03d}",
                "backlog_id": backlog_id,
                "section": current_section,
                "backlog_line": line_number,
                "task": task,
                "gate": gate,
                "status_class": status.split(":", 1)[0],
                "domain": domain,
                "blocker_type": blocker_type,
                "current_status": status,
                "open_need": extract_open_need(backlog_id, status),
                "evidence_class": evidence_class_for(blocker_type),
                "confidence_gate": acceptance_gate,
                "acceptance_gate": acceptance_gate,
                "implementation_hints": IMPLEMENTATION_HINTS.get(backlog_id, []),
                "replacement_action": replacement_action_for(domain, status),
            }
        )
    return rows


def replacement_action_for(domain: str, status: str) -> str:
    if domain == "visual_art":
        return "replace C++ bitmap rows or mark user-accepted final art in the manifest"
    if domain == "hardware":
        return "record hardware evidence in T_ECHO_LITE_HARDWARE_ACCEPTANCE.md"
    if domain == "game_rewards":
        return "replace explicit reward/probability table rows and regenerate coverage"
    if domain == "shop_catalog":
        return "replace shop/catalog rule rows and regenerate catalog/resource reports"
    if domain == "death_passaway":
        return "replace pass-away threshold rows and regenerate growth coverage"
    if domain == "medicine_sickness":
        return "replace sickness/toothache probability or duration rows"
    if domain == "care_timing":
        return "replace stage care timing/stat decay rows"
    if domain == "growth_family":
        return "replace growth/family timing or probability rows"
    return "update source ledger, code table, and generated audit together"


def parse_source_ids() -> dict[str, dict[str, str]]:
    source_ids: dict[str, dict[str, str]] = {}
    in_table = False
    for line in read(SOURCE_LEDGER).splitlines():
        if line.startswith("## Source IDs"):
            in_table = True
            continue
        if in_table and line.startswith("## "):
            break
        if not in_table or not line.startswith("|"):
            continue
        cells = parse_cells(line)
        if len(cells) < 5 or is_separator(cells) or cells[0] == "ID":
            continue
        source_id = cells[0].strip("`")
        source_ids[source_id] = {
            "source": cells[1],
            "scope": cells[2],
            "level": cells[3],
            "notes": cells[4],
        }
    return source_ids


def parse_rule_ledger() -> list[dict[str, str]]:
    rules: list[dict[str, str]] = []
    in_table = False
    for line in read(SOURCE_LEDGER).splitlines():
        if line.startswith("## Rule Ledger"):
            in_table = True
            continue
        if in_table and line.startswith("## "):
            break
        if not in_table or not line.startswith("|"):
            continue
        cells = parse_cells(line)
        if len(cells) < 4 or is_separator(cells) or cells[0] == "Rule group":
            continue
        rules.append(
            {
                "rule_group": cells[0],
                "accepted_sources": cells[1],
                "confidence": cells[2],
                "firmware_status": cells[3],
            }
        )
    return rules


def attach_source_candidates(rows: list[dict[str, object]]) -> None:
    for row in rows:
        row["candidate_source_ids"] = DOMAIN_SOURCE_CANDIDATES.get(
            str(row["domain"]), []
        )


def counts(rows: list[dict[str, object]], key: str) -> dict[str, int]:
    return dict(sorted(Counter(str(row.get(key, "")) for row in rows).items()))


def write_json(rows: list[dict[str, object]]) -> None:
    data = {
        "generated_by": "analysis/source_blocker_matrix/generate_source_blocker_matrix.py",
        "source_files": [
            "CONNECT_REMAINING_BACKLOG.md",
            "CONNECT_SOURCE_LEDGER.md",
        ],
        "row_count": len(rows),
        "status_class_counts": counts(rows, "status_class"),
        "domain_counts": counts(rows, "domain"),
        "blocker_type_counts": counts(rows, "blocker_type"),
        "source_ids": parse_source_ids(),
        "rows": rows,
    }
    JSON_OUT.write_text(json.dumps(data, indent=2) + "\n", encoding="utf-8")


def write_csv(rows: list[dict[str, object]]) -> None:
    fields = [
        "id",
        "backlog_id",
        "section",
        "backlog_line",
        "task",
        "gate",
        "status_class",
        "domain",
        "blocker_type",
        "open_need",
        "evidence_class",
        "confidence_gate",
        "acceptance_gate",
        "candidate_source_ids",
        "implementation_hints",
        "replacement_action",
        "current_status",
    ]
    with CSV_OUT.open("w", encoding="utf-8", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=fields)
        writer.writeheader()
        for row in rows:
            payload = dict(row)
            payload["candidate_source_ids"] = "; ".join(
                str(value) for value in row.get("candidate_source_ids", [])
            )
            payload["implementation_hints"] = "; ".join(
                str(value) for value in row.get("implementation_hints", [])
            )
            writer.writerow({field: payload.get(field, "") for field in fields})


def write_markdown(rows: list[dict[str, object]]) -> None:
    domain_counts = counts(rows, "domain")
    status_counts = counts(rows, "status_class")
    blocker_type_counts = counts(rows, "blocker_type")
    lines = [
        "# EchoPet Source Blocker Matrix",
        "",
        "Generated by `analysis/source_blocker_matrix/generate_source_blocker_matrix.py`.",
        "",
        "This matrix turns substitute/source-gated backlog rows into stable",
        "evidence requirements. It does not promote substitute behavior to final",
        "Connection parity; it states what source or rights-cleared evidence must",
        "arrive before a row can be marked final.",
        "",
        "## Summary",
        "",
        f"- Source blocker rows: `{len(rows)}`.",
        f"- Status classes: `{', '.join(f'{k}:{v}' for k, v in status_counts.items())}`.",
        f"- Domains: `{', '.join(f'{k}:{v}' for k, v in domain_counts.items())}`.",
        f"- Blocker types: `{', '.join(f'{k}:{v}' for k, v in blocker_type_counts.items())}`.",
        "",
        "## Rows",
        "",
        "| ID | Backlog | Domain | Type | Status | Open need | Evidence required | Action |",
        "| --- | --- | --- | --- | --- | --- | --- | --- |",
    ]
    for row in rows:
        lines.append(
            f"| `{row['id']}` | `{md_cell(row['backlog_id'])}` | "
            f"`{md_cell(row['domain'])}` | `{md_cell(row['blocker_type'])}` | "
            f"`{md_cell(row['status_class'])}` | "
            f"{md_cell(row['open_need'])} | {md_cell(row['evidence_class'])} | "
            f"{md_cell(row['replacement_action'])} |"
        )

    lines += [
        "",
        "## Implementation Hints",
        "",
        "| ID | Candidate sources | Code/data hints | Acceptance gate |",
        "| --- | --- | --- | --- |",
    ]
    for row in rows:
        lines.append(
            f"| `{row['id']}` | {md_cell('; '.join(row.get('candidate_source_ids', [])))} | "
            f"{md_cell('; '.join(row.get('implementation_hints', [])))} | "
            f"{md_cell(row['confidence_gate'])} |"
        )
    MD_OUT.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> None:
    rows = parse_backlog_rows()
    attach_source_candidates(rows)
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    write_json(rows)
    write_csv(rows)
    write_markdown(rows)


if __name__ == "__main__":
    main()
