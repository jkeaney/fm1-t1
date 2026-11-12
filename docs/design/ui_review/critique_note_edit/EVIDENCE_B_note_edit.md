<!-- SPDX-License-Identifier: GPL-3.0-only -->
# EVIDENCE B: NOTE_EDIT page (deterministic measurements, facts only)

Scope: the NOTE_EDIT page of `firmware/src/ui_sq.c` as rendered by `tests/ui_sq_note_test.c` (`note_open`, `note_set`, `note_dice`, `note_final`) on the HI-CON palette
(index 7, `UI_HICON_INDEX`, the default). No opinions. Every number below is printed by a script in `docs/design/ui_review/critique_note_edit/scripts/`; the raw outputs are in `.../data/*.out.md`.

## 0. Method and provenance

- **Renders.** The four frames were re-rendered by `scripts/make_trace.py`, which writes `scripts/ne_trace_test.c` (= `tests/ui_sq_note_test.c` + a draw-op logger: every `cv_text_on / cv_text_fit / cv_rect / cv_frame / cv_icon / cv_line` call and every
  ink box is logged in screen coordinates, with the strip it came from) and compiles it to `/tmp/ne_trace_test`. `build/`, `firmware/`, `tests/` and the tables were not touched; `build/gen/*.h` was read only. The instrumented test
  passes with 0 failures and its `note_*_orig.png` is pixel-identical (0 px) to the unmodified test compiled the same way. Each shot is also re-drawn with `uisq_page = UI_PG_PERF` (same machine state, same ms) as `note_*_perf.png` (section 7).
- **Not the same pixels as `build/ui_sq/note_*.png`.** Those files were written 08:39:23; `build/gen/ui_fonts.h`, `ui_palettes.h`, `ui_icons.h`, `ui_sqicons.h` were regenerated 08:40:06 to 08:40:09. Against the current `build/gen` the four files differ from
  the re-renders by 201, 243, 562 and 294 px; every differing pixel lies inside a text ink box (glyph anti-aliasing; section 7b). All measurements here use the re-renders in `data/`.
- **What the frames show** (from the test): stopped, 120 BPM, CH1 `ACID` on MIDI 1 selected, CH2-CH4 drawn as muted chips, LENGTH 16, NOTE_EDIT open on step 9 (column 9, `edit` = 9). The test links a stub `ALGOS` table (`t_step` hashes the step), so every `GEN ...` value is the stub's, not a real algorithm's.
  - `note_open`: step 9 just opened, all four fields AUTO.
  - `note_set`: after ARP (SLIDE), LFO (PROB), ENV (ACCENT, then back), OCT+ x4 (stops at +2) and OCT- x5 (stops at -2): chips `SLIDE` `PROB 50` `OCT-2`; K4 tile framed (turn highlight, its value is AUTO).
  - `note_dice`: after SCL dice x9 and FX (ALL) dice: PITCH `F2`, GATE `100`, VELO `80`, RATCH AUTO; chips `NO SLIDE` `PROB 50` `OCT-2`; a toast in the message slot (`1` chip, `ALL`, `EDIT=UNDO`) paints over the header's `ACID`, `MIDI`, `1`.
  - `note_final`: after the pixel-test sequence: VELO `83` set (K3 tile framed), others AUTO.
- **Two extra frames for section 5**, drawn through the same element code from a hand-set `UisqView` (`worst_w1`, `worst_w2`): W1 = step 64 (column 16), all four fields set to their widest text (`C#-1`, `100`, `ACC`... see the table), all four chips (`NO SLIDE`, `PROB 50`, `OCT+2`, `OFF`);
  W2 = step 1 (column 1), PITCH `A#-1` (negative octave), GATE `TIE`, VELO `ACC`, RATCH AUTO, no chips.
- **Units.** 8 px per mm (240 px = 30 mm, as in `critique_v3_2`); cap heights from `mock.measure("H", font)`: AF_S 9 px, AF_M 11 px, AF_L 21 px; arcmin = cap_mm / (distance in mm) x 3438. Contrast = WCAG 2 ratio of the nominal RGB565 values (`mock.contrast_ratio`),
  glyph anti-aliasing not included. Text bounding boxes are the glyph bitmap box (`ink`); "lit" boxes are the pixels that are the text colour or a blend of it over its background.
- Tools used: `docs/design/ui_review/mock/mock.py` (`measure`, `contrast_ratio`, `text_w`, the same font tables, cap heights) via `scripts/ne_lib.py`; the diff clusterer is a local copy of `mock.py diff`'s idea (8-neighbour clusters).


## 1. Text inventory per render

Measured cap px: {'S': 9, 'M': 11, 'L': 21} ; 8 px/mm; arcmin = cap_mm / dist_mm * 3438
| font | cap px | cap mm | arcmin @35 cm | arcmin @50 cm |
|---|---|---|---|---|
| AF_S | 9 | 1.125 | 11.1 | 7.7 |
| AF_M | 11 | 1.375 | 13.5 | 9.5 |
| AF_L | 21 | 2.625 | 25.8 | 18.0 |


#### note_open (28 text draws visible; later filled rects cover (fraction of ink box): none)
| # | string | font | x,y(line top) | advance px | ink box x0,y0,x1,y1 | ink w x h | cap mm | arcmin@35 | arcmin@50 | fg (token) | bg passed (token) | fg/bg |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| 1 | `120` | M | 16,1 | 27 | 17,4,43,17 | 26x13 | 1.375 | 13.5 | 9.5 | TEXT #FFFAFF | BG #000000 | 20.37 |
| 2 | `1` | M | 48,1 | 9 | 49,5,54,16 | 5x11 | 1.375 | 13.5 | 9.5 | BG #000000 | CH1 #E69D00 | 9.19 |
| 3 | `2` | M | 62,1 | 9 | 62,4,71,16 | 9x12 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 4 | `3` | M | 76,1 | 9 | 76,4,85,17 | 9x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 5 | `4` | M | 90,1 | 9 | 90,5,99,16 | 9x11 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 6 | `ACID` | M | 105,1 | 33 | 105,4,138,17 | 33x13 | 1.375 | 13.5 | 9.5 | CH1 #E69D00 | BG #000000 | 9.19 |
| 7 | `MIDI` | S | 143,4 | 23 | 143,7,166,16 | 23x9 | 1.125 | 11.1 | 7.7 | MID #B4AEB4 | BG #000000 | 9.65 |
| 8 | `1` | M | 168,1 | 9 | 169,5,174,16 | 5x11 | 1.375 | 13.5 | 9.5 | WHITE #FFFFFF | BG #000000 | 21.00 |
| 9 | `PITCH` | M | 9,23 | 42 | 9,26,50,39 | 41x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 10 | `AUTO` | M | 10,45 | 40 | 10,48,50,61 | 40x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 11 | `GEN C2` | S | 9,65 | 42 | 9,68,51,78 | 42x10 | 1.125 | 11.1 | 7.7 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 12 | `GATE` | M | 72,23 | 36 | 72,26,108,39 | 36x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 13 | `AUTO` | M | 70,45 | 40 | 70,48,110,61 | 40x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 14 | `GEN 40` | S | 70,65 | 40 | 70,68,110,78 | 40x10 | 1.125 | 11.1 | 7.7 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 15 | `VELO` | M | 131,23 | 37 | 131,26,167,39 | 36x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 16 | `AUTO` | M | 130,45 | 40 | 130,48,170,61 | 40x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 17 | `GEN 43` | S | 130,65 | 40 | 130,68,170,78 | 40x10 | 1.125 | 11.1 | 7.7 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 18 | `RATCH` | M | 186,23 | 47 | 186,26,233,39 | 47x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 19 | `AUTO` | M | 190,45 | 40 | 190,48,230,61 | 40x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 20 | `GEN 1X` | S | 190,65 | 40 | 190,68,231,78 | 41x10 | 1.125 | 11.1 | 7.7 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 21 | `STEP` | S | 3,79 | 28 | 3,82,31,92 | 28x10 | 1.125 | 11.1 | 7.7 | MID #B4AEB4 | BG #000000 | 9.65 |
| 22 | `9` | M | 34,76 | 9 | 34,79,43,92 | 9x13 | 1.375 | 13.5 | 9.5 | WHITE #FFFFFF | BG #000000 | 21.00 |
| 23 | `= SET BY YOU` | S | 162,79 | 75 | 163,82,237,92 | 74x10 | 1.125 | 11.1 | 7.7 | MID #B4AEB4 | BG #000000 | 9.65 |
| 24 | `S` | S | 116,189 | 7 | 116,192,123,202 | 7x10 | 1.125 | 11.1 | 7.7 | TEXT #FFFAFF | RAISE #414041 | 10.01 |
| 25 | `F` | S | 131,189 | 7 | 131,192,137,201 | 6x9 | 1.125 | 11.1 | 7.7 | TEXT #FFFAFF | RAISE #414041 | 10.01 |
| 26 | `R` | S | 146,189 | 7 | 146,192,153,201 | 7x9 | 1.125 | 11.1 | 7.7 | TEXT #FFFAFF | RAISE #414041 | 10.01 |
| 27 | `KNOBS EDIT` | M | 2,221 | 83 | 2,224,85,237 | 83x13 | 1.375 | 13.5 | 9.5 | TEXT #FFFAFF | BG #000000 | 20.37 |
| 28 | `HOME = DONE` | M | 139,221 | 99 | 139,224,237,237 | 98x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | BG #000000 | 9.65 |


#### note_set (30 text draws visible; later filled rects cover (fraction of ink box): none)
| # | string | font | x,y(line top) | advance px | ink box x0,y0,x1,y1 | ink w x h | cap mm | arcmin@35 | arcmin@50 | fg (token) | bg passed (token) | fg/bg |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| 1 | `120` | M | 16,1 | 27 | 17,4,43,17 | 26x13 | 1.375 | 13.5 | 9.5 | TEXT #FFFAFF | BG #000000 | 20.37 |
| 2 | `1` | M | 48,1 | 9 | 49,5,54,16 | 5x11 | 1.375 | 13.5 | 9.5 | BG #000000 | CH1 #E69D00 | 9.19 |
| 3 | `2` | M | 62,1 | 9 | 62,4,71,16 | 9x12 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 4 | `3` | M | 76,1 | 9 | 76,4,85,17 | 9x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 5 | `4` | M | 90,1 | 9 | 90,5,99,16 | 9x11 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 6 | `ACID` | M | 105,1 | 33 | 105,4,138,17 | 33x13 | 1.375 | 13.5 | 9.5 | CH1 #E69D00 | BG #000000 | 9.19 |
| 7 | `MIDI` | S | 143,4 | 23 | 143,7,166,16 | 23x9 | 1.125 | 11.1 | 7.7 | MID #B4AEB4 | BG #000000 | 9.65 |
| 8 | `1` | M | 168,1 | 9 | 169,5,174,16 | 5x11 | 1.375 | 13.5 | 9.5 | WHITE #FFFFFF | BG #000000 | 21.00 |
| 9 | `PITCH` | M | 9,23 | 42 | 9,26,50,39 | 41x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 10 | `AUTO` | M | 10,45 | 40 | 10,48,50,61 | 40x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 11 | `GEN C2` | S | 9,65 | 42 | 9,68,51,78 | 42x10 | 1.125 | 11.1 | 7.7 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 12 | `GATE` | M | 72,23 | 36 | 72,26,108,39 | 36x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 13 | `AUTO` | M | 70,45 | 40 | 70,48,110,61 | 40x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 14 | `GEN 40` | S | 70,65 | 40 | 70,68,110,78 | 40x10 | 1.125 | 11.1 | 7.7 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 15 | `VELO` | M | 131,23 | 37 | 131,26,167,39 | 36x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 16 | `AUTO` | M | 130,45 | 40 | 130,48,170,61 | 40x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 17 | `GEN 43` | S | 130,65 | 40 | 130,68,170,78 | 40x10 | 1.125 | 11.1 | 7.7 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 18 | `RATCH` | M | 186,23 | 47 | 186,26,233,39 | 47x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 19 | `AUTO` | M | 190,45 | 40 | 190,48,230,61 | 40x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 20 | `GEN 1X` | S | 190,65 | 40 | 190,68,231,78 | 41x10 | 1.125 | 11.1 | 7.7 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 21 | `STEP` | S | 3,79 | 28 | 3,82,31,92 | 28x10 | 1.125 | 11.1 | 7.7 | MID #B4AEB4 | BG #000000 | 9.65 |
| 22 | `9` | M | 34,76 | 9 | 34,79,43,92 | 9x13 | 1.375 | 13.5 | 9.5 | WHITE #FFFFFF | BG #000000 | 21.00 |
| 23 | `SLIDE` | S | 50,78 | 31 | 50,81,80,91 | 30x10 | 1.125 | 11.1 | 7.7 | BG #000000 | CH1 #E69D00 | 9.19 |
| 24 | `PROB 50` | S | 87,78 | 47 | 87,81,134,91 | 47x10 | 1.125 | 11.1 | 7.7 | BG #000000 | CH1 #E69D00 | 9.19 |
| 25 | `OCT-2` | S | 140,78 | 35 | 140,81,175,91 | 35x10 | 1.125 | 11.1 | 7.7 | BG #000000 | CH1 #E69D00 | 9.19 |
| 26 | `S` | S | 116,189 | 7 | 116,192,123,202 | 7x10 | 1.125 | 11.1 | 7.7 | TEXT #FFFAFF | RAISE #414041 | 10.01 |
| 27 | `F` | S | 131,189 | 7 | 131,192,137,201 | 6x9 | 1.125 | 11.1 | 7.7 | TEXT #FFFAFF | RAISE #414041 | 10.01 |
| 28 | `R` | S | 146,189 | 7 | 146,192,153,201 | 7x9 | 1.125 | 11.1 | 7.7 | TEXT #FFFAFF | RAISE #414041 | 10.01 |
| 29 | `KNOBS EDIT` | M | 2,221 | 83 | 2,224,85,237 | 83x13 | 1.375 | 13.5 | 9.5 | TEXT #FFFAFF | BG #000000 | 20.37 |
| 30 | `HOME = DONE` | M | 139,221 | 99 | 139,224,237,237 | 98x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | BG #000000 | 9.65 |


#### note_dice (30 text draws visible; later filled rects cover (fraction of ink box): `ACID` 1.00, `MIDI` 1.00, `1` 1.00)
| # | string | font | x,y(line top) | advance px | ink box x0,y0,x1,y1 | ink w x h | cap mm | arcmin@35 | arcmin@50 | fg (token) | bg passed (token) | fg/bg |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| 1 | `120` | M | 16,1 | 27 | 17,4,43,17 | 26x13 | 1.375 | 13.5 | 9.5 | TEXT #FFFAFF | BG #000000 | 20.37 |
| 2 | `1` | M | 48,1 | 9 | 49,5,54,16 | 5x11 | 1.375 | 13.5 | 9.5 | BG #000000 | CH1 #E69D00 | 9.19 |
| 3 | `2` | M | 62,1 | 9 | 62,4,71,16 | 9x12 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 4 | `3` | M | 76,1 | 9 | 76,4,85,17 | 9x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 5 | `4` | M | 90,1 | 9 | 90,5,99,16 | 9x11 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 6 | `PITCH` | M | 9,23 | 42 | 9,26,50,39 | 41x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 7 | `F2` | L | 13,36 | 33 | 14,43,45,64 | 31x21 | 2.625 | 25.8 | 18.0 | WHITE #FFFFFF | CHF1 #6A4800 | 8.27 |
| 8 | `GEN C2` | S | 9,65 | 42 | 9,68,51,78 | 42x10 | 1.125 | 11.1 | 7.7 | TEXT #FFFAFF | CHF1 #6A4800 | 8.02 |
| 9 | `GATE` | M | 72,23 | 36 | 72,26,108,39 | 36x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 10 | `100` | L | 63,36 | 54 | 66,43,116,65 | 50x22 | 2.625 | 25.8 | 18.0 | WHITE #FFFFFF | CHF1 #6A4800 | 8.27 |
| 11 | `GEN 40` | S | 70,65 | 40 | 70,68,110,78 | 40x10 | 1.125 | 11.1 | 7.7 | TEXT #FFFAFF | CHF1 #6A4800 | 8.02 |
| 12 | `VELO` | M | 131,23 | 37 | 131,26,167,39 | 36x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 13 | `80` | L | 132,36 | 36 | 133,43,167,65 | 34x22 | 2.625 | 25.8 | 18.0 | WHITE #FFFFFF | CHF1 #6A4800 | 8.27 |
| 14 | `GEN 43` | S | 130,65 | 40 | 130,68,170,78 | 40x10 | 1.125 | 11.1 | 7.7 | TEXT #FFFAFF | CHF1 #6A4800 | 8.02 |
| 15 | `RATCH` | M | 186,23 | 47 | 186,26,233,39 | 47x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 16 | `AUTO` | M | 190,45 | 40 | 190,48,230,61 | 40x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 17 | `GEN 1X` | S | 190,65 | 40 | 190,68,231,78 | 41x10 | 1.125 | 11.1 | 7.7 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 18 | `STEP` | S | 3,79 | 28 | 3,82,31,92 | 28x10 | 1.125 | 11.1 | 7.7 | MID #B4AEB4 | BG #000000 | 9.65 |
| 19 | `9` | M | 34,76 | 9 | 34,79,43,92 | 9x13 | 1.375 | 13.5 | 9.5 | WHITE #FFFFFF | BG #000000 | 21.00 |
| 20 | `NO SLIDE` | S | 50,78 | 50 | 50,81,100,91 | 50x10 | 1.125 | 11.1 | 7.7 | BG #000000 | CH1 #E69D00 | 9.19 |
| 21 | `PROB 50` | S | 106,78 | 47 | 106,81,153,91 | 47x10 | 1.125 | 11.1 | 7.7 | BG #000000 | CH1 #E69D00 | 9.19 |
| 22 | `OCT-2` | S | 159,78 | 35 | 159,81,194,91 | 35x10 | 1.125 | 11.1 | 7.7 | BG #000000 | CH1 #E69D00 | 9.19 |
| 23 | `S` | S | 116,189 | 7 | 116,192,123,202 | 7x10 | 1.125 | 11.1 | 7.7 | TEXT #FFFAFF | RAISE #414041 | 10.01 |
| 24 | `F` | S | 131,189 | 7 | 131,192,137,201 | 6x9 | 1.125 | 11.1 | 7.7 | TEXT #FFFAFF | RAISE #414041 | 10.01 |
| 25 | `R` | S | 146,189 | 7 | 146,192,153,201 | 7x9 | 1.125 | 11.1 | 7.7 | TEXT #FFFAFF | RAISE #414041 | 10.01 |
| 26 | `KNOBS EDIT` | M | 2,221 | 83 | 2,224,85,237 | 83x13 | 1.375 | 13.5 | 9.5 | TEXT #FFFAFF | BG #000000 | 20.37 |
| 27 | `HOME = DONE` | M | 139,221 | 99 | 139,224,237,237 | 98x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | BG #000000 | 9.65 |
| 28 | `1` | M | 109,2 | 9 | 110,6,115,17 | 5x11 | 1.375 | 13.5 | 9.5 | BG #000000 | CH1 #E69D00 | 9.19 |
| 29 | `ALL` | M | 124,1 | 26 | 124,5,149,16 | 25x11 | 1.375 | 13.5 | 9.5 | BG #000000 | WHITE #FFFFFF | 21.00 |
| 30 | `EDIT=UNDO` | S | 172,4 | 65 | 172,7,237,17 | 65x10 | 1.125 | 11.1 | 7.7 | BG #000000 | WHITE #FFFFFF | 21.00 |


