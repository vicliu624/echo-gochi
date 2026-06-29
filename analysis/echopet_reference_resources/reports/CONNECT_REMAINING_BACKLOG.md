# EchoPet Remaining Burn-Down Backlog

This is the detailed execution backlog derived from
`CONNECT_PARITY_TASKS.md`, `CONNECT_ALIGNMENT_CHECKLIST.md`,
`CONNECT_SOURCE_LEDGER.md`, `CONNECT_GROWTH_SPEC.md`,
`VISUAL_REFERENCE_CONTRACT.md`, `GIF_ANIMATION_AUDIT.md`,
`analysis/sprite_render_audit/SPRITE_RENDER_AUDIT.md`, and
`T_ECHO_LITE_HARDWARE_ACCEPTANCE.md`.

The generated current-state audit lives at
`analysis/completion_audit/completion_audit.md`. It reports remaining checklist
`[ ]` rows, task `TODO` rows, hardware/dual-device gates, source substitutes,
and hardware acceptance `TODO` rows from the current files.

The generated hardware-readiness audit lives at
`analysis/hardware_readiness/hardware_readiness.md`. It classifies each
hardware acceptance `TODO` row as source-side `READY`, `PARTIAL`, or `MISSING`
without pretending that a physical T-Echo-Lite observation has already passed.

The generated source-blocker matrix lives at
`analysis/source_blocker_matrix/source_blocker_matrix.md`. It turns
`CODED-SUBSTITUTE`, source-gated, and final-art rows into stable evidence
requirements without promoting substitute data to final Connection parity.

Status legend:

- `TODO`: no implementation yet.
- `DOING`: actively being worked.
- `CODED`: code/data exists; needs validation.
- `CODED-SUBSTITUTE`: explicit firmware table/flow exists, but it is not the
  official exact table yet.
- `HW`: requires one physical T-Echo-Lite.
- `DUAL`: requires two devices or a hardware-equivalent two-device setup.
- `BLOCKED-SOURCE`: exact official/source value missing.
- `DONE`: accepted.

Gate legend:

- `SPEC`: specification/source work.
- `DATA`: tables/manifests/resources.
- `CODE`: firmware/model/rendering.
- `UI`: player-visible flow.
- `SIM`: host-side validation.
- `HW`: real T-Echo-Lite observation.
- `DUAL`: two-device observation.

## P0 Interaction And Refresh

| ID | Task | Gate | Status |
| --- | --- | --- | --- |
| `P0.00` | Flash ordinary EchoPet baseline to one T-Echo-Lite. | HW | DONE |
| `P0.01` | Confirm notice/result hold survives synchronous partial refresh on real panel. | HW | HW |
| `P0.02` | Validate A/B/C hint readability on compact and large layouts. | HW | HW |
| `P0.03` | Validate sprite proof entry/exit by Home/Mail long press. | HW | HW: code-side proof count now uses the shared sprite frame count instead of stale 37-frame limit; TCA8418 bit7 is treated as release, debounce/config registers are explicit, and KeyShield HOLD now emits at threshold instead of waiting for release |
| `P0.04` | Measure Esc/Home/Email short press repeat behavior. | HW | HW |
| `P0.05` | Measure Esc/Home/Email long press threshold and release behavior. | HW | HW: code now suppresses release-after-hold duplicates; physical threshold/release timing still must be measured |
| `P0.06` | Stress menu cycling against synchronous partial-refresh pacing. | HW | HW |
| `P0.07` | Stress password/shop/game input against stale frames. | HW | HW |
| `P0.08` | Run long refresh whitening/ghosting observation. | HW | HW |

## P1 Care Action Flow Completion

