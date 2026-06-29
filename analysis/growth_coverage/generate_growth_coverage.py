from __future__ import annotations

import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
SOURCE_ROOT = ROOT / "src" if (ROOT / "src").exists() else ROOT
MODEL_H = SOURCE_ROOT / "EchoPetModel.h"
MODEL_CPP = SOURCE_ROOT / "EchoPetModel.cpp"
DISPLAY_CPP = SOURCE_ROOT / "EchoPetDisplay.cpp"
INO = SOURCE_ROOT / "EchoPet.ino"
CATALOG_CPP = SOURCE_ROOT / "EchoPetCharacterCatalog.cpp"
CATALOG_H = SOURCE_ROOT / "EchoPetCharacterCatalog.h"
ITEM_CATALOG_CPP = SOURCE_ROOT / "EchoPetCatalog.cpp"
ITEM_CATALOG_H = SOURCE_ROOT / "EchoPetCatalog.h"
OUT = ROOT / "analysis" / "growth_coverage" / "growth_coverage.md"


def read(path: Path) -> str:
    return path.read_text(encoding="utf-8")


def first(pattern: str, text: str, flags: int = 0) -> str:
    match = re.search(pattern, text, flags)
    return match.group(1) if match else ""


def yes_no(value: bool) -> str:
    return "yes" if value else "no"


def one_line(value: str) -> str:
    return re.sub(r"\s+", " ", value).strip()


def clean_cpp_flags(value: str) -> str:
    cleaned = one_line(value)
    prefix = "static_cast<uint8_t>("
    if cleaned.startswith(prefix) and cleaned.endswith(")"):
        cleaned = cleaned[len(prefix):-1].strip()
    return cleaned


def between(text: str, start: str, end: str) -> str:
    start_index = text.find(start)
    if start_index < 0:
        return ""
    end_index = text.find(end, start_index + len(start))
    if end_index < 0:
        return text[start_index:]
    return text[start_index:end_index]


def catalog_constants(header: str) -> dict[str, str]:
    return {
        name: value.strip()
        for name, value in re.findall(
            r"constexpr\s+uint8_t\s+(kCatalog\w+)\s*=\s*([^;]+);",
            header,
        )
    }


def resolve_catalog_token(token: str, constants: dict[str, str]) -> str:
    token = token.strip()
    if token in constants:
        return constants[token]
    return token


def split_catalog_initializer(raw: str) -> list[str]:
    values: list[str] = []
    current: list[str] = []
    depth = 0
    for char in raw:
        if char == "<":
            depth += 1
        elif char == ">" and depth:
            depth -= 1
        if char == "," and depth == 0:
            values.append("".join(current).strip())
            current = []
        else:
            current.append(char)
    if current:
        values.append("".join(current).strip())
    return values


def parse_catalog_segment_rows(item_catalog: str,
                               constants: dict[str, str]) -> list[list[str]]:
    block = between(item_catalog, "const CatalogSegment kCatalogSegments[] = {", "};")
    rows: list[list[str]] = []
    entries: list[str] = []
    current: list[str] = []
    collecting = False
    for line in block.splitlines():
        stripped = line.strip()
        if stripped.startswith("{"):
            collecting = True
            current = [stripped]
        elif collecting:
            current.append(stripped)
        if collecting and stripped.endswith("},"):
            entry = " ".join(current).strip()
            while entry.startswith("{"):
                entry = entry[1:].strip()
            if entry.endswith("},"):
                entry = entry[:-2].strip()
            entries.append(entry)
            collecting = False
            current = []
    for raw in entries:
        values = split_catalog_initializer(re.sub(r"\s+", " ", raw))
        if len(values) < 13:
            continue
        rows.append([
            values[0],
            values[1],
            values[2].strip('"'),
            values[3].replace("CatalogKind::k", ""),
            values[4],
            values[5],
            values[6],
            values[7],
            values[8],
            values[9],
            values[10],
            values[11],
            resolve_catalog_token(values[12], constants),
        ])
    return rows


