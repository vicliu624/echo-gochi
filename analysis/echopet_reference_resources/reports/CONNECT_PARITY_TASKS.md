# EchoPet Connect Parity Task Queue

This queue is derived from `CONNECT_ALIGNMENT_CHECKLIST.md`. A checklist row is
not done because code exists; it is done only when the player-visible behavior
works on the T-Echo-Lite screen and the result can be validated.

Execution order and acceptance gates are defined in `CONNECT_BURNDOWN_PLAN.md`.
Hardware-only closure evidence is recorded in
`T_ECHO_LITE_HARDWARE_ACCEPTANCE.md`.
Detailed burn-down rows and dependency order are maintained in
`CONNECT_REMAINING_BACKLOG.md`.

Status:

- `TODO`: not implemented or known wrong
- `DOING`: currently being implemented
- `CODED`: code path exists, still needs hardware/player validation
- `CODED-SUBSTITUTE`: explicit code/table exists, but official exact source
  data still needs replacement
- `HW`: requires real T-Echo-Lite observation before closing
- `DONE`: accepted for the current target

## P0 Interaction And Refresh

- `P0-00` `DONE`: Flash the earlier ordinary `EchoPet` baseline build to
  T-Echo-Lite.
  On 2026-06-17 the device was touched from `COM46` into bootloader `COM47`,
  then PlatformIO `nrfutil` serial DFU reported `Device programmed.`
- `P0-00B` `DONE`: Flash the current ordinary `20260617_161310` package.
  The bootloader appeared as `USB VID_239A PID_00DA (COM47)`, and
  PlatformIO `nrfutil` reported `Device programmed.` LoRa remains unflashed
  until the dual-device validation pass.
- `P0-01` `CODED`: Notice/result feedback must survive e-paper refresh latency.
  Notices are now held for at least one visible interval after the synchronous
  refresh path renders the submitted frame.
- `P0-02` `CODED`: Every non-game screen must expose A/B/C semantics. Large
  layout uses the bottom info line; compact layout uses a 64x64 bottom hint.
- `P0-03` `CODED`: Home/B long-press now enters the runtime `CLOCK` edit
  pause screen. Sprite proof remains reachable from Mail/C long-press. The
  KeyShield input layer decodes TCA8418 bit7 as release, starts hold timing on
  physical press, enables debounce/config registers explicitly, emits HOLD once
  the threshold is reached, and suppresses release-after-hold so a hold cannot
  also fire a short press.
- `P0-04` `HW`: Validate Esc/Home/Mail short press, long press, debounce,
  release timing, and accidental repeats on real hardware.
- `P0-05` `HW`: Validate synchronous partial-refresh panel pacing under menu
  cycling, games, password input, shop/item browsing, setup, friend deletion,
  and link standby.
- `P0-06` `HW`: Validate whitening, ghosting, and missed/stale frames after long
  repeated operation.

## P1 Fixed Icons And Care Actions

- `P1-01` `CODED`: Each of the ten fixed icons needs a complete entry/action/result
  flow, not just a page.
- `P1-02` `CODED`: Toilet needs exact mess timing, sickness/care-mistake effects,
  before/action/after visual phases, and hardware-visible cleanup.
- `P1-03` `CODED-SUBSTITUTE`: Medicine needs illness vs toothache
  dosage/refusal/cure feedback and health-meter skull/tooth validation. Sick
  or toothache state now refuses meal, snack, game, item, and catalog-item use
  with a distinct `SICK` notice; sweet-snack toothache now uses a persisted
  60 pet-minute substitute "short period" window, and a missed toothache-only
  call escalates to sickness before any care mistake is recorded. Save v10
  persists the stage-local sickness count, and the fourth sickness in one
  growth stage can pass away as a Connection-era substitute rule. Low hygiene,
  mess count, high weight, and sweet-snack sickness risks are now centralized in
  explicit `SicknessTriggerRule` rows. Exact sickness probability, official
  toothache short-period length, and cure timing remain source substitutes.
