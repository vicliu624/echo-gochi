from __future__ import annotations

import re
from collections import Counter
from dataclasses import dataclass
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
SOURCE_ROOT = ROOT / "src" if (ROOT / "src").exists() else ROOT
OUT = ROOT / "analysis" / "hardware_readiness" / "hardware_readiness.md"

HARDWARE = ROOT / "T_ECHO_LITE_HARDWARE_ACCEPTANCE.md"
INO = SOURCE_ROOT / "EchoPet.ino"
INPUT_CPP = SOURCE_ROOT / "EchoPetInput.cpp"
DISPLAY_CPP = SOURCE_ROOT / "EchoPetDisplay.cpp"
SPRITES_H = SOURCE_ROOT / "EchoPetSprites.h"
SPRITES_CPP = SOURCE_ROOT / "EchoPetSprites.cpp"
MODEL_CPP = SOURCE_ROOT / "EchoPetModel.cpp"
UI_CPP = SOURCE_ROOT / "EchoPetUi.cpp"
CATALOG_H = SOURCE_ROOT / "EchoPetCatalog.h"
CATALOG_CPP = SOURCE_ROOT / "EchoPetCatalog.cpp"
LINK_CONTRACT = ROOT / "LINK_CONTRACT.md"
PARITY_REPORT = ROOT / "analysis" / "parity_coverage" / "parity_resource_coverage.md"
GROWTH_REPORT = ROOT / "analysis" / "growth_coverage" / "growth_coverage.md"


@dataclass
class HardwareTodo:
    line: int
    section: str
    name: str
    expected: str


@dataclass
class Readiness:
    status: str
    evidence: str
    remaining_gate: str


def read(path: Path) -> str:
    return path.read_text(encoding="utf-8")


TEXTS = {
    "ino": read(INO),
    "input": read(INPUT_CPP),
    "display": read(DISPLAY_CPP),
    "sprites_h": read(SPRITES_H),
    "sprites_cpp": read(SPRITES_CPP),
    "model": read(MODEL_CPP),
    "ui": read(UI_CPP),
    "catalog_h": read(CATALOG_H),
    "catalog": read(CATALOG_CPP),
    "link": read(LINK_CONTRACT),
    "parity": read(PARITY_REPORT),
    "growth": read(GROWTH_REPORT),
}


def has(text_key: str, *patterns: str) -> bool:
    text = TEXTS[text_key]
    return all(pattern in text for pattern in patterns)


def has_any(text_key: str, *patterns: str) -> bool:
    text = TEXTS[text_key]
    return any(pattern in text for pattern in patterns)


def md_cell(value: str) -> str:
    return value.replace("|", "\\|")


def parse_hardware_todos() -> list[HardwareTodo]:
    rows: list[HardwareTodo] = []
    section = ""
    for number, line in enumerate(read(HARDWARE).splitlines(), start=1):
        if line.startswith("## "):
            section = line[3:].strip()
            continue
        if not line.startswith("|"):
            continue
        cells = [cell.strip() for cell in line.strip("|").split("|")]
        if len(cells) < 3:
            continue
        if cells[0] in {"Case", "Flow", "Frame family", "---"}:
            continue
        if not any(cell == "TODO" for cell in cells):
            continue
        name = cells[0].strip("`")
        if section == "Sprite Proof Acceptance":
            expected = "64x64 and 128x128 sprite proof pass"
        else:
            expected = cells[1]
        rows.append(HardwareTodo(number, section, name, expected))
    return rows


def ready_if(ok: bool, evidence: str, remaining_gate: str,
             partial_reason: str | None = None) -> Readiness:
    if ok and partial_reason:
        return Readiness("PARTIAL", evidence.rstrip(".") + "; " + partial_reason,
                         remaining_gate)
    if ok:
        return Readiness("READY", evidence, remaining_gate)
    return Readiness("MISSING", "No matching source evidence found.",
                     "SOURCE before HW")


