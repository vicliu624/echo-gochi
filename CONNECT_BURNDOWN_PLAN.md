# EchoPet Connect Parity Burn-Down Plan

This plan turns the checklist into an execution order. The target is not "more
features"; the target is measurable Tamagotchi Connection-style player-visible
parity on T-Echo-Lite.

## Source Hierarchy

Use sources in this order when there is a conflict:

1. Official Bandai Tamagotchi Connection 2024 manual and official website pages.
2. Local concept GIFs supplied in `C:\Users\vicliu\Projects\pet`, when they
   define motion phases or interaction expectations.
3. V3-era manuals and community research for precise legacy values that the
   official 2024 pages do not publish.
4. Hardware observation on T-Echo-Lite, for refresh, button timing, sprite
   proof, and two-device link behavior.

Primary source links:

- Official how-to page:
  `https://tamagotchi-official.com/gb/series/connection/howto/`
- Official character list:
  `https://tamagotchi-official.com/gb/series/connection/character/`
- Official manual index:
  `https://tamagotchi-official.com/manual/toy/connection/`
- Official English manual PDF:
  `https://tamagotchi-official.com/manual/toy/connection/connection_web_manual_IS_EN.pdf`
- Official password/news page:
  `https://tamagotchi-official.com/gb/series/connection/news/?c=update`
- V3 manual reference:
  `https://tamaplanet.com/images/docs/Tama_Manuals/tama_v3.pdf`

Official image/legal boundary remains strict: official images are reference
material for scale, pose, timing, and frame requirements. Do not embed copied
official artwork unless rights-cleared assets are supplied.

## Burn-Down Gates

A row is accepted only after it passes its own gate.

| Gate | Meaning | Required evidence |
| --- | --- | --- |
| `SPEC` | Behavior and data are specified against references. | Table/spec doc with source notes. |
| `DATA` | Exact data is represented in code or generated assets. | Code table or asset manifest. |
| `UI` | Player-visible screens and A/B/C flow exist. | Firmware behavior plus local screenshots/photos. |
| `SIM` | Deterministic tests or host checks pass where possible. | Build/test output or generated audit report. |
| `HW` | T-Echo-Lite observation passes. | `T_ECHO_LITE_HARDWARE_ACCEPTANCE.md` entry with result. |
| `DUAL` | Two-device behavior passes. | Link transcript plus both-device observations. |

## Phase 0: Flashed Baseline

- Status: done for the current ordinary `EchoPet` build on 2026-06-17.
- Evidence: PlatformIO serial DFU uploaded `.pio\build\EchoPet\firmware.zip`
  through bootloader `COM47` and reported `Device programmed.`
- Next firmware target: after each meaningful code tranche, produce a fresh UF2
  and DFU zip, then flash ordinary and LoRa builds separately.

## Phase 1: Lock Precise Parity Data

Goal: stop placeholder values from driving gameplay.

Tasks:

- Source ledger: record source IDs, confidence levels, firmware-use rules, and
  current-code mismatches in `CONNECT_SOURCE_LEDGER.md`.
- Growth route table: stage timing, generation effects, gender/odd-even effects,
  care mistakes, training, weight, happiness, hunger, sleep, discipline, social,
  and random branch constraints in `CONNECT_GROWTH_SPEC.md`.
- Death/pass-away/runaway table: illness, hunger/happiness depletion, age,
  care-mistake accumulation, neglect timers, memory/rebirth behavior, and reset
  prompt behavior.
- Stat decay/event table: hunger, happiness, weight, training, poop, illness,
  toothache, sleep, attention, missed calls, and clock-change/pause side effects.
- Shop table: shop inventory groups, prices, stock limits, daily/rotating stock,
  sold-out/full/no-money behavior, item use restrictions, and adult-only items.
- Password/secret-code table: 10-digit password menu items, shop-owner surprised
  secret-code entry, duplicate code behavior, unlock/store/result screens.
- Game reward table: Get, Bump, Flag, Heading, Memory, Sprint, Hoops unlocks,
  scoring, fail states, happiness/weight/money effects, round count, reward
  curve, and age/stage restrictions.

Acceptance:

- `SPEC` complete for each table.
- Existing placeholder catalog names like `MEAL001` and synthetic price curves
  are not accepted as final parity data.

## Phase 2: Convert Data Into Firmware Behavior