#### note_final (28 text draws visible; later filled rects cover (fraction of ink box): none)
| # | string | font | x,y(line top) | advance px | ink box x0,y0,x1,y1 | ink w x h | cap mm | arcmin@35 | arcmin@50 | fg (token) | bg passed (token) | fg/bg |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| 1 | `120` | M | 16,1 | 27 | 17,4,43,17 | 26x13 | 1.375 | 13.5 | 9.5 | TEXT #FFFAFF | BG #000000 | 20.37 |
| 2 | `1` | M | 48,1 | 9 | 49,5,54,16 | 5x11 | 1.375 | 13.5 | 9.5 | BG #000000 | CH1 #E69D00 | 9.19 |
| 3 | `2` | M | 62,1 | 9 | 62,4,71,16 | 9x12 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 4 | `3` | M | 76,1 | 9 | 76,4,85,17 | 9x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 5 | `4` | M | 90,1 | 9 | 90,5,99,16 | 9x11 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 6 | `ACID` | M | 105,1 | 33 | 105,4,138,17 | 33x13 | 1.375 | 13.5 | 9.5 | CH1 #E69D00 | BG #000000 | 9.19 |
| 7 | `MIDI` | S | 143,4 | 23 | 143,7,166,16 | 23x9 | 1.125 | 11.1 | 7.7 | MID #B4AEB4 | BG #000000 | 9.65 |
| 8 | `1` | M | 168,1 | 9 | 169,5,174,16 | 5x11 | 1.375 | 13.5 | 9.5 | WHITE #FFFFFF | BG #000000 | 21.00 |
| 9 | `PITCH` | M | 9,23 | 42 | 9,26,50,39 | 41x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 10 | `AUTO` | M | 10,45 | 40 | 10,48,50,61 | 40x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 11 | `GEN C2` | S | 9,65 | 42 | 9,68,51,78 | 42x10 | 1.125 | 11.1 | 7.7 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 12 | `GATE` | M | 72,23 | 36 | 72,26,108,39 | 36x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 13 | `AUTO` | M | 70,45 | 40 | 70,48,110,61 | 40x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 14 | `GEN 40` | S | 70,65 | 40 | 70,68,110,78 | 40x10 | 1.125 | 11.1 | 7.7 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 15 | `VELO` | M | 131,23 | 37 | 131,26,167,39 | 36x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 16 | `83` | L | 132,36 | 36 | 133,43,167,65 | 34x22 | 2.625 | 25.8 | 18.0 | WHITE #FFFFFF | CHF1 #6A4800 | 8.27 |
| 17 | `GEN 43` | S | 130,65 | 40 | 130,68,170,78 | 40x10 | 1.125 | 11.1 | 7.7 | TEXT #FFFAFF | CHF1 #6A4800 | 8.02 |
| 18 | `RATCH` | M | 186,23 | 47 | 186,26,233,39 | 47x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 19 | `AUTO` | M | 190,45 | 40 | 190,48,230,61 | 40x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 20 | `GEN 1X` | S | 190,65 | 40 | 190,68,231,78 | 41x10 | 1.125 | 11.1 | 7.7 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 21 | `STEP` | S | 3,79 | 28 | 3,82,31,92 | 28x10 | 1.125 | 11.1 | 7.7 | MID #B4AEB4 | BG #000000 | 9.65 |
| 22 | `9` | M | 34,76 | 9 | 34,79,43,92 | 9x13 | 1.375 | 13.5 | 9.5 | WHITE #FFFFFF | BG #000000 | 21.00 |
| 23 | `= SET BY YOU` | S | 162,79 | 75 | 163,82,237,92 | 74x10 | 1.125 | 11.1 | 7.7 | MID #B4AEB4 | BG #000000 | 9.65 |
| 24 | `S` | S | 116,189 | 7 | 116,192,123,202 | 7x10 | 1.125 | 11.1 | 7.7 | TEXT #FFFAFF | RAISE #414041 | 10.01 |
| 25 | `F` | S | 131,189 | 7 | 131,192,137,201 | 6x9 | 1.125 | 11.1 | 7.7 | TEXT #FFFAFF | RAISE #414041 | 10.01 |
| 26 | `R` | S | 146,189 | 7 | 146,192,153,201 | 7x9 | 1.125 | 11.1 | 7.7 | TEXT #FFFAFF | RAISE #414041 | 10.01 |
| 27 | `KNOBS EDIT` | M | 2,221 | 83 | 2,224,85,237 | 83x13 | 1.375 | 13.5 | 9.5 | TEXT #FFFAFF | BG #000000 | 20.37 |
| 28 | `HOME = DONE` | M | 139,221 | 99 | 139,224,237,237 | 98x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | BG #000000 | 9.65 |


#### worst_w1 (31 text draws visible; later filled rects cover (fraction of ink box): none)
| # | string | font | x,y(line top) | advance px | ink box x0,y0,x1,y1 | ink w x h | cap mm | arcmin@35 | arcmin@50 | fg (token) | bg passed (token) | fg/bg |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| 1 | `120` | M | 16,1 | 27 | 17,4,43,17 | 26x13 | 1.375 | 13.5 | 9.5 | TEXT #FFFAFF | BG #000000 | 20.37 |
| 2 | `1` | M | 48,1 | 9 | 49,5,54,16 | 5x11 | 1.375 | 13.5 | 9.5 | BG #000000 | CH1 #E69D00 | 9.19 |
| 3 | `2` | M | 62,1 | 9 | 62,4,71,16 | 9x12 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 4 | `3` | M | 76,1 | 9 | 76,4,85,17 | 9x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 5 | `4` | M | 90,1 | 9 | 90,5,99,16 | 9x11 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 6 | `ACID` | M | 105,1 | 33 | 105,4,138,17 | 33x13 | 1.375 | 13.5 | 9.5 | CH1 #E69D00 | BG #000000 | 9.19 |
| 7 | `MIDI` | S | 143,4 | 23 | 143,7,166,16 | 23x9 | 1.125 | 11.1 | 7.7 | MID #B4AEB4 | BG #000000 | 9.65 |
| 8 | `1` | M | 168,1 | 9 | 169,5,174,16 | 5x11 | 1.375 | 13.5 | 9.5 | WHITE #FFFFFF | BG #000000 | 21.00 |
| 9 | `PITCH` | M | 9,23 | 42 | 9,26,50,39 | 41x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 10 | `C#-1` | M | 12,45 | 35 | 12,48,44,61 | 32x13 | 1.375 | 13.5 | 9.5 | WHITE #FFFFFF | CHF1 #6A4800 | 8.27 |
| 11 | `GEN G#-1` | S | 3,65 | 54 | 3,68,55,78 | 52x10 | 1.125 | 11.1 | 7.7 | TEXT #FFFAFF | CHF1 #6A4800 | 8.02 |
| 12 | `GATE` | M | 72,23 | 36 | 72,26,108,39 | 36x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 13 | `100` | L | 63,36 | 54 | 66,43,116,65 | 50x22 | 2.625 | 25.8 | 18.0 | WHITE #FFFFFF | CHF1 #6A4800 | 8.27 |
| 14 | `GEN 100` | S | 66,65 | 47 | 66,68,113,78 | 47x10 | 1.125 | 11.1 | 7.7 | TEXT #FFFAFF | CHF1 #6A4800 | 8.02 |
| 15 | `VELO` | M | 131,23 | 37 | 131,26,167,39 | 36x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 16 | `ACC` | M | 135,45 | 30 | 135,48,165,61 | 30x13 | 1.375 | 13.5 | 9.5 | WHITE #FFFFFF | CHF1 #6A4800 | 8.27 |
| 17 | `GEN 99` | S | 130,65 | 40 | 130,68,170,78 | 40x10 | 1.125 | 11.1 | 7.7 | TEXT #FFFAFF | CHF1 #6A4800 | 8.02 |
| 18 | `RATCH` | M | 186,23 | 47 | 186,26,233,39 | 47x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 19 | `4X` | L | 192,36 | 36 | 193,43,228,64 | 35x21 | 2.625 | 25.8 | 18.0 | WHITE #FFFFFF | CHF1 #6A4800 | 8.27 |
| 20 | `GEN 4X` | S | 190,65 | 40 | 190,68,231,78 | 41x10 | 1.125 | 11.1 | 7.7 | TEXT #FFFAFF | CHF1 #6A4800 | 8.02 |
| 21 | `STEP` | S | 3,79 | 28 | 3,82,31,92 | 28x10 | 1.125 | 11.1 | 7.7 | MID #B4AEB4 | BG #000000 | 9.65 |
| 22 | `64` | M | 34,76 | 18 | 34,79,52,92 | 18x13 | 1.375 | 13.5 | 9.5 | WHITE #FFFFFF | BG #000000 | 21.00 |
| 23 | `NO SLIDE` | S | 59,78 | 50 | 59,81,109,91 | 50x10 | 1.125 | 11.1 | 7.7 | BG #000000 | CH1 #E69D00 | 9.19 |
| 24 | `PROB 50` | S | 115,78 | 47 | 115,81,162,91 | 47x10 | 1.125 | 11.1 | 7.7 | BG #000000 | CH1 #E69D00 | 9.19 |
| 25 | `OCT+2` | S | 168,78 | 37 | 168,81,205,91 | 37x10 | 1.125 | 11.1 | 7.7 | BG #000000 | CH1 #E69D00 | 9.19 |
| 26 | `OFF` | S | 211,78 | 22 | 211,81,232,91 | 21x10 | 1.125 | 11.1 | 7.7 | BG #000000 | CH1 #E69D00 | 9.19 |
| 27 | `S` | S | 116,189 | 7 | 116,192,123,202 | 7x10 | 1.125 | 11.1 | 7.7 | TEXT #FFFAFF | RAISE #414041 | 10.01 |
| 28 | `F` | S | 131,189 | 7 | 131,192,137,201 | 6x9 | 1.125 | 11.1 | 7.7 | TEXT #FFFAFF | RAISE #414041 | 10.01 |
| 29 | `R` | S | 146,189 | 7 | 146,192,153,201 | 7x9 | 1.125 | 11.1 | 7.7 | TEXT #FFFAFF | RAISE #414041 | 10.01 |
| 30 | `KNOBS EDIT` | M | 2,221 | 83 | 2,224,85,237 | 83x13 | 1.375 | 13.5 | 9.5 | TEXT #FFFAFF | BG #000000 | 20.37 |
| 31 | `HOME = DONE` | M | 139,221 | 99 | 139,224,237,237 | 98x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | BG #000000 | 9.65 |


#### worst_w2 (28 text draws visible; later filled rects cover (fraction of ink box): none)
| # | string | font | x,y(line top) | advance px | ink box x0,y0,x1,y1 | ink w x h | cap mm | arcmin@35 | arcmin@50 | fg (token) | bg passed (token) | fg/bg |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| 1 | `120` | M | 16,1 | 27 | 17,4,43,17 | 26x13 | 1.375 | 13.5 | 9.5 | TEXT #FFFAFF | BG #000000 | 20.37 |
| 2 | `1` | M | 48,1 | 9 | 49,5,54,16 | 5x11 | 1.375 | 13.5 | 9.5 | BG #000000 | CH1 #E69D00 | 9.19 |
| 3 | `2` | M | 62,1 | 9 | 62,4,71,16 | 9x12 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 4 | `3` | M | 76,1 | 9 | 76,4,85,17 | 9x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 5 | `4` | M | 90,1 | 9 | 90,5,99,16 | 9x11 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 6 | `ACID` | M | 105,1 | 33 | 105,4,138,17 | 33x13 | 1.375 | 13.5 | 9.5 | CH1 #E69D00 | BG #000000 | 9.19 |
| 7 | `MIDI` | S | 143,4 | 23 | 143,7,166,16 | 23x9 | 1.125 | 11.1 | 7.7 | MID #B4AEB4 | BG #000000 | 9.65 |
| 8 | `1` | M | 168,1 | 9 | 169,5,174,16 | 5x11 | 1.375 | 13.5 | 9.5 | WHITE #FFFFFF | BG #000000 | 21.00 |
| 9 | `PITCH` | M | 9,23 | 42 | 9,26,50,39 | 41x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 10 | `A#-1` | M | 13,45 | 34 | 13,49,44,60 | 31x11 | 1.375 | 13.5 | 9.5 | WHITE #FFFFFF | CHF1 #6A4800 | 8.27 |
| 11 | `GEN D#-1` | S | 3,65 | 53 | 3,68,54,78 | 51x10 | 1.125 | 11.1 | 7.7 | TEXT #FFFAFF | CHF1 #6A4800 | 8.02 |
| 12 | `GATE` | M | 72,23 | 36 | 72,26,108,39 | 36x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 13 | `TIE` | L | 70,36 | 40 | 70,43,109,64 | 39x21 | 2.625 | 25.8 | 18.0 | WHITE #FFFFFF | CHF1 #6A4800 | 8.27 |
| 14 | `GEN TIE` | S | 68,65 | 43 | 68,68,111,78 | 43x10 | 1.125 | 11.1 | 7.7 | TEXT #FFFAFF | CHF1 #6A4800 | 8.02 |
| 15 | `VELO` | M | 131,23 | 37 | 131,26,167,39 | 36x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 16 | `ACC` | M | 135,45 | 30 | 135,48,165,61 | 30x13 | 1.375 | 13.5 | 9.5 | WHITE #FFFFFF | CHF1 #6A4800 | 8.27 |
| 17 | `GEN ACC` | S | 125,65 | 50 | 125,68,175,78 | 50x10 | 1.125 | 11.1 | 7.7 | TEXT #FFFAFF | CHF1 #6A4800 | 8.02 |
| 18 | `RATCH` | M | 186,23 | 47 | 186,26,233,39 | 47x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 19 | `AUTO` | M | 190,45 | 40 | 190,48,230,61 | 40x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 20 | `GEN 3X` | S | 190,65 | 40 | 190,68,231,78 | 41x10 | 1.125 | 11.1 | 7.7 | MID #B4AEB4 | SURF #292829 | 6.75 |
| 21 | `STEP` | S | 3,79 | 28 | 3,82,31,92 | 28x10 | 1.125 | 11.1 | 7.7 | MID #B4AEB4 | BG #000000 | 9.65 |
| 22 | `1` | M | 34,76 | 9 | 35,80,40,91 | 5x11 | 1.375 | 13.5 | 9.5 | WHITE #FFFFFF | BG #000000 | 21.00 |
| 23 | `= SET BY YOU` | S | 162,79 | 75 | 163,82,237,92 | 74x10 | 1.125 | 11.1 | 7.7 | MID #B4AEB4 | BG #000000 | 9.65 |
| 24 | `S` | S | 116,189 | 7 | 116,192,123,202 | 7x10 | 1.125 | 11.1 | 7.7 | TEXT #FFFAFF | RAISE #414041 | 10.01 |
| 25 | `F` | S | 131,189 | 7 | 131,192,137,201 | 6x9 | 1.125 | 11.1 | 7.7 | TEXT #FFFAFF | RAISE #414041 | 10.01 |
| 26 | `R` | S | 146,189 | 7 | 146,192,153,201 | 7x9 | 1.125 | 11.1 | 7.7 | TEXT #FFFAFF | RAISE #414041 | 10.01 |
| 27 | `KNOBS EDIT` | M | 2,221 | 83 | 2,224,85,237 | 83x13 | 1.375 | 13.5 | 9.5 | TEXT #FFFAFF | BG #000000 | 20.37 |
| 28 | `HOME = DONE` | M | 139,221 | 99 | 139,224,237,237 | 98x13 | 1.375 | 13.5 | 9.5 | MID #B4AEB4 | BG #000000 | 9.65 |


### Icons and glyphs (1 bpp, non-text) per render
- note_open: 20 icons: SQI_STOP_S 9x9@3,6 MID, SQI_DICE_ALL 13x13@13,148 WHITE, SQI_DICE_NOTES 13x13@53,148 WHITE, SQI_DICE_ACCNT 13x13@93,148 WHITE, SQI_DICE_SHAPE 13x13@133,148 WHITE, SQI_UNDO 13x11@173,149 TEXT, SQI_SETTINGS 13x11@213,149 TEXT, SQI_HOME 13x11@13,167 TEXT, SQI_SAVE 13x11@53,167 TEXT, SQI_DICE_ARTIC 13x13@93,166 WHITE, SQI_DICE_HITS 13x13@133,166 WHITE, SQI_PLAY 13x11@173,167 TEXT, SQI_REC_RING 13x11@213,167 TEXT, SQI_X7 7x7@27,193 MID, SQI_X7 7x7@42,193 MID, SQI_X7 7x7@72,193 MID, SQI_MIDI_S 7x5@87,194 TEXT, SQI_LEFT 5x7@178,193 TEXT, SQI_RIGHT 5x7@193,193 TEXT, SQI_RST 5x7@223,193 TEXT
- note_set: 20 icons: SQI_STOP_S 9x9@3,6 MID, SQI_DICE_ALL 13x13@13,148 WHITE, SQI_DICE_NOTES 13x13@53,148 WHITE, SQI_DICE_ACCNT 13x13@93,148 WHITE, SQI_DICE_SHAPE 13x13@133,148 WHITE, SQI_UNDO 13x11@173,149 TEXT, SQI_SETTINGS 13x11@213,149 TEXT, SQI_HOME 13x11@13,167 TEXT, SQI_SAVE 13x11@53,167 TEXT, SQI_DICE_ARTIC 13x13@93,166 WHITE, SQI_DICE_HITS 13x13@133,166 WHITE, SQI_PLAY 13x11@173,167 TEXT, SQI_REC_RING 13x11@213,167 TEXT, SQI_X7 7x7@27,193 MID, SQI_X7 7x7@42,193 MID, SQI_X7 7x7@72,193 MID, SQI_MIDI_S 7x5@87,194 TEXT, SQI_LEFT 5x7@178,193 TEXT, SQI_RIGHT 5x7@193,193 TEXT, SQI_RST 5x7@223,193 TEXT
- note_dice: 20 icons: SQI_STOP_S 9x9@3,6 MID, SQI_DICE_ALL 13x13@13,148 WHITE, SQI_DICE_NOTES 13x13@53,148 WHITE, SQI_DICE_ACCNT 13x13@93,148 WHITE, SQI_DICE_SHAPE 13x13@133,148 WHITE, SQI_UNDO 13x11@173,149 TEXT, SQI_SETTINGS 13x11@213,149 TEXT, SQI_HOME 13x11@13,167 TEXT, SQI_SAVE 13x11@53,167 TEXT, SQI_DICE_ARTIC 13x13@93,166 WHITE, SQI_DICE_HITS 13x13@133,166 WHITE, SQI_PLAY 13x11@173,167 TEXT, SQI_REC_RING 13x11@213,167 TEXT, SQI_X7 7x7@27,193 MID, SQI_X7 7x7@42,193 MID, SQI_X7 7x7@72,193 MID, SQI_MIDI_S 7x5@87,194 TEXT, SQI_LEFT 5x7@178,193 TEXT, SQI_RIGHT 5x7@193,193 TEXT, SQI_RST 5x7@223,193 TEXT
- note_final: 20 icons: SQI_STOP_S 9x9@3,6 MID, SQI_DICE_ALL 13x13@13,148 WHITE, SQI_DICE_NOTES 13x13@53,148 WHITE, SQI_DICE_ACCNT 13x13@93,148 WHITE, SQI_DICE_SHAPE 13x13@133,148 WHITE, SQI_UNDO 13x11@173,149 TEXT, SQI_SETTINGS 13x11@213,149 TEXT, SQI_HOME 13x11@13,167 TEXT, SQI_SAVE 13x11@53,167 TEXT, SQI_DICE_ARTIC 13x13@93,166 WHITE, SQI_DICE_HITS 13x13@133,166 WHITE, SQI_PLAY 13x11@173,167 TEXT, SQI_REC_RING 13x11@213,167 TEXT, SQI_X7 7x7@27,193 MID, SQI_X7 7x7@42,193 MID, SQI_X7 7x7@72,193 MID, SQI_MIDI_S 7x5@87,194 TEXT, SQI_LEFT 5x7@178,193 TEXT, SQI_RIGHT 5x7@193,193 TEXT, SQI_RST 5x7@223,193 TEXT


Persistent vs page-specific: the header (`120`, chips `1`..`4`, `ACID`, `MIDI`, `1`), the four dice icons + EDIT/GLO/HOME/SAVE/ARP/SEQ/PLAY/REC icons, the black-key glyphs (`S F R`, MIDI glyph, arrows, RST, muted `x` icons) and the 16 white keys
are drawn by the shared element code; the knob tiles, status line and footer are NOTE_EDIT's (section 6 lists the strings that exist on one page only).


## 2. Contrast (HI-CON)

Flag rules: text under 4.5:1, non-text under 3:1.

### tokens on HI-CON (RGB565 -> sRGB as png_write expands)
| token(s) | RGB565 | hex |
|---|---|---|
| BG = INK = INK | 0x0000 | #000000 |
| SURF | 0x2945 | #292829 |
| TEXT | 0xFFDF | #FFFAFF |
| THEME | 0xFF40 | #FFEA00 |
| ACCENT | 0x073F | #00E6FF |
| MID | 0xB576 | #B4AEB4 |
| DIM | 0x6B4D | #6A696A |
| LINE | 0x3166 | #312C31 |
| SEL | 0xCDC0 | #CDBA00 |
| TINT | 0x20E0 | #201C00 |
| REC | 0xFA49 | #FF484A |
| RAISE | 0x4208 | #414041 |
| KEY | 0xC618 | #C5C2C5 |
| LANE | 0x41E4 | #413C20 |
| GRID | 0x39A7 | #393439 |
| CH1 | 0xE4E0 | #E69D00 |
| CH2 | 0x55BD | #52B6EE |
| CH3 | 0x04EE | #009D73 |
| CH4 | 0xCBD4 | #CD79A4 |
| CHF1 | 0x6A40 | #6A4800 |
| CHF2 | 0x2AAD | #29556A |
| CHF3 | 0x0246 | #004831 |
| CHF4 | 0x61C9 | #62384A |
| PH | 0x528A | #525052 |
| DICE | 0x4A49 | #4A484A |
| OFF | 0xD945 | #DE2829 |
| WHITE | 0xFFFF | #FFFFFF |

**Text pairs.** The fg/bg ratio of every text draw is in the last column of the section-1 tables; the actual pixel colour under each ink box equals the stated background in every case (0 mismatches over the six frames, `scripts/s2_contrast.py` (a)). Distinct pairs:

| fg | bg | ratio | <4.5? | font(s) | users (render:string, first 8) |
|---|---|---|---|---|---|
| MID #B4AEB4 | SURF #292829 | 6.75 | no | M/S | open:2, open:3, open:4, open:PITCH, open:AUTO, open:GATE, open:AUTO, open:VELO ... |
| TEXT #FFFAFF | CHF1 #6A4800 | 8.02 | no | S | dice:GEN C2, dice:GEN 40, dice:GEN 43, final:GEN 43, worst_w1:GEN G#-1, worst_w1:GEN 100, worst_w1:GEN 99, worst_w1:GEN 4X ... |
| WHITE #FFFFFF | CHF1 #6A4800 | 8.27 | no | L/M | dice:F2, dice:100, dice:80, final:83, worst_w1:100, worst_w1:4X, worst_w2:TIE, worst_w1:C#-1 ... |
| BG #000000 | CH1 #E69D00 | 9.19 | no | M/S | open:1, set:1, dice:1, dice:1, final:1, worst_w1:1, worst_w2:1, set:SLIDE ... |
| CH1 #E69D00 | BG #000000 | 9.19 | no | M | open:ACID, set:ACID, final:ACID, worst_w1:ACID, worst_w2:ACID |
| MID #B4AEB4 | BG #000000 | 9.65 | no | M/S | open:MIDI, open:STEP, open:= SET BY YOU, set:MIDI, set:STEP, dice:STEP, final:MIDI, final:STEP ... |
| TEXT #FFFAFF | RAISE #414041 | 10.01 | no | S | open:S, open:F, open:R, set:S, set:F, set:R, dice:S, dice:F ... |
| TEXT #FFFAFF | BG #000000 | 20.37 | no | M | open:120, open:KNOBS EDIT, set:120, set:KNOBS EDIT, dice:120, dice:KNOBS EDIT, final:120, final:KNOBS EDIT ... |
| WHITE #FFFFFF | BG #000000 | 21.00 | no | M | open:1, open:9, set:1, set:9, dice:9, final:1, final:9, worst_w1:1 ... |
| BG #000000 | WHITE #FFFFFF | 21.00 | no | M/S | dice:ALL, dice:EDIT=UNDO |

Text pairs under 4.5:1: **none** (lowest: MID on SURF 6.75, used for the tile labels, AUTO, the GEN lines of AUTO tiles, header chips 2-4).

**Non-text pairs that touch on the final frame** (4-neighbour edges, text ink masked; edge px = number of touching pixel pairs). Rows flagged `!` are under 3:1.


#### note_open: 18 distinct touching pairs
| colour A | colour B | ratio | edge px | first at | flag |
|---|---|---|---|---|---|
| BG #000000 | SURF #292829 | 1.43 | 2825 | 60,0 | ! |
| BG #000000 | RAISE #414041 | 2.03 | 604 | 10,185 | ! |
| CH1 #E69D00 | WHITE #FFFFFF | 2.29 | 71 | 118,99 | ! |
| BG #000000 | DICE #4A484A | 2.32 | 660 | 1,145 | ! |
| CHF1 #6A4800 | CH1 #E69D00 | 3.62 | 16 | 155,82 |  |
| BG #000000 | DIM #6A696A | 3.84 | 547 | 141,122 |  |
| RAISE #414041 | CH1 #E69D00 | 4.52 | 26 | 12,192 |  |
| RAISE #414041 | MID #B4AEB4 | 4.75 | 132 | 27,192 |  |
| DIM #6A696A | WHITE #FFFFFF | 5.47 | 19 | 120,130 |  |
| SURF #292829 | CH1 #E69D00 | 6.43 | 180 | 1,206 |  |
| SURF #292829 | MID #B4AEB4 | 6.75 | 36 | 71,1 |  |
| DICE #4A484A | WHITE #FFFFFF | 9.06 | 848 | 14,147 |  |
| BG #000000 | CH1 #E69D00 | 9.19 | 1199 | 46,0 |  |
| BG #000000 | MID #B4AEB4 | 9.65 | 48 | 72,0 |  |
| RAISE #414041 | TEXT #FFFAFF | 10.01 | 126 | 182,192 |  |
| SURF #292829 | TEXT #FFFAFF | 14.25 | 412 | 218,148 |  |
| SURF #292829 | WHITE #FFFFFF | 14.69 | 5 | 126,186 |  |
| BG #000000 | WHITE #FFFFFF | 21.00 | 299 | 46,20 |  |

#### note_set: 17 distinct touching pairs
| colour A | colour B | ratio | edge px | first at | flag |
|---|---|---|---|---|---|
| BG #000000 | SURF #292829 | 1.43 | 2644 | 60,0 | ! |
| BG #000000 | RAISE #414041 | 2.03 | 604 | 10,185 | ! |
| CH1 #E69D00 | WHITE #FFFFFF | 2.29 | 25 | 118,99 | ! |
| BG #000000 | DICE #4A484A | 2.32 | 660 | 1,145 | ! |
| BG #000000 | DIM #6A696A | 3.84 | 555 | 126,122 |  |
| RAISE #414041 | CH1 #E69D00 | 4.52 | 26 | 12,192 |  |
| RAISE #414041 | MID #B4AEB4 | 4.75 | 132 | 27,192 |  |
| DIM #6A696A | WHITE #FFFFFF | 5.47 | 19 | 120,130 |  |
| SURF #292829 | CH1 #E69D00 | 6.43 | 288 | 182,24 |  |
| SURF #292829 | MID #B4AEB4 | 6.75 | 36 | 71,1 |  |
| DICE #4A484A | WHITE #FFFFFF | 9.06 | 848 | 14,147 |  |
| BG #000000 | CH1 #E69D00 | 9.19 | 1409 | 46,0 |  |
| BG #000000 | MID #B4AEB4 | 9.65 | 48 | 72,0 |  |
| RAISE #414041 | TEXT #FFFAFF | 10.01 | 126 | 182,192 |  |
| SURF #292829 | TEXT #FFFAFF | 14.25 | 412 | 218,148 |  |
| SURF #292829 | WHITE #FFFFFF | 14.69 | 39 | 126,186 |  |
| BG #000000 | WHITE #FFFFFF | 21.00 | 311 | 46,20 |  |

#### note_dice: 19 distinct touching pairs
| colour A | colour B | ratio | edge px | first at | flag |
|---|---|---|---|---|---|
| BG #000000 | SURF #292829 | 1.43 | 2208 | 60,0 | ! |
| BG #000000 | RAISE #414041 | 2.03 | 604 | 10,185 | ! |
| CH1 #E69D00 | WHITE #FFFFFF | 2.29 | 87 | 107,1 | ! |
| BG #000000 | DICE #4A484A | 2.32 | 660 | 1,145 | ! |
| BG #000000 | CHF1 #6A4800 | 2.54 | 382 | 8,40 | ! |
| CHF1 #6A4800 | CH1 #E69D00 | 3.62 | 44 | 7,41 |  |
| BG #000000 | DIM #6A696A | 3.84 | 555 | 126,122 |  |
| RAISE #414041 | CH1 #E69D00 | 4.52 | 26 | 12,192 |  |
| RAISE #414041 | MID #B4AEB4 | 4.75 | 132 | 27,192 |  |
| DIM #6A696A | WHITE #FFFFFF | 5.47 | 19 | 120,130 |  |
| SURF #292829 | CH1 #E69D00 | 6.43 | 288 | 182,24 |  |
| SURF #292829 | MID #B4AEB4 | 6.75 | 36 | 71,1 |  |
| DICE #4A484A | WHITE #FFFFFF | 9.06 | 848 | 14,147 |  |
| BG #000000 | CH1 #E69D00 | 9.19 | 1474 | 46,0 |  |
| BG #000000 | MID #B4AEB4 | 9.65 | 48 | 72,0 |  |
| RAISE #414041 | TEXT #FFFAFF | 10.01 | 126 | 182,192 |  |
| SURF #292829 | TEXT #FFFAFF | 14.25 | 412 | 218,148 |  |
| SURF #292829 | WHITE #FFFFFF | 14.69 | 39 | 126,186 |  |
| BG #000000 | WHITE #FFFFFF | 21.00 | 469 | 103,0 |  |

#### note_final: 19 distinct touching pairs
| colour A | colour B | ratio | edge px | first at | flag |
|---|---|---|---|---|---|
| BG #000000 | SURF #292829 | 1.43 | 2625 | 60,0 | ! |
| BG #000000 | RAISE #414041 | 2.03 | 604 | 10,185 | ! |
| CH1 #E69D00 | WHITE #FFFFFF | 2.29 | 71 | 118,99 | ! |
| BG #000000 | DICE #4A484A | 2.32 | 660 | 1,145 | ! |
| BG #000000 | CHF1 #6A4800 | 2.54 | 44 | 128,40 | ! |
| CHF1 #6A4800 | CH1 #E69D00 | 3.62 | 112 | 127,41 |  |
| BG #000000 | DIM #6A696A | 3.84 | 555 | 21,122 |  |
| RAISE #414041 | CH1 #E69D00 | 4.52 | 26 | 12,192 |  |
| RAISE #414041 | MID #B4AEB4 | 4.75 | 132 | 27,192 |  |
| DIM #6A696A | WHITE #FFFFFF | 5.47 | 19 | 120,130 |  |
| SURF #292829 | CH1 #E69D00 | 6.43 | 214 | 122,24 |  |
| SURF #292829 | MID #B4AEB4 | 6.75 | 36 | 71,1 |  |
| DICE #4A484A | WHITE #FFFFFF | 9.06 | 848 | 14,147 |  |
| BG #000000 | CH1 #E69D00 | 9.19 | 1295 | 46,0 |  |
| BG #000000 | MID #B4AEB4 | 9.65 | 48 | 72,0 |  |
| RAISE #414041 | TEXT #FFFAFF | 10.01 | 126 | 182,192 |  |
| SURF #292829 | TEXT #FFFAFF | 14.25 | 412 | 218,148 |  |
| SURF #292829 | WHITE #FFFFFF | 14.69 | 5 | 126,186 |  |
| BG #000000 | WHITE #FFFFFF | 21.00 | 299 | 46,20 |  |

#### worst_w1: 19 distinct touching pairs
| colour A | colour B | ratio | edge px | first at | flag |
|---|---|---|---|---|---|
| BG #000000 | SURF #292829 | 1.43 | 2187 | 60,0 | ! |
| BG #000000 | RAISE #414041 | 2.03 | 604 | 10,185 | ! |
| CH1 #E69D00 | WHITE #FFFFFF | 2.29 | 46 | 225,118 | ! |
| BG #000000 | DICE #4A484A | 2.32 | 660 | 1,145 | ! |
| BG #000000 | CHF1 #6A4800 | 2.54 | 408 | 8,40 | ! |
| CHF1 #6A4800 | CH1 #E69D00 | 3.62 | 140 | 7,41 |  |
| BG #000000 | DIM #6A696A | 3.84 | 561 | 21,122 |  |
| RAISE #414041 | CH1 #E69D00 | 4.52 | 26 | 12,192 |  |
| RAISE #414041 | MID #B4AEB4 | 4.75 | 132 | 27,192 |  |
| DIM #6A696A | WHITE #FFFFFF | 5.47 | 13 | 225,130 |  |
| SURF #292829 | CH1 #E69D00 | 6.43 | 214 | 122,24 |  |
| SURF #292829 | MID #B4AEB4 | 6.75 | 36 | 71,1 |  |
| DICE #4A484A | WHITE #FFFFFF | 9.06 | 848 | 14,147 |  |
| BG #000000 | CH1 #E69D00 | 9.19 | 1519 | 46,0 |  |
| BG #000000 | MID #B4AEB4 | 9.65 | 48 | 72,0 |  |
| RAISE #414041 | TEXT #FFFAFF | 10.01 | 126 | 182,192 |  |
| SURF #292829 | TEXT #FFFAFF | 14.25 | 412 | 218,148 |  |
| SURF #292829 | WHITE #FFFFFF | 14.69 | 29 | 231,186 |  |
| BG #000000 | WHITE #FFFFFF | 21.00 | 262 | 46,20 |  |

#### worst_w2: 19 distinct touching pairs
| colour A | colour B | ratio | edge px | first at | flag |
|---|---|---|---|---|---|
| BG #000000 | SURF #292829 | 1.43 | 2312 | 60,0 | ! |
| BG #000000 | RAISE #414041 | 2.03 | 604 | 10,185 | ! |
| CH1 #E69D00 | WHITE #FFFFFF | 2.29 | 48 | 0,117 | ! |
| BG #000000 | DICE #4A484A | 2.32 | 660 | 1,145 | ! |
| BG #000000 | CHF1 #6A4800 | 2.54 | 286 | 8,40 | ! |
| CHF1 #6A4800 | CH1 #E69D00 | 3.62 | 138 | 7,41 |  |
| BG #000000 | DIM #6A696A | 3.84 | 555 | 21,122 |  |
| RAISE #414041 | CH1 #E69D00 | 4.52 | 26 | 12,192 |  |
| RAISE #414041 | MID #B4AEB4 | 4.75 | 132 | 27,192 |  |
| DIM #6A696A | WHITE #FFFFFF | 5.47 | 19 | 15,130 |  |
| SURF #292829 | CH1 #E69D00 | 6.43 | 214 | 122,24 |  |
| SURF #292829 | MID #B4AEB4 | 6.75 | 36 | 71,1 |  |
| DICE #4A484A | WHITE #FFFFFF | 9.06 | 848 | 14,147 |  |
| BG #000000 | CH1 #E69D00 | 9.19 | 1344 | 46,0 |  |
| BG #000000 | MID #B4AEB4 | 9.65 | 48 | 72,0 |  |
| RAISE #414041 | TEXT #FFFAFF | 10.01 | 126 | 182,192 |  |
| SURF #292829 | TEXT #FFFAFF | 14.25 | 412 | 218,148 |  |
| SURF #292829 | WHITE #FFFFFF | 14.69 | 28 | 2,186 |  |
| BG #000000 | WHITE #FFFFFF | 21.00 | 280 | 46,20 |  |

### (c) named non-text pairs that carry NOTE_EDIT meaning (not all touch; nominal tokens); '!' = under 3:1
| pair | A | B | ratio | flag |
|---|---|---|---|---|
| set tile fill vs AUTO tile fill (the shape cue that says 'set') | CHF1 #6A4800 | SURF #292829 | 1.78 | ! |
| set tile fill vs the BG gap between tiles | CHF1 #6A4800 | BG #000000 | 2.54 | ! |
| set tile level edge (2 px, both sides) vs the set fill | CH1 #E69D00 | CHF1 #6A4800 | 3.62 |  |
| AUTO tile fill vs BG | SURF #292829 | BG #000000 | 1.43 | ! |
| turned-tile frame (2 px) CH1 vs BG | CH1 #E69D00 | BG #000000 | 9.19 |  |
| turned-tile frame CH1 vs AUTO tile SURF | CH1 #E69D00 | SURF #292829 | 6.43 |  |
| turned-tile frame CH1 vs set fill CHF1 | CH1 #E69D00 | CHF1 #6A4800 | 3.62 |  |
| edited-step frame (music row) WHITE vs BG | WHITE #FFFFFF | BG #000000 | 21.00 |  |
| edited-step frame WHITE vs a CH1 bar it touches | WHITE #FFFFFF | CH1 #E69D00 | 2.29 | ! |
| edited-step frame WHITE vs rest dash DIM | WHITE #FFFFFF | DIM #6A696A | 5.47 |  |
| edited-key frame WHITE vs white key body SURF | WHITE #FFFFFF | SURF #292829 | 14.69 |  |
| edited-key frame WHITE vs key front CH1 | WHITE #FFFFFF | CH1 #E69D00 | 2.29 | ! |
| status chip fill CH1 vs BG | CH1 #E69D00 | BG #000000 | 9.19 |  |
| legend square fill CHF1 vs BG | CHF1 #6A4800 | BG #000000 | 2.54 | ! |
| legend square frame CH1 vs BG | CH1 #E69D00 | BG #000000 | 9.19 |  |
| legend square fill CHF1 vs the tile set fill CHF1 (same colour: the legend key IS the fill) | CHF1 #6A4800 | CHF1 #6A4800 | 1.00 | ! |
| key front (step on) CH1 vs white key SURF | CH1 #E69D00 | SURF #292829 | 6.43 |  |
| music bar CH1 vs BG | CH1 #E69D00 | BG #000000 | 9.19 |  |
| black key RAISE vs BG | RAISE #414041 | BG #000000 | 2.03 | ! |
| black key RAISE vs white key SURF | RAISE #414041 | SURF #292829 | 1.42 | ! |
| button tile (dice) DICE vs BG | DICE #4A484A | BG #000000 | 2.32 | ! |
| button tile (dice) DICE vs button tile SURF (grouping) | DICE #4A484A | SURF #292829 | 1.62 | ! |
| button tile SURF vs BG | SURF #292829 | BG #000000 | 1.43 | ! |
| dice icon WHITE on DICE | WHITE #FFFFFF | DICE #4A484A | 9.06 |  |
| icon TEXT on SURF | TEXT #FFFAFF | SURF #292829 | 14.25 |  |
| muted-chip digit MID on SURF (header 2 3 4) | MID #B4AEB4 | SURF #292829 | 6.75 |  |
| transport stop square MID on BG | MID #B4AEB4 | BG #000000 | 9.65 |  |
| rest dash DIM vs BG | DIM #6A696A | BG #000000 | 3.84 |  |
| toast slab WHITE vs BG (note_dice) | WHITE #FFFFFF | BG #000000 | 21.00 |  |


Non-text pairs under 3:1 present on all four frames: BG/SURF 1.43 (tile/button/key bodies against the screen), BG/RAISE 2.03 (black keys), CH1/WHITE 2.29 (the edit frame against the amber bars and the amber key front), BG/DICE 2.32. Present when a tile is set (`note_dice`, `note_final`): BG/CHF1 2.54 (the set fill against the gap). The set fill against the AUTO fill, CHF1/SURF, is 1.78 (they do not touch). Level edge CH1/CHF1 3.62 passes.

## 3. Grid, bands, alignment, overlap, clipping

Spec: `docs/design/SCREEN_SPEC_V2.md` section 2 (16 columns of 15 px = the white keys; 4 x 60 knob tiles; 6 x 40 buttons; bands: header 1-19 (+ 20-21 bar), tiles 24-76, status 77-91 (caps 80), music row 92-141, buttons 146-180, keybed 186-220, footer caps 225-236). Firmware strips 0-119 and 120-239.

**Summary (facts)**
- No text ink crosses y=120 on any of the six frames. Filled rects and frames that straddle y=120 are the music bars and the edit frame (13 px bars at x=15i+1, frame `119,93 17x49` + `120,94 15x47`), drawn by both strips by design.
- No two visible text ink boxes overlap on any frame. Tightest neighbours on a shared row: `MIDI` -> `1` 3 px, `STEP` -> step number 3 px (4 px on `1`), digits `2 3 4` 5 px, `ACID` -> `MIDI` 5 px.
- Texts reaching the right side: `HOME = DONE` lit to x=236 (3 px margin to x=239), `= SET BY YOU` to x=236, `RATCH` to 232, `GEN 1X` / `4X` to 230 / 227. Nothing reaches x>=238 or x<=1.
- The GEN line's last strong row is 76 = the last row of the tile's value area (rows 41..76): 0 px of padding below it on every tile; the faintest blend row (alpha under 0.5) lands on row 77, which is BG (the tile ends at 76). Label band: strong rows 27..37 in 24..39 (3 px above, 2 px below). AUTO (AF_M): strong rows 49..59 (value area 41..76). Set AF_L value: strong rows 43..63 (faint to 64), then the GEN line at 68..76 (4 rows between).
- Tile text ink centre vs the tile centre (tile x + 29), over the 72 tile texts on the six frames: offsets -2.0 (1 text), -1.5 (2), -1.0 (7), -0.5 (16), 0 (37), +0.5 (7), +1.0 (2) px.
- The edit frame: music row outer frame x=119..135 for column 9 (column x=120..134, bar x=121..133): it overhangs the column by 1 px each side, is adjacent (0 px gap) to the previous column's bar end (x=118) and the next column's bar start (x=136) wherever those steps are on, and touches the edited bar (inner ring x=120 / 134 beside the bar's x=121 / 133). On columns 1 and 16 the outer ring is at x=-1 / x=240 (outside 0..239; `W2`, `W1`).
- The edited key's white frame (x=121..134, y=186..218, the key's own 14 x 33 body) is partly covered by the black keys drawn after it (S at x=115..124 and F at x=130..139 with a 1 px BG surround, y=186..205): 41 of its 90 ring pixels are white on `note_open` (the top edge is visible at x=126..128 only; the left/right edges only from y=206).
- Status chips: rect `y=79 h=13`, text inset 2 px left; 2 px between chips; first chip x=48 for a 1-digit step, x=57 for a 2-digit step (the chip row moves 9 px right when the step number reaches 10). Chip left edges: `note_set` x=48, 85, 138 (x mod 15 = 3, 10, 3); `note_dice` 48, 104, 157 (3, 14, 7); W1 57, 113, 166, 209 (12, 8, 1, 14): none on a 15 px column boundary. The legend square sits at x=154,y=82 (6 x 6) and its text right edge at 237.
- Knob tiles x=1,61,121,181 (w58) = the 60 px pitch (4 columns each), +1 px; button tiles x=1,41,...,201 (w38, pitch 40), y=146 and 164 (h17, pitch 18); white keys x=15i+1 (w14, h33, y=186); button icons are centred in their tile within 1 px (13 x 13 dice: 12 left / 13 right, 2 top / 2 bottom; 13 x 11: 12 / 13, 3 / 3).
- Legend `= SET BY YOU` (+ its square) is drawn on `note_open`, `note_final`, `worst_w2`, and not on `note_set`, `note_dice`, `worst_w1` (the chip row ends at x=177, 196 and 235; the legend needs the row to end at x <= 154).