- `P1-04` `CODED`: Discipline needs visibly distinct successful timeout,
  successful praise, invalid discipline, missed attention, and false-call cases.
- `P1-05` `CODED`: Lights/sleep needs stage-based bedtime/wake rules, good/bad
  lights-off response, wake-up, nap, and stable dark-room rendering. The
  lights-off scene now avoids a full-width black fill and uses a low-ink
  border/hatch dark-room pattern, moon/star marks, and a separate ON/OFF
  selector; hardware whitening/ghosting validation remains open.
- `P1-06` `CODED`: Attention icon needs exact highlight, timeout, and care-mistake
  behavior. Successful FOOD submenu meal/snack care now clears the matching
  attention call through the same model rule as direct care actions.

## P2 Core Pet Model

- `P2-01` `CODED`: Save schema audit against all player-visible Connection fields.
  Schema is v10 after adding full per-catalog quantity storage,
  parent/partner catalog identity slots, persisted sweet-snack short-period
  tracking, and persisted stage-local sickness count in the previous reserved
  bytes; v6-sized saves are accepted by stored `save.size` and migrated into
  the catalog stock table, v7 family rows fall back to unknown partner
  identity, and v9 saves default the new sickness count to zero.
- `P2-02` `CODED-SUBSTITUTE`: Death/runaway/memory/rebirth flow must be
  Connect-complete. Elder pass-away no longer happens from age/low friendship
  alone; it now requires neglect-like low state plus accumulated care mistakes.
  Exact Grim Gotchi thresholds remain a source gap.
- `P2-03` `CODED`: Pause/time-change side effects are modeled as runtime
  `CLOCK` edit mode suppressing `pet.tick()` until the time is confirmed or
  canceled. Original-device button chord still needs acceptance.
- `P2-04` `CODED`: Stat bounds, decay rates, timers, and event probabilities need
  parity tables and tests.
- `P2-05` `CODED`: Growth effects from training, discipline, weight, happiness,
  hunger, sleep, and missed calls need parity tests.

## P3 Food, Items, Shop, Password, Souvenir

- `P3-01` `CODED`: Food item selection, refusal, fullness, weight/happiness change,
  toothache risk, and animation feedback need parity pass, including the staged
  prop/bite/crumb/result flow from `food and snacks.gif`. The default food menu
  is now the sourced 4 meals plus 4 snacks and is permanent; shop/password food
  is inventory-backed and one-use. The active Food/Snack scene now draws the
  selected prop, plate, bite travel, crumbs, done/check/heart, and refusal cross
  as separate compact objects.
- `P3-02` `CODED`: Shop food/item stock and bought-item usage must match the
  expected shop flow, including the booth/shopkeeper feedback from `shop.gif`.
  The hidden-code entry gesture is now modeled closer to Connection behavior:
  in Shop, the official 1+3 A gesture shows the surprised/`SECRET?`
  shopkeeper state, then enter an 8-symbol A/B/C secret-code page. Normal Shop browsing now uses four
  stock slots mapped to catalog items instead of cycling all 160 catalog rows;
  stock changes across the sourced 12 AM / 3 PM / 7 PM restock periods. Shop
  now substitutes birthday/holiday foods into the first normal stock slot,
  sale pricing applies 50% off on deterministic sale dates, and Ojitchi's cart
  appears as a one-item offer at the 11 AM / 5 PM visit windows. Exact sale
  frequency and cart availability duration remain source gaps.
- `P3-03` `CODED`: Food inventory limits, item inventory limits, and money/points
  effects need validation.
