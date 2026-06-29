# EchoPet Functional Parity Contract

Target behavior: Tamagotchi Connection / Connection V3 style player-visible behavior
and visual motion language, using the official Connection web pages and manuals
as references.

This project must not depend on original Tamagotchi ROM/source code. Official
website art and animation are reference material for shape, scale, pose, and
motion timing. Do not mechanically copy or redistribute official image assets
inside the repository unless the user supplies rights-cleared assets. Functional
parity means the button flow, menu reachability, state changes, restrictions,
rewards, timing, feedback, persistence, and link outcomes match the target
behavior closely enough that a player recognizes the same virtual-pet system.
Visual parity means the pet size, placement, life-stage silhouette, local feature
animation, and action poses are close enough to the official Connection reference
that movement and interaction can be validated on hardware.

Status legend:

- `[x]` aligned enough for the current target
- `[~]` implemented but behavior still needs parity work
- `[ ]` missing or known wrong

References used for parity notes:

- Bandai Tamagotchi Connection 2024 instruction manual
- Tama Planet Tamagotchi Connection V3 manual
- Official Tamagotchi Connection how-to page:
  `https://tamagotchi-official.com/gb/series/connection/howto/`
- Official Tamagotchi Connection character and animation reference pages
- Local visual frame contract: `VISUAL_REFERENCE_CONTRACT.md`
- Local GIF animation concept audit: `GIF_ANIMATION_AUDIT.md`
- Firmware sprite render audit: `analysis/sprite_render_audit/SPRITE_RENDER_AUDIT.md`
- Ordered burn-down plan: `CONNECT_BURNDOWN_PLAN.md`
- T-Echo-Lite hardware acceptance log: `T_ECHO_LITE_HARDWARE_ACCEPTANCE.md`
- Source ledger: `CONNECT_SOURCE_LEDGER.md`
- Growth specification: `CONNECT_GROWTH_SPEC.md`
- Generated completion audit:
  `analysis/completion_audit/completion_audit.md`
- Generated hardware readiness audit:
  `analysis/hardware_readiness/hardware_readiness.md`
- Visual alignment gate:
  `VISUAL_ALIGNMENT_GATE.md` and
  `analysis/visual_alignment/visual_alignment_report.md`

## Non-Negotiable Scope

- [~] Current phase gate: visual alignment is the hard gate before additional
  behavior expansion. Simulator previews for both profiles must remain clean,
  and any visual rows still marked `OPEN` or `HW` in
  `analysis/visual_alignment/visual_alignment_report.md` cannot be described as
  complete visual parity.
- [x] Character art can be original/substitute.
- [~] Character art can no longer be arbitrary: `VISUAL_REFERENCE_CONTRACT.md`,
  the 50-row character manifest, per-catalog bitmap routing, fallback
  frame-family silhouettes, and visual-trait overlays now constrain
  scale/silhouette/poses. Official 50-row character references are now isolated
  under `analysis/official_reference/`, and
  `generate_character_source_arrays.py` converts them into the runtime
  50-row C/C++ catalog and idle bitmap tables. The generated
  `character_source_alignment_status_contact_sheet.png` and JSON now compare
  each official reference row against its 16x16 and 24x24 C++ rows. Final
  detail readability and hardware proof remain open.
- [~] Local GIF concept designs under `C:\Users\vicliu\Projects\pet` are treated
  as animation contracts in `GIF_ANIMATION_AUDIT.md` and `analysis/gif_audit/`;
  coded scenes now map the care/shop/game phases to named props and frames.
  Hardware pacing acceptance remains open.
- [x] Runtime animation and icon resources must be C/C++ source data only:
  `PROGMEM` bitmap arrays, `FramePart`/`ComposedFrame` tables, and layout
  resource byte arrays. PNG/JPG/GIF files are allowed only under `analysis/` as
  proof artifacts or concept-audit outputs; they are not firmware assets.
  This boundary is guarded by `RUNTIME_RESOURCE_CONTRACT.md` and
  `analysis/runtime_resource_audit/runtime_resource_audit.md`.
- [~] Gameplay behavior is not allowed to be "demo text pages"; current menus
  route to model actions, animated scenes, inventory/shop/password state,
  mini-games, and LINK flows. Exact Connect timing/data still need parity work.
- [~] Every fixed icon must open a real interactive behavior or a stateful scene.
- [~] Menu behavior, care mistakes, rewards, item effects, and link outcomes need
  explicit acceptance checks.
- [~] LoRa mode must preserve the same player-visible behavior as IR mode; only
  the transport changes. `LINK_CONTRACT.md` and source-level kind matching now
  enforce the shared behavior boundary, while two-device LoRa proof remains open.
