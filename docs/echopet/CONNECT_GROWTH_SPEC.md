# EchoPet Connect Growth Specification

Status: P9-02 adult-rule-table and source-pool character selection tranches
are implemented and compiled. The adult Serious/Normal/Naughty/Frail/Stubborn
threshold rows are transcribed into firmware tables. This is not full parity
yet: exact low-probability edge cases, same-pool route probability proof,
hardware-visible transition proof, and final per-character art remain open.

Primary sources:

- `CONNECT_SOURCE_LEDGER.md`
- `WIKI-CHAR-2024`
- `WIKI-FAQ-2024`
- `GG-GROWTH-2024`
- `OFF-HOWTO` for official top-level scope

## Required Model Concepts

The current EchoPet model has one aggregate `careMistakes_` counter and a local
`GrowthRoute` score algorithm. That structure is not adequate for Connection
parity. The growth model needs these concepts:

| Concept | Required representation | Why |
| --- | --- | --- |
| Physical care mistakes | Stage-local counter. | CODED: stored in model/save/snapshot. Exact source table still needed for every trigger. |
| Mental care mistakes | Stage-local counter. | CODED: stored in model/save/snapshot. Exact source table still needed for every trigger. |
| Stage-local reset | Reset physical/mental counters at each growth stage. | CODED. Needs model test coverage. |
| Growth tier | Tier 1-4 for child/teen ancestry and Serious/Normal/Naughty/Frail/Stubborn for adult result. | CODED as explicit child, teen, and adult rule tables. Adult tier ranges are transcribed into `kAdultGrowthRules`; child/teen trigger edge cases and hardware proof remain open. |
| Generation parity | Odd/even generation selector. | CODED-SUBSTITUTE: `CharacterCatalogEntry` rows carry generation masks, and `chooseCharacterCatalogId()` filters by current odd/even/first generation before selecting from the pool. |
| Parent adult tiers | Store both parents' adult tiers for child tier selection. | CODED: stored in save/snapshot, carried in LINK `FriendPacket` v3, and consumed by the generation 2+ child-tier table. |
| Born-from-unhealthy flag | Derived from parent tiers. | CODED: the saved flag means both parents are in unhealthy adult tiers, matching the source Serious-adult gate. The field name is retained for save compatibility. |
| Perfect-game reversal | Decrement specific mistake class on exact games. | CODED for Get/Flag mental and Bump/Heading physical. Exact perfect conditions still need game table pass. |
| Current-stage character pool | Explicit table of candidates. | CODED-SUBSTITUTE: all 50 catalog rows have generation/tier/gender source-pool masks plus compact source page/slot/frame-family IDs. Exact same-pool probability behavior, final art, and hardware-visible proof still need source/hardware validation. |

## Life Stage Timing

| Stage | Target duration / trigger | Source confidence | Current EchoPet behavior |
| --- | --- | --- | --- |
| Egg | Hatch after setup; exact 2024 delay still needs confirmation. Provisional target remains "a few minutes" until manual/hardware proof. | C | CODED: 5 minutes. |
| Baby | 1 hour. | B | CODED: child threshold is egg hatch + 60 minutes. |
| Child | 24 hours. | B | CODED: teen threshold is child threshold + 1440 minutes. |
| Teen | 72 hours. | B | CODED: adult threshold is teen threshold + 4320 minutes. |
| Adult | About 4 adult days before senior if kept unmarried; matchmaker begins 4 adult days after adult transition. | B | CODED-SUBSTITUTE: elder threshold is adult threshold + 4 days; matchmaker first window is also adult threshold + 4 days at 10:30/15:00/19:00. Exact overlap behavior still needs hardware/source proof. |
| ParentCare | Parent stays with baby for 24 hours. | B | CODED: 1440 minutes. Visible partner/baby flow remains incomplete. |
| Elder/Senior | Infinite until death by neglect or marriage; senior obtained by keeping adult unmarried through the about-4-day adult duration. | B | CODED-SUBSTITUTE. Opposite-gender elder LOVE now preserves the Oyajitchi lineage; exact transition timing and matchmaker/elder overlap still need source or hardware-visible proof. |

