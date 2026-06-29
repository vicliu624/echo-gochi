from __future__ import annotations

import json
import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
SOURCE_ROOT = ROOT / "src" if (ROOT / "src").exists() else ROOT
OUT = ROOT / "analysis" / "completion_audit" / "completion_audit.md"

CHECKLIST = ROOT / "CONNECT_ALIGNMENT_CHECKLIST.md"
TASKS = ROOT / "CONNECT_PARITY_TASKS.md"
BACKLOG = ROOT / "CONNECT_REMAINING_BACKLOG.md"
HARDWARE = ROOT / "T_ECHO_LITE_HARDWARE_ACCEPTANCE.md"
SOURCE_LEDGER = ROOT / "CONNECT_SOURCE_LEDGER.md"
LINK_CONTRACT = ROOT / "LINK_CONTRACT.md"
GROWTH_REPORT = ROOT / "analysis" / "growth_coverage" / "growth_coverage.md"
PARITY_REPORT = ROOT / "analysis" / "parity_coverage" / "parity_resource_coverage.md"
HARDWARE_READINESS = ROOT / "analysis" / "hardware_readiness" / "hardware_readiness.md"
HARDWARE_ACCEPTANCE_MATRIX = (
    ROOT
    / "analysis"
    / "hardware_acceptance_matrix"
    / "hardware_acceptance_matrix.json"
)
SOURCE_BLOCKER_MATRIX = (
    ROOT
    / "analysis"
    / "source_blocker_matrix"
    / "source_blocker_matrix.json"
)
SOURCE_EVIDENCE_PROBE = (
    ROOT
    / "analysis"
    / "source_evidence_probe"
    / "source_evidence_probe.json"
)
VISUAL_REPORT = ROOT / "analysis" / "visual_alignment" / "visual_alignment_report.md"
RESOURCE_MANIFEST_REPORT = (
    ROOT
    / "analysis"
    / "resource_replacement_manifest"
    / "resource_replacement_manifest.md"
)
RESOURCE_DRAWING_SPEC_REPORT = (
    ROOT / "analysis" / "resource_drawing_spec" / "resource_drawing_spec.md"
)
CATALOG_DRAWING_SPEC = (
    ROOT / "analysis" / "resource_drawing_spec" / "catalog_art_drawing_spec.json"
)
SOUVENIR_DRAWING_SPEC = (
    ROOT / "analysis" / "resource_drawing_spec" / "souvenir_art_drawing_spec.json"
)
CHARACTER_REPLACEMENT_MANIFEST = (
    ROOT
    / "analysis"
    / "resource_replacement_manifest"
    / "character_replacement_manifest.json"
)
CATALOG_REPLACEMENT_MANIFEST = (
    ROOT
    / "analysis"
    / "resource_replacement_manifest"
    / "catalog_replacement_manifest.json"
)
SOUVENIR_REPLACEMENT_MANIFEST = (
    ROOT
    / "analysis"
    / "resource_replacement_manifest"
    / "souvenir_replacement_manifest.json"
)


def read(path: Path) -> str:
    return path.read_text(encoding="utf-8")


def line_matches(path: Path, pattern: str) -> list[tuple[int, str]]:
    rx = re.compile(pattern)
    rows: list[tuple[int, str]] = []
    for number, line in enumerate(read(path).splitlines(), start=1):
        if rx.search(line):
            rows.append((number, line.rstrip()))
    return rows


def classify_open_checklist(line: str) -> str:
    lowered = line.lower()
    if "hardware" in lowered or "validated" in lowered or "busy-pin" in lowered:
        return "HW"
    if "dual" in lowered or "two-device" in lowered or "lora" in lowered:
        return "DUAL"
    if "source" in lowered or "exact" in lowered or "official" in lowered:
        return "SOURCE"
    return "OPEN"


def md_cell(value: str) -> str:
    return value.replace("|", "\\|")


def table_status_rows(path: Path, statuses: set[str]) -> list[tuple[int, str, str]]:
    rows: list[tuple[int, str, str]] = []
    for number, line in enumerate(read(path).splitlines(), start=1):
        if not line.startswith("|"):
            continue
        cells = [cell.strip() for cell in line.strip("|").split("|")]
        if len(cells) < 4:
            continue
        status = cells[-1]
        if status in statuses or any(status.startswith(prefix) for prefix in statuses):
            rows.append((number, status, line.rstrip()))
    return rows


def task_status_lines(status: str) -> list[tuple[int, str]]:
    pattern = rf"^- `[^`]+` `{re.escape(status)}`:"
    return line_matches(TASKS, pattern)