- [~] Exact data tables are required for growth, death/pass-away, stat decay,
  shop/catalog, password/secret code, and game rewards. Synthetic placeholder
  values are not final parity. Growth model infrastructure is now coded, but
  full official route rows are still open.
- [ ] Hardware-only claims must be closed in `T_ECHO_LITE_HARDWARE_ACCEPTANCE.md`
  before any `HW` task is marked done.

## Display And Controls

- [x] T-Echo-Lite physical buttons are mapped as Connect-style A/B/C:
  Esc = A/select, Home = B/confirm, Email = C/cancel/status.
- [x] `128x64` layout reserves a `64x64` center playfield with `32x64` menu rails.
- [x] `176x192` layout reserves a `128x128` center playfield with side menu rails.
- [x] Main screen no longer has persistent multi-row debug HUD.
- [x] Top/bottom info space is limited to at most one larger text line each.
- [~] Application-level redraw suppression is connected for normal animation and
  menu changes. EchoPet compares the previous and current frame buffers and
  skips the hardware refresh when no pixels changed.
- [~] T-Echo-Lite e-paper refresh is intentionally back on the LilyGo-proven
  path used by `trail-mate`: before the first partial update it establishes a
  FAST base map, then submits the whole frame through `display(PARTIAL_REFRESH)`.
  The experimental byte-window partial path was removed because it caused
  progressive whitening on real hardware.
- [~] Partial refresh submission now follows the simpler blocking LilyGo sample
  call path: changed frames wait only for the minimum submit interval, establish
  the FAST base map once, and call `display(PARTIAL_REFRESH, true)`
  synchronously. The extra application-level pending/in-flight BUSY coalescing
  state machine was removed because it could hide animation steps and made the
  refresh behavior harder to reason about. Hardware validation is still required
  for missed frames, stale frames, ghosting, whitening, and button
  responsiveness under repeated menu/game input.
- [x] Ten fixed shell icons now have explicit dual-size C/C++ bitmap resources:
  compact 12x12 rows and large 30x30 rows in
  `EchoPetMenuIconResources.cpp`. Both sizes share the same official semantic
  table and must stay visually recognizable as the same concepts.
- [x] Official fixed-menu reference material is now isolated under
  `analysis/official_reference/`, including the Bandai Connection manual PDF,
  manual callout crops, and
  `generated/fixed_menu_official_alignment_proof.png`.
- [x] Fixed-menu 12x12 and 30x30 C/C++ arrays are now generated from
  analysis-only official manual references by
  `analysis/official_reference/generate_menu_icon_source_arrays.py`. The
  compact 12x12 rows and large 30x30 rows now both use the same official
  device/menu tile crop for each fixed slot, with the source choice recorded in
  `fixed_menu_source_policy_contact_sheet.png` and
  `fixed_menu_source_policy.json`. Generated previews live under
  `analysis/official_reference/generated/`, including
  `fixed_menu_compact_official_source_contact_sheet.png`.
- [~] The 128-class shell icons are now true 30x30 1-bit resources. They are
  derived from official manual tile crops, but still require human acceptance
  against the official reference proof because the manual crop source is
  low-resolution and contains nearby callout marks.
- [~] Attention icon highlight now pulses when the pet is calling for care.
  Beep/sound and exact timeout cadence still need hardware parity validation.

## Interaction Completeness And E-Paper Pacing

Hardware-test finding: the previous checklist was too feature-name oriented. A
feature is not accepted just because a menu entry or text page exists. It is
accepted only when the player can understand the A/B/C controls, see entry and
result feedback on the real e-paper panel, and observe the state change that the
Connection behavior implies.

- [~] Home/B long-press enters the runtime `CLOCK` edit screen on T-Echo-Lite;
  while editing, `pet.tick()` is suppressed so the clock-set state behaves as
  the source-backed pause method rather than a fake pause toggle. Mail/C
  long-press enters the hidden sprite proof view from the home screen. KeyShield
  hold events are now emitted when the hold threshold is reached instead of only
  after release, and release after a hold is suppressed so it cannot also fire a
  short press. A/B advances raw sprite frames and C exits. The proof page is
  diagnostic only, not a Connection gameplay feature.
- [ ] Short-press and long-press semantics must be validated on Esc/Home/Mail
  with real hardware, including debounce, hold duration, accidental repeats, and
  release timing.
- [~] Every interactive screen must expose the current A/B/C meaning through
  labels, layout, or obvious selected state. The player must not need to guess
  whether a button cycles, confirms, cancels, sends, buys, feeds, disciplines,
  or exits. Current build adds compact/large control hints, but hardware fit
  still needs validation.
- [~] A fixed-icon action is not accepted unless it has visible entry feedback,
  active/action feedback, result feedback, and a clear way to exit.
- [~] Care actions must be staged as before/action/after flows. Static pages or
  text-only pages do not satisfy toilet, medicine, discipline, lights, feeding,
  item, shop, password, or connection parity. Toilet, medicine, discipline, and
  lights now distinguish success/no-op/error result states visually.
