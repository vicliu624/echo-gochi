#!/usr/bin/env python3
"""Probe sourced Connection facts against EchoPet code tables.

The output is intentionally conservative: a row can pass code evidence while
still remaining a source or hardware blocker for exact timing/probability/art.
"""

from __future__ import annotations

import csv
import json
import re
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Callable


ROOT = Path(__file__).resolve().parents[2]
OUT_DIR = Path(__file__).resolve().parent


@dataclass
class EvidenceRow:
    row_id: str
    domain: str
    source_ids: str
    claim: str
    code_target: str
    code_evidence: str
    status: str
    remaining_gap: str


def read(rel: str) -> str:
    return (ROOT / rel).read_text(encoding="utf-8")


SOURCES = {
    "ino": read("src/EchoPet.ino"),
    "model_h": read("src/EchoPetModel.h"),
    "model": read("src/EchoPetModel.cpp"),
    "catalog_h": read("src/EchoPetCatalog.h"),
    "catalog": read("src/EchoPetCatalog.cpp"),
    "ledger": read("CONNECT_SOURCE_LEDGER.md"),
}


def has(text: str, pattern: str) -> bool:
    return re.search(pattern, text, re.MULTILINE | re.DOTALL) is not None


def code_bool(ok: bool, detail: str) -> tuple[str, str]:
    return ("PASS" if ok else "MISSING", detail)


def count_reward_rows(array_name: str) -> int:
    match = re.search(
        rf"const\s+\w+\s+{array_name}\[\]\s*=\s*\{{(?P<body>.*?)\n\}};",
        SOURCES["model"],
        re.MULTILINE | re.DOTALL,
    )
    if not match:
        return 0
    return len(re.findall(r"^\s*\{\{", match.group("body"), re.MULTILINE))


def probe_shop_secret() -> EvidenceRow:
    ok = (
        has(SOURCES["ino"], r"kShopSecretTapGoal\s*=\s*4")
        and has(SOURCES["ino"], r"kShopSecretInitialPauseWindowMs\s*=\s*3000")
        and has(SOURCES["ino"], r"shopSecretTapCount\s*<=\s*1\s*\?")
    )
    status, evidence = code_bool(
        ok,
        "kShopSecretTapGoal=4 plus a separate first-to-second pause window and quick follow-up tap window",
    )
    return EvidenceRow(
        "EVID-001",
        "shop_secret_gesture",
        "OFF-HOWTO; WIKI-2024; GG-SPECIAL-2024",
        "Secret-code entry is one A at the shop/shopkeeper followed by three quick A presses until surprise.",
        "src/EchoPet.ino::handleShopSecretTap",
        evidence,
        status,
        "Hardware still must prove T-Echo-Lite button repeat/hold timing does not add extra taps.",
    )


def probe_shop_windows() -> EvidenceRow:
    ok = all(
        has(SOURCES["catalog"], pattern)
        for pattern in (
            r"kShopRestockWindows\[\]\s*=\s*\{\s*\{0,\s*0\}",
            r"\{15U\s*\*\s*60U,\s*1\}",
            r"\{19U\s*\*\s*60U,\s*2\}",
            r"kShopVendorWindows\[\]\s*=\s*\{\s*\{11U\s*\*\s*60U,\s*12U\s*\*\s*60U,\s*0\}",
            r"\{17U\s*\*\s*60U,\s*18U\s*\*\s*60U,\s*1\}",
        )
    )
    status, evidence = code_bool(
        ok,
        "shop restock rows 00:00/15:00/19:00 and Ojitchi windows 11:00-12:00/17:00-18:00 are explicit",
    )
    return EvidenceRow(
        "EVID-002",
        "shop_windows",
        "WIKI-2024; WIKI-ITEM-2024",
        "Regular shop restocks at 12 AM, 3 PM, and 7 PM; Ojitchi cart appears at 11 AM and 5 PM.",
        "src/EchoPetCatalog.cpp::kShopRestockWindows/kShopVendorWindows",
        evidence,
        status,
        "Exact Ojitchi availability duration is still substitute because sources name visit times but not a measured duration.",
    )


def probe_shop_sale() -> EvidenceRow:
    ok = has(SOURCES["catalog"], r"kShopSaleRule\s*=\s*\{37,\s*11,\s*13,\s*50\}")
    status = "CODED-SUBSTITUTE" if ok else "MISSING"
    return EvidenceRow(
        "EVID-003",
        "shop_sale",
        "WIKI-ITEM-2024",
        "On random sale dates, regular shop items are 50% off.",
        "src/EchoPetCatalog.cpp::kShopSaleRule",
        "50% discount is explicit; sale is date-hash based with modulo 13 and vendor visits suppress sale",
        status,
        "Exact sale frequency/date distribution remains unsourced.",
    )


