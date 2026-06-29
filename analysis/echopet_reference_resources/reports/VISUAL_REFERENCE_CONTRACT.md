# EchoPet Visual Reference Contract

Target: the default validation pet should visually track the Tamagotchi
Connection official presentation closely enough for movement, scale, placement,
and local animation to be verified on hardware.

Official reference pages:

- How-to / animation reference:
  `https://tamagotchi-official.com/gb/series/connection/howto/`
- Character list:
  `https://tamagotchi-official.com/gb/series/connection/character/`
- Mametchi reference:
  `https://tamagotchi-official.com/gb/series/connection/character/219/`

Legal/resource boundary:

- Official images are reference material for silhouette, scale, timing, and
  required animation states.
- Do not mechanically copy, redistribute, or embed official website images in
  this repository unless rights-cleared assets are supplied by the user.
- The firmware resource model must remain able to swap in a rights-cleared
  sprite sheet later without changing gameplay code.

## Required Frame Families

Each pet resource set must provide named frame families. A future pet can replace
the default pet by drawing these frames and reusing the same anchors.

Status legend:

- `[x]` implemented as named frame family
- `[~]` partly represented, needs better official-reference alignment
- `[ ]` missing

## Official How-To Audit Groups

The official how-to page is treated as a visual checklist for player-visible
motion groups:

- Nurture menus: status, food/snack, poop cleanup, discipline, medicine, lights.
- Character growth: egg, baby, child, teen, adult, parent/next generation.
- Activities: Get, Bump, Flag, Heading, Memory, Sprint, Hoops.
- Collection: shop/items, password rewards, souvenirs.
- Connection: Visit, Present, Game, relationship/love, next generation.

These groups must be represented by original or rights-cleared low-resource
assets. Official website images remain reference material only unless the user
supplies a license-cleared asset set.

Generated metadata audit:

- `analysis/official_howto_audit/generate_official_howto_audit.py`
- `analysis/official_howto_audit/OFFICIAL_HOWTO_VISUAL_AUDIT.md`
- `analysis/official_howto_audit/summary.csv`
- `analysis/official_howto_audit/official_howto_asset_metrics.json`
- `analysis/parity_coverage/generate_parity_resource_coverage.py`
- `analysis/parity_coverage/parity_resource_coverage.md`
- `analysis/sprite_render_audit/generate_sprite_render_audit.py`
- `analysis/sprite_render_audit/SPRITE_RENDER_AUDIT.md`
- `analysis/sprite_render_audit/sprite_frame_metrics.csv`

The generated audit stores URL, grouping, dimensions, frame count, unique-frame
count, duration, and motion bounding boxes. It intentionally does not store
official image pixels or contact sheets.

Generated firmware sprite render audit:

- The host-side renderer parses `EchoPetSprites.cpp`, `EchoPetSprites.h`, and
  `EchoPetDisplay.cpp`, then rasterizes the same 1-bit bitmap assets,
  `FramePart` anchors, mirror flags, and `connectFrame()` mappings used by the
  firmware.
- Current result: 101 `SpriteFrame` enum rows render, no enum rows are missing a
  composed-frame mapping, and no rendered pixels fall outside each frame's
  nominal bounding box.
- Contact sheets live under `analysis/sprite_render_audit/`:
  `sprite_contact_sheet.png`, `sprite_egg_hatch.png`,
  `sprite_mametchi_motion.png`, `sprite_care_scenes.png`, and
  `sprite_game_scenes.png`.
- This proves source-side sprite composition and anchor sanity. It does not
  prove e-paper refresh speed, whitening/ghosting, official-look final art, or
  true hardware placement.

Current official how-to metadata coverage:

- `nurture`: 8 assets, 5 animated; max 44 frames.
- `growth`: 3 assets, 1 animated; max 2 frames.
- `game`: 9 assets, 7 animated; max 65 frames.
- `item_shop`: 7 assets, 3 animated; max 48 frames.
- `password_secret`: 6 assets, 2 animated; max 58 frames.
- `connect`: 9 assets, 7 animated; max 63 frames.

Character catalog routing:

- The firmware carries a 50-row character catalog with source page/slot,
  route, stage, care tier, `frameFamily`, and `visualTraits` metadata.
