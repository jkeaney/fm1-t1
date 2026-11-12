# SPDX-License-Identifier: GPL-3.0-only (run from repo root after make_trace.py and the s*.py scripts have written data/*.out.md)
import re, os
D = "docs/design/ui_review/critique_note_edit"
def rd(n): return open(os.path.join(D, "data", n)).read()
def cut(txt, start, end=None):
    i = txt.index(start); j = txt.index(end, i + 1) if end else len(txt)
    return txt[i:j].rstrip() + "\n"
s1, s2, s3, s4, s5, s6, s7 = (rd("s%d_%s.out.md" % (i, n)) for i, n in ((1, "text_inventory"), (2, "contrast"), (3, "grid"), (4, "accuracy"), (5, "edge"), (6, "counts"), (7, "diff")))
OUT = []
w = OUT.append
w("""<!-- SPDX-License-Identifier: GPL-3.0-only -->
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

""")
w("## 1. Text inventory per render\n")
w(cut(s1, "cap px measured", "#### note_open").replace("cap px measured:  ", "Measured cap px: ") + "\n")
for n in ("note_open", "note_set", "note_dice", "note_final", "worst_w1", "worst_w2"):
    w(cut(s1, "#### %s " % n, "#### " if False else None).split("\n#### ")[0].split("\n### icons")[0] + "\n")