- `P3-04` `CODED-SUBSTITUTE`: Exact Connection item/souvenir catalog data and
  per-item art remain incomplete. The 32 listed 2024 10-digit password values,
  seven shop
  A/B/C secret-code values, 48 source-backed food rows, seasonal/birthday food
  rows, and item rows through slot 95 are coded with explicit catalog
  label/price/flag/icon/use-scene overrides. Final-code Hohotchi replacement
  and Honey/Love Potion's next-link relationship boost are coded. The King
  donation 1000GP threshold now enables a one-generation Super
  Unchikun/no-mess auto-clean projection, and the POINT page now has a compact
  King/donation scene with donation progress and result feedback. Item-specific
  final art and the exact King cutscene remain open as source/art blockers, not
  missing firmware structure.

## P4 Games

- `P4-01` `CODED`: Games have direct A/B/C hints and e-paper-compatible Hoops
  input. Manual-backed input fixes are included: Get uses A/C movement with
  timed catch/dodge resolution instead of B confirmation, Bump and Sprint accept
  any button, Flag maps black/white/both to A/C/A+C with a timed fake-signal
  `WAIT` branch, and Hoops has B shoot plus timeout miss.
- `P4-02` `CODED-SUBSTITUTE`: Get, Bump, Flag, Heading, Memory, Sprint, and Hoops need
  faithful Connection/V3 mechanics, failure conditions, reward curves, unlock
  timing, age restrictions, weight changes, and visible win/lose pacing. The
  local GIFs now define required scene contracts for props, meters, prompts,
  character poses, and result animation. Bump now biases power toward heavier
  pets while using a V3-style 0-8 round by four-weight-band prize chart, so
  heavier weight no longer directly boosts score; Sprint now requires fewer
  taps and gives better success chances for lighter pets. Wallet gains clamp
  at 9,999 Gotchi Points. Non-Bump low-score Gotchi Point rewards now use an
  explicit five-score curve instead of hidden linear proportional scaling. The
  current e-paper compression is no longer a hidden `5` constant:
  `kGamePacingRules` now reports each game's known source goal, firmware round
  count, score cap, timeout window, and compression/input flags through
  `analysis/growth_coverage/`, and the renderer reads the active round/score
  limits from the model snapshot instead of hard-coding `/5` progress text.
  Memory uses the sourced 20-pattern cadence that ends with two eight-arrow
  A/B/C patterns.
  Exact official 2024 probability tables, non-Bump low-score reward curves, and
  final original-length mechanics remain source/hardware gaps.
- `P4-03` `HW`: Validate game input responsiveness and result feedback on the
  physical e-paper panel. Source target intervals are now 250-300 ms for active
  games and 360 ms for ordinary action scenes; final submissions use the
  synchronous 250 ms partial-refresh submit guard.

## P5 Connection, Friends, Love, LoRa

- `P5-01` `CODED`: IR-visible Connection behavior must be specified as the source
  contract before LoRa substitution is considered complete.
- `P5-02` `CODED`: Visit, Present, Game, and Love need complete standby, send,
  receive, timeout, cancel, result, friend-update, partner, baby, and next-gen
  visible flows. Love now has an explicit ineligible rejection state, and baby
  creation is gated to eligible Love packets instead of any Partner-level
  Visit/Present/Game packet.
- `P5-03` `CODED`: Friend list capacity, ordering, labels, history, and delete
  prompts need parity pass.
- `P5-04` `CODED`: LoRa uses the same packet/user-flow surface as the serial
  development carrier, but this still needs two-device validation.

## P6 Health Meter And Setup

- `P6-01` `CODED`: Health meter page order and fields need exact parity: hunger,
  happiness, training, age, weight, name, gender, generation, points, and health
  indicators.
- `P6-02` `CODED`: Health page cycling with A/B/C needs parity pass.
- `P6-03` `CODED`: Empty-save setup now follows the official manual order:
  date/time, birthday, user name, boy/girl reveal, pet name, and hatch
  confirmation. Digit editing, validation, sound feedback, and hatch timing
  still need hardware/player parity validation.

## P7 Visual And Asset Contract