| ID | Task | Gate | Status |
| --- | --- | --- | --- |
| `P1.01` | Confirm each fixed icon has entry/action/result/exit flow. | UI/HW | CODED/HW: the 10 fixed icons now render from official-reference-derived C++ bitmap rows at 12x12 compact and 30x30 large sizes; both sizes share the same official device/menu tile source per slot, and `fixed_menu_official_alignment_manifest.json` reports `PASS`. Entry/action/result/exit hardware feel remains HW. |
| `P1.02` | Make toilet mess timing source-backed. | SPEC/DATA | CODED-SUBSTITUTE: general poo stacking/sickness and Super Unchikun auto-clean behavior are sourced and coded; poop interval now uses a stage-shaped cross-version table from `CURL-ENTAMA-CARE` instead of one fixed period: Baby 6, Child 80, Teen 100, Adult/Parent 140, and Elder 160 pet-minutes. Exact Connection 2024 poop interval by life stage remains missing |
| `P1.03` | Make toilet cleanup use `toilet.mess/sweep/done/no_mess` phases. | CODE/UI | CODED |
| `P1.04` | Make medicine dosage/refusal/cure probability source-backed. | SPEC/DATA | CODED-SUBSTITUTE: official/manual sources cover skull/tooth medicine use, possible multi-dose sickness, no-sickness refusal, and sickness refusal of food/games/items; firmware now refuses meal/snack/game/item/catalog-use while sick/toothache with `SICK` notice, uses one-dose toothache cure and possible multi-dose sickness cure, persists a 60 pet-minute substitute sweet-snack short-period window, persists stage-local sickness count in save v10, escalates missed toothache-only care to sickness before care-mistake counting, can pass away on the fourth sickness in one growth stage, and centralizes low-hygiene/mess/high-weight/sweet-snack sickness risk in explicit `SicknessTriggerRule` rows; exact sickness probability, official toothache short-period length, and cure/timing probabilities remain substitute |
| `P1.05` | Make medicine use sick symbol/dose/wait/recover/refuse phases. | CODE/UI | CODED |
| `P1.06` | Split discipline into praise, time-out, invalid, false-call, missed-call. | CODE/UI | CODED |
| `P1.07` | Source attention timeout and care-mistake conditions. | SPEC/DATA | CODED-SPEC: source ledger records the 15-minute ordinary attention window, 60-minute lights-left-on window, and physical/mental care-mistake classes from current community guide cross-checks; ordinary miss, care-call cadence, empty-stat mistake cadence, sleep attention cadence, and lights-left-on offset are centralized in explicit `AttentionTimingRule` rows; successful FOOD submenu meal/snack care now clears the matching hunger/happiness attention call through the same model rule as direct care actions |
| `P1.08` | Implement stage-correct bedtime/lights call and 60-minute miss window. | CODE/UI | CODED |
| `P1.09` | Validate care actions on hardware. | HW | HW |

## P2 Core Pet Model

| ID | Task | Gate | Status |
| --- | --- | --- | --- |
| `P2.01` | Version save schema for growth tier/care mistake migration. | CODE | CODED: save schema is now v10 after adding full per-catalog quantity storage, parent/partner catalog identity slots, persisted sweet-snack short-period tracking, and persisted stage-local sickness count without increasing the packed save size; v6-sized saves are accepted by stored `save.size` and migrated into the catalog stock table instead of being silently reinterpreted, v7 family rows fall back to unknown partner identity, and v9 saves preserve sweet-snack timing while defaulting the new stage sickness count to zero |
| `P2.02` | Specify pause/time-change side effects. | SPEC | CODED-SPEC: no generic fake pause; the source-backed behavior is a clock-set state that pauses pet tick/growth until the time is confirmed |
| `P2.03` | Implement pause/time-change behavior if source-backed. | CODE/UI | CODED: runtime `CLOCK` edit mode stops `pet.tick()` until B confirms or C cancels; Home/B long-press enters it on T-Echo-Lite, and Mail/C long-press remains the sprite-proof debug entry |
| `P2.04` | Specify stat decay tables. | SPEC/DATA | CODED |
| `P2.05` | Implement stat decay tables. | CODE/SIM | CODED |
| `P2.06` | Specify death/pass-away/runaway thresholds. | SPEC | CODED-SUBSTITUTE: source ledger records no old-age death for Connection 2024 seniors, neglect/sickness as the death direction, and a Connection-era fourth-sickness-in-one-stage death rule; exact Grim Gotchi threshold remains missing |
| `P2.07` | Implement death/pass-away/runaway from sourced thresholds. | CODE/UI | CODED-SUBSTITUTE: elder pass-away no longer triggers from age/low friendship alone; it requires low friendship, accumulated care mistakes, and a currently low stat condition. Repeated sickness now uses a stage-local persisted counter and passes away at the fourth sickness in one growth stage |
| `P2.08` | Add host report for model table coverage. | SIM | CODED |

## P3 Food, Items, Shop, Password, Souvenir