- [~] Notice/result messages must remain visible long enough to survive the
  panel refresh. A message is wrong if the model clears it before the user can
  see it. Notices are now held for a minimum visible interval after the
  synchronous refresh path renders them.
- [~] High-interaction paths must be tested against synchronous partial-refresh
  panel pacing: menu cycling, game input, password digit editing, shop/item
  browsing, setup entry, friend delete prompts, and link standby/cancel/send.
  Source-side readiness is covered by the refresh/game/link readiness reports;
  final acceptance remains a T-Echo-Lite hardware gate.
- [~] Games must be e-paper-compatible. Timing-only mechanics are not accepted
  on this panel unless the visual timing is actually visible and repeatable on
  hardware. Current game logic is input-step driven; active games now target
  250-300 ms animation intervals, ordinary action scenes target 360 ms, and
  final panel submissions use the synchronous 250 ms partial-refresh submit
  guard. Final pacing still needs hardware validation.
- [~] Game screens now show compact A/B/C hints and model-exported score/round
  limits. Reward, miss, win, lose, and 250-300 ms pacing feedback still need
  hardware validation.
- [~] The sprite proof view is a required QA step for each named sprite frame
  before judging scene composition. Sprite defects must be recorded against the
  frame name, anchor, and scene that uses it. Frame metadata now exists, and
  host-side contact sheets now render all 101 `SpriteFrame` rows with zero
  missing mappings and zero nominal-frame overflows. Real proof photos still
  need to be recorded.
- [~] Checklist rows must describe player-visible acceptance criteria, not only
  internal systems or code objects. `CONNECT_PARITY_TASKS.md` now tracks this
  as a task queue with CODED/HW/TODO states.

## Fixed Connect Menu Icons

The fixed icon order is:

1. Health meter
2. Food
3. Toilet
4. Connection
5. Attention
6. Discipline
7. Medicine
8. Lights
9. Friend list
10. Games / activity

Current status:

- [x] The fixed list is limited to these ten icons.
- [x] Selection cycles through the ten icons.
- [x] Each icon currently opens a dedicated page or scene.
- [~] Several pages are still simplified and do not yet reproduce the complete
  Connect behavior behind that icon.

## Life Setup And Core State

- [~] Initial setup flow exists for empty save in the official manual order:
  date/time, birthday, user name, boy/girl reveal, pet name, and hatch
  confirmation. Language selection is omitted because this firmware is
  English-only. Digit editing style, clock/date validation, sound feedback, and
  hatch timing still need hardware validation.
- [~] Saved state exists.
- [~] Save data schema has named fields for currently modeled player-visible
  state, including toothache and attention timing. Exact Connection field audit
  remains incomplete.
- [~] Age, generation, weight, money/points, hunger, happiness, training, illness,
  toothache, sleep, poop, relationship, and inventory-like state exist.
- [~] Exact stat bounds, decay rates, timers, and event probabilities need parity
  tables and tests.
- [~] Death/pass-away memory and rebirth flow exist. When passed, Home/B
  long-press opens RESET confirmation; while alive, Home/B long-press opens the
  runtime CLOCK edit pause and Mail/C long-press opens sprite proof. Exact
  Connect pass-away/runaway timing still needs parity.
- [~] Exact growth, death/pass-away/runaway, stat decay, and event probability
  tables are now P9 burn-down tasks. Growth now has a coded physical/mental
  mistake model and real pet-minute stage timing, but death, decay, and event
  probability tables still contain substitute rules.
- [~] Pause/time-change side effects are partially coded: runtime CLOCK edit
  suppresses `pet.tick()` while editing, so growth/stat time does not advance
  during the edit screen. Exact Connect clock-change side effects and hardware
  button validation remain open.

## Growth And Character Routes

- [~] Growth stages exist.
- [~] Growth now stores stage-local physical/mental care mistakes, growth tier,
  adult tier, parent tiers, and born-from-unhealthy ancestry in the model/save
  schema. The old route labels remain only as a visual fallback for the current
  small sprite archetype set.
- [~] `CONNECT_GROWTH_SPEC.md` now has coded growth tranches: real pet-minute
  timing, stage-local reset, save migration, matchmaker time windows,
  perfect-game care mistake reversal, LINK-carried peer adult tier, generation
  2+ parent-pair child table, child-tier teen transition table, explicit
  adult-tier transition table, Serious-adult parent gate requiring both
  parents unhealthy, special Oyajitchi lineage, and generation/tier/gender
  source-pool character selection. Adult tier thresholds are now transcribed
  into firmware rows; final per-character art, exact same-pool probability,
  edge-case transition ordering, and hardware proof remain open.