Goal: make every table drive actual model/UI behavior.

Tasks:

- Replace generated catalog placeholders with explicit table entries.
- Replace deterministic synthetic growth route shortcuts with table-driven
  branch resolution.
- Tie death/pass-away and rebirth to documented thresholds.
- Drive game payout and stat deltas from exact reward tables.
- Make password/shop code results deterministic and duplicate-aware.
- Add save-schema fields only when the player-visible behavior requires them.

Acceptance:

- `DATA`, `UI`, and relevant `SIM` gates pass.
- No new table is accepted unless it has a visible screen/state path.

## Phase 3: Care, Shop, Password, And Game Scene Completion

Goal: remove all text-page/demo behavior and match the GIF-derived scene
contracts.

Tasks:

- Food/snack prop and bite/chew/crumb/result animation.
- Poop cleanup sweep/flush progressive animation.
- Medicine sick/dose/wait/cure/refusal animation.
- Discipline TIME OUT/PRAISE/invalid/missed-attention feedback.
- Lights ON/OFF picker, sleep/Z, dark-room, wake/invalid feedback.
- Shop booth/shopkeeper/item preview/purchase result animation.
- Seven game scene contracts from `GIF_ANIMATION_AUDIT.md`.

Acceptance:

- `UI` and `HW` gates pass for every fixed icon and every game.
- No scene is accepted if the player must infer the action from text alone.

## Phase 4: IR Visible Behavior Before LoRa

Goal: define the Connection user flow independently from the carrier.

Tasks:

- Specify IR-visible menus and sequence for Visit, Present, Game, and Love.
- Specify roles, standby, send, receive, timeout, cancel, fail, duplicate,
  friend update, present receive, game result, and relation update screens.
- Define exact packet-to-model contract only after the user-visible sequence is
  stable.
- Preserve the same user-visible sequence when LoRa carries the packet.

Acceptance:

- `SPEC` complete in `LINK_CONTRACT.md`.
- `DUAL` complete with two devices before LoRa parity is accepted.

## Phase 5: Love / Partner / Baby / Next Generation

Goal: make Love a complete Connection-style visible flow, not a reward flag.

Tasks:

- Adult compatibility and relation thresholds.
- Love standby and receive/send animation.
- Partner appearance, hearts/visit/accept/reject result.
- Baby appearance, parent/baby care state, parent departure, next generation
  setup/hatch transition.
- Save continuity: generation, parent memory, friend/family history.

Acceptance:

- `SPEC`, `DATA`, `UI`, and `DUAL` gates pass.
- One-device simulation is not enough; the visible two-device flow must pass.

## Phase 6: Official-Feeling Replaceable Resource Set

Goal: build a complete low-resource original resource set that can validate
movement and silhouette at both screen sizes.

Tasks:

- 50+ character route resources: named silhouettes and frame families for egg,
  baby, child, teen, adult, elder, parent, baby/next-gen, pass-away.
- Mametchi-like default proof character refined first, because it is the visual
  yardstick for size, eyes, mouth, feet, and local feature animation.
- 150+ item/dish/souvenir icons: explicit manifest, 64-class and 128-class
  render profiles, per-item fallback policy, and per-item proof status.
- Fixed menu icon refinement: ten official-style but original icons, readable on
  both layouts.

Acceptance:

- `DATA`: every character/item has a manifest entry.
- `UI`: every used frame can be shown by sprite proof.
- `HW`: sprite proof photos record alignment and defects.

## Phase 7: T-Echo-Lite Hardware Acceptance

Goal: close all HW rows with evidence.

Tasks:

- Buttons: Esc/Home/Email short press, long press, debounce, release timing, and
  accidental-repeat checks.
- Refresh: baseline boot screen, menu cycling, idle animation, care scenes,
  dark-room, shop, password, each game, and long repeated operation.
- Sprite proof: every named frame family in `VISUAL_REFERENCE_CONTRACT.md`.
- Two-device link: ordinary carrier/LoRa flow where available, including
  timeout/cancel/failure paths.
- Regression package: UF2/DFU zip, build log, hardware log, and known defects.

Acceptance:

- `T_ECHO_LITE_HARDWARE_ACCEPTANCE.md` contains pass/fail observations.
- A feature with no real-device evidence remains `HW`, not `DONE`.