def button_readiness(name: str) -> Readiness:
    common_short = has("input", "buttonEvent", "ConnectButtonEvent::kA",
                       "ConnectButtonEvent::kB", "ConnectButtonEvent::kC")
    keyshield_ready = has("input", "kTcaKeyReleasedMask",
                          "(raw & kTcaKeyReleasedMask) == 0",
                          "kRegDebounceDisable1", "kRegDebounceDisable2",
                          "kRegDebounceDisable3",
                          "kCfgInterruptPulse",
                          "kCfgFifoOverflowInterrupt",
                          "kCfgKeyEventInterrupt")
    hold_ready = has("input", "kLongPressMs", "tcaLongSent",
                     "result = buttonEvent(index, true)") and keyshield_ready
    release_suppressed = has("input", "alreadySentLong",
                             "if (!alreadySentLong)")
    if name == "Esc short press":
        ok = common_short and keyshield_ready and has("input", "kKeyEsc = 42") and has(
            "ino", "selectNextAction()", "echopet::uiNext(ui, pet.snapshot())")
        return ready_if(
            ok,
            "`EchoPetInput.cpp` maps Esc to A with TCA8418 bit7 treated as release and debounce enabled; `EchoPet.ino` routes A to fixed-icon/menu cursor advance.",
            "HW button repeat / debounce observation",
        )
    if name == "Home short press":
        ok = common_short and keyshield_ready and has("input", "kKeyHome = 43") and has(
            "ino", "activateSelectedAction()", "activateUiSelection()")
        return ready_if(
            ok,
            "`EchoPetInput.cpp` maps Home to B with TCA8418 bit7 treated as release and debounce enabled; `EchoPet.ino` routes B to action/menu confirmation.",
            "HW button repeat / debounce observation",
        )
    if name == "Email short press":
        ok = common_short and keyshield_ready and has("input", "kKeyMail = 44") and has(
            "ino", "showStatus()", "echopet::uiExit(ui)")
        return ready_if(
            ok,
            "`EchoPetInput.cpp` maps Email to C with TCA8418 bit7 treated as release and debounce enabled; `EchoPet.ino` routes C to status/back/cancel paths.",
            "HW button repeat / debounce observation",
        )
    if name == "Esc long press":
        ok = hold_ready and release_suppressed and has(
            "input", "ConnectButtonEvent::kALong")
        return ready_if(
            ok,
            "`kALong` is emitted at `kLongPressMs`; release-after-hold short event is suppressed.",
            "HW long-threshold timing observation",
        )
    if name == "Home long press":
        ok = hold_ready and release_suppressed and has(
            "ino", "case ConnectButtonEvent::kBLong", "enterClockSetMode()",
            "ui.mode != UiMode::kClockSet && pet.tick(nowMs)")
        return ready_if(
            ok,
            "`kBLong` enters CLOCK from home; model ticking is skipped while CLOCK edit is active.",
            "HW long-threshold timing observation",
        )
    if name == "Email long press":
        ok = hold_ready and release_suppressed and has(
            "ino", "case ConnectButtonEvent::kCLong",
            "enterUiMode(UiMode::kSpriteProof)")
        return ready_if(
            ok,
            "`kCLong` enters the sprite proof diagnostic path from home.",
            "HW long-threshold timing observation",
        )
    if name == "Release timing":
        ok = hold_ready and release_suppressed
        return ready_if(
            ok,
            "`tcaLongSent` and `alreadySentLong` suppress a trailing short event after HOLD.",
            "HW release timing observation",
        )
    return Readiness("MISSING", "No button readiness rule.", "SOURCE before HW")


def refresh_pipeline_ready() -> bool:
    return has("ino", "frameChanged", "waitForDisplaySubmitInterval",
               "setRAMValueBaseMap", "PARTIAL_REFRESH",
               "constexpr uint32_t kDisplaySubmitGuardMs = 50;")