- Idle main-scene rendering now uses
  `EchoPetCharacterVisuals.cpp` source-backed 24x24 per-catalog bitmap rows
  selected by concrete `catalogId`, while FRIEND/FAMILY/catalog portraits keep
  the 16x16 per-catalog rows for dense list contexts. Action scenes such as feeding, toilet,
  medicine, link, games, and pass-away still use the named `SpriteFrame`
  contracts.
- The main scene still applies each row's `visualTraits` byte as an original
  low-resource overlay only when the high-bit complete-bitmap marker is clear.
  The current 50 catalog rows set that marker, so the old programmatic
  head/side/face overlays no longer distort official-derived character bitmaps.
  The 9 `frameFamily` silhouettes remain as fallback resource families, but
  normal idle portraits no longer depend on them as the primary visual identity.
- `analysis/official_reference/generated/character_official_alignment_proof.png`
  compares the official 50-row character reference sheet against the current
  source-rendered character proof. It remains the human review surface for
  future replacement packs, while the current generated source set is tracked
  by the per-row status JSON below.
- `analysis/official_reference/generated/character_generated_source_contact_sheet.png`
  previews the 50 runtime C/C++ 16x16 portrait rows generated from
  analysis-only official references. `character_idle_generated_source_contact_sheet.png`
  previews the matching 24x24 idle rows used on the main screen. Source-side
  character visual acceptance is closed by
  `character_source_alignment_status.json`, which currently reports 50
  `official-derived` rows and zero review flags; T-Echo-Lite readability still
  requires hardware proof.
- `analysis/official_reference/generated/character_source_alignment_status_contact_sheet.png`
  is the per-character replacement-art surface: each row shows the official
  reference thumbnail beside the generated 16x16 and 24x24 C++ rows, with
  `character_source_alignment_status.json` recording ink/bounding-box metrics
  and review flags. A future character set must preserve these row definitions
  and replace the C++ rows deliberately rather than changing gameplay code.
- `analysis/resource_replacement_manifest/character_replacement_manifest.json`
  and `.csv` expose the same 50-row character replacement contract in a
  machine-readable form: catalog row id, stage, growth route, source page/slot,
  generation/tier/gender masks, 16x16 portrait bitmap symbol, 24x24 idle bitmap
  symbol, current official-derived status, and review flags. Current counts are
  50 official-derived rows, 50 portrait rows, 50 idle rows, and zero review
  flags.

Item catalog routing:

- The firmware carries a 160-row catalog with kind, behavior, price, icon,
  source group, flags, use-scene, and `visualTraits` metadata.
- The ITEM page now uses the selected 160-row catalog index directly, so the
  displayed item and the item used by B/confirm no longer diverge.
- Shop and item previews use `CatalogEntry::icon`; item-use feedback uses
  `CatalogEntry::useScene` to choose a compact action composition.
- Catalog preview/use-scene rendering applies `CatalogEntry::visualTraits` as
  a low-resource edge/detail overlay so every row has a stable replacement-art
  hook before final hand-drawn assets exist.
- Catalog preview rendering now has a 160-row `kCatalogEntryBitmaps` C/C++
  source table in `EchoPetCatalogVisuals.cpp`, generated by
  `analysis/screen_simulator/generate_catalog_entry_source_arrays.py`. The
  runtime preview path passes the concrete catalog index to
  `drawCatalogEntryBitmap(...)` instead of depending only on the older shared
  41-family `icon` field.
- Twelve unambiguous item rows are now official-reference-backed source
  overrides generated from official how-to item icon sheets: PENCIL rows 48/112,
  CAP rows 58/122, SHOVEL rows 54/118, BALL row 60, BALLOON rows 56/63/120,
  and TRUMPET rows 84/141.
  The PC and rabbit official icons are not mapped because the current catalog
  has no unambiguous equivalent row for those concepts.
- Souvenir browsing uses `drawSouvenirIcon(...)` and the 64 source-backed
  per-memory bitmap rows in `EchoPetCatalogVisuals.cpp`, with
  `catalogSouvenirVisualTraits(index)` kept for found/unknown page metadata.