- `P7-01` `CODED`: Character art must follow Connection scale, silhouette, action
  poses, and motion timing closely enough for hardware validation. Local
  `character.png` and `family.gif` are now treated as concept inputs for route
  silhouette coverage only, not runtime firmware assets.
- `P7-02` `CODED`: Official visual references must be audited into named frame
  families for egg, baby, child, teen, adult, care, food, toilet, medicine,
  lights, discipline, games, connection, item, happy, sad, sick, sleep, death,
  Love, parent, baby, shop/password/souvenir, and pass-away.
- `P7-03` `CODED`: Sprite metadata must define frame name, anchor, layer, mirror
  rule, and duration so future hand-drawn pets can replace the default set.
- `P7-04` `CODED`: Local feature animation such as eyes, mouth, feet, props, and
  expression overlays now covers egg crack/hatch, food crumbs/refuse/done,
  toilet mess/sweep/done/no-mess, medicine skull/tooth/dose/recover/refuse,
  lights selector/on/off/wake/invalid, shop booth/counter, shopkeeper
  idle/surprise/happy, shop preview/results, love/baby/parent, and game
  note/meter/flag/ball/memory/runner/hoop frames.
  It still needs final official-reference art and hardware sprite-proof photos.
- `P7-05` `HW`: Sprite proof photos must be recorded against frame names before
  scene composition can be accepted.

## P8 Local GIF Animation Contracts

- `P8-01` `DONE`: Audit local GIF concept files from
  `C:\Users\vicliu\Projects\pet` into `GIF_ANIMATION_AUDIT.md`, with generated
  contact sheets and frame metrics under `analysis/gif_audit/`.
- `P8-02` `CODED`: Implement the `food and snacks.gif` scene contract: selected
  food/snack prop, bite/chew mouth frames, crumbs/disappear phase, refusal, and
  done/result feedback. The active scene now routes these through separate
  plate/stage-badge objects plus the named food sprite frames; exact bite count,
  timing, and hardware readability remain open.
- `P8-03` `CODED`: Implement the `Clean up the poop.gif` scene contract: visible
  mess, pet reaction, multi-frame sweep/flush prop, progressive removal, after
  recovery, and no-mess branch. The active Toilet scene now draws visible
  messes, sweep tool, progressive mess removal, clean slot, done heart/sparkle,
  and no-mess question branch; exact mess timing and hardware readability remain
  open.
- `P8-04` `CODED`: Implement the `Cure when ill.gif` scene contract: skull/tooth
  symbol, medicine dose prop, wait/reaction, cure/failure/refusal branches, and
  recovery pose. The active Medicine scene now draws a bottle, sick/tooth/cured
  or refusal badge, dose trail, recovery frame, and refusal frame; exact timing
  and hardware readability remain open.
- `P8-05` `CODED`: Implement the `Discipline when needed.gif` scene contract:
  separate TIME OUT, PRAISE, invalid, attention-call, and missed-call visual
  phases tied to training/care-mistake updates. The active scene now routes
  attention-call and missed/invalid feedback through compact scene frames.
- `P8-06` `CODED`: Implement the `Lights off at night!.gif` scene contract:
  proofable ON/OFF selector, sleep/Z frames, dark-room state, wake/invalid
  feedback frames, and hardware-stable dark-room refresh. The active scene now
  routes ON/OFF selector, low-ink dark-room pattern, sleep/Z, wake, and invalid
  branches; final stability still needs T-Echo-Lite observation.
- `P8-07` `CODED`: Implement the `shop.gif` scene contract: proofable
  booth/counter frame, shopkeeper idle/surprise/happy frames, item preview,
  buy success, no-money, full-stock, and sold-out feedback frames.
- `P8-08` `CODED`: Translate `family.gif` and `character.png` concept inputs
  into a source-backed route/family visual coverage target: egg, baby, child,
  teen, adult, parent, baby/next-gen, and family-history silhouettes must be
  named and replaceable. The FAMILY page
  now draws empty/populated compatibility `FamilyRecord` rows plus sourced
  `FamilyAncestryRecord` parent/partner/baby/tier rows as compact album cards
  instead of text-only records.
