<!-- SPDX-License-Identifier: GPL-3.0-only -->
# ADR-0006: Screen architecture v2 (scaling glance screen, message slot, keybed footer)

Status: **accepted** (owner, 2026-11-12: "Design locked"). Spec: `docs/design/SCREEN_SPEC_V2.md`.

## Context
The screen is 240 x 240 px and 30 mm wide, read at 30-40 cm with busy hands. The shipping dense layout (`ui_sq.c`, P4-011)
and mock-ups v1-v3.2 were rejected as "too much information, too small" and "not good enough". A design session
(docs/design/ui_review/HANDOFF/) interviewed the owner with pictures and iterated v4-v12 with an independent two-part
critique each round (19 -> 24 -> 27 -> 26 /40, no P0 from v9 on). The owner judged v11 "good enough" and locked the full
state set (v12).

## Decision
1. One main screen in six bands in panel order: header, knob tiles, status line, music row, 2 x 6 button picture,
   keybed footer (SCREEN_SPEC_V2 section 2). The horizontal grid is the keybed's: 16 columns of 15 px.
2. The screen scales with the channels in use; channels are added by SELECT past the last and removed by a long press
   on their mute key.
3. Controls without a permanent picture are named only while touched, in one message slot (x104-239 of the header) that
   never hides transport, tempo or channels; one standing exception: the MIDI setup cue.
4. Colour per channel (Okabe-Ito) on the HI-CON palette; every state also differs by shape.
5. Icons, not words, for the 12 buttons: a die frame means re-roll and its inside names the layer.
6. The footer draws the keybed in its current mode (steps, REC notes, CHAN back).
7. Nothing on the screen is algorithm-specific in code: algorithms describe their UI (ADR-0007).
8. Every message string and every algorithm's labels/values are width-checked at build time.

## Consequences
- `ui_sq.c`'s zones (P4-011) are replaced; its page enum, toast queue and table-driven labels are reused (P4-026).
- P4-012 (cut-over) wires this screen, not the old one; it waits for P4-026..P4-029.
- New palette tokens (channel colours, playhead, dice, OFF) need values on all palettes plus a mono fallback.
- ~25 1-bpp icons (~400 B), a message string table, value-name tables per algorithm (ADR-0007): costed in P4-026/27.
- Decisions 116 (glance-only content), 117 (toasts + cue band + held MAP) and 118 (HOME hold shows a MAP) are superseded
  by this ADR; 119 (pitch-height bars, accent as a shape) stands.
- Known gaps accepted at lock are listed in SCREEN_SPEC_V2 section 9 and re-checked on the unit at HW-1.
