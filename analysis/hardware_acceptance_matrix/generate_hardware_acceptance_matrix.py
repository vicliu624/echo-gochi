from __future__ import annotations

import csv
import json
from collections import Counter
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
OUT_DIR = ROOT / "analysis" / "hardware_acceptance_matrix"
HARDWARE = ROOT / "T_ECHO_LITE_HARDWARE_ACCEPTANCE.md"
HARDWARE_READINESS = ROOT / "analysis" / "hardware_readiness" / "hardware_readiness.md"

JSON_OUT = OUT_DIR / "hardware_acceptance_matrix.json"
CSV_OUT = OUT_DIR / "hardware_acceptance_matrix.csv"
MD_OUT = OUT_DIR / "hardware_acceptance_matrix.md"


SECTION_GROUP = {
    "Button Acceptance": ("button", "BTN"),
    "Refresh Acceptance": ("refresh", "REF"),
    "Sprite Proof Acceptance": ("sprite_proof", "SPR"),
    "Two-Device Acceptance": ("two_device", "DUAL"),
}


def read(path: Path) -> str:
    return path.read_text(encoding="utf-8")


def md_cell(value: object) -> str:
    return str(value).replace("|", "\\|")


def strip_code(value: str) -> str:
    return value.strip().strip("`")


def parse_markdown_cells(line: str) -> list[str]:
    return [cell.strip() for cell in line.strip().strip("|").split("|")]


def is_table_separator(cells: list[str]) -> bool:
    return bool(cells) and all(set(cell.replace(":", "").strip()) <= {"-"} for cell in cells)


def parse_hardware_rows() -> list[dict[str, object]]:
    rows: list[dict[str, object]] = []
    section = ""
    group_counts: Counter[str] = Counter()
    for line_number, line in enumerate(read(HARDWARE).splitlines(), start=1):
        if line.startswith("## "):
            section = line[3:].strip()
            continue
        if section not in SECTION_GROUP or not line.startswith("|"):
            continue
        cells = parse_markdown_cells(line)
        if not cells or is_table_separator(cells):
            continue
        if cells[0] in {"Case", "Flow", "Frame family"}:
            continue

        group, prefix = SECTION_GROUP[section]
        group_counts[group] += 1
        row_id = f"HW-{prefix}-{group_counts[group]:03d}"
        case_name = strip_code(cells[0])

        if section == "Sprite Proof Acceptance":
            result_64 = cells[1] if len(cells) > 1 else ""
            result_128 = cells[2] if len(cells) > 2 else ""
            evidence_notes = cells[3] if len(cells) > 3 else ""
            results = {"64x64": result_64, "128x128": result_128}
            expected = (
                "Frame family renders with correct anchor, clipping, and readability "
                "in both 64x64 and 128x128 main scenes."
            )
        else:
            expected = cells[1] if len(cells) > 1 else ""
            result = cells[2] if len(cells) > 2 else ""
            evidence_notes = cells[3] if len(cells) > 3 else ""
            results = {"single": result}

        rows.append({
            "id": row_id,
            "group": group,
            "section": section,
            "source_log_line": line_number,
            "case": case_name,
            "expected": expected,
            "results": results,
            "acceptance_status": acceptance_status(results),
            "evidence_notes": evidence_notes,
            "requires": requirement_for(group),
            "proof_type": proof_type_for(group, case_name),
            "screen_scope": screen_scope_for(group),
            "suggested_route": suggested_route_for(group, case_name),
        })
    return rows


def acceptance_status(results: dict[str, str]) -> str:
    statuses = {value.strip().upper() for value in results.values() if value.strip()}
    if not statuses:
        return "UNKNOWN"
    if statuses == {"PASS"}:
        return "PASS"
    if "FAIL" in statuses:
        return "FAIL"
    if "TODO" in statuses:
        return "TODO"
    return "MIXED"