| ID | Task | Gate | Status |
| --- | --- | --- | --- |
| `P3.01` | Source default meals/snacks and bought food rules. | SPEC/DATA | CODED: default Meal/Snack menu is now 4+4 permanent foods from the 2024 item-list source, while shop/password/catalog food consumes inventory stock |
| `P3.02` | Implement food refusal/fullness/toothache/weight effects from table. | CODE/UI | CODED |
| `P3.03` | Implement food/snack prop/bite/crumb/result animation. | CODE/UI | CODED |
| `P3.04` | Source shop stock/restock/price rules. | SPEC/DATA | CODED-SUBSTITUTE: 4-slot stock mapping, explicit `ShopRestockWindow` rows for sourced 12 AM / 3 PM / 7 PM restock periods, date-source-backed birthday/holiday substitutions including Dec 23/24 Whole Cake plus Turkey in slots 0/1, explicit `ShopSaleRule` 50% sale pricing, and explicit `ShopVendorWindow` rows for Ojitchi 11 AM / 5 PM single-item cart are coded. Sale date frequency, exact seasonal collision priority, and exact cart availability duration remain substitute because current sources only say random sale dates and 11 AM / 5 PM visits. |
| `P3.05` | Replace synthetic `MEAL001` catalog entries with explicit manifest. | DATA/CODE | CODED |
| `P3.06` | Implement shop booth/shopkeeper visual flow. | CODE/UI | CODED: shop page now uses explicit shop booth/counter, shopkeeper idle/surprise/happy, item-preview, and buy-result sprite frames for normal, secret-code, sale, cart, bought, no-points, full, and sold-out states |
| `P3.07` | Source 10-digit password item table. | SPEC/DATA | CODED: 32 2024 password values mapped to explicit catalog overrides |
| `P3.08` | Source shop-secret-code table and input gesture. | SPEC/DATA | CODED: official 1+3 A surprise entry, with a short pause allowed after the first shopkeeper A and three quick A taps opening the 8-symbol A/B/C page; 7 A/B/C secret codes, Hohotchi final-code replacement, and Honey/Love Potion next-link boost |
| `P3.09` | Implement duplicate password/code behavior. | CODE/UI | CODED |
| `P3.10` | Build souvenir list/collection table. | DATA/CODE | CODED: includes a no-mess/Super Unchikun projection triggered by 1000 donated GP in one generation, compact POINT-page King/donation feedback, and 64 souvenir visual-trait icons |
| `P3.11` | Hardware-validate shop/password/item flows. | HW | HW |

## P4 Games

| ID | Task | Gate | Status |
| --- | --- | --- | --- |
| `P4.01` | Source unlock stages for Get/Bump/Flag/Heading/Memory/Sprint/Hoops. | SPEC/DATA | CODED |
| `P4.02` | Source exact reward curves and stat effects. | SPEC/DATA | CODED-SPEC: standard-game top Gotchi Point prizes are sourced from `WIKI-2024`; known-length games now use source-length round/score caps for Get/Bump/Flag/Heading/Memory/Sprint/Hoops; perfect-care reversal tests against the active per-game score cap; Flag fake-signal `WAIT` rounds are modeled as short prompts where pressing misses and waiting scores; Memory uses the sourced 20-pattern schedule with two final 8-arrow A/B/C patterns from the 2024 source pass; Bump uses the V3-style 0-8 round weight-band prize chart; game refusal and post-win weight loss now use a stage base-weight table instead of hunger/energy gates; Bump still uses heavier-weight power bias and Sprint uses lighter-weight tap/chance bias. Official low-score scaling, exact Bump/Sprint probability curves, and exact non-point stat effects still need exact tables |
| `P4.03` | Implement table-driven game rewards. | CODE/SIM | CODED |
| `P4.04` | Implement Get note/bad item/catch/miss mechanics and animation. | CODE/UI | CODED: timed A/C catch-dodge window |
| `P4.05` | Implement Bump meter/opponent/push/fall mechanics and animation. | CODE/UI | CODED |
| `P4.06` | Implement Flag prompt/response mechanics and animation. | CODE/UI | CODED: black/white/both flag mapping uses A/C/A+C, single-flag side variation, button responses, timed fake-signal `WAIT` branch, short-flag visual, and missed-input/timeout outcomes are wired through model and renderer |
| `P4.07` | Implement Heading ball path/hit/miss mechanics and animation. | CODE/UI | CODED |
| `P4.08` | Implement Memory reveal/replay/wrong/GOOD mechanics and animation. | CODE/UI | CODED |
| `P4.09` | Implement Sprint lane/finish repeated-input mechanics and animation. | CODE/UI | CODED |
| `P4.10` | Implement Hoops hoop/ball arc/shoot/made/miss mechanics and animation. | CODE/UI | CODED: B shoot plus timeout miss |
| `P4.11` | Implement perfect-game care mistake reversal hooks. | CODE/SIM | CODED |
| `P4.12` | Hardware-validate all seven games. | HW | HW: active game targets are now 250-300 ms, ordinary action scenes are 360 ms, renderer progress uses model-exported round/score limits, and real e-paper pacing still must be observed |