- [~] Care-mistake counting and growth branch thresholds exist but need exact
  Connection/V3 table validation and host-side model tests.
- [~] Training, discipline, weight, happiness, hunger, sleep, and missed-call
  effects on growth now have source-coverage checks in
  `analysis/growth_coverage/growth_coverage.md`. Exact Connect numeric parity
  tests and official thresholds remain open.
- [x] Character sprite art may remain substitute while route behavior is aligned.

## Food

- [~] Meal and snack behavior exists.
- [~] Connect-style food screens, item selection, refusal conditions, fullness,
  weight gain, happiness gain, toothache risk, and animation feedback need parity.
  Hungry/Happy now reserve two hidden hearts above the four visible health-meter
  hearts; the extra reserve affects meal/snack/play fullness and decay without
  drawing extra pips.
  Item-list Yogurt and Steak rows are coded as liked-by-all catalog foods.
  Parseable character liked/disliked catalog-food rows now fill Happy to the
  hidden cap or clear Happy; ambiguous source rows remain unknown rather than
  guessed.
  The sweet-snack toothache path now follows the sourced fifteen-consecutive
  snack trigger through `kSweetSnackToothacheStreak`, with the "short period"
  boundary persisted as a 60 pet-minute substitute window in save v9/v10; exact
  official short-period timing remains a source gap.
  Sick/toothache state now refuses meal, snack, game, item, and catalog-item use
  with `SICK` notice instead of silently allowing normal care loops. A missed
  toothache-only care call now escalates to sickness and resets the 15-minute
  call timer instead of immediately counting as a care mistake.
- [~] `food and snacks.gif` exposes a staged feed contract: selected
  food/snack prop, bite/chew mouth frames, crumbs/disappear frames, refusal, and
  explicit done/result feedback. Current firmware now routes those phases
  through compact scene objects: selected food, plate, bite travel, crumbs,
  done check/heart, and refusal cross. Exact Connection bite count, timing, and
  hardware readability remain open.
- [~] Shop food/item stock and bought-item usage are not Connect-complete, but
  source-backed food/item name/price tranches, stock caps, secret/password
  rewards, reusable/one-shot item behavior, sale/cart windows, and use-scene
  routing are coded as explicit substitute data.
- [~] Food stock is capped to 19 and item stock is capped to 32. Shop purchases
  refuse with `FULL` before spending points when the target stock is full. Exact
  price tables still need validation.

## Toilet

- [~] Toilet action cleans messes and has a graphical scene.
- [~] Exact mess timing, before/after toilet timing, sickness effects, and care
  mistake rules need parity. Current firmware has explicit mess counts,
  cleanup/no-mess branches, sickness/toothache links elsewhere in the model, and
  staged visual cleanup, but exact event timing remains substitute.
- [~] Toilet scene now distinguishes cleanup from no-mess result and animates
  sweep/sparkle phases. Exact before/after timing still needs validation.
- [~] `Clean up the poop.gif` shows that cleanup must include visible mess,
  pet reaction, multi-frame sweep/flush prop motion, progressive removal, and
  after-state recovery. Current firmware now routes these through compact scene
  objects: visible messes, sweep tool, progressive mess removal, clean slot,
  done heart/sparkle, and no-mess question branch. Exact Connection mess timing,
  sickness side effects, and hardware readability remain open.

## Games And Activity

- [~] A game system exists and can award points.
- [~] The current games are not exact faithful implementations of Connect/V3
  activities, but all seven have coded mechanics, props, prompts, result
  feedback, and table-driven substitute pacing/rewards.
- [~] Seven expected activities need real mechanics and reward curves:
  Get, Bump, Flag, Heading, Memory, Sprint, and Hoops.
- [~] Local GIF concepts for Get, Bump, Flag, Heading, Memory, Sprint, and Hoops
  define required prop-level animation and visible result states. Each game must
  match its GIF-derived scene contract before it can be marked accepted:
  falling notes/bad items, force meter/opponent shove, flag prompt/response,
  ball path/head hit, memory sequence replay, sprint lane/finish, and
  hoop/ball arc/shoot result. Firmware now exposes compact substitute props for
  each of those games, while frame-by-frame source and hardware parity remain
  open.
- [~] Core game controls are now closer to the manual-visible behavior: Get is a
  timed A/C catch-or-dodge window, Bump and Sprint accept any button press, and
  Flag maps black/white/both flags to `A`/`C`/`A+C` with short fake-signal
  `WAIT` rounds. Hoops uses `A/C` aim, `B` shoot, plus timeout miss. Bump now
  uses a V3-style 0-8 round by four-weight-band prize chart and no longer
  grants heavier pets extra score directly; heavier weight only biases the
  shove chance. Timing, non-Bump reward curves, and failure windows still need
  source and hardware validation.