def refresh_readiness(name: str) -> Readiness:
    pipeline = refresh_pipeline_ready()
    if name == "Cold boot":
        ok = has("ino", "beginDisplay();", "display.begin()",
                 "display.setRotation(1)", "display.setTextWrap(false)",
                 "display.fillScreen(EPD_WHITE)", "display.clearBuffer()",
                 "render(true)") and has("display", "drawMenuColumn",
                                          "drawMainFrame")
        return ready_if(ok,
                        "`setup()` initializes display through `beginDisplay()`, clears the EPD buffer, then renders one full boot frame with fixed menus.",
                        "HW boot-frame observation")
    if name == "Idle 5 minutes":
        ok = pipeline and has("ino", "shouldRunIdleAnimation",
                              "refreshDisplay(fullRefresh)")
        return ready_if(ok,
                        "Refresh path uses frame diff, the Trail-Mate-style 50 ms submit interval, and synchronous whole-frame partial submit after initial full refresh.",
                        "HW whitening / ghosting observation")
    if name == "Menu cycling 50 presses":
        ok = pipeline and has("ino", "selectNextAction",
                              "selectedMenuIndex") and has(
                                  "display", "drawMenuColumn",
                                  "drawMenuIcon",
                                  "kEchoPetMenuIcons64Compact",
                                  "kEchoPetMenuIcons128Large")
        return ready_if(ok,
                        "Fixed-icon selection changes redraw through the partial-refresh pipeline.",
                        "HW high-rate button + panel pacing observation")
    if name == "Notice/result messages":
        ok = pipeline and has("ino", "kNoticeMinVisibleMs",
                              "maybeClearRenderedNotice")
        return ready_if(ok,
                        "Notice clear waits for a minimum visible interval after the synchronous refresh path has rendered it.",
                        "HW notice duration observation")

    scene_rules = {
        "Food scene": (
            has("display", "drawFoodPage", "drawFoodPlate",
                "SpriteFrame::kFoodCrumbs", "SpriteFrame::kFoodDone",
                "SpriteFrame::kFoodRefuse") and has(
                    "ino", "kFoodSceneAnimationMs", "case UiMode::kMeal:",
                    "case UiMode::kSnack:", "return kFoodSceneAnimationMs;"),
            "Food/snack scene has selected prop, bite travel, crumbs, done, refusal routing, and explicit meal/snack pacing.",
        ),
        "Toilet scene": (
            has("display", "drawToiletScene", "visibleMesses",
                "SpriteFrame::kToiletSweep0", "SpriteFrame::kToiletSweep1",
                "SpriteFrame::kToiletDone") and has(
                    "ino", "kToiletSceneAnimationMs",
                    "return kToiletSceneAnimationMs;"),
            "Toilet scene has visible mess, sweep, progressive removal, done, no-mess routing, and explicit scene pacing.",
        ),
        "Medicine scene": (
            has("display", "drawMedicineScene", "drawMedicineBottle",
                "drawDoseTrail", "SpriteFrame::kMedicineRecover",
                "SpriteFrame::kMedicineRefuse") and has(
                    "ino", "kMedicineSceneAnimationMs",
                    "return kMedicineSceneAnimationMs;"),
            "Medicine scene has sick/tooth, dose, recover, refusal routing, and explicit scene pacing.",
        ),
        "Discipline scene": (
            has("display", "drawDisciplineScene",
                "SpriteFrame::kAttentionCall",
                "SpriteFrame::kAttentionMissed",
                "SpriteFrame::kDisciplineInvalid") and has(
                    "ino", "kDisciplineSceneAnimationMs",
                    "return kDisciplineSceneAnimationMs;"),
            "Discipline scene has TIME OUT, PRAISE, invalid, call, missed routing, and explicit scene pacing.",
        ),
        "Lights dark-room": (
            has("display", "drawLightsScene", "drawDarkRoomPattern",
                "SpriteFrame::kLightsSelectorOff",
                "SpriteFrame::kLightsWake",
                "SpriteFrame::kLightsInvalid") and has(
                    "ino", "kLightsSceneAnimationMs",
                    "return kLightsSceneAnimationMs;"),
            "Lights scene uses low-ink dark-room pattern, ON/OFF selector, sleep/wake/invalid routing, and explicit scene pacing.",
        ),
        "Shop scene": (
            has("display", "drawShopPage", "SpriteFrame::kShopBooth",
                "SpriteFrame::kShopkeeperIdle",
                "SpriteFrame::kShopItemPreview",
                "shopResultFrame") and has("ino", "kCatalogSceneAnimationMs",
                                            "return kCatalogSceneAnimationMs;"),
            "Shop scene has booth, shopkeeper, preview, buy-result routing, and explicit catalog pacing.",
        ),
        "Password entry": (
            has("display", "drawPasswordPage") and has(
                "model", "enterPassword", "enterSecretCode") and has(
                    "ino", "kCatalogSceneAnimationMs",
                    "case UiMode::kPassword:"),
            "Password and shop-secret entry have digit/symbol editing, result handling, and explicit catalog pacing.",
        ),
    }
    if name in scene_rules:
        ok, evidence = scene_rules[name]
        return ready_if(ok and pipeline, evidence,
                        "HW scene pacing / readability observation")

    game_frames = {
        "Get game": ("MiniGameKind::kGet", "SpriteFrame::kGameGetNote",
                     "SpriteFrame::kGameGetBad"),
        "Bump game": ("MiniGameKind::kBump", "SpriteFrame::kGameBumpMeter",
                      "SpriteFrame::kGameBumpPush"),
        "Flag game": ("MiniGameKind::kFlag", "SpriteFrame::kGameFlagLeft",
                      "SpriteFrame::kGameFlagRight"),
        "Heading game": ("MiniGameKind::kHeading",
                         "SpriteFrame::kGameHeadingBall"),
        "Memory game": ("MiniGameKind::kMemory",
                        "SpriteFrame::kGameMemoryReveal",
                        "SpriteFrame::kGameMemoryCursor"),
        "Sprint game": ("MiniGameKind::kSprint",
                        "SpriteFrame::kGameSprintRunner0",
                        "SpriteFrame::kGameSprintRunner1"),
        "Hoops game": ("MiniGameKind::kHoops",
                       "SpriteFrame::kGameHoopsHoop",
                       "SpriteFrame::kGameHoopsShoot"),
    }
    if name in game_frames:
        ok = pipeline and has("display", "drawGameScreen", *game_frames[name])
        ok = ok and has("model", "gameInput", "finishMiniGame")
        return ready_if(
            ok,
            f"{name} has model input/result handling and display sprite routing; active game target intervals are reported in parity coverage.",
            "HW active-game speed / readability observation",
        )
    return Readiness("MISSING", "No refresh readiness rule.", "SOURCE before HW")


