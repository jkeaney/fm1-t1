<!-- SPDX-License-Identifier: GPL-3.0-only -->
# UI PROPOSAL v1: FM-1 PERF screen

Source: `proposal_v1.py` (a copy of `mock/current_perf.py`, zones redrawn; same Z0..Z7 tiling, same AF_S/AF_M/AF_L, same blit-strip limits
except blit B, see item 5; same fixtures plus `long_names` and `undo_toast` defined in the file). `before_v1.py` = the current screen on the
same six scenes. `make_sheets.py` builds the side-by-side sheets. `python proposal_v1.py --report` prints every number quoted below.
Palette GREY (the shipping palette is monochrome: `T.ACCENT`, `T.REC`, `T.THEME` are all near-white in it, so the new hues are literal RGB565).

Per scene (idle, playing, row2, snake_mute, long_names, undo_toast): `proposal_v1_SCENE.png`, `_4x.png`, `_grid.png`, and
`SCENE_before_after.png` (current left, proposal right, both 2x). Render tool warnings for the proposal: none (no clipped, no ellipsised text).
The current screen (left sheets) warns: `HLP/MN` cut by the canvas, and legend labels `CHN SOL ROL PG- PG+ RST` ellipsised to `x...`.

Fixture changes: `playing` and `undo_toast` also light channel 2's activity square (the shipped fixture lights only channel 1). `idle`,
`row2`, `snake_mute` are the shipped fixtures. `long_names`: EXT 126, P16, selected channel 4 (STAG, MIDI 16, muted, L64, page 4/4, playing
step 52), channel 3 MIDI OFF (not muted), lane steps 4 accent, 5 slide, 6 ratchet x2, 7 accent+slide, 8 accent+slide+ratchet x3, 9 ratchet x3.

## The 8 brief items