Implementation note: `ECHOPET_MINUTE_MS` may remain configurable for accelerated
test builds, but the pet-minute thresholds must be parity values. Test
acceleration must scale the minute duration, not shrink the growth table.

## Care Mistake Classification

| Trigger | Class | Timeout | Code status |
| --- | --- | --- | --- |
| Hunger meter empty and attention call missed | Physical | 15 minutes | CODED as physical through attention timeout and empty hunger pressure. Exact call timing still needs a source table. |
| Scolding call missed | Physical | 15 minutes | CODED through `AttentionReason::kNaughty`. Naming still needs a UI/spec pass. |
| Sick call not cured | Physical | 15 minutes | CODED through attention timeout. A missed toothache-only call first escalates to sickness and resets the timer; the later missed sickness call records the physical mistake. |
| Happy meter empty and attention call missed | Mental | 15 minutes | CODED as mental through attention timeout and empty happy pressure. |
| Praise call missed | Mental | 15 minutes | CODED through `AttentionReason::kPraise`. Exact false-call behavior still needs a UI pass. |
| Lights left on after sleep call | Mental | 60 minutes | CODED as mental, but still uses a simplified sleep-call pattern rather than a fully sourced bedtime table. |
| Toothache alone | Not a care mistake | N/A | CODED: toothache is not recorded directly; if its care call is missed, it becomes sickness before any care mistake can be recorded. |
| Wrong discipline action | Not a care mistake | N/A | CODED: wrong discipline applies feedback but does not increment care mistakes. |

## Care Mistake Reversal

| Game | Reversed class | Required trigger | Current code status |
| --- | --- | --- | --- |
| Get | Mental | Perfect score. Exact score threshold still needs table pass. | CODED for score 5. |
| Flag | Mental | Perfect score. Exact score threshold still needs table pass. | CODED for score 5. |
| Bump | Physical | Perfect score / full win condition. Exact threshold still needs table pass. | CODED for score 5. |
| Heading | Physical | Perfect score. Exact threshold still needs table pass. | CODED for score 5. |

Reversal should not hide the original mistake event from diagnostics. The model
should track both raw mistakes and effective mistakes if later evidence shows
the original device distinguishes them.

## Tier Names

Use these model names rather than the existing route labels:

| Internal name | Player/source tier | Meaning |
| --- | --- | --- |
| `Tier1` | Serious | Healthiest/lowest mistake path. |
| `Tier2` | Normal | Good/medium path. |
| `Tier3` | Naughty or Frail branch input | Mixed mistake pressure; exact adult split depends on physical vs mental. |
| `Tier4` | Stubborn / weakest ancestry path | High mistake path. |
| `AdultSerious` | Serious adult result | 0-1 mistakes, constrained by parent/teen tier. |
| `AdultNormal` | Normal adult result | Moderate mistakes. |
| `AdultNaughty` | Naughty adult result | Mental-heavy mistakes. |
| `AdultFrail` | Frail adult result | Physical-heavy mistakes. |
| `AdultStubborn` | Stubborn adult result | High physical and mental mistakes. |

The current `GrowthRoute::kBalanced/Athlete/Scholar/Social/Dreamer/Rascal` can
remain temporarily for rendering archetypes only, but it must stop deciding
growth parity once the tier table is implemented.

## Baby Selection

| Generation context | Candidate(s) | Rule |
| --- | --- | --- |
| First generation | `Kuroteletchi`, `Shiroteletchi` | 50/50 gender-linked baby selection. |
| After two adults marry | `Kuroteletchi`, `Shiroteletchi` | 50/50, unless a special parent-pair rule overrides it. |
| Special Oyajitchi path | Male baby then Oyajitchi path | CODED: opposite-gender elder LOVE sets a special lineage flag, parent departure forces a male baby, and the baby grows directly to Oyajitchi after the baby duration. |