w("### Icons and glyphs (1 bpp, non-text) per render\n" + cut(s1, "### icons / glyphs drawn").split("\n", 1)[1] + "\n")
w("""Persistent vs page-specific: the header (`120`, chips `1`..`4`, `ACID`, `MIDI`, `1`), the four dice icons + EDIT/GLO/HOME/SAVE/ARP/SEQ/PLAY/REC icons, the black-key glyphs (`S F R`, MIDI glyph, arrows, RST, muted `x` icons) and the 16 white keys
are drawn by the shared element code; the knob tiles, status line and footer are NOTE_EDIT's (section 6 lists the strings that exist on one page only).

""")
w("## 2. Contrast (HI-CON)\n\nFlag rules: text under 4.5:1, non-text under 3:1.\n")
w(cut(s2, "### tokens on HI-CON", "### (a) text pairs"))
w("**Text pairs.** The fg/bg ratio of every text draw is in the last column of the section-1 tables; the actual pixel colour under each ink box equals the stated background in every case (0 mismatches over the six frames, `scripts/s2_contrast.py` (a)). Distinct pairs:\n")
w(cut(s2, "### (a2) distinct text colour pairs", "### (b) non-text").split("\n", 1)[1])
w("Text pairs under 4.5:1: **none** (lowest: MID on SURF 6.75, used for the tile labels, AUTO, the GEN lines of AUTO tiles, header chips 2-4).\n")
w("**Non-text pairs that touch on the final frame** (4-neighbour edges, text ink masked; edge px = number of touching pixel pairs). Rows flagged `!` are under 3:1.\n")
w(cut(s2, "### (b) non-text", "### (c) named").split("\n", 1)[1])
w(cut(s2, "### (c) named non-text pairs") + "\n")
w("Non-text pairs under 3:1 present on all four frames: BG/SURF 1.43 (tile/button/key bodies against the screen), BG/RAISE 2.03 (black keys), CH1/WHITE 2.29 (the edit frame against the amber bars and the amber key front), BG/DICE 2.32. Present when a tile is set (`note_dice`, `note_final`): BG/CHF1 2.54 (the set fill against the gap). The set fill against the AUTO fill, CHF1/SURF, is 1.78 (they do not touch). Level edge CH1/CHF1 3.62 passes.\n")
w("## 3. Grid, bands, alignment, overlap, clipping\n\nSpec: `docs/design/SCREEN_SPEC_V2.md` section 2 (16 columns of 15 px = the white keys; 4 x 60 knob tiles; 6 x 40 buttons; bands: header 1-19 (+ 20-21 bar), tiles 24-76, status 77-91 (caps 80), music row 92-141, buttons 146-180, keybed 186-220, footer caps 225-236). Firmware strips 0-119 and 120-239.\n")
w("""**Summary (facts)**
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
""")
w(cut(s3, "### note_open", "## geometry").replace("### ", "#### ").replace("#### texts:", "texts:") if False else "")
w("### 3a. Per-render text positions (lit pixel boxes, column index, band)\n")
body = s3[:s3.index("## geometry of the NOTE_EDIT")]
body = body.replace("\n### ", "\n#### ").replace("\n#### texts:", "\ntexts:")
w(body + "\n")
geo = s3[s3.index("## geometry of the NOTE_EDIT"):]
geo = geo.replace("## geometry", "### 3b. Geometry").replace("\n### ", "\n#### ").replace("\n## status line capacity", "\n#### Status line capacity")
w(geo + "\n")
w("## 4. Accuracy against source tables\n\nSources: `docs/design/button_modes.tsv`, `knob_map.tsv`, `key_map.tsv`, `note_edit_text.tsv`, `page_states.tsv`, scenario `docs/design/scenarios/S03-step-entry-note-edit-pages.md` steps 7-22, feature `docs/design/features/P4-014-note-edit-page.md` section 5 (proposed picture).\n")
w(s4.split("### page_states.tsv")[0] + "\n")
w("""### 4a. page_states.tsv, S03 and feature-file text vs the frames (strings quoted from the sources)

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

""")
w("## 5. Edge cases from `ui_ne_text` and the strings built from it\n")
w(s5 + "\n")
w("""### 5a. The two hand-set frames
- W1 (step 64, all set, 4 chips): `STEP` `64`, chips `NO SLIDE` x=59, `PROB 50` x=115, `OCT+2` x=168, `OFF` x=211; last chip right edge x=235 (limit 237); no legend; edit frame outer ring x=224..240 (x=240 is outside the 240 px screen); PITCH `C#-1` drawn in AF_M (AF_L is 67 px); GEN line `GEN G#-1` 54 px
  (slot 58) at x=3..55 in tile 1..58; `GATE 100` AF_L 54 px (limit 54, fits exactly); `ACC` AF_M; `4X` AF_L. No clipping, no ellipsis, no overlap.
- W2 (step 1, set PITCH `A#-1`, GATE `TIE`, VELO `ACC`): PITCH value AF_M 34 px; GEN lines `GEN D#-1` (53), `GEN TIE`, `GEN ACC` (50); edit frame outer ring at x=-1 (outside the screen); legend present.

""")
w("## 6. Counts per render (distinct text items and colours)\n")
w(s6.split("\n", 1)[0] if False else s6 + "\n")
w("## 7. Pixel diff of the persistent parts\n\n7a. `note_X.png` vs `note_X_perf.png` (the same machine state drawn with `uisq_page = UI_PG_PERF`, i.e. the main-screen draw path). Regions as in SCREEN_SPEC_V2 section 2.\n")
w(s7.replace("\n### note_X.png", "\n7b. note_X.png").replace("### ", "#### ") + "\n")
w("""Summary of 7a: header, message slot and the 12 button tiles are identical (0 differing px) on all four frames. The keybed differs by 41 px (the edited key's white frame, 38 + 3 px); the music row by 250 px
(the edit frame 248 px + 2 px of the PERF `PAGE 1/1` text descender at x=224..225, y=92); the knob tiles, the status line and the footer differ by design (tile content, `STEP`/chips/legend vs `P01 ROW PAGE`, `KNOBS EDIT / HOME = DONE` vs `KEYS = STEPS / HOLD = EDIT`).

## 8. Reproduce

```sh
.venv/bin/python docs/design/ui_review/critique_note_edit/scripts/make_trace.py     # instrumented test -> data/*.png, *.ops, tokens.tsv, widths.tsv
for s in s1_text_inventory s2_contrast s3_grid s4_accuracy s5_edge s6_counts s7_diff; do
  .venv/bin/python docs/design/ui_review/critique_note_edit/scripts/$s.py > docs/design/ui_review/critique_note_edit/data/$s.out.md; done
.venv/bin/python docs/design/ui_review/critique_note_edit/scripts/assemble_md.py   # rewrites this file
```
""")
open(os.path.join(D, "EVIDENCE_B_note_edit.md"), "w").write("\n".join(OUT))