| # | What I drew | Measured | Did not fit / how resolved | Verdict |
| --- | --- | --- | --- | --- |
| 1 Header Z0 | 9x9 drawn transport (play triangle `T.ACCENT`, stop square `T.TEXT`, REC disc `T.REC`), clock dot (on phase), BPM digits AF_M, source word AF_S in `T.MID`, preset, `N ALGO Mx`, page map, `Lnn` | AF_M `126` 27, `--` 13; AF_S `EXT` 21, `WAIT` 28, `P16` 21, `P05?` 27, `4 STAG M16` 66, `4 STAG MOFF` 73 (slot 73, fits), `1 PATT MOFF` 71, `L64` 20. Slots: shape 2, dot 12, BPM 22, source 51, preset 75, algo 103..176, map 177..216, L at 218..238 (right edge 238 of 240). Tempo group ends x=72 (EXT 126) | Fits only because I moved every slot (H-slot table in SCREEN_SPEC 2.1 changes) and the preset slot is 28 (for `P05?`). 2 px of margin left at the right. `WAIT` replaces the digits, `EXT --` draws `--` in AF_M. REC was not in a shipped fixture: the shape code exists but is not in any render | Works, tight |
| 2 Cells Z1 | Selected = inverted, as now. Line 2 is always `MIDI n` / `MIDI OFF`. Muted: name in `T.MID` (6.42:1 on BG; the current `T.DIM` is 2.87:1) plus a solid 12x11 chip with the letter `M` (`T.INK` on `T.MID` 6.42:1; on a selected cell `T.SEL` on `T.INK` 10.42:1). Activity: lit = solid 4x4, idle = 2x2 dot, bottom right | `MUTE` AF_S = 32 px (+2 chip pad 34); `4 STAG` 39: 73 px > the 56 px ink width | **The chip does not carry the word MUTE.** With `MIDI OFF` (48 px) on line 2 and the name (up to 40 px) on line 1, no 34 px chip fits anywhere. Shape (solid chip) + the letter that is on the key that toggles it (`M3`) is what I could do. The brief's shape + word is not met. Idle/lit activity is distinguished by size only (4x4 vs 2x2), same colour; the lit square is `T.INK` on SEL (10.42:1) and `T.THEME` on BG (16.8:1) | Partial |
| 3 Lane Z2 | Header `PG 1/2  FOLLOW` (`hold=EDIT` gone; `hold` for not-following became `PINNED`, my change), playing-page marker = 6x5 triangle + `Pn`. Cell: bars 28 px max from y+30; mark row y+32..38; playing column filled `PLAYCOL` over all 46 px plus the 2 px underline kept. Marks: accent = 7x4 (5x5 when sharing) filled triangle; slide = 7x6 rising arrow; ratchet = 1 px bars x2/x3 (3 or 5 px wide, 7 high). Accent also = amber bar with a 2 px light cap. Edited step = 2x2 dot under the marks | Max mark row: arrow 7 + 1 + 3 bars 5 = 13 of 14. `PLAYCOL` vs `T.LANE` 1.45:1 (low; the underline and `>Pn` are the redundancy). Cap `T.THEME` on `H_ACCNT` 1.69:1 | Accent + slide + ratchet on one step does not fit 14 px (5+7+5+2): the accent triangle yields to the bar hue + cap (step 8 in `long_names`). Ratchet capped at 3 bars (x4 and more look like x3). Marks shown only for steps that are ON (the current screen draws `x2` on an off step). The old `'` edited glyph became the 2x2 dot (a spec change). **Colour tokens**: accent mark, slide mark, ratchet mark and bar all need NEW tokens (`H_ACCNT`, `H_ARTIC`, `H_HITS`, `PLAYCOL`); the edit dot, cap, underline reuse `T.TEXT`, `T.THEME`, `T.ACCENT` | Works |
| 4 Band Z3 | Active row 34 px: label AF_S over value AF_M on `T.SEL`; inactive row 18 px: one AF_S line `LABEL value` (label `T.MID` 6.42:1, value `T.TEXT` 12.3:1; the current `T.DIM` 2.87:1), cell separators in `T.LINE`. Rows swap height with the latched row. Tag column 40 px (cells 50): `ROW n` in AF_S, filled when active | Active: LEGATO 43 of 46 and `100` in AF_M 27: all four AF_M values + labels fit, no change needed. Inactive, 47 px budget with a 2 px gap: 20 of the 48 labels in the six algorithms exceed it | **Peek labels are shortened** (`OFFSET OFFS`, `ACCNT ACNT`, `PEDAL PDAL`, `RATCH RTCH`, `PUNCH PNC`, `CURVE CURV`, `RANGE RNGE`, `TOTAL TOTL`, `SHAPE SHPE`, `LEGATO LEGA`, `CTONE CTNE`) when the full pair + a 2-digit value does not fit. Abbreviations like `PNC` are poor. Tag column grew 36 to 40 px, cells 51 to 50. Label/value gap (2 px) is the same as the gap between pairs (3 px), only the separator and the colour split them | Works, labels weak |
| 5 Buttons Z4 | Line 1 layer name once with a 5x5 hue + shape mark (NOTES diamond, HITS bars, ACCNT triangle, ARTIC arrow, SHAPE wave; absent layer = grey mark + `NONE`); line 2 only if it adds meaning, else hold role with a 6x3 chevron; `ALL` shows five 2x2 hue chips; PLAY/REC carry the header shapes | Row 1 needs 235 of 240 (avg pad 0.8 px, ink can touch the 1 px separator); row 2 222..233. Text rows y+0 / y+12, so ink ends at y+23 of 26, and blit B is 108 high (was 104), so even a descender cannot clip: tool reports no cut text | Rows are crowded. Strip: blit B 240x108 = 25920 px, under the 29760 CV_MAX; the 4 extra rows are overwritten by blit C. **Strings**: `HLP/MN` to `MENU` (the real outcome of the hold: HELP is only while held and is explained by the 0.25 s strip `HOME: touch=HELP let go=MENU`; `HELP` as label would promise something the release does not do); `UNDO L` to `REVERT` (undo of a preset load, 42 px; `UNLOAD` would be wrong); `GT+SLD` to `SLIDE` (ACID's artic layer is gate and slide: `SLIDE` drops the gate half); `RT+HLD` to `RATCH`. `NOTES` under `NOTES`, `ACCNT`, `HITS` under their own name gone | Works |
| 6 Legend Z5 | `M1..M4` text, muted = BG fill + `T.LINE` frame + `T.MID` text + 1 px strike (6.42:1 on BG; unmuted `T.TEXT` on `T.RAISE` 7.19:1). Glyphs (all `T.TEXT` on `T.RAISE`): CHN sliders 9x7, SOLO S-in-box 9x9, FILL F 6x9, ROLL R 6x9, PAGE- / PAGE+ chevrons 5x7, RESTART bar+arrow 7x7 | Cue gap: cue ink rows 14..22, legend ends at row 11 = 2 px. Pixels come from inside Z6 (text moves down 2 px); Z5 12, Z6 12, Z7 4 unchanged, tiling intact | SOLO/FILL/ROLL are hold keys, PAGE-/+ and RESTART are taps; nothing distinguishes them. No word anywhere: first-time legibility of F, R, the sliders is low. `T.MID` on RAISE is only 3.75:1, so I did not use it for the muted text (BG fill instead) | Works, learnability untested |
| 7 Cue Z6 | Text unchanged, 2 px lower | `T.TEXT` on `T.BG` 12.33:1 (>=7). Toast: `T.ACCENT` on `T.TINT` 15.80:1, bar starts at row 13, 8x6 undo arrow glyph before the text | none | Works |
| 8 Colour | Five layer hues + `PLAYCOL`; REC = existing `T.REC`; mute = chip + strike (existing tokens); selection = existing `T.SEL` | table below | Hues appear in: HITS = lane bars + button mark; ACCNT = accent bars, accent mark, button; ARTIC = slide and ratchet marks + button; **NOTES and SHAPE appear only on their button mark (the lane has no pitch or shape element), so the "matching lane mark" requirement is not met for those two.** Every hue is paired with a distinct shape (bars, triangle, arrow, diamond, wave) and the layer word | Partial |

### New RGB565 tokens vs the existing `T_*` set

| Token | RGB565 | Hex | on BG | on LANE | on PLAYCOL | on RAISE | on TINT | Use |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| H_NOTES | 0xE455 | #e689ac | 8.14 | 4.75 | 3.27 | 4.75 | 6.62 | NOTES button mark |
| H_HITS | 0x6D5C | #6aaae6 | 8.11 | 4.73 | 3.25 | 4.73 | 6.60 | lit bars, HITS button mark |
| H_ACCNT | 0xED48 | #eeaa41 | 9.98 | 5.82 | 4.00 | 5.82 | 8.12 | accent bar and mark, ACCNT mark |
| H_ARTIC | 0x5E11 | #5ac28b | 9.08 | 5.30 | 3.64 | 5.30 | 7.39 | slide and ratchet marks, ARTIC mark |
| H_SHAPE | 0xB49D | #b491ee | 7.84 | 4.57 | 3.15 | 4.57 | 6.38 | SHAPE button mark |
| PLAYCOL | 0x4A8C | #4a5062 | 2.49 | 1.45 | 1.00 | 2.03 | 1.45 | playing column fill |

(Contrast values from the `--report` table, computed on the earlier hex rounding; the 0x565 values above are what is drawn.) Hues vs `T.SEL`
are not needed: no hue is drawn on a selected cell. Existing tokens re-used where the current screen used `T.DIM`: `T.MID` replaces `T.DIM`
for every text that is read (muted names, peek row, page-map outlines) because `T.DIM` is 2.87:1 on BG and 1.67:1 on `T.LANE`.

## Firmware cost (honest)

- `ui_sq.c`: rewrite of `uisq_draw_z0..z6` (all six zones change), a 1-bit glyph blit primitive (the mock's `glyph1` has no firmware counterpart; roughly 25 glyphs, 5x5 to 9x9 / 9x9 shapes, about 200 bytes of flash), tempo-source and REC state fed into Z0, blit B 240x108, new Z0 slot table, new button width logic (layer mark 6 px, chevron 7 px), the peek-row shortening rule.
- `gfx.c` palette: 6 new tokens. They are literal in this mock; for the 7+ shipping palettes (AMBER etc.) I have not designed per-palette hues. Five fixed hues on a monochrome or amber theme will clash.
- Generated tables: `button_modes.tsv` (hold labels `HLP/MN` to `MENU`, `UNDO L` to `REVERT`), the algorithm sub-label table (`GT+SLD` to `SLIDE`, `RT+HLD` to `RATCH`, `NONE` for absent), `knob_map.tsv` gains a peek-label column (about 12 entries), `tools/gen_ui_tables.py` and the test's golden PNGs (`ui_sq_test`), `widths.tsv` rows for every changed string (`PINNED`, `REVERT`, `MENU`, peek pairs, the header slots), SCREEN_SPEC 2.1 (H slots), 2.2 (cell chip), 2.3 (lane header, cell layout), 2.4 (tag column 40 px), 2.5 (button table), 2.6 (cue gap).
- Behaviour/spec decisions hidden in the mock: marks hidden for off steps; ratchet capped at 3 bars; accent triangle yields at three marks; `PINNED` word; `M` chip instead of `MUTE`.
- No font change: only AF_S / AF_M used. Nothing below AF_S is needed mid-performance (the 5x5 marks are decoration, every one is paired with a word).

## Least sure about

1. The muted chip has no word (item 2): the brief's shape + word is not met; `M` only echoes the M-key.
2. The peek row: abbreviated labels (`PNC`, `PDAL`, `OFFS`) and a 2 px label/value gap are weak; grouping depends on a faint `T.LINE` separator and a 6.4 vs 12.3 contrast split.
3. Hue at arm's length: five mid-luminance hues on dark plus bars in blue is calmer than neon but I have not seen it on the panel; the bars in blue change the lane from the neutral look to a colour-coded one, and NOTES / SHAPE hues have no lane element.
4. `MENU` as the HOME hold label hides that HELP is available while held; `SLIDE` for ACID's artic layer drops the gate half; `REVERT` has not been tested with users.
5. Legend glyphs (F, R, sliders, bar-arrow) carry no word and no tap/hold distinction; `PLAYCOL` is a 1.45:1 tint whose job is mostly done by the underline; button row 1 is 235 of 240 px and visibly cramped.