## Child Tier Selection

First generation:

| Candidate pool | Rule |
| --- | --- |
| `Tamatchi` / `Mohitamatchi` family | First generation always uses the first-generation child pool, with 50/50 selection where applicable. |

Generation 2+:

| Result tier | Parent adult tier pairing |
| --- | --- |
| Tier 1 | Serious + Serious, or Serious + Normal. |
| Tier 2 | Serious + Naughty/Frail/Stubborn, or Normal + Normal/Naughty. |
| Tier 3 | Normal + Frail/Stubborn, or Naughty + Naughty/Frail. |
| Tier 4 | Frail + Frail, or Stubborn + Naughty/Frail/Stubborn. |

Implementation status: generation 2+ child tier selection now uses
`childTierFromParentAdultTiers()` instead of baby-stage mistake totals. Both
parents' adult tiers and known catalog identities are stored in the active
parent-care state before the parent leaves; family-history display remains
compact but can now expose the known LOVE partner portrait in the ancestry row.

## Teen Tier Selection

First generation:

| Teen tier | Care mistakes |
| --- | --- |
| Tier 1 | 0-1 physical or mental mistakes. |
| Tier 2 | 2 physical or mental mistakes. |
| Tier 3 | 3 physical or mental mistakes. |
| Tier 4 | 4+ physical or mental mistakes. |

Generation 2+:

| Child tier | Teen tier result |
| --- | --- |
| Tier 1 child | Tier 1 if both mistake classes are <=2; Tier 2 if both classes are >=3. If only one class exceeds 2, remain Tier 1. |
| Tier 2 child | Tier 2 if both mistake classes are <=2; Tier 3 if both classes are >=3. If only one class exceeds 2, remain Tier 2. |
| Tier 3 child | Tier 3 if both mistake classes are <=3; Tier 4 if both classes are >=4. If only one class exceeds 3, remain Tier 3. |
| Tier 4 child | Tier 4 for any mistake count. |

Odd/even generations choose different character names for the same tier.
Character-pool mapping is now a firmware data table: each teen row carries a
generation mask plus healthy/unhealthy tier mask, and the model selects within
the matched pool with `nextRand()`.

## Adult Tier Selection

Adult selection depends on teen tier and the physical-vs-mental mistake split.

| Adult result | From Tier 1/2 teen | From Tier 3/4 teen |
| --- | --- | --- |
| Serious | Tier 1: 0-1 mistakes in either class. Tier 2: physical = 0 and mental 0-1. | Tier 3: physical = 0 and mental = 0. Tier 4: not obtainable. |
| Normal | Tier 1: 2 mistakes in either class. Tier 2: physical 1-2 and mental 2. | Tier 3: 1 mistake in either class. Tier 4: 0-1 mistakes in either class. |
| Naughty | Physical low, mental high. | Lower mental threshold than Tier 1/2 teen. |
| Frail | Physical high, mental low. | Lower physical threshold than Tier 1/2 teen. |
| Stubborn | Physical high and mental high. | Lower threshold than Tier 1/2 teen. |

Firmware now routes adult selection through `kAdultGrowthRules`, an explicit
table keyed by previous teen tier, physical mistake range, mental mistake range,
and healthy-parent requirement. Mental-heavy results route to Naughty,
physical-heavy results route to Frail, and high physical plus high mental
pressure routes to the legacy `AdultTier::kSpecial` bucket used as the Stubborn
substitute. The Serious-adult parent gate now only blocks the Serious result
when both parents are unhealthy. The threshold rows are transcribed from
`WIKI-CHAR-2024`; exact same-pool character probability, edge-case transition
ordering, and hardware-visible proof still need validation before this can be
marked exact Connection parity.

Odd/even generations choose different adult character names for the same adult
tier. Same-tier characters are selected from that generation's source-pool
mask with `nextRand()`, while the old `GrowthRoute` label remains only as a
rendering fallback/archetype hint.

## Senior / Elder Selection

