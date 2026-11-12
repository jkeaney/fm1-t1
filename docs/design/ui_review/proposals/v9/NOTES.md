<!-- SPDX-License-Identifier: GPL-3.0-only -->
# v9 (design session 2026-11-12): after critique v8 (19/40) and the owner's decisions

Render: `python3 ../../mock/mock.py render v9.py --out out && python3 sheets_v9.py`. Palette HI-CON (existing; mono=0).

## Owner decisions applied
- Music row: the **selected channel full width** (pitch bars, white accent cap, slide bridge, hatched = just diced,
  hollow = silent); the other channels in use are **2 px hit strips** under it, in channel order.
- Holds: a **hint while held** (top bar from 0.25 s names the hold, e.g. `HOLD EDIT: RESET`; a bar fills to the fire
  time). HOME uses its TSV strip text `TOUCH=HELP  LET GO=MENU`. The corner notch is gone.
- **SELECT past the last channel adds one** (`+` chip; toast `NEW` + `ALGO=PICK`). Needs a button_modes/knob_map row.

## Critique v8 fixes
MIDI OFF: inverted red `MIDI OFF` chip, a red cap on that channel's chip, its bars hollow red, a toast with the channel
chip and `CHN=SET` (how to fix it). REC: its own knob band (CURSOR ACC SLIDE OCT, knob_map `*` REC rows), plain keys,
the page_states cue `REC: PLAY KEYS TO WRITE`. Dice tiles: neutral lighter grey + white icons (no hue). Value lists: position
dots instead of a meter. One font per knob for its whole range (only the zero word KEYS drops to AF_M). Preset and page map
moved to a status line (preset | row marker | page boxes): the header no longer collides. ROW marker shows 3 squares only
when the channel has a RIG wire. Toasts carry the channel chip. ARP DIR parse fixed; PATTERNS PICK reads `ACD 1`..`MEL 20`
(SPEC section 1 tags); STAGE TOTAL `48(32)`.

## The algorithm interface the screen needs (proposal for AlgoDesc, firmware/src/sq_types.h)
The screen code names no algorithm (mock: `algos_v9.py` is the only per-algorithm source; a DEMO algorithm registered in
one place renders on every element). For firmware the same needs a small UI part per algorithm. Proposed fields (exact
layout to be decided in the ADR):
| Field | Today | Needed for |
| --- | --- | --- |
| `name[8]`, short name <= 6 | `name[8]` | header (`PATT`) |
| `morph[].label/min/max/def` | yes (MorphDesc) | knob tiles |
| `morph[].names` (value names table or NULL) | no (TSV only) | FWD/BWD/PEND, LINE..WAVE, 1/4..4X: tile text + position dots instead of a meter |
| `morph[].fmt` (unit suffix e.g. ` OCT`) and zero word (`KEYS`) | no | `2 OCT`, `KEYS` |
| `value_text(morph, value, cache, buf)` optional hook | no | algorithm-only text: PICK `HSE 3`, STAGE TOTAL `48(32)` |
| `lane` mode: pitched / single | no | music row (bar height = pitch, or a hit marker) |
| `layer[].n == 0` (absent) | yes | the matching die is greyed |
| `sub[]` dice sub-labels | yes | toast text (to draw) |
A build-time check (like `tools/gen_ui_tables.py`) runs `algos_v9.check()`'s rules over every row: label <= 6 chars and fits,
widest value fits its tile in AF_L else AF_M, short name fits the header.