- [~] Game entry now follows the source-shaped base-weight refusal rule instead
  of local hunger/energy gates. Stage base weights are table-driven
  (`Baby=5`, `Child=10`, `Teen=20`, `Adult/Parent/Elder=30`), growth raises
  weight to the new base, and winning a game cannot reduce below the current
  base. Hardware still needs to confirm the player-visible refusal/recovery
  loop at base weight.
- [~] Game screens show compact A/B/C control hints, prompt, score, and
  model-exported round/score limits. Miss/win/lose feedback and 250-300 ms
  action pacing are still not accepted until they are visible on real hardware
  during repeated play.
- [~] Unlock timing, age/stage restrictions, score thresholds, weight changes, and
  point rewards need parity.
- [~] Exact per-game reward tables are incomplete: scoring, round count, miss
  condition, unlock/stage restriction, happiness change, weight change, and
  Gotchi Point curve are table-driven substitutes with sourced top prizes. The
  Bump prize table and 9,999 Gotchi Point wallet cap are now explicit; exact
  2024 low-score curves, original-length mechanics, and probability windows
  remain open.
- [~] Game screens must animate enough to be recognizable as the original behavior,
  without requiring original art.

## Connection / Friends / Presents

- [~] A connection state machine and LoRa build exist.
- [~] IR-visible behavior is specified in `LINK_CONTRACT.md` for Visit, Present,
  Game, and Love before LoRa substitution. Exact official infrared pulse timing
  remains out of scope; two-device visible proof is still required.
- [~] IR-visible behavior is now specified independently from transport:
  `FriendPacket` v4 and `receiveFriendPacket()` remain the gameplay boundary
  for Serial/IR/LoRa. The packet carries peer `catalogId` in addition to
  growth/adult tier and gender so friend-list identity is visible and transport
  code still stays gameplay-neutral.
- [~] Connect standby, host/join roles, timeout, cancellation, result screens, and
  friend updates are coded. Receive windows reject mismatched LINK kinds before
  model mutation, and standby auto-replies with the same kind for Visit, Present,
  Game, and Love.
- [~] Visit/play/present/game outcomes need parity.
- [~] Love/partner/baby/next-generation is coded as a model/UI flow: eligibility,
  reject/accept, partner scene, baby scene, parent departure, generation
  increment, peer catalog-id identity, known LOVE partner identity in family
  history, and save continuity exist. It still needs two-device T-Echo-Lite
  proof for sender/receiver timing and LoRa equivalence.
- [~] Friend slots, relationship, visits, presents, and delete-like flows exist.
- [~] Friend list capacity, ordering, relationship labels, visit/gift counts, and
  deletion prompts are visible. Friend portraits now render from the stored
  peer catalog id, with archetype fallback for old rows. Exact Connection
  ordering/history parity still needs validation.
- [~] LoRa transport maps to the same behavior contract without changing user
  flow in code; the remaining gate is two-device hardware validation.

## Discipline And Attention

- [~] Discipline action exists and has a graphical scene.
- [~] Praise/time-out style conditions need exact parity.
- [~] Attention calls, missed calls, false calls, training gain, care mistakes, and
  icon visibility need exact parity.
- [~] User feedback distinguishes successful timeout, successful praise,
  invalid discipline, active attention-call, and missed/invalid feedback.
  Exact missed-attention timing still needs parity.
- [~] `Discipline when needed.gif` requires separate visible TIME OUT, PRAISE,
  invalid, attention-call, and missed-call feedback phases. Current firmware now
  routes those phases through compact low-resource scene frames; exact
  Connection timing and hardware readability remain open.

## Medicine

- [~] Medicine action exists and has a graphical scene.
- [~] Illness vs toothache conditions, dosage count, cure probability, refusal,
  and animations need parity.
  Food/game/item refusal while sick or toothache is now enforced in the model;
  toothache-only missed care now becomes sickness before a care mistake can be
  counted; exact cure probability and source timing remain substitute values.
- [~] Health meter indicators for skull/tooth states need validation.
- [~] `Cure when ill.gif` requires sick symbol, medicine/dose prop, waiting or
  negative reaction, cure/failure/refusal branches, and a distinct recovery pose.
  Current firmware now routes these through compact scene objects: bottle,
  skull/tooth/cured/refuse badge, dose trail, recovery frame, and refusal frame.
  Exact Connection timing, probability, and hardware readability remain open.

## Lights / Sleep

- [~] Lights action exists and has a graphical scene.
- [~] Sleep schedule is now stage-based, missed bedtime lights-off can trigger
  attention/care mistakes, and wake-up auto-clears lights-off. Nap and exact
  Connection timings still need parity.
- [~] Dark-room rendering should be stable and not cause intrusive full-screen
  flashes. Current firmware no longer draws the lights-off scene as a full-width
  black fill; it uses a low-ink border/hatch dark-room pattern, moon/star marks,
  and a separate ON/OFF selector. Real ghosting/whitening still needs hardware
  acceptance.