def probe_game_top_prizes() -> EvidenceRow:
    expected = (
        r"\{Stage::kBaby,\s*MiniGameKind::kGet,\s*100",
        r"\{Stage::kChild,\s*MiniGameKind::kGet,\s*150",
        r"\{Stage::kChild,\s*MiniGameKind::kBump,\s*1800",
        r"\{Stage::kTeen,\s*MiniGameKind::kGet,\s*225",
        r"\{Stage::kTeen,\s*MiniGameKind::kFlag,\s*300",
        r"\{Stage::kTeen,\s*MiniGameKind::kHeading,\s*400",
        r"\{Stage::kAdult,\s*MiniGameKind::kGet,\s*300",
        r"\{Stage::kAdult,\s*MiniGameKind::kBump,\s*1200",
        r"\{Stage::kAdult,\s*MiniGameKind::kFlag,\s*600",
        r"\{Stage::kAdult,\s*MiniGameKind::kHeading,\s*400",
        r"\{Stage::kAdult,\s*MiniGameKind::kMemory,\s*400",
        r"\{Stage::kAdult,\s*MiniGameKind::kSprint,\s*400",
        r"\{Stage::kAdult,\s*MiniGameKind::kHoops,\s*100",
        r"\{Stage::kElder,\s*MiniGameKind::kGet,\s*300",
        r"\{Stage::kElder,\s*MiniGameKind::kBump,\s*600",
        r"\{Stage::kElder,\s*MiniGameKind::kFlag,\s*400",
        r"\{Stage::kElder,\s*MiniGameKind::kHeading,\s*100",
    )
    ok = all(has(SOURCES["model"], pattern) for pattern in expected)
    status, evidence = code_bool(ok, "stage/game top-prize table entries are present")
    return EvidenceRow(
        "EVID-004",
        "game_rewards",
        "WIKI-2024",
        "Known 2024 top-prize table by stage and mini-game.",
        "src/EchoPetModel.cpp::kGameRewardRules",
        evidence,
        status,
        "Low-score table and Bump/Sprint probability curves remain exact-source blockers.",
    )


def probe_game_lengths() -> EvidenceRow:
    expected = (
        r"\{MiniGameKind::kGet,\s*100,\s*100,\s*100",
        r"\{MiniGameKind::kBump,\s*8,\s*8,\s*8",
        r"\{MiniGameKind::kFlag,\s*9,\s*9,\s*9",
        r"\{MiniGameKind::kHeading,\s*20,\s*20,\s*20",
        r"\{MiniGameKind::kMemory,\s*8,\s*20,\s*20",
        r"\{MiniGameKind::kSprint,\s*8,\s*8,\s*8",
        r"\{MiniGameKind::kHoops,\s*30,\s*30,\s*30",
    )
    ok = all(has(SOURCES["model"], pattern) for pattern in expected)
    status, evidence = code_bool(ok, "game goal/round/score caps are explicit")
    return EvidenceRow(
        "EVID-005",
        "game_pacing",
        "WIKI-2024; OFF-HOWTO",
        "Seven game goals are Get 100, Bump 8, Flag 9, Heading 20, Memory 20 patterns, Sprint 8, Hoops 30.",
        "src/EchoPetModel.cpp::kGamePacingRules",
        evidence,
        status,
        "Real e-paper readability and exact moment-to-moment timing remain hardware-gated.",
    )


def probe_toothache() -> EvidenceRow:
    ok = (
        has(SOURCES["model"], r"kSweetSnackToothacheStreak\s*=\s*15")
        and has(SOURCES["model"], r"kSweetSnackShortPeriodMinutes\s*=\s*60")
    )
    status = "CODED-SUBSTITUTE" if ok else "MISSING"
    return EvidenceRow(
        "EVID-006",
        "medicine_sickness",
        "WIKI-SICKNESS; WIKI-FAQ-2024",
        "Toothache is triggered by fifteen consecutive sweet snacks in a short period.",
        "src/EchoPetModel.cpp::kSweetSnackToothacheStreak/kSweetSnackShortPeriodMinutes",
        "15-snack streak is explicit; short-period duration is a 60 pet-minute substitute",
        status,
        "Official 2024 short-period length and cure probability remain exact-source blockers.",
    )