### 3a. Per-render text positions (lit pixel boxes, column index, band)


#### note_open

texts: band, lit pixel extents (x0,y0..x1,y1 inclusive), column index of the ink left edge (15 px columns), offset into that column, distance from the right edge
| string | font | band(by lit y0..y1) | lit box (any blend) | strong box (alpha>=0.5) | col (x0//15) | x0 mod 15 | right margin to x=239 | crosses y=120? | outside its band? |
|---|---|---|---|---|---|---|---|---|---|
| `120` | M | header | 17,4..42,16 | 18,5..41,15 | 1 | 2 | 197 | no |  |
| `1` | M | header | 49,5..53,15 | 50,5..53,15 | 3 | 4 | 186 | no |  |
| `2` | M | header | 62,4..70,15 | 63,4..70,15 | 4 | 2 | 169 | no |  |
| `3` | M | header | 76,4..84,16 | 76,4..84,16 | 5 | 1 | 155 | no |  |
| `4` | M | header | 90,5..98,15 | 90,5..98,15 | 6 | 0 | 141 | no |  |
| `ACID` | M | header | 105,4..137,16 | 105,5..137,15 | 7 | 0 | 102 | no |  |
| `MIDI` | S | header | 143,7..165,15 | 144,7..165,15 | 9 | 8 | 74 | no |  |
| `1` | M | header | 169,5..173,15 | 170,5..173,15 | 11 | 4 | 66 | no |  |
| `PITCH` | M | knob tiles | 9,26..49,38 | 10,27..49,37 | 0 | 9 | 190 | no |  |
| `AUTO` | M | knob tiles | 10,48..49,60 | 10,49..48,59 | 0 | 10 | 190 | no |  |
| `GEN C2` | S | knob tiles -> status line | 9,68..50,77 | 9,68..49,76 | 0 | 9 | 189 | no | faint blend row outside the band |
| `GATE` | M | knob tiles | 72,26..107,38 | 72,27..107,37 | 4 | 12 | 132 | no |  |
| `AUTO` | M | knob tiles | 70,48..109,60 | 70,49..108,59 | 4 | 10 | 130 | no |  |
| `GEN 40` | S | knob tiles -> status line | 70,68..109,77 | 70,68..109,76 | 4 | 10 | 130 | no | faint blend row outside the band |
| `VELO` | M | knob tiles | 131,26..166,38 | 131,27..166,37 | 8 | 11 | 73 | no |  |
| `AUTO` | M | knob tiles | 130,48..169,60 | 130,49..168,59 | 8 | 10 | 70 | no |  |
| `GEN 43` | S | knob tiles -> status line | 130,68..169,77 | 130,68..169,76 | 8 | 10 | 70 | no | faint blend row outside the band |
| `RATCH` | M | knob tiles | 186,26..232,38 | 187,27..231,37 | 12 | 6 | 7 | no |  |
| `AUTO` | M | knob tiles | 190,48..229,60 | 190,49..228,59 | 12 | 10 | 10 | no |  |
| `GEN 1X` | S | knob tiles -> status line | 190,68..230,77 | 190,68..229,76 | 12 | 10 | 9 | no | faint blend row outside the band |
| `STEP` | S | status line | 3,82..30,91 | 4,82..29,90 | 0 | 3 | 209 | no |  |
| `9` | M | status line | 34,79..42,91 | 35,80..41,90 | 2 | 4 | 197 | no |  |
| `= SET BY YOU` | S | status line | 163,82..236,91 | 163,82..236,90 | 10 | 13 | 3 | no |  |
| `S` | S | keybed | 116,192..122,201 | 117,192..122,200 | 7 | 11 | 117 | no |  |
| `F` | S | keybed | 131,192..136,200 | 132,192..136,200 | 8 | 11 | 103 | no |  |
| `R` | S | keybed | 146,192..152,200 | 147,192..152,200 | 9 | 11 | 87 | no |  |
| `KNOBS EDIT` | M | footer | 2,224..84,236 | 3,225..84,235 | 0 | 2 | 155 | no |  |
| `HOME = DONE` | M | footer | 139,224..236,236 | 140,225..236,235 | 9 | 4 | 3 | no |  |

text ink-box overlaps (bitmap boxes): none
neighbouring text gaps < 6 px on shared rows: `2` -> `3` gap 5 px; `3` -> `4` gap 5 px; `ACID` -> `MIDI` gap 5 px; `MIDI` -> `1` gap 3 px; `STEP` -> `9` gap 3 px
rect/frame ops crossing y=120 (drawn in both strips): R 1,117 13x7 CH1, R 16,103 13x21 CH1, R 31,105 13x19 CH1, R 46,113 13x11 CH1, R 61,113 13x11 CH1, R 76,103 13x21 CH1, R 91,117 13x7 CH1, R 106,99 13x25 CH1, R 121,118 13x6 CH1, R 151,106 13x18 CH1, R 166,99 13x25 CH1, R 181,118 13x6 CH1, R 226,118 13x6 CH1, F 119,93 17x49 WHITE, F 120,94 15x47 WHITE
text ops crossing y=120 by bitmap box: none
text ink boxes with x1 > 239 or x0 < 0: none

#### note_set

texts: band, lit pixel extents (x0,y0..x1,y1 inclusive), column index of the ink left edge (15 px columns), offset into that column, distance from the right edge
| string | font | band(by lit y0..y1) | lit box (any blend) | strong box (alpha>=0.5) | col (x0//15) | x0 mod 15 | right margin to x=239 | crosses y=120? | outside its band? |
|---|---|---|---|---|---|---|---|---|---|
| `120` | M | header | 17,4..42,16 | 18,5..41,15 | 1 | 2 | 197 | no |  |
| `1` | M | header | 49,5..53,15 | 50,5..53,15 | 3 | 4 | 186 | no |  |
| `2` | M | header | 62,4..70,15 | 63,4..70,15 | 4 | 2 | 169 | no |  |
| `3` | M | header | 76,4..84,16 | 76,4..84,16 | 5 | 1 | 155 | no |  |
| `4` | M | header | 90,5..98,15 | 90,5..98,15 | 6 | 0 | 141 | no |  |
| `ACID` | M | header | 105,4..137,16 | 105,5..137,15 | 7 | 0 | 102 | no |  |
| `MIDI` | S | header | 143,7..165,15 | 144,7..165,15 | 9 | 8 | 74 | no |  |
| `1` | M | header | 169,5..173,15 | 170,5..173,15 | 11 | 4 | 66 | no |  |
| `PITCH` | M | knob tiles | 9,26..49,38 | 10,27..49,37 | 0 | 9 | 190 | no |  |
| `AUTO` | M | knob tiles | 10,48..49,60 | 10,49..48,59 | 0 | 10 | 190 | no |  |
| `GEN C2` | S | knob tiles -> status line | 9,68..50,77 | 9,68..49,76 | 0 | 9 | 189 | no | faint blend row outside the band |
| `GATE` | M | knob tiles | 72,26..107,38 | 72,27..107,37 | 4 | 12 | 132 | no |  |
| `AUTO` | M | knob tiles | 70,48..109,60 | 70,49..108,59 | 4 | 10 | 130 | no |  |
| `GEN 40` | S | knob tiles -> status line | 70,68..109,77 | 70,68..109,76 | 4 | 10 | 130 | no | faint blend row outside the band |
| `VELO` | M | knob tiles | 131,26..166,38 | 131,27..166,37 | 8 | 11 | 73 | no |  |
| `AUTO` | M | knob tiles | 130,48..169,60 | 130,49..168,59 | 8 | 10 | 70 | no |  |
| `GEN 43` | S | knob tiles -> status line | 130,68..169,77 | 130,68..169,76 | 8 | 10 | 70 | no | faint blend row outside the band |
| `RATCH` | M | knob tiles | 186,26..232,38 | 187,27..231,37 | 12 | 6 | 7 | no |  |
| `AUTO` | M | knob tiles | 190,48..229,60 | 190,49..228,59 | 12 | 10 | 10 | no |  |
| `GEN 1X` | S | knob tiles | 190,68..230,75 | 190,68..229,75 | 12 | 10 | 9 | no |  |
| `STEP` | S | status line | 3,82..30,91 | 4,82..29,90 | 0 | 3 | 209 | no |  |
| `9` | M | status line | 34,79..42,91 | 35,80..41,90 | 2 | 4 | 197 | no |  |
| `SLIDE` | S | status line | 50,81..79,90 | 51,81..79,89 | 3 | 5 | 160 | no |  |
| `PROB 50` | S | status line | 87,81..133,90 | 88,81..133,89 | 5 | 12 | 106 | no |  |
| `OCT-2` | S | status line | 140,81..174,90 | 140,81..173,89 | 9 | 5 | 65 | no |  |
| `S` | S | keybed | 116,192..122,201 | 117,192..122,200 | 7 | 11 | 117 | no |  |
| `F` | S | keybed | 131,192..136,200 | 132,192..136,200 | 8 | 11 | 103 | no |  |
| `R` | S | keybed | 146,192..152,200 | 147,192..152,200 | 9 | 11 | 87 | no |  |
| `KNOBS EDIT` | M | footer | 2,224..84,236 | 3,225..84,235 | 0 | 2 | 155 | no |  |
| `HOME = DONE` | M | footer | 139,224..236,236 | 140,225..236,235 | 9 | 4 | 3 | no |  |

text ink-box overlaps (bitmap boxes): none
neighbouring text gaps < 6 px on shared rows: `2` -> `3` gap 5 px; `3` -> `4` gap 5 px; `ACID` -> `MIDI` gap 5 px; `MIDI` -> `1` gap 3 px; `STEP` -> `9` gap 3 px
rect/frame ops crossing y=120 (drawn in both strips): R 1,117 13x7 CH1, R 16,103 13x21 CH1, R 31,105 13x19 CH1, R 46,113 13x11 CH1, R 61,113 13x11 CH1, R 76,103 13x21 CH1, R 91,117 13x7 CH1, R 106,99 13x25 CH1, R 151,106 13x18 CH1, R 166,99 13x25 CH1, R 181,118 13x6 CH1, R 226,118 13x6 CH1, F 119,93 17x49 WHITE, F 120,94 15x47 WHITE
text ops crossing y=120 by bitmap box: none
text ink boxes with x1 > 239 or x0 < 0: none

#### note_dice

texts: band, lit pixel extents (x0,y0..x1,y1 inclusive), column index of the ink left edge (15 px columns), offset into that column, distance from the right edge
| string | font | band(by lit y0..y1) | lit box (any blend) | strong box (alpha>=0.5) | col (x0//15) | x0 mod 15 | right margin to x=239 | crosses y=120? | outside its band? |
|---|---|---|---|---|---|---|---|---|---|
| `120` | M | header | 17,4..42,16 | 18,5..41,15 | 1 | 2 | 197 | no |  |
| `1` | M | header | 49,5..53,15 | 50,5..53,15 | 3 | 4 | 186 | no |  |
| `2` | M | header | 62,4..70,15 | 63,4..70,15 | 4 | 2 | 169 | no |  |
| `3` | M | header | 76,4..84,16 | 76,4..84,16 | 5 | 1 | 155 | no |  |
| `4` | M | header | 90,5..98,15 | 90,5..98,15 | 6 | 0 | 141 | no |  |
| `PITCH` | M | knob tiles | 9,26..49,38 | 10,27..49,37 | 0 | 9 | 190 | no |  |
| `F2` | L | knob tiles | 14,43..44,63 | 14,43..43,63 | 0 | 14 | 195 | no |  |
| `GEN C2` | S | knob tiles -> status line | 9,68..50,77 | 9,68..49,76 | 0 | 9 | 189 | no | faint blend row outside the band |
| `GATE` | M | knob tiles | 72,26..107,38 | 72,27..107,37 | 4 | 12 | 132 | no |  |
| `100` | L | knob tiles | 66,43..115,64 | 67,43..115,63 | 4 | 6 | 124 | no |  |
| `GEN 40` | S | knob tiles -> status line | 70,68..109,77 | 70,68..109,76 | 4 | 10 | 130 | no | faint blend row outside the band |
| `VELO` | M | knob tiles | 131,26..166,38 | 131,27..166,37 | 8 | 11 | 73 | no |  |
| `80` | L | knob tiles | 133,43..166,64 | 133,43..166,63 | 8 | 13 | 73 | no |  |
| `GEN 43` | S | knob tiles -> status line | 130,68..169,77 | 130,68..168,76 | 8 | 10 | 70 | no | faint blend row outside the band |
| `RATCH` | M | knob tiles | 186,26..232,38 | 187,27..231,37 | 12 | 6 | 7 | no |  |
| `AUTO` | M | knob tiles | 190,48..229,60 | 190,49..228,59 | 12 | 10 | 10 | no |  |
| `GEN 1X` | S | knob tiles | 190,68..230,75 | 190,68..229,75 | 12 | 10 | 9 | no |  |
| `STEP` | S | status line | 3,82..30,91 | 4,82..29,90 | 0 | 3 | 209 | no |  |
| `9` | M | status line | 34,79..42,91 | 35,80..41,90 | 2 | 4 | 197 | no |  |
| `NO SLIDE` | S | status line | 50,81..99,90 | 51,81..99,89 | 3 | 5 | 140 | no |  |
| `PROB 50` | S | status line | 106,81..152,90 | 107,81..152,89 | 7 | 1 | 87 | no |  |
| `OCT-2` | S | status line | 159,81..193,90 | 159,81..192,89 | 10 | 9 | 46 | no |  |
| `S` | S | keybed | 116,192..122,201 | 117,192..122,200 | 7 | 11 | 117 | no |  |
| `F` | S | keybed | 131,192..136,200 | 132,192..136,200 | 8 | 11 | 103 | no |  |
| `R` | S | keybed | 146,192..152,200 | 147,192..152,200 | 9 | 11 | 87 | no |  |
| `KNOBS EDIT` | M | footer | 2,224..84,236 | 3,225..84,235 | 0 | 2 | 155 | no |  |
| `HOME = DONE` | M | footer | 139,224..236,236 | 140,225..236,235 | 9 | 4 | 3 | no |  |
| `1` | M | header | 110,6..114,16 | 111,6..114,16 | 7 | 5 | 125 | no |  |
| `ALL` | M | header | 124,5..148,15 | 124,5..148,15 | 8 | 4 | 91 | no |  |
| `EDIT=UNDO` | S | header | 172,7..236,16 | 173,7..236,15 | 11 | 7 | 3 | no |  |

text ink-box overlaps (bitmap boxes): none
neighbouring text gaps < 6 px on shared rows: `2` -> `3` gap 5 px; `3` -> `4` gap 5 px; `STEP` -> `9` gap 3 px
rect/frame ops crossing y=120 (drawn in both strips): R 1,117 13x7 CH1, R 16,103 13x21 CH1, R 31,105 13x19 CH1, R 46,113 13x11 CH1, R 61,113 13x11 CH1, R 76,103 13x21 CH1, R 91,117 13x7 CH1, R 106,99 13x25 CH1, R 151,106 13x18 CH1, R 166,99 13x25 CH1, R 181,118 13x6 CH1, R 226,118 13x6 CH1, F 119,93 17x49 WHITE, F 120,94 15x47 WHITE
text ops crossing y=120 by bitmap box: none
text ink boxes with x1 > 239 or x0 < 0: none

#### note_final

texts: band, lit pixel extents (x0,y0..x1,y1 inclusive), column index of the ink left edge (15 px columns), offset into that column, distance from the right edge
| string | font | band(by lit y0..y1) | lit box (any blend) | strong box (alpha>=0.5) | col (x0//15) | x0 mod 15 | right margin to x=239 | crosses y=120? | outside its band? |
|---|---|---|---|---|---|---|---|---|---|
| `120` | M | header | 17,4..42,16 | 18,5..41,15 | 1 | 2 | 197 | no |  |
| `1` | M | header | 49,5..53,15 | 50,5..53,15 | 3 | 4 | 186 | no |  |
| `2` | M | header | 62,4..70,15 | 63,4..70,15 | 4 | 2 | 169 | no |  |
| `3` | M | header | 76,4..84,16 | 76,4..84,16 | 5 | 1 | 155 | no |  |
| `4` | M | header | 90,5..98,15 | 90,5..98,15 | 6 | 0 | 141 | no |  |
| `ACID` | M | header | 105,4..137,16 | 105,5..137,15 | 7 | 0 | 102 | no |  |
| `MIDI` | S | header | 143,7..165,15 | 144,7..165,15 | 9 | 8 | 74 | no |  |
| `1` | M | header | 169,5..173,15 | 170,5..173,15 | 11 | 4 | 66 | no |  |
| `PITCH` | M | knob tiles | 9,26..49,38 | 10,27..49,37 | 0 | 9 | 190 | no |  |
| `AUTO` | M | knob tiles | 10,48..49,60 | 10,49..48,59 | 0 | 10 | 190 | no |  |
| `GEN C2` | S | knob tiles -> status line | 9,68..50,77 | 9,68..49,76 | 0 | 9 | 189 | no | faint blend row outside the band |
| `GATE` | M | knob tiles | 72,26..107,38 | 72,27..107,37 | 4 | 12 | 132 | no |  |
| `AUTO` | M | knob tiles | 70,48..109,60 | 70,49..108,59 | 4 | 10 | 130 | no |  |
| `GEN 40` | S | knob tiles -> status line | 70,68..109,77 | 70,68..109,76 | 4 | 10 | 130 | no | faint blend row outside the band |
| `VELO` | M | knob tiles | 131,26..166,38 | 131,27..166,37 | 8 | 11 | 73 | no |  |
| `83` | L | knob tiles | 133,43..166,64 | 133,43..165,63 | 8 | 13 | 73 | no |  |
| `GEN 43` | S | knob tiles | 130,68..169,75 | 130,68..168,75 | 8 | 10 | 70 | no |  |
| `RATCH` | M | knob tiles | 186,26..232,38 | 187,27..231,37 | 12 | 6 | 7 | no |  |
| `AUTO` | M | knob tiles | 190,48..229,60 | 190,49..228,59 | 12 | 10 | 10 | no |  |
| `GEN 1X` | S | knob tiles -> status line | 190,68..230,77 | 190,68..229,76 | 12 | 10 | 9 | no | faint blend row outside the band |
| `STEP` | S | status line | 3,82..30,91 | 4,82..29,90 | 0 | 3 | 209 | no |  |
| `9` | M | status line | 34,79..42,91 | 35,80..41,90 | 2 | 4 | 197 | no |  |
| `= SET BY YOU` | S | status line | 163,82..236,91 | 163,82..236,90 | 10 | 13 | 3 | no |  |
| `S` | S | keybed | 116,192..122,201 | 117,192..122,200 | 7 | 11 | 117 | no |  |
| `F` | S | keybed | 131,192..136,200 | 132,192..136,200 | 8 | 11 | 103 | no |  |
| `R` | S | keybed | 146,192..152,200 | 147,192..152,200 | 9 | 11 | 87 | no |  |
| `KNOBS EDIT` | M | footer | 2,224..84,236 | 3,225..84,235 | 0 | 2 | 155 | no |  |
| `HOME = DONE` | M | footer | 139,224..236,236 | 140,225..236,235 | 9 | 4 | 3 | no |  |

text ink-box overlaps (bitmap boxes): none
neighbouring text gaps < 6 px on shared rows: `2` -> `3` gap 5 px; `3` -> `4` gap 5 px; `ACID` -> `MIDI` gap 5 px; `MIDI` -> `1` gap 3 px; `STEP` -> `9` gap 3 px
rect/frame ops crossing y=120 (drawn in both strips): R 1,117 13x7 CH1, R 31,105 13x19 CH1, R 46,113 13x11 CH1, R 61,113 13x11 CH1, R 76,103 13x21 CH1, R 91,117 13x7 CH1, R 106,99 13x25 CH1, R 121,118 13x6 CH1, R 151,106 13x18 CH1, R 166,99 13x25 CH1, R 181,118 13x6 CH1, R 226,118 13x6 CH1, F 119,93 17x49 WHITE, F 120,94 15x47 WHITE
text ops crossing y=120 by bitmap box: none
text ink boxes with x1 > 239 or x0 < 0: none

#### worst_w1

texts: band, lit pixel extents (x0,y0..x1,y1 inclusive), column index of the ink left edge (15 px columns), offset into that column, distance from the right edge
| string | font | band(by lit y0..y1) | lit box (any blend) | strong box (alpha>=0.5) | col (x0//15) | x0 mod 15 | right margin to x=239 | crosses y=120? | outside its band? |
|---|---|---|---|---|---|---|---|---|---|
| `120` | M | header | 17,4..42,16 | 18,5..41,15 | 1 | 2 | 197 | no |  |
| `1` | M | header | 49,5..53,15 | 50,5..53,15 | 3 | 4 | 186 | no |  |
| `2` | M | header | 62,4..70,15 | 63,4..70,15 | 4 | 2 | 169 | no |  |
| `3` | M | header | 76,4..84,16 | 76,4..84,16 | 5 | 1 | 155 | no |  |
| `4` | M | header | 90,5..98,15 | 90,5..98,15 | 6 | 0 | 141 | no |  |
| `ACID` | M | header | 105,4..137,16 | 105,5..137,15 | 7 | 0 | 102 | no |  |
| `MIDI` | S | header | 143,7..165,15 | 144,7..165,15 | 9 | 8 | 74 | no |  |
| `1` | M | header | 169,5..173,15 | 170,5..173,15 | 11 | 4 | 66 | no |  |
| `PITCH` | M | knob tiles | 9,26..49,38 | 10,27..49,37 | 0 | 9 | 190 | no |  |
| `C#-1` | M | knob tiles | 12,48..43,60 | 13,49..43,59 | 0 | 12 | 196 | no |  |
| `GEN G#-1` | S | knob tiles -> status line | 3,68..54,77 | 3,68..53,76 | 0 | 3 | 185 | no | faint blend row outside the band |
| `GATE` | M | knob tiles | 72,26..107,38 | 72,27..107,37 | 4 | 12 | 132 | no |  |
| `100` | L | knob tiles | 66,43..115,64 | 67,43..115,63 | 4 | 6 | 124 | no |  |
| `GEN 100` | S | knob tiles -> status line | 66,68..112,77 | 66,68..112,76 | 4 | 6 | 127 | no | faint blend row outside the band |
| `VELO` | M | knob tiles | 131,26..166,38 | 131,27..166,37 | 8 | 11 | 73 | no |  |
| `ACC` | M | knob tiles | 135,48..164,60 | 135,49..163,59 | 9 | 0 | 75 | no |  |
| `GEN 99` | S | knob tiles | 130,68..169,75 | 130,68..168,75 | 8 | 10 | 70 | no |  |
| `RATCH` | M | knob tiles | 186,26..232,38 | 187,27..231,37 | 12 | 6 | 7 | no |  |
| `4X` | L | knob tiles | 193,43..227,63 | 193,44..227,63 | 12 | 13 | 12 | no |  |
| `GEN 4X` | S | knob tiles -> status line | 190,68..230,77 | 190,68..229,76 | 12 | 10 | 9 | no | faint blend row outside the band |
| `STEP` | S | status line | 3,82..30,91 | 4,82..29,90 | 0 | 3 | 209 | no |  |
| `64` | M | status line | 34,79..51,91 | 35,80..51,90 | 2 | 4 | 188 | no |  |
| `NO SLIDE` | S | status line | 59,81..108,90 | 60,81..108,89 | 3 | 14 | 131 | no |  |
| `PROB 50` | S | status line | 115,81..161,90 | 116,81..161,89 | 7 | 10 | 78 | no |  |
| `OCT+2` | S | status line | 168,81..204,90 | 168,81..203,89 | 11 | 3 | 35 | no |  |
| `OFF` | S | status line | 211,81..231,90 | 211,81..231,89 | 14 | 1 | 8 | no |  |
| `S` | S | keybed | 116,192..122,201 | 117,192..122,200 | 7 | 11 | 117 | no |  |
| `F` | S | keybed | 131,192..136,200 | 132,192..136,200 | 8 | 11 | 103 | no |  |
| `R` | S | keybed | 146,192..152,200 | 147,192..152,200 | 9 | 11 | 87 | no |  |
| `KNOBS EDIT` | M | footer | 2,224..84,236 | 3,225..84,235 | 0 | 2 | 155 | no |  |
| `HOME = DONE` | M | footer | 139,224..236,236 | 140,225..236,235 | 9 | 4 | 3 | no |  |

text ink-box overlaps (bitmap boxes): none
neighbouring text gaps < 6 px on shared rows: `2` -> `3` gap 5 px; `3` -> `4` gap 5 px; `ACID` -> `MIDI` gap 5 px; `MIDI` -> `1` gap 3 px; `STEP` -> `64` gap 3 px
rect/frame ops crossing y=120 (drawn in both strips): R 1,117 13x7 CH1, R 31,105 13x19 CH1, R 46,113 13x11 CH1, R 61,113 13x11 CH1, R 76,103 13x21 CH1, R 91,117 13x7 CH1, R 106,99 13x25 CH1, R 121,118 13x6 CH1, R 151,106 13x18 CH1, R 166,99 13x25 CH1, R 181,118 13x6 CH1, R 226,118 13x6 CH1, F 224,93 17x49 WHITE, F 225,94 15x47 WHITE
text ops crossing y=120 by bitmap box: none
text ink boxes with x1 > 239 or x0 < 0: none

#### worst_w2

texts: band, lit pixel extents (x0,y0..x1,y1 inclusive), column index of the ink left edge (15 px columns), offset into that column, distance from the right edge
| string | font | band(by lit y0..y1) | lit box (any blend) | strong box (alpha>=0.5) | col (x0//15) | x0 mod 15 | right margin to x=239 | crosses y=120? | outside its band? |
|---|---|---|---|---|---|---|---|---|---|
| `120` | M | header | 17,4..42,16 | 18,5..41,15 | 1 | 2 | 197 | no |  |
| `1` | M | header | 49,5..53,15 | 50,5..53,15 | 3 | 4 | 186 | no |  |
| `2` | M | header | 62,4..70,15 | 63,4..70,15 | 4 | 2 | 169 | no |  |
| `3` | M | header | 76,4..84,16 | 76,4..84,16 | 5 | 1 | 155 | no |  |
| `4` | M | header | 90,5..98,15 | 90,5..98,15 | 6 | 0 | 141 | no |  |
| `ACID` | M | header | 105,4..137,16 | 105,5..137,15 | 7 | 0 | 102 | no |  |
| `MIDI` | S | header | 143,7..165,15 | 144,7..165,15 | 9 | 8 | 74 | no |  |
| `1` | M | header | 169,5..173,15 | 170,5..173,15 | 11 | 4 | 66 | no |  |
| `PITCH` | M | knob tiles | 9,26..49,38 | 10,27..49,37 | 0 | 9 | 190 | no |  |
| `A#-1` | M | knob tiles | 13,49..43,59 | 13,49..43,59 | 0 | 13 | 196 | no |  |
| `GEN D#-1` | S | knob tiles -> status line | 3,68..53,77 | 3,68..52,76 | 0 | 3 | 186 | no | faint blend row outside the band |
| `GATE` | M | knob tiles | 72,26..107,38 | 72,27..107,37 | 4 | 12 | 132 | no |  |
| `TIE` | L | knob tiles | 70,43..108,63 | 71,44..108,63 | 4 | 10 | 131 | no |  |
| `GEN TIE` | S | knob tiles -> status line | 68,68..110,77 | 68,68..109,76 | 4 | 8 | 129 | no | faint blend row outside the band |
| `VELO` | M | knob tiles | 131,26..166,38 | 131,27..166,37 | 8 | 11 | 73 | no |  |
| `ACC` | M | knob tiles | 135,48..164,60 | 135,49..163,59 | 9 | 0 | 75 | no |  |
| `GEN ACC` | S | knob tiles | 125,68..174,75 | 125,68..173,75 | 8 | 5 | 65 | no |  |
| `RATCH` | M | knob tiles | 186,26..232,38 | 187,27..231,37 | 12 | 6 | 7 | no |  |
| `AUTO` | M | knob tiles | 190,48..229,60 | 190,49..228,59 | 12 | 10 | 10 | no |  |
| `GEN 3X` | S | knob tiles -> status line | 190,68..230,77 | 190,68..229,76 | 12 | 10 | 9 | no | faint blend row outside the band |
| `STEP` | S | status line | 3,82..30,91 | 4,82..29,90 | 0 | 3 | 209 | no |  |
| `1` | M | status line | 35,80..39,90 | 36,80..39,90 | 2 | 5 | 200 | no |  |
| `= SET BY YOU` | S | status line | 163,82..236,91 | 163,82..236,90 | 10 | 13 | 3 | no |  |
| `S` | S | keybed | 116,192..122,201 | 117,192..122,200 | 7 | 11 | 117 | no |  |
| `F` | S | keybed | 131,192..136,200 | 132,192..136,200 | 8 | 11 | 103 | no |  |
| `R` | S | keybed | 146,192..152,200 | 147,192..152,200 | 9 | 11 | 87 | no |  |
| `KNOBS EDIT` | M | footer | 2,224..84,236 | 3,225..84,235 | 0 | 2 | 155 | no |  |
| `HOME = DONE` | M | footer | 139,224..236,236 | 140,225..236,235 | 9 | 4 | 3 | no |  |

text ink-box overlaps (bitmap boxes): none
neighbouring text gaps < 6 px on shared rows: `2` -> `3` gap 5 px; `3` -> `4` gap 5 px; `ACID` -> `MIDI` gap 5 px; `MIDI` -> `1` gap 3 px; `STEP` -> `1` gap 4 px
rect/frame ops crossing y=120 (drawn in both strips): R 1,117 13x7 CH1, R 31,105 13x19 CH1, R 46,113 13x11 CH1, R 61,113 13x11 CH1, R 76,103 13x21 CH1, R 91,117 13x7 CH1, R 106,99 13x25 CH1, R 121,118 13x6 CH1, R 151,106 13x18 CH1, R 166,99 13x25 CH1, R 181,118 13x6 CH1, R 226,118 13x6 CH1, F -1,93 17x49 WHITE, F 0,94 15x47 WHITE
text ops crossing y=120 by bitmap box: none
text ink boxes with x1 > 239 or x0 < 0: none



### 3b. Geometry of the NOTE_EDIT-specific elements vs the grid (from the op log)

#### note_open
tile rects (label band 58x16 / value area 58x36): 1,24 58x16 SURF; 1,41 58x36 SURF; 61,24 58x16 SURF; 61,41 58x36 SURF; 121,24 58x16 SURF; 121,41 58x36 SURF; 181,24 58x16 SURF; 181,41 58x36 SURF
| tile | text | font | tile x | tile centre x | ink centre x | offset (ink - tile centre) | text rows lit y0..y1 (any blend) | strong rows (alpha>=0.5) | position in tile (value area rows 41..76, label band 24..39) |
|---|---|---|---|---|---|---|---|---|---|
| K1 | `PITCH` | M | 1 | 30.0 | 29.5 | -0.5 | 26..38 | 27..37 | label band 24..39 (bottom pad 2) |
| K1 | `AUTO` | M | 1 | 30.0 | 30.0 | +0.0 | 48..60 | 49..59 | inside; bottom pad 17 rows (strong), faint blend reaches row 60 |
| K1 | `GEN C2` | S | 1 | 30.0 | 30.0 | +0.0 | 68..77 | 68..76 | faint blend row 77 is past the tile's last row 76; strong rows end 76 (pad 0) |
| K2 | `GATE` | M | 61 | 90.0 | 90.0 | +0.0 | 26..38 | 27..37 | label band 24..39 (bottom pad 2) |
| K2 | `AUTO` | M | 61 | 90.0 | 90.0 | +0.0 | 48..60 | 49..59 | inside; bottom pad 17 rows (strong), faint blend reaches row 60 |
| K2 | `GEN 40` | S | 61 | 90.0 | 90.0 | +0.0 | 68..77 | 68..76 | faint blend row 77 is past the tile's last row 76; strong rows end 76 (pad 0) |
| K3 | `VELO` | M | 121 | 150.0 | 149.0 | -1.0 | 26..38 | 27..37 | label band 24..39 (bottom pad 2) |
| K3 | `AUTO` | M | 121 | 150.0 | 150.0 | +0.0 | 48..60 | 49..59 | inside; bottom pad 17 rows (strong), faint blend reaches row 60 |
| K3 | `GEN 43` | S | 121 | 150.0 | 150.0 | +0.0 | 68..77 | 68..76 | faint blend row 77 is past the tile's last row 76; strong rows end 76 (pad 0) |
| K4 | `RATCH` | M | 181 | 210.0 | 209.5 | -0.5 | 26..38 | 27..37 | label band 24..39 (bottom pad 2) |
| K4 | `AUTO` | M | 181 | 210.0 | 210.0 | +0.0 | 48..60 | 49..59 | inside; bottom pad 17 rows (strong), faint blend reaches row 60 |
| K4 | `GEN 1X` | S | 181 | 210.0 | 210.5 | +0.5 | 68..77 | 68..76 | faint blend row 77 is past the tile's last row 76; strong rows end 76 (pad 0) |
status line items: `STEP` x3..31 y82..92; `9` x34..43 y79..92; `= SET BY YOU` x163..237 y82..92
status chips (rect x,y,w,h): none; chip text insets (ink x0 - rect x, rect right - ink x1): 
legend square(s): R 154,82 CHF1, F 154,82 CH1
white frames: 119,93 17x49, 120,94 15x47, 121,186 14x33
music-row frame: outer x=119..135 (w17), inner x=120..134 (w15), y=93..141 outer; column 8 spans x=120..134, bar x=121..133 (13 px); frame outer overhangs the column by 1 px left and 1 px right; the next/previous bars end/start 0 px / 0 px from it
key frame: x=121..134 y=186..218 (white key body x=121..134, 14 px, pitch 15)
button tiles: 12, x = [1, 41, 81, 121, 161, 201], y = [146, 164] (spec 6 x 40 pitch, 2 rows pitch 18)
white keys: 16, x = [1, 16, 31, 46, 61, 76, 91, 106, 121, 136, 151, 166, 181, 196, 211, 226] (x - 15*i = [1]), y=[186] h=33
button icon insets in their 38x17 tile: DICE_ALL dx=12(l)/13(r) dy=2(t)/2(b); DICE_NOTES dx=12(l)/13(r) dy=2(t)/2(b); DICE_ACCNT dx=12(l)/13(r) dy=2(t)/2(b); DICE_SHAPE dx=12(l)/13(r) dy=2(t)/2(b); UNDO dx=12(l)/13(r) dy=3(t)/3(b); SETTINGS dx=12(l)/13(r) dy=3(t)/3(b); HOME dx=12(l)/13(r) dy=3(t)/3(b); SAVE dx=12(l)/13(r) dy=3(t)/3(b); DICE_ARTIC dx=12(l)/13(r) dy=2(t)/2(b); DICE_HITS dx=12(l)/13(r) dy=2(t)/2(b); PLAY dx=12(l)/13(r) dy=3(t)/3(b); REC_RING dx=12(l)/13(r) dy=3(t)/3(b)

#### note_set
tile rects (label band 58x16 / value area 58x36): 1,24 58x16 SURF; 1,41 58x36 SURF; 61,24 58x16 SURF; 61,41 58x36 SURF; 121,24 58x16 SURF; 121,41 58x36 SURF; 181,24 58x16 SURF; 181,41 58x36 SURF
| tile | text | font | tile x | tile centre x | ink centre x | offset (ink - tile centre) | text rows lit y0..y1 (any blend) | strong rows (alpha>=0.5) | position in tile (value area rows 41..76, label band 24..39) |
|---|---|---|---|---|---|---|---|---|---|
| K1 | `PITCH` | M | 1 | 30.0 | 29.5 | -0.5 | 26..38 | 27..37 | label band 24..39 (bottom pad 2) |
| K1 | `AUTO` | M | 1 | 30.0 | 30.0 | +0.0 | 48..60 | 49..59 | inside; bottom pad 17 rows (strong), faint blend reaches row 60 |
| K1 | `GEN C2` | S | 1 | 30.0 | 30.0 | +0.0 | 68..77 | 68..76 | faint blend row 77 is past the tile's last row 76; strong rows end 76 (pad 0) |
| K2 | `GATE` | M | 61 | 90.0 | 90.0 | +0.0 | 26..38 | 27..37 | label band 24..39 (bottom pad 2) |
| K2 | `AUTO` | M | 61 | 90.0 | 90.0 | +0.0 | 48..60 | 49..59 | inside; bottom pad 17 rows (strong), faint blend reaches row 60 |
| K2 | `GEN 40` | S | 61 | 90.0 | 90.0 | +0.0 | 68..77 | 68..76 | faint blend row 77 is past the tile's last row 76; strong rows end 76 (pad 0) |
| K3 | `VELO` | M | 121 | 150.0 | 149.0 | -1.0 | 26..38 | 27..37 | label band 24..39 (bottom pad 2) |
| K3 | `AUTO` | M | 121 | 150.0 | 150.0 | +0.0 | 48..60 | 49..59 | inside; bottom pad 17 rows (strong), faint blend reaches row 60 |
| K3 | `GEN 43` | S | 121 | 150.0 | 150.0 | +0.0 | 68..77 | 68..76 | faint blend row 77 is past the tile's last row 76; strong rows end 76 (pad 0) |
| K4 | `RATCH` | M | 181 | 210.0 | 209.5 | -0.5 | 26..38 | 27..37 | label band 24..39 (bottom pad 2) |
| K4 | `AUTO` | M | 181 | 210.0 | 210.0 | +0.0 | 48..60 | 49..59 | inside; bottom pad 17 rows (strong), faint blend reaches row 60 |
| K4 | `GEN 1X` | S | 181 | 210.0 | 210.5 | +0.5 | 68..75 | 68..75 | inside; bottom pad 1 rows (strong), faint blend reaches row 75 |
status line items: `STEP` x3..31 y82..92; `9` x34..43 y79..92; `SLIDE` x50..80 y81..91; `PROB 50` x87..134 y81..91; `OCT-2` x140..175 y81..91
status chips (rect x,y,w,h): 48,79,35,13 (x mod 15 = 3), 85,79,51,13 (x mod 15 = 10), 138,79,39,13 (x mod 15 = 3); chip text insets (ink x0 - rect x, rect right - ink x1): `SLIDE` 2,3, `PROB 50` 2,2, `OCT-2` 2,2
  chip gap 83..85 = 2 px
  chip gap 136..138 = 2 px
  last chip right edge x=177; legend present: False
legend square(s): none
white frames: 119,93 17x49, 120,94 15x47, 121,186 14x33
music-row frame: outer x=119..135 (w17), inner x=120..134 (w15), y=93..141 outer; column 8 spans x=120..134, bar x=121..133 (13 px); frame outer overhangs the column by 1 px left and 1 px right; the next/previous bars end/start 0 px / 0 px from it
key frame: x=121..134 y=186..218 (white key body x=121..134, 14 px, pitch 15)
button tiles: 12, x = [1, 41, 81, 121, 161, 201], y = [146, 164] (spec 6 x 40 pitch, 2 rows pitch 18)
white keys: 16, x = [1, 16, 31, 46, 61, 76, 91, 106, 121, 136, 151, 166, 181, 196, 211, 226] (x - 15*i = [1]), y=[186] h=33
button icon insets in their 38x17 tile: DICE_ALL dx=12(l)/13(r) dy=2(t)/2(b); DICE_NOTES dx=12(l)/13(r) dy=2(t)/2(b); DICE_ACCNT dx=12(l)/13(r) dy=2(t)/2(b); DICE_SHAPE dx=12(l)/13(r) dy=2(t)/2(b); UNDO dx=12(l)/13(r) dy=3(t)/3(b); SETTINGS dx=12(l)/13(r) dy=3(t)/3(b); HOME dx=12(l)/13(r) dy=3(t)/3(b); SAVE dx=12(l)/13(r) dy=3(t)/3(b); DICE_ARTIC dx=12(l)/13(r) dy=2(t)/2(b); DICE_HITS dx=12(l)/13(r) dy=2(t)/2(b); PLAY dx=12(l)/13(r) dy=3(t)/3(b); REC_RING dx=12(l)/13(r) dy=3(t)/3(b)

#### note_dice
tile rects (label band 58x16 / value area 58x36): 1,24 58x16 SURF; 1,41 58x36 CHF1; 61,24 58x16 SURF; 61,41 58x36 CHF1; 121,24 58x16 SURF; 121,41 58x36 CHF1; 181,24 58x16 SURF; 181,41 58x36 SURF
| tile | text | font | tile x | tile centre x | ink centre x | offset (ink - tile centre) | text rows lit y0..y1 (any blend) | strong rows (alpha>=0.5) | position in tile (value area rows 41..76, label band 24..39) |
|---|---|---|---|---|---|---|---|---|---|
| K1 | `PITCH` | M | 1 | 30.0 | 29.5 | -0.5 | 26..38 | 27..37 | label band 24..39 (bottom pad 2) |
| K1 | `F2` | L | 1 | 30.0 | 29.5 | -0.5 | 43..63 | 43..63 | inside; bottom pad 13 rows (strong), faint blend reaches row 63 |
| K1 | `GEN C2` | S | 1 | 30.0 | 30.0 | +0.0 | 68..77 | 68..76 | faint blend row 77 is past the tile's last row 76; strong rows end 76 (pad 0) |
| K2 | `GATE` | M | 61 | 90.0 | 90.0 | +0.0 | 26..38 | 27..37 | label band 24..39 (bottom pad 2) |
| K2 | `100` | L | 61 | 90.0 | 91.0 | +1.0 | 43..64 | 43..63 | inside; bottom pad 13 rows (strong), faint blend reaches row 64 |
| K2 | `GEN 40` | S | 61 | 90.0 | 90.0 | +0.0 | 68..77 | 68..76 | faint blend row 77 is past the tile's last row 76; strong rows end 76 (pad 0) |
| K3 | `VELO` | M | 121 | 150.0 | 149.0 | -1.0 | 26..38 | 27..37 | label band 24..39 (bottom pad 2) |
| K3 | `80` | L | 121 | 150.0 | 150.0 | +0.0 | 43..64 | 43..63 | inside; bottom pad 13 rows (strong), faint blend reaches row 64 |
| K3 | `GEN 43` | S | 121 | 150.0 | 150.0 | +0.0 | 68..77 | 68..76 | faint blend row 77 is past the tile's last row 76; strong rows end 76 (pad 0) |
| K4 | `RATCH` | M | 181 | 210.0 | 209.5 | -0.5 | 26..38 | 27..37 | label band 24..39 (bottom pad 2) |
| K4 | `AUTO` | M | 181 | 210.0 | 210.0 | +0.0 | 48..60 | 49..59 | inside; bottom pad 17 rows (strong), faint blend reaches row 60 |
| K4 | `GEN 1X` | S | 181 | 210.0 | 210.5 | +0.5 | 68..75 | 68..75 | inside; bottom pad 1 rows (strong), faint blend reaches row 75 |
status line items: `STEP` x3..31 y82..92; `9` x34..43 y79..92; `NO SLIDE` x50..100 y81..91; `PROB 50` x106..153 y81..91; `OCT-2` x159..194 y81..91
status chips (rect x,y,w,h): 48,79,54,13 (x mod 15 = 3), 104,79,51,13 (x mod 15 = 14), 157,79,39,13 (x mod 15 = 7); chip text insets (ink x0 - rect x, rect right - ink x1): `NO SLIDE` 2,2, `PROB 50` 2,2, `OCT-2` 2,2
  chip gap 102..104 = 2 px
  chip gap 155..157 = 2 px
  last chip right edge x=196; legend present: False
legend square(s): none
white frames: 119,93 17x49, 120,94 15x47, 121,186 14x33
music-row frame: outer x=119..135 (w17), inner x=120..134 (w15), y=93..141 outer; column 8 spans x=120..134, bar x=121..133 (13 px); frame outer overhangs the column by 1 px left and 1 px right; the next/previous bars end/start 0 px / 0 px from it
key frame: x=121..134 y=186..218 (white key body x=121..134, 14 px, pitch 15)
button tiles: 12, x = [1, 41, 81, 121, 161, 201], y = [146, 164] (spec 6 x 40 pitch, 2 rows pitch 18)
white keys: 16, x = [1, 16, 31, 46, 61, 76, 91, 106, 121, 136, 151, 166, 181, 196, 211, 226] (x - 15*i = [1]), y=[186] h=33
button icon insets in their 38x17 tile: DICE_ALL dx=12(l)/13(r) dy=2(t)/2(b); DICE_NOTES dx=12(l)/13(r) dy=2(t)/2(b); DICE_ACCNT dx=12(l)/13(r) dy=2(t)/2(b); DICE_SHAPE dx=12(l)/13(r) dy=2(t)/2(b); UNDO dx=12(l)/13(r) dy=3(t)/3(b); SETTINGS dx=12(l)/13(r) dy=3(t)/3(b); HOME dx=12(l)/13(r) dy=3(t)/3(b); SAVE dx=12(l)/13(r) dy=3(t)/3(b); DICE_ARTIC dx=12(l)/13(r) dy=2(t)/2(b); DICE_HITS dx=12(l)/13(r) dy=2(t)/2(b); PLAY dx=12(l)/13(r) dy=3(t)/3(b); REC_RING dx=12(l)/13(r) dy=3(t)/3(b)

#### note_final
tile rects (label band 58x16 / value area 58x36): 1,24 58x16 SURF; 1,41 58x36 SURF; 61,24 58x16 SURF; 61,41 58x36 SURF; 121,24 58x16 SURF; 121,41 58x36 CHF1; 181,24 58x16 SURF; 181,41 58x36 SURF
| tile | text | font | tile x | tile centre x | ink centre x | offset (ink - tile centre) | text rows lit y0..y1 (any blend) | strong rows (alpha>=0.5) | position in tile (value area rows 41..76, label band 24..39) |
|---|---|---|---|---|---|---|---|---|---|
| K1 | `PITCH` | M | 1 | 30.0 | 29.5 | -0.5 | 26..38 | 27..37 | label band 24..39 (bottom pad 2) |
| K1 | `AUTO` | M | 1 | 30.0 | 30.0 | +0.0 | 48..60 | 49..59 | inside; bottom pad 17 rows (strong), faint blend reaches row 60 |
| K1 | `GEN C2` | S | 1 | 30.0 | 30.0 | +0.0 | 68..77 | 68..76 | faint blend row 77 is past the tile's last row 76; strong rows end 76 (pad 0) |
| K2 | `GATE` | M | 61 | 90.0 | 90.0 | +0.0 | 26..38 | 27..37 | label band 24..39 (bottom pad 2) |
| K2 | `AUTO` | M | 61 | 90.0 | 90.0 | +0.0 | 48..60 | 49..59 | inside; bottom pad 17 rows (strong), faint blend reaches row 60 |
| K2 | `GEN 40` | S | 61 | 90.0 | 90.0 | +0.0 | 68..77 | 68..76 | faint blend row 77 is past the tile's last row 76; strong rows end 76 (pad 0) |
| K3 | `VELO` | M | 121 | 150.0 | 149.0 | -1.0 | 26..38 | 27..37 | label band 24..39 (bottom pad 2) |
| K3 | `83` | L | 121 | 150.0 | 150.0 | +0.0 | 43..64 | 43..63 | inside; bottom pad 13 rows (strong), faint blend reaches row 64 |
| K3 | `GEN 43` | S | 121 | 150.0 | 150.0 | +0.0 | 68..75 | 68..75 | inside; bottom pad 1 rows (strong), faint blend reaches row 75 |
| K4 | `RATCH` | M | 181 | 210.0 | 209.5 | -0.5 | 26..38 | 27..37 | label band 24..39 (bottom pad 2) |
| K4 | `AUTO` | M | 181 | 210.0 | 210.0 | +0.0 | 48..60 | 49..59 | inside; bottom pad 17 rows (strong), faint blend reaches row 60 |
| K4 | `GEN 1X` | S | 181 | 210.0 | 210.5 | +0.5 | 68..77 | 68..76 | faint blend row 77 is past the tile's last row 76; strong rows end 76 (pad 0) |
status line items: `STEP` x3..31 y82..92; `9` x34..43 y79..92; `= SET BY YOU` x163..237 y82..92
status chips (rect x,y,w,h): none; chip text insets (ink x0 - rect x, rect right - ink x1): 
legend square(s): R 154,82 CHF1, F 154,82 CH1
white frames: 119,93 17x49, 120,94 15x47, 121,186 14x33
music-row frame: outer x=119..135 (w17), inner x=120..134 (w15), y=93..141 outer; column 8 spans x=120..134, bar x=121..133 (13 px); frame outer overhangs the column by 1 px left and 1 px right; the next/previous bars end/start 0 px / 0 px from it
key frame: x=121..134 y=186..218 (white key body x=121..134, 14 px, pitch 15)
button tiles: 12, x = [1, 41, 81, 121, 161, 201], y = [146, 164] (spec 6 x 40 pitch, 2 rows pitch 18)
white keys: 16, x = [1, 16, 31, 46, 61, 76, 91, 106, 121, 136, 151, 166, 181, 196, 211, 226] (x - 15*i = [1]), y=[186] h=33
button icon insets in their 38x17 tile: DICE_ALL dx=12(l)/13(r) dy=2(t)/2(b); DICE_NOTES dx=12(l)/13(r) dy=2(t)/2(b); DICE_ACCNT dx=12(l)/13(r) dy=2(t)/2(b); DICE_SHAPE dx=12(l)/13(r) dy=2(t)/2(b); UNDO dx=12(l)/13(r) dy=3(t)/3(b); SETTINGS dx=12(l)/13(r) dy=3(t)/3(b); HOME dx=12(l)/13(r) dy=3(t)/3(b); SAVE dx=12(l)/13(r) dy=3(t)/3(b); DICE_ARTIC dx=12(l)/13(r) dy=2(t)/2(b); DICE_HITS dx=12(l)/13(r) dy=2(t)/2(b); PLAY dx=12(l)/13(r) dy=3(t)/3(b); REC_RING dx=12(l)/13(r) dy=3(t)/3(b)

#### worst_w1
tile rects (label band 58x16 / value area 58x36): 1,24 58x16 SURF; 1,41 58x36 CHF1; 61,24 58x16 SURF; 61,41 58x36 CHF1; 121,24 58x16 SURF; 121,41 58x36 CHF1; 181,24 58x16 SURF; 181,41 58x36 CHF1
| tile | text | font | tile x | tile centre x | ink centre x | offset (ink - tile centre) | text rows lit y0..y1 (any blend) | strong rows (alpha>=0.5) | position in tile (value area rows 41..76, label band 24..39) |
|---|---|---|---|---|---|---|---|---|---|
| K1 | `PITCH` | M | 1 | 30.0 | 29.5 | -0.5 | 26..38 | 27..37 | label band 24..39 (bottom pad 2) |
| K1 | `C#-1` | M | 1 | 30.0 | 28.0 | -2.0 | 48..60 | 49..59 | inside; bottom pad 17 rows (strong), faint blend reaches row 60 |
| K1 | `GEN G#-1` | S | 1 | 30.0 | 29.0 | -1.0 | 68..77 | 68..76 | faint blend row 77 is past the tile's last row 76; strong rows end 76 (pad 0) |
| K2 | `GATE` | M | 61 | 90.0 | 90.0 | +0.0 | 26..38 | 27..37 | label band 24..39 (bottom pad 2) |
| K2 | `100` | L | 61 | 90.0 | 91.0 | +1.0 | 43..64 | 43..63 | inside; bottom pad 13 rows (strong), faint blend reaches row 64 |
| K2 | `GEN 100` | S | 61 | 90.0 | 89.5 | -0.5 | 68..77 | 68..76 | faint blend row 77 is past the tile's last row 76; strong rows end 76 (pad 0) |
| K3 | `VELO` | M | 121 | 150.0 | 149.0 | -1.0 | 26..38 | 27..37 | label band 24..39 (bottom pad 2) |
| K3 | `ACC` | M | 121 | 150.0 | 150.0 | +0.0 | 48..60 | 49..59 | inside; bottom pad 17 rows (strong), faint blend reaches row 60 |
| K3 | `GEN 99` | S | 121 | 150.0 | 150.0 | +0.0 | 68..75 | 68..75 | inside; bottom pad 1 rows (strong), faint blend reaches row 75 |
| K4 | `RATCH` | M | 181 | 210.0 | 209.5 | -0.5 | 26..38 | 27..37 | label band 24..39 (bottom pad 2) |
| K4 | `4X` | L | 181 | 210.0 | 210.5 | +0.5 | 43..63 | 44..63 | inside; bottom pad 13 rows (strong), faint blend reaches row 63 |
| K4 | `GEN 4X` | S | 181 | 210.0 | 210.5 | +0.5 | 68..77 | 68..76 | faint blend row 77 is past the tile's last row 76; strong rows end 76 (pad 0) |
status line items: `STEP` x3..31 y82..92; `64` x34..52 y79..92; `NO SLIDE` x59..109 y81..91; `PROB 50` x115..162 y81..91; `OCT+2` x168..205 y81..91; `OFF` x211..232 y81..91
status chips (rect x,y,w,h): 57,79,54,13 (x mod 15 = 12), 113,79,51,13 (x mod 15 = 8), 166,79,41,13 (x mod 15 = 1), 209,79,26,13 (x mod 15 = 14); chip text insets (ink x0 - rect x, rect right - ink x1): `NO SLIDE` 2,2, `PROB 50` 2,2, `OCT+2` 2,2, `OFF` 2,3
  chip gap 111..113 = 2 px
  chip gap 164..166 = 2 px
  chip gap 207..209 = 2 px
  last chip right edge x=235; legend present: False
legend square(s): none
white frames: 224,93 17x49, 225,94 15x47, 226,186 14x33
music-row frame: outer x=224..240 (w17), inner x=225..239 (w15), y=93..141 outer; column 15 spans x=225..239, bar x=226..238 (13 px); frame outer overhangs the column by 1 px left and 1 px right; the next/previous bars end/start 0 px / 0 px from it
key frame: x=226..239 y=186..218 (white key body x=226..239, 14 px, pitch 15)
button tiles: 12, x = [1, 41, 81, 121, 161, 201], y = [146, 164] (spec 6 x 40 pitch, 2 rows pitch 18)
white keys: 16, x = [1, 16, 31, 46, 61, 76, 91, 106, 121, 136, 151, 166, 181, 196, 211, 226] (x - 15*i = [1]), y=[186] h=33
button icon insets in their 38x17 tile: DICE_ALL dx=12(l)/13(r) dy=2(t)/2(b); DICE_NOTES dx=12(l)/13(r) dy=2(t)/2(b); DICE_ACCNT dx=12(l)/13(r) dy=2(t)/2(b); DICE_SHAPE dx=12(l)/13(r) dy=2(t)/2(b); UNDO dx=12(l)/13(r) dy=3(t)/3(b); SETTINGS dx=12(l)/13(r) dy=3(t)/3(b); HOME dx=12(l)/13(r) dy=3(t)/3(b); SAVE dx=12(l)/13(r) dy=3(t)/3(b); DICE_ARTIC dx=12(l)/13(r) dy=2(t)/2(b); DICE_HITS dx=12(l)/13(r) dy=2(t)/2(b); PLAY dx=12(l)/13(r) dy=3(t)/3(b); REC_RING dx=12(l)/13(r) dy=3(t)/3(b)

#### worst_w2
tile rects (label band 58x16 / value area 58x36): 1,24 58x16 SURF; 1,41 58x36 CHF1; 61,24 58x16 SURF; 61,41 58x36 CHF1; 121,24 58x16 SURF; 121,41 58x36 CHF1; 181,24 58x16 SURF; 181,41 58x36 SURF
| tile | text | font | tile x | tile centre x | ink centre x | offset (ink - tile centre) | text rows lit y0..y1 (any blend) | strong rows (alpha>=0.5) | position in tile (value area rows 41..76, label band 24..39) |
|---|---|---|---|---|---|---|---|---|---|
| K1 | `PITCH` | M | 1 | 30.0 | 29.5 | -0.5 | 26..38 | 27..37 | label band 24..39 (bottom pad 2) |
| K1 | `A#-1` | M | 1 | 30.0 | 28.5 | -1.5 | 49..59 | 49..59 | inside; bottom pad 17 rows (strong), faint blend reaches row 59 |
| K1 | `GEN D#-1` | S | 1 | 30.0 | 28.5 | -1.5 | 68..77 | 68..76 | faint blend row 77 is past the tile's last row 76; strong rows end 76 (pad 0) |
| K2 | `GATE` | M | 61 | 90.0 | 90.0 | +0.0 | 26..38 | 27..37 | label band 24..39 (bottom pad 2) |
| K2 | `TIE` | L | 61 | 90.0 | 89.5 | -0.5 | 43..63 | 44..63 | inside; bottom pad 13 rows (strong), faint blend reaches row 63 |
| K2 | `GEN TIE` | S | 61 | 90.0 | 89.5 | -0.5 | 68..77 | 68..76 | faint blend row 77 is past the tile's last row 76; strong rows end 76 (pad 0) |
| K3 | `VELO` | M | 121 | 150.0 | 149.0 | -1.0 | 26..38 | 27..37 | label band 24..39 (bottom pad 2) |
| K3 | `ACC` | M | 121 | 150.0 | 150.0 | +0.0 | 48..60 | 49..59 | inside; bottom pad 17 rows (strong), faint blend reaches row 60 |
| K3 | `GEN ACC` | S | 121 | 150.0 | 150.0 | +0.0 | 68..75 | 68..75 | inside; bottom pad 1 rows (strong), faint blend reaches row 75 |
| K4 | `RATCH` | M | 181 | 210.0 | 209.5 | -0.5 | 26..38 | 27..37 | label band 24..39 (bottom pad 2) |
| K4 | `AUTO` | M | 181 | 210.0 | 210.0 | +0.0 | 48..60 | 49..59 | inside; bottom pad 17 rows (strong), faint blend reaches row 60 |
| K4 | `GEN 3X` | S | 181 | 210.0 | 210.5 | +0.5 | 68..77 | 68..76 | faint blend row 77 is past the tile's last row 76; strong rows end 76 (pad 0) |
status line items: `STEP` x3..31 y82..92; `1` x35..40 y80..91; `= SET BY YOU` x163..237 y82..92
status chips (rect x,y,w,h): none; chip text insets (ink x0 - rect x, rect right - ink x1): 
legend square(s): R 154,82 CHF1, F 154,82 CH1
white frames: -1,93 17x49, 0,94 15x47, 1,186 14x33
music-row frame: outer x=-1..15 (w17), inner x=0..14 (w15), y=93..141 outer; column 0 spans x=0..14, bar x=1..13 (13 px); frame outer overhangs the column by 1 px left and 1 px right; the next/previous bars end/start 0 px / 0 px from it
key frame: x=1..14 y=186..218 (white key body x=1..14, 14 px, pitch 15)
button tiles: 12, x = [1, 41, 81, 121, 161, 201], y = [146, 164] (spec 6 x 40 pitch, 2 rows pitch 18)
white keys: 16, x = [1, 16, 31, 46, 61, 76, 91, 106, 121, 136, 151, 166, 181, 196, 211, 226] (x - 15*i = [1]), y=[186] h=33
button icon insets in their 38x17 tile: DICE_ALL dx=12(l)/13(r) dy=2(t)/2(b); DICE_NOTES dx=12(l)/13(r) dy=2(t)/2(b); DICE_ACCNT dx=12(l)/13(r) dy=2(t)/2(b); DICE_SHAPE dx=12(l)/13(r) dy=2(t)/2(b); UNDO dx=12(l)/13(r) dy=3(t)/3(b); SETTINGS dx=12(l)/13(r) dy=3(t)/3(b); HOME dx=12(l)/13(r) dy=3(t)/3(b); SAVE dx=12(l)/13(r) dy=3(t)/3(b); DICE_ARTIC dx=12(l)/13(r) dy=2(t)/2(b); DICE_HITS dx=12(l)/13(r) dy=2(t)/2(b); PLAY dx=12(l)/13(r) dy=3(t)/3(b); REC_RING dx=12(l)/13(r) dy=3(t)/3(b)

#### Status line capacity arithmetic (firmware formula uisq_ne_status_layout; text_w from the mock = firmware advance)
STEP 9 chips ['SLIDE']: end x=85; legend needs xe+8+75 = 168 <= 237? yes (drawn)
STEP 9 chips ['NO SLIDE', 'PROB 50', 'OCT-2']: end x=198; legend needs xe+8+75 = 281 <= 237? NO (legend not drawn)
STEP 9 chips ['NO SLIDE', 'PROB 50', 'OCT+2', 'OFF']: end x=228; legend needs xe+8+75 = 311 <= 237? NO (legend not drawn)
STEP 9 chips ['ON', 'SLIDE', 'PROB 50', 'OCT+2']: end x=204; legend needs xe+8+75 = 287 <= 237? NO (legend not drawn)
STEP 9 chips ['OFF', 'NO SLIDE', 'PROB 50', 'OCT-2']: end x=226; legend needs xe+8+75 = 309 <= 237? NO (legend not drawn)
STEP 64 chips ['SLIDE']: end x=94; legend needs xe+8+75 = 177 <= 237? yes (drawn)
STEP 64 chips ['NO SLIDE', 'PROB 50', 'OCT-2']: end x=207; legend needs xe+8+75 = 290 <= 237? NO (legend not drawn)
STEP 64 chips ['NO SLIDE', 'PROB 50', 'OCT+2', 'OFF']: end x=237; legend needs xe+8+75 = 320 <= 237? NO (legend not drawn)
STEP 64 chips ['ON', 'SLIDE', 'PROB 50', 'OCT+2']: end x=213; legend needs xe+8+75 = 296 <= 237? NO (legend not drawn)
STEP 64 chips ['OFF', 'NO SLIDE', 'PROB 50', 'OCT-2']: end x=235; legend needs xe+8+75 = 318 <= 237? NO (legend not drawn)


## 4. Accuracy against source tables

Sources: `docs/design/button_modes.tsv`, `knob_map.tsv`, `key_map.tsv`, `note_edit_text.tsv`, `page_states.tsv`, scenario `docs/design/scenarios/S03-step-entry-note-edit-pages.md` steps 7-22, feature `docs/design/features/P4-014-note-edit-page.md` section 5 (proposed picture).

### buttons: panel order (ui_sq.c uisq_btn_ctl / uisq_btn_icon) -> icon drawn -> button_modes NOTE_EDIT tap row (label, action) and the PERF label of the same button
| # | control | icon drawn (tile) | tile fill / icon colour | button_modes NOTE_EDIT label | NOTE_EDIT tap action (first 150 chars) | PERF label | text on the tile? |
|---|---|---|---|---|---|---|---|
| 1 | B_FX | SQI_DICE_ALL 13x13 | DICE / WHITE | ALL | press: DICE THIS STEP: new pitch, accent, slide and gate for the edited step from fresh entropy, written as edits and auditioned; EDIT tap undoes | ALL | no |
| 2 | B_SCL | SQI_DICE_NOTES 13x13 | DICE / WHITE | NOTE | press: dice the edited step's pitch: a new scale note, written as its PITCH edit and auditioned; EDIT tap undoes | NOTES | no |
| 3 | B_ENV | SQI_DICE_ACCNT 13x13 | DICE / WHITE | ACCENT | press: toggle ACCENT on the edited step: velocity to ACC_VEL 120, or back to 80 / AUTO; EDIT tap undoes | ACCNT | no |
| 4 | B_LFO | SQI_DICE_SHAPE 13x13 | DICE / WHITE | PROB | press: toggle PROB on the edited step: 100 % / 50 % (PROB_LOW), drawn deterministically per pass; EDIT tap undoes | SHAPE | no |
| 5 | B_EDIT | SQI_UNDO 13x11 | SURF / TEXT | UNDO | UNDO: the edited step back to how it was when this NOTE EDIT visit began; again = redo | UNDO | no |
| 6 | B_GLO | SQI_SETTINGS 13x11 | SURF / TEXT | SET | open SET (latched): BPM SWING GATE DIV (closes NOTE EDIT) | SET | no |
| 7 | B_HOME | SQI_HOME 13x11 | SURF / TEXT | HOME | release under 0.7 s, nothing touched: back to PERF (leaves NOTE_EDIT) | HOME | no |
| 8 | B_SAVE | SQI_SAVE 13x11 | SURF / TEXT | SAVE | release under 0.7 s: save the whole device state to the slot in the header; while a slot is pending (header Pnn?), save to the PENDING slot instead an | SAVE | no |
| 9 | B_ARP | SQI_DICE_ARTIC 13x13 | DICE / WHITE | SLIDE | press: toggle SLIDE on the edited step: this note glides into the next step (step 1 after the last step; TB-303 rule; no glide if that step is silent) | ARTIC | no |
| 10 | B_SEQ | SQI_DICE_HITS 13x13 | DICE / WHITE | ON/OFF | press: toggle the edited step ON/OFF (same as tapping its key); EDIT tap undoes | HITS | no |
| 11 | B_PLAY | SQI_PLAY 13x11 | SURF / TEXT | PLAY | press: start / stop all channels (stop sends every note-off then CC123) | PLAY | no |
| 12 | B_REC | SQI_REC_RING 13x11 | SURF / TEXT | REC | press: open REC with the write cursor on the edited step | REC | no |
| - | B_OCTDN | (no tile on the screen) | | OCT- | release: the edited step one octave down (OCT -2..+2), auditioned | | no |
| - | B_OCTUP | (no tile on the screen) | | OCT+ | release: the edited step one octave up (OCT -2..+2), auditioned | | no |

### knobs: knob_map NOTE_EDIT rows vs the four tiles
- K1 `PITCH`: one scale degree per detent from the current note; every detent sounds the note (MIDI + preview); the first detent turns AUTO into a set pitch | range: AUTO, then 0..96 semitones above 36+key (stored unsnapped, played snapped)
- K2 `GATE`: note length of the edited step, x global GATE; top value TIE (holds into the next step) | range: AUTO, 10..100 % in 10 % steps, TIE
- K3 `VELO`: velocity of the edited step; value reads ACC at >= ACC_THR 100 (TB-3 accent) | range: AUTO, 1..127, step 1 fast 8
- K4 `RATCH`: repeats inside the step (SP5 spacing) | range: AUTO, 1x 2x 3x 4x
- SELECT `CH n`: turn: select channel CH1..CH4; closes NOTE EDIT (the edited step belongs to the old channel) | range: CH1..CH4
- ALGO `ALGO`: turn: algorithm of the selected channel ACID EUCLID SNAKE STAGE ARP PATTERNS; lands at the next step; morphs and seeds kept per algorithm (S7); edits kept | range: 6 algorithms
- PRESET `Pnn`: turn: choose a preset slot P01..P16; it becomes PENDING (header Pnn?, strip Pnn: SAVE = save here); stopped: it loads 1.5 s after the last detent (SQ_LOAD_SETTLE); playing: on the first 768-tick bar t | range: P01..P16

### keys: key_map NOTE_EDIT rows (white W01 and every black key) vs what the keybed draws
- KEY_W01_F3 tap: label `STEP` | release under 0.4 s: step 1 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: di
- KEY_W01_F3 hold: label `EDIT` | held 0.4 s: move NOTE EDIT to step 1 (no new page)
- KEY_B01_F#3 tap: label `M1` | tap: mute / unmute CH1 (latched; phase keeps running)
- KEY_B01_F#3 hold: label `PRV1` | held 0.4 s: hint PREVIEW CH1; released before 1.5 s = PREVIEW on/off of CH1 (on release; toast); with ALGO turned: its preview sound ACID LEAD BASS PAD STRINGS PIANO (tur
- KEY_B02_G#3 tap: label `M2` | tap: mute / unmute CH2
- KEY_B02_G#3 hold: label `PRV2` | held 0.4 s: hint PREVIEW CH2; released before 1.5 s = PREVIEW on/off of CH2 (on release; toast); with ALGO turned: its preview sound ACID LEAD BASS PAD STRINGS PIANO (tur
- KEY_B03_A#3 tap: label `M3` | tap: mute / unmute CH3
- KEY_B03_A#3 hold: label `PRV3` | held 0.4 s: hint PREVIEW CH3; released before 1.5 s = PREVIEW on/off of CH3 (on release; toast); with ALGO turned: its preview sound ACID LEAD BASS PAD STRINGS PIANO (tur
- KEY_B04_C#4 tap: label `M4` | tap: mute / unmute CH4
- KEY_B04_C#4 hold: label `PRV4` | held 0.4 s: hint PREVIEW CH4; released before 1.5 s = PREVIEW on/off of CH4 (on release; toast); with ALGO turned: its preview sound ACID LEAD BASS PAD STRINGS PIANO (tur
- KEY_B05_D#4 tap: label `CHN` | switch to the CHAN page (closes NOTE EDIT)
- KEY_B06_F#4 hold: label `SOL` | hold: only the selected channel sounds; release restores the mutes
- KEY_B07_G#4 hold: label `FIL` | hold: the selected channel's K1 density goes up while held (force: light = halfway, firm = maximum); release restores
- KEY_B08_A#4 hold: label `ROL` | hold: every step of the selected channel repeats while held (force: 2x light, 3x medium, 4x firm)
- KEY_B09_C#5 tap: label `PG-` | tap: view the previous page of the selected channel (no wrap; FOLLOW off)
- KEY_B10_D#5 tap: label `PG+` | tap: view the next page (FOLLOW off); on the last page: a prompt that names what PG+ again will do: ADD PAGE n = COPY OF n-1 (edits and source; LENGTH to 16 x pages, max 
- KEY_B11_F#5 tap: label `RST` | tap: all channels to step 1 at the next step

### note_edit_text.tsv: each id, the string, and whether it appears on any of the 4 renders + 2 worst cases (as a whole string or as a part of a built string)
- AUTO       `AUTO` -> note_dice, note_final, note_open, note_set, worst_w2
- ACC        `ACC` -> worst_w1, worst_w2
- TIE        `TIE` -> worst_w2
- X          `X` -> note_dice, note_final, note_open, note_set, worst_w1, worst_w2
- GEN        `GEN` -> note_dice, note_final, note_open, note_set, worst_w1, worst_w2
- STEP       `STEP` -> note_dice, note_final, note_open, note_set, worst_w1, worst_w2
- ON         `ON` -> NOT on any of the six frames
- OFF        `OFF` -> worst_w1
- SLIDE      `SLIDE` -> note_dice, note_set, worst_w1
- NO_SLIDE   `NO SLIDE` -> note_dice, worst_w1
- PROB       `PROB` -> note_dice, note_set, worst_w1
- OCT        `OCT` -> note_dice, note_set, worst_w1
- PLUS       `+` -> worst_w1
- MINUS      `-` -> note_dice, note_set, worst_w1, worst_w2
- SET_MARK   `= SET BY YOU` -> note_final, note_open, worst_w2
- KNOBS_EDIT `KNOBS EDIT` -> note_dice, note_final, note_open, note_set, worst_w1, worst_w2
- HOME_DONE  `HOME = DONE` -> note_dice, note_final, note_open, note_set, worst_w1, worst_w2
- NOTE_C     `C` -> note_dice, note_final, note_open, note_set
- NOTE_CS    `C#` -> worst_w1
- NOTE_D     `D` -> NOT on any of the six frames
- NOTE_DS    `D#` -> worst_w2
- NOTE_E     `E` -> NOT on any of the six frames
- NOTE_F     `F` -> note_dice
- NOTE_FS    `F#` -> NOT on any of the six frames
- NOTE_G     `G` -> NOT on any of the six frames
- NOTE_GS    `G#` -> worst_w1
- NOTE_A     `A` -> NOT on any of the six frames
- NOTE_AS    `A#` -> worst_w2
- NOTE_B     `B` -> NOT on any of the six frames

### strings on the NOTE_EDIT frames that are NOT in ui_ne_text, knob_map, or a persistent PERF element (built or literal in ui_sq.c)
- `PROB 50`: the suffix ` 50` is a literal in ui_sq.c (uisq_cat(..., " 50")); tsv PROB holds `PROB` only
- `GEN <v>`: the joining space is a literal in uisq_ne_gen_line
- `S` `F` `R`: black-key letters, literals in uisq_draw_keybed (the same on PERF)
- digits (STEP n, 10..100, 1..127, n X, OCT sign digit, note octave): built by uisq_catu; `120` tempo, `1`..`4` chips, `ACID` (algorithm short name), `MIDI` + channel: persistent header



### 4a. page_states.tsv, S03 and feature-file text vs the frames (strings quoted from the sources)

| source | says | frame shows |
|---|---|---|
| page_states NOTE_EDIT `screen` | header + lane (edited step outlined) + band: 4 knob cells with AUTO/set state + 6 button cells (ALL NOTE ON/OFF ACCENT SLIDE PROB) + OCT cells OCT-/OCT+ + legend + cue | tiles, outlined step, legend (when room) present; button cells are icons with no ALL/NOTE/ON/OFF/ACCENT/SLIDE/PROB words; no OCT- / OCT+ cells (OCT shows only as a chip when set); cue = footer |
| page_states NOTE_EDIT `cue` | `STEP n: KNOBS EDIT  HOME=done` | status line `STEP` + `9`; footer `KNOBS EDIT` (left, TEXT) and `HOME = DONE` (right, MID); case and spacing of the HOME words differ from the cue text |
| S03 step 7 | banner `CH1 NOTE EDIT STEP 9`; lane cell 9 boxed; `PITCH AUTO` `GATE AUTO` `VELO AUTO` `RATCH AUTO` (set values bright with `*`); tag `OCT-` `OCT+`; BTN labels `ALL NOTE ACCENT PROB UNDO SET / HOME SAVE SLIDE ON/OFF PLAY REC`; CUE `STEP 9: KNOBS EDIT  HOME=done` | no banner (the header still shows chip `1`, `ACID`, `MIDI 1`); step 9 boxed (white frame on the music row and on key 9); four AUTO tiles; set = filled tile and white value, no `*`; no OCT tag; no button words; footer as above |
| S03 step 8 | `PITCH` shows the scale note name and `*` | note name (`F2`) in AF_L (AF_M for 24 of 128 names), no `*` |
| S03 step 9 | cell `GATE TIE`; lane glyph `-` | tile value `TIE` (AF_L, 40 px) in W2 |
| S03 step 10 | `VELO` value 1..127 and `*`; at 100 or more `ACC` | `83` (note_final), `ACC` in AF_M (AF_L `ACC` is 58 px, over the 54 px limit) |
| S03 step 11 | `RATCH 2x*` | `nX` with a capital X (note_edit_text X = `RATCH suffix: 2X`; knob_map range text and S03 use `2x`) |
| S03 step 12 | button label reads `SLIDE`; `~` drawn dim toward cell 10 when it is silent | chip `SLIDE` / `NO SLIDE` in the status line; button tile unchanged (SQI_DICE_ARTIC icon, no label) |
| S03 step 13, 14 | `PROB 50`; `ACCENT` cell lit; lane `!` | chip `PROB 50`; the ENV tile is not lit; K3 reads `ACC` when >= 100 |
| S03 step 15 | `OCT+1` on the tag | chip `OCT+1` / `OCT-2` / `OCT+2` in the status line |
| S03 step 17 | PITCH, accent, slide and gate show new set values with `*` | note_dice: PITCH `F2`, GATE `100`, VELO `80` (not ACC), chips `NO SLIDE` ...; toast `ALL` / `EDIT=UNDO` |
| S03 step 18 | toast `UNDO: STEP 9` | not on these four frames; the test asserts the `ui_msgs` toast `UNDO  EDIT=REDO` (P4-014 section 2 row 7) |
| P4-014 proposed picture | status line `STEP n` (AF_S + AF_M); chips `ON/OFF`, `SLIDE` / `NO SLIDE`, `PROB 50`, `OCT+1`; legend when the chips leave room; footer `KNOBS EDIT` left, `HOME = DONE` right | as drawn; legend present on 2 of the 4 frames (the 2 where the chip row is short or empty) |
| P4-014 section 4 | layer buttons ease 9 < 11: "the icons are the PERF dice icons: the picture does not say 'this step'" | the 12 button tiles are pixel-identical to the same-state PERF frame (section 7) |
| knob_map SELECT / ALGO / PRESET rows (NOTE_EDIT) | `CH n`, `ALGO`, `Pnn` | selected chip `1` and `ACID` drawn (header); no preset number `Pnn` on this page (PERF shows `P01` at the status line's left); no `ROW` / `PAGE n/m` on this page |
| knob_map K1 range | `AUTO, then 0..96 semitones above 36+key` | note names (`C#-1`..`C#9` as MIDI note names), not semitone numbers |
| knob_map K2 range | `AUTO, 10..100 % in 10 % steps, TIE` | `10`..`100` without a `%` sign |
| key_map NOTE_EDIT W01..W16 | tap = step ON/OFF, hold = move NOTE EDIT; labels `STEP` / `EDIT` | no key labels; step on = key front in channel colour, edited key = white frame |
| key_map NOTE_EDIT B01..B11 | `M1 M2 M3 M4` (hold `PRV1..4`), `CHN`, `SOL`, `FIL`, `ROL`, `PG-`, `PG+`, `RST` | glyphs and letters identical to PERF (coloured square M1; muted `x` for M2-M4; MIDI glyph; `S F R`; arrows; RST) |

**Mismatch list (frame vs source), numbered**
1. Footer words `KNOBS EDIT` / `HOME = DONE` vs page_states / S03 cue `STEP n: KNOBS EDIT  HOME=done` (the step number moved to the status line; HOME words differ in case and spacing). `note_edit_text.tsv` matches the frame.
2. S03 banner `CH1 NOTE EDIT STEP 9` is not drawn on any frame.
3. S03 / page_states set marker `*` is not drawn (the set tile is a filled tile with a white value and two 7 x 2 amber edge ticks).
4. page_states / S03 button-cell words (ALL NOTE ACCENT PROB UNDO SET HOME SAVE SLIDE ON/OFF PLAY REC) are not drawn; the tiles carry the PERF icons. Icon vs NOTE_EDIT label: LFO tile = SHAPE die while the NOTE_EDIT action is PROB; ARP tile = ARTIC die while the action is SLIDE; SEQ tile = HITS die while the action is ON/OFF; EDIT = undo arrow (label UNDO); GLO = cog (label SET).
5. OCT- / OCT+ cells and the S03 OCT tag are not drawn; OCT appears only as a chip once set (AUTO = no chip).
6. `2X` (note_edit_text, frame) vs `2x` (knob_map range, S03 step 11).
7. K2 `%` and K1 semitone numbers: frame shows `10..100` and note names.
8. Preset slot `Pnn`, `ROW`, `PAGE n/m` are absent while NOTE_EDIT is open (the status line is replaced).
9. `ON` (chip) and `+` appear only in the hand-set frames' strings (`OFF`, `OCT+2` in W1); the four test frames never show an `ON` chip. Note-name ids NOTE_D/E/F#/G/G#/A/B and TIE/ACC appear only in the hand-set or width tables, not on the four frames.
10. The literal ` 50` of `PROB 50` and the space of `GEN <v>` are code literals (`ui_sq.c`), not table words.


## 5. Edge cases from `ui_ne_text` and the strings built from it

slots (from ui_sq.c uisq_draw_notetiles / uisq_ne_status_layout / uisq_draw_keybed): tile 58 px wide at x=1+60k; value font AF_L when text_w(AF_L,t) <= 54 else AF_M (no further fallback; no ellipsis call on the page);
label AF_M centred in 58 px; GEN line AF_S centred in 58 px; chips AF_S + 4 px pad + 2 px gap, row must end <= 237 else the legend is dropped (chips themselves are never dropped); footer AF_M left at x=2 and right-aligned to x=238.

### ui_ne_text, every string (generated table), advance px in each font
| id | string | AF_S | AF_M | AF_L | used as |
|---|---|---|---|---|---|
| 0 | `AUTO` | 31 | 40 | 76 | tile value AF_M (AUTO is never set: AF_M always) |
| 1 | `ACC` | 24 | 30 | 58 | VELO value (AF_L if fits else AF_M) and GEN line |
| 2 | `TIE` | 16 | 21 | 40 | GATE value and GEN line |
| 3 | `X` | 7 | 9 | 18 | RATCH suffix (2X) |
| 4 | `GEN` | 24 | 30 | 56 | GEN line prefix + space |
| 5 | `STEP` | 28 | 36 | 67 | status line AF_S |
| 6 | `ON` | 17 | 21 | 40 | chip |
| 7 | `OFF` | 22 | 27 | 51 | chip |
| 8 | `SLIDE` | 31 | 39 | 74 | chip |
| 9 | `NO SLIDE` | 50 | 64 | 120 | chip |
| 10 | `PROB` | 30 | 38 | 72 | chip prefix (+ ' 50') |
| 11 | `OCT` | 24 | 30 | 58 | chip prefix (+ sign + digit) |
| 12 | `+` | 7 | 9 | 18 | chip sign |
| 13 | `-` | 5 | 6 | 12 | chip / note-name sign |
| 14 | `= SET BY YOU` | 75 | 95 | 168 | legend (when room) |
| 15 | `KNOBS EDIT` | 66 | 83 | 158 | footer left AF_M |
| 16 | `HOME = DONE` | 78 | 99 | 174 | footer right AF_M |
| 17 | `C` | 8 | 10 | 20 | note-name part |
| 18 | `C#` | 15 | 19 | 37 | note-name part |
| 19 | `D` | 8 | 10 | 19 | note-name part |
| 20 | `D#` | 15 | 19 | 36 | note-name part |
| 21 | `E` | 7 | 8 | 16 | note-name part |
| 22 | `F` | 7 | 8 | 15 | note-name part |
| 23 | `F#` | 14 | 17 | 32 | note-name part |
| 24 | `G` | 8 | 11 | 20 | note-name part |
| 25 | `G#` | 15 | 20 | 37 | note-name part |
| 26 | `A` | 8 | 10 | 19 | note-name part |
| 27 | `A#` | 15 | 19 | 36 | note-name part |
| 28 | `B` | 7 | 9 | 17 | note-name part |

### knob labels (AF_M in a 58 px tile)
- K1 `PITCH` AF_M 42 px (slot 58, spare 16)
- K2 `GATE` AF_M 36 px (slot 58, spare 22)
- K3 `VELO` AF_M 37 px (slot 58, spare 21)
- K4 `RATCH` AF_M 47 px (slot 58, spare 11)

### PITCH, MIDI notes 0..127 (note name + octave n/12-1; the knob range is 36 + KEY + 12*OCTAVE up to +96 semitones; every one of 128 is printable) (128 values)
widest AF_M value: `C#-1` (35 px); widest AF_L value: `C#-1` (67 px); widest GEN line (AF_S): `GEN G#-1` (54 px, slot 58)
set value drawn in AF_L (wL <= 54): 104 values; in AF_M (wL > 54): 24 values: `A#-1`(L 66), `C#-1`(L 67), `C#0`(L 55), `C#1`(L 55), `C#2`(L 55), `C#3`(L 55), `C#4`(L 55), `C#5`(L 55), `C#6`(L 55), `C#7`(L 55), `C#8`(L 55), `C#9`(L 55), `D#-1`(L 66), `F#-1`(L 62), `G#-1`(L 67), `G#0`(L 55), `G#1`(L 55), `G#2`(L 55), `G#3`(L 55), `G#4`(L 55), `G#5`(L 55), `G#6`(L 55), `G#7`(L 55), `G#8`(L 55)
values over a slot of 58 px (AF_M value or GEN line): none
negative-octave names (notes 0..11): `C-1` M26 L50 genS46, `C#-1` M35 L67 genS53, `D-1` M26 L49 genS46, `D#-1` M34 L66 genS53, `E-1` M24 L46 genS45, `F-1` M24 L45 genS45, `F#-1` M32 L62 genS52, `G-1` M26 L50 genS47, `G#-1` M35 L67 genS54, `A-1` M25 L49 genS46, `A#-1` M34 L66 genS53, `B-1` M25 L47 genS46
note names by length: 2 chars: 68 names; 3 chars: 55 names; 4 chars: 5 names
font chosen across the 128: {'L': 104, 'M': 24}
AF_L widths: min 33 max 67; AF_M widths: min 17 max 35; GEN-line (AF_S) widths: min 40 max 54
  octave -1 (notes 0-11): AF_L fits 7 of 12; AF_M widths 24..35; GEN-line widths 45..54
  octave 0: AF_L fits 10 of 12; AF_M widths 17..29; GEN-line widths 40..49
  octaves 1-8: AF_L fits 80 of 96; AF_M widths 17..29; GEN-line widths 40..49
  octave 9: AF_L fits 7 of 8; AF_M widths 17..28; GEN-line widths 40..49

### GATE field 1..11 (10..100 %, 11 = TIE) (11 values)
widest AF_M value: `100` (27 px); widest AF_L value: `100` (54 px); widest GEN line (AF_S): `GEN 100` (47 px, slot 58)
set value drawn in AF_L (wL <= 54): 11 values; in AF_M (wL > 54): 0 values
values over a slot of 58 px (AF_M value or GEN line): none

### VELO 1..127 (>= 100 reads ACC) (127 values)
widest AF_M value: `ACC` (30 px); widest AF_L value: `ACC` (58 px); widest GEN line (AF_S): `GEN ACC` (50 px, slot 58)
set value drawn in AF_L (wL <= 54): 99 values; in AF_M (wL > 54): 28 values: `ACC`(L 58)
values over a slot of 58 px (AF_M value or GEN line): none

### RATCH 1..4 (4 values)
widest AF_M value: `1X` (18 px); widest AF_L value: `1X` (36 px); widest GEN line (AF_S): `GEN 1X` (40 px, slot 58)
set value drawn in AF_L (wL <= 54): 4 values; in AF_M (wL > 54): 0 values
values over a slot of 58 px (AF_M value or GEN line): none

GATE: `10` L36, `20` L36, `30` L36, `40` L36, `50` L36, `60` L36, `70` L36, `80` L36, `90` L36, `100` L54, `TIE` L40
VELO AF_L widths: 1-digit 18..18, 2-digit 36..36, ACC (L) 58

### status line: STEP n widths and chip row ends (the firmware's formula; the test asserts <= 237 for STEP 64 + four widest chips)
- `STEP` AF_S 28 + gap 3 + `1` AF_M 9 + 5 = chips start x=48
- `STEP` AF_S 28 + gap 3 + `9` AF_M 9 + 5 = chips start x=48
- `STEP` AF_S 28 + gap 3 + `10` AF_M 18 + 5 = chips start x=57
- `STEP` AF_S 28 + gap 3 + `64` AF_M 18 + 5 = chips start x=57
chip pitch (text + 4 pad + 2 gap): ON 23, OFF 28, SLIDE 37, NO SLIDE 56, PROB 50 53, OCT+1 43, OCT-2 41, OCT+2 43
legend `= SET BY YOU` AF_S advance 75 + 8 + square; drawn only when chips_end + 8 + 75 <= 237, i.e. chips_end <= 154
  STEP 9: chips start x=48; chip width (pitch) that can precede the legend = 106 px
  STEP 64: chips start x=57; chip width (pitch) that can precede the legend = 97 px

chip-field combinations (each field absent or one of its values) for STEP 9, count that keep the legend / that overflow 237:
  72 combinations: legend shown for 35, dropped for 37; widest chip row ends x=228; rows ending past 237: 0

### footer: `KNOBS EDIT` AF_M 83 at x=2 (ends 85), `HOME = DONE` AF_M 99 right-aligned to 238 (starts 139): gap between them 54 px

### truncation / ellipsis flags in the op logs (ink flag bit 0 = text_fit ellipsised; bit 1 = cut by the strip canvas, which is expected for a text lying wholly in the other strip)
- note_open: ellipsised texts 0 of 56 draws
- note_set: ellipsised texts 0 of 60 draws
- note_dice: ellipsised texts 0 of 66 draws
- note_final: ellipsised texts 0 of 56 draws
- worst_w1: ellipsised texts 0 of 62 draws
- worst_w2: ellipsised texts 0 of 56 draws


### 5a. The two hand-set frames
- W1 (step 64, all set, 4 chips): `STEP` `64`, chips `NO SLIDE` x=59, `PROB 50` x=115, `OCT+2` x=168, `OFF` x=211; last chip right edge x=235 (limit 237); no legend; edit frame outer ring x=224..240 (x=240 is outside the 240 px screen); PITCH `C#-1` drawn in AF_M (AF_L is 67 px); GEN line `GEN G#-1` 54 px
  (slot 58) at x=3..55 in tile 1..58; `GATE 100` AF_L 54 px (limit 54, fits exactly); `ACC` AF_M; `4X` AF_L. No clipping, no ellipsis, no overlap.
- W2 (step 1, set PITCH `A#-1`, GATE `TIE`, VELO `ACC`): PITCH value AF_M 34 px; GEN lines `GEN D#-1` (53), `GEN TIE`, `GEN ACC` (50); edit frame outer ring at x=-1 (outside the screen); legend present.


## 6. Counts per render (distinct text items and colours)

| render | text draws (visible) | distinct strings | distinct (string,font) | AF_L / AF_M / AF_S draws | distinct text fg colours | distinct text bg colours | icons | filled rects (R) | frames (F) | unique RGB values in the frame | of which exact palette tokens | AA blends (other) | colours drawn by rect/frame/icon/text ops (cv_pset hatches not logged) |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| note_open | 28 | 24 | 24 | 0 / 18 / 10 | 5 (BG, CH1, MID, TEXT, WHITE) | 4 (BG, CH1, RAISE, SURF) | 20 | 129 | 4 | 71 | 13 | 58 | BG, CH1, CHF1, DICE, DIM, MID, RAISE, SURF, TEXT, WHITE |
| note_set | 30 | 26 | 26 | 0 / 18 / 12 | 5 (BG, CH1, MID, TEXT, WHITE) | 4 (BG, CH1, RAISE, SURF) | 20 | 130 | 5 | 71 | 12 | 59 | BG, CH1, DICE, DIM, MID, RAISE, SURF, TEXT, WHITE |
| note_dice | 30 | 29 | 29 | 3 / 15 / 12 | 4 (BG, MID, TEXT, WHITE) | 6 (BG, CH1, CHF1, RAISE, SURF, WHITE) | 20 | 138 | 5 | 98 | 14 | 84 | BG, CH1, CHF1, DICE, DIM, MID, RAISE, SURF, TEXT, WHITE |
| note_final | 28 | 25 | 25 | 1 / 17 / 10 | 5 (BG, CH1, MID, TEXT, WHITE) | 5 (BG, CH1, CHF1, RAISE, SURF) | 20 | 130 | 6 | 93 | 13 | 80 | BG, CH1, CHF1, DICE, DIM, MID, RAISE, SURF, TEXT, WHITE |
| worst_w1 | 31 | 30 | 30 | 2 / 16 / 13 | 5 (BG, CH1, MID, TEXT, WHITE) | 5 (BG, CH1, CHF1, RAISE, SURF) | 20 | 139 | 5 | 94 | 13 | 81 | BG, CH1, CHF1, DICE, DIM, MID, RAISE, SURF, TEXT, WHITE |
| worst_w2 | 28 | 26 | 26 | 1 / 17 / 10 | 5 (BG, CH1, MID, TEXT, WHITE) | 5 (BG, CH1, CHF1, RAISE, SURF) | 20 | 134 | 6 | 90 | 13 | 77 | BG, CH1, CHF1, DICE, DIM, MID, RAISE, SURF, TEXT, WHITE |

### the same counts on the PERF page, same state (the locked main-screen code path), for comparison
| render | text draws (visible) | distinct strings | AF_L / AF_M / AF_S | distinct text fg | icons | unique RGB values |
|---|---|---|---|---|---|---|
| note_open_perf | 27 | 23 | 4 / 15 / 8 | 5 | 20 | 78 |
| note_set_perf | 27 | 23 | 4 / 15 / 8 | 5 | 20 | 78 |
| note_dice_perf | 27 | 23 | 4 / 15 / 8 | 4 | 20 | 75 |
| note_final_perf | 27 | 23 | 4 / 15 / 8 | 5 | 20 | 78 |

### page-specific (NOTE_EDIT) vs persistent text, per render: strings that exist only here (not on the same-state PERF render)
- note_open: NOTE_EDIT-only: `PITCH`@9,23, `AUTO`@10,45, `GEN C2`@9,65, `GATE`@72,23, `AUTO`@70,45, `GEN 40`@70,65, `VELO`@131,23, `AUTO`@130,45, `GEN 43`@130,65, `RATCH`@186,23, `AUTO`@190,45, `GEN 1X`@190,65, `STEP`@3,79, `9`@34,76, `= SET BY YOU`@162,79, `KNOBS EDIT`@2,221, `HOME = DONE`@139,221; PERF-only (absent on NOTE_EDIT): `DENS`@11,23, `12`@12,41, `PEDAL`@67,23, `6`@81,41, `JUMP`@130,23, `5`@141,41, `ACCNT`@185,23, `3`@201,41, `P01`@3,76, `ROW`@85,79, `1`@117,78, `2`@130,78, `1/1`@215,76, `PAGE`@185,79, `KEYS = STEPS`@2,221, `HOLD = EDIT`@151,221
- note_set: NOTE_EDIT-only: `PITCH`@9,23, `AUTO`@10,45, `GEN C2`@9,65, `GATE`@72,23, `AUTO`@70,45, `GEN 40`@70,65, `VELO`@131,23, `AUTO`@130,45, `GEN 43`@130,65, `RATCH`@186,23, `AUTO`@190,45, `GEN 1X`@190,65, `STEP`@3,79, `9`@34,76, `SLIDE`@50,78, `PROB 50`@87,78, `OCT-2`@140,78, `KNOBS EDIT`@2,221, `HOME = DONE`@139,221; PERF-only (absent on NOTE_EDIT): `DENS`@11,23, `12`@12,41, `PEDAL`@67,23, `6`@81,41, `JUMP`@130,23, `5`@141,41, `ACCNT`@185,23, `3`@201,41, `P01`@3,76, `ROW`@85,79, `1`@117,78, `2`@130,78, `1/1`@215,76, `PAGE`@185,79, `KEYS = STEPS`@2,221, `HOLD = EDIT`@151,221
- note_dice: NOTE_EDIT-only: `PITCH`@9,23, `F2`@13,36, `GEN C2`@9,65, `GATE`@72,23, `100`@63,36, `GEN 40`@70,65, `VELO`@131,23, `80`@132,36, `GEN 43`@130,65, `RATCH`@186,23, `AUTO`@190,45, `GEN 1X`@190,65, `STEP`@3,79, `9`@34,76, `NO SLIDE`@50,78, `PROB 50`@106,78, `OCT-2`@159,78, `KNOBS EDIT`@2,221, `HOME = DONE`@139,221; PERF-only (absent on NOTE_EDIT): `DENS`@11,23, `12`@12,41, `PEDAL`@67,23, `6`@81,41, `JUMP`@130,23, `5`@141,41, `ACCNT`@185,23, `3`@201,41, `P01`@3,76, `ROW`@85,79, `1`@117,78, `2`@130,78, `1/1`@215,76, `PAGE`@185,79, `KEYS = STEPS`@2,221, `HOLD = EDIT`@151,221
- note_final: NOTE_EDIT-only: `PITCH`@9,23, `AUTO`@10,45, `GEN C2`@9,65, `GATE`@72,23, `AUTO`@70,45, `GEN 40`@70,65, `VELO`@131,23, `83`@132,36, `GEN 43`@130,65, `RATCH`@186,23, `AUTO`@190,45, `GEN 1X`@190,65, `STEP`@3,79, `9`@34,76, `= SET BY YOU`@162,79, `KNOBS EDIT`@2,221, `HOME = DONE`@139,221; PERF-only (absent on NOTE_EDIT): `DENS`@11,23, `12`@12,41, `PEDAL`@67,23, `6`@81,41, `JUMP`@130,23, `5`@141,41, `ACCNT`@185,23, `3`@201,41, `P01`@3,76, `ROW`@85,79, `1`@117,78, `2`@130,78, `1/1`@215,76, `PAGE`@185,79, `KEYS = STEPS`@2,221, `HOLD = EDIT`@151,221


## 7. Pixel diff of the persistent parts

7a. `note_X.png` vs `note_X_perf.png` (the same machine state drawn with `uisq_page = UI_PG_PERF`, i.e. the main-screen draw path). Regions as in SCREEN_SPEC_V2 section 2.


#### note_open vs note_open_perf (same state): 9442 of 57600 pixels differ
| region | area px | differing px | % of region | clusters (x0,y0..x1,y1 : px) |
|---|---|---|---|---|
| header left (transport, tempo, chips, algo, MIDI)  x0-103 y0-21 | 2288 | 0 | 0.0 | identical |
| message slot x104-239 y0-21 | 2992 | 0 | 0.0 | identical |
| knob tiles y22-76 | 13200 | 6470 | 49.0 | 28,26..49,38:178; 67,26..111,38:385; 130,26..169,38:345; 185,26..234,38:439; 9,27..28,37:171; 1,48..58,76:1611; 61,48..118,76:1337; 121,48..178,76:834; 159,48..169,60:78; 181,48..238,76:1014; 219,48..229,60:78 |
| status line y77-91 | 3600 | 1217 | 33.8 | 12,77..13,77:2; 39,77..40,77:2; 73,77..74,77:2; 106,77..106,77:1; 133,77..134,77:2; 166,77..166,77:1; 193,77..194,77:2; 3,79..30,91:234; 34,79..42,91:73; 115,79..125,91:143; 128,79..138,91:44; 180,79..236,91:416 ... |
| music row y92-145 | 12960 | 250 | 1.9 | 224,92..225,92:2; 119,93..135,141:248 |
| buttons y146-181 | 8640 | 0 | 0.0 | identical |
| keybed y182-220 | 9360 | 41 | 0.4 | 126,186..128,186:3; 121,206..134,218:38 |
| footer y221-239 | 4560 | 1464 | 32.1 | 13,224..50,236:340; 54,224..73,236:159; 75,224..98,236:180; 150,224..236,236:714; 139,225..148,235:70; 236,226..236,226:1 |

#### note_set vs note_set_perf (same state): 10279 of 57600 pixels differ
| region | area px | differing px | % of region | clusters (x0,y0..x1,y1 : px) |
|---|---|---|---|---|
| header left (transport, tempo, chips, algo, MIDI)  x0-103 y0-21 | 2288 | 0 | 0.0 | identical |
| message slot x104-239 y0-21 | 2992 | 0 | 0.0 | identical |
| knob tiles y22-76 | 13200 | 6400 | 48.5 | 28,26..49,38:178; 67,26..111,38:385; 130,26..169,38:345; 185,26..234,38:439; 9,27..28,37:171; 1,48..58,76:1611; 61,48..118,76:1337; 121,48..178,76:834; 159,48..169,60:78; 182,48..237,76:944; 219,48..229,60:78 |
| status line y77-91 | 3600 | 2124 | 59.0 | 12,77..13,77:2; 39,77..40,77:2; 73,77..74,77:2; 106,77..106,77:1; 133,77..134,77:2; 166,77..166,77:1; 182,77..187,77:6; 232,77..237,77:6; 3,79..30,91:234; 34,79..42,91:73; 48,79..82,91:446; 85,79..116,91:403 ... |
| music row y92-145 | 12960 | 250 | 1.9 | 224,92..225,92:2; 119,93..135,141:248 |
| buttons y146-181 | 8640 | 0 | 0.0 | identical |
| keybed y182-220 | 9360 | 41 | 0.4 | 126,186..128,186:3; 121,206..134,218:38 |
| footer y221-239 | 4560 | 1464 | 32.1 | 13,224..50,236:340; 54,224..73,236:159; 75,224..98,236:180; 150,224..236,236:714; 139,225..148,235:70; 236,226..236,226:1 |

#### note_dice vs note_dice_perf (same state): 10985 of 57600 pixels differ
| region | area px | differing px | % of region | clusters (x0,y0..x1,y1 : px) |
|---|---|---|---|---|
| header left (transport, tempo, chips, algo, MIDI)  x0-103 y0-21 | 2288 | 0 | 0.0 | identical |
| message slot x104-239 y0-21 | 2992 | 0 | 0.0 | identical |
| knob tiles y22-76 | 13200 | 6915 | 52.4 | 28,26..49,38:178; 67,26..111,38:385; 130,26..169,38:345; 185,26..234,38:439; 9,27..28,37:171; 1,41..58,76:1093; 61,41..118,76:1334; 121,41..178,76:1774; 182,48..237,76:944; 219,48..229,60:78; 9,68..16,76:47; 70,68..77,76:47 ... |
| status line y77-91 | 3600 | 2315 | 64.3 | 12,77..13,77:2; 39,77..40,77:2; 73,77..74,77:2; 106,77..106,77:1; 133,77..134,77:2; 166,77..166,77:1; 182,77..187,77:6; 232,77..237,77:6; 3,79..30,91:234; 34,79..42,91:73; 48,79..154,91:1291; 157,79..212,91:602 ... |
| music row y92-145 | 12960 | 250 | 1.9 | 224,92..225,92:2; 119,93..135,141:248 |
| buttons y146-181 | 8640 | 0 | 0.0 | identical |
| keybed y182-220 | 9360 | 41 | 0.4 | 126,186..128,186:3; 121,206..134,218:38 |
| footer y221-239 | 4560 | 1464 | 32.1 | 13,224..50,236:340; 54,224..73,236:159; 75,224..98,236:180; 150,224..236,236:714; 139,225..148,235:70; 236,226..236,226:1 |

#### note_final vs note_final_perf (same state): 10256 of 57600 pixels differ
| region | area px | differing px | % of region | clusters (x0,y0..x1,y1 : px) |
|---|---|---|---|---|
| header left (transport, tempo, chips, algo, MIDI)  x0-103 y0-21 | 2288 | 0 | 0.0 | identical |
| message slot x104-239 y0-21 | 2992 | 0 | 0.0 | identical |
| knob tiles y22-76 | 13200 | 7275 | 55.1 | 28,26..49,38:178; 67,26..111,38:385; 130,26..169,38:345; 185,26..234,38:439; 9,27..28,37:171; 122,41..177,75:1692; 1,48..58,76:1611; 61,48..118,76:1337; 181,48..238,76:1014; 219,48..229,60:78; 134,72..137,75:13; 122,76..127,76:6 ... |
| status line y77-91 | 3600 | 1226 | 34.1 | 12,77..13,77:2; 39,77..40,77:2; 73,77..74,77:2; 106,77..106,77:1; 122,77..127,77:6; 172,77..177,77:6; 193,77..194,77:2; 3,79..30,91:234; 34,79..42,91:73; 115,79..125,91:143; 128,79..138,91:44; 180,79..236,91:416 ... |
| music row y92-145 | 12960 | 250 | 1.9 | 224,92..225,92:2; 119,93..135,141:248 |
| buttons y146-181 | 8640 | 0 | 0.0 | identical |
| keybed y182-220 | 9360 | 41 | 0.4 | 126,186..128,186:3; 121,206..134,218:38 |
| footer y221-239 | 4560 | 1464 | 32.1 | 13,224..50,236:340; 54,224..73,236:159; 75,224..98,236:180; 150,224..236,236:714; 139,225..148,235:70; 236,226..236,226:1 |

7b. note_X.png (this evidence's re-render with the current build/gen) vs build/ui_sq/note_X.png (written by tests/ui_sq_note_test.c at 08:39; build/gen/ui_fonts.h was regenerated 08:40)
- note_open: 201 px differ; by region header left (transport, tempo, chips, algo, MIDI) 115, message slot 25, knob tiles y22-76 40, status line y77-91 21
  differing pixels inside a text ink box: 201 of 201; outside any text ink box: 0
- note_set: 243 px differ; by region header left (transport, tempo, chips, algo, MIDI) 115, message slot 25, knob tiles y22-76 40, status line y77-91 63
  differing pixels inside a text ink box: 243 of 243; outside any text ink box: 0
- note_dice: 562 px differ; by region header left (transport, tempo, chips, algo, MIDI) 115, message slot 25, knob tiles y22-76 359, status line y77-91 63
  differing pixels inside a text ink box: 562 of 562; outside any text ink box: 0
- note_final: 294 px differ; by region header left (transport, tempo, chips, algo, MIDI) 115, message slot 25, knob tiles y22-76 133, status line y77-91 21
  differing pixels inside a text ink box: 294 of 294; outside any text ink box: 0


Summary of 7a: header, message slot and the 12 button tiles are identical (0 differing px) on all four frames. The keybed differs by 41 px (the edited key's white frame, 38 + 3 px); the music row by 250 px
(the edit frame 248 px + 2 px of the PERF `PAGE 1/1` text descender at x=224..225, y=92); the knob tiles, the status line and the footer differ by design (tile content, `STEP`/chips/legend vs `P01 ROW PAGE`, `KNOBS EDIT / HOME = DONE` vs `KEYS = STEPS / HOLD = EDIT`).

## 8. Reproduce

```sh
.venv/bin/python docs/design/ui_review/critique_note_edit/scripts/make_trace.py     # instrumented test -> data/*.png, *.ops, tokens.tsv, widths.tsv
for s in s1_text_inventory s2_contrast s3_grid s4_accuracy s5_edge s6_counts s7_diff; do
  .venv/bin/python docs/design/ui_review/critique_note_edit/scripts/$s.py > docs/design/ui_review/critique_note_edit/data/$s.out.md; done
.venv/bin/python docs/design/ui_review/critique_note_edit/scripts/assemble_md.py   # rewrites this file
```