def main() -> None:
    header = read(MODEL_H)
    model = read(MODEL_CPP)
    display = read(DISPLAY_CPP)
    ino = read(INO)
    catalog = read(CATALOG_CPP)
    catalog_header = read(CATALOG_H)
    item_catalog = read(ITEM_CATALOG_CPP)
    item_catalog_header = read(ITEM_CATALOG_H)
    model_flat = re.sub(r"\s+", " ", model)
    item_catalog_flat = re.sub(r"\s+", " ", item_catalog)
    catalog_const = catalog_constants(item_catalog_header)

    constants = [
        "kEggHatchAgeMinutes",
        "kBabyToChildAgeMinutes",
        "kChildToTeenAgeMinutes",
        "kTeenToAdultAgeMinutes",
        "kAdultToElderAgeMinutes",
        "kMatchmakerFirstAgeMinutes",
        "kParentLeaveMinutes",
        "kManualMatchmakerAgeMinutes",
    ]
    constant_rows = []
    for name in constants:
        value = first(rf"constexpr\s+(?:uint32_t|uint16_t)\s+{name}\s*=\s*([^;]+);", model)
        constant_rows.append((name, value or "missing"))

    required_header_tokens = [
        "enum class CareMistakeClass",
        "enum class GrowthTier",
        "enum class AdultTier",
        "physicalCareMistakes_",
        "mentalCareMistakes_",
        "growthTier_",
        "adultTier_",
        "parentAdultTierA_",
        "parentAdultTierB_",
        "bornFromUnhealthyParents_",
    ]
    header_rows = [(token, yes_no(token in header)) for token in required_header_tokens]

    direct_increment_count = model.count("careMistakes_++")
    direct_increment_bypass = (
        direct_increment_count > 1 or
        "void EchoPetModel::recordCareMistake" not in model
    )
    bump_input_block = between(
        model,
        "case MiniGameKind::kBump:",
        "case MiniGameKind::kFlag:",
    )
    sprint_input_block = between(
        model,
        "case MiniGameKind::kSprint:",
        "case MiniGameKind::kHoops:",
    )
    sickness_block = first(
        r"(bool actionBlockedBySickness\(Action action\).*?\})", model, re.S)
    secret_reward_rows = re.findall(
        r"\{\{[0-2,\s]+\},\s*(?:14[4-9]|150)\}",
        model,
    )
    password_reward_rows = re.findall(
        r"\{\{[0-9,\s]+\},\s*PasswordRewardKind::k",
        model,
    )
    catalog_override_rows = re.findall(
        r'\{(?:14[4-9]|150),\s*"[^"]+"',
        item_catalog,
    )
    all_catalog_override_indexes = {
        int(index)
        for index in re.findall(r'\{\s*(\d+),\s*"[^"]+"', item_catalog)
    }
    missing_named_catalog_rows = [
        index for index in range(152) if index not in all_catalog_override_indexes
    ]
    password_catalog_override_rows = re.findall(
        r'\{(?:2[4-9]|3[0-9]|4[0-1]|4[8-9]|5[0-9]|6[0-1]),\s*"[^"]+"',
        item_catalog,
    )
    model_checks = [
        ("direct careMistakes_++ bypass found", yes_no(direct_increment_bypass)),
        ("careMistakes_++ confined to recordCareMistake", yes_no(not direct_increment_bypass)),
        ("recordCareMistake entry point", yes_no("void EchoPetModel::recordCareMistake" in model)),
        ("reverseCareMistake entry point", yes_no("bool EchoPetModel::reverseCareMistake" in model)),
        ("stage-local reset entry point", yes_no("void EchoPetModel::resetStageCareMistakes" in model)),
        ("chooseRoute delegates to routeForGrowthState", yes_no("return routeForGrowthState();" in model)),
        ("matchmaker three-clock helper", yes_no(
            "const MatchmakerClockRule kMatchmakerClockRules" in model
            and "matchmakerNoticeReady" in model
            and "10UL * 60UL + 30UL" in model
            and "15UL * 60UL" in model
            and "19UL * 60UL" in model
        )),
        ("Get/Flag mental reversal", yes_no("MiniGameKind::kFlag" in model and "CareMistakeClass::kMental" in model)),
        ("Bump/Heading physical reversal", yes_no("MiniGameKind::kHeading" in model and "CareMistakeClass::kPhysical" in model)),
        ("stat decay table present", yes_no("const StatDecayRule kStatDecayRules" in model)),
        ("pass-away rule table present", yes_no("const PassAwayRule kPassAwayRules" in model)),
        ("adult-to-senior uses FAQ-aligned four-adult-day duration", yes_no("kAdultToElderAgeMinutes = kTeenToAdultAgeMinutes + 4UL * 1440UL" in model_flat and "{Stage::kElder, kAdultToElderAgeMinutes}" in model)),
        ("matchmaker starts four adult days after adult transition", yes_no("kMatchmakerFirstAgeMinutes = kTeenToAdultAgeMinutes + 4UL * 1440UL" in model_flat and "kMatchmakerClockRules" in model and "matchmakerNoticeReady" in model)),
        ("growth schedule rule table present", yes_no(
            "const GrowthScheduleRule kGrowthScheduleRules" in model
            and "{Stage::kElder, kAdultToElderAgeMinutes}" in model
            and "{Stage::kAdult, kTeenToAdultAgeMinutes}" in model
            and "{Stage::kTeen, kChildToTeenAgeMinutes}" in model
            and "{Stage::kChild, kBabyToChildAgeMinutes}" in model
            and "{Stage::kBaby, kEggHatchAgeMinutes}" in model
            and "scheduledStageForAge(stage_, growthFlags_, ageMinutes_)" in model
        )),
        ("family timing rule present", yes_no(
            "const FamilyTimingRule kFamilyTimingRule" in model
            and "kManualMatchmakerAgeMinutes" in model
            and "kFamilyTimingRule.parentLeaveMinutes" in model
            and "kFamilyTimingRule.manualMatchmakerAgeMinutes" in model
        )),
        ("matchmaker clock rule table present", yes_no(
            "const MatchmakerClockRule kMatchmakerClockRules" in model
            and "10UL * 60UL + 30UL" in model
            and "15UL * 60UL" in model
            and "19UL * 60UL" in model
            and "matchmakerNoticeReady(ageMinutes_, clockMinutes_)" in model
        )),
        ("stage base weight rule table present", yes_no(
            "const StageBaseWeightRule kStageBaseWeightRules" in model
            and "{Stage::kBaby, 5}" in model
            and "{Stage::kChild, 10}" in model
            and "{Stage::kTeen, 20}" in model
            and "{Stage::kAdult, 30}" in model
            and "rule.baseWeight" in model
        )),
        ("poop/event constants present", yes_no("messPeriodForStage" in model and "SicknessTriggerKind::kMess" in model)),
        ("source calendar event rule table present", yes_no(
            "const CalendarEventRule kCalendarEventRules" in model
            and "{10, 31, 10, 31, kCalendarStandardSlots" in model
            and "{12, 24, 12, 24, kCalendarSlotLate" in model
            and "calendarSlotMaskForMinute" in model
            and "calendarEventKey(month_, day_, slot)" in model
            and "dateInRange(month_, day_, rule.startMonth" in model
            and "Notice::kBirthday" in model
            and "setupDaysInMonth" in ino
            and "day_ > daysInMonth(month_)" in model
            and "awardSouvenir(ageDay" not in model
            and "awardCatalogItem(static_cast<uint8_t>((ageDay" not in model
        )),
        ("stage care timing table present", yes_no(
            "const StageCareTimingRule kStageCareTimingRules" in model
            and "{Stage::kBaby, 3, 5, 6}" in model
            and "{Stage::kChild, 40, 45, 80}" in model
            and "{Stage::kTeen, 50, 55, 100}" in model
            and "{Stage::kAdult, 70, 75, 140}" in model
            and "{Stage::kElder, 80, 110, 160}" in model
            and "statDecayPeriodForStage(rule.meter, stage_, rule.periodMinutes)" in model
        )),
        ("hidden hungry/happy hearts present", yes_no(
            "constexpr uint8_t kVisibleHeartMeterMax = 100" in model
            and "constexpr uint8_t kHiddenHeartMeterMax = 150" in model
            and "uint8_t clampAddHungryHappy" in model
            and "hunger_ >= kHiddenHeartMeterMax" in model
            and "if (value > 100) value = 100" in display
        )),
        ("character food taste table present", yes_no(
            "const CharacterFoodTasteRule kCharacterFoodTasteRules" in model
            and "applyCatalogFoodTaste(index)" in model
            and "kCatalogYogurtIndex = 87" in item_catalog_header
            and "kCatalogSteakIndex = 145" in item_catalog_header
            and "catalogFoodLikedByAll" in model
            and "happiness_ = kHiddenHeartMeterMax" in model
            and "happiness_ = 0" in model
            and "{18, 0, 1}" in model
            and "{29, 145, 6}" in model
        )),
        ("stage sleep window table present", yes_no(
            "const SleepWindowRule kSleepWindowRules" in model
            and "{Stage::kEgg, 23 * 60, 7 * 60}" in model
            and "{Stage::kBaby, 20 * 60, 8 * 60}" in model
            and "{Stage::kAdult, 22 * 60, 7 * 60}" in model
            and "sleepWindowForStage(stage_)" in model
        )),
        ("attention timing rule table present", yes_no(
            "const AttentionTimingRule kAttentionTimingRules" in model
            and "AttentionTimingKind::kOrdinaryMiss, 15, 0" in model
            and "AttentionTimingKind::kCareCall, 29, 0" in model
            and "AttentionTimingKind::kEmptyStatMistake, 30, 0" in model
            and "AttentionTimingKind::kSleepAttention, 30, 0" in model
            and "AttentionTimingKind::kLightsLeftOnMistake, 60, 20" in model
            and "attentionTimingRuleFor" in model
        )),
        ("game reward table present", yes_no("const GameRewardRule kGameRewardRules" in model)),
        ("game pacing contract table present", yes_no("const GamePacingRule kGamePacingRules" in model and "gameRoundLimitFor" in model)),
        ("game limits exported to snapshot", yes_no("uint8_t gameRoundLimit;" in header and "uint8_t gameScoreLimit;" in header and "s.gameRoundLimit" in model and "s.gameScoreLimit" in model)),
        ("game score prize curve present", yes_no("const uint8_t kGameScorePrizePercent" in model and "gamePrizeForScore" in model)),
        ("known source-length game rounds active", yes_no(
            "{MiniGameKind::kGet, 100, 100, 100" in model
            and "{MiniGameKind::kBump, 8, 8, 8" in model
            and "{MiniGameKind::kFlag, 9, 9, 9" in model
            and "{MiniGameKind::kHeading, 20, 20, 20" in model
            and "{MiniGameKind::kMemory, 8, 20, 20" in model
            and "{MiniGameKind::kSprint, 8, 8, 8" in model
            and "{MiniGameKind::kHoops, 30, 30, 30" in model
        )),
        ("Flag A+C and fake-signal wait mechanic present", yes_no(
            "constexpr uint8_t kFlagFakeSignal = 1" in model
            and "kFlagFakeSignalChancePercent" in model
            and "kAC" in header
            and "constexpr uint32_t kFlagChordWindowMs = 140" in ino
            and "handleActiveGameInput" in ino
            and "submitGameButton(GameButton::kAC, nowMs)" in ino
            and "snapshot.gameExpectedYes == static_cast<uint8_t>(GameButton::kAC)" in ino
            and "const uint8_t flagKind = nextRand(3)" in model
            and "buttonValue(GameButton::kAC)" in model
            and "nextRand(2) == 0 ? 0 : 2" in model
            and "gamePrompt_ = fakeSignal ? GamePrompt::kWait : GamePrompt::kNone" in model
            and "completeMiniGameRound(gameHazard_ == kFlagFakeSignal, 1, nowMs)" in model
            and "correct = gameHazard_ == 0 && buttonValue(button) == gameExpectedYes_" in model
            and "drawFlagSignal" in display
            and "const bool fake = pet.gameHazard != 0" in display
        )),
        ("Memory uses sourced 3/4-step 20-pattern sequence cadence", yes_no(
            "uint32_t gameHazard_" in header
            and "uint32_t gameHazard;" in header
            and "uint8_t memorySequenceAt(uint32_t sequence, uint8_t index)" in model
            and "uint8_t memoryPatternLengthForRound(uint8_t round)" in model
            and "2, 2, 2, 3, 3, 3, 4, 4, 4, 5" in model
            and "5, 5, 6, 6, 6, 7, 7, 7, 8, 8" in model
            and "gameHazard_ |= static_cast<uint32_t>(code) << (i * 2)" in model
            and "gameCursor_ + 1 < gameTarget_" in model
        )),
        ("long-game scores map to five result tiers", yes_no(
            "uint8_t gameScoreTier(uint8_t score, uint8_t scoreLimit)" in model
            and "const uint8_t scoreTier = gameScoreTier(score, scoreLimit)" in model
            and "kHappyCurve[scoreTier]" in model
            and "kGameScorePrizePercent[tier]" in model
        )),
        ("Bump weight prize chart present", yes_no("const BumpPrizeRow kBumpPrizeChart" in model and "{8, {2400, 1800, 1200, 600}}" in model and "bumpWeightBand" in model and "sourceRoundsForCompressedScore" in model)),
        ("Bump heavier weight does not directly boost score", yes_no("gain = weight_ >= 60 ? 2 : 1" not in bump_input_block and "gain = 1;" in bump_input_block)),
        ("Gotchi point cap 9999 present", yes_no("constexpr uint16_t kMaxGotchiPoints = 9999" in model and "points_ > kMaxGotchiPoints" in model and "clampAdd16(points_, pointPrize, kMaxGotchiPoints)" in model)),
        ("game unlock table present", yes_no("const GameUnlockRule kGameUnlockRules" in model)),
        ("Get timed catch/dodge window", yes_no("kGetRoundWindowMs" in model and "gameHazard_ ? (gameCursor_ != gameTarget_)" in model)),
        ("Bump any-button push trigger", yes_no("power >= foe" in bump_input_block and "button" not in bump_input_block)),
        ("Sprint repeated-tap target helper", yes_no("sprintTapTargetFor" in model and "gameCursor_ < gameTarget_" in sprint_input_block)),
        ("Sprint lighter-weight chance helper", yes_no("sprintWinChanceFor" in model and "weight_ <= 20 ? 2 : 1" in sprint_input_block)),
        ("Hoops shoot plus timeout miss", yes_no("kHoopsShotWindowMs" in model and "completeMiniGameRound(false, 0, nowMs)" in model)),
        ("secret-code A/B/C entry point", yes_no("bool EchoPetModel::enterSecretCode" in model)),
        ("seven shop secret-code rewards present", yes_no(len(secret_reward_rows) == 7 and "kSecretCodeRewards" in model)),
        ("Hohotchi final-code replacement present", yes_no("kCatalogHohotchiIndex" in model and "ownedSecretCodeCount" in model and "kCatalogFlagReusable" in item_catalog)),
        ("Honey next-link relationship boost present", yes_no("kCatalogHoneyIndex" in read(SOURCE_ROOT / "EchoPetCatalog.h") and "kMemoryFlagHoneyPending" in model and "friendRecord->visits = 24" in model)),
        ("secret reusable/costume item behaviors present", yes_no("kCatalogClockIndex" in read(SOURCE_ROOT / "EchoPetCatalog.h") and "kCatalogRcCar3Index" in read(SOURCE_ROOT / "EchoPetCatalog.h") and "kMemoryFlagNyatchiCostume" in read(SOURCE_ROOT / "EchoPetModel.h") and "kMemoryFlagHohotchiCostume" in read(SOURCE_ROOT / "EchoPetModel.h") and "kMemoryFlagCostumeMask" in model)),
        ("Love baby gated to eligible Love flow", yes_no("Notice::kLoveRejected" in model and "kind == LinkKind::kLove && localAdult && remoteAdult" in model)),
        ("FriendPacket carries growth/adult tier/gender/catalog id", yes_no("uint8_t growthTier;" in header and "uint8_t adultTier;" in header and "uint8_t gender;" in header and "uint8_t catalogId;" in header and "packet.gender = static_cast<uint8_t>(gender_)" in model and "packet.catalogId = characterCatalogId_" in model)),
        ("FriendPacket validates growth/adult tier/gender/catalog id before model effects", yes_no("validGrowthTier(packet.growthTier)" in model and "validAdultTier(packet.adultTier)" in model and "validGender(packet.gender)" in model and "packet.catalogId < kCharacterCatalogCount" in model and "validLinkKind(packet.kind)" in model)),
        ("Friend records preserve peer catalog id", yes_no("uint8_t catalogId;" in header and "record->catalogId" in model and "packet.catalogId < kCharacterCatalogCount" in model)),
        ("Love flow stores peer adult tier", yes_no("startBaby(static_cast<AdultTier>(packet.adultTier), oyajitchiLineage," in model and "void EchoPetModel::startBaby(AdultTier partnerAdultTier" in model)),
        ("Parent-care preserves local parent catalog id", yes_no("parentCatalogIdA_ =" in model and "characterCatalogId_ < kCharacterCatalogCount" in model and "parentCatalogIdA" in header)),
        ("Love flow passes peer catalog id into baby flow", yes_no("startBaby(static_cast<AdultTier>(packet.adultTier), oyajitchiLineage," in model and "packet.catalogId" in model and "parentCatalogIdB_" in header)),
        ("Family record stores known partner catalog behind route sentinel", yes_no("kFamilyRecordPartnerCatalogValid" in header and "family_[0].character = parentCatalogIdB_" in model and "family_[0].route = static_cast<uint8_t>(route_) |" in model)),
        ("Family ancestry record stores parent partner baby and tier rows", yes_no(
            "struct FamilyAncestryRecord" in header
            and "FamilyAncestryRecord familyAncestry[kFamilyHistoryCount]" in header
            and "FamilyAncestryRecord familyAncestry_[kFamilyHistoryCount]" in header
            and "ancestry.parentCatalogIdA = parentCatalog" in model
            and "ancestry.parentCatalogIdB =" in model
            and "ancestry.babyCatalogId =" in model
            and "ancestry.childGrowthTier = static_cast<uint8_t>(childGrowthTier)" in model
        )),
        ("Family page renders sourced ancestry row", yes_no(
            "const FamilyAncestryRecord& ancestry = pet.familyAncestry[i]" in display
            and "drawFamilyAlbumCard(display, r, &f, ancestryPtr, phase)" in display
            and "familyAncestryHasPartnerCatalog(ancestry)" in display
            and "G%u %u+%s>%u" in display
        )),
        ("Serious adult parent gate requires both parents unhealthy", yes_no("const bool bothParentsUnhealthy" in model and "adultTierIsUnhealthy(parentAdultTierA_) &&" in model and "adultTierIsUnhealthy(parentAdultTierB_)" in model)),
        ("parent-pair child tier table present", yes_no("childTierFromParentAdultTiers" in model and "sameAdultPair(a, b, AdultTier::kSerious, AdultTier::kNormal)" in model and "sameAdultPair(a, b, AdultTier::kNormal, AdultTier::kFrail)" in model)),
        ("child-stage transition uses parent-pair table", yes_no("childTierFromParentAdultTiers(parentAdultTierA_" in model and "parentAdultTierB_" in model)),
        ("child-tier teen transition table present", yes_no("teenTierFromChildTier" in model and "physical >= 3 && mental >= 3" in model and "physical >= 4 && mental >= 4" in model)),
        ("adult-tier transition rule table present", yes_no("const AdultGrowthRule kAdultGrowthRules" in model and "adultGrowthRuleMatches" in model and "for (const AdultGrowthRule& rule : kAdultGrowthRules)" in model)),
        ("source adult-tier threshold rows present", yes_no(
            # WIKI-CHAR-2024 adult rows: Tier 1/2 teens use 3+ as the
            # Naughty/Frail/Stubborn threshold; Tier 3/4 teens use 2+.
            "{GrowthTier::kTierOne, 0, 1, 0, 1, kAdultGrowthRequiresHealthyParents" in model
            and "{GrowthTier::kTierOne, 2, 2, 0, 2, 0, AdultTier::kNormal}" in model
            and "{GrowthTier::kTierOne, 0, 2, 3, kAdultGrowthAny, 0" in model
            and "{GrowthTier::kTierOne, 3, kAdultGrowthAny, 0, 2, 0, AdultTier::kFrail}" in model
            and "{GrowthTier::kTierOne, 3, kAdultGrowthAny, 3, kAdultGrowthAny, 0" in model
            and "{GrowthTier::kTierTwo, 0, 0, 0, 1, kAdultGrowthRequiresHealthyParents" in model
            and "{GrowthTier::kTierThree, 0, 0, 0, 0, kAdultGrowthRequiresHealthyParents" in model
            and "{GrowthTier::kTierThree, 0, 1, 2, kAdultGrowthAny, 0" in model
            and "{GrowthTier::kTierThree, 2, kAdultGrowthAny, 0, 1, 0, AdultTier::kFrail}" in model
            and "{GrowthTier::kTierFour, 0, 1, 0, 1, 0, AdultTier::kNormal}" in model
            and "{GrowthTier::kTierFour, 0, kAdultGrowthAny, 0, kAdultGrowthAny, 0" in model
            and "bothParentsUnhealthy" in model
        )),
        ("character catalog source-pool fields present", yes_no("uint8_t generationMask;" in catalog_header and "uint8_t tierMask;" in catalog_header and "uint8_t genderMask;" in catalog_header and "kCharacterGenerationOdd" in catalog_header and "kCharacterTierHealthy" in catalog_header)),
        ("character selection uses source-pool masks", yes_no("characterSourcePoolMatches" in model and "entry.generationMask" in model and "entry.tierMask" in model and "entry.genderMask" in model)),
        ("same-pool character selection consumes RNG", yes_no("const uint8_t target = nextRand(count);" in model and "chooseCharacterCatalogId(Stage stage" in model)),
        ("Oyajitchi special lineage represented", yes_no("kGrowthFlagOyajitchiLineage" in model and "strcmp(characterCatalogEntry(i).name, \"Oyajitchi\") == 0" in model)),
        ("Oyajitchi elder LOVE condition present", yes_no("stage_ == Stage::kElder" in model and "static_cast<Stage>(packet.stage) == Stage::kElder" in model and "static_cast<Gender>(packet.gender) != gender_" in model)),
        ("Oyajitchi skips ordinary child/teen growth", yes_no(
            "(growthFlags & kGrowthFlagOyajitchiLineage)" in model
            and "ageMinutes >= kBabyToChildAgeMinutes" in model
            and "return Stage::kAdult;" in model
            and "scheduledStageForAge(stage_, growthFlags_, ageMinutes_)" in model
        )),
        ("Oyajitchi lineage forces male baby", yes_no("oyajitchiLineage" in model and "? Gender::kBoy" in model)),
        ("LINK standby same-kind auto-reply present", yes_no(
            "prepareLinkReplyPacket" in model
            and "emitLinkPacketForSelection(kind, replySelection, true)" in ino
            and "link reply emitted" in ino
        )),
        ("LINK game ids separated from local mini-games", yes_no(
            "enum class LinkGameKind" in header
            and "bool prepareLinkGamePacket(LinkGameKind game" in header
            and "uint8_t gameKind;" in header
            and "const LinkGameKind game = echopet::linkGameFromIndex(state.cursor)" in ino
        )),
        ("LINK item-backed games require catalog ownership", yes_no(
            "linkGameOwnedInBits(catalogOwned_, game)" in model
            and "kCatalogBuildingBlockIndex" in item_catalog_header
            and "kCatalogRcCar1Index" in item_catalog_header
            and "if (!ownsLinkGameItem(game))" in model
            and "if (!validLinkGameKind(packet.gameKind) || !ownsLinkGameItem(game))" in model
        )),
        ("LINK Presents carry exact catalog ids", yes_no(
            "GiftKind::kCatalogItem" in model
            and "bool prepareCatalogGiftPacket(uint8_t index" in header
            and "pet.prepareCatalogGiftPacket(state.cursor" in ino
            and "receiveCatalogGift(packet.giftId, inventoryFull)" in model
            and "return kCatalogItemCount + 1;" in read(SOURCE_ROOT / "EchoPetUi.cpp")
        )),
        ("explicit 10-digit password rewards present", yes_no(len(password_reward_rows) >= 32)),
        ("password duplicate guard present", yes_no("reward.repeatable" in model and "kPasswordDone" in model)),
        ("catalog segment manifest present", yes_no("kCatalogSegments" in item_catalog)),
        ("catalog password overrides present", yes_no(len(password_catalog_override_rows) >= 32)),
        ("catalog secret-code overrides present", yes_no(len(catalog_override_rows) == 7 and "kCatalogOverrides" in item_catalog)),
        ("four-slot shop stock mapping present", yes_no("constexpr uint8_t kShopSlotCount = 4" in read(SOURCE_ROOT / "EchoPetCatalog.h") and "catalogShopIndex" in item_catalog)),
        ("shop restock sale vendor rule tables present", yes_no("struct ShopRestockWindow" in item_catalog and "kShopRestockWindows" in item_catalog and "{15U * 60U, 1}" in item_catalog and "{19U * 60U, 2}" in item_catalog and "struct ShopVendorWindow" in item_catalog and "kShopVendorWindows" in item_catalog and "{11U * 60U, 12U * 60U, 0}" in item_catalog and "{17U * 60U, 18U * 60U, 1}" in item_catalog and "struct ShopSaleRule" in item_catalog and "kShopSaleRule = {37, 11, 13, 50}" in item_catalog)),
        ("seasonal birthday/holiday shop substitutions present", yes_no(
            "month == birthdayMonth && day == birthdayDay" in item_catalog
            and "return 87;  // Yogurt birthday item." in item_catalog
            and "month == 2 && day == 14" in item_catalog
            and "month == 11 && day == 14" in item_catalog
            and "month == 3 && day == 12" in item_catalog
            and "month == 10 && day == 14" in item_catalog
            and "month == 3 && day == 14" in item_catalog
            and "month == 5 && day == 14" in item_catalog
            and "month == 12 && (day == 23 || day == 24)" in item_catalog
            and "shopSlot == 1) return 85;  // Turkey." in item_catalog
            and "month == 12 && day == 25" in item_catalog
        )),
        ("souvenir name manifest present", yes_no("kSouvenirNames" in item_catalog and "MEMORY%02u" not in item_catalog)),
        ("source item label/price corrections present", yes_no("\"ENERGY DRINK\"" in item_catalog and "\"MARRON CAKE\"" in item_catalog and "\"ROLLER BLADES\"" in item_catalog and "\"FISHING POLE\"" in item_catalog and "{88, \"TV\", CatalogKind::kItem, 1, 5000" in item_catalog and "{89, \"WEIGHTS\", CatalogKind::kItem, 0, 600" in item_catalog and "{90, \"WIG\", CatalogKind::kItem, 2, 1000" in item_catalog and "{91, \"WINGS\", CatalogKind::kItem, 2, 1000" in item_catalog)),
        ("all non-souvenir catalog rows source-named", yes_no(len(missing_named_catalog_rows) == 0 and len(all_catalog_override_indexes) >= 152)),
        ("source item use-stage/reusable flags present", yes_no("kCatalogFlagChildPlus" in item_catalog_header and "kCatalogFlagTeenPlus" in item_catalog_header and "childPlusStage" in model and "teenPlusStage" in model and "kCatalogFlagReusable | kCatalogFlagChildPlus" in item_catalog and "kCatalogFlagReusable | kCatalogFlagTeenPlus" in item_catalog and "kCatalogFlagTravel | kCatalogFlagAdultOnly" in item_catalog and "\"TAMA DRINK\"" in item_catalog)),
        ("full per-catalog quantity storage present", yes_no("catalogStock[kCatalogItemCount]" in header and "catalogStock_[kCatalogItemCount]" in header and "memcpy(save.catalogStock, catalogStock_" in model and "memcpy(s.catalogStock, catalogStock_" in model and "rebuildCatalogStockFromOwned" in model)),
        ("catalog consumable stock cleanup present", yes_no("clearCatalogStockIfEmpty(uint8_t index" in model and "clearCatalogStockIfEmpty(index, entry)" in model and "catalogBitClear(catalogOwned_, index)" in model)),
        ("catalog storage caps use catalog totals", yes_no("catalogStockTotal(CatalogKind::kFood) >= kMaxFoodStock" in model and "catalogStockTotal(CatalogKind::kItem) >= kMaxItemStock" in model)),
        ("ticket fixed souvenir rewards present", yes_no("ticketSouvenirForCatalogIndex" in model and "kCatalogTicket1Index" in item_catalog_header and "return 9;" in model and "return 13;" in model and "\"SKIS\"" in item_catalog and "\"PALM TREE\"" in item_catalog and "\"SURFBOARD\"" in item_catalog and "\"PANDA BEAR\"" in item_catalog and "\"MARACAS\"" in item_catalog)),
        ("dependent source item behaviors present", yes_no("kCatalogMusicDiscIndex" in item_catalog_header and "kCatalogBoomBoxIndex" in item_catalog_header and "kCatalogMakeupIndex" in item_catalog_header and "kCatalogMirrorIndex" in item_catalog_header and "kCatalogShaverIndex" in item_catalog_header and "kCatalogTamaDrinkIndex" in item_catalog_header and "catalogBitClear(catalogOwned_, kCatalogBoomBoxIndex)" in model and "kCatalogMirrorIndex" in model and "\"Oyajitchi\"" in model)),
        ("Tama Drink sickness guard present", yes_no("kMemoryFlagTamaDrinkGuard" in header and "guardedSickChance" in model and "memoryFlags_ |= kMemoryFlagTamaDrinkGuard" in model and "memoryFlags_ &= ~kMemoryFlagTamaDrinkGuard" in model)),
        ("catalog random item rule table present", yes_no("struct CatalogRandomRule" in model and "kCatalogRandomRules" in model and "catalogRandomRuleFor" in model and "breakChancePercent" in model and "kCatalogPlantIndex, 3, kPlantPointReward" in model and "kCatalogLampIndex, 6, kLampPointReward, 25" in model)),
        ("source random and gendered item effects present", yes_no("kCatalogActionFigureIndex" in item_catalog_header and "kCatalogDoll1Index" in item_catalog_header and "kCatalogDoll2Index" in item_catalog_header and "kCatalogPlantIndex" in item_catalog_header and "kCatalogShovelIndex" in item_catalog_header and "kCatalogChestIndex" in item_catalog_header and "kCatalogFishingPoleIndex" in item_catalog_header and "kCatalogLampIndex" in item_catalog_header and "kCatalogHairGelIndex" in item_catalog_header and "gender_ == Gender::kBoy" in model and "gender_ == Gender::kGirl" in model and "consumeCatalogItem(index, entry)" in model and "catalogStock_[index]--" in model and "catalogBitClear(catalogOwned_, kCatalogLampIndex)" in model and "chestLampExclusiveCatalogItem" in model)),
        ("Chest/Lamp exclusive item pool present", yes_no("chestLampExclusiveCatalogItem" in model and "kCatalogDoll2Index" in model and "kCatalogMakeupIndex" in model and "kCatalogShaverIndex" in model and "kCatalogTamaDrinkIndex" in model)),
        ("Fishing Pole trash-retain reward-destroy behavior present", yes_no("{69, \"FISHING POLE\", CatalogKind::kItem, 7, 400, 7, 0x00" in item_catalog and "consumeCatalogItem(index, entry)" in first(r"(if \(index == kCatalogFishingPoleIndex\).*?changed = true;)", model, re.S) and "awardCatalogItem(kCatalogFishingPoleIndex)" in first(r"(if \(index == kCatalogFishingPoleIndex\).*?changed = true;)", model, re.S))),
    ]
    stat_rows = re.findall(
        r"\{StatMeter::k(\w+),\s*(\d+),\s*(-?\d+),\s*(\d+)\}",
        model,
    )
    growth_effect_checks = [
        (
            "Discipline can clear Naughty attention and raise training",
            yes_no(
                "attentionReason_ == AttentionReason::kNaughty" in model
                and "disciplineScore_ = clampAdd(disciplineScore_, 8)" in model
            ),
        ),
        (
            "Praise can clear Praise/Sad-style attention and raise training",
            yes_no(
                "attentionReason_ == AttentionReason::kPraise" in model
                and "disciplineScore_ = clampAdd(disciplineScore_, 4)" in model
            ),
        ),
        (
            "Missed-call timeout records physical or mental care mistakes",
            yes_no(
                "AttentionTimingKind::kOrdinaryMiss" in model
                and "attentionMinutes_ >= missWindow.periodMinutes" in model
                and "recordCareMistake(CareMistakeClass::kPhysical)" in model
                and "recordCareMistake(CareMistakeClass::kMental)" in model
            ),
        ),
        (
            "Food/snack paths alter hunger, happiness, and weight",
            yes_no(
                "hunger_ = clampAddHungryHappy(hunger_, 25)" in model
                and "happiness_ = clampAddHungryHappy(happiness_, 16)" in model
                and "weight_ = clampAdd16(weight_, 2, 999)" in model
            ),
        ),
        (
            "Sweet snack path can create toothache risk",
            yes_no(
                "kSweetSnackToothacheStreak = 15" in model
                and "sweetSnackStreak_" in header
                and "++sweetSnackStreak_ >= kSweetSnackToothacheStreak" in model
                and "toothache_ = 1" in model
            ),
        ),
        (
            "Sweet snack toothache uses persisted short-period window",
            yes_no(
                "kSweetSnackShortPeriodMinutes = 60" in model
                and "sweetSnackWithinShortPeriod" in model
                and "lastSweetSnackMinute_" in header
                and "save.lastSweetSnackMinute" in model
                and "lastSweetSnackMinute_ = kSweetSnackNoLastMinute" in model
            ),
        ),
        (
            "Sickness trigger rule table present",
            yes_no(
                "struct SicknessTriggerRule" in model
                and "kSicknessTriggerRules" in model
                and "SicknessTriggerKind::kLowHygiene, 18, 8, 1, 2" in model
                and "SicknessTriggerKind::kMess, 3, 18, 1, 2" in model
                and "SicknessTriggerKind::kHighWeight, 35, 8, 1, 2" in model
                and "SicknessTriggerKind::kSweetSnack, 16, 9, 1, 1" in model
                and "sicknessTriggerRuleFor" in model
                and "guardedSickChance(rule.chancePercent" in model
            ),
        ),
        (
            "Sick/toothache state refuses food games and items",
            yes_no(
                "Notice::kSickRefuse" in model
                and "actionBlockedBySickness" in model
                and "Action::kMeal" in sickness_block
                and "Action::kSnack" in sickness_block
                and "Action::kGame" in sickness_block
                and "Action::kItem" in sickness_block
                and model.count("notice_ = Notice::kSickRefuse;") >= 5
            ),
        ),
        (
            "Untreated toothache escalates before care mistake",
            yes_no(
                "missedToothacheOnly" in model
                and "toothache_ = 0;" in model
                and "becomeSick(1);" in model
                and "attentionMinutes_ = 0;" in model
                and "recordCareMistake(CareMistakeClass::kPhysical)" in model
            ),
        ),
        (
            "Fourth sickness in one stage can pass away",
            yes_no(
                "kStageSicknessPassAwayCount = 4" in model
                and "uint8_t stageSicknesses_" in header
                and "uint8_t stageSicknesses;" in header
                and "stageSicknesses_++" in model
                and "stageSicknesses_ >= kStageSicknessPassAwayCount" in model
                and "stageSicknesses_ = 0;" in model
                and "save.stageSicknesses = stageSicknesses_" in model
            ),
        ),
        (
            "Game completion changes hunger/happiness/weight and can reverse care mistakes",
            yes_no(
                "hunger_ = clampAddHungryHappy(hunger_, -static_cast<int>(3 + scoreTier))" in model
                and "happiness_ = clampAddHungryHappy(happiness_, kHappyCurve[scoreTier])" in model
                and "baseWeightForStage(stage_)" in model
                and "if (weight_ > baseWeight) weight_--" in model
                and "reverseCareMistake(CareMistakeClass::kMental)" in model
                and "reverseCareMistake(CareMistakeClass::kPhysical)" in model
            ),
        ),
        (
            "Games refuse at source-shaped stage base weight instead of hunger/energy gates",
            yes_no(
                "uint16_t baseWeightForStage(Stage stage)" in model
                and "const StageBaseWeightRule kStageBaseWeightRules" in model
                and "{Stage::kChild, 10}" in model
                and "{Stage::kTeen, 20}" in model
                and "{Stage::kAdult, 30}" in model
                and "weight_ <= baseWeightForStage(stage_)" in model
                and "energy_ < 12 || hunger_ < 8" not in model
            ),
        ),
        (
            "Sleep schedule drives asleep mood and bedtime lights behavior",
            yes_no(
                "bool EchoPetModel::isBedtime" in model
                and "sleepWindowForStage(stage_)" in model
                and "mood_ = Mood::kAsleep" in model
                and "lightsOff_" in model
            ),
        ),
        (
            "Stat decay table covers hunger, happiness, energy, and hygiene",
            yes_no({"Hunger", "Happiness", "Energy", "Hygiene"}.issubset(
                {row[0] for row in stat_rows}
            )),
        ),
    ]

    pass_away_block = first(
        r"const PassAwayRule kPassAwayRules\[\]\s*=\s*\{(.*?)\};",
        model,
        re.S,
    )
    pass_away_rows = re.findall(
        r"\{([^{}]+?),\s*(\d+),\s*([^,{}]+),\s*([^,{}]+),\s*([^,{}]+),\s*([^,{}]+)\}",
        pass_away_block,
        re.S,
    )
    adult_growth_block = first(
        r"const AdultGrowthRule kAdultGrowthRules\[\]\s*=\s*\{(.*?)\};",
        model,
        re.S,
    )
    adult_growth_rows = re.findall(
        r"\{GrowthTier::k(\w+),\s*([^,{}]+),\s*([^,{}]+),\s*"
        r"([^,{}]+),\s*([^,{}]+),\s*([^,{}]+),\s*AdultTier::k(\w+)\}",
        adult_growth_block,
        re.S,
    )
    game_reward_block = first(
        r"const GameRewardRule kGameRewardRules\[\]\s*=\s*\{(.*?)\};",
        model,
        re.S,
    )
    game_reward_rows = re.findall(
        r"\{Stage::k(\w+),\s*MiniGameKind::k(\w+),\s*(\d+),\s*(\d+),\s*(\d+)\}",
        game_reward_block,
    )
    game_score_curve_block = first(
        r"const uint8_t kGameScorePrizePercent\[[^\]]*\]\s*=\s*\{(.*?)\};",
        model,
        re.S,
    )
    game_score_curve = [
        int(value) for value in re.findall(r"\d+", game_score_curve_block)
    ]
    bump_prize_block = first(
        r"const BumpPrizeRow kBumpPrizeChart\[\]\s*=\s*\{(.*?)\};",
        model,
        re.S,
    )
    bump_prize_rows = re.findall(
        r"\{(\d+),\s*\{(\d+),\s*(\d+),\s*(\d+),\s*(\d+)\}\}",
        bump_prize_block,
    )
    game_unlock_block = first(
        r"const GameUnlockRule kGameUnlockRules\[\]\s*=\s*\{(.*?)\};",
        model,
        re.S,
    )
    game_unlock_rows = re.findall(
        r"\{Stage::k(\w+),\s*(\d+)\}",
        game_unlock_block,
    )
    game_pacing_block = first(
        r"const GamePacingRule kGamePacingRules\[\]\s*=\s*\{(.*?)\};",
        model,
        re.S,
    )
    game_pacing_rows = re.findall(
        r"\{MiniGameKind::k(\w+),\s*(\d+),\s*(\d+),\s*(\d+),\s*([^,]+),\s*(.*?)\}",
        game_pacing_block,
        re.S,
    )
    catalog_segment_rows = parse_catalog_segment_rows(item_catalog,
                                                      catalog_const)
    souvenir_names = re.findall(r'"([^"]+)"', first(
        r"const char\* const kSouvenirNames\[kCatalogSouvenirCount\]\s*=\s*\{(.*?)\};",
        item_catalog,
        re.S,
    ))

    character_rows = re.findall(r'CHARACTER_ROW(?:_TRAITS)?\(\s*"[^"]+".*?\)',
                                catalog, re.S)
    character_count = len(character_rows)
    character_rows_with_source = sum(
        1 for row in character_rows
        if re.search(r',\s*[1-7]\s*,\s*[1-8]\s*,\s*\d+(?:\s*,\s*k\w+)?\s*\)$',
                     row.strip(), re.S)
    )
    character_rows_with_source_pool = sum(
        1 for row in character_rows
        if "kCharacterGeneration" in row
        and "kCharacterTier" in row
        and "kCharacterGender" in row
    )
    placeholder_names = re.findall(r'"(Mame-[^"]+|Meme-[^"]+|Kuchi-[^"]+|Tama-[^"]+|Elder-[^"]+)"', catalog)

    OUT.parent.mkdir(parents=True, exist_ok=True)
    lines = [
        "# EchoPet Growth Coverage Report",
        "",
        "Generated by `analysis/growth_coverage/generate_growth_coverage.py`.",
        "",
        "This report checks implementation coverage only. It does not prove exact",
        "Connection parity; source transcription, 50+ official route rows, and",
        "hardware observation still gate acceptance.",
        "",
        "## Timing Constants",
        "",
        "| Constant | Code value |",
        "| --- | --- |",
    ]
    lines += [f"| `{name}` | `{value}` |" for name, value in constant_rows]
    lines += [
        "",
        "## Required Model Fields",
        "",
        "| Token | Present |",
        "| --- | --- |",
    ]
    lines += [f"| `{token}` | {present} |" for token, present in header_rows]
    lines += [
        "",
        "## Code Path Checks",
        "",
        "| Check | Result |",
        "| --- | --- |",
    ]
    lines += [f"| {name} | {result} |" for name, result in model_checks]
    lines += [
        "",
        "## Growth Effect Path Checks",
        "",
        "| Effect path | Source coverage |",
        "| --- | --- |",
    ]
    lines += [f"| {name} | {result} |" for name, result in growth_effect_checks]
    lines += [
        "",
        "## Stat Decay Table",
        "",
        "| Meter | Period minutes | Delta | Skips bedtime |",
        "| --- | --- | --- | --- |",
    ]
    lines += [
        f"| {meter} | `{period}` | `{delta}` | {yes_no(skip == '1')} |"
        for meter, period, delta, skip in stat_rows
    ]
    lines += [
        "",
        "## Pass-Away Table",
        "",
        "| Flags | Min mistakes | Min age | Max hunger | Max happy | Max hygiene |",
        "| --- | --- | --- | --- | --- | --- |",
    ]
    for flags, mistakes, age, hunger, happy, hygiene in pass_away_rows:
        if "StatMeter::" in flags:
            continue
        if "kPassAway" not in flags and flags.strip() != "0":
            continue
        lines.append(
            f"| `{flags.strip()}` | `{mistakes}` | `{age.strip()}` |"
            f" `{hunger.strip()}` | `{happy.strip()}` | `{hygiene.strip()}` |"
        )
    lines += [
        "",
        "## Adult Growth Rule Table",
        "",
        "| Previous tier | Physical range | Mental range | Flags | Result |",
        "| --- | --- | --- | --- | --- |",
    ]
    lines += [
        f"| {tier} | `{phys_min.strip()}..{phys_max.strip()}` |"
        f" `{ment_min.strip()}..{ment_max.strip()}` | `{flags.strip()}` |"
        f" {result} |"
        for (
            tier,
            phys_min,
            phys_max,
            ment_min,
            ment_max,
            flags,
            result,
        ) in adult_growth_rows
    ]
    lines += [
        "",
        "## Game Unlock Table",
        "",
        "| Stage | Unlocked count |",
        "| --- | --- |",
    ]
    lines += [f"| {stage} | `{count}` |" for stage, count in game_unlock_rows]
    lines += [
        "",
        "## Game Reward Table",
        "",
        "| Stage | Game | Full prize | Heavy prize | Heavy weight |",
        "| --- | --- | --- | --- | --- |",
    ]
    lines += [
        f"| {stage} | {game} | `{prize}` | `{heavy}` | `{weight}` |"
        for stage, game, prize, heavy, weight in game_reward_rows
    ]
    lines += [
        "",
        "## Game Score Prize Curve",
        "",
        "| Score | Full-prize percent |",
        "| ---: | ---: |",
    ]
    lines += [
        f"| {score} | `{percent}` |"
        for score, percent in enumerate(game_score_curve)
    ]
    lines += [
        "",
        "## Bump Prize Chart",
        "",
        "| Source rounds won | 0-50 lb | 51-75 lb | 76-90 lb | 91-99 lb |",
        "| ---: | ---: | ---: | ---: | ---: |",
    ]
    lines += [
        f"| `{rounds}` | `{band0}` | `{band1}` | `{band2}` | `{band3}` |"
        for rounds, band0, band1, band2, band3 in bump_prize_rows
    ]
    lines += [
        "",
        "## Game Pacing Contract",
        "",
        "| Game | Source goal | Firmware rounds | Score cap | Auto window ms | Flags |",
        "| --- | --- | --- | --- | --- | --- |",
    ]
    lines += [
        f"| {game} | `{source}` | `{rounds}` | `{cap}` | `{window.strip()}` |"
        f" `{clean_cpp_flags(flags)}` |"
        for game, source, rounds, cap, window, flags in game_pacing_rows
    ]
    lines += [
        "",
        "## Character Catalog Coverage",
        "",
        f"- Catalog rows found: `{character_count}`.",
        f"- Rows with source page/slot/frame family: `{character_rows_with_source}`.",
        f"- Rows with generation/tier/gender source-pool masks: `{character_rows_with_source_pool}`.",
        f"- Placeholder rows still present: `{len(placeholder_names)}`.",
    ]
    if placeholder_names:
        lines.append(f"- Placeholder names: `{', '.join(sorted(set(placeholder_names)))}`.")
    lines += [
        "",
        "## Item Catalog Segment Manifest",
        "",
        "| Start | End | Prefix | Kind | Behavior base | Behavior modulo | Base price | Price base | Price modulo | Price step | Flags | Source group | Use scene |",
        "| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |",
    ]
    lines += [
        f"| `{start}` | `{end}` | {prefix} | {kind} | `{behavior_base}` |"
        f" `{behavior_mod}` | `{base_price}` | `{price_base}` |"
        f" `{price_mod}` | `{price_step}` | `{flags}` | `{source_group}` |"
        f" `{use_scene}` |"
        for (
            start,
            end,
            prefix,
            kind,
            behavior_base,
            behavior_mod,
            base_price,
            price_base,
            price_mod,
            price_step,
            flags,
            source_group,
            use_scene,
        ) in catalog_segment_rows
    ]
    lines += [
        "",
        "## Souvenir Manifest",
        "",
        f"- Non-souvenir catalog override rows found: `{len(all_catalog_override_indexes)}`.",
        f"- Non-souvenir rows still using generated fallback labels: `{len(missing_named_catalog_rows)}`.",
        f"- Named souvenir rows found: `{len(souvenir_names)}`.",
        f"- Numeric MEMORY placeholders present: `{yes_no('MEMORY%02u' in item_catalog)}`.",
    ]
    if missing_named_catalog_rows:
        lines.append(
            f"- Missing named catalog rows: `{', '.join(map(str, missing_named_catalog_rows))}`."
        )
    lines += [
        "",
        "## Remaining P9-02 Gaps",
        "",
        "- Odd/even generation character pools are now encoded as compact source-pool masks; final per-character art and hardware-visible proof remain open.",
        "- Parent-pair child tier rules and FAMILY sourced ancestry rows are coded; final official art and hardware-visible proof remain open.",
        "- Adult Naughty/Frail/Stubborn thresholds are now explicit firmware table rows transcribed from `WIKI-CHAR-2024`; exact same-pool probabilities, final art, and hardware-visible proof remain open.",
        "- Oyajitchi special lineage is coded; exact official cutscene wording/art and two-device visibility still require resource/hardware proof.",
        "- Character rows now carry compact source page/slot and visual-family IDs; exact route/probability rows and final per-character art remain open.",
    ]
    OUT.write_text("\n".join(lines) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()