def probe_attention_windows() -> EvidenceRow:
    ok = all(
        has(SOURCES["model"], pattern)
        for pattern in (
            r"\{AttentionTimingKind::kOrdinaryMiss,\s*15,\s*0\}",
            r"\{AttentionTimingKind::kLightsLeftOnMistake,\s*60,\s*20\}",
        )
    )
    status, evidence = code_bool(
        ok, "ordinary 15-minute miss and lights-left-on 60-minute miss rows are explicit"
    )
    return EvidenceRow(
        "EVID-007",
        "care_mistake_timing",
        "WIKI-FAQ-2024",
        "Ordinary care calls use a 15-minute care window; lights-left-on has a longer miss window.",
        "src/EchoPetModel.cpp::kAttentionTimingRules",
        evidence,
        status,
        "Hardware-visible attention icon/call cadence still needs observation.",
    )


def probe_death_substitute() -> EvidenceRow:
    ok = (
        has(SOURCES["model"], r"kStageSicknessPassAwayCount\s*=\s*4")
        and has(SOURCES["model"], r"kPassAwayRules\[\]")
        and has(SOURCES["ledger"], r"seniors live indefinitely until neglected enough")
    )
    status = "CODED-SUBSTITUTE" if ok else "MISSING"
    return EvidenceRow(
        "EVID-008",
        "death_passaway",
        "WIKI-DEATH; WIKI-FAQ-2024",
        "Connection-era death is constrained to neglect/sickness direction rather than age alone.",
        "src/EchoPetModel.cpp::kPassAwayRules/stageSicknesses_",
        "stage-local fourth-sickness and elder low-stat substitute rules are explicit",
        status,
        "Exact 2024 Grim/pass-away thresholds and wording remain source blockers.",
    )


def probe_inventory_caps() -> EvidenceRow:
    ok = all(
        has(SOURCES["model"], pattern)
        for pattern in (
            r"kMaxFoodStock\s*=\s*19",
            r"kMaxItemStock\s*=\s*32",
            r"catalogStockTotal\(CatalogKind::kFood\)\s*>=\s*kMaxFoodStock",
            r"catalogStockTotal\(CatalogKind::kItem\)\s*>=\s*kMaxItemStock",
        )
    )
    status, evidence = code_bool(
        ok, "food catalog stock is capped at 19 total and item catalog stock at 32 total"
    )
    return EvidenceRow(
        "EVID-009",
        "inventory_caps",
        "WIKI-ITEM-2024",
        "Meal and Snack storage share a 19-count cap; Items storage has a 32-count cap.",
        "src/EchoPetModel.cpp::kMaxFoodStock/kMaxItemStock/buyCatalogItem",
        evidence,
        status,
        "Exact UI wording for full storage still needs hardware-readable screen proof.",
    )


def probe_default_foods() -> EvidenceRow:
    ok = all(
        has(SOURCES["model_h"], pattern)
        for pattern in (
            r"kDefaultMealCount\s*=\s*4",
            r"kDefaultSnackCount\s*=\s*4",
            r"kFoodKindCount\s*=\s*kDefaultMealCount\s*\+\s*kDefaultSnackCount",
            r"enum class FoodKind.*kScone.*kSushi.*kBread.*kCereal.*kCone.*kPudding.*kTart.*kApple",
        )
    ) and all(
        has(SOURCES["model"], pattern)
        for pattern in (
            r"case FoodKind::kScone:\s*return \"SCONE\"",
            r"case FoodKind::kApple:\s*return \"APPLE\"",
        )
    )
    status, evidence = code_bool(
        ok,
        "default meals Scone/Sushi/Bread/Cereal and snacks Cone/Pudding/Tart/Apple are fixed enum rows",
    )
    return EvidenceRow(
        "EVID-010",
        "default_foods",
        "WIKI-ITEM-2024; WIKI-CARE",
        "The default meals and snacks are permanent choices and do not run out.",
        "src/EchoPetModel.h::FoodKind and src/EchoPetModel.cpp::useFood",
        evidence,
        status,
        "Exact visual bite timing remains a sprite/hardware proof item.",
    )


def probe_wallet_cap() -> EvidenceRow:
    ok = (
        has(SOURCES["model"], r"kMaxGotchiPoints\s*=\s*9999")
        and len(re.findall(r"kMaxGotchiPoints", SOURCES["model"])) >= 4
    )
    status, evidence = code_bool(
        ok, "Gotchi Point gains and save-load clamps reference the 9999 wallet cap"
    )
    return EvidenceRow(
        "EVID-011",
        "wallet_cap",
        "WIKI-GOTCHI-POINTS; WIKI-2024",
        "Connection/V2-V3 style Gotchi Point wallet caps at 9,999.",
        "src/EchoPetModel.cpp::kMaxGotchiPoints",
        evidence,
        status,
        "Exact point-gain low-score tables remain separate game-reward source gaps.",
    )


