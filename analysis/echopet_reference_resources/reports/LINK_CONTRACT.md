# EchoPet Link Contract

This file defines the boundary that lets EchoPet specify the infrared-style
player-visible LINK behavior first and then move the carrier to LoRa without
changing that behavior.

## Stable Gameplay Contract

- The gameplay message is `FriendPacket`.
- The stable receive entry point is `EchoPetModel::receiveFriendPacket()`.
- A successful receive may affect friendship, happiness, friend visit count,
  social growth score, memory flags, points, relation level, and gift items.
- A failed receive must not change pet state.
- A packet from the same local pet must be rejected.
- Duplicate/repeat friends are allowed, but receive a smaller reward than a new
  friend.
- When all 45 friend slots are full, the pet model replaces the weakest stored
  friend by relation, visits, and gifts. The transport must not choose the slot.
- Supported gameplay modes are VISIT, PRESENT, GAME, and LOVE.
- PRESENT may carry an exact catalog food/item id or points. Souvenir/memory
  rows are browsable but not transferable as gifts.
- GAME may carry a game kind and score; the receiver resolves the local result
  using the same pet model reward rules.
- LOVE carries the same friend identity packet and is resolved only by the pet
  model. It can promote a compatible adult relationship to partner/baby flow; the
  transport is not allowed to trigger next-generation state directly.
- `FriendPacket` v4 carries the sender's current `growthTier`, `adultTier`,
  `gender`, and `catalogId`. The model uses `adultTier` during compatible LOVE
  flow so the next generation can select its child tier from both parents
  instead of assuming a fixed normal partner; it uses elder-stage
  opposite-gender packets to preserve the special Oyajitchi lineage. The
  `catalogId` makes friend-list and family-adjacent views show the peer's
  concrete character-catalog identity instead of a generic archetype.
  Transport code must copy these bytes unchanged.
- Transport code must never award items, alter growth routes, change stats, or
  decide friend rewards directly.
- The transport layer is not considered complete until the player-visible
  standby, cancel, timeout, receive, result, and failure flows match the
  Connection behavior contract.
- Current player-visible flow:
  CONNECT -> GAME/PRESENT/VISIT/LOVE -> item/game selection if needed -> LINK
  STANDBY. In standby, B sends the selected packet, C cancels, and the session
  times out after 30 seconds. After a successful send, the SENT result screen
  keeps a 30-second receive window so the peer's matching packet can turn the
  result into RECEIVED. Packets outside standby or the SENT receive window are
  ignored as carrier noise.
- Current result states are SENT, RECEIVED, TIMEOUT, CANCEL, and FAILED.

## Current Source Enforcement

The current firmware enforces these parts of the contract in code:

- Receive is accepted only while the local UI is in LINK STANDBY or while the
  local SENT result screen is still inside the 30-second receive window.
- Incoming packets whose `FriendPacket.kind` does not match the current local
  LINK mode are rejected before `receiveFriendPacket()` is called, so mismatched
  Visit/Present/Game/Love packets cannot change pet state.
- A device waiting in standby emits a same-kind reply after a successful receive
  for Visit, Present, Game, and Love. This keeps the original sender from being
  stranded on SENT when both devices are participating.
- Passive standby replies are prepared without overwriting the receiver's result
  notice. The receiver can keep showing VISIT/GIFT/RESULT/PARTNER/BABY instead
  of being changed back to READY by the reply packet.
- Present standby replies first try the selected gift/points entry. If that
  entry is unavailable, the firmware falls back to a model-chosen Present packet
  so the visible link flow can still complete. Active player-initiated Present
  sends still fail with NO ITEM or NO PTS instead of silently falling back.
- Game standby replies first try the selected unlocked game. If that selected
  game is not available, the firmware falls back to a model-chosen game packet
  when possible. Active player-initiated Game sends still fail when the selected
  game is locked or unavailable.

## Current Development Carrier

The current example uses Serial text lines as a development carrier:

```text
EP:<hex FriendPacket>
```

`EchoPetLink` owns this debug line format. It is not pet behavior.

## Future Infrared Carrier

An infrared backend should transmit the exact `FriendPacket` bytes or a frame
that decodes losslessly to the same `FriendPacket`.

The IR backend may add preamble, pulse timing, framing bytes, retries, or CRCs,
but it must call `receiveFriendPacket()` at most once per accepted gameplay
packet.