## P5 Connection, Friends, Love, LoRa

| ID | Task | Gate | Status |
| --- | --- | --- | --- |
| `P5.01` | Complete IR-visible Visit spec. | SPEC | CODED |
| `P5.02` | Complete IR-visible Present spec. | SPEC | CODED: Present contract now specifies exact catalog food/item gifts plus a points row, legacy coarse food/item receive compatibility, send-time sender debit, receiver-full refusal, and souvenir/no-send boundaries |
| `P5.03` | Complete IR-visible Game spec. | SPEC | CODED: LINK contract now separates seven Connection game ids from local mini-game ids and records item-backed requirements for Ball/RC Car/Rope/Building Block/Balloon/Trumpet |
| `P5.04` | Complete IR-visible Love spec. | SPEC | CODED |
| `P5.05` | Implement Visit visible flow and model effects. | CODE/UI | CODED |
| `P5.06` | Implement Present visible flow and inventory edge cases. | CODE/UI | CODED: PRESENT browses all 160 catalog rows plus points, sends `GiftKind::kCatalogItem` with exact `giftId`, debits selected `catalogStock` or reusable owned bit only when the packet is prepared, rejects souvenir rows/missing stock/no points, and receiver awards or refuses exact catalog gifts using catalog food/item capacity checks |
| `P5.07` | Implement Connection Game visible flow and rewards. | CODE/UI | CODED: CONNECT->GAME exposes G.POINT plus six item-backed Connection games, `FriendPacket.gameKind` carries the Connection game id, sender and receiver both require the corresponding catalog item before mutation, standby replies echo the sender's game id, and result rewards are model-only table substitutes |
| `P5.08` | Implement Love reject/partner/baby/parent departure flow. | CODE/UI | CODED: ineligible Love now rejects; baby is gated to eligible Love flow only; LINK standby now emits a same-kind reply on accepted Love packets so the sender can resolve partner/baby after receiving the return packet; `FriendPacket` v4 carries peer adult tier/gender/catalog-id so both parent routes, opposite-gender elder Oyajitchi lineage, visible friend identity, and known LOVE partner identity in family history feed the next generation flow |
| `P5.09` | Preserve carrier-independent `FriendPacket` boundary. | CODE | CODED: v4 packet validation covers kind/stage/character/catalog-id/relation/growth-tier/adult-tier before model effects, and Serial/LoRa both carry the packed payload |
| `P5.10` | Validate two-device Visit/Present/Game/Love. | DUAL | DUAL: source now rejects mismatched LINK kinds, standby auto-replies same-kind Visit/Present/Game/Love, and GAME standby replies echo the incoming Connection game id; hardware still must prove both screens advance correctly |
| `P5.11` | Validate LoRa equivalence to IR-visible contract. | DUAL | DUAL: Serial/LoRa share `FriendPacket` and `receiveFriendPacket()` behavior; two-device LoRa proof still open |

## P6 Health Meter And Setup

| ID | Task | Gate | Status |
| --- | --- | --- | --- |
| `P6.01` | Source setup field order and editing behavior. | SPEC | CODED: official manual page order recorded |
| `P6.02` | Implement exact setup order/validation if different. | CODE/UI | CODED: gender is reveal-only before pet naming |
| `P6.03` | Source health meter page order. | SPEC | CODED: official manual field order recorded |
| `P6.04` | Implement exact health meter order and icons. | CODE/UI | CODED |
| `P6.05` | Hardware-validate setup and health pages. | HW | HW |

## P7/P11 Visual And Resource Production

