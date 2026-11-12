<!-- SPDX-License-Identifier: GPL-3.0-only -->
# v5, round 2 (design session 2026-11-12): section by section, with options, then fitted

Owner's brief for this round: "focus on the space, the grid, and then each function we need to display, and in what
context. then for each item ... the most novel way we can communicate it ... in the smallest space ... do section by
section, with options, then make it all fit." Feedback must cover: what I just did, the knob being turned, what is
pending, what I can do next: "done cleanly, with ui feedback, quick toasts, instant response."
Render: `python3 ../../mock/mock.py render v5.py --out out && python3 sheets.py`. NOT independently critiqued yet.

## 1. Space and grid
- 240 px = 30 mm, 8 px/mm; owner sits 30-40 cm away: AF_S ~10.8, AF_M ~13.7, AF_L ~25.5 arcmin at 35 cm.
  **Proposal: relax decision 116** so AF_S may carry short reference labels (second knob row, key glyph letters);
  anything read while playing stays AF_M/AF_L or a shape.
- **Columns: 16 x 15 px = the 16 white keys.** Step n of the music row sits over white key n; the black keys sit
  between the columns in their real 3-2-3-2-1 places (centres 15 30 45 75 90 120 135 150 180 195 225). Knobs and
  buttons use 4 x 60 / 6 x 40, the panel's own counts.
- **Bands, top to bottom, in panel order** (layout A): header 0-21 (transport, BPM, SELECT chips, ALGO, PRESETS) ·
  knobs 24-92 (live row big + other row small) · music 93-151 · buttons 156-198 (2 x 6, as on the panel) ·
  keybed 204-236. Layout B puts the encoders as a 2 x 2 block left of the knobs (as on the panel) and a piano roll in
  the middle.

## 2. Inventory: every function, its context, where it lives (from button_modes / knob_map / key_map / page_states)
| Item | Context | Home in A | Encoding |
| --- | --- | --- | --- |
| Transport (play/stop/REC) | always | header | icon |
| BPM | always | header | AF_M digits |
| Channels in use + selected (SELECT) | always | header chips | colour chip + number; `+` = next free channel (add) |
| Algorithm of selected channel (ALGO) | always | header | glyph + name in channel colour |
| Preset (PRESETS) / pending `Pnn?` | always / pending | header | boxed while pending |
| K1-K4 live row | always | knob band | name + big value + meter |
| The other knob row (OCT-/OCT+) | always | under the knobs | AF_S name + value |
| Knob being turned | while turning (~1 s) | knob band | cell widens: old > new, full range |
| Pattern of every channel + playhead | always | music band | shade stack (M-B) or roll (M-C) |
| Accent / slide / rest / muted / just diced | always | music band | white corner / bridge / floor dash / dotted / hatched |
| Page and length (1-4 pages) | always | keybed | page boxes |
| 12 buttons and what they do | always | button block | icon per role, dice group tinted, pressed = lit |
| Mute keys M1-M4 | always | keybed | speaker in channel colour, crossed when muted |
| CHN, SOL, FIL, ROL, PG-, PG+, RST | always | keybed | glyph at the key's real position |
| What I just did + next action | on press (~1 s) | top toast | icon + result + hint (`EDIT=UNDO`) |
| Holds (EDIT=RESET, GLO=HARM, SAVE=UNDO L, HOME=menu) | on hold | top toast | to draw next round |

## Counts (script)
A 3 ch: 25 text items + 12 button icons + 11 key glyphs; B 3 ch: 19 text items. v4 was 15, v3.2 28: more information,
mostly as shapes.

## Firmware costs to state
Palette MONO (GREY greys coloured text, `gfx.c:154`) or channel tokens that bypass it; 1-bit icon blits (~30 icons of
11 x 11 = ~500 B); 12-step shade ramps per channel computed at run time; the dial arc (K-B) needs a precomputed mask.