def report_count(text: str, label: str) -> str:
    match = re.search(rf"{re.escape(label)}: `([^`]+)`", text)
    return match.group(1) if match else "unknown"


def read_json(path: Path) -> dict:
    return json.loads(read(path)) if path.exists() else {}


def replacement_manifest_summary(path: Path) -> dict[str, object]:
    data = read_json(path)
    if not data:
        return {
            "present": "no",
            "row_count": "unknown",
            "runtime_rows": "unknown",
            "runtime_match": "unknown",
            "official_source": "unknown",
            "open_art": "unknown",
            "candidate_tiles": "unknown",
        }
    art_counts = data.get("art_status_counts", {})
    official_source = art_counts.get(
        "official_source_override", art_counts.get("official-derived", 0))
    row_count = int(data.get("row_count", 0))
    open_art = row_count - int(official_source)
    runtime_rows = data.get("detected_runtime_rows")
    if runtime_rows is None and "detected_portrait_rows" in data:
        runtime_rows = (
            f"{data.get('detected_portrait_rows', 'unknown')} portrait / "
            f"{data.get('detected_idle_rows', 'unknown')} idle"
        )
    return {
        "present": "yes",
        "row_count": row_count,
        "runtime_rows": runtime_rows if runtime_rows is not None else "unknown",
        "runtime_match": data.get("runtime_row_match", "unknown"),
        "official_source": official_source,
        "open_art": open_art,
        "candidate_tiles": data.get("official_candidate_tile_count", 0),
    }


def hardware_matrix_summary(path: Path) -> dict[str, object]:
    data = read_json(path)
    if not data:
        return {
            "present": "no",
            "row_count": "unknown",
            "todo": "unknown",
            "ready": "unknown",
            "groups": "unknown",
            "requirements": "unknown",
        }
    status_counts = data.get("acceptance_status_counts", {})
    readiness_counts = data.get("source_readiness_counts", {})
    group_counts = data.get("group_counts", {})
    requirement_counts = data.get("requirement_counts", {})
    return {
        "present": "yes",
        "row_count": data.get("row_count", "unknown"),
        "todo": status_counts.get("TODO", 0),
        "ready": readiness_counts.get("READY", 0),
        "groups": ", ".join(
            f"{key}:{value}" for key, value in sorted(group_counts.items())
        ) or "unknown",
        "requirements": ", ".join(
            f"{key}:{value}" for key, value in sorted(requirement_counts.items())
        ) or "unknown",
    }


def drawing_spec_summary(catalog_path: Path,
                         souvenir_path: Path) -> dict[str, object]:
    catalog = read_json(catalog_path)
    souvenir = read_json(souvenir_path)
    if not catalog or not souvenir:
        return {
            "present": "no",
            "catalog_rows": "unknown",
            "catalog_open": "unknown",
            "souvenir_rows": "unknown",
            "souvenir_open": "unknown",
            "runtime_match": "unknown",
        }
    catalog_states = catalog.get("replacement_state_counts", {})
    souvenir_states = souvenir.get("replacement_state_counts", {})
    return {
        "present": "yes",
        "catalog_rows": catalog.get("row_count", "unknown"),
        "catalog_open": catalog_states.get("needs_final_art", 0),
        "souvenir_rows": souvenir.get("row_count", "unknown"),
        "souvenir_open": souvenir_states.get("needs_final_art", 0),
        "runtime_match": (
            catalog.get("runtime_row_match") is True
            and souvenir.get("runtime_row_match") is True
        ),
    }


def source_blocker_summary(path: Path) -> dict[str, object]:
    data = read_json(path)
    if not data:
        return {
            "present": "no",
            "row_count": "unknown",
            "status_classes": "unknown",
            "domains": "unknown",
            "blocker_types": "unknown",
        }
    status_counts = data.get("status_class_counts", {})
    domain_counts = data.get("domain_counts", {})
    blocker_type_counts = data.get("blocker_type_counts", {})
    return {
        "present": "yes",
        "row_count": data.get("row_count", "unknown"),
        "status_classes": ", ".join(
            f"{key}:{value}" for key, value in sorted(status_counts.items())
        ) or "unknown",
        "domains": ", ".join(
            f"{key}:{value}" for key, value in sorted(domain_counts.items())
        ) or "unknown",
        "blocker_types": ", ".join(
            f"{key}:{value}" for key, value in sorted(blocker_type_counts.items())
        ) or "unknown",
    }