- Souvenir/MEMORY rows now have an official-reference semantic proof chain:
  `analysis/official_reference/generate_souvenir_reference_sheets.py` groups
  official how-to GIF/PNG candidates by souvenir row, writes the 64-row status
  manifest, and maintains the generated C/C++ `kSouvenirMemoryBitmaps` table.
  TRUMPET and BALL PLAY are unambiguous official item-icon source overrides;
  the remaining rows use official scene semantics where available, but the
  cached official site material is not a complete 64-row souvenir atlas.
- These routes are replacement contracts, not final per-item artwork. The
  current 160-row table preserves the previous low-resource proof as explicit
  per-row source data where no official-source override exists, so future
  replacement is row-local. Exact
  per-item/per-souvenir Connection-style icons and props still require final
  drawing/acceptance.
- `analysis/resource_replacement_manifest/` is the machine-readable replacement
  surface for this remaining art work. It generates JSON, CSV, and Markdown
  manifests from the current C++ truth: 160 catalog rows and 64 souvenir rows,
  all with stable row IDs, 16x16 runtime bitmap dimensions, source status,
  use-scene metadata, and a no-runtime-raster replacement contract. Current
  counts are 12 official-source catalog rows and 2 official-source souvenir
  rows; the remaining 148 catalog rows and 62 souvenir rows need accepted final
  art or rights-cleared replacement rows.
- `analysis/resource_drawing_spec/` is the row-local drawing brief for that
  remaining work. It generates JSON, CSV, and Markdown specs for all 160 catalog
  rows and 64 souvenir rows, including the object/semantic family to draw, the
  runtime C++ bitmap symbol, the 16x16 row encoding, preservation rules, and
  the native/compact-scene recognizability acceptance rule. This spec makes
  future user-drawn replacement art precise, but it does not mark substitute
  rows as final artwork.
- `analysis/official_reference/generated/catalog_item_official_alignment_proof.png`
  compares the official item how-to assets and animation frames against the
  current `analysis/screen_simulator/out/catalog_items_proof_x*.png` source
  rendering. This proof is the current review surface for deciding whether each
  catalog-row drawing has reached the official reference semantics. The proof
  does not make official PNG/GIF files runtime resources.
- `analysis/official_reference/generated/catalog_item_source_semantic_status_contact_sheet.png`
  lists the 152 non-souvenir catalog rows, their current C++ source icon, and
  whether the row is an official-source C++ override or still a section-level
  official scene reference with substitute art.
- The detected official item LCD tile sheet currently contains 18 representative
  crops from the official item PC GIF. It is a style and semantics reference,
  not evidence that the 160 EchoPet catalog rows have final per-row artwork.

Family history routing:

- The FAMILY page consumes compatibility `FamilyRecord` snapshot rows plus
  `FamilyAncestryRecord` parent/partner/baby/tier source rows instead of
  rendering a text-only list or guessing baby identity from generation parity.
- `drawFamilyAlbumCard(...)` draws a compact album composition for empty and
  populated history slots: parent portrait, known LOVE partner portrait when a
  v8+ family row or ancestry row stores the peer catalog id, unknown-partner
  silhouette for old or Matchmaker rows, heart, next-generation baby marker,
  child-tier/source-line marker, and care-score pips.
- Friend-list and FAMILY portraits now use the 50-row character catalog's
  per-catalog bitmap rows plus `visualTraits` metadata rather than
  seed-generated generic faces. `FriendPacket` v4 carries peer `catalogId` so
  new friend records and LOVE-created family records can preserve the concrete
  visual identity seen through LINK.
- This is the low-resource visible replacement contract for family history.
  Exact Love/partner/baby/parent-departure official-look scenes still require
  final original or rights-cleared art and two-device validation.

Fixed menu shell:

- Only the 10 Connection-style fixed icons are rendered in the side menu zones:
  health, food, toilet, connection, attention/care, discipline, medicine,
  lights, friends, and game/activity.
- `EchoPetMenuIconResources.cpp` owns the fixed-menu semantic table and both
  C/C++ packed bitmap sets: 12x12 compact rows and 30x30 large rows.
- The larger layout draws those 30x30 rows directly inside the 32-pixel side
  columns.