def requirement_for(group: str) -> str:
    if group == "two_device":
        return "two_lora_devices"
    if group == "sprite_proof":
        return "single_device_both_screen_profiles"
    return "single_device"


def screen_scope_for(group: str) -> str:
    if group == "sprite_proof":
        return "compact_64x64_and_large_128x128_main_scene"
    if group == "two_device":
        return "active_device_profile_on_both_devices"
    return "active_device_profile"


def proof_type_for(group: str, case_name: str) -> str:
    if group == "button":
        return "close_video_key_and_screen"
    if group == "two_device":
        return "dual_device_video"
    if group == "sprite_proof":
        return "photo_pair_or_short_video"
    if case_name in {"Cold boot", "Idle 5 minutes", "Menu cycling 50 presses"}:
        return "continuous_video"
    if "game" in case_name.lower():
        return "gameplay_video"
    return "scene_video_or_photo_sequence"


def suggested_route_for(group: str, case_name: str) -> str:
    button_routes = {
        "Esc short press": "Start on HOME; press Esc once; verify the fixed menu/cursor advances exactly one slot.",
        "Home short press": "Start on HOME; select a fixed menu icon; press Home once; verify the selected action opens once.",
        "Email short press": "Enter any cancellable menu/status page; press Email once; verify cancel/back/status behavior once.",
        "Esc long press": "Hold Esc past the long-press threshold; verify only the defined hold event fires.",
        "Home long press": "Start on HOME; hold Home; verify CLOCK edit opens and pet ticking pauses during edit.",
        "Email long press": "Start on HOME; hold Email; verify the sprite proof diagnostic opens.",
        "Release timing": "Trigger any long press; release the key; verify no trailing short press fires.",
    }
    if group == "button":
        return button_routes.get(
            case_name,
            "Start on HOME; perform the named physical key action and record the screen response.",
        )

    refresh_routes = {
        "Cold boot": "Power or reset the device; record from blank panel through the first stable EchoPet frame.",
        "Idle 5 minutes": "Leave HOME idle for five minutes; record enough panel refreshes to catch whitening or ghosting.",
        "Menu cycling 50 presses": "Press Esc fifty times from HOME; record selection movement and panel behavior.",
        "Notice/result messages": "Trigger a notice or game result; verify it remains visible long enough to read.",
        "Food scene": "Open FOOD, run meal and snack scenes, and capture bite/refuse/result phases.",
        "Toilet scene": "Trigger visible mess if needed, open TOILET, and capture sweep/removal phases.",
        "Medicine scene": "Use MEDS while sick/tooth state is visible; capture dose/recover/refuse phases.",
        "Discipline scene": "Use TRAIN/discipline branches for TIME OUT, PRAISE, invalid, and missed-call cases.",
        "Lights dark-room": "Use LIGHTS ON/OFF around sleep/dark-room states; record stability and readability.",
        "Shop scene": "Open shop/catalog purchase flow; capture booth, keeper, item preview, and buy feedback.",
        "Password entry": "Open password entry; edit digits and submit both success and failure examples.",
        "Get game": "Open GAME->GET; record input, catch/miss, and reward result.",
        "Bump game": "Open GAME->BUMP; record meter, push, fall/result, and reward result.",
        "Flag game": "Open GAME->FLAG; record prompt, response, miss/good, and reward result.",
        "Heading game": "Open GAME->HEADING; record ball path, hit/miss, and reward result.",
        "Memory game": "Open GAME->MEMORY; record reveal, replay, wrong, good, and reward result.",
        "Sprint game": "Open GAME->SPRINT; record repeated input, finish, and reward result.",
        "Hoops game": "Open GAME->HOOPS; record aim, shot, made/miss, and reward result.",
    }
    if group == "refresh":
        return refresh_routes.get(
            case_name,
            "Run the named scene on one T-Echo-Lite and record enough frames to judge e-paper pacing.",
        )

    if group == "sprite_proof":
        return (
            "Hold Email from HOME to enter sprite proof; navigate to this frame family; "
            "capture compact 64x64 and large 128x128 proof if both display profiles are available."
        )

    dual_routes = {
        "Visit success": "Run CONNECT->VISIT on two LoRa builds; record standby, send, receive, and result on both screens.",
        "Visit timeout": "Open VISIT standby on one device only; record timeout and confirm no pet state changes.",
        "Present success": "Run CONNECT->PRESENT with a sendable catalog item; record sender debit and receiver award.",
        "Present full inventory": "Fill receiver inventory/stock first, then send PRESENT and record the visible refusal path.",
        "Game success": "Run CONNECT->GAME with matching game item ownership; record both devices resolving the same result.",
        "Love reject": "Run LOVE with an ineligible pair; record visible rejection and no baby state.",
        "Love partner": "Run LOVE with an eligible pair; record partner promotion flow.",
        "Love baby": "Run LOVE with an eligible pair through baby/next-generation entry; record both devices.",
        "LoRa parity": "Repeat one successful and one failed CONNECT flow over LoRa and compare to the IR-visible contract screens.",
    }
    return dual_routes.get(
        case_name,
        "Run the named two-device LoRa flow with the same build on both devices and record both screens.",
    )