- `P8-09` `CODED`: Implement GIF-derived game contracts:
  Get falling notes/bad items/catch/miss; Bump force meter/opponent/push/fall;
  Flag black/white/A+C-both prompt, side variation, fake WAIT short-flag, and
  response; Heading ball path/hit/miss; Memory reveal/replay/wrong/GOOD; Sprint
  race lane/finish; Hoops hoop/ball arc/shoot.
- `P8-10` `CODED`: Extend the renderer/resource authoring model so props, eyes,
  mouths, hands, feet, flags, balls, notes, sweep tools, medicine symbols,
  shop booth/counter, shopkeeper expressions, shop preview/result cards, and
  meters can be reused as local animated parts.
- `P8-11` `HW`: Validate GIF-derived high-motion scenes on T-Echo-Lite for
  visible pacing, synchronous partial-refresh stability, ghosting, whitening,
  and button response.

## P9 Exact Parity Data Tables

- `P9-01` `CODED-SUBSTITUTE`: Build a source ledger for every exact value used
  by gameplay: official manual/page, V3 manual, community-derived rule, local
  GIF concept, or direct T-Echo-Lite observation. `CONNECT_SOURCE_LEDGER.md`
  now records growth, shop, password, secret-code, games, death/pass-away,
  medicine, toilet, King donation, visual-reference, and hardware-boundary
  sources for the current firmware behavior. It is not `DONE`: exact death
  thresholds, poop timing, sale frequency, probability curves, original-length
  game mechanics, and rights-cleared final pixel art remain source/art gaps.
- `P9-02` `CODED`: Specify and implement the growth table: stage timing,
  generation/gender effects, care mistakes, training, weight, hunger,
  happiness, sleep, discipline, social score, and random/route constraints.
  Firmware now stores physical/mental care mistakes, resets them on growth,
  uses real pet-minute stage thresholds, carries peer adult tier through
  `FriendPacket` v4, applies the generation 2+ parent-pair child-tier table,
  applies the explicit adult-tier rule table, moves stage thresholds into
  `GrowthScheduleRule`, parent/manual-Matchmaker timing into
  `FamilyTimingRule`, scheduled Matchmaker clock visits into
  `MatchmakerClockRule`, and stage base weights into `StageBaseWeightRule`,
  delegates the old route API to
  the growth state, applies perfect game mistake reversal for
  Get/Flag/Bump/Heading, represents the special Oyajitchi lineage from
  opposite-gender elder LOVE, gates Serious adult only when both parents are
  unhealthy, and selects character catalog rows through generation/tier/gender
  source-pool masks. It is not `DONE`: final per-character art, exact
  same-pool route probabilities, edge-case transition ordering, and hardware
  proof still need full production/validation.
- `P9-03` `CODED`: Specify and implement death/pass-away/runaway behavior:
  depletion thresholds, sickness/neglect timers, age effects, memory/rebirth
  screens, reset prompt, and save continuity. Initial `PassAwayRule` table is
  in firmware and reported by `analysis/growth_coverage/`; exact official
  thresholds and a distinct runaway/pass-away source split remain open.