def parse_sprite_enum() -> set[str]:
    match = re.search(r"enum class SpriteFrame[^{]*\{(.*?)\};",
                      TEXTS["sprites_h"], re.S)
    if not match:
        return set()
    names: set[str] = set()
    for raw in match.group(1).splitlines():
        token = raw.split("//", 1)[0].strip().rstrip(",")
        if token.startswith("k"):
            names.add(token)
    return names


SPRITE_ENUM = parse_sprite_enum()


SPRITE_FAMILIES: dict[str, tuple[list[str], str | None]] = {
    "egg.*": (
        ["kEgg0", "kEgg1", "kEggCrack0", "kEggCrack1", "kEggHatch"],
        None,
    ),
    "baby.*": (["kBaby0", "kBaby1"], None),
    "child.*": (["kChild0", "kChild1"], None),
    "teen.*": (["kTeen0", "kTeen1"], None),
    "adult.mametchi.*": (["kAdult0", "kAdult1"], None),
    "food.*": (
        ["kEatMeal0", "kEatMeal1", "kEatSnack0", "kEatSnack1",
         "kFoodCrumbs", "kFoodRefuse", "kFoodDone"],
        None,
    ),
    "toilet.*": (
        ["kToiletMess", "kToiletSweep0", "kToiletSweep1",
         "kToiletDone", "kToiletNoMess"],
        None,
    ),
    "medicine.*": (
        ["kMedicineSickSkull", "kMedicineSickTooth", "kMedicineDose0",
         "kMedicineDose1", "kMedicineRecover", "kMedicineRefuse"],
        None,
    ),
    "discipline.*": (
        ["kDisciplineTimeout", "kDisciplinePraise", "kDisciplineInvalid",
         "kAttentionCall", "kAttentionMissed"],
        None,
    ),
    "lights.*": (
        ["kLightsOn", "kLightsOff", "kLightsSelectorOn",
         "kLightsSelectorOff", "kLightsWake", "kLightsInvalid"],
        None,
    ),
    "shop.*": (
        ["kShopBooth", "kShopkeeperIdle", "kShopkeeperSurprise",
         "kShopkeeperHappy", "kShopItemPreview", "kShopBuyOk",
         "kShopBuyNoMoney", "kShopBuyFull", "kShopSoldOut"],
        None,
    ),
    "game.get.*": (
        ["kGameGetNote", "kGameGetBad", "kGameGetCatch", "kGameGetMiss"],
        None,
    ),
    "game.bump.*": (
        ["kGameBumpMeter", "kGameBumpPush", "kGameBumpFall"],
        None,
    ),
    "game.flag.*": (
        ["kGameFlagLeft", "kGameFlagRight", "kGameFlagBoth",
         "kGameFlagGood", "kGameFlagMiss"],
        None,
    ),
    "game.heading.*": (
        ["kGameHeadingBall", "kGameHeadingHit", "kGameHeadingMiss"],
        None,
    ),
    "game.memory.*": (
        ["kGameMemoryReveal", "kGameMemoryCursor", "kGameMemoryGood",
         "kGameMemoryWrong"],
        None,
    ),
    "game.sprint.*": (
        ["kGameSprintRunner0", "kGameSprintRunner1", "kGameSprintFinish"],
        None,
    ),
    "game.hoops.*": (
        ["kGameHoopsHoop", "kGameHoopsShoot", "kGameHoopsMade",
         "kGameHoopsMiss"],
        None,
    ),
    "love.*": (["kLovePartner", "kLoveBaby"], None),
    "family.*": (["kLoveBaby", "kParentDepart"], None),
    "passaway": (["kPassed"], None),
}