- The compact 128x64 layout draws the 12x12 rows directly inside the compact
  rail. The compact rows must remain derived from the same official slot source
  semantics as the 30x30 rows. Current compact and large rows both use the
  official manual device/menu tile crop for each slot; the center-screen tiny
  manual icons are analysis references only and are no longer mixed into the
  runtime side-rail icon set. A future replacement may be hand-drawn later, but
  it cannot change what the slot means or use a different icon concept.
- The 12x12 and 30x30 resources must share the same ten-menu semantic order:
  health, food, toilet, connection, attention/care, discipline, medicine,
  lights, friends, and game/activity.
- `analysis/screen_simulator/out/fixed_menu_icons_proof_x*.png` renders both
  sizes in one proof sheet for regression review.
- `analysis/official_reference/` stores official reference material outside the
  firmware runtime path. `generated/fixed_menu_official_alignment_proof.png`
  compares the current source icons against official manual callouts, and
  `fixed_menu_official_alignment_manifest.json` reports `PASS`.
- Current fixed-menu drawings are generated from analysis-only official manual
  tile crops into C/C++ bitmap rows by
  `analysis/official_reference/generate_menu_icon_source_arrays.py`. The
  generated `fixed_menu_source_policy_contact_sheet.png` and JSON prove that
  12x12 and 30x30 rows share one official device/menu tile source per slot.
  Source-side fixed-menu recognizability is closed by the manifest; hardware
  smoke testing still decides whether the selected slot, entry action, result,
  and exit feel correct on the e-paper panel. Other official how-to decoration
  icons are not valid substitutes for these ten fixed menu slots unless they
  explicitly carry the same fixed-menu semantics.

Runtime pacing adaptation:

- Official GIF frame timing is source evidence for motion density, not a direct
  e-paper frame deadline.
- `EchoPet.ino` maps high-motion games to faster observable steps: Heading,
  Hoops, Bump, and Sprint at 250 ms, Get and Flag at 260 ms, and Memory at
  300 ms.
- Care/action scenes use named pacing constants: food/snack 360 ms, toilet
  420 ms, medicine 500 ms, discipline 520 ms, lights 720 ms, family/friend
  520 ms, LINK 360 ms, and catalog/shop/item/souvenir/point pages 420 ms.
- Hardware acceptance still decides whether these values are readable on the
  T-Echo-Lite panel without whitening or missed/stale frames.

## Local GIF Concept Inputs

The local concept files are now copied into
`analysis/echopet_reference_resources/concept_design_gifs`. They originated
from `C:\Users\vicliu\Projects\pet` and are treated as concept animation
contracts when they expose missing or weak behavior. Detailed audit results are
recorded in `GIF_ANIMATION_AUDIT.md`; generated contact sheets and metrics are
under `analysis/gif_audit/`.

Covered local inputs:

- `food and snacks.gif`
- `Clean up the poop.gif`
- `Cure when ill.gif`
- `Discipline when needed.gif`
- `Lights off at night!.gif`
- `shop.gif`
- `family.gif`
- `character.png`
- `Get music.gif`, `BUMP GAME.gif`, `FLAG GAME.gif`, `HEADING GAME.gif`,
  `MEMORY GAME.gif`, `SPRINT GAME.gif`, `HOOPS GAME.gif`

These files add a stronger requirement than "show a sprite": action screens
need staged props, local feature animation, visible branch feedback, and
game-specific scene objects.

## Life Stages

- [~] `egg.idle.0`, `egg.idle.1`
  - Current firmware uses a centered 3x3 low-resource egg body with eyes and
    mouth composited inside the shell. This fixes the earlier proof defect where
    the face tiles were below the egg.
- [~] `egg.crack.0`, `egg.crack.1`, `egg.hatch`
  - Low-resource composed frames now exist and are reachable from setup hatch,
    egg idle age progression, and sprite proof. Exact official art is still a
    reference contract, not embedded firmware pixels.
- [~] `baby.idle.0`, `baby.idle.1`
- [~] `child.idle.0`, `child.idle.1`
- [~] `teen.idle.0`, `teen.idle.1`
- [~] `adult.mametchi.idle.0`, `adult.mametchi.idle.1`
- [~] `elder.idle.0`, `elder.idle.1`

## Local Feature Layers