def probe_king_donation() -> EvidenceRow:
    ok = all(
        has(SOURCES["model"], pattern)
        for pattern in (
            r"kSuperCleanerDonation\s*=\s*1000",
            r"donations_\s*=\s*clampAdd16\(donations_,\s*100",
            r"awardSouvenir\(39\)",
            r"donations_\s*>=\s*kSuperCleanerDonation.*?messCount_\s*=\s*0",
        )
    )
    status = "CODED-SUBSTITUTE" if ok else "MISSING"
    return EvidenceRow(
        "EVID-012",
        "king_donation",
        "WIKI-KING-2024",
        "Donating 1000 Gotchi Points to the King unlocks the Super Unchikun-style no-mess cleanup projection.",
        "src/EchoPetModel.cpp::kSuperCleanerDonation/actionDonate/updatePoop",
        "1000-point threshold, 100-point donations, souvenir award, and auto mess cleanup are explicit",
        status,
        "Exact King congratulation/cutscene art remains source/visual blocker.",
    )


def probe_password_secret_tables() -> EvidenceRow:
    password_count = count_reward_rows("kPasswordRewards")
    secret_count = count_reward_rows("kSecretCodeRewards")
    ok = password_count == 32 and secret_count == 7 and all(
        has(SOURCES["model"], pattern)
        for pattern in (
            r"decodeCatalogPassword",
            r"enterSecretCode",
            r"buyCatalogItem\(reward.catalogIndex\)",
        )
    )
    status, evidence = code_bool(
        ok, f"password reward rows={password_count}, shop secret-code rows={secret_count}"
    )
    return EvidenceRow(
        "EVID-013",
        "password_secret_tables",
        "WIKI-CODE-2024; OFF-NEWS; GG-SPECIAL-2024",
        "The 2024 password table has 32 10-digit entries and the shop secret-code table has seven A/B/C entries.",
        "src/EchoPetModel.cpp::kPasswordRewards/kSecretCodeRewards",
        evidence,
        status,
        "Older V3 password-generator compatibility remains out of scope until separately sourced.",
    )


def probe_link_game_requirements() -> EvidenceRow:
    ok = all(
        has(SOURCES["model_h"], pattern)
        for pattern in (
            r"enum class LinkGameKind.*kGotchiPoint.*kBall.*kRcCar.*kRope.*kBuildingBlock.*kBalloon.*kTrumpet",
            r"kLinkGameCount\s*=\s*7",
        )
    ) and all(
        has(SOURCES["model"], pattern)
        for pattern in (
            r"case LinkGameKind::kGotchiPoint:\s*return true",
            r"kCatalogBallIndex",
            r"kCatalogRcCar1Index.*kCatalogRcCar2Index.*kCatalogRcCar3Index",
            r"kCatalogRopeIndex",
            r"kCatalogBuildingBlockIndex",
            r"kCatalogBalloonIndex",
            r"kCatalogTrumpetAIndex.*kCatalogTrumpetBIndex",
            r"linkGameRequiredLabel",
        )
    )
    status, evidence = code_bool(
        ok,
        "seven Connection game rows exist; item-backed rows require matching catalog ownership",
    )
    return EvidenceRow(
        "EVID-014",
        "link_game_requirements",
        "WIKI-ITEM-2024; OFF-HOWTO",
        "Connection games include Gotchi Point plus item-backed Ball, RC Car, Rope, Building Block, Balloon, and Trumpet rows.",
        "src/EchoPetModel.h::LinkGameKind and src/EchoPetModel.cpp::linkGameOwnedInBits",
        evidence,
        status,
        "Exact dual-device timing, win animation, and reward odds remain DUAL/HW/source blockers.",
    )


