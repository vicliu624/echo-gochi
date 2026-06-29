# GAT562 Compact V3 Function Animation Report

Date: 2026-06-30

Scope: GAT562 128x64 compact EchoPet UI. The left fixed-menu rail remains the
entry selector. The right playfield is treated as a Tamagotchi Connection V3
LCD-style projection: 32x30 native pixels, rendered at 2x inside the 96x64
main area.

## Sources

- Official Connection how-to page:
  `https://tamagotchi-official.com/gb/series/connection/howto/`
- Official/Bandai Connection instruction manual cache:
  `analysis/official_reference/raw/manuals/bandai_connection_2024_instruction_manual.pdf`
- Official how-to metadata and local frame contract:
  `analysis/official_howto_audit/OFFICIAL_HOWTO_VISUAL_AUDIT.md`
- Local GIF animation contract:
  `analysis/echopet_reference_resources/reports/GIF_ANIMATION_AUDIT.md`
- Official fixed-menu icon proof:
  `analysis/official_reference/generated/fixed_menu_official_alignment_proof.png`
- V3 Connection sprite references:
  `https://www.spriters-resource.com/lcd_handhelds/tamagotchiconnectionversion3/`
- V3 guide behavior notes:
  `https://www.tamatalk.com/threads/tamagotchi-v3-guide.107297/`
- Connection franchise behavior summary:
  `https://tamagotchi.fandom.com/wiki/Tamagotchi_Connection_(franchise)`

Runtime note: official raster assets stay analysis-only. Firmware uses C/C++
bitmap tables and hand-authored 1-bit replacement scenes.

## Implementation Summary

- Added `drawCompactV3FunctionScreen(...)` in `src/EchoPetDisplay.cpp`.
- GAT562 compact function pages now bypass the old mixed large-screen renderers:
  `drawChoiceMenu`, text-heavy activity pages, link score/result mashups, and
  oversized page widgets.
- Added compact V3 drawing primitives for the 32x30 native playfield:
  scaled rect/line/text, hearts, pet silhouettes, food, game props, connection
  signal, medicine bag, lamp, open book, and attention/mess indicators.
- Updated `analysis/screen_simulator/host_screen_simulator.cpp` so proof
  scenarios carry the actual fixed-menu slot index, including Care/Attention.
- Generated the 10-page proof contact sheet:
  `analysis/screen_simulator/out/compact_v3_10_function_report_contact_x4.png`.

## 10 Function Pages