- [~] `Lights off at night!.gif` requires an ON/OFF selector, sleep pose/Z
  frames, dark-room state, wake/invalid feedback, and hardware-stable dark-room
  refresh. Selector, sleep, wake, invalid, and low-ink dark-room routing are
  coded; final stability still needs hardware acceptance.

## Health Meter

- [~] Health meter page exists with graphical meters.
- [x] Hungry/Happy meters intentionally display only four visible hearts while
  the model keeps two hidden hearts internally for Connection-era fullness.
- [~] Screen order and exact visible fields need parity:
  hunger, happiness, training, age, weight, name, gender, generation, points, and
  other version-specific fields. Current build shows these fields across six
  pages, but exact order still needs validation.
- [~] Page cycling with A/B/C exists: A/B advance and C exits. Exact Connection
  page order and sound feedback still need parity.

## Items / Shop / Password / Souvenir

- [~] Internal item/shop/password/souvenir actions exist.
- [~] Their menu reachability is now aligned with the ACTIVITY submenu surface:
  game, item, shop, password, souvenir, and point/donation.
- [~] The point/donation page now has compact King/donation visual feedback:
  coin stack, donation progress bar, no-points slash, donate sparkle, and
  completion heart. Exact King congratulation/cutscene art remains open.
- [~] Shop/item/password/souvenir now use a 160-item catalog and a 64-memory
  souvenir bitset. Shop browsing is constrained to four stock slots mapped to
  catalog items, and password/secret-code rows have explicit overrides. Item
  effects still map to compact behavior archetypes, and full exact item
  names/prices/restock rules remain a parity-data task.
- [~] `shop.gif` requires a visible shop booth/shopkeeper scene with neutral,
  surprise/happy, purchase success, no-money, full-stock, and sold-out feedback.
  The proofable shop booth/counter frame, shopkeeper frames, item preview,
  purchase-result variants, official 1+3 A surprise entry, and 8-symbol A/B/C
  secret-code entry are coded; result timing still needs hardware-visible
  refinement.
- [~] Exact 150+ item data and visuals are incomplete: item names, prices,
  kinds, shop/password/secret source, use restriction, behavior, icon, and use
  animation are represented in the 160-row catalog contract, and every row now
  has a concrete 16x16 C++ source bitmap row in `kCatalogEntryBitmaps` for
  catalog preview rendering. Item visual basis still retains 41 shared families
  for non-catalog props and fallback drawing, while the 64-row souvenir browser
  has source-backed per-memory C++ bitmap rows. Official item reference material
  is isolated under
  `analysis/official_reference/` and
  `generated/catalog_item_official_alignment_proof.png` compares those official
  item references against the current EchoPet source-rendered catalog proof.
  `generated/catalog_item_source_semantic_status_contact_sheet.png` now lists
  all 152 non-souvenir catalog rows with their current C++ icon and status:
  official-source override or section-level official scene reference with
  substitute art.
  MEMORY/souvenir rows now have the same official-reference boundary through
  `analysis/official_reference/generated/souvenir_official_alignment_proof.png`:
  the proof groups official how-to scene/icon candidates by row, shows all 64
  current C++ bitmap rows with semantic status, and records TRUMPET/BALL PLAY
  as unambiguous official-source C++ overrides. The official site material is
  still not a complete 64-row souvenir atlas, so unaccepted rows remain
  source-backed substitutes rather than silently becoming "official".
  The current 160-row table is generated from the previous low-resource proof
  so replacement is now row-local; exact non-food item rows and final
  human-accepted per-row C/C++ item and souvenir art remain open.

## Animation And Asset Architecture

- [~] Larger character sprites and layered face features exist.
- [~] Egg movement exists, and the idle egg face is now composited inside the
  shell rather than below it. Low-resource crack/hatch frames are now coded and
  proofable; final official-reference art and hardware photos remain open.
- [x] Official Connection website visual references have a concrete local
  frame-family contract in `VISUAL_REFERENCE_CONTRACT.md`.
- [~] Official Connection website visual references and local GIF concepts must
  be audited into frame-by-frame requirement lists for egg, baby, child, teen,
  adult, care, food, toilet, medicine, lights, discipline, games, connection,
  item, happy, sad, sick, sleep, family, shop, and death/pass-away states.
- [x] Local GIF concept files have an explicit audit in `GIF_ANIMATION_AUDIT.md`
  with contact sheets and frame metrics under `analysis/gif_audit/`.
- [x] Firmware sprite composition has a host-side render audit under
  `analysis/sprite_render_audit/`. It parses the firmware source tables, emits
  contact sheets for all 101 `SpriteFrame` rows, and currently reports zero
  missing mappings and zero pixels outside each nominal frame.