def source_evidence_summary(path: Path) -> dict[str, object]:
    if not path.exists():
        return {
            "present": "no",
            "row_count": "unknown",
            "status_counts": "unknown",
        }
    data = json.loads(read(path))
    counts: dict[str, int] = {}
    for row in data:
        status = str(row.get("status", "unknown"))
        counts[status] = counts.get(status, 0) + 1
    return {
        "present": "yes",
        "row_count": len(data),
        "status_counts": ", ".join(
            f"{key}:{value}" for key, value in sorted(counts.items())
        ) or "unknown",
    }


def visual_status_rows(
        text: str, statuses: set[str]) -> list[tuple[int, str, str, str]]:
    rows: list[tuple[int, str, str, str]] = []
    for number, line in enumerate(text.splitlines(), start=1):
        if not line.startswith("|"):
            continue
        cells = [cell.strip() for cell in line.strip("|").split("|")]
        if len(cells) < 3 or cells[0] in {"Check", "---"}:
            continue
        status = cells[1].strip("`")
        if status in statuses:
            rows.append((number, status, cells[0], cells[2]))
    return rows


def main() -> None:
    checklist_open = line_matches(CHECKLIST, r"^- \[ \]")
    task_todo = task_status_lines("TODO")
    task_doing = task_status_lines("DOING")
    task_hw = task_status_lines("HW")
    task_substitute = task_status_lines("CODED-SUBSTITUTE")
    backlog_hw_dual = table_status_rows(BACKLOG, {"HW", "DUAL"})
    backlog_substitute = table_status_rows(BACKLOG, {"CODED-SUBSTITUTE", "BLOCKED-SOURCE"})
    hardware_todo = line_matches(HARDWARE, r"\|\s*TODO\s*\|")

    source_ledger_text = read(SOURCE_LEDGER)
    exact_source_gaps = [
        phrase
        for phrase in [
            "exact death thresholds",
            "poop timing",
            "sale frequency",
            "probability curves",
            "final pixel art",
            "exact official low-score table",
            "per-item/souvenir art",
        ]
        if phrase in source_ledger_text
    ]

    growth_effect_ok = "## Growth Effect Path Checks" in read(GROWTH_REPORT)
    link_contract_ok = "## Current Source Enforcement" in read(LINK_CONTRACT)
    link_report_ok = "## Link Visible Contract" in read(PARITY_REPORT)
    hardware_readiness_text = (
        read(HARDWARE_READINESS) if HARDWARE_READINESS.exists() else ""
    )
    hardware_readiness_ok = "## Readiness Rows" in hardware_readiness_text
    hardware_ready = report_count(hardware_readiness_text,
                                  "Source readiness `READY`")
    hardware_partial = report_count(hardware_readiness_text,
                                    "Source readiness `PARTIAL`")
    hardware_missing = report_count(hardware_readiness_text,
                                    "Source readiness `MISSING`")
    visual_text = read(VISUAL_REPORT) if VISUAL_REPORT.exists() else ""
    visual_status = report_count(visual_text, "Overall visual gate status")
    visual_pass = report_count(visual_text, "PASS checks")
    visual_open = report_count(visual_text, "OPEN checks")
    visual_hw = report_count(visual_text, "HW checks")
    visual_fail = report_count(visual_text, "FAIL checks")
    visual_gate_open_rows = visual_status_rows(visual_text,
                                               {"OPEN", "HW", "FAIL"})
    character_replacement = replacement_manifest_summary(
        CHARACTER_REPLACEMENT_MANIFEST)
    catalog_replacement = replacement_manifest_summary(
        CATALOG_REPLACEMENT_MANIFEST)
    souvenir_replacement = replacement_manifest_summary(
        SOUVENIR_REPLACEMENT_MANIFEST)
    resource_manifest_present = RESOURCE_MANIFEST_REPORT.exists()
    drawing_spec = drawing_spec_summary(CATALOG_DRAWING_SPEC,
                                        SOUVENIR_DRAWING_SPEC)
    drawing_spec_present = RESOURCE_DRAWING_SPEC_REPORT.exists()
    hardware_matrix = hardware_matrix_summary(HARDWARE_ACCEPTANCE_MATRIX)
    source_blockers = source_blocker_summary(SOURCE_BLOCKER_MATRIX)
    source_evidence = source_evidence_summary(SOURCE_EVIDENCE_PROBE)

    lines = [
        "# EchoPet Completion Audit",
        "",
        "Generated by `analysis/completion_audit/generate_completion_audit.py`.",
        "",
        "This audit separates source/spec/code evidence from hardware or external",
        "source blockers. It is not a claim of 100% Connection parity.",
        "",
        "## Current Evidence Summary",
        "",
        f"- Checklist open `[ ]` rows: `{len(checklist_open)}`.",
        f"- Task rows still marked `TODO`: `{len(task_todo)}`.",
        f"- Task rows still marked `DOING`: `{len(task_doing)}`.",
        f"- Task rows marked `HW`: `{len(task_hw)}`.",
        f"- Task rows marked `CODED-SUBSTITUTE`: `{len(task_substitute)}`.",
        f"- Backlog `HW`/`DUAL` rows: `{len(backlog_hw_dual)}`.",
        f"- Backlog source/substitute rows: `{len(backlog_substitute)}`.",
        f"- Hardware acceptance `TODO` rows: `{len(hardware_todo)}`.",
        f"- Growth effect source coverage present: `{'yes' if growth_effect_ok else 'no'}`.",
        f"- LINK source enforcement contract present: `{'yes' if link_contract_ok else 'no'}`.",
        f"- LINK parity report section present: `{'yes' if link_report_ok else 'no'}`.",
        f"- Hardware readiness report present: `{'yes' if hardware_readiness_ok else 'no'}`.",
        f"- Hardware TODO source readiness READY/PARTIAL/MISSING: `{hardware_ready}` / `{hardware_partial}` / `{hardware_missing}`.",
        f"- Hardware acceptance matrix present: `{hardware_matrix['present']}`.",
        f"- Hardware acceptance matrix rows/TODO/source-READY: `{hardware_matrix['row_count']}` / `{hardware_matrix['todo']}` / `{hardware_matrix['ready']}`.",
        f"- Source blocker matrix present: `{source_blockers['present']}`.",
        f"- Source blocker matrix rows: `{source_blockers['row_count']}`.",
        f"- Source blocker matrix blocker types: `{source_blockers['blocker_types']}`.",
        f"- Source evidence probe present: `{source_evidence['present']}`.",
        f"- Source evidence probe rows/statuses: `{source_evidence['row_count']}` / `{source_evidence['status_counts']}`.",
        f"- Visual gate status PASS/OPEN/HW/FAIL: `{visual_pass}` / `{visual_open}` / `{visual_hw}` / `{visual_fail}` (`{visual_status}`).",
        f"- Resource replacement manifest present: `{'yes' if resource_manifest_present else 'no'}`.",
        f"- Resource drawing spec present: `{'yes' if drawing_spec_present and drawing_spec['present'] == 'yes' else 'no'}`.",
        f"- Resource drawing spec catalog/souvenir rows needing final art: `{drawing_spec['catalog_rows']}` / `{drawing_spec['catalog_open']}` and `{drawing_spec['souvenir_rows']}` / `{drawing_spec['souvenir_open']}`.",
        f"- Character replacement rows/open/final-official-derived: `{character_replacement['row_count']}` / `{character_replacement['open_art']}` / `{character_replacement['official_source']}`.",
        f"- Catalog replacement rows/open/final-official-source: `{catalog_replacement['row_count']}` / `{catalog_replacement['open_art']}` / `{catalog_replacement['official_source']}`.",
        f"- Souvenir replacement rows/open/final-official-source: `{souvenir_replacement['row_count']}` / `{souvenir_replacement['open_art']}` / `{souvenir_replacement['official_source']}`.",
        "",
        "## Open Checklist Rows",
        "",
        "| Line | Gate | Requirement |",
        "| ---: | --- | --- |",
    ]
    for number, line in checklist_open:
        lines.append(
            f"| {number} | `{classify_open_checklist(line)}` | {md_cell(line)} |"
        )

    lines += [
        "",
        "## Remaining Task Rows Marked TODO Or DOING",
        "",
    ]
    active_tasks = [("TODO", number, line) for number, line in task_todo]
    active_tasks += [("DOING", number, line) for number, line in task_doing]
    if active_tasks:
        lines += ["| Line | Status | Task |", "| ---: | --- | --- |"]
        for status, number, line in active_tasks:
            lines.append(f"| {number} | `{status}` | {md_cell(line)} |")
    else:
        lines.append("- none")

    lines += [
        "",
        "## Resource Replacement Manifest",
        "",
        "| Surface | Present | Rows | Runtime rows | Runtime match | Official-source rows | Candidate official tiles | Rows still needing accepted final art |",
        "| --- | --- | ---: | ---: | --- | ---: | ---: | ---: |",
        f"| character | `{character_replacement['present']}` | `{character_replacement['row_count']}` | `{character_replacement['runtime_rows']}` | `{character_replacement['runtime_match']}` | `{character_replacement['official_source']}` | `{character_replacement['candidate_tiles']}` | `{character_replacement['open_art']}` |",
        f"| catalog | `{catalog_replacement['present']}` | `{catalog_replacement['row_count']}` | `{catalog_replacement['runtime_rows']}` | `{catalog_replacement['runtime_match']}` | `{catalog_replacement['official_source']}` | `{catalog_replacement['candidate_tiles']}` | `{catalog_replacement['open_art']}` |",
        f"| souvenir | `{souvenir_replacement['present']}` | `{souvenir_replacement['row_count']}` | `{souvenir_replacement['runtime_rows']}` | `{souvenir_replacement['runtime_match']}` | `{souvenir_replacement['official_source']}` | `{souvenir_replacement['candidate_tiles']}` | `{souvenir_replacement['open_art']}` |",
    ]

    lines += [
        "",
        "## Resource Drawing Spec",
        "",
        "| Present | Runtime match | Catalog rows | Catalog rows needing final art | Souvenir rows | Souvenir rows needing final art |",
        "| --- | --- | ---: | ---: | ---: | ---: |",
        f"| `{'yes' if drawing_spec_present and drawing_spec['present'] == 'yes' else 'no'}` | `{drawing_spec['runtime_match']}` | `{drawing_spec['catalog_rows']}` | `{drawing_spec['catalog_open']}` | `{drawing_spec['souvenir_rows']}` | `{drawing_spec['souvenir_open']}` |",
    ]

    lines += [
        "",
        "## Source Blocker Matrix",
        "",
        "| Present | Rows | Status classes | Domains | Blocker types |",
        "| --- | ---: | --- | --- | --- |",
        f"| `{source_blockers['present']}` | `{source_blockers['row_count']}` | `{source_blockers['status_classes']}` | `{source_blockers['domains']}` | `{source_blockers['blocker_types']}` |",
    ]

    lines += [
        "",
        "## Source Evidence Probe",
        "",
        "| Present | Rows | Status counts |",
        "| --- | ---: | --- |",
        f"| `{source_evidence['present']}` | `{source_evidence['row_count']}` | `{source_evidence['status_counts']}` |",
    ]

    lines += [
        "",
        "## Hardware Acceptance Matrix",
        "",
        "| Present | Rows | Acceptance TODO | Source READY | Groups | Requirements |",
        "| --- | ---: | ---: | ---: | --- | --- |",
        f"| `{hardware_matrix['present']}` | `{hardware_matrix['row_count']}` | `{hardware_matrix['todo']}` | `{hardware_matrix['ready']}` | `{hardware_matrix['groups']}` | `{hardware_matrix['requirements']}` |",
    ]

    lines += [
        "",
        "## Visual Alignment Gates",
        "",
    ]
    if visual_gate_open_rows:
        lines += ["| Line | Status | Check | Detail |",
                  "| ---: | --- | --- | --- |"]
        for number, status, check, detail in visual_gate_open_rows:
            lines.append(
                f"| {number} | `{md_cell(status)}` | {md_cell(check)} | {md_cell(detail)} |"
            )
    else:
        lines.append("- none")

    lines += [
        "",
        "## Hardware And Dual-Device Gates",
        "",
        "| File line | Status | Row |",
        "| ---: | --- | --- |",
    ]
    for number, status, line in backlog_hw_dual:
        lines.append(f"| {number} | `{md_cell(status)}` | {md_cell(line)} |")

    lines += [
        "",
        "## Hardware Acceptance TODO Rows",
        "",
        "| Line | Row |",
        "| ---: | --- |",
    ]
    for number, line in hardware_todo:
        lines.append(f"| {number} | {md_cell(line)} |")

    lines += [
        "",
        "## Source Or Rights-Cleared Art Gaps",
        "",
        "| File line | Status | Row |",
        "| ---: | --- | --- |",
    ]
    for number, status, line in backlog_substitute:
        lines.append(f"| {number} | `{md_cell(status)}` | {md_cell(line)} |")

    lines += [
        "",
        "## Source Ledger Gap Keywords",
        "",
        f"- `{', '.join(exact_source_gaps) if exact_source_gaps else 'none detected'}`",
        "",
        "## Audit Conclusion",
        "",
        "Source-side task rows no longer contain active work only if this report",
        "shows zero for both `Task rows still marked TODO` and `Task rows still",
        "marked DOING`. Remaining completion blockers are then hardware/dual-device",
        "observations, exact external source tables, or rights-cleared final art",
        "resources.",
    ]

    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text("\n".join(lines) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()