- `P9-04` `CODED`: Specify and implement stat decay and event probability tables:
  hunger, happiness, weight, poop, illness, toothache, attention, sleep,
  missed calls, pause/time-change effects, and clock rollovers. Hunger,
  happiness, energy, hygiene, poop, sickness triggers, and the sourced
  fifteen-consecutive-snack toothache trigger are now explicit
  tables/constants; sickness trigger thresholds/probabilities/severity rolls
  are isolated in `SicknessTriggerRule`; ordinary miss, care-call, empty-stat,
  sleep-attention, and lights-left-on timing are isolated in
  `AttentionTimingRule`; stage sleep/wake windows are isolated in
  `SleepWindowRule`; lifecycle stage timing, ParentCare departure, manual
  Matchmaker fallback age, scheduled Matchmaker clock visits, and stage base
  weights are also isolated in explicit rules; Hungry/Happy keep a sourced
  two-hidden-heart reserve above the four visible hearts through
  `kHiddenHeartMeterMax` while the display still caps at four pips; the "short
  period" boundary is persisted as a 60 pet-minute substitute window in save
  v9/v10; toothache-only
  missed attention now
  escalates to sickness and resets the timer before the later sickness miss can
  count as a physical care mistake; v10 also persists the stage-local sickness
  count for the fourth-sickness pass-away substitute rule; runtime clock-set
  pause is coded as a state that
  suppresses `pet.tick()` while editing. Exact source values, official toothache
  short-period length, exact attention table, exact sleep table, and exact
  lifecycle timing still need completion.
- `P9-05` `CODED-SUBSTITUTE`: Replace synthetic shop/catalog data with exact
  Connection table data: item names, kinds, prices, shop rotation, inventory
  limits, adult-only restrictions, duplicate/full/no-money/sold-out behavior.
  Catalog logic now uses an explicit segment manifest reported by
  `analysis/growth_coverage/`, a four-slot stock mapper, birthday/holiday
  first-slot substitution, 152 explicit non-souvenir catalog overrides, and explicit
  password/secret overrides. Source label/price corrections are now applied for
  Energy Drink, Marron Cake, Roller Blades, Fishing Pole, TV, Weights, Wig, and
  Wings. Parseable character liked/disliked catalog-food rows are now isolated in
  `CharacterFoodTasteRule`: liked food fills Happy to the hidden-heart cap and
  disliked food clears Happy, while ambiguous rows stay unknown; item-list
  Yogurt/Steak rows are coded as liked-by-all catalog foods. The sourced item
  tranche now has child+/teen+/adult-only use gates plus reusable/single-use flags
  for the known rows. Ticket 1-5 now award the
  fixed source souvenirs, Music Disc requires Boom Box and can break it,
  Make-Up requires Mirror, Shaver is restricted to Oyajitchi, and Tama Drink
  halves sickness chances until the next growth stage. Action Figure/Doll
  enjoyment follows gender, Plant/Shovel/Chest/Lamp/Fishing Pole use
  source-shaped random reward/penalty/break branches, Chest/Lamp can award the
  four sourced exclusive item rows, Fishing Pole retains the item on trash and
  destroys it after reward outcomes, catalog foods/items have per-catalog
  quantity storage with v6-sized save migration and total storage caps, and Hair
  Gel no longer falls through to medicine/charm behavior. Rows 96-143 are now
  source-named extension/duplicate rows instead of generated fallback labels;
  Plant/Shovel/Chest/Fishing Pole/Lamp now read their current substitute branch
  counts, point rewards, and Lamp break chance from explicit
  `CatalogRandomRule` rows. It still needs exact random probability tables,
  exact reward amounts, exact item animation timing, art IDs, and exact shop
  rotation.
- `P9-06` `CODED-SUBSTITUTE`: Replace placeholder password handling with exact
  10-digit password and shop-secret-code tables, including duplicate entry
  behavior, unlock/store flow, and result screens. Password rewards now include
  the 32 listed 2024 10-digit values with explicit catalog slots and
  non-repeatable duplicate handling; Shop secret-code entry uses the official
  1+3 A gesture and opens an A/B/C page
  with the seven sourced code values, explicit catalog overrides, and Hohotchi
  final-code replacement. Honey/Love Potion now stores a one-shot relationship
  boost for the next received friend packet. Clock and RC Car are repeatable,
  and Nyatchi/Hohotchi are temporary costume projections removed at sleep.
  Item-specific use art remains open.
