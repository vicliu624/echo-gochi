# EchoPet Local GIF Animation Audit

Source directory: `C:\Users\vicliu\Projects\pet`

Generated audit artifacts:

- `analysis/gif_audit/overview.png`
- `analysis/gif_audit/summary.csv`
- `analysis/gif_audit/gif_frame_metrics.json`
- `analysis/gif_audit/contact_*.png`

These GIFs are treated as local concept references for player-visible behavior,
not as firmware-ready source art. The acceptance target is a reusable animation
contract: phases, frame names, anchors, local animated parts, input windows,
result states, and refresh pacing. A scene is not accepted just because the menu
exists or because a text/result page appears.

## Metric Summary

| Source | Size | Frames | Unique | Duration | Parity impact |
| --- | --- | ---: | ---: | ---: | --- |
| `BUMP GAME.gif` | 213x200 | 43 | 15 | 14780 ms | Missing force meter, opponent, push impact, fall/cry, and win sequence. |
| `Clean up the poop.gif` | 213x200 | 44 | 36 | 2640 ms | Cleanup must be a staged sweep/flush motion, not an instant state change. |
| `Cure when ill.gif` | 213x200 | 16 | 6 | 8000 ms | Cure needs sick symbol, dose/wait, refusal/failure, and recovery result. |
| `Discipline when needed.gif` | 213x200 | 11 | 5 | 6000 ms | Discipline must split time-out, praise, invalid, and attention-call cases. |
| `family.gif` | 1000x580 | 2 | 2 | 1000 ms | Roster/progression visual source for route silhouettes and family views. |
| `FLAG GAME.gif` | 213x200 | 13 | 6 | 13000 ms | Flag game needs visible left/right/both prompts, response pose, and result. |
| `food and snacks.gif` | 213x200 | 7 | 7 | 3500 ms | Food/snack needs item prop, bite/chew, crumbs/disappear, and state result. |
| `Get music.gif` | 213x200 | 11 | 11 | 13500 ms | Get game needs falling notes/bad items, catcher position, score, and miss. |
| `HEADING GAME.gif` | 213x200 | 65 | 30 | 10200 ms | Heading needs ball path, heading pose, miss/hit, and round pacing. |
| `HOOPS GAME.gif` | 213x200 | 29 | 29 | 3700 ms | Hoops needs hoop, ball arc, jump/shoot pose, score, and miss. |
| `Lights off at night!.gif` | 213x200 | 16 | 6 | 9000 ms | Lights need ON/OFF picker, sleep pose, dark-room state, and wake result. |
| `MEMORY GAME.gif` | 213x200 | 14 | 6 | 16000 ms | Memory needs sequence reveal, player replay, incorrect feedback, and GOOD. |
| `shop.gif` | 213x200 | 2 | 2 | 3000 ms | Shop needs booth/shopkeeper state, not just a catalog text page. |
| `SPRINT GAME.gif` | 213x200 | 33 | 23 | 14100 ms | Sprint needs race lane/finish, repeated-input runner motion, and result. |

## Required Scene Contracts

### Food And Snacks

Current status: coded projection exists; hardware acceptance open. The feed
action now exposes low-resource bite, crumb, refuse, and done frames, but exact
official-style art and e-paper pacing still need proof.

Required behavior:

- Selection frame: meal/snack icon or chosen food appears before the pet eats.
- Eating loop: food prop alternates with open/closed mouth; food can be
  positioned near the pet rather than baked into the full body bitmap.
- Consume result: prop disappears or turns into crumbs; stats, weight, fullness,
  refusal, and toothache risk are applied only through explicit result states.
- Resource contract: `food.icon.*`, `food.bite.0/1`, `food.crumbs`,
  `mouth.chew.0/1`, `food.refuse`, `food.done`.

### Poop Cleanup

Current status: coded projection exists; hardware acceptance open. The scene
now exposes mess, sweep, done, and no-mess frames with a reusable sweep prop.

Required behavior:

- Before state: mess is visible in the playfield with the pet reacting.
- Action state: cleanup tool/sweep enters as a separate prop and moves across
  the mess in multiple frames.
- After state: mess disappears, pet returns to normal/happy pose, and the
  no-mess branch remains visually distinct.
- Resource contract: `toilet.mess.0/1`, `toilet.tool.sweep.0..n`,
  `toilet.pet.react`, `toilet.done`, `toilet.no_mess`.

### Medicine / Cure

Current status: coded projection exists; hardware acceptance open. Illness and
toothache now have skull/tooth, dose, recovery, and refusal frames.

Required behavior:

- Sick state: skull/tooth or sickness symbol appears above/near the pet.
- Medicine state: dose prop and waiting/negative reaction are visible.
- Result state: skull/tooth clears on cure; refusal/failure remains distinct.
- Resource contract: `medicine.sick.skull`, `medicine.sick.tooth`,
  `medicine.dose.0/1`, `medicine.wait`, `medicine.recover`,
  `medicine.refuse`.

### Discipline / Attention

Current status: coded projection exists; hardware acceptance open. The branches
now include timeout, praise, invalid discipline, attention call, and missed-call
frame symbols.

Required behavior:

- Time-out and praise must be separate visible modes, not the same pose with
  different text.
- Attention-call, false-call, successful response, missed-call, and invalid
  discipline each need a visual result.
- Training/care-mistake updates must be attached to the result state.
- Resource contract: `discipline.prompt.timeout`,
  `discipline.prompt.praise`, `discipline.react.good`,
  `discipline.react.bad`, `discipline.invalid`, `attention.missed`.

### Lights / Sleep

Current status: coded, not hardware-accepted. The lights scene now has
proofable selector on/off, sleep/off, wake, and invalid frame families; the
dark-room refresh still needs physical panel acceptance.

Required behavior:

- Lights menu uses a visible ON/OFF selector.
- Sleep pose and Z symbol remain visible before dark-room transition.
- Dark-room rendering must be stable on the T-Echo-Lite panel and must not
  produce progressive whitening or an intrusive full-screen flash.
- Wake/invalid lights-off cases need visible feedback.
- Resource contract: `lights.selector.on`, `lights.selector.off`,
  `sleep.pose.0/1`, `sleep.z.0/1`, `lights.dark.room`,
  `lights.wake`, `lights.invalid`.

### Shop

Current status: coded, not hardware-accepted. The shop now has proofable
booth/counter, shopkeeper, item-preview, and buy-result frame families instead
of only a catalog text page.

Required behavior:

- Shop entry shows booth/front counter and shopkeeper.
- Browsing, affordable purchase, insufficient points, full stock, sold-out, and
  special/password reward states must be visually distinct.
- The shopkeeper should have at least neutral and surprised/happy expression
  frames.
- Resource contract: `shop.booth`, `shop.keeper.idle`,
  `shop.keeper.surprise`, `shop.keeper.happy`, `shop.item.preview`,
  `shop.buy.ok`, `shop.buy.no_money`, `shop.buy.full`, `shop.sold_out`.

### Family / Route Roster

Current status: incomplete. The GIF is a roster/progression concept rather than
a single gameplay animation.

Required behavior:

- The route catalog must expose more than placeholder archetypes: egg, baby,
  child, teen, adult, parent, baby/next-generation, and family-history views
  need named silhouettes.
- Partner/baby/parent departure must be visible in the connection/love flow.
- Resource contract: `family.roster.page`, `family.partner`,
  `family.baby`, `family.parent.depart`, `family.history.entry`.

### Games

Current status: coded projection exists; hardware acceptance open. The seven
games now have proofable low-resource props/frames for notes, bad items, force
meter, flags, heading ball, memory reveal/cursor, sprint runner, and hoops.

Required behavior by game:

- Get: falling note/good item, bad item, catcher position, score counter,
  catch/miss result, and round pacing.
- Bump: force meter, opponent sprite, push/impact pose, weight/power influence,
  lose fall/cry, and win text.
- Flag: left/right/both prompt icons, pet flag response poses, mistake window,
  GOOD/result feedback.
- Heading: ball descent, heading pose, hit/miss result, round progression.
- Memory: sequence reveal, player replay cursor, wrong input, GOOD/result.
- Sprint: race lane, finish marker, repeated-input runner poses, win/lose.
- Hoops: hoop, ball arc, jump/shoot pose, score counter, made/miss.

Resource contract:

- `game.get.note`, `game.get.bad`, `game.get.catch`, `game.get.miss`
- `game.bump.meter.*`, `game.bump.opponent`, `game.bump.push`,
  `game.bump.fall`, `game.bump.win`
- `game.flag.prompt.left/right/both`, `game.flag.pose.left/right/both`,
  `game.flag.good`, `game.flag.miss`
- `game.heading.ball.*`, `game.heading.hit`, `game.heading.miss`
- `game.memory.reveal.*`, `game.memory.cursor`, `game.memory.good`,
  `game.memory.wrong`
- `game.sprint.runner.*`, `game.sprint.finish`, `game.sprint.win`,
  `game.sprint.lose`
- `game.hoops.hoop`, `game.hoops.ball.*`, `game.hoops.shoot`,
  `game.hoops.made`, `game.hoops.miss`

## Renderer And Refresh Implications

- These scenes require layered sprites and prop overlays. Full-body bitmaps can
  still be used for true pose changes, but eyes, mouths, props, signs, balls,
  flags, notes, sweep tools, and meters must remain separately addressable when
  they are the only animated part.
- Animation state should be defined as named phases with durations and input
  windows. The game loop should not rely on hidden text state or a single static
  result frame.
- On e-paper, high-motion scenes must be authored as visible step animations
  with input pacing compatible with synchronous partial-refresh submission. A
  fast GIF cadence must be adapted to observable T-Echo-Lite frames without
  changing the player-visible rules.
- Dirty/redraw suppression must be evaluated per visible phase. Some GIFs have
  mostly local motion, while others intentionally perform a full scene change
  such as dark-room or shop booth.

## Acceptance Judgment

None of the GIF-derived gameplay/action contracts are fully accepted in the
current firmware. The current implementation has menu reachability, model state,
and some graphical scenes, but it still lacks the specific prop-level animation,
branch feedback, game pacing, shop presentation, and family/route visuals shown
by these concept references.