| ID | Task | Gate | Status |
| --- | --- | --- | --- |
| `P7.01` | Build 50+ character manifest with source IDs and frame families. | DATA | CODED: 50 character rows have source page/slot IDs, generated 16x16 portrait rows, generated 24x24 idle rows, and a row-local official/16x16/24x24 status proof plus JSON under `analysis/official_reference/generated/`; `character_source_alignment_status.json` has 50 `official-derived` rows with zero review flags. `analysis/resource_replacement_manifest/character_replacement_manifest.json` now exposes the stable replacement contract for all 50 rows: catalog row id, source page/slot, stage/route/tier/gender metadata, 16x16 portrait bitmap symbol, and 24x24 idle bitmap symbol. Hardware readability remains HW. |
| `P7.02` | Build 150+ item manifest with source IDs and icon/use animation. | DATA | CODED-PARTIAL: 160 catalog rows now expose kind/behavior/price/source/use-scene plus a stable low-resource `visualTraits` byte consumed by catalog previews and use scenes; 160 explicit C++ preview bitmap rows exist; 12 unambiguous non-souvenir item rows have official-source item-icon overrides (PENCIL 48/112, CAP 58/122, SHOVEL 54/118, BALL 60, BALLOON 56/63/120, TRUMPET 84/141); the catalog item official-reference proof now includes a 152-row semantic-status sheet; 64 souvenir rows have per-memory C++ source bitmaps plus an official semantic proof manifest, with TRUMPET and BALL PLAY generated from unambiguous official item-icon crops; `analysis/resource_replacement_manifest/` now exposes JSON/CSV row-local replacement contracts for all 160 catalog rows and 64 souvenir rows, leaving 148 catalog rows and 62 souvenir rows needing accepted final art |
| `P7.03` | Refine default Mametchi-like proof character. | DATA/CODE/UI | CODED |
| `P7.04` | Add missing hatch/crack/egg frames. | DATA/CODE/UI | CODED: egg idle face, low-resource crack frames, and hatch frame are composited inside the shell; exact official-look final art and hardware proof remain open |
| `P7.05` | Add local prop resource layer API for food/sweep/medicine/game props. | CODE | CODED |
| `P7.05a` | Guard runtime resources as C/C++ bitmap arrays, not PNG/JPG/GIF assets. | SIM/CODE | CODED: `RUNTIME_RESOURCE_CONTRACT.md` defines the boundary, `EchoPetSprites.cpp`, `EchoPetResources64.cpp`, and `EchoPetResources128.cpp` remain the firmware resource truth, and `analysis/runtime_resource_audit/generate_runtime_resource_audit.py` rejects raster files or file-load patterns from runtime paths while allowing analysis-only contact sheets |
| `P7.06` | Add shop booth, shopkeeper, preview, and result visual frames. | DATA/CODE/UI | CODED: `kShopBooth`, `kShopkeeperIdle`, `kShopkeeperSurprise`, `kShopkeeperHappy`, `kShopItemPreview`, `kShopBuyOk`, `kShopBuyNoMoney`, `kShopBuyFull`, and `kShopSoldOut` are renderer-mapped and sprite-proof labelled |
| `P7.07` | Add family/love/parent/baby visual frames. | DATA/CODE/UI | CODED: love/parent/baby sprite frames exist, FRIEND/FAMILY portraits now use character-catalog `frameFamily`/`visualTraits` instead of seed-generated faces, and FAMILY history renders a compact album card from compatibility `FamilyRecord` rows plus sourced `FamilyAncestryRecord` parent/partner/baby/tier rows with catalog-derived parent/baby avatars, known LOVE partner avatars, and an unknown-partner silhouette for legacy/Matchmaker rows |
| `P7.08` | Add character fallback policy, active frame-family routing, and manifest coverage report. | SIM | CODED: 50 catalog rows now route idle visuals through 50 official-derived C++ idle rows with 9 fallback frame families and a row-local official/source status report; source-side art acceptance is closed by zero review flags. Exact same-pool growth-route probability and hardware readability remain open. |
| `P7.09` | Sprite-proof every frame family on hardware. | HW | HW: host-side sprite render audit now parses the firmware source and emits contact sheets for all 101 `SpriteFrame` rows with zero missing mappings and zero nominal-frame overflows; proof page exposes expanded low-resource hatch, care, lights selector/on/off/wake/invalid, shop booth/counter, shopkeeper, shop preview/results, love/family, and game frame symbols; real T-Echo-Lite photos/video remain required |

## P8 Animation Contracts