- `P9-07` `CODED-SUBSTITUTE`: Specify and implement exact game reward tables for
  Get, Bump, Flag, Heading, Memory, Sprint, and Hoops: scoring, misses, rounds,
  unlocks, age restrictions, happiness, weight, and Gotchi Point curves.
  Firmware now has `GameUnlockRule` and `GameRewardRule` tables reported by
  `analysis/growth_coverage/`, plus a V3-style `kBumpPrizeChart`, the 9,999
  Gotchi Point wallet cap, and `GamePacingRule` rows for round limit, known
  source goal, score cap, timeout, and compression/input flags. Perfect-care
  reversal now tests against the active per-game score cap. Exact sourced 2024
  low-score curves, Bump/Sprint probability tables, and final hardware-readable
  pacing still need replacement.
- `P9-08` `CODED`: Add host-side checks or generated reports that compare code
  tables against the parity data manifests before firmware packaging. Initial
  model report generator and output live under `analysis/growth_coverage/`.
  Resource/frame plumbing coverage now also lives under
  `analysis/parity_coverage/`; exact official shop/password/game/death manifests
  still need stronger source-backed reports.

## P10 IR-Visible Contract And LoRa Equivalence

- `P10-01` `CODED`: Expand `LINK_CONTRACT.md` into a full IR-visible behavior
  specification before LoRa is considered complete: menus, prompts, standby,
  send, receive, timeout, cancel, failure, result, and retry behavior.
- `P10-02` `CODED`: Specify Visit flow for two devices: compatible states,
  animations, friend slot update, relation/visit counters, rewards, duplicate
  friend behavior, full friend-list behavior, and peer catalog-id identity.
- `P10-03` `CODED`: Specify Present flow for two devices: item/food/points
  selection, send/receive animations, receiver inventory limits, rejection/full
  behavior, and sender/receiver result screens.
- `P10-04` `CODED`: Specify Connection Game flow for two devices: selectable game
  items, simultaneous/paired result, local vs remote score effect, rewards, and
  mismatch/failure behavior.
- `P10-05` `CODED`: Specify Love/partner/baby/next-generation flow for two
  devices: eligibility, relation threshold, reject/accept, partner scene, baby
  scene, parent departure, generation increment, peer catalog-id identity,
  known LOVE partner identity in family history, and save continuity.
- `P10-06` `CODED`: Keep the transport boundary stable: carrier code may move
  bytes by Serial/IR/LoRa, but only the pet model resolves gameplay state.
  Incoming packets must match the current local LINK kind before model mutation,
  and standby now emits same-kind Visit/Present/Game/Love replies without
  overwriting the receiver's result notice.
- `P10-07` `HW`: Validate all Visit/Present/Game/Love paths on two T-Echo-Lite
  devices or a hardware-equivalent two-device setup. Source-level acceptance is
  coded; both physical screens still need proof for standby, SENT, RECEIVED,
  timeout, cancel, mismatch, partner, and baby paths.
- `P10-08` `HW`: Validate LoRa build equivalence: for the same link scenario,
  user-visible behavior must match the IR-visible contract.

## P11 Character And Item Resource Production

- `P11-00` `CODED`: Official how-to animation resources are now represented by
  a metadata-only audit under `analysis/official_howto_audit/`. It records
  grouping, dimensions, frame counts, duration, and contract hints without
  copying or packaging official pixels.
- `P11-00B` `CODED`: Runtime animation/icon resources are source-backed C/C++
  data only: `PROGMEM` bitmap arrays, `FramePart`/`ComposedFrame` composition
  rows, and layout resource byte arrays. PNG/JPG/GIF contact sheets are
  analysis artifacts only and are rejected from runtime paths by
  `analysis/runtime_resource_audit/generate_runtime_resource_audit.py`.
