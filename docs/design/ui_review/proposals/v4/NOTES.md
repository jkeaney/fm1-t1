<!-- SPDX-License-Identifier: GPL-3.0-only -->
# v4, round 1 (design session 2026-11-12): direction draft, NOT critiqued yet

Built from the owner interview (`HANDOFF/08_OPEN_QUESTIONS.md` section 0). Render: `python3 ../../mock/mock.py render
proposal_v4.py --out out`; text count: `python3 count_text.py`.

## What changed against v3.2, and why (owner's words in brackets)
- **Scales with channels in use** ("only showcase 4 channels when they are set up and in use"): 1 row fills the area,
  2 rows halve it, 4 rows are compact. Unused channels are not drawn.
- **Colour per channel** (owner choice) on the chip, lane and knob meters. Shape carries every state too: selected =
  filled chip + raised row; muted = grey hollow bars + struck-through chip (+ speaker icon when the row is tall).
- **Palette:** rendered on `MONO` (black/white, `mono=0`), because `GREY` is mono and greys out coloured text
  (`gfx.c:154`). Firmware cost: either default to MONO or add `T_CH1..T_CH4` that bypass the mono ramp; tokens needed
  on every palette.
- **No ROW strip, no cue line** ("waste of space"): the knob row is a 3-square marker (owner: "small marker").
- **Algorithm named once, above the four knobs it drives**, in the selected channel's colour.
- **Lane = pitch height** (decision 119), accent = a separate white cap, slide = a joining stripe, rest = a floor dot,
  playhead = a lit column.
- **Press = big flash + highlight** (owner: "both"): the flash replaces the knob band for ~1 s (dice icon, the layer
  in AF_L, the printed button boxed + DICED) so the lanes stay visible; changed steps turn white with an underline.
- **MIDI channel** shown on the chip only when the row is tall (1-2 channels).

## Counts (script, `count_text.py`)
1 ch 14 items, 2 ch 17, 4 ch 15 (v3.2: 28). Dice flash visible ~12 (the covered knob text is drawn then covered).
Contrast (MONO): channel colours on black 8.1-12.8:1; ink on chips 7.7-12.2:1; MID on RAISE 7.7:1; DIM only for
non-text (3.8:1).

## Open / not drawn yet
How a channel is added (proposal: SELECT past the last used channel shows a `+` row), the panel-shaped instruction
screen with icons (owner: the held map is the wrong idea), REC/EXT/preset-pending states, mono fallback.