def parse_readiness_rows() -> dict[tuple[str, str], dict[str, str]]:
    if not HARDWARE_READINESS.exists():
        return {}
    readiness: dict[tuple[str, str], dict[str, str]] = {}
    for line in read(HARDWARE_READINESS).splitlines():
        if not line.startswith("|"):
            continue
        cells = parse_markdown_cells(line)
        if len(cells) < 6 or is_table_separator(cells):
            continue
        if cells[0] == "HW log line":
            continue
        section = cells[1]
        case_name = strip_code(cells[2])
        readiness[(section, case_name)] = {
            "source_readiness": strip_code(cells[3]),
            "source_evidence": cells[4],
            "remaining_gate": cells[5],
        }
    return readiness


def attach_readiness(rows: list[dict[str, object]]) -> None:
    readiness = parse_readiness_rows()
    for row in rows:
        key = (str(row["section"]), str(row["case"]))
        item = readiness.get(key, {})
        row["source_readiness"] = item.get("source_readiness", "UNKNOWN")
        row["source_evidence"] = item.get("source_evidence", "")
        row["remaining_gate"] = item.get("remaining_gate", "HW observation")


def row_counts(rows: list[dict[str, object]], key: str) -> dict[str, int]:
    counts: Counter[str] = Counter(str(row.get(key, "")) for row in rows)
    return dict(sorted(counts.items()))


def write_json(rows: list[dict[str, object]]) -> None:
    status_counts = row_counts(rows, "acceptance_status")
    readiness_counts = row_counts(rows, "source_readiness")
    group_counts = row_counts(rows, "group")
    requirement_counts = row_counts(rows, "requires")
    data = {
        "generated_by": "analysis/hardware_acceptance_matrix/generate_hardware_acceptance_matrix.py",
        "source_hardware_acceptance": str(HARDWARE.relative_to(ROOT)),
        "source_readiness_report": str(HARDWARE_READINESS.relative_to(ROOT)),
        "row_count": len(rows),
        "acceptance_status_counts": status_counts,
        "source_readiness_counts": readiness_counts,
        "group_counts": group_counts,
        "requirement_counts": requirement_counts,
        "rows": rows,
    }
    JSON_OUT.write_text(json.dumps(data, indent=2) + "\n", encoding="utf-8")