def sprite_readiness(name: str) -> Readiness:
    frames, partial_reason = SPRITE_FAMILIES.get(name, ([], None))
    missing = [frame for frame in frames if frame not in SPRITE_ENUM]
    plumbing_ok = has("parity", "Missing renderer map: `none`",
                      "Missing proof label: `none`") and has(
                          "display", "drawSpriteProofPage",
                          "kSpriteProofFrameCount")
    if missing:
        return Readiness("MISSING", f"Missing SpriteFrame enum rows: {', '.join(missing)}.",
                         "SOURCE before HW")
    evidence = (
        f"Frames {', '.join('`' + frame + '`' for frame in frames)} are enum/render/proof labelled; "
        "`Mail/C HOLD` opens sprite proof."
    )
    return ready_if(plumbing_ok, evidence,
                    "HW 64x64 and 128x128 sprite-proof observation",
                    partial_reason)


def link_readiness(name: str) -> Readiness:
    base = has("link", "## Shared Visible Sequence", "## Visit Flow",
               "## Present Flow", "## Connection Game Flow",
               "## Love / Partner / Baby Flow", "## LoRa Equivalence Rule")
    source_window = has("ino", "acceptIncomingFriendPacket",
                        "packet.kind != ui.entry[0]",
                        "emitLinkPacketForSelection(kind, replySelection, true)")
    if name == "Visit success":
        ok = base and source_window and has("model", "case LinkKind::kVisit",
                                            "Notice::kFriendVisit")
        return ready_if(ok,
                        "LINK contract plus source Visit receive/reply path are present.",
                        "DUAL/HW two-device visible-flow observation")
    if name == "Visit timeout":
        ok = base and has("ino", "kLinkStandbyTimeoutMs",
                          "updateLinkStandby",
                          "enterLinkResult(kLinkUiTimeout)")
        return ready_if(ok,
                        "30 s standby timeout enters TIMEOUT result without receive mutation.",
                        "DUAL/HW timeout observation")
    if name == "Present success":
        ok = base and source_window and has("model", "case LinkKind::kPresent",
                                            "GiftKind::kCatalogItem",
                                            "receiveCatalogGift(packet.giftId, inventoryFull)",
                                            "Notice::kFriendPresent")
        return ready_if(ok,
                        "Present packets can carry exact catalog item ids, sender debit and receiver award paths are coded, and the result notice path is present.",
                        "DUAL/HW two-device visible-flow observation")
    if name == "Present full inventory":
        ok = has("model", "case LinkKind::kPresent",
                 "notice_ = Notice::kInventoryFull",
                 "catalogStockTotal(CatalogKind::kFood) >= kMaxFoodStock",
                 "catalogStockTotal(CatalogKind::kItem) >= kMaxItemStock",
                 "bool EchoPetModel::receiveCatalogGift")
        return ready_if(
            ok,
            "Received catalog Presents check receiver catalog stock before award and show `InventoryFull` instead of clamping silently.",
            "DUAL/HW receiver-full visible-flow proof",
        )
    if name == "Game success":
        ok = base and source_window and has("model", "case LinkKind::kGame",
                                            "validLinkGameKind(packet.gameKind)",
                                            "ownsLinkGameItem(game)",
                                            "packet.gameScore",
                                            "Notice::kFriendGame")
        return ready_if(ok,
                        "Connection Game packets carry game kind/score, require matching item ownership, and resolve through the model.",
                        "DUAL/HW two-device visible-flow observation")
    if name == "Love reject":
        ok = base and source_window and has("model", "Notice::kLoveRejected",
                                            "localAdult && remoteAdult")
        return ready_if(ok,
                        "Love compatibility gate and rejection notice are coded.",
                        "DUAL/HW two-device visible-flow observation")
    if name == "Love partner":
        ok = base and source_window and has("model", "Notice::kPartner",
                                            "RelationLevel::kPartner")
        return ready_if(ok,
                        "Love partner promotion and partner notice are coded.",
                        "DUAL/HW two-device visible-flow observation")
    if name == "Love baby":
        ok = base and source_window and has("model", "startBaby()",
                                            "Notice::kBaby",
                                            "parentLeaves")
        return ready_if(ok,
                        "Compatible Love can start baby/parent-care and next-generation flow.",
                        "DUAL/HW two-device visible-flow observation")
    if name == "LoRa parity":
        ok = base and has("ino", "sendLoraFriendPacket",
                          "pollLoraFriendPacket",
                          "acceptIncomingFriendPacket(packet, \"LoRa\")")
        return ready_if(ok,
                        "Serial and LoRa carriers share `FriendPacket` and `acceptIncomingFriendPacket()`.",
                        "DUAL/HW LoRa equivalence observation")
    return Readiness("MISSING", "No LINK readiness rule.", "SOURCE before HW")