| Slot | V3 target | Previous EchoPet/GAT562 problem | New compact drawing |
| --- | --- | --- | --- |
| 01 Health Meter | V3 Health shows care meters such as Hungry/Happy with hearts, and pages through stats. | Compact page reused EchoPet status pages and could show unrelated status text/extra metrics. | Draws V3-style `HUNGRY` and `HAPPY` rows with four hearts each. Attention can blink an exclamation marker. Proof: `compact_health_x4.png`. |
| 02 Food | V3 Food opens Meal/Snack selection; feeding animation shows food prop, bite/chew, crumbs/result. | Compact menu fell back to `drawChoiceMenu` and page labels, then mixed food label text with oversized props. | Draws a two-option Meal/Snack selection with a left cursor and simple food/snack prop. Meal/Snack action draws pet plus moving food and crumb/result phase. Proof: `compact_food_menu_x4.png`, `compact_meal_x4.png` when generated. |
| 03 Toilet | V3 cleanup is not instant: mess is visible, then a vertical flush/sweep wall wipes pet and mess across the LCD. | Earlier firmware made mess disappear too abruptly or showed non-V3 cleanup behavior. | Keeps the V3 32x30 cleanup rows and right-to-left checker wall already derived from `img_breed_03.gif`; compact mode clips the scene inside the 32x30 projection. Proof: `compact_toilet_x4.png`. |
| 04 Games/Activity | V3 Activity leads to games and activity items; official how-to covers Get, Bump, Flag, Heading, Memory, Sprint, Hoops. | The page showed a text menu and could display unrelated score/selection widgets. | Draws V3-style game props in the projected LCD. The mini-game branch now maps to game-specific LCD props: notes/bucket, bump meter, flags, heading ball, memory blocks, sprint track, or hoops. Proof: `compact_activity_menu_x4.png`. |
| 05 Connection/Meet | V3 Connection uses infrared/heart semantics, then Stand-by and Visit/Present/Game/Love/results with two characters. | The reported meet screen mixed `SCORE`, `MEAL`, house/status icons, and a large unrelated pet. This was the clearest wrong-object bug. | Draws only connection semantics: two characters, heart/signal, gift/game/love variant, or `STAND BY`/failure text when applicable. It no longer renders `SCORE`, `MEAL`, house, or health widgets in this page. Proof: `compact_connection_menu_x4.png`. |
| 06 Care/Attention | V3 Attention is a highlighted call icon when the character needs something; it is not a normal content dashboard. | EchoPet mapped this slot back into Health, so it could look like a duplicate status page. | Draws a compact attention-call scene: pet, call bubble/alert mark, and visible mess hint if care is needed. Proof: `compact_care_x4.png`. |
| 07 Discipline | V3 Discipline separates Time Out and Praise, with visible character reaction and training context. | Compact page used a generic drawn box/label composition that did not resemble V3 discipline call/reaction. | Draws an explicit discipline reaction: yelling/time-out mark or praise hearts plus pet reaction. Proof: `compact_discipline_menu_x4.png`. |
| 08 Medicine | V3 medicine shows sick/tooth indicator, medicine/dose, and recovery/refusal states. | The page mixed a large bottle, badge, dose trail, and pet frame in a layout closer to a custom dashboard. | Draws medicine bag, sick/tooth marker, pet, and small dose motion in the 32x30 projected LCD. Proof: `compact_medicine_x4.png`. |
| 09 Lights | V3 Lights uses ON/OFF selection and sleep/dark room behavior. | Compact page drew custom selector text/room geometry and could feel like a UI panel rather than an LCD animation. | Draws a lamp and ON/OFF selector when awake; when lights are off, draws a black V3 sleep screen with a `Z`, matching the official dark-screen idea. Proof: `compact_lights_x4.png`. |
| 10 Friend List | V3 Friend List is a notebook/book function for connected friends and relationship state. | Compact page drew two boxed portraits and text-heavy friend labels. | Draws an open book with two small avatars and a relationship heart; no mixed status/menu labels. Proof: `compact_friends_x4.png`. |

## Current Proof Artifacts

- `analysis/screen_simulator/out/compact_v3_10_function_report_contact_x4.png`
- `analysis/screen_simulator/out/compact_health_x4.png`
- `analysis/screen_simulator/out/compact_food_menu_x4.png`
- `analysis/screen_simulator/out/compact_toilet_x4.png`
- `analysis/screen_simulator/out/compact_activity_menu_x4.png`
- `analysis/screen_simulator/out/compact_connection_menu_x4.png`
- `analysis/screen_simulator/out/compact_care_x4.png`
- `analysis/screen_simulator/out/compact_discipline_menu_x4.png`
- `analysis/screen_simulator/out/compact_medicine_x4.png`
- `analysis/screen_simulator/out/compact_lights_x4.png`
- `analysis/screen_simulator/out/compact_friends_x4.png`

## Acceptance Notes

- The 10 fixed-menu entries now have compact V3-style scene ownership. The main
  playfield should no longer show the earlier wrong-object mixes such as
  `SCORE` plus `MEAL` inside the Connection/Meet view.
- The result is a complete first pass across all 10 fixed functions, not a
  pixel-perfect ROM extraction. Exact timing and per-frame pose parity still
  need hardware video acceptance and, where available, frame-specific reference
  clips.
- GAT562 firmware was built after the change. Flashing still depends on the
  board enumerating as a usable USB serial/bootloader port.