| Result | Rule |
| --- | --- |
| `Ojitchi` | Male senior, obtained by keeping an adult unmarried to the age threshold. |
| `Otokitchi` | Female senior, same condition. |
| `Oyajitchi` | CODED: special path from opposite-gender elder LOVE; the next-generation male baby becomes Oyajitchi after the baby duration and skips the ordinary child/teen table. |

Current code has `Stage::kElder` plus a saved `growthFlags` bit for the special
Oyajitchi lineage. Exact original cutscene wording/art still needs visual
production and hardware observation.

## Character Catalog Migration

Current catalog status:

- It contains the 50 official names listed across the official character pages.
- Placeholder names such as `Mame-A`, `Tama-Friend`, and `Elder-A` have been
  removed.
- It still assigns each entry to local routes such as `Balanced`, `Scholar`, and
  `Rascal`; these are visual fallback routes, not the source growth tiers.
- Each row now also carries generation, tier, and gender masks used by
  `chooseCharacterCatalogId()`; route labels are no longer the primary
  selector for growth results.
- It maps many named characters to a small `CharacterKind` archetype set, which
  is acceptable only as temporary rendering fallback.

Required catalog fields:

| Field | Purpose |
| --- | --- |
| `id` | Stable character ID independent of display name. |
| `name` | Character display name. |
| `stage` | Baby, child, teen, adult, senior, costume, parent. |
| `generationParity` | CODED as compact generation masks: first, odd, even, after-first, or any. |
| `growthTier` | CODED as tier masks: Tier1-4, healthy/unhealthy, special, or any. |
| `parentRule` | CODED in model for child parent-pair tier and special senior/Oyajitchi path; not stored as a separate catalog column yet. |
| `genderRule` | CODED as male/female/any masks in each row. |
| `likedFood` / `dislikedFood` | Needed for P9 food/item parity. |
| `visualFamily` | Resource family name; may point to fallback until P11 completes. |
| `sourceId` | Ledger source ID. |

## Code Migration Tasks

1. DONE: Add data structures for growth tiers and care mistake classes.
2. CODED: Extend save schema for stage-local physical/mental mistakes, current
   tier, parent tiers, born-from-unhealthy ancestry, family catalog identity,
   and sweet-snack short-period tracking. Raw/effective mistake history and
   special lineage still need expansion.
3. CODED: Replace the old `chooseRoute()` heuristic with growth-state
   projection.
4. CODED: Keep old `GrowthRoute` only as a visual fallback until P11 resources
   are complete.
5. CODED: Replace `updateGrowth()` thresholds with real pet-minute constants.
6. CODED: Reset stage-local care mistakes on growth.
7. CODED: Implement perfect-game reversal per mistake class.
8. CODED: Add generation 2+ parent-pair child-tier table and LINK packet adult
   tier propagation so Love/partner/baby flow can preserve both parents'
   growth paths.
9. CODED: Add Oyajitchi special lineage representation for opposite-gender
   elder LOVE, forced male baby, and one-hour direct Oyajitchi growth.
10. CODED: Replace adult-tier if/else routing with an explicit
   `AdultGrowthRule` table reported by host-side coverage.
11. CODED: Replace route-first character selection with generation/tier/gender
   source-pool filtering and candidate-pool RNG.
12. CODED: Add initial host-side growth coverage report under
   `analysis/growth_coverage/`.

## Acceptance Criteria For P9-02

- `EchoPetModel` no longer decides growth from local score heuristics.
- Stage timing uses parity minute thresholds.
- Physical and mental mistakes are stored separately and reset per stage.
- Teen/adult character pool selection depends on generation parity.
- Character catalog rows expose generation/tier/gender source-pool masks, and
  `chooseCharacterCatalogId()` consumes those masks before selecting a row.
- Parent-pair child tier rules exist for generation 2+.
- Senior and special Oyajitchi paths are represented.
- Every character table row points back to `CONNECT_SOURCE_LEDGER.md`.
- A generated report can list the source-pool table and current implementation
  coverage before any firmware is flashed.