## LoRa Carrier

The optional `EchoPet_LoRa` build enables an SX1262 LoRa backend. It preserves
the same gameplay payload and acceptance rules:

```text
LoRa frame -> FriendPacket -> receiveFriendPacket()
```

LoRa may add addressing, spreading factor, channel configuration,
acknowledgments, deduplication, encryption, or retries. None of those transport
details may change the reward curve or visible LINK behavior.

The expected player-visible result is:

```text
IR LINK result == LoRa LINK result
```

for the same valid `FriendPacket`.

## Required IR-Visible Specification

This section is the firmware contract for the Connection-style visible behavior.
It is written carrier-first only at the final boundary: Serial debug, infrared,
and LoRa must all expose the same player sequence and call the same model entry
points. Exact official infrared pulse timing remains out of scope here; visible
menu/result behavior is the thing that must stay stable.

Each LINK mode needs these fields:

- Entry path from the fixed Connection icon.
- A/B/C meaning on each screen.
- Local role selection, if any.
- Standby screen text/animation.
- Send trigger and send animation.
- Receive animation and accepted packet conditions.
- Timeout, cancel, failed, duplicate, and incompatible-pair results.
- Model effects after the visible result screen, not before.
- Exact result screen duration and return path.

## Shared Visible Sequence

| Stage | Screen | A button | B button | C button | Timeout | Model effect |
| --- | --- | --- | --- | --- | --- | --- |
| Entry | Fixed CONNECT icon | Next fixed icon before entry | Enter CONNECT menu | Status/back from home | none | none |
| Mode select | `CONNECT` choice list | Cycle `GAME/PRESENT/VISIT/LOVE` | Confirm mode | Back home | none | none |
| Optional select | Gift or game selector | Cycle item/game | Confirm selection | Back to CONNECT | none | none |
| Standby | `LINK STANDBY` with TX/RX animation | no-op | Send packet | Cancel | 30 s -> TIMEOUT | none before send |
| Sent window | `SENT` result with receive window | no-op | Back/keep result | Back | 30 s receive window | sender gift/points already committed once packet is emitted |
| Received | Result scene per mode | no-op | Back | Back | notice hold only | `receiveFriendPacket()` applies all receiver effects |
| Failed/cancel | FAILED/CANCEL/TIMEOUT scene | no-op | Back | Back | notice hold only | no receiver effect |

## Visit Flow

| Field | Contract |
| --- | --- |
| Entry | CONNECT -> VISIT -> LINK STANDBY. |
| Packet | `FriendPacket.kind = VISIT`; no gift/game fields. |
| Success scene | Two devices/pets with alternating send/receive waves and a heart. |
| Sender result | `SENT`; sender remains in reply window so the peer can respond. |
| Receiver result | `VISIT`; friend slot is added/updated, visits increment, relation may increase, happiness/social score increase. |
| Duplicate | Same friend updates relation/visit count with lower reward than first contact. |
| Full list | Model replaces the weakest friend by relation, visits, and gifts; transport never chooses the slot. |
| Failure | Invalid checksum, self packet, invalid stage/character/catalog id/relation, or wrong receive window -> FAILED with no model effect. |

## Present Flow

| Field | Contract |
| --- | --- |
| Entry | CONNECT -> PRESENT -> choose one of the 160 catalog rows or points -> LINK STANDBY. |
| Packet | `FriendPacket.kind = PRESENT`; `GiftKind::kCatalogItem` carries the exact catalog row in `giftId`, while `GiftKind::kPoints` carries points. Legacy coarse food/item gift fields remain accepted for compatibility only. |
| Selection scene | Gift box plus selected catalog icon/name, row number, stock/owned/no-send state, or points. |
| Sender debit | The selected catalog food/item or points are deducted only when the packet is prepared for send, not while browsing and not after cancel. Reusable items transfer by clearing the sender's owned bit; consumable food/items decrement the selected `catalogStock` row. |
| Receiver scene | Open gift box and happy pet; bottom result label is `GIFT`. |
| Receiver effect | Award the received exact catalog food/item or points, increment friend gifts, update relation/happiness/social score. |
| Edge cases | Missing local catalog stock/owned reusable item -> FAILED/NO ITEM; souvenir row selected -> FAILED/NO ITEM; insufficient points -> FAILED/NO PTS; receiver catalog food/item stock full or duplicate reusable item -> FULL/InventoryFull with no gift award; invalid gift kind/id -> friendship update only, no inventory award. |