- [x] `eye.open.left/right`
- [x] `eye.blink.left/right`
- [x] `eye.sleep.left/right`
- [x] `eye.sad.left/right`
- [x] `eye.sick.left/right`
- [x] `mouth.smile`
- [x] `mouth.open`
- [x] `mouth.sad`
- [x] `foot.left/right`
- [x] `emotion.heart`, `emotion.sweat`, `emotion.z`
- [x] `emotion.angry`, `emotion.question`, `emotion.music`
- [~] `prop.food.*`, `prop.crumbs`, `prop.medicine`, `prop.skull`,
  `prop.tooth`, `prop.sweep`, `prop.flag.left/right`, `prop.ball`,
  `prop.note`, `prop.bad_item`, `prop.hoop`, `prop.shop_counter`,
  `prop.force_meter`
  - Reusable low-resource parts now exist in firmware for crumbs, mess, sweep,
    skull, tooth, flag, note, hoop, force meter, baby, ball, medicine, food,
    and expression marks. Per-item official-look art remains incomplete.
- [x] `shop.booth/counter`
- [x] `shopkeeper.face.idle/surprise/happy`
- [x] `shop.item.preview`
- [x] `shop.buy.ok/no_money/full/sold_out`

## Care / Action Frames

- [~] `idle.walk.left`, `idle.walk.right`
- [~] `happy`
- [~] `sad`
- [~] `sick`
- [~] `sleep`
- [~] `friend`
  - Friend-list portraits now render from peer `catalogId` carried in
    `FriendPacket` v4, with archetype fallback for older stored rows. Final
    official-look friend sprites and two-device proof remain open.
- [x] `eat.meal.0`, `eat.meal.1`
- [x] `eat.snack.0`, `eat.snack.1`
- [x] `toilet.before`, `toilet.after`
- [x] `medicine.take.0`, `medicine.take.1`
- [x] `lights.on`
- [x] `lights.off`
- [x] `lights.selector.on/off`
- [x] `lights.wake`
- [x] `lights.invalid`
- [x] `discipline.timeout`
- [x] `discipline.praise`
- [x] `attention.call`
- [x] `item.play`
- [x] `connection.send`
- [x] `connection.receive`
- [x] `game.win`
- [x] `game.lose`
- [x] `passaway`
- [~] `food.select.meal/snack`, `food.bite.0/1`, `food.crumbs`,
  `food.refuse`, `food.done`
  - Food/Snack now has scene-level prop routing in addition to full-body frames:
    selected food, plate, bite travel, crumbs, done check/heart, and refusal
    cross. Exact Connection bite count and final official art remain open.
- [~] `toilet.mess.0/1`, `toilet.tool.sweep.0..n`,
  `toilet.pet.react`, `toilet.done`, `toilet.no_mess`
  - Toilet now has scene-level cleanup routing in addition to full-body frames:
    visible messes, sweep tool, progressive removal, clean slot, done
    heart/sparkle, and no-mess question branch. Exact Connection timing and
    final official art remain open.
- [~] `medicine.sick.skull`, `medicine.sick.tooth`, `medicine.dose.0/1`,
  `medicine.wait`, `medicine.recover`, `medicine.refuse`
  - Medicine now has scene-level prop routing in addition to full-body frames:
    a medicine bottle, sick/tooth/cured/refuse badge, animated dose trail,
    recovery pose, and refusal pose. Exact Connection timing and final official
    art remain open.
- [~] `discipline.prompt.timeout`, `discipline.prompt.praise`,
  `discipline.react.good`, `discipline.react.bad`, `discipline.invalid`,
  `attention.missed`
- [~] `lights.selector.on`, `lights.selector.off`, `sleep.pose.0/1`,
  `sleep.z.0/1`, `lights.dark.room`, `lights.wake`, `lights.invalid`
  - Lights on/off, selector on/off, wake, and invalid now have separate
    proofable pet frames. The dark-room scene now uses a low-ink border/hatch
    pattern with moon/star marks instead of a full-width black fill; final
    e-paper stability remains hardware proof.
- [~] `shop.booth`, `shop.keeper.idle`, `shop.keeper.surprise`,
  `shop.keeper.happy`, `shop.item.preview`, `shop.buy.ok`, `shop.buy.no_money`,
  `shop.buy.full`, `shop.sold_out`
  - The booth/counter now has proofable `SpriteFrame::kShopBooth` coverage, and
    the shopkeeper, item-preview, and buy-result states have proofable
    `SpriteFrame` rows. Exact per-item art and hardware-readable result timing
    still need proof.
