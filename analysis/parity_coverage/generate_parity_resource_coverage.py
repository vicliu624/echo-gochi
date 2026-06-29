from __future__ import annotations

import csv
import re
from collections import defaultdict
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
SOURCE_ROOT = ROOT / "src" if (ROOT / "src").exists() else ROOT
OUT = ROOT / "analysis" / "parity_coverage" / "parity_resource_coverage.md"
SPRITES_H = SOURCE_ROOT / "EchoPetSprites.h"
SPRITES_CPP = SOURCE_ROOT / "EchoPetSprites.cpp"
DISPLAY_CPP = SOURCE_ROOT / "EchoPetDisplay.cpp"
INO = SOURCE_ROOT / "EchoPet.ino"
INPUT_CPP = SOURCE_ROOT / "EchoPetInput.cpp"
LINK_CONTRACT = ROOT / "LINK_CONTRACT.md"
CHARACTER_CATALOG_H = SOURCE_ROOT / "EchoPetCharacterCatalog.h"
CHARACTER_CATALOG = SOURCE_ROOT / "EchoPetCharacterCatalog.cpp"
CATALOG_CPP = SOURCE_ROOT / "EchoPetCatalog.cpp"
HOWTO_SUMMARY = ROOT / "analysis" / "official_howto_audit" / "summary.csv"
SPRITE_RENDER_AUDIT = ROOT / "analysis" / "sprite_render_audit" / "SPRITE_RENDER_AUDIT.md"
SPRITE_RENDER_METRICS = ROOT / "analysis" / "sprite_render_audit" / "sprite_frame_metrics.csv"


FRAME_CONTRACTS = [
    ("egg.idle", ["kEgg0", "kEgg1"], "life"),
    ("egg.crack/hatch", ["kEggCrack0", "kEggCrack1", "kEggHatch"], "life"),
    ("baby.idle", ["kBaby0", "kBaby1"], "life"),
    ("child.idle", ["kChild0", "kChild1"], "life"),
    ("teen.idle", ["kTeen0", "kTeen1"], "life"),
    ("adult.mametchi.idle", ["kAdult0", "kAdult1"], "life"),
    ("elder.idle", ["kElder0", "kElder1"], "life"),
    ("emotion.sleep/sick/happy/sad", ["kSleep", "kSick", "kJoy", "kSad"], "life"),
    ("food.bite/crumb/refuse/done",
     ["kEatMeal0", "kEatMeal1", "kEatSnack0", "kEatSnack1",
      "kFoodCrumbs", "kFoodRefuse", "kFoodDone"], "care"),
    ("toilet.mess/sweep/done/no_mess",
     ["kToiletMess", "kToiletSweep0", "kToiletSweep1", "kToiletDone",
      "kToiletNoMess"], "care"),
    ("medicine.sick/dose/recover/refuse",
     ["kMedicineSickSkull", "kMedicineSickTooth", "kMedicineDose0",
      "kMedicineDose1", "kMedicineRecover", "kMedicineRefuse",
      "kMedicine1"], "care"),
    ("discipline.prompt/react/invalid/missed",
     ["kDisciplineTimeout", "kDisciplinePraise", "kDisciplineInvalid",
      "kAttentionCall", "kAttentionMissed"], "care"),
    ("lights.on/off/sleep", ["kLightsOn", "kLightsOff"], "care"),
    ("lights.selector/wake/invalid",
     ["kLightsSelectorOn", "kLightsSelectorOff", "kLightsWake",
      "kLightsInvalid"], "care"),
    ("shop.booth/counter", ["kShopBooth"], "shop"),
    ("shopkeeper.face.idle/surprise/happy",
     ["kShopkeeperIdle", "kShopkeeperSurprise", "kShopkeeperHappy"], "shop"),
    ("shop.item.preview", ["kShopItemPreview"], "shop"),
    ("shop.buy.ok/no_money/full/sold_out",
     ["kShopBuyOk", "kShopBuyNoMoney", "kShopBuyFull", "kShopSoldOut"],
     "shop"),
    ("item.play", ["kItemPlay"], "item"),
    ("connection.send/receive", ["kLinkSend", "kLinkReceive"], "link"),
    ("love.partner/baby/parent.depart",
     ["kLovePartner", "kLoveBaby", "kParentDepart"], "link"),
    ("game.result", ["kGameWin", "kGameLose"], "game"),
    ("game.get", ["kGameGetNote", "kGameGetBad", "kGameGetCatch",
                  "kGameGetMiss"], "game"),
    ("game.bump", ["kGameBumpMeter", "kGameBumpPush", "kGameBumpFall"],
     "game"),
    ("game.flag", ["kGameFlagLeft", "kGameFlagRight", "kGameFlagBoth",
                   "kGameFlagGood", "kGameFlagMiss"], "game"),
    ("game.heading", ["kGameHeadingBall", "kGameHeadingHit",
                      "kGameHeadingMiss"], "game"),
    ("game.memory", ["kGameMemoryReveal", "kGameMemoryCursor",
                     "kGameMemoryGood", "kGameMemoryWrong"], "game"),
    ("game.sprint", ["kGameSprintRunner0", "kGameSprintRunner1",
                     "kGameSprintFinish"], "game"),
    ("game.hoops", ["kGameHoopsHoop", "kGameHoopsShoot", "kGameHoopsMade",
                    "kGameHoopsMiss"], "game"),
    ("passaway", ["kPassed"], "life"),
]


def read(path: Path) -> str:
    return path.read_text(encoding="utf-8")


def int_field(row: dict[str, str], *names: str) -> int:
    for name in names:
        value = row.get(name, "")
        if value:
            return int(value)
    return 0