- [~] The default adult validation character should visually align with the
  official Mametchi-like Connection silhouette in scale, eye spacing, body
  placement, feet, and idle walk/bob motion.
- [x] Sprite definitions now expose named action frames for idle, hatch/crack,
  food/snack, toilet, medicine, lights, discipline, attention, item play,
  connection, love/partner/baby, game families, game win/lose, and pass-away.
- [x] Runtime sprite/menu-icon resources are source-backed C/C++ bitmap arrays,
  not PNG/JPG/GIF assets. `EchoPetSprites.cpp`,
  `EchoPetCharacterVisuals.cpp`, `EchoPetCatalogVisuals.cpp`, and
  `EchoPetResources128.cpp` are the current bitmap truth; generated contact
  sheets remain analysis-only, and `EchoPetResources64.cpp` is layout truth
  rather than a second shell-icon set.
- [~] Sprite definitions still need full named-frame coverage for all
  stage-specific character routes and every individual shop/password/souvenir
  item animation.
- [~] Official-feeling 50+ character resources are not complete. The current
  50-row catalog maps official-page names to source page/slot, fallback visual
  family IDs, and concrete 16x16 catalog bitmap rows. Main-scene idle rendering
  plus FRIEND/FAMILY avatars now draw from those 50 source-backed per-catalog
  bitmap rows. Those rows are now generated from the analysis-only official
  character references, and the `visualTraits` high bit suppresses the previous
  programmatic overlay so official-derived silhouettes are not distorted.
  Official character page thumbnails and alt text are cached under
  `analysis/official_reference/raw/character`, with
  `generated/character_official_alignment_proof.png` and
  `generated/character_source_alignment_status_contact_sheet.png` exposing the
  row-local official/16x16/24x24 gap; final detail readability and hardware
  proof remain open.
- [~] Sprite definitions need a complete named-frame contract so future pets can
  be redrawn without touching gameplay code.
- [~] Local feature animation now has reusable low-resource eyes, mouth, feet,
  crumbs, sweep tools, medicine symbols, flags, balls, notes, hoops, baby props,
  and game meters. Shopkeeper-specific faces and per-item art remain open.
- [~] Asset metadata now defines required frame family, anchor, layer, mirror
  rule, and duration in `VISUAL_REFERENCE_CONTRACT.md`. The code-side sprite
  table still needs to expose all of that metadata directly.
- [~] Action scenes now go through application-level redraw suppression and the
  LilyGo-proven whole-frame partial refresh path. Hardware ghosting and
  orientation still need real-device validation.
- [x] Two resource files remain separated by layout class:
  `EchoPetResources64.cpp` and `EchoPetResources128.cpp`.

## Official How-To Visual Audit

The official how-to page is now an explicit acceptance input, not a vague visual
inspiration. The local implementation must match these player-visible groups
with original/rights-cleared low-resource artwork:

- [x] The official how-to page has a metadata-only audit under
  `analysis/official_howto_audit/`. It records dimensions, frame counts,
  unique-frame counts, durations, motion bounding boxes, and local
  frame-family hints while intentionally not storing official image pixels.

- [~] Nurture/menu actions: status check, food/snack, poop cleanup,
  discipline, illness cure, lights off.
- [~] Growth presentation: egg to adult, more than 50 official-style character
  targets. Current code has 50 named character rows with source page/slot IDs
  plus generation/tier/gender source-pool masks, per-catalog bitmap routing,
  fallback frame families, and distinct `visualTraits` overlays; full unique
  official-look sprite art per slot is still incomplete.
- [~] Seven activity animations and reward curves: Get, Bump, Flag, Heading,
  Memory, Sprint, Hoops. Runtime pacing now uses game-specific e-paper
  intervals derived from official animation density rather than one shared
  slow interval.