def write_csv(rows: list[dict[str, object]]) -> None:
    with CSV_OUT.open("w", encoding="utf-8", newline="") as handle:
        writer = csv.DictWriter(
            handle,
            fieldnames=[
                "id",
                "group",
                "section",
                "source_log_line",
                "case",
                "expected",
                "result_single",
                "result_64x64",
                "result_128x128",
                "acceptance_status",
                "source_readiness",
                "requires",
                "proof_type",
                "screen_scope",
                "remaining_gate",
                "suggested_route",
                "evidence_notes",
                "source_evidence",
            ],
        )
        writer.writeheader()
        for row in rows:
            results = row["results"]
            assert isinstance(results, dict)
            writer.writerow({
                "id": row["id"],
                "group": row["group"],
                "section": row["section"],
                "source_log_line": row["source_log_line"],
                "case": row["case"],
                "expected": row["expected"],
                "result_single": results.get("single", ""),
                "result_64x64": results.get("64x64", ""),
                "result_128x128": results.get("128x128", ""),
                "acceptance_status": row["acceptance_status"],
                "source_readiness": row["source_readiness"],
                "requires": row["requires"],
                "proof_type": row["proof_type"],
                "screen_scope": row["screen_scope"],
                "remaining_gate": row["remaining_gate"],
                "suggested_route": row["suggested_route"],
                "evidence_notes": row["evidence_notes"],
                "source_evidence": row["source_evidence"],
            })


def write_markdown(rows: list[dict[str, object]]) -> None:
    status_counts = row_counts(rows, "acceptance_status")
    readiness_counts = row_counts(rows, "source_readiness")
    group_counts = row_counts(rows, "group")
    requirement_counts = row_counts(rows, "requires")

    lines = [
        "# EchoPet Hardware Acceptance Matrix",
        "",
        "Generated by `analysis/hardware_acceptance_matrix/generate_hardware_acceptance_matrix.py`.",
        "",
        "This matrix turns the hardware acceptance log into stable test cases.",
        "It does not mark any hardware behavior as PASS unless the source log",
        "already contains PASS evidence. Source readiness is tracked separately",
        "from physical acceptance.",
        "",
        "## Summary",
        "",
        f"- Acceptance rows: `{len(rows)}`.",
        f"- Acceptance statuses: `{', '.join(f'{k}:{v}' for k, v in status_counts.items())}`.",
        f"- Source readiness: `{', '.join(f'{k}:{v}' for k, v in readiness_counts.items())}`.",
        f"- Groups: `{', '.join(f'{k}:{v}' for k, v in group_counts.items())}`.",
        f"- Requirements: `{', '.join(f'{k}:{v}' for k, v in requirement_counts.items())}`.",
        "",
        "## Rows",
        "",
        "| ID | Group | Case | Acceptance | Source readiness | Requires | Proof | Route |",
        "| --- | --- | --- | --- | --- | --- | --- | --- |",
    ]
    for row in rows:
        lines.append(
            f"| `{row['id']}` | `{md_cell(row['group'])}` | "
            f"{md_cell(row['case'])} | `{md_cell(row['acceptance_status'])}` | "
            f"`{md_cell(row['source_readiness'])}` | `{md_cell(row['requires'])}` | "
            f"`{md_cell(row['proof_type'])}` | {md_cell(row['suggested_route'])} |"
        )

    lines += [
        "",
        "## Source Readiness Detail",
        "",
        "| ID | Remaining gate | Source evidence |",
        "| --- | --- | --- |",
    ]
    for row in rows:
        lines.append(
            f"| `{row['id']}` | {md_cell(row['remaining_gate'])} | "
            f"{md_cell(row['source_evidence'])} |"
        )

    lines += [
        "",
        "## Use",
        "",
        "- Update `T_ECHO_LITE_HARDWARE_ACCEPTANCE.md` with real hardware evidence.",
        "- Re-run this generator after evidence changes.",
        "- Keep runtime resources as C/C++ bitmap arrays; this matrix is test metadata only.",
    ]
    MD_OUT.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> None:
    rows = parse_hardware_rows()
    attach_readiness(rows)
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    write_json(rows)
    write_csv(rows)
    write_markdown(rows)


if __name__ == "__main__":
    main()