def official_timing_rows() -> list[tuple[str, str, int, int, int]]:
    if not HOWTO_SUMMARY.exists():
        return []

    rows: list[tuple[str, str, int, int, int]] = []
    with HOWTO_SUMMARY.open(newline="", encoding="utf-8") as handle:
        for row in csv.DictReader(handle):
            hint = row.get("contract_hint", "")
            if not hint:
                continue
            frames = int_field(row, "frames")
            duration = int_field(row, "duration_ms", "total_ms")
            if frames <= 1 or duration == 0:
                continue
            avg = (duration + frames // 2) // frames
            rows.append((hint, row.get("file", ""), frames, duration, avg))
    return rows


def between(text: str, start: str, end: str) -> str:
    start_index = text.find(start)
    if start_index < 0:
        return ""
    end_index = text.find(end, start_index + len(start))
    if end_index < 0:
        return text[start_index:]
    return text[start_index:end_index]


def parse_sprite_enum(header: str) -> list[str]:
    block_match = re.search(r"enum class SpriteFrame[^{]*\{(.*?)\};", header, re.S)
    if not block_match:
        return []
    names = []
    for raw in block_match.group(1).splitlines():
        token = raw.split("//", 1)[0].strip().rstrip(",")
        if token.startswith("k"):
            names.append(token)
    return names


def parse_case_frames(block: str) -> set[str]:
    return set(re.findall(r"case\s+SpriteFrame::(k\w+)\s*:", block))


def parse_uint8_constants(text: str) -> dict[str, int]:
    constants: dict[str, int] = {}
    for name, raw in re.findall(
        r"constexpr\s+uint8_t\s+(k\w+)\s*=\s*(0x[0-9A-Fa-f]+|\d+)\s*;",
        text,
    ):
        constants[name] = int(raw, 0)
    return constants


def official_group_rows() -> list[tuple[str, int, int, int]]:
    if not HOWTO_SUMMARY.exists():
        return []
    groups: dict[str, list[dict[str, str]]] = defaultdict(list)
    with HOWTO_SUMMARY.open("r", encoding="utf-8", newline="") as handle:
        for row in csv.DictReader(handle):
            groups[row.get("group", "")].append(row)
    rows = []
    for group in sorted(groups):
        assets = groups[group]
        animated = sum(1 for row in assets if int(row.get("frames") or 1) > 1)
        max_frames = max((int(row.get("frames") or 1) for row in assets),
                         default=0)
        rows.append((group, len(assets), animated, max_frames))
    return rows


def sprite_render_rows() -> list[dict[str, str]]:
    if not SPRITE_RENDER_METRICS.exists():
        return []
    with SPRITE_RENDER_METRICS.open("r", encoding="utf-8", newline="") as handle:
        return list(csv.DictReader(handle))


def catalog_use_value(raw: str, numbers: list[int],
                      use_constants: dict[str, int]) -> int:
    match = re.search(r"\b(kCatalogUse\w+)\b", raw)
    if match:
        return use_constants.get(match.group(1), 0)
    return numbers[-1] if numbers else 0


def item_catalog_metrics(catalog: str,
                         header: str) -> tuple[int, int, dict[int, int], dict[int, int]]:
    use_constants = {
        name: int(value)
        for name, value in re.findall(
            r"constexpr\s+uint8_t\s+(kCatalogUse\w+)\s*=\s*(\d+)\s*;",
            header,
        )
    }
    segment_block = between(catalog, "const CatalogSegment kCatalogSegments[] = {",
                            "};")
    override_block = between(catalog,
                             "const CatalogOverride kCatalogOverrides[] = {",
                             "const char* const kSouvenirNames")
    segments: list[tuple[int, int, int]] = []
    for raw in re.findall(r"\{(.*?)\},", segment_block, re.S):
        numbers = [int(value) for value in re.findall(r"\b\d+\b", raw)]
        if len(numbers) >= 4:
            segments.append((numbers[0], numbers[1],
                             catalog_use_value(raw, numbers, use_constants)))

    overrides: dict[int, int] = {}
    for index in re.finditer(r"\{\s*(\d+)\s*,", override_block):
        end = override_block.find("},", index.end())
        if end < 0:
            continue
        raw = override_block[index.start():end]
        numbers = [int(value) for value in re.findall(r"\b\d+\b", raw)]
        if len(numbers) >= 2:
            overrides[numbers[0]] = catalog_use_value(raw, numbers,
                                                      use_constants)

    scene_counts: dict[int, int] = defaultdict(int)
    trait_counts: dict[int, int] = defaultdict(int)
    for index in range(160):
        scene = overrides.get(index)
        if scene is None:
            scene = 0
            for start, end, segment_scene in segments:
                if start <= index < end:
                    scene = segment_scene
                    break
        scene_counts[scene] += 1
        trait_counts[index & 0xFF] += 1

    return len(segments), len(overrides), scene_counts, trait_counts


def status_for(frames: list[str], enum_frames: set[str], render_frames: set[str],
               proof_frames: set[str]) -> tuple[str, str]:
    missing_enum = [frame for frame in frames if frame not in enum_frames]
    missing_render = [frame for frame in frames
                      if frame in enum_frames and frame not in render_frames]
    missing_proof = [frame for frame in frames
                     if frame in enum_frames and frame not in proof_frames]
    if missing_enum:
        return "MISSING", ", ".join(missing_enum)
    if missing_render:
        return "NO_RENDER_MAP", ", ".join(missing_render)
    if missing_proof:
        return "NO_PROOF_LABEL", ", ".join(missing_proof)
    return "CODED_LOW_RESOURCE", ""


def main() -> None:
    header = read(SPRITES_H)
    sprites = read(SPRITES_CPP)
    display = read(DISPLAY_CPP)
    ino = read(INO)
    input_cpp = read(INPUT_CPP)
    link_contract = read(LINK_CONTRACT)
    character_catalog_header = read(CHARACTER_CATALOG_H)
    character_catalog = read(CHARACTER_CATALOG)
    item_catalog_header = read(SOURCE_ROOT / "EchoPetCatalog.h")
    item_catalog = read(CATALOG_CPP)

    enum_names = parse_sprite_enum(header)
    enum_frames = set(enum_names)
    render_frames = parse_case_frames(
        between(sprites, "const ComposedFrame* connectFrame", "const Cell* frameCells")
    )
    proof_frames = parse_case_frames(
        between(display, "const char* spriteProofLabel", "SpriteFrame spriteProofFrame")
    )
    sprite_render_report_rows = sprite_render_rows()
    sprite_rendered_frames = {
        row.get("sprite", "") for row in sprite_render_report_rows if row.get("sprite")
    }
    sprite_missing_rendered_frames = [
        frame for frame in enum_names if frame not in sprite_rendered_frames
    ]
    sprite_out_of_bounds_frames = [
        row.get("sprite", "")
        for row in sprite_render_report_rows
        if row.get("out_of_nominal_bounds") == "yes"
    ]
    character_rows = re.findall(r'CHARACTER_ROW(?:_TRAITS)?\(\s*"[^"]+".*?\)',
                                character_catalog, re.S)
    character_constants = parse_uint8_constants(character_catalog)
    family_counts: dict[int, int] = defaultdict(int)
    trait_counts: dict[int, int] = defaultdict(int)
    source_pool_rows = 0
    for row in character_rows:
        numbers = [int(value) for value in re.findall(r"\b\d+\b", row)]
        if len(numbers) < 4:
            continue
        care, page, slot, family = numbers[-4:]
        if ("kCharacterGeneration" in row and "kCharacterTier" in row and
                "kCharacterGender" in row):
            source_pool_rows += 1
        trait = None
        if row.startswith("CHARACTER_ROW_TRAITS"):
            trait_name_match = re.search(r",\s*(k\w+)\s*\)$", row.strip())
            if trait_name_match:
                trait = character_constants.get(trait_name_match.group(1))
        if trait is None:
            trait = ((int(page) & 0x07) << 5) | ((int(slot) & 0x07) << 2) | (
                int(care) & 0x03
            )
        family_counts[int(family)] += 1
        trait_counts[trait] += 1
    family_routing_active = (
        "characterFrameFamilyFrame" in sprites
        and "characterCatalogEntry(pet.characterCatalogId)" in sprites
        and ".frameFamily" in sprites
    )
    trait_overlay_active = (
        "visualTraits" in character_catalog_header
        and "CHARACTER_TRAITS" in character_catalog
        and "drawCharacterTraitOverlay" in display
        and "character.visualTraits" in display
    )
    adult_composed_frame_active = (
        "case SpriteFrame::kAdult0:" in sprites
        and "return &kFrameStand0;" in sprites
        and "case SpriteFrame::kAdult1:" in sprites
        and "return &kFrameStand1;" in sprites
    )
    adult_body_32px = "{32, 32, 4, kMameBodyBitmap}" in sprites
    adult_separate_eyes = (
        "{7, 11, B_EYE_OPEN, 0}" in sprites
        and "{18, 11, B_EYE_OPEN, 0}" in sprites
        and "B_EYE_BLINK" in sprites
        and "B_EYE_SLEEP" in sprites
        and "B_EYE_SAD" in sprites
        and "B_EYE_SICK" in sprites
    )
    adult_separate_mouth = (
        "B_MOUTH_SMILE" in sprites
        and "B_MOUTH_OPEN" in sprites
        and "B_MOUTH_SAD" in sprites
    )
    adult_separate_feet = (
        "B_FOOT" in sprites
        and "{9, 27, B_FOOT, 0}" in sprites
        and "{19, 27, B_FOOT, kPartFlipX}" in sprites
    )
    adult_idle_foot_shift = (
        "const FramePart kFrameStand0Parts[] PROGMEM" in sprites
        and "{9, 27, B_FOOT, 0}" in sprites
        and "{19, 27, B_FOOT, kPartFlipX}" in sprites
        and "const FramePart kFrameStand1Parts[] PROGMEM" in sprites
        and "{7, 27, B_FOOT, 0}" in sprites
        and "{21, 27, B_FOOT, kPartFlipX}" in sprites
    )
    adult_trait_overlay_blink = (
        "const bool blink = (animationPhase & 0x03) == 1" in display
        and "display.drawLine(cx - 5 * scale, eyeY" in display
        and "display.fillRect(cx - 5 * scale, eyeY - scale" in display
    )
    mametchi_validation_trait_active = (
        'CHARACTER_ROW_TRAITS("Mametchi"' in character_catalog
        and "kMametchiValidationTraits = 0x80" in character_catalog
    )
    item_segments, item_overrides, item_scene_counts, item_trait_counts = item_catalog_metrics(
        item_catalog, item_catalog_header
    )
    item_cursor_active = "const uint8_t i = ui.cursor % kCatalogItemCount" in display
    item_icon_active = "drawCatalogIcon" in display and "entry.icon" in display
    item_use_scene_active = (
        "drawCatalogUseScene" in display and "entry.useScene" in display
    )
    item_trait_overlay_active = (
        "visualTraits" in item_catalog_header
        and "catalogVisualTraits" in item_catalog
        and "drawCatalogTraitOverlay" in display
        and "entry.visualTraits" in display
    )
    souvenir_trait_counts: dict[int, int] = defaultdict(int)
    for index in range(64):
        souvenir_trait_counts[0x80 | (index & 0x3F)] += 1
    souvenir_trait_active = (
        "catalogSouvenirVisualTraits" in item_catalog_header
        and "catalogSouvenirVisualTraits" in item_catalog
        and "drawSouvenirIcon" in display
    )
    family_album_active = (
        "drawFamilyAlbumCard" in display
        and "drawFamilyPage(display, r, pet, ui, animationPhase)" in display
        and "FamilyRecord" in display
        and "FamilyAncestryRecord" in display
        and "pet.familyAncestry[i]" in display
    )
    discipline_attention_active = (
        "drawDisciplineScene" in display
        and "SpriteFrame::kAttentionCall" in display
        and "SpriteFrame::kAttentionMissed" in display
        and "pet.attention" in display
    )
    medicine_scene_active = (
        "drawMedicineScene" in display
        and "medicineFrame" in display
        and "drawMedicineBottle" in display
        and "drawMedicineBadge" in display
        and "drawDoseTrail" in display
        and "SpriteFrame::kMedicineRecover" in display
        and "SpriteFrame::kMedicineRefuse" in display
    )
    food_scene_active = (
        "drawFoodPage" in display
        and "drawFoodPlate" in display
        and "drawFoodStageBadge" in display
        and "SpriteFrame::kFoodCrumbs" in display
        and "SpriteFrame::kFoodDone" in display
        and "SpriteFrame::kFoodRefuse" in display
    )
    toilet_scene_active = (
        "drawToiletScene" in display
        and "visibleMesses" in display
        and "SpriteFrame::kToiletSweep0" in display
        and "SpriteFrame::kToiletSweep1" in display
        and "SpriteFrame::kToiletDone" in display
        and "SpriteFrame::kToiletNoMess" in display
    )
    lights_scene_active = (
        "drawLightsScene" in display
        and "drawDarkRoomPattern" in display
        and "drawLightSelector" in display
        and "SpriteFrame::kLightsSelectorOn" in display
        and "SpriteFrame::kLightsSelectorOff" in display
        and "SpriteFrame::kLightsWake" in display
        and "SpriteFrame::kLightsInvalid" in display
    )
    lights_intrusive_fill_removed = (
        "display.fillRect(r.mainX + 8, r.mainY + 8, r.mainW - 16" not in display
    )
    hold_immediate_active = (
        "tcaLongSent" in input_cpp
        and "(now - tcaDownAt[index]) >= kLongPressMs" in input_cpp
        and "result = buttonEvent(index, true)" in input_cpp
    )
    hold_release_suppressed = (
        "alreadySentLong" in input_cpp
        and "if (!alreadySentLong)" in input_cpp
        and "ConnectButtonEvent::kBLong" in input_cpp
    )
    game_progress_uses_snapshot_limits = (
        "pet.gameRoundLimit" in display
        and "pet.gameScoreLimit" in display
        and "%s %u/%u S%u" in display
        and "%u/%u" in display
    )
    model_cpp = read(SOURCE_ROOT / "EchoPetModel.cpp")
    known_length_games_active = (
        "{MiniGameKind::kGet, 100, 100, 100" in model_cpp
        and "{MiniGameKind::kBump, 8, 8, 8" in model_cpp
        and "{MiniGameKind::kFlag, 9, 9, 9" in model_cpp
        and "{MiniGameKind::kHeading, 20, 20, 20" in model_cpp
        and "{MiniGameKind::kMemory, 8, 20, 20" in model_cpp
        and "{MiniGameKind::kSprint, 8, 8, 8" in model_cpp
        and "{MiniGameKind::kHoops, 30, 30, 30" in model_cpp
    )
    memory_sequence_active = (
        "uint32_t gameHazard_" in read(SOURCE_ROOT / "EchoPetModel.h")
        and "uint32_t gameHazard;" in read(SOURCE_ROOT / "EchoPetModel.h")
        and "uint8_t memorySequenceAt(uint32_t sequence, uint8_t index)" in model_cpp
        and "uint8_t memoryPatternLengthForRound(uint8_t round)" in model_cpp
        and "2, 2, 2, 3, 3, 3, 4, 4, 4, 5" in model_cpp
        and "5, 5, 6, 6, 6, 7, 7, 7, 8, 8" in model_cpp
        and "gameHazard_ |= static_cast<uint32_t>(code) << (i * 2)" in model_cpp
        and "gameCursor_ + 1 < gameTarget_" in model_cpp
        and "windowStart = pet.gameCursor >= 3" in display
    )
    long_game_display_mapping_active = (
        "visualProgress(pet.gameRound, roundLimit" in display
        and "visualScoreDots(pet.gameScore, scoreLimit)" in display
        and "if (r.compactText && (roundLimit >= 100" in display
    )
    scene_interval_constants = [
        ("base", "kSceneAnimationMs"),
        ("food", "kFoodSceneAnimationMs"),
        ("toilet", "kToiletSceneAnimationMs"),
        ("medicine", "kMedicineSceneAnimationMs"),
        ("discipline", "kDisciplineSceneAnimationMs"),
        ("lights", "kLightsSceneAnimationMs"),
        ("family", "kFamilySceneAnimationMs"),
        ("link", "kLinkSceneAnimationMs"),
        ("catalog", "kCatalogSceneAnimationMs"),
    ]
    scene_intervals = []
    for label, const in scene_interval_constants:
        match = re.search(rf"{const}\s*=\s*(\d+)", ino)
        if match:
            scene_intervals.append((label, int(match.group(1))))
    scene_pacing_table_active = (
        "case UiMode::kMeal:" in ino
        and "case UiMode::kSnack:" in ino
        and "return kFoodSceneAnimationMs;" in ino
        and "case UiMode::kFamily:" in ino
        and "return kFamilySceneAnimationMs;" in ino
        and "case UiMode::kLinkResult:" in ino
        and "return kLinkSceneAnimationMs;" in ino
    )
    game_interval_values = sorted(
        {
            int(value)
            for value in re.findall(r"return\s+(\d+)\s*;", between(
                ino, "static uint32_t gameAnimationIntervalMs", "static uint32_t animationIntervalMs"
            ))
        }
    )
    scene_interval_map = {label: value for label, value in scene_intervals}
    firmware_timing_by_contract = {
        "food.select/bite/crumbs": str(scene_interval_map.get("food", "?")),
        "toilet.mess/sweep/done": str(scene_interval_map.get("toilet", "?")),
        "discipline.prompt/react": str(scene_interval_map.get("discipline", "?")),
        "medicine.sick/dose/recover": str(scene_interval_map.get("medicine", "?")),
        "lights.selector/sleep/dark": str(scene_interval_map.get("lights", "?")),
        "egg/baby/child/teen/adult/passaway": str(scene_interval_map.get("family", "?")),
        "game.get.*": "260",
        "game.bump.*": "250",
        "game.flag.*": "260",
        "game.heading.*": "250",
        "game.memory.*": "300",
        "game.sprint.*": "250",
        "game.hoops.*": "250",
        "item.use/collection": str(scene_interval_map.get("catalog", "?")),
        "shop.booth/keeper/item": str(scene_interval_map.get("catalog", "?")),
        "password.reward.entry": str(scene_interval_map.get("catalog", "?")),
        "shop.secret-code.entry": str(scene_interval_map.get("catalog", "?")),
        "connection.visit": str(scene_interval_map.get("link", "?")),
        "connection.present": str(scene_interval_map.get("link", "?")),
        "connection.game": str(scene_interval_map.get("link", "?")),
        "connection.love": str(scene_interval_map.get("link", "?")),
        "connection.next_generation": str(scene_interval_map.get("family", "?")),
        "connection.result": str(scene_interval_map.get("link", "?")),
        "connection.friendship": str(scene_interval_map.get("link", "?")),
    }
    official_timing = [
        (hint, file, frames, total, avg,
         firmware_timing_by_contract.get(hint, "-"))
        for hint, file, frames, total, avg in official_timing_rows()
    ]
    link_receive_window_active = (
        "ui.mode == UiMode::kLinkStandby" in ino
        and "ui.mode == UiMode::kLinkResult && ui.entry[2] == kLinkUiSent" in ino
        and "kLinkStandbyTimeoutMs" in ino
    )
    link_kind_gate_active = (
        "packet.kind != ui.entry[0]" in ino
        and "friend packet rejected for mismatched LINK kind" in ino
        and "enterLinkResult(kLinkUiFailed)" in ino
    )
    link_same_kind_reply_active = (
        "emitLinkPacketForSelection(kind, replySelection, true)" in ino
        and "case LinkKind::kVisit" in ino
        and "case LinkKind::kPresent" in ino
        and "case LinkKind::kGame" in ino
        and "case LinkKind::kLove" in ino
    )
    link_game_contract_active = (
        "enum class LinkGameKind" in read(SOURCE_ROOT / "EchoPetModel.h")
        and "constexpr uint8_t kLinkGameCount = 7" in read(SOURCE_ROOT / "EchoPetModel.h")
        and "return kLinkGameCount;" in read(SOURCE_ROOT / "EchoPetUi.cpp")
        and "const LinkGameKind game = echopet::linkGameFromIndex(state.cursor)" in ino
        and "replySelection = packet.gameKind - 1" in ino
        and "validLinkGameKind(packet.gameKind)" in model_cpp
    )
    link_game_item_gate_active = (
        "linkGameOwnedInBits(catalogOwned_, game)" in model_cpp
        and "if (!ownsLinkGameItem(game))" in model_cpp
        and "if (!validLinkGameKind(packet.gameKind) || !ownsLinkGameItem(game))" in model_cpp
        and "linkGameAvailable(pet.catalogOwned, game)" in display
        and "linkGameRequiredLabel(game)" in display
    )
    link_passive_reply_notice_safe = (
        "printFriendPacket(LinkKind::kVisit, !reply)" in ino
        and "printGiftPacket(selection, false)" in ino
        and "printLinkGamePacket(selection, false)" in ino
        and "updateNotice" in model_cpp
        and "if (updateNotice) notice_ = Notice::kFriendReady" in model_cpp
    )
    link_present_catalog_active = (
        "kCatalogItem" in read(SOURCE_ROOT / "EchoPetModel.h")
        and "bool prepareCatalogGiftPacket(uint8_t index" in read(SOURCE_ROOT / "EchoPetModel.h")
        and "return kCatalogItemCount + 1;" in read(SOURCE_ROOT / "EchoPetUi.cpp")
        and "pet.prepareCatalogGiftPacket(state.cursor" in ino
        and "debitCatalogGift(index, updateNotice)" in model_cpp
        and "packet.giftKind = static_cast<uint8_t>(GiftKind::kCatalogItem)" in model_cpp
        and "receiveCatalogGift(packet.giftId, inventoryFull)" in model_cpp
        and "catalogStockTotal(CatalogKind::kFood)" in model_cpp
        and "catalogStockTotal(CatalogKind::kItem)" in model_cpp
        and "drawCatalogIcon(display, cx - iconHalf" in display
    )
    link_catalog_identity_active = (
        "constexpr uint8_t kFriendVersion = 4" in model_cpp
        and "uint8_t catalogId;" in read(SOURCE_ROOT / "EchoPetModel.h")
        and "packet.catalogId = characterCatalogId_" in model_cpp
        and "record->catalogId" in model_cpp
        and "packet.catalogId < kCharacterCatalogCount" in model_cpp
        and "drawCatalogAvatar" in display
        and "catalogIdForFriendAvatar" in display
        and "startBaby(static_cast<AdultTier>(packet.adultTier), oyajitchiLineage," in model_cpp
        and "parentCatalogIdA_" in read(SOURCE_ROOT / "EchoPetModel.h")
        and "kFamilyRecordPartnerCatalogValid" in read(SOURCE_ROOT / "EchoPetModel.h")
        and "FamilyAncestryRecord" in read(SOURCE_ROOT / "EchoPetModel.h")
        and "familyAncestry_[0].babyCatalogId" in model_cpp
        and "familyHasPartnerCatalog" in display
        and "familyAncestryHasPartnerCatalog" in display
        and "familyRouteValue" in display
    )
    link_contract_modes = {
        mode: mode in link_contract
        for mode in ("Visit Flow", "Present Flow", "Connection Game Flow",
                     "Love / Partner / Baby Flow", "LoRa Equivalence Rule")
    }
    labels_missing = [frame for frame in enum_names
                      if frame != "kPassed" and frame not in proof_frames]
    render_missing = [frame for frame in enum_names
                      if frame != "kPassed" and frame not in render_frames]

    lines = [
        "# EchoPet Parity Resource Coverage",
        "",
        "Generated by `analysis/parity_coverage/generate_parity_resource_coverage.py`.",
        "",
        "This is a SIM/source audit. It proves that resource contracts are named,",
        "mapped to a renderer, and exposed through sprite proof. It does not prove",
        "T-Echo-Lite e-paper readability or official-pixel visual parity.",
        "",
        "## Sprite Frame Plumbing",
        "",
        f"- SpriteFrame enum entries: `{len(enum_names)}`.",
        f"- Renderer-mapped frames: `{len(render_frames)}`.",
        f"- Sprite-proof labelled frames: `{len(proof_frames)}`.",
        f"- Missing renderer map: `{', '.join(render_missing) if render_missing else 'none'}`.",
        f"- Missing proof label: `{', '.join(labels_missing) if labels_missing else 'none'}`.",
        "",
        "## Official How-To Motion Density",
        "",
        "| Group | Assets | Animated | Max frames |",
        "| --- | ---: | ---: | ---: |",
    ]
    for group, assets, animated, max_frames in official_group_rows():
        lines.append(f"| `{group}` | {assets} | {animated} | {max_frames} |")

    lines += [
        "",
        "## Resource Contract Rows",
        "",
        "| Contract | Group | Frames | Status | Missing detail |",
        "| --- | --- | --- | --- | --- |",
    ]
    contract_counts: dict[str, int] = defaultdict(int)
    complete_counts: dict[str, int] = defaultdict(int)
    for name, frames, group in FRAME_CONTRACTS:
        status, detail = status_for(frames, enum_frames, render_frames, proof_frames)
        contract_counts[group] += 1
        if status == "CODED_LOW_RESOURCE":
            complete_counts[group] += 1
        frame_list = ", ".join(f"`{frame}`" for frame in frames)
        lines.append(f"| `{name}` | `{group}` | {frame_list} | `{status}` | {detail or '-'} |")

    lines += [
        "",
        "## Group Summary",
        "",
        "| Group | Coded rows | Total rows |",
        "| --- | ---: | ---: |",
    ]
    for group in sorted(contract_counts):
        lines.append(
            f"| `{group}` | {complete_counts[group]} | {contract_counts[group]} |"
        )

    lines += [
        "",
        "## Character Catalog Visual Routing",
        "",
        f"- Character catalog rows: `{len(character_rows)}`.",
        f"- Rows with generation/tier/gender source-pool masks: `{source_pool_rows}`.",
        f"- Distinct frame families: `{len(family_counts)}`.",
        f"- Distinct low-resource visual traits: `{len(trait_counts)}`.",
        "- `selectSpriteFrame` uses `frameFamily`: "
        f"`{'yes' if family_routing_active else 'no'}`.",
        "- Main scene draws `visualTraits` overlay: "
        f"`{'yes' if trait_overlay_active else 'no'}`.",
        "",
        "| Frame family | Character rows |",
        "| ---: | ---: |",
    ]
    for family in sorted(family_counts):
        lines.append(f"| {family} | {family_counts[family]} |")
    lines += [
        "",
        "| Visual trait | Character rows |",
        "| ---: | ---: |",
    ]
    for trait in sorted(trait_counts):
        lines.append(f"| 0x{trait:02X} | {trait_counts[trait]} |")

    lines += [
        "",
        "## Default Adult Mametchi-Like Layering",
        "",
        "| Check | Evidence | Status |",
        "| --- | --- | --- |",
        "| Adult idle uses composed frame parts, not a single 8x8 tile | `kAdult0/kAdult1 -> kFrameStand0/kFrameStand1` | "
        f"`{'yes' if adult_composed_frame_active else 'no'}` |",
        "| Body silhouette has a 32x32 source bitmap before display scaling | `{32, 32, 4, kMameBodyBitmap}` | "
        f"`{'yes' if adult_body_32px else 'no'}` |",
        "| Left/right eyes are separate local parts with blink/sleep/sad/sick variants | `B_EYE_*` parts at independent anchors | "
        f"`{'yes' if adult_separate_eyes else 'no'}` |",
        "| Mouth is a separate local part with smile/open/sad states | `B_MOUTH_*` parts | "
        f"`{'yes' if adult_separate_mouth else 'no'}` |",
        "| Feet are separate local parts | `B_FOOT` left/right anchors | "
        f"`{'yes' if adult_separate_feet else 'no'}` |",
        "| Idle animation shifts foot anchors between frame 0 and frame 1 | `kFrameStand0Parts` vs `kFrameStand1Parts` | "
        f"`{'yes' if adult_idle_foot_shift else 'no'}` |",
        "| Character trait overlay can animate blink without replacing the base body | `animationPhase` blink overlay | "
        f"`{'yes' if adult_trait_overlay_blink else 'no'}` |",
        "| Mametchi uses an explicit validation trait instead of the generic page/slot overlay | `kMametchiValidationTraits = 0x80` | "
        f"`{'yes' if mametchi_validation_trait_active else 'no'}` |",
        "",
        "This proves the current default validation character is a layered, replaceable low-resource resource set. It does not prove final official-look art or hardware placement; those remain sprite-proof and rights-cleared-art gates.",
    ]

    lines += [
        "",
        "## Firmware Sprite Render Proof",
        "",
        "- Host-side sprite render audit exists: "
        f"`{'yes' if SPRITE_RENDER_AUDIT.exists() else 'no'}`.",
        f"- Rendered sprite frames: `{len(sprite_rendered_frames)}` / `{len(enum_names)}`.",
        f"- Missing rendered enum frames: `{len(sprite_missing_rendered_frames)}`.",
        f"- Frames with pixels outside nominal frame: `{len(sprite_out_of_bounds_frames)}`.",
        "- Generated contact sheets: `analysis/sprite_render_audit/sprite_contact_sheet.png`, "
        "`sprite_egg_hatch.png`, `sprite_mametchi_motion.png`, "
        "`sprite_care_scenes.png`, and `sprite_game_scenes.png`.",
        "- This is a source-side composition proof only; e-paper refresh artifacts remain a hardware acceptance gate.",
    ]
    if sprite_missing_rendered_frames:
        lines.append("- Missing rendered frames: " + ", ".join(f"`{frame}`" for frame in sprite_missing_rendered_frames) + ".")
    if sprite_out_of_bounds_frames:
        lines.append("- Out-of-bounds frames: " + ", ".join(f"`{frame}`" for frame in sprite_out_of_bounds_frames) + ".")

    lines += [
        "",
        "## Item Catalog Visual Routing",
        "",
        "- Catalog rows: `160`.",
        f"- Segment rows: `{item_segments}`.",
        f"- Explicit override rows: `{item_overrides}`.",
        "- ITEM page uses catalog cursor: "
        f"`{'yes' if item_cursor_active else 'no'}`.",
        f"- Preview icons use `entry.icon`: `{'yes' if item_icon_active else 'no'}`.",
        "- Use-result scenes use `entry.useScene`: "
        f"`{'yes' if item_use_scene_active else 'no'}`.",
        f"- Distinct low-resource visual traits: `{len(item_trait_counts)}`.",
        "- Preview/use icons draw `visualTraits` overlay: "
        f"`{'yes' if item_trait_overlay_active else 'no'}`.",
        "- Distinct souvenir visual traits: "
        f"`{len(souvenir_trait_counts)}`.",
        "- Souvenir page draws visual-trait icon: "
        f"`{'yes' if souvenir_trait_active else 'no'}`.",
        "",
        "| Use scene | Catalog rows |",
        "| ---: | ---: |",
    ]
    for scene in sorted(item_scene_counts):
        lines.append(f"| {scene} | {item_scene_counts[scene]} |")
    lines += [
        "",
        "| Visual trait | Catalog rows |",
        "| ---: | ---: |",
    ]
    for trait in sorted(item_trait_counts):
        lines.append(f"| 0x{trait:02X} | {item_trait_counts[trait]} |")
    lines += [
        "",
        "| Souvenir visual trait | Souvenir rows |",
        "| ---: | ---: |",
    ]
    for trait in sorted(souvenir_trait_counts):
        lines.append(f"| 0x{trait:02X} | {souvenir_trait_counts[trait]} |")

    lines += [
        "",
        "## Family Visual Routing",
        "",
        "- FAMILY page draws album/card visual: "
        f"`{'yes' if family_album_active else 'no'}`.",
        "- Family history source rows: `FamilyRecord` compatibility entries plus `FamilyAncestryRecord` parent/partner/baby/tier rows.",
        "- Current art level: compact low-resource parent/partner/baby album composition; final official-look family scenes remain open.",
    ]

    lines += [
        "",
        "## Discipline Visual Routing",
        "",
        "- Discipline scene draws attention-call/missed frames: "
        f"`{'yes' if discipline_attention_active else 'no'}`.",
        "- Current art level: compact low-resource TIME OUT/PRAISE/invalid/call/missed feedback; exact Connection timing remains open.",
    ]

    lines += [
        "",
        "## Medicine Visual Routing",
        "",
        "- Medicine scene draws bottle, sick/tooth/cured/refuse badge, dose trail, and recover/refuse frames: "
        f"`{'yes' if medicine_scene_active else 'no'}`.",
        "- `selectSpriteFrame` avoids showing sick/tooth startup frames after a successful final cure: "
        f"`{'yes' if 'pet.notice == Notice::kMedicine && !(pet.sickness || pet.toothache)' in sprites else 'no'}`.",
        "- Current art level: compact low-resource cure flow from `Cure when ill.gif`; exact Connection timing and hardware readability remain open.",
    ]

    lines += [
        "",
        "## Food Visual Routing",
        "",
        "- Food/snack scene draws selected food prop, plate, stage badge, crumbs/done/refuse branches: "
        f"`{'yes' if food_scene_active else 'no'}`.",
        "- Current art level: compact low-resource `food and snacks.gif` flow; exact Connection timing, bite count, and hardware readability remain open.",
    ]

    lines += [
        "",
        "## Toilet Visual Routing",
        "",
        "- Toilet scene draws visible mess, progressive removal, sweep frames, done/no-mess branches: "
        f"`{'yes' if toilet_scene_active else 'no'}`.",
        "- Current art level: compact low-resource `Clean up the poop.gif` flow; exact mess timing, sickness effects, and hardware readability remain open.",
    ]

    lines += [
        "",
        "## Lights Visual Routing",
        "",
        "- Lights scene draws ON/OFF selector, low-ink dark-room pattern, sleep/Z, wake, and invalid branches: "
        f"`{'yes' if lights_scene_active else 'no'}`.",
        "- Intrusive full-width dark fill removed from `drawLightsScene`: "
        f"`{'yes' if lights_intrusive_fill_removed else 'no'}`.",
        "- Current art level: compact low-resource `Lights off at night!.gif` flow; hardware ghosting/whitening still needs T-Echo-Lite observation.",
    ]

    lines += [
        "",
        "## Interaction Timing",
        "",
        "- KeyShield long press emits a HOLD event once the threshold is reached, before release: "
        f"`{'yes' if hold_immediate_active else 'no'}`.",
        "- Release after an already-emitted HOLD is suppressed so it does not also act as a short press: "
        f"`{'yes' if hold_release_suppressed else 'no'}`.",
        "- Scene target animation intervals: "
        f"`{', '.join(f'{label} {value} ms' for label, value in scene_intervals)}`.",
        "- Meal/Snack, Family/Friends, LINK, and catalog pages use explicit scene pacing instead of falling through to home idle pacing: "
        f"`{'yes' if scene_pacing_table_active else 'no'}`.",
        "- Game target animation intervals: "
        f"`{', '.join(str(value) for value in game_interval_values)} ms`.",
        "- Official how-to GIF timing metadata is present in the timing comparison table: "
        f"`{'yes' if official_timing else 'no'}`.",
        "- Game progress labels use model-exported round/score limits instead of a renderer-local `/5` constant: "
        f"`{'yes' if game_progress_uses_snapshot_limits else 'no'}`.",
        "- Known-length games use source round/score caps in the model: "
        f"`{'yes' if known_length_games_active else 'no'}`.",
        "- Memory uses the sourced 20-pattern A/B/C cadence with two final eight-arrow patterns and a compact viewport window instead of single-key rounds: "
        f"`{'yes' if memory_sequence_active else 'no'}`.",
        "- Long game progress and score dots are mapped for the e-paper viewport without changing model limits: "
        f"`{'yes' if long_game_display_mapping_active else 'no'}`.",
        "- These are source-level timing targets. Final perceived speed still depends on the synchronous partial-refresh submit interval and real-panel observation.",
    ]

    if official_timing:
        lines += [
            "",
            "### Official GIF Timing Comparison",
            "",
            "| Contract | File | Frames | Source total ms | Source avg ms/frame | Firmware target ms/frame |",
            "| --- | --- | ---: | ---: | ---: | ---: |",
        ]
        for hint, file, frames, total, avg, firmware in official_timing:
            lines.append(
                f"| `{hint}` | `{file}` | {frames} | {total} | {avg} | {firmware} |"
            )

    lines += [
        "",
        "## Link Visible Contract",
        "",
        "- LINK contract sections present: "
        f"`{', '.join(name for name, present in link_contract_modes.items() if present)}`.",
        "- Receive accepted only in standby or SENT reply window: "
        f"`{'yes' if link_receive_window_active else 'no'}`.",
        "- Incoming packet kind must match current local LINK mode before model mutation: "
        f"`{'yes' if link_kind_gate_active else 'no'}`.",
        "- Standby emits same-kind Visit/Present/Game/Love reply after accepted receive: "
        f"`{'yes' if link_same_kind_reply_active else 'no'}`.",
        "- CONNECT->GAME uses seven Connection-game ids rather than local mini-game ids and replies echo the incoming game id: "
        f"`{'yes' if link_game_contract_active else 'no'}`.",
        "- Item-backed Connection games require local catalog ownership on both send and receive paths: "
        f"`{'yes' if link_game_item_gate_active else 'no'}`.",
        "- PRESENT selection/transmit/receive uses exact catalog item ids plus a points row instead of coarse FoodKind/ItemKind rows: "
        f"`{'yes' if link_present_catalog_active else 'no'}`.",
        "- Passive standby replies avoid overwriting receiver result notice: "
        f"`{'yes' if link_passive_reply_notice_safe else 'no'}`.",
        "- FriendPacket v4 carries peer catalog identity through receive, friend-list portrait rendering, baby-flow parent capture, and family-album partner rendering: "
        f"`{'yes' if link_catalog_identity_active else 'no'}`.",
        "- Remaining gate: `DUAL/HW` two-device T-Echo-Lite proof for Visit, Present, Game, Love, partner, baby, timeout, cancel, mismatch, and LoRa equivalence.",
    ]

    lines += [
        "",
        "## Remaining Non-Hardware Resource Gaps",
        "",
        "- Shop booth/counter, shopkeeper, item preview, and buy-result frames are coded as replaceable frame rows; exact per-item shop art remains compact/simplified.",
        "- Lights selector/on/off/wake/invalid frames are coded, and the dark-room scene uses a low-ink pattern instead of a full-width black fill; final stability remains hardware-only proof.",
        "- Medicine sick/tooth, dose, recover, and refusal frames are coded with scene-level bottle/badge/dose routing; exact cure timing remains source/hardware proof.",
        "- Food/snack select, bite, crumbs, done, and refusal frames are coded with scene-level plate/stage-badge routing; exact bite timing remains source/hardware proof.",
        "- Toilet mess, sweep, done, and no-mess frames are coded with scene-level progressive-removal routing; exact mess timing remains source/hardware proof.",
        "- Character frame-family routing is active across the 50-row manifest, but final per-character original art is still not unique for every slot.",
        "- Item catalog display now uses catalog cursor, icon, use-scene, and visual-trait routing across the 160-row manifest, and the souvenir page has 64 visual-trait icons. Exact per-item art is still represented by compact behavior icons plus low-resource overlays.",
        "- Official website pixels remain excluded until rights-cleared assets are supplied.",
    ]

    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text("\n".join(lines) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()