## Connection Game Flow

| Field | Contract |
| --- | --- |
| Entry | CONNECT -> GAME -> choose one of the seven Connection game slots -> LINK STANDBY. |
| Packet | `FriendPacket.kind = GAME`; `gameKind` carries the Connection game id, not a local mini-game id; generated score is carried for model-only result resolution. |
| Selection scene | Shows `G.POINT`, `BALL`, `RC CAR`, `ROPE`, `BLDG`, `BALLOON`, and `TRUMPET`; item games show `READY` only when the local catalog has the required item class. |
| Success scene | Two pet faces and a central result marker; bottom result label is `RESULT`. |
| Receiver effect | Receiver must also own the required item for item-backed games; local score is generated, compared with peer score, points/happiness/play/social score are awarded by table. |
| Edge cases | Missing local item -> FAILED/NO ITEM with no friend/reward mutation; invalid game id -> packet reject; standby reply echoes the sender's game id so both devices resolve the same Connection game. |
| Carrier rule | LoRa/IR/Serial must not compute winners or rewards; only the model may resolve the result. |

## Love / Partner / Baby Flow

| Field | Contract |
| --- | --- |
| Entry | CONNECT -> LOVE -> LINK STANDBY. |
| Compatibility | Local sender must be adult-like; receiver only starts baby if receiver is adult-like and peer stage is adult-like. |
| Growth data | Compatible packets carry the peer's adult tier, gender, and catalog id; `startBaby()` stores local/peer adult tiers plus local/peer catalog identities before parent-care begins, and opposite-gender elder LOVE stores the Oyajitchi lineage flag. The peer catalog id is recorded in the friend slot and, after parent departure, in the family row for visible partner/friend identity. |
| Reject scene | Non-adult sender, invalid peer, self packet, or non-adult receiver -> FAILED/REJECT; no baby state. |
| Partner scene | Hearts around send/receive frame; relation is promoted to `PARTNER`. |
| Baby scene | If compatible, receiver enters `ParentCare`; result label is `BABY` and the family/baby composition is shown. |
| Parent-care effect | Parent and baby remain in parent-care state for 24 pet hours before `parentLeaves()` creates the next generation. |
| Next generation | Parent departure adds family history, increments generation, resets body/stats for the baby, and the child-stage transition uses the stored parent-pair adult tier table. Oyajitchi lineage instead forces a male baby and skips ordinary child/teen growth into adult Oyajitchi after the baby duration. |
| Dual-device gate | A single-device receive proves only the model path. Full acceptance requires two T-Echo-Lite devices showing matching send/receive/result states. |

## Two-Device Flow Matrix

| Flow | Required visible stages | Model effects | Current status |
| --- | --- | --- | --- |
| Visit | select VISIT, standby, send/receive, visit animation, result | friend slot, relation, visits, happiness/social score | CODED, DUAL pending |
| Present | select PRESENT, choose exact catalog food/item or points, standby, transfer animation, result | sender/receiver catalog inventory, gifts, relation | CODED, DUAL pending |
| Game | select GAME, choose G.POINT or an item-backed game, standby, paired game/result | points, relationship, game history | CODED, DUAL pending |
| Love reject | select LOVE, standby, incompatible/reject result | no baby, possible relation update | CODED, DUAL pending |
| Love partner | select LOVE, standby, compatible partner animation | partner/family state, relation | CODED, DUAL pending |
| Love baby | partner/baby sequence, baby appears, parent-care state | next generation preparation, save continuity | CODED, DUAL pending |
| Parent departure | parent leaves, baby remains, generation continues | generation increment, family history | CODED, HW pending |

## LoRa Equivalence Rule

LoRa-specific behaviors such as spreading factor, channel, retries,
acknowledgment, deduplication, and addressing are transport details. They may
improve reliability, but they must not change:

- The menu sequence.
- The required standby/cancel/timeout/result screens.
- The model effects of Visit, Present, Game, or Love.
- The reward, relationship, baby, or generation rules.
- The player's expectation of who sends first and what both devices display.

The LoRa build remains incomplete until every row in the two-device matrix has a
matching T-Echo-Lite hardware observation.