- [~] 150+ item collection breadth: 160 catalog entries exist and can be bought,
  awarded, stored, shown, and used through compact behavior archetypes. The
  ITEM page now follows the 160-row catalog cursor, preview art uses each row's
  icon field, use-result art uses each row's use-scene field, and preview/use
  art now draws each row's `visualTraits` overlay. Ticket 1-5 award the fixed
  Skis/Palm Tree/Surfboard/Panda Bear/Maracas souvenirs; Music Disc requires
  Boom Box and can break it; Make-Up requires Mirror; Shaver is restricted to
  Oyajitchi; Tama Drink halves sickness chance until the next growth stage;
  Action Figure/Doll enjoyment follows gender; Plant/Shovel/Chest/Lamp/Fishing
  Pole use source-shaped random reward/penalty/break branches, Chest/Lamp can
  award the four sourced exclusive item rows, Fishing Pole retains the item on
  trash and destroys it after reward outcomes, catalog foods/items have
  per-catalog quantity storage with v6-sized save migration and total storage
  caps, and Hair Gel is a cosmetic consumable rather than a medicine/charm
  effect. All non-souvenir catalog rows 0-151 now have explicit source-named
  overrides so generated `MUSIC097`/`TRAVEL121` labels no longer reach the
  player; rows 96-143 are source-named extension/duplicate rows rather than
  new exact official rows. Catalog preview art now routes through 160 explicit
  C++ row bitmaps generated by
  `analysis/screen_simulator/generate_catalog_entry_source_arrays.py`; 12
  unambiguous item rows now have official-source C++ bitmap overrides from the
  official how-to item icon sheets: PENCIL 48/112, CAP 58/122, SHOVEL 54/118,
  BALL 60, BALLOON 56/63/120, and TRUMPET 84/141. The MEMORY page now draws 64 separate
  source-backed per-memory souvenir bitmaps, and the souvenir proof chain under
  `analysis/official_reference/generated/souvenir_official_alignment_proof.png`
  compares those rows against official how-to semantic candidates. TRUMPET and
  BALL PLAY are currently generated from unambiguous official item-icon source
  crops; the other MEMORY rows stay substitute until row art is accepted or
  supplied.
  Exact random probability tables, exact reward amounts, exact item animation
  timing, most final per-item art, and final per-souvenir art are still
  incomplete. The official
  item how-to PNG/GIF references now have a
  generated local proof sheet under
  `analysis/official_reference/generated/catalog_item_official_alignment_proof.png`,
  so future closure requires replacing or refining runtime C/C++ bitmap rows
  against that proof rather than relying on generic icon families.
- [~] Password entry flow: 10-digit input exists, the 32 listed 2024 password
  values are mapped to explicit catalog label/price overrides, Shop 1+3 A
  secret-code entry opens an A/B/C code page with the seven sourced shop-code
  values, and a compressed catalog-password family can unlock any of the 160
  entries. Hohotchi final-code replacement and reusable-use semantics are
  coded. Honey/Love Potion now consumes the secret-code item and boosts the next
  received friend relationship. Clock and RC Car are repeatable, and
  Nyatchi/Hohotchi are temporary costume projections removed at sleep.
  Item-specific art, two-device timing, and older V3 password-generator
  compatibility are still incomplete.
- [~] Connection presentation: Visit, Present, Game, and Love are visible LINK
  modes. Standby/result screens exist; ineligible Love now rejects, and baby is
  gated to eligible Love flow rather than any Partner-level connection. Unique
  official Love/partner/baby animations still need full sprite coverage. The
  FAMILY history page now renders compact album cards from saved family rows
  using character-catalog parent/baby avatars, known LOVE partner avatars when
  the row carries a partner catalog sentinel, and an explicit unknown-partner
  silhouette for legacy/Matchmaker rows instead of text-only records or
  seed-generated faces.

## Local GIF Concept Audit

The local concept GIFs in `C:\Users\vicliu\Projects\pet` are now explicit
acceptance inputs. Their detailed findings live in `GIF_ANIMATION_AUDIT.md`.

- [x] GIFs were decomposed into contact sheets and frame metrics:
  Bump, poop cleanup, cure, discipline, family/route roster, Flag, food/snack,
  Get music, Heading, Hoops, lights, Memory, shop, and Sprint.
- [~] The GIF-derived frame families are represented by named contracts and
  partial firmware scenes. Medicine now has explicit bottle, dose, badge,
  recover, and refuse routing; not all props/branches across every GIF are
  complete reusable resources yet.
- [~] The GIF-derived scene phases are wired into the model/UI/game state
  machines with explicit per-mode pacing constants for food/snack, toilet,
  medicine, discipline, lights, family/friend, LINK, and catalog pages. Exact
  phase-by-phase source parity is still open.
- [ ] The GIF-derived high-motion scenes have not been validated against
  T-Echo-Lite synchronous partial-refresh pacing, ghosting, whitening, or
  button responsiveness.

## Current Acceptance Result

As of this checklist revision, EchoPet is not accepted as 100% Connect parity
yet. An ordinary `EchoPet` build has previously been flashed to one T-Echo-Lite,
and the current `EchoPet_LoRa` build was flashed on 2026-06-22 through
PlatformIO `nrfutil` serial DFU. The firmware now has the fixed shell/menu
frame, persistent state, table-driven growth/death/stat/game-reward tranches,
Connection-style food/care/shop/game scenes, an IR-visible link contract, Love /
partner / baby model flow, a 50-row character manifest, a 160-entry item segment
manifest, and the LilyGo-proven whole-frame partial refresh path. It still needs
exact source-backed shop/password/game/care tables plus T-Echo-Lite hardware and
two-device LoRa/Connection acceptance before it can be considered complete.
