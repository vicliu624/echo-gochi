# Firmware Sprite Render Audit

This audit renders the firmware's current low-resource sprite parts from source.
It is generated from `EchoPetSprites.cpp`, `EchoPetSprites.h`, and `EchoPetDisplay.cpp`.
It does not copy or embed official Tamagotchi pixels.

## Summary

- Sprite enum frames: 101
- Rendered composed frames: 101
- Missing `connectFrame()` mappings: 0
- Missing composed frame definitions: 0
- Frames with pixels outside their nominal frame: 0

## Generated Files

- `analysis\sprite_render_audit\sprite_contact_sheet.png`
- `analysis\sprite_render_audit\sprite_mametchi_motion.png`
- `analysis\sprite_render_audit\sprite_egg_hatch.png`
- `analysis\sprite_render_audit\sprite_care_scenes.png`
- `analysis\sprite_render_audit\sprite_game_scenes.png`
- `analysis\sprite_render_audit\sprite_frame_metrics.csv`

## Structural Checks

| Check | Status | Details |
| --- | --- | --- |
| all mapped frames draw black pixels | PASS |  |
| all composed pixels stay inside nominal frame | PASS |  |
| egg face/crack anchors are inside the 32x32 shell frame | PASS |  |
| adult idle uses separate eyes, mouth, and mirrored feet | PASS | stand0 feet=[(9, 27, 0), (19, 27, 1)] stand1 feet=[(7, 27, 0), (21, 27, 1)] |
| adult idle foot anchors move between kAdult0 and kAdult1 | PASS | stand0 feet=[(9, 27, 0), (19, 27, 1)] stand1 feet=[(7, 27, 0), (21, 27, 1)] |

## Remaining Acceptance Boundary

- These PNGs prove code-side sprite composition only.
- Hardware proof is still required for e-paper refresh speed, whitening/ghosting, and button timing.
- Official-look final art still requires original or rights-cleared replacement sprites.
- If a user photo shows a visual bug, compare it first against this audit to decide whether the defect is source sprite composition or display refresh behavior.