- [~] `family.roster.page`, `family.partner`, `family.baby`,
  `family.parent.depart`, `family.history.entry`
  - FAMILY history cards now draw catalog-derived parent/baby avatars and an
    exact known LOVE partner avatar when `FamilyRecord` carries the partner
    catalog sentinel. Old/Matchmaker rows still use an unknown-partner
    silhouette; Love/parent-departure hardware proof remains open.
- [~] `game.get.note`, `game.get.bad`, `game.get.catch`, `game.get.miss`
- [~] `game.bump.meter.*`, `game.bump.opponent`, `game.bump.push`,
  `game.bump.fall`, `game.bump.win`
- [~] `game.flag.prompt.left/right/both`, `game.flag.pose.left/right/both`,
  `game.flag.good`, `game.flag.miss`
- [~] `game.heading.ball.*`, `game.heading.hit`, `game.heading.miss`
- [~] `game.memory.reveal.*`, `game.memory.cursor`, `game.memory.good`,
  `game.memory.wrong`
- [~] `game.sprint.runner.*`, `game.sprint.finish`, `game.sprint.win`,
  `game.sprint.lose`
- [~] `game.hoops.hoop`, `game.hoops.ball.*`, `game.hoops.shoot`,
  `game.hoops.made`, `game.hoops.miss`
  - These families now have proofable low-resource frame symbols and selected
    scene wiring. Exact official cadence, per-round result staging, and
    hardware readability remain acceptance work.

Current implemented frame symbols:

- `SpriteFrame::kEgg0`, `SpriteFrame::kEgg1`,
  `SpriteFrame::kEggCrack0`, `SpriteFrame::kEggCrack1`,
  `SpriteFrame::kEggHatch`
- `SpriteFrame::kEatMeal0`, `SpriteFrame::kEatMeal1`
- `SpriteFrame::kEatSnack0`, `SpriteFrame::kEatSnack1`
- `SpriteFrame::kFoodCrumbs`, `SpriteFrame::kFoodRefuse`,
  `SpriteFrame::kFoodDone`
- `SpriteFrame::kToilet0`, `SpriteFrame::kToilet1`
- `SpriteFrame::kToiletMess`, `SpriteFrame::kToiletSweep0`,
  `SpriteFrame::kToiletSweep1`, `SpriteFrame::kToiletDone`,
  `SpriteFrame::kToiletNoMess`
- `SpriteFrame::kMedicine0`, `SpriteFrame::kMedicine1`
- `SpriteFrame::kMedicineSickSkull`, `SpriteFrame::kMedicineSickTooth`,
  `SpriteFrame::kMedicineDose0`, `SpriteFrame::kMedicineDose1`,
  `SpriteFrame::kMedicineRecover`, `SpriteFrame::kMedicineRefuse`
- `SpriteFrame::kLightsOn`, `SpriteFrame::kLightsOff`
- `SpriteFrame::kLightsSelectorOn`, `SpriteFrame::kLightsSelectorOff`,
  `SpriteFrame::kLightsWake`, `SpriteFrame::kLightsInvalid`
- `SpriteFrame::kDisciplineTimeout`, `SpriteFrame::kDisciplinePraise`,
  `SpriteFrame::kDisciplineInvalid`
- `SpriteFrame::kAttentionCall`, `SpriteFrame::kAttentionMissed`,
  `SpriteFrame::kItemPlay`
- `SpriteFrame::kShopBooth`
- `SpriteFrame::kShopkeeperIdle`, `SpriteFrame::kShopkeeperSurprise`,
  `SpriteFrame::kShopkeeperHappy`
- `SpriteFrame::kShopItemPreview`, `SpriteFrame::kShopBuyOk`,
  `SpriteFrame::kShopBuyNoMoney`, `SpriteFrame::kShopBuyFull`,
  `SpriteFrame::kShopSoldOut`
- `SpriteFrame::kLinkSend`, `SpriteFrame::kLinkReceive`
- `SpriteFrame::kLovePartner`, `SpriteFrame::kLoveBaby`,
  `SpriteFrame::kParentDepart`