def probe_item_dependencies() -> EvidenceRow:
    ok = all(
        has(SOURCES["model"], pattern)
        for pattern in (
            r"kCatalogMusicDiscIndex.*?kCatalogBoomBoxIndex",
            r"catalogBitClear\(catalogOwned_,\s*kCatalogBoomBoxIndex\)",
            r"kCatalogMakeupIndex.*?kCatalogMirrorIndex",
            r"kCatalogShaverIndex.*?characterCatalogEntry\(characterCatalogId_\)\.name.*?\"Oyajitchi\"",
            r"kCatalogTamaDrinkIndex.*?kMemoryFlagTamaDrinkGuard",
            r"kCatalogHoneyIndex.*?kMemoryFlagHoneyPending",
            r"kCatalogActionFigureIndex.*?gender_\s*==\s*Gender::kBoy",
            r"kCatalogDoll1Index.*?Gender::kGirl",
        )
    )
    status, evidence = code_bool(
        ok,
        "Music Disc/Boom Box, Make-Up/Mirror, Shaver/Oyajitchi, Tama Drink, Honey, and gender enjoyment gates are explicit",
    )
    return EvidenceRow(
        "EVID-015",
        "item_dependencies",
        "WIKI-ITEM-2024",
        "Special item use restrictions and side effects should follow the listed Connection item behavior.",
        "src/EchoPetModel.cpp::useCatalogItem",
        evidence,
        status,
        "Per-item scene art and exact destruction/probability rates remain visual/source blockers.",
    )


def probe_random_item_effects() -> EvidenceRow:
    ok = all(
        has(SOURCES["model"], pattern)
        for pattern in (
            r"\{kCatalogPlantIndex,\s*3,\s*kPlantPointReward,\s*0\}",
            r"\{kCatalogShovelIndex,\s*3,\s*kShovelPointReward,\s*0\}",
            r"\{kCatalogChestIndex,\s*6,\s*kChestPointReward,\s*0\}",
            r"\{kCatalogFishingPoleIndex,\s*4,\s*kFishingPolePointReward,\s*0\}",
            r"\{kCatalogLampIndex,\s*6,\s*kLampPointReward,\s*25\}",
            r"chestLampExclusiveCatalogItem",
        )
    )
    status = "CODED-SUBSTITUTE" if ok else "MISSING"
    return EvidenceRow(
        "EVID-016",
        "random_item_effects",
        "WIKI-ITEM-2024",
        "Plant, Shovel, Chest, Fishing Pole, and Lamp use random reward/penalty branches, including exclusive item outcomes.",
        "src/EchoPetModel.cpp::kCatalogRandomRules/chestLampExclusiveCatalogItem/useCatalogItem",
        "source-shaped branch counts, point rewards, lamp break chance, and exclusive-item pool are explicit",
        status,
        "Exact original probability tables, reward amounts, and full animation pacing remain substitute.",
    )


def rows() -> list[EvidenceRow]:
    probes: list[Callable[[], EvidenceRow]] = [
        probe_shop_secret,
        probe_shop_windows,
        probe_shop_sale,
        probe_game_top_prizes,
        probe_game_lengths,
        probe_toothache,
        probe_attention_windows,
        probe_death_substitute,
        probe_inventory_caps,
        probe_default_foods,
        probe_wallet_cap,
        probe_king_donation,
        probe_password_secret_tables,
        probe_link_game_requirements,
        probe_item_dependencies,
        probe_random_item_effects,
    ]
    return [probe() for probe in probes]


def write_markdown(items: list[EvidenceRow]) -> None:
    lines = [
        "# EchoPet Source Evidence Probe",
        "",
        "Generated by `analysis/source_evidence_probe/generate_source_evidence_probe.py`.",
        "",
        "This report checks whether sourced Connection facts have explicit code",
        "targets. It does not close hardware rows or exact-source gaps by itself.",
        "",
        "## Summary",
        "",
    ]
    counts: dict[str, int] = {}
    for item in items:
        counts[item.status] = counts.get(item.status, 0) + 1
    for status in sorted(counts):
        lines.append(f"- `{status}`: `{counts[status]}`")
    lines.extend(
        [
            "",
            "## Rows",
            "",
            "| ID | Status | Domain | Sources | Claim | Code target | Evidence | Remaining gap |",
            "| --- | --- | --- | --- | --- | --- | --- | --- |",
        ]
    )
    for item in items:
        lines.append(
            "| {row_id} | `{status}` | {domain} | {source_ids} | {claim} | `{code_target}` | {code_evidence} | {remaining_gap} |".format(
                **asdict(item)
            )
        )
    lines.append("")
    (OUT_DIR / "source_evidence_probe.md").write_text("\n".join(lines), encoding="utf-8")


def main() -> None:
    items = rows()
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    (OUT_DIR / "source_evidence_probe.json").write_text(
        json.dumps([asdict(item) for item in items], indent=2, ensure_ascii=False),
        encoding="utf-8",
    )
    with (OUT_DIR / "source_evidence_probe.csv").open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(handle, fieldnames=list(asdict(items[0]).keys()))
        writer.writeheader()
        for item in items:
            writer.writerow(asdict(item))
    write_markdown(items)
    print(f"Wrote {len(items)} source evidence rows to {OUT_DIR}")


if __name__ == "__main__":
    main()