| ID | Task | Gate | Status |
| --- | --- | --- | --- |
| `P8.01` | GIF audit and contact sheets. | SPEC | DONE |
| `P8.02` | Map each GIF phase to named renderer state. | DATA/CODE | CODED |
| `P8.03` | Implement GIF-derived care scenes. | CODE/UI | CODED |
| `P8.03a` | Route discipline TIME OUT/PRAISE/invalid/attention/missed visible feedback. | CODE/UI | CODED-SUBSTITUTE: compact scene frames are active and `kDisciplineSceneAnimationMs` now keeps the page on explicit scene pacing instead of home idle pacing; official/local GIF timing metadata is included in parity coverage, while hardware readability remains open |
| `P8.03b` | Route medicine sick/tooth/dose/wait/recover/refuse visible feedback. | CODE/UI | CODED-SUBSTITUTE: compact bottle, badge, dose trail, recovery, and refusal routing are active and `kMedicineSceneAnimationMs` now keeps the page on explicit scene pacing; official/local GIF timing metadata is included in parity coverage, while cure odds and hardware readability remain open |
| `P8.03c` | Route food/snack select/bite/crumb/done/refuse visible feedback. | CODE/UI | CODED-SUBSTITUTE: compact selected food, plate, bite travel, crumbs, done/check/heart, and refusal cross routing are active; Meal/Snack pages now use `kFoodSceneAnimationMs` instead of falling through to 2200 ms home idle pacing; official/local GIF timing metadata is included in parity coverage, while bite count and hardware readability remain open |
| `P8.03d` | Route toilet visible-mess/sweep/progressive-removal/done/no-mess feedback. | CODE/UI | CODED-SUBSTITUTE: compact visible messes, sweep tool, progressive removal, clean slot, done heart/sparkle, and no-mess question branch are active and `kToiletSceneAnimationMs` now keeps the page on explicit scene pacing; official/local GIF timing metadata is included in parity coverage, while exact mess interval, sickness effects, and hardware readability remain open |
| `P8.03e` | Route lights ON/OFF selector, low-ink dark-room, sleep/Z, wake, and invalid feedback. | CODE/UI | CODED-SUBSTITUTE: compact selector, hatch-pattern dark-room, moon/star marks, sleep/Z, wake, and invalid routing are active and `kLightsSceneAnimationMs` now keeps the page on explicit slow scene pacing; official/local GIF timing metadata is included in parity coverage, while exact sleep/nap behavior and hardware whitening/ghosting remain open |
| `P8.04` | Implement GIF-derived game scenes. | CODE/UI | CODED |
| `P8.05` | Hardware-validate GIF-derived pacing. | HW | HW |
| `P8.06` | Official how-to metadata audit without storing official pixels. | SPEC/SIM | DONE |
| `P8.07` | Map official how-to assets to named frame-family contract hints. | SPEC/DATA | CODED |
| `P8.08` | Hardware-validate official-howto-derived pacing and readability. | HW | HW |

## P9 Exact Data Tables