def readiness_for(row: HardwareTodo) -> Readiness:
    if row.section == "Button Acceptance":
        return button_readiness(row.name)
    if row.section == "Refresh Acceptance":
        return refresh_readiness(row.name)
    if row.section == "Sprite Proof Acceptance":
        return sprite_readiness(row.name)
    if row.section == "Two-Device Acceptance":
        return link_readiness(row.name)
    return Readiness("MISSING", "No readiness rule for this section.",
                     "SOURCE before HW")


def main() -> None:
    todos = parse_hardware_todos()
    rows = [(todo, readiness_for(todo)) for todo in todos]
    counts = Counter(readiness.status for _, readiness in rows)
    gate_counts = Counter(readiness.remaining_gate.split()[0]
                          for _, readiness in rows)

    lines = [
        "# EchoPet Hardware Readiness Audit",
        "",
        "Generated by `analysis/hardware_readiness/generate_hardware_readiness.py`.",
        "",
        "This is a source-side readiness report for the rows still marked",
        "`TODO` in `T_ECHO_LITE_HARDWARE_ACCEPTANCE.md`. It deliberately does",
        "not convert hardware observations into PASS. It only answers whether",
        "the firmware has enough source/spec hooks to run the physical test.",
        "",
        "## Summary",
        "",
        f"- Hardware acceptance rows still requiring observation: `{len(todos)}`.",
        f"- Source readiness `READY`: `{counts.get('READY', 0)}`.",
        f"- Source readiness `PARTIAL`: `{counts.get('PARTIAL', 0)}`.",
        f"- Source readiness `MISSING`: `{counts.get('MISSING', 0)}`.",
        f"- Remaining gates observed in this report: `{', '.join(f'{k}:{v}' for k, v in sorted(gate_counts.items()))}`.",
        "",
        "## Readiness Rows",
        "",
        "| HW log line | Section | Case | Source readiness | Evidence | Remaining gate |",
        "| ---: | --- | --- | --- | --- | --- |",
    ]
    for todo, readiness in rows:
        lines.append(
            f"| {todo.line} | {md_cell(todo.section)} | `{md_cell(todo.name)}` | "
            f"`{readiness.status}` | {md_cell(readiness.evidence)} | "
            f"{md_cell(readiness.remaining_gate)} |"
        )

    lines += [
        "",
        "## Interpretation",
        "",
        "- `READY` means the source/spec/render path exists and the remaining work is a real device observation.",
        "- `PARTIAL` means the path can be exercised, but the current source still has a known functional ambiguity.",
        "- `MISSING` means the row should not be sent to hardware yet because source evidence is absent.",
        "- Official-pixel or rights-cleared final art gaps are tracked by `VISUAL_REFERENCE_CONTRACT.md` and the parity report, not by this hardware-readiness proof gate.",
        "",
        "This report preserves the user's current constraint: no flashing was",
        "attempted or required to generate it.",
    ]

    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text("\n".join(lines) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()