- `P11-01` `CODED`: Build a 50+ character visual manifest: route slot, stage,
  silhouette family, required frame families, 64-class proof status, 128-class
  proof status, and source note. The firmware catalog now uses the 50 official
  names listed by the official character pages and the placeholder `Mame-*`,
  `Tama-*`, and `Elder-*` rows are gone. Compact source page/slot and frame
  family IDs are present and now drive idle sprite selection plus small
  friend/family portrait projection through the renderer; each row also has
  generation/tier/gender source-pool masks plus a stable low-resource
  `visualTraits` byte that drives the main-scene overlay layer. Exact final art
  and 64/128 hardware proof status remain open.
- `P11-02` `CODED`: Refine the default Mametchi-like validation character first:
  scale, twin-ear/twin-horn silhouette, square eyes, layered blink, mouth,
  feet, idle walk/bob, happy/sad/sick/sleep/action poses.
- `P11-03` `CODED-SUBSTITUTE`: Produce original low-resource replacement
  resources for all 50 character route slots without embedding copied official
  art. The current firmware has active frame-family routing plus 50 distinct
  low-resource `visualTraits` rows drawn as head/side/face/foot overlays in the
  main scene. This is still a compact original substitute, not final
  official-look art for every character.
- `P11-04` `CODED`: Build a 150+ item visual manifest: item name, kind, behavior,
  price/password/shop source, icon frame, use animation, 64-class proof, and
  128-class proof. Firmware now carries a 160-entry segment manifest with
  source-group and use-scene fields plus a named souvenir table. The ITEM page
  now uses the 160-row catalog cursor, preview drawing uses `entry.icon`,
  use-result drawing uses `entry.useScene`, and catalog rendering consumes each
  row's stable `visualTraits` byte; all 152 non-souvenir catalog rows are
  explicit source-named overrides, several shop items have distinct compact
  icon drawings, and the 64-row souvenir browser has a separate visual-trait
  icon path. Exact final per-item/souvenir art remains open.
- `P11-05` `CODED-SUBSTITUTE`: Produce original low-resource icons/props for
  all 150+ food, item, password, secret, and souvenir entries. Firmware now
  assigns all 160 catalog rows a distinct low-resource `visualTraits` byte and
  draws edge/detail overlays on top of the base icon/use-scene art; the 64
  souvenir rows also have distinct compact visual-trait icons on the MEMORY
  page. This gives a proofable per-row replacement contract, but it is still
  compact substitute art rather than final hand-drawn Connection-style art for
  every entry.
- `P11-06` `CODED`: Update `EchoPetResources64.*` and `EchoPetResources128.*` so
  every required frame has a named resource or explicit documented fallback.
- `P11-07` `HW`: Sprite proof every character, action, game prop, shop booth,
  shopkeeper, shop preview/result, family, Love, pass-away, and item frame on
  both screen layouts.

## P12 Full Hardware Burn-Down

- `P12-01` `HW`: Button matrix acceptance: Esc/Home/Email short press, long
  press, release timing, debounce, accidental repeats, and hidden diagnostic
  paths.
- `P12-02` `HW`: Refresh acceptance: cold boot, idle, menu cycling, notice hold,
  care scenes, games, shop, password, dark-room, long repeated use, whitening,
  ghosting, stale frames, and synchronous partial-refresh pacing.
- `P12-03` `HW`: Fixed icon acceptance: each of the ten Connect icons opens a
  real behavior with entry/action/result feedback and a clear exit.
- `P12-04` `HW`: Setup and health-meter acceptance: field order, digit editing,
  page order, text fit, icon state, and saved-state continuity.
- `P12-05` `HW`: Game acceptance: each of the seven games is understandable at
  e-paper speed and shows score, miss, win/lose, and reward feedback.
- `P12-06` `HW`: Resource acceptance: every sprite proof row has a pass/fail
  entry and defects are tied to frame names, anchors, and scenes.
- `P12-07` `HW`: Release acceptance: ordinary and LoRa firmware packages are
  built, flashed, smoke-tested, and linked to the hardware acceptance log.