- `SpriteFrame::kGameWin`, `SpriteFrame::kGameLose`
- `SpriteFrame::kGameGetNote`, `SpriteFrame::kGameGetBad`,
  `SpriteFrame::kGameGetCatch`, `SpriteFrame::kGameGetMiss`
- `SpriteFrame::kGameBumpMeter`, `SpriteFrame::kGameBumpPush`,
  `SpriteFrame::kGameBumpFall`
- `SpriteFrame::kGameFlagLeft`, `SpriteFrame::kGameFlagRight`,
  `SpriteFrame::kGameFlagBoth`, `SpriteFrame::kGameFlagGood`,
  `SpriteFrame::kGameFlagMiss`
- `SpriteFrame::kGameHeadingBall`, `SpriteFrame::kGameHeadingHit`,
  `SpriteFrame::kGameHeadingMiss`
- `SpriteFrame::kGameMemoryReveal`, `SpriteFrame::kGameMemoryCursor`,
  `SpriteFrame::kGameMemoryGood`, `SpriteFrame::kGameMemoryWrong`
- `SpriteFrame::kGameSprintRunner0`, `SpriteFrame::kGameSprintRunner1`,
  `SpriteFrame::kGameSprintFinish`
- `SpriteFrame::kGameHoopsHoop`, `SpriteFrame::kGameHoopsShoot`,
  `SpriteFrame::kGameHoopsMade`, `SpriteFrame::kGameHoopsMiss`
- `SpriteFrame::kPassed`

## Replaceable Sprite Metadata Contract

Every future pet resource set must provide this metadata before it is considered
drawable. The renderer may still store pixels as compact 1-bit parts, but the
author-facing contract is frame-based so a new pet can be drawn without changing
gameplay code.

| Frame family | Anchor | Layers | Mirror | Duration |
| --- | --- | --- | --- | --- |
| `egg.idle.0/1` | bottom-center | shell, crack optional | no | 900 ms |
| `egg.crack.0/1`, `egg.hatch` | bottom-center | shell, crack, pop | no | 700 ms |
| `baby.idle.0/1` | feet-center | body, eyes, mouth, feet | yes | 900 ms |
| `child.idle.0/1` | feet-center | body, eyes, mouth, feet | yes | 900 ms |
| `teen.idle.0/1` | feet-center | body, eyes, mouth, feet | yes | 900 ms |
| `adult.*.idle.0/1` | feet-center | body, eyes, mouth, feet | yes | 900 ms |
| `elder.idle.0/1` | feet-center | body, eyes, mouth, feet | yes | 1100 ms |
| `happy`, `sad`, `sick`, `sleep` | feet-center | body, expression, symbol | yes | 1200 ms |
| `eat.meal.0/1`, `eat.snack.0/1` | feet-center | body, mouth, food prop | yes | 700 ms |
| `toilet.before/after` | feet-center | body, mess prop, cleanup prop | yes | 700 ms |
| `medicine.take.0/1` | feet-center | body, med prop, sparkle/error | yes | 700 ms |
| `lights.off` | feet-center | body, sleep eyes, Z symbol | yes | 1200 ms |
| `discipline.timeout`, `discipline.praise` | feet-center | body, face, board/heart | yes | 900 ms |
| `attention.call` | feet-center | body, alert symbol | yes | 700 ms |
| `item.play` | feet-center | body, held/near prop | yes | 900 ms |
| `connection.send/receive` | feet-center | body, signal/heart prop | yes | 700 ms |
| `game.win/lose` | feet-center | body, expression, result prop | yes | 900 ms |
| `love.partner`, `love.baby`, `parent.depart` | feet-center | body, partner/baby prop | yes | 1200 ms |
| `passaway` | center | memory marker | no | static |
| `food.select/bite/crumbs/refuse/done` | feet-center | body, mouth, food prop, plate, crumbs, done mark, refusal cross | yes | 500-900 ms |
| `toilet.mess/sweep/done/no_mess` | playfield/world | body, mess prop, sweep/flush prop, progressive removal, clean slot, no-mess branch | no | 80-700 ms |
| `medicine.sick/dose/wait/recover/refuse` | feet-center | body, sick symbol, medicine prop, badge, dose trail, expression | yes | 500-1200 ms |
| `discipline.prompt/react/invalid/missed` | feet-center | body, prompt text/sign, expression symbol | yes | 700-1500 ms |
| `lights.selector/sleep/dark/wake/invalid` | playfield/world | body, selector text, Z, low-ink dark-room pattern, moon/star marks, wake/invalid mark | no | 900-1500 ms |
| `shop.booth/keeper/item/buy.*` | playfield/world | booth, shopkeeper face, item preview, result text | no | 1000-1500 ms |
| `family.roster/partner/baby/depart/history` | playfield/world | one or more characters, baby prop, history marker | no | 1000-1500 ms |
| `game.get.*` | playfield/world | catcher/body, notes, bad item, score, result | no | 500-1500 ms |
| `game.bump.*` | playfield/world | force meter, player, opponent, impact, result | no | 100-1500 ms |
| `game.flag.*` | feet-center | body, left/right flags, prompt, result text | yes | 700-1500 ms |
| `game.heading.*` | playfield/world | player, ball path, hit/miss result | no | 100-700 ms |
| `game.memory.*` | feet-center | body, prompt symbols, cursor, result text | yes | 1000-1500 ms |
| `game.sprint.*` | playfield/world | runner poses, lane, finish marker, result | no | 100-700 ms |
| `game.hoops.*` | playfield/world | hoop, ball arc, shooter pose, score, result | no | 100-700 ms |