| ID | Task | Gate | Status |
| --- | --- | --- | --- |
| `P9.01` | Complete source ledger for growth/shop/password/game/death. | SPEC | CODED-SUBSTITUTE: ledger now records official/community sources for growth, shop, password, games, death, King donation, and official visual-reference boundaries; exact death thresholds, poop timing, sale frequency, probability curves, and rights-cleared final pixel art remain open |
| `P9.02` | Implement table-driven growth model. | DATA/CODE/SIM | CODED: parent-pair child tier selection, child-tier teen transition, adult-tier transition from the transcribed `WIKI-CHAR-2024` Serious/Normal/Naughty/Frail/Stubborn rows, Serious-adult gate requiring both parents unhealthy, special Oyajitchi lineage, generation/tier/gender source-pool character selection, `GrowthScheduleRule` stage thresholds, `FamilyTimingRule` parent/manual-Matchmaker timing, `MatchmakerClockRule` scheduled clock rows, four-adult-day matchmaker eligibility, and FAQ-aligned about-four-adult-day senior transition are explicit code/data table paths; final per-character art, exact same-pool route probabilities, exact matchmaker/senior same-day overlap, edge-case transition ordering, and hardware proof remain open |
| `P9.03` | Implement death/pass-away/runaway table. | DATA/CODE/SIM | CODED-SUBSTITUTE: severe neglect, sick+empty-state pass-away, stage-local fourth-sickness pass-away, and elder/Grim substitute rules exist; elder/Grim is constrained to neglect rather than old age alone, but exact 2024 source thresholds and wording remain open |
| `P9.04` | Implement stat decay/event table. | DATA/CODE/SIM | CODED-SUBSTITUTE: hunger, happiness, and poop now use `kStageCareTimingRules` with Baby 3/5/6, Child 40/45/80, Teen 50/55/100, Adult/Parent 70/75/140, and Elder 80/110/160 pet-minute Hungry/Happy/poop periods from low-confidence cross-version source `CURL-ENTAMA-CARE`; Hungry/Happy also use the sourced Connection-era two-hidden-heart reserve through `kHiddenHeartMeterMax` while the health display remains capped to four visible hearts; energy, hygiene, sickness trigger rows, attention timing rows, stage sleep-window rows, lifecycle timing rows, fifteen-consecutive-snack toothache trigger, persisted 60 pet-minute substitute toothache short-period window, toothache-to-sickness missed-call escalation, lights-left-on timing, clock-edit pause, real month-length date validation, and `CalendarEventRule` rows for birthday/New Year/winter/Easter/back-to-school/Halloween/Thanksgiving/Christmas/Santaclautchi notices are explicit constants/tables and are reported by growth coverage. Exact official Connection 2024 decay windows, official sleep windows, official lifecycle timing, official toothache short-period length, calendar cutscene art/wording, and other official probability/timing values remain source-gated |
| `P9.05` | Implement shop/catalog table. | DATA/CODE/SIM | CODED-SUBSTITUTE: 4-slot shop stock, explicit restock/sale/vendor window tables, birthday/holiday slot substitutions including Dec 23/24 Whole Cake plus Turkey, 50% sale pricing, Ojitchi cart vendor pool, password/secret overrides, source-named/priced food catalog slots 0-47 and 85-87, source-named/priced item catalog slots 62-95, source-named extension/duplicate rows for slots 96-143, corrected source labels/prices, parseable `CharacterFoodTasteRule` liked/disliked catalog-food rows with liked food filling Happy and disliked food clearing Happy, item-list liked-by-all Yogurt/Steak rules, child+/teen+/adult-only item-use gates, reusable/single-use corrections, fixed Ticket 1-5 souvenir awards, Music Disc/Boom Box dependency and break chance, Make-Up/Mirror dependency, Shaver/Oyajitchi restriction, Tama Drink sickness guard, gendered Action Figure/Doll enjoyment, source-shaped Plant/Shovel/Chest/Lamp/Fishing Pole random reward/penalty/break behavior, explicit `CatalogRandomRule` rows for substitute branch counts/point rewards/Lamp break chance, Chest/Lamp exclusive item awards, Fishing Pole trash-retain/reward-destroy behavior, full per-catalog quantity storage with v6-sized save migration, catalog-owned cleanup when consumable stock reaches zero, catalog food/item total storage caps, Hair Gel cosmetic consumption, ITEM-page 160-row cursor routing, icon routing, use-scene routing, 160 distinct catalog `visualTraits` overlays, 160 explicit catalog C++ preview rows, 12 official-source item-icon overrides, a 152-row catalog item semantic-status proof, 64 per-memory souvenir C++ rows, an official souvenir semantic proof manifest, and two official-source souvenir overrides are coded; ambiguous food-taste source rows, sale frequency, exact cart availability duration, exact seasonal collision priority, exact random probability tables, exact reward amounts, exact item animation timing, most final per-item art, and final souvenir art remain open |
| `P9.06` | Implement password/secret-code table. | DATA/CODE/SIM | CODED-SUBSTITUTE: 32 password values plus 7 shop secret codes, Hohotchi replacement, Honey/Love Potion next-link boost, repeatable Clock/RC Car/Hohotchi, and temporary Nyatchi/Hohotchi costume projection coded; exact per-item art open |
| `P9.07` | Implement game reward table. | DATA/CODE/SIM | CODED-SUBSTITUTE: sourced top-prize table is coded; known-length games use source-length round/score caps for Get 100, Bump 8, Flag 9, Heading 20, Memory max-pattern 8 with 20 scored pattern rounds, Sprint 8, and Hoops 30; perfect-care reversal tests against the active per-game score cap; Memory now uses the sourced 20-pattern A/B/C cadence plus a compact display window instead of single-key rounds; Bump uses an explicit V3-style 0-8 round by four-weight-band prize chart with 2400/1800/1200/600 perfect prizes and no longer boosts score directly for heavier pets; wallet gains clamp at 9,999 Gotchi Points; game entry refuses at stage base weight through `StageBaseWeightRule` rows and game wins reduce weight only down to that base; Sprint weight direction is modeled in mechanics; non-Bump low-score prizes map long scores through an explicit five-tier curve, and `GamePacingRule` rows expose each game's source goal, firmware round count, score cap, timeout, and input flags. Exact official 2024 low-score table and Bump/Sprint probability curves remain substitute |
| `P9.08` | Build parity data coverage report. | SIM | CODED: growth coverage, resource/visual contract coverage, runtime-resource boundary coverage, and firmware sprite render coverage reports are generated under `analysis/growth_coverage/`, `analysis/parity_coverage/`, `analysis/runtime_resource_audit/`, and `analysis/sprite_render_audit/` |