Frame authoring rules:

- Anchors must be stable across frames in the same family. Eye or mouth motion
  must not shift the body anchor.
- Eyes, mouth, feet, and props should remain replaceable local parts where
  possible. Do not bake every expression into a full-body bitmap unless the
  shape truly changes.
- A frame can reuse another frame only if the player-visible pose is intended to
  be identical. Reuse must be documented by frame name, not hidden in code.
- The 64x64 and 128x128 playfields may use the same logical frame names, but
  each resource profile may choose different scale, crop, or detail level.

## Default Adult Mametchi-Like Alignment

Acceptance points for the default adult validation character:

- [~] Head/body silhouette occupies roughly the same proportion as the official
  Mametchi reference when rendered in the center playfield.
- [~] Top twin-ear/twin-horn shape is visible at 1x and 2x scale.
- [x] Eyes are large, square, separately layered, and can blink independently of
  the body bitmap.
- [x] Feet are small local parts and shift between idle frames.
- [x] Mouth is separately layered and can change smile/open/sad without redrawing
  the whole body.
- [~] Idle movement is side-to-side plus local foot/bob movement, not a static
  sprite. Exact official-look silhouette and hardware placement remain open.

Code-side layering proof:

- `SpriteFrame::kAdult0/kAdult1` route to composed `kFrameStand0/1` parts, not
  the old tile-cell fallback.
- The base body is a 32x32 bitmap before 64x64 or 128x128 display scaling.
- Eyes, mouth, and feet are local parts with explicit anchors; blink, sleep,
  sad, sick, smile, open mouth, and sad mouth states are separate parts.
- `kFrameStand0Parts` and `kFrameStand1Parts` move the feet instead of
  redrawing a whole sprite, so future pet resources can replace the same named
  part contract.
- `analysis/sprite_render_audit/SPRITE_RENDER_AUDIT.md` now verifies all 101
  firmware `SpriteFrame` rows render into contact sheets with zero missing
  mappings and zero pixels outside their nominal frame.
- Mametchi uses an explicit `kMametchiValidationTraits = 0x80` row so the
  default validation character is not distorted by the generic source-page /
  source-slot overlay used to make the broader 50-row catalog visually distinct.
- `analysis/parity_coverage/parity_resource_coverage.md` now reports this as
  `Default Adult Mametchi-Like Layering`.

## Screen Resource Requirement

- `EchoPetResources64.*` must keep the `64x64` center playfield.
- `EchoPetResources128.*` must keep the `128x128` center playfield.
- The shell icons use true 1-bit bitmap resources at both required sizes in
  `EchoPetMenuIconResources.cpp`: 12x12 compact and 30x30 large. They must
  preserve menu semantics across sizes instead of relying on lossy automatic
  projection.
- Pet frames are authored in a compact native size, then scaled by the display
  resource profile. Do not duplicate every sprite solely for screen size unless
  the silhouette cannot remain readable after scaling.