## P10/P12 Acceptance

| ID | Task | Gate | Status |
| --- | --- | --- | --- |
| `A.01` | Build ordinary firmware package after each completed code tranche. | SIM | CODED |
| `A.02` | Build LoRa firmware package after link tranche. | SIM | CODED |
| `A.03` | Flash ordinary build and log result. | HW | DONE: ordinary `20260617_161310` flashed on `COM47` and logged; later packages are build-only and are not flashed by user request |
| `A.04` | Flash LoRa build and log result. | HW | DONE: current `EchoPet_LoRa` build flashed on `COM47` via PlatformIO `nrfutil` serial DFU on 2026-06-22 15:53 +08:00 and logged; device returned as application serial `COM46`; two-device LoRa parity remains `DUAL` |
| `A.05` | Hardware smoke-test all fixed icons. | HW | HW |
| `A.06` | Hardware smoke-test all games. | HW | HW |
| `A.07` | Hardware smoke-test all sprite proof families. | HW | HW |
| `A.08` | Two-device smoke-test Visit/Present/Game/Love/LoRa. | DUAL | DUAL |

## Immediate Execution Order

1. `FLASH-CONSENT`: user explicitly re-enabled one LoRa flash on 2026-06-22,
   and `A.04` is now logged as done. Future flash/upload tasks still require a
   fresh explicit user request; otherwise continue source, code, resource,
   simulation, build, and package work only.
2. `P0`: after flashing is re-enabled, run the button and refresh acceptance
   matrix on one T-Echo-Lite:
   short/long/release timing, synchronous partial-refresh pacing, idle
   whitening, menu cycling, notices, and high-input screens.
3. `P1/P8`: hardware-validate all fixed-icon care/action scenes against their
   entry/action/result/exit contract: food, toilet, medicine, discipline,
   lights, shop, password, item, attention, and health.
4. `P4/P8`: hardware-validate all seven games at e-paper speed. Record whether
   Get, Bump, Flag, Heading, Memory, Sprint, and Hoops are readable, responsive,
   and visibly show miss/win/reward.
5. `P7/P11`: sprite-proof every frame family on both layouts. Use
   `analysis/sprite_render_audit/` as the source-side baseline first, then
   record hardware defects by named frame, anchor, scene, and screen class
   before accepting visual parity.
6. `P9.01`: complete the source ledger rows for exact growth, death/runaway,
   stat decay, food/shop, password/secret-code, and game reward values.
7. `P9.05/P9.06/P9.07`: replace the remaining CODED-SUBSTITUTE tables with
   sourced exact data where sources can be found; keep unsourced values marked
   substitute rather than silently promoting them.
8. `P11.03/P11.05`: replace the compact `visualTraits` substitute layers and
   remaining source-backed substitute rows with rights-cleared final resources
   for catalog item/food/password/souvenir rows that still lack official-source
   or user-accepted final art. Character rows are source-side accepted by the
   current 50-row generated C++ bitmap set, but future user-drawn character
   packs must preserve the same row IDs and frame definitions. Official art may
   be used only as a reference contract, not packaged; current official-source
   C++ overrides must stay explicitly documented by row and source crop.
9. `P5/P10`: validate Visit, Present, Game, and Love on two devices or a
   hardware-equivalent two-device setup, then prove the LoRa build preserves the
   same user-visible contract as the IR-visible spec.
10. `P12`: re-run the full release acceptance matrix after the exact-data and
    resource passes, then build a final ordinary/LoRa package linked to the
    completed hardware log.
