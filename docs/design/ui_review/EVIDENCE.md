# UI review: EVIDENCE (B)

Facts and measurements only: no opinions, no recommendations. Inputs: `firmware/src/ui_sq.c` (P4-011), the generated tables `build/gen/ui_tables.h` and fonts `build/gen/ui_fonts.h`,
`docs/design/SCREEN_SPEC.md`, `SEQUENCER_UX.md`, `page_states.tsv`, `button_modes.tsv`, `knob_map.tsv`, `key_map.tsv`, `help_text.tsv`, `control_map.tsv`, `tests/design/widths.tsv`.

## 0. How this was measured

- `measure.sh` compiles the real `gfx.c` (real fonts, real `text_w`, real `cv_text_fit`), the real `ui_sq.c`, `sq_core.c`, `sq_chan.c` and the generated tables on the host, with the test harness of `tests/ui_sq_test.c` (its prefix is reused verbatim), and hooks every text and rectangle that `ui_sq.c` draws (`GFX_HOOK_TEXT`, `GFX_HOOK_INK`, `GFX_HOOK_PEN`, wrappers on `cv_text_fit`, `cv_rect`, `cv_frame`). No firmware or TSV file is edited.
- Scenarios dumped (raw: `build/ui_review/texts_all.tsv`, about 15 000 draw records): PERF in 11 states (default power-on, ROW 2 live, knob hot, test render with length 20, playing, playing with CH2 selected, viewing page 2, toast, algorithm-pick strip, mutes, mute + MIDI OFF), every page x every algorithm id 0..5 on the selected channel (13 pages; MENU is the Felucca menu and draws nothing), and all four channels on every algorithm with MIDI 16.
- Palette: the screenshots in `build/ui_sq/` and all numbers here use palette 0 **GREY** (`panel.c` sets `settings.palette = UI_GREY_INDEX` as the default). Contrast is also computed for all 10 palettes of `ui_palettes.h`.
- Algorithm data: the shipped `ALGOS[]` (`sq_algos.c`) has only row 0 (ACID) filled; rows 1..5 are zero, for which `ui_sq.c` draws no dice sub-label and the raw knob index as the morph slot. The test harness (and the PNGs) use a stand-in `ALGOS` table with names, morph ranges and sub-labels for all six. `measure.sh` replaces the stand-in ACID `knob_morph` (identity) with the shipped `{0,5,2,3,4,1,6,7}` so ACID knob labels and values agree; algorithms 1..5 are the stand-in's.
- Colour roles (`T_TEXT` ...) are assigned from the draw calls in `ui_sq.c` and verified against the RGB565 values the real draw recorded; the sRGB hex uses `r*255/31, g*255/63, b*255/31` as the test's PNG writer does. A `-` or `...` in a state column means the state does not change that element.
- Pixel coordinates are screen pixels (0..239). Text positions are the pen (line top) the code passes; ink rows are measured from the real glyph boxes.
- Files: `EVIDENCE.md` (this), `elements.tsv` (86 rows, one per drawn element), `contrast.py`, `fonts.tsv`, `palettes.tsv`, `text_pairs.tsv`, `truncations.tsv`, `measure.sh` + `measure_body.c` + `mh_decl.h` (instrumentation), `analyze.py`, `build_evidence.py`.

## 1. Palette and contrast

#### Colours used by ui_sq.c (palette 0 GREY, the firmware default: panel.c sets settings.palette = UI_GREY_INDEX)

| Role | RGB565 | sRGB hex | Plain name |
| --- | --- | --- | --- |
| T_ACCENT | 0xFFDF | #FFFAFF | very light grey |
| T_BG | 0x0841 | #080808 | black grey |
| T_DIM | 0x5ACB | #5A595A | mid grey |
| T_GRID | 0x2945 | #292829 | dark grey |
| T_INK | 0x0841 | #080808 | black grey |
| T_LANE | 0x39C7 | #393839 | dark grey |
| T_LINE | 0x2945 | #292829 | dark grey |
| T_MID | 0x9492 | #949194 | mid grey |
| T_RAISE | 0x39C7 | #393839 | dark grey |
| T_REC | 0xFFDF | #FFFAFF | very light grey |
| T_SEL | 0xBDD7 | #BDBABD | light grey |
| T_TEXT | 0xCE59 | #CDCACD | light grey |
| T_THEME | 0xEF5D | #EEEAEE | very light grey |
| T_TINT | 0x2104 | #202020 | very dark grey |
| (fixed) clock dot dark | 0xFD80 | #FFB200 | mid orange/yellow |
| (fixed) clock dot light | 0xCB80 | #CD7100 | mid orange/yellow |

#### Contrast of TEXT (fg ink on the bg it is drawn on), every page and algorithm

Each row is a pair some text in ui_sq.c is drawn with; `used by` lists element classes (n = index) and the pages where it occurs.

| fg on bg | used by | GREY | GREEN | AMBER | ICE | VIOLET | ROSE | PAPER | HI-CON | NIGHT | MONO | min (palette) |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| T_DIM on T_RAISE | KEY.Bn [ADD_PAGE,ALGO_PICK,CHAN,FIRST_RUN,MOVE_HELD,NOTE_EDIT,PERF,REC,RESET,RIG,SCALE,SET] | 1.67 | 2.17 | 2.25 | 2.27 | 2.26 | 2.38 | 1.69 | 1.89 | 2.08 | 1.00 | 1.00 (MONO) |
| T_DIM on T_BG | BTN.Rn.Cn.SUB,BTN.Rn.Cn.TAP,CHn.Ln,H5.1,H5.2,KNOB.Rn.Kn.LABEL,KNOB.Rn.Kn.VALUE [ADD_PAGE,ALGO_PICK,CHAN,FIRST_RUN,HELP,MOVE_HELD,NOTE_EDIT,PERF,REC,RESET,RIG,SCALE,SET] | 2.87 | 3.77 | 3.80 | 3.80 | 3.71 | 3.80 | 2.58 | 3.84 | 3.03 | 2.63 | 2.58 (PAPER) |
| T_MID on T_BG | BTN.Rn.Cn.SUB,CHn.Ln,KNOB.Rn.Kn.LABEL,LANE.HDR,STATUS.2 [ADD_PAGE,ALGO_PICK,CHAN,FIRST_RUN,HELP,MOVE_HELD,NOTE_EDIT,PERF,REC,RESET,RIG,SCALE,SET] | 6.42 | 8.80 | 8.51 | 8.53 | 8.66 | 8.51 | 5.70 | 9.65 | 7.00 | 21.00 | 5.70 (PAPER) |
| T_TEXT on T_LANE | LANE.GLYPH [ADD_PAGE,ALGO_PICK,CHAN,FIRST_RUN,HELP,MOVE_HELD,NOTE_EDIT,PERF,REC,RESET,RIG,SET] | 7.19 | 11.50 | 11.43 | 11.52 | 11.83 | 12.66 | 10.61 | 10.77 | 10.29 | 7.99 | 7.19 (GREY) |
| T_TEXT on T_RAISE | KEY.Bn [ADD_PAGE,ALGO_PICK,CHAN,FIRST_RUN,MOVE_HELD,NOTE_EDIT,PERF,REC,RESET,RIG,SCALE,SET] | 7.19 | 9.98 | 9.99 | 10.06 | 10.15 | 10.62 | 9.65 | 10.01 | 9.72 | 7.99 | 7.19 (GREY) |
| T_INK on T_SEL | CHn.Ln,H5.1,KNOB.Rn.Kn.LABEL,KNOB.Rn.Kn.VALUE,TAG.OCTDN.L1,TAG.OCTDN.L2,TAG.OCTUP.L1,TAG.OCTUP.L2 [ADD_PAGE,ALGO_PICK,CHAN,FIRST_RUN,HELP,MOVE_HELD,PERF,RESET,RIG,SCALE,SET] | 10.42 | 6.92 | 6.67 | 6.00 | 5.06 | 5.17 | 4.70 | 10.62 | 7.62 | 21.00 | 4.70 (PAPER) |
| T_TEXT on T_BG | BTN.Rn.Cn.TAP,CHn.Ln,CUE,H1,H2,H3,H4,H6,TAG.OCTDN.L1,TAG.OCTDN.L2,TAG.OCTUP.L1,TAG.OCTUP.L2,Z2.HARMONY [ADD_PAGE,ALGO_PICK,CHAN,FIRST_RUN,HELP,MOVE_HELD,NOTE_EDIT,PERF,REC,RESET,RIG,SCALE,SET] | 12.33 | 17.34 | 16.88 | 16.86 | 16.67 | 16.98 | 14.74 | 20.37 | 14.17 | 21.00 | 12.33 (GREY) |
| T_ACCENT on T_TINT | TOAST [PERF] | 15.80 | 11.32 | 9.41 | 7.01 | 10.64 | 12.05 | 4.54 | 11.21 | 17.26 | 21.00 | 4.54 (PAPER) |
| T_ACCENT on T_BG | BTN.Rn.Cn.TAP,KNOB.Rn.Kn.VALUE,LANE.PLAYPAGE,Z1.BANNER [HELP,NOTE_EDIT,PERF,RESET,SCALE,SET] | 19.43 | 13.94 | 11.39 | 8.42 | 12.44 | 13.95 | 5.50 | 13.76 | 20.37 | 21.00 | 5.50 (PAPER) |
| T_REC on T_BG | H1,Z1.BANNER [REC] | 19.43 | 5.83 | 5.82 | 5.72 | 5.77 | 5.81 | 5.50 | 6.28 | 6.28 | 21.00 | 5.50 (PAPER) |

Lowest ratio over all pairs and palettes: 1.00 (T_DIM on T_RAISE, MONO).
Lowest ratio on palette 0 GREY: 1.67 (T_DIM on T_RAISE).
Pairs under 3.0:1 on GREY: 2 of 10 (T_DIM/T_BG, T_DIM/T_RAISE)
Pairs under 4.5:1 on GREY: 2 of 10 (T_DIM/T_BG, T_DIM/T_RAISE)
Pairs under 7.0:1 on GREY: 3 of 10 (T_DIM/T_BG, T_DIM/T_RAISE, T_MID/T_BG)

#### Contrast of non-text GRAPHICS (fills, outlines, bars, dividers)

Pairs from the graphic rows of elements.tsv that no text uses. A fill's ratio is against the background it sits on.

| fg on bg | used by | GREY | GREEN | AMBER | ICE | VIOLET | ROSE | PAPER | HI-CON | NIGHT | MONO | min (palette) |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| T_TINT on T_BG | TOAST.FILL | 1.23 | 1.23 | 1.21 | 1.20 | 1.17 | 1.16 | 1.21 | 1.23 | 1.18 | 1.00 | 1.00 (MONO) |
| T_GRID on T_LANE | LANE.BAR | 1.26 | 1.03 | 1.02 | 1.03 | 1.01 | 1.03 | 1.04 | 1.10 | 1.08 | 2.63 | 1.01 (VIOLET) |
| T_LINE on T_BG | BTN.SEPARATOR | 1.36 | 1.58 | 1.59 | 1.61 | 1.60 | 1.58 | 1.43 | 1.54 | 1.36 | 2.63 | 1.36 (NIGHT) |
| T_RAISE on T_BG | KEY.CELL | 1.72 | 1.74 | 1.69 | 1.68 | 1.64 | 1.60 | 1.53 | 2.03 | 1.46 | 2.63 | 1.46 (NIGHT) |
| T_THEME on T_LANE | LANE.BAR | 9.81 | 6.93 | 6.93 | 5.99 | 5.12 | 5.48 | 5.52 | 9.00 | 8.56 | 7.99 | 5.12 (VIOLET) |
| T_SEL on T_BG | KNOB.LIVE_FILL; TAG.LIVE_FILL | 10.42 | 6.92 | 6.67 | 6.00 | 5.06 | 5.17 | 4.70 | 10.62 | 7.62 | 21.00 | 4.70 (PAPER) |
| T_ACCENT on T_LANE | LANE.BAR | 11.32 | 9.24 | 7.71 | 5.75 | 8.82 | 10.40 | 3.96 | 7.27 | 14.78 | 7.99 | 3.96 (PAPER) |

Lowest ratio over all pairs and palettes: 1.00 (T_TINT on T_BG, MONO).
Lowest ratio on palette 0 GREY: 1.23 (T_TINT on T_BG).
Pairs under 3.0:1 on GREY: 4 of 7 (T_GRID/T_LANE, T_LINE/T_BG, T_RAISE/T_BG, T_TINT/T_BG)
Pairs under 4.5:1 on GREY: 4 of 7 (T_GRID/T_LANE, T_LINE/T_BG, T_RAISE/T_BG, T_TINT/T_BG)
Pairs under 7.0:1 on GREY: 4 of 7 (T_GRID/T_LANE, T_LINE/T_BG, T_RAISE/T_BG, T_TINT/T_BG)



Notes on the contrast numbers:
- In the MONO palette `T_DIM`, `T_RAISE`, `T_LINE` and `T_LANE` are all the same grey (0x528A), so the ratio of T_DIM on T_RAISE reads 1.00; `ramp()` in `gfx.c` substitutes `T_BG` ink when fg == bg ("MONO: DIM on a RAISE well: black, recessed"), so the drawn ink is not the 1.00 pair.
- On GREY, `T_BG`, `T_INK` are the same value (0x0841), `T_SURF`/`T_TINT` (0x2104), `T_LINE`/`T_GRID` (0x2945), `T_RAISE`/`T_LANE` (0x39C7) and `T_ACCENT`/`T_REC`/(0xFFDF) coincide. Text drawn on a `T_RAISE` legend well (Z5) and on a `T_LANE` step cell (Z2) is on the same 0x39C7.
- Antialiased text: the glyph ramp runs from the bg to the fg over 16 levels, so the ratios above are the maximum (full-coverage) ratio of each pair; thin strokes of AF_S (stems 1 px wide) never reach full coverage on every pixel.
- Contrast of `T_ACCENT` vs `T_SEL` and `T_TEXT` vs `T_SEL` is not drawn anywhere on the PERF screen (text on a `T_SEL` fill is always `T_INK`).

## 2. Fonts

`ui_sq.c` draws every string with **AF_S** (grep: no `AF_M`, no `AF_L` in the file). AF_M and AF_L are listed because SCREEN_SPEC 1 and the spec table name them ("AF_M is used only for one-word banners"); `ui_sq.c` does not use them. Data: `build/gen/ui_fonts.h` (generated by `tools/gen_aa_font.py`, Inter Tight, 4-bit alpha, 4 horizontal phases for S and M, 2 for L); measured by `dump_font()` in `measure_body.c` from the real glyph tables; `fonts.tsv` is the full dump.

| Property | AF_S | AF_M | AF_L |
| --- | --- | --- | --- |
| Nominal size (spec / header comment) | 12 px ("12 px line") | 15 px | 28 px |
| Line box height `h` (struct field) | 15 px | 19 px | 35 px |
| Baseline (`asc`) | row 12 | row 15 | row 28 |
| Capital ink height (H) | 9 px (rows 3..11) | 11 px (rows 4..14) | 21 px (rows 7..27) |
| Lowercase x-height (x) | 7 px (rows 5..11) | 9 px (rows 6..14) | absent (lowercase folded to capitals) |
| Ascender ink (d / l) | 10 px (rows 3..12) / 9 px (rows 3..11) | 12 px (rows 4..15) / 11 px (rows 4..14) | absent |
| Descender ink (g / p / y) | 10 px (rows 5..14) / 10 px (rows 5..14) / 10 px (rows 5..14) | 13 px (rows 6..18) / 13 px (rows 6..18) / 13 px (rows 6..18) | absent |
| Digit ink height (0 / 1) | 10 px (rows 3..12) / 9 px (rows 3..11) | 13 px (rows 3..15) / 11 px (rows 4..14) | 22 px (rows 7..28) / 21 px (rows 7..27) |
| Small marks: `.` / `'` / `~` / `-` ink height | 2 / 4 / 3 / 2 px | 3 / 5 / 4 / 3 px | `.` 6 px |
| Mean advance: A-Z / a-z / 0-9 | 7.51 / 5.84 / 7.00 px | 9.58 / 7.52 / 9.00 px | 18.31 / - / 18.00 px |
| Widest advance (ASCII 33..126), mean advance of all non-space glyphs | 11(@), 6.20 px | 14(W), 7.98 px | n/a (digits 18) |
| Typical glyph widths (advance, px) | `0` 7, `W` 11, `i` 2 | `0` 9, `W` 14, `i` 3 | `0` 18, `W` 27, `i` 7 |
| Real `text_w` samples | `ACCENT` 46, `HELLO WORLD` 79 | `ACCENT` 58, `HELLO WORLD` 100 | `ACCENT` 111 (no lowercase), `HELLO WORLD` 190 |
| Lowercase exists | yes (a-z, 26 glyphs) | yes | **no**: `fold()` maps a-z to A-Z |
| Characters present | ASCII 32..126 (95 glyphs) + extras 169 176 183 196 214 220 228 246 252 and 133 (the ellipsis `…`, the glyph `text_fit` appends) | same set | space (32) + extras: `# + - . /`, `0-9`, `A`..`Y` (no `Z`, no `,` `:` `;` `'` `!` `~` `<` `>` `=` `(` `)` `%` `[` `]` `*`) |
| Kerning pairs | 919 | 992 | 123 |
| Cap band (macro) | `AF_S_CAP_Y 3`, `AF_S_CAP_H 9` | 4, 11 | 7, 21 |

Sizes in use on the PERF screen (not font faces): clock dot 8x8, hold mark 5x7, activity dot 4x4, page-map cells 9x11, lane step cell 14x46 (bar 12 px wide, 2..32 px high, glyph row 14 px), separator 1x20. The smallest text ink in use is AF_S: capitals/digits 9..10 px high, lowercase x-height 7 px, the punctuation glyphs `.` (2 px), `'` (4 px), `~` (3 px), `-` (2 px).

## 3. Screen map of the PERF page

### 3.1 Zones (SCREEN_SPEC 2; constants `UISQ_Z*` in `ui_sq.c`)

| Zone | Name | x,y,w,h | Blit | Font | Holds |
| --- | --- | --- | --- | --- | --- |
| Z0 | HEAD | 0,0,240,20 | A (0..108) | AF_S | transport, clock dot + tempo, preset, channel/algorithm/MIDI, page map, length |
| Z1 | CHANNELS | 0,20,240,28 | A | AF_S | 4 cells 60x28 |
| Z2 | LANE | 0,48,240,60 | A | AF_S | header line 14 px + 16 cells 15x46 pitch |
| Z3 | BAND | 0,108,240,52 | B (108..212) | AF_S | tag column 36 px + 4 knob cells 51 px; PERF: 2 rows of 26 px |
| Z4 | BUTTONS | 0,160,240,52 | B | AF_S | 2 rows x 6 buttons, 26 px each, tap line + sub line |
| Z5 | LEGEND | 0,212,240,12 | C (212..240) | AF_S | 11 black-key cells 20x12 at pitch 21/22/23 |
| Z6 | CUE / TOAST | 0,224,240,12 | C | AF_S | cue strip or toast |
| Z7 | MARGIN | 0,236,240,4 | C | none | blank (T_BG) |

Blit sizes: A 240x108 = 25 920 px, B 240x104 = 24 960 px, C 240x28 = 6 720 px (limit 240x124). Redraw: a blit is redrawn when the signature of its inputs changes or `ui.force`. Timing constants: toast 2000 ms (`UISQ_TOAST_MS`), ALGO_PICK strip 2000 ms (`UISQ_PICK_MS`), hot knob 600 ms (`UISQ_HOT_MS`).

### 3.2 Per-element map (text elements; every string is `AF_S`)

Columns: slot x,y,w,h = the box SCREEN_SPEC 2 / the draw code assigns (screen px); default string = what PERF draws at power-on (`PERF_default`); px = `text_w`; limit = the width the code passes to `cv_text_fit` (or the cell width for button taps); variants = distinct strings seen over all PERF scenarios and all six algorithm ids (`elements.tsv` has them with widths); colours = fg/bg roles with the state that selects them.

**Z0 HEAD**

| id | x,y,w,h | default string | px / limit | variants | colours observed (fg/bg) |
| --- | --- | --- | --- | --- | --- |
| H1 | 2,4,10,12 | `.` | 3 / 10 | 2 | T_TEXT/T_BG |
| H2 | 23,4,45,12 | `INT 120` | 42 / 45 | 1 | T_TEXT/T_BG |
| H3 | 68,4,28,12 | `P01` | 21 / 28 | 1 | T_TEXT/T_BG |
| H4 | 96,4,76,12 | `1 ACID M1` | 56 / 76 | 13 | T_TEXT/T_BG |
| H5.1 | 174,4,9,11 | `1` | 7 / 9 | 1 | T_DIM/T_BG; T_INK/T_SEL |
| H5.2 | 184,4,9,11 | `2` | 7 / 9 | 1 | T_DIM/T_BG |
| H6 | 214,4,24,12 | `L16` | 20 / 24 | 2 | T_TEXT/T_BG |

**Z1 CHANNELS**

| id | x,y,w,h | default string | px / limit | variants | colours observed (fg/bg) |
| --- | --- | --- | --- | --- | --- |
| CH1.L1 | 2,22,56,12 | `1 ACID` | 36 / 56 | 6 | T_INK/T_SEL; T_TEXT/T_BG |
| CH1.L2 | 2,34,56,12 | `MIDI 1` | 33 / 56 | 2 | T_INK/T_SEL; T_MID/T_BG |
| CH2.L1 | 62,22,56,12 | `2 EUCL` | 39 / 56 | 6 | T_DIM/T_BG; T_TEXT/T_BG; T_INK/T_SEL |
| CH2.L2 | 62,34,56,12 | `MUTE 2` | 42 / 56 | 4 | T_DIM/T_BG; T_MID/T_BG; T_INK/T_SEL |
| CH3.L1 | 122,22,56,12 | `3 SNAK` | 40 / 56 | 6 | T_DIM/T_BG |
| CH3.L2 | 122,34,56,12 | `MUTE 3` | 42 / 56 | 4 | T_DIM/T_BG |
| CH4.L1 | 182,22,56,12 | `4 STAG` | 39 / 56 | 6 | T_DIM/T_BG |
| CH4.L2 | 182,34,56,12 | `MUTE 4` | 42 / 56 | 2 | T_DIM/T_BG |

**Z2 LANE**

| id | x,y,w,h | default string | px / limit | variants | colours observed (fg/bg) |
| --- | --- | --- | --- | --- | --- |
| LANE.GLYPH | 0,95,14,13 | `!` | 3 / 14 | 4 | T_TEXT/T_LANE |
| LANE.HDR | 2,49,234,12 | `PG 1/1  FOLLOW  hold=EDIT` | 148 / 234 | 3 | T_MID/T_BG |
| LANE.PLAYPAGE | 200,49,36,12 | `>P1` | 21 / 36 | 1 | T_ACCENT/T_BG |

**Z3 BAND**

| id | x,y,w,h | default string | px / limit | variants | colours observed (fg/bg) |
| --- | --- | --- | --- | --- | --- |
| TAG.OCTDN.L1 | 0,109,36,12 | `ROW` | 27 / 33 | 1 | T_INK/T_SEL; T_TEXT/T_BG |
| TAG.OCTDN.L2 | 2,121,33,12 | `1` | 7 / 33 | 1 | T_INK/T_SEL; T_TEXT/T_BG |
| TAG.OCTUP.L1 | 0,135,36,12 | `ROW` | 27 / 33 | 1 | T_TEXT/T_BG; T_INK/T_SEL |
| TAG.OCTUP.L2 | 2,147,33,12 | `2` | 7 / 33 | 1 | T_TEXT/T_BG; T_INK/T_SEL |
| KNOB.R1.K1.LABEL | 38,109,49,12 | `DENS` | 30 / 49 | 2 | T_INK/T_SEL; T_DIM/T_BG |
| KNOB.R1.K1.VALUE | 38,121,49,12 | `12` | 14 / 49 | 6 | T_INK/T_SEL; T_DIM/T_BG |
| KNOB.R1.K2.LABEL | 89,109,49,12 | `SLIDE` | 31 / 49 | 6 | T_INK/T_SEL; T_DIM/T_BG |
| KNOB.R1.K2.VALUE | 89,121,49,12 | `4` | 7 / 49 | 4 | T_INK/T_SEL; T_DIM/T_BG |
| KNOB.R1.K3.LABEL | 140,109,49,12 | `JUMP` | 31 / 49 | 5 | T_INK/T_SEL; T_DIM/T_BG |
| KNOB.R1.K3.VALUE | 140,121,49,12 | `5` | 7 / 49 | 4 | T_INK/T_SEL; T_DIM/T_BG |
| KNOB.R1.K4.LABEL | 191,109,49,12 | `ACCNT` | 39 / 49 | 4 | T_INK/T_SEL; T_DIM/T_BG |
| KNOB.R1.K4.VALUE | 191,121,49,12 | `3` | 7 / 49 | 3 | T_INK/T_SEL; T_DIM/T_BG |
| KNOB.R2.K1.LABEL | 38,135,49,12 | `OFFSET` | 42 / 49 | 3 | T_DIM/T_BG; T_INK/T_SEL |
| KNOB.R2.K1.VALUE | 38,147,49,12 | `0` | 7 / 49 | 2 | T_DIM/T_BG; T_INK/T_SEL |
| KNOB.R2.K2.LABEL | 89,135,49,12 | `PEDAL` | 35 / 49 | 6 | T_DIM/T_BG; T_INK/T_SEL |
| KNOB.R2.K2.VALUE | 89,147,49,12 | `6` | 7 / 49 | 5 | T_DIM/T_BG; T_INK/T_SEL; T_ACCENT/T_BG |
| KNOB.R2.K3.LABEL | 140,135,49,12 | `PUNCH` | 40 / 49 | 2 | T_DIM/T_BG; T_INK/T_SEL |
| KNOB.R2.K3.VALUE | 140,147,49,12 | `40` | 14 / 49 | 4 | T_DIM/T_BG; T_INK/T_SEL |
| KNOB.R2.K4.LABEL | 191,135,49,12 | `DIR` | 18 / 49 | 4 | T_DIM/T_BG; T_INK/T_SEL |
| KNOB.R2.K4.VALUE | 191,147,49,12 | `0` | 7 / 49 | 3 | T_DIM/T_BG; T_INK/T_SEL |

**Z4 BUTTONS**

| id | x,y,w,h | default string | px / limit | variants | colours observed (fg/bg) |
| --- | --- | --- | --- | --- | --- |
| BTN.R1.C1.TAP | 0,161,23,12 | `ALL` | 20 / 23 | 1 | T_TEXT/T_BG |
| BTN.R1.C2.SUB | 23,173,41,12 | `NOTES` | 38 / 41 | 1 | T_MID/T_BG |
| BTN.R1.C2.TAP | 23,161,41,12 | `NOTES` | 38 / 41 | 1 | T_TEXT/T_BG |
| BTN.R1.C3.SUB | 64,173,42,12 | `ACCNT` | 39 / 42 | 2 | T_MID/T_BG |
| BTN.R1.C3.TAP | 64,161,42,12 | `ACCNT` | 39 / 42 | 1 | T_TEXT/T_BG |
| BTN.R1.C4.SUB | 106,173,47,12 | `OCTAVE` | 44 / 47 | 4 | T_MID/T_BG |
| BTN.R1.C4.TAP | 106,161,47,12 | `SHAPE` | 37 / 47 | 1 | T_TEXT/T_BG |
| BTN.R1.C5.SUB | 153,173,44,12 | `RESET` | 35 / 44 | 1 | T_MID/T_BG |
| BTN.R1.C5.TAP | 153,161,44,12 | `UNDO` | 33 / 44 | 1 | T_TEXT/T_BG |
| BTN.R1.C6.SUB | 197,173,43,12 | `HARM` | 33 / 43 | 1 | T_MID/T_BG |
| BTN.R1.C6.TAP | 197,161,43,12 | `SET` | 21 / 43 | 1 | T_TEXT/T_BG |
| BTN.R2.C1.SUB | 0,199,53,12 | `HLP/MN` | 44 / 53 | 1 | T_MID/T_BG |
| BTN.R2.C1.TAP | 0,187,53,12 | `HOME` | 34 / 53 | 1 | T_TEXT/T_BG |
| BTN.R2.C2.SUB | 53,199,51,12 | `UNDO L` | 42 / 51 | 1 | T_MID/T_BG |
| BTN.R2.C2.TAP | 53,187,51,12 | `SAVE` | 28 / 51 | 1 | T_TEXT/T_BG |
| BTN.R2.C3.SUB | 104,199,46,12 | `GT+SLD` | 43 / 46 | 5 | T_MID/T_BG; T_DIM/T_BG |
| BTN.R2.C3.TAP | 104,187,46,12 | `ARTIC` | 33 / 46 | 1 | T_TEXT/T_BG; T_DIM/T_BG |
| BTN.R2.C4.SUB | 150,199,28,12 | `HITS` | 25 / 28 | 4 | T_MID/T_BG |
| BTN.R2.C4.TAP | 150,187,28,12 | `HITS` | 25 / 28 | 1 | T_TEXT/T_BG |
| BTN.R2.C5.TAP | 178,187,34,12 | `PLAY` | 28 / 34 | 1 | T_TEXT/T_BG |
| BTN.R2.C6.TAP | 212,187,28,12 | `REC` | 22 / 28 | 1 | T_TEXT/T_BG |

**Z5 LEGEND**

| id | x,y,w,h | default string | px / limit | variants | colours observed (fg/bg) |
| --- | --- | --- | --- | --- | --- |
| KEY.B01 | 0,212,20,12 | `M1` | 17 / 18 | 1 | T_TEXT/T_RAISE |
| KEY.B02 | 21,212,20,12 | `M2` | 17 / 18 | 1 | T_DIM/T_RAISE; T_TEXT/T_RAISE |
| KEY.B03 | 42,212,20,12 | `M3` | 17 / 18 | 1 | T_DIM/T_RAISE |
| KEY.B04 | 65,212,20,12 | `M4` | 17 / 18 | 1 | T_DIM/T_RAISE |
| KEY.B05 | 86,212,20,12 | `CHN` | 25 / 18 | 1 | T_TEXT/T_RAISE |
| KEY.B06 | 109,212,20,12 | `SOL` | 22 / 18 | 1 | T_TEXT/T_RAISE |
| KEY.B07 | 130,212,20,12 | `FIL` | 15 / 18 | 1 | T_TEXT/T_RAISE |
| KEY.B08 | 151,212,20,12 | `ROL` | 22 / 18 | 1 | T_TEXT/T_RAISE |
| KEY.B09 | 174,212,20,12 | `PG-` | 20 / 18 | 1 | T_TEXT/T_RAISE |
| KEY.B10 | 195,212,20,12 | `PG+` | 23 / 18 | 1 | T_TEXT/T_RAISE |
| KEY.B11 | 218,212,20,12 | `RST` | 21 / 18 | 1 | T_TEXT/T_RAISE |

**Z6 CUE / TOAST**

| id | x,y,w,h | default string | px / limit | variants | colours observed (fg/bg) |
| --- | --- | --- | --- | --- | --- |
| CUE | 0,224,240,12 | `PLAY: start   K1-K4: morph` | 138 / 240 | 3 | T_TEXT/T_BG |
| TOAST | 4,224,236,12 | `SAVED P05` | 60 / 236 | 1 | T_ACCENT/T_TINT |

### 3.3 Colour by state (the rule in the draw code; verified against the recorded draw colours)

| Element | Rule |
| --- | --- |
| H1 | normal `.`/`>` T_TEXT on T_BG; REC page `R` T_REC on T_BG |
| H2 | T_TEXT on T_BG (one state; clock dot beside it is fixed amber) |
| H3 | T_TEXT on T_BG (one state) |
| H4 | T_TEXT on T_BG (one state; follows the selected channel) |
| H6 | T_TEXT on T_BG (one state) |
| H5 | digit: viewed/exists T_DIM on T_BG; playing T_INK on T_SEL fill |
| CH.L1 | normal T_TEXT/T_BG; selected T_INK/T_SEL; muted T_DIM/T_BG |
| CH.L2 | normal T_MID/T_BG; selected T_INK/T_SEL; muted T_DIM/T_BG; MIDI OFF, unmuted, unselected T_ACCENT/T_BG (no 3 s timer in code) |
| LANE.HDR | T_MID on T_BG (one state) |
| LANE.PLAYPAGE | playing only: T_ACCENT on T_BG |
| LANE.GLYPH | T_TEXT on T_LANE (one state) |
| TAG | live half (two-row pages, row selected): T_INK on T_SEL; otherwise T_TEXT on T_BG; sub-line T_MID |
| KNOB.LABEL | live row (two-row page): T_INK on T_SEL; not live: T_DIM on T_BG; single-row pages: T_MID on T_BG |
| KNOB.VALUE | live row (two-row page): T_INK on T_SEL always; not live: T_DIM on T_BG, T_ACCENT on T_BG for 600 ms after that column's knob is turned; single-row pages: T_THEME (T_ACCENT when hot) on T_BG |
| STATUS | T_MID on T_BG (single-row pages only) |
| BTN.TAP | normal T_TEXT on T_BG; dice layer absent: T_DIM; tap label differing from the printed word (RST.., BACK, CANCEL): T_ACCENT |
| BTN.SUB | T_MID on T_BG; layer absent (`-`): T_DIM |
| KEY | T_TEXT on T_RAISE; muted channel's cell (M1..M4): T_DIM on T_RAISE |
| CUE | T_TEXT on T_BG |
| TOAST | T_ACCENT on T_TINT fill |

Non-text elements (rows `kind=graphic` in `elements.tsv`, with geometry and colours): clock dot 8x8 at (12,6) fixed amber 0xFD80 (dark) / 0xCB80 (light), not a theme role; page-map cells 9x11 at x=174+10k (outline T_TEXT when viewed or playing, T_DIM otherwise, T_SEL fill when playing); channel select fill 58x28 T_SEL; activity dot 4x4 at (60c+54, 24); lane cell 14x46 at x=15n (T_LANE live, T_BG past LENGTH); lane bar 12 px wide, height 2 + vel*30/127 (T_THEME, T_ACCENT when velocity >= 100), off-step bar 12x2 T_GRID; edited frame (NOTE_EDIT) and playhead underline 14x2 at y=106 T_ACCENT; live-row fill T_SEL; separator 1x20 T_LINE; hold mark 5x7 T_MID; legend well 20x12 T_RAISE; toast fill 240x12 T_TINT.

### 3.4 Strings cut with an ellipsis (`text_fit` flag 1) in the current render

Over every scenario (PERF family and every page x algorithm): **64** distinct (page, element, string) cuts, **all in the Z5 black-key legend** (limit 18 px: a 20 px cell less 1 px each side). No label in Z0..Z4 or Z6 is cut in any scenario. The ellipsis glyph (AF_S glyph 133) replaces all but the first letter.

PERF page:

| Element | Full string | Drawn as | Full px | Limit px |
| --- | --- | --- | --- | --- |
| KEY.B05 (key D#4) | `CHN` | `C…` | 25 | 18 |
| KEY.B06 (key F#4) | `SOL` | `S…` | 22 | 18 |
| KEY.B08 (key A#4) | `ROL` | `R…` | 22 | 18 |
| KEY.B09 (key C#5) | `PG-` | `P…` | 20 | 18 |
| KEY.B10 (key D#5) | `PG+` | `P…` | 23 | 18 |
| KEY.B11 (key F#5) | `RST` | `R…` | 21 | 18 |

Other pages (same cells; full list in `truncations.tsv`):

| Element | Full string | Drawn as | Full px | Pages |
| --- | --- | --- | --- | --- |
| KEY.B05 | `BACK` | `B…` | 30 | CHAN, RIG |
| KEY.B01 | `NOTE` | `N…` | 30 | REC |
| KEY.B02 | `NOTE` | `N…` | 30 | REC |
| KEY.B03 | `NOTE` | `N…` | 30 | REC |
| KEY.B04 | `NOTE` | `N…` | 30 | REC |
| KEY.B05 | `NOTE` | `N…` | 30 | REC |
| KEY.B06 | `NOTE` | `N…` | 30 | REC |
| KEY.B07 | `NOTE` | `N…` | 30 | REC |
| KEY.B08 | `NOTE` | `N…` | 30 | REC |
| KEY.B09 | `NOTE` | `N…` | 30 | REC |
| KEY.B10 | `NOTE` | `N…` | 30 | REC |
| KEY.B11 | `NOTE` | `N…` | 30 | REC |
| KEY.B05 | `COPY P` | `C…` | 41 | RESET |

(The six cuts that repeat on every page that carries the PERF legend - CHN, SOL, ROL, PG-, PG+, RST on KEY.B05/06/08/09/10/11 - are omitted from this second table; they are the PERF table above.)

Legend labels that are NOT cut but have 1 px slack: `M1`..`M4` (17 px of 18), `FIL` (15 of 18). The four mute keys M1..M4 draw distinct strings.

### 3.5 Tightest fits (slack = limit - text_w, PERF family, all algorithm ids)

| String | Limit px | Slack px |
| --- | --- | --- |
| `M1` | 18 | 1 |
| `M2` | 18 | 1 |
| `M3` | 18 | 1 |
| `M4` | 18 | 1 |
| `RATCH` | 38 | 1 |
| `RESTS` | 36 | 1 |
| `ROTATE` | 43 | 1 |
| `RT+HLD` | 44 | 1 |
| `ACCNT` | 42 | 3 |
| `ARTIC` | 36 | 3 |
| `FIL` | 18 | 3 |
| `GATES` | 39 | 3 |
| `GT+SLD` | 46 | 3 |
| `HITS` | 28 | 3 |

Z4 button cells are sized per page and algorithm from the widest label plus padding 3 (1 when the row is tight), then widened to 240; PERF row 1 cell widths for ACID: 23 / 41 / 42 / 47 / 44 / 43 (sum 240), row 2: 53 / 51 / 46 / 28 / 34 / 28. Row-2 label ink on PERF ACID: HLP/MN 44 (+ hold mark 6), UNDO L 42 (+6), GT+SLD 43, HITS 25, PLAY 28, REC 22.

### 3.6 Ink that leaves its own box by 1 px (measured from glyph rows)

| Element | Ink rows vs own box rows |
| --- | --- |
| LANE.HDR | ink rows 51..62 vs container rows 48..61 |
| BTN.R2.C1.SUB | ink rows 201..212 vs container rows 186..211 |
| KEY.B03 | ink rows 215..224 vs container rows 212..223 |
| KEY.B05 | ink rows 215..224 vs container rows 212..223 |
| KEY.B06 | ink rows 215..224 vs container rows 212..223 |
| CUE | ink rows 223..235 vs container rows 224..235 |

These come from glyphs that reach above or below the cap band (`/`, round digits and the ellipsis: AF_S rows 2..13 for `/`, 3..12 for round glyphs); the box is the zone, cell or half the element is drawn in. The legend cell is 12 px high (rows 212..223) and Z6 text starts at row 224 (pen 221, cap top row 3).

## 4. Function, purpose, controls and states of each element

Wording in quotation marks is taken from the section named (several clauses of one section are joined and abridged; the words are the spec's). Control ids are `control_map.tsv` ids (`EN_*` encoders, `B_*` buttons, `KEY_W*` white keys, `KEY_B*` black keys).

| Element (ids in `elements.tsv`) | Function / purpose as the spec states it | Source | Driven by | States it can show |
| --- | --- | --- | --- | --- |
| H1 | "transport: `>` playing, `.` stopped, `R` REC armed (T_REC)" | SCREEN_SPEC 2.1 H1 | B_PLAY,B_REC | stopped '.', playing '>', REC armed 'R' (T_REC) |
| H2 | "clock dot (8x8 at x=12,y=6) and tempo source (section 6): dot + text from x=23" | SCREEN_SPEC 2.1 H2; 6.2 | none on PERF (tempo: EN_K1 on SET) | INT n / EXT -- / EXT n / WAIT (6.2); ui_sq.c draws INT n only (placeholder) |
| H3 | "preset: `P01`, `P05?` pending, `P05>` loading" | SCREEN_SPEC 2.1 H3 | EN_PRESET,B_SAVE | Pnn / Pnn? / Pnn>; ui_sq.c draws Pnn and Pnn? |
| H4 | "`1 ACID M1` channel number, algorithm (4 letters), MIDI channel `M1`..`M16` or `MOFF`" | SCREEN_SPEC 2.1 H4 | EN_SELECT,EN_ALGO,EN_K2 (CHAN) | any channel x algorithm x MIDI 1..16/OFF |
| H5.1..H5.4, H5.CELL | "page map, drawn as graphics: viewed = outlined (T_TEXT), playing = filled (T_SEL fill, T_INK digit), exists = plain T_DIM outline, hidden-with-content = dotted T_DIM outline, never existed = 2x2 dot, partial last page = dashed bottom edge" | SCREEN_SPEC 2.1 H5 | KEY_B09_C#5 (PG-),KEY_B10_D#5 (PG+),EN_K1 (CHAN LENGTH) | viewed, playing, exists, hidden, never existed, partial |
| H6 | "`L32` pattern length" | SCREEN_SPEC 2.1 H6 | EN_K1 (CHAN LENGTH) | L1..L64 |
| CH1..CH4 .L1/.L2, CH.SELECT_FILL, CH.ACTIVITY_DOT | "4 channel cells (60x28 each): line 1 `1 ACID`, line 2 `MIDI 2`/`MIDI OFF`; muted draws the whole cell dim and line 2 reads `MUTE 2`; selected cell inverted (T_SEL fill, T_INK text); a 4x4 activity dot flashes on each note out" | SCREEN_SPEC 2.2 and 3 (Z1 row) | EN_SELECT, EN_ALGO, KEY_B01..KEY_B04 (M1..M4), EN_K2 on CHAN | normal, selected, muted, MIDI OFF (T_ACCENT), sounding (dot) |
| LANE.GLYPH | "16 step cells, bottom 14 px one glyph row: `'` edited, `!` accent, `~` slide, `x2` ratchet; cell n is the label of white key n+1" | SCREEN_SPEC 2.3 | KEY_W01_F3 .. KEY_W16_G5 | on/off, velocity, accent, slide, ratchet, edited, past LENGTH (dim), playhead |
| LANE.HDR | "lane header line: `PG 1/2  FOLLOW  hold=EDIT        >P2` (viewed page / pages, FOLLOW, the white-key hold, the playhead page)" | SCREEN_SPEC 2.3; SEQUENCER_UX 2.1 | KEY_B09_C#5,KEY_B10_D#5,EN_K4 (CHAN FOLLOW) | viewed page, FOLLOW on/off |
| LANE.PLAYPAGE | "`>P3` appears at the lane's right edge when the playhead is on another page" | SEQUENCER_UX 1.5 | B_PLAY | playing only |
| TAG.OCTDN/OCTUP .L1/.L2, TAG.LIVE_FILL | "tag column = the OCT- (upper half) and OCT+ (lower half) button labels of the page; PERF: `OCT-` over `h:peek`, `OCT+` over `h:peek`" | SCREEN_SPEC 2.4 BT | B_OCTDN (upper half), B_OCTUP (lower half) | live row filled (T_SEL); labels from button_modes.tsv (ROW 1 / ROW 2) |
| KNOB.R1/R2.K1..K4 .LABEL/.VALUE, KNOB.LIVE_FILL | "one knob cell: line 1 the label (AF_S, T_MID), line 2 the value (AF_S, T_THEME, T_ACCENT while the knob is hot); ROW 2 (S1..S4) drawn dim; the live row is filled (T_SEL)" | SCREEN_SPEC 2.4 BK1..BK4 / BS1..BS4 | EN_K1..EN_K4 (ROW 1 = K1..K4, ROW 2 = S1..S4 via B_OCTDN / B_OCTUP) | live row, dim row, hot (600 ms after a turn) |
| BTN.R1/R2.C1..C6 .TAP/.SUB, BTN.SEPARATOR, BTN.HOLD_MARK | "2 rows of 6 buttons, tap label y+1, sub-label y+13; sub-label = dice layer of the algorithm (ALGORITHM_SET 4.4) or the hold role with a hold mark; absent layer = dim with a dash; tap label that differs from the printed word is T_ACCENT" | SCREEN_SPEC 2.5 | B_FX B_SCL B_ENV B_LFO B_EDIT B_GLO / B_HOME B_SAVE B_ARP B_SEQ B_PLAY B_REC | normal, dim (layer absent), accent (RST/BACK/CANCEL) |
| KEY.B01..B11, KEY.CELL | "Z5: 11 cells of 20 px at pitch 21 in the physical 3-2-3-2-1 grouping; each cell shows the page's label for that black key (max 6 characters); inverted while the key is down; muted-channel cells dim" | SCREEN_SPEC 2.6 | KEY_B01_F#3 .. KEY_B11_F#5 | normal, muted (T_DIM), key-down inverted (spec; not drawn by ui_sq.c) |
| CUE | "Z6: one line, 34 characters nominal (binding limit 240 px), priority table SEQUENCER_UX 11.2" | SCREEN_SPEC 2.6; SEQUENCER_UX 11.2 | (read-only) | page cue; PERF stopped 'PLAY: start   K1-K4: morph'; PERF playing 'TAP STEP=ON/OFF  HOLD=EDIT NOTE'; ALGO_PICK list for 2 s |
| TOAST | "a toast is drawn in the Z6 rectangle, fill T_TINT, ink T_ACCENT, text left-aligned at x=4, 2000 ms; replaces the cue strip" | SCREEN_SPEC 2.7 | (any control that answers with a toast) | transient |
| LANE.CELL_BG | "16 step cells, 14 px ink + 1 px gap; cells past LENGTH are dim" | SCREEN_SPEC 2.3 | KEY_W01_F3..KEY_W16_G5 | live, past LENGTH |
| LANE.BAR | "top 32 px a velocity bar (lit = a step plays, height = velocity)" | SCREEN_SPEC 2.3 | KEY_W01_F3..KEY_W16_G5 | on, accent, off |
| LANE.EDIT_FRAME | "edited cell boxed" | SCREEN_SPEC 3 (NOTE_EDIT row) | KEY_W01_F3..KEY_W16_G5 (hold) | NOTE_EDIT only |
| LANE.PLAYHEAD | "the playhead is a T_ACCENT underline of the cell" | SCREEN_SPEC 2.3 | B_PLAY | playing |
| TOAST.FILL | "toast fill T_TINT, ink T_ACCENT" | SCREEN_SPEC 2.7 | (any control that answers) | transient 2000 ms |

Additional statements the spec makes about elements (verbatim):

- Whole screen, VISION rule 1 as SCREEN_SPEC opens: "at every moment the screen shows what the device is **doing** and the current role of **every** knob and button." (SCREEN_SPEC, intro)
- SEQUENCER_UX R-4: "Every knob is labelled on every page; a deliberately inert knob is drawn `-`." (SEQUENCER_UX 0, framework rules)
- SEQUENCER_UX R-8: "Every hold is named on the page where it starts: the `h:` sub-label on the PERF button picture ..., `hold=EDIT` in the lane header for the white keys, the black-key legend for SOLO / FILL / ROLL." (SEQUENCER_UX 0)
- SCREEN_SPEC 2.4 on BT: "tag column = the OCT- (upper half, y=108..134) and OCT+ (lower half, y=134..160) button labels of the page; see 7. PERF: `OCT-` over `h:peek`, `OCT+` over `h:peek`"; `button_modes.tsv` (PERF, B_OCTDN / B_OCTUP) and `control_map.tsv` give the PERF labels `ROW 1` / `ROW 2`; the render draws `ROW` over `1` and `ROW` over `2` (two lines, `uisq_wrap`).
- SCREEN_SPEC 2.2 (muted): "A **muted** channel draws the whole cell dim and line 2 reads `MUTE 2`, `MUTE 12` (49) or `MUTE -` (MIDI OFF) ... Three of the four channels are muted at power-on, so this is the default screen." The render at power-on (`sq_boot`): CH1 unmuted MIDI 1, CH2..CH4 muted, MIDI 2..4.
- SCREEN_SPEC 6.2 clock dot: "amber solid = the FM-1's own clock (internal, and it is sending clock out); cyan with a 2 x 2 centre hole = a host clock drives the tempo; cyan ring (4 x 4 hole) blinking at 2 Hz = armed, waiting for the host to start" and "never colour alone".

## 5. The other pages: what the spec says, and what `ui_sq.c` draws

`ui_sq.c` gives every page an entry in `uisq_pages[]`; every `input` function is `uisq_pg_stub` (does nothing). Drawing is one generic path: the zones are drawn from the page's tables (knob, button, key labels, cue from `page_states.tsv`) and the page-specific code is limited to what is listed in the last column. Knob values are drawn only on PERF (`lp == UI_PG_PERF`); on every other page a knob cell is its label only.

| Page | page_states.tsv `screen` / SCREEN_SPEC 3 row (abridged) | `ui_sq.c` draws | Real content or stub |
| --- | --- | --- | --- |
| PERF | header + channel band + 16-cell lane + 8-cell knob band (live row filled) + button picture + black-key legend + cue strip; cue `PLAY: start   K1-K4: morph` | all zones with real state (channels, lane via `sq_eval`, morph values, playhead, page map, toast, algo-pick strip); H2 INT only; H3 `Pnn` only | real content (input stub) |
| NOTE_EDIT | header + lane (edited step outlined) + band PITCH GATE VELO RATCH with AUTO/set state + 6 buttons + OCT cells + legend + cue `STEP n: KNOBS EDIT  HOME=done`; Z1 banner `CH1 NOTE EDIT STEP 21` | Z1 banner `CH1 NOTE EDIT STEP 21` (T_ACCENT on T_TINT), edited-step frame (T_ACCENT) in the lane, knob labels PITCH GATE VELO RATCH without values, status line 2 `* = set by you (EDIT: undo)`, button/legend/cue from tables; status line 1 (`AUTO = generated: ...`) is empty | partial: banner, frame, labels, one status line; no AUTO/value cells |
| CHAN | header + lane + band LENGTH MIDI SOURCE FOLLOW + page map (4 cells: used / viewed / playing); tag `-PAGE`/`+PAGE`; status `PAGES  1 OWN  2 P1 ...` and `D#4 = BACK` | channel cells and PERF lane unchanged, knob labels LENGTH MIDI SOURCE FOLLOW (no values), tag `-PAGE`/`+PAGE`, status line 2 `D#4 = BACK`; the four large page-map cells and status line 1 are not drawn | stub-level: labels and one status line |
| SET | header + lane + band BPM SWING GATE DIV + 2 status lines (TEMPO INT/EXT, OUT MASTER/FOLLOW, host-clock texts); tag `OCTAVE -/+` | knob labels BPM SWING GATE DIV (no values), tag `OCT-`/`OCT+`, both status lines empty strings, GLO reads `BACK`, cue `SET: GLO hold=HARM  HOME=back` | stub-level: labels only; tempo-source status lines not drawn |
| SCALE | HARMONY screen: Z2 progression strip (degree numerals AF_L, chord names) + legend `F3 G3 ... = 1..7`; band SCALE KEY VOICE MOVE; status DRONE / ROLE; buttons PROG BARS ROLE NEW | Z2 draws only the word `HARMONY` (T_TEXT) at x=2; knob labels SCALE KEY VOICE MOVE (no values); buttons PROG, BARS, ROLE, NEW from tables; legend keys all `-`; status lines empty | stub (Z2 placeholder word) |
| REC | banner REC + lane with write cursor + band CURSOR ACC SLIDE OCT + `KEYS = NOTES (all 27)`; cue `REC: PLAY KEYS TO WRITE  REC=done` | Z1 banner `CH1 REC writing to page 1` (T_REC on T_RAISE), H1 `R` in T_REC, knob labels CURSOR ACC SLIDE OCT (no values), status line 2 `KEYS = NOTES (all 27)`, legend `NOTE` x11 (cut to `N...`), REC button reads `DONE`; no write cursor `^` | partial: banner, labels, status line |
| RESET | the page beneath with every resettable cell tagged `RST` + banner `RESET`; K1..K4 = `RST`; tag `RST R1`/`RST R2`; buttons `RST AL`, `RST NT` ... | no banner, no `RST` tags on cells; knob labels `RST` x4 (no values), tag `RST`/`R1`, `RST`/`R2` wrapped on two lines, buttons `RST AL RST NT RST AC RST SH CANCEL -` etc. from tables, legend `-` except `COPY P` | stub-level: labels from tables |
| HELP | the page beneath dimmed + a 2-line help box over y=108..212 naming the touched control; cue `HELP: <control>  let go = back` | no dimming, no help box; every knob and most button labels read `?`, HOME cell `BACK` over `HELP`, PLAY cell `PLAY`, from tables; cue `HELP: let go = back` (override); legend cells from table | stub |
| MENU | Felucca menu drawn by `ui_menu.c` | `ui_sq_draw` returns 0 (draws nothing) | not this file's |
| FIRST_RUN | overlay of PERF: cue strip shows the guided step; step 1 = `D#4 K2: MIDI CHANNEL`, then `PRESS PLAY` ... (SEQUENCER_UX 11.3) | PERF layout; cue string fixed `D#4 K2: MIDI CHANNEL`; no step sequencing | stub (one static cue) |
| ADD_PAGE | prompt overlay: a ghost page cell + cue text `ADD PAGE n = COPY OF n-1? PG+ = yes` (or SHOW PAGE n / SHOW ALL OF PAGE n) | PERF layout; cue `ADD PAGE 2 = COPY OF 1? PG+ = yes` (the generated example text); no ghost page cell | stub |
| MOVE_HELD | overlay: held black-key cell inverted; FILL / ROLL cell shows the live amount; cue `ROLL 3x: let go to stop` | PERF layout; cue `ROLL 3x: let go to stop`; no inverted cell | stub |
| ALGO_PICK | overlay: the cue strip lists all six algorithms with the current one in brackets, `ACID EUCL SNAK STAG [ARP] PATT` (185 px of 240), 2 s after the last detent | PERF layout; the strip text is built from `uisq_short[]` when `uisq_pick_until` is non-zero (`[ACID] EUCL SNAK STAG ARP PATT` for CH1; 185 px). When `uisq_page` is set to ALGO_PICK without a detent the cue is the empty string | real for the strip (via `ui_sq_algo_pick_touch`); the page entry itself is a stub |
| RIG | RIG panel in the lane area: banner `RIG CHn`, four wires, one line each; band PARAM LAYER FORM MOVE; tag `RIDE -/+`; cue `RIG: TAP 1-4 = WHO DRIVES  HOLD = PICK` | PERF lane, knob labels PARAM LAYER FORM MOVE (no values), tag `RIDE-`/`RIDE+`, cue from table; no banner, no wire lines | stub |

Per-page text drawn by the generic path (page_<NAME>_algo0 scenario, `texts_all.tsv`): the Z4 buttons of NOTE_EDIT show `ALL NOTE ACCENT PROB UNDO SET / HOME SAVE SLIDE ON/OFF PLAY REC` (the page's own tap labels); SET `GLO` cell reads `BACK`; HELP draws `?` for 10 of the 12 button taps; HOME reads `BACK` over `HELP`, PLAY reads `PLAY`.

## 6. Counts (for the working-memory heuristic)

Scenario `PERF_default` (power-on, stopped, ROW 1 live, ACID selected; CH1 unmuted, CH2..CH4 muted) and `PERF_playing` (transport running, CH1 sounding).

| Zone | Text draws, default | distinct strings, default | text draws, playing | Non-text items drawn (default) | Visible interactive labels |
| --- | --- | --- | --- | --- | --- |
| Z0 | 6 | 6 | 7 | clock dot, 4 page-map cells (frames/dots) | H3 preset (EN_PRESET), H4 (EN_SELECT/EN_ALGO) |
| Z1 | 8 | 8 | 8 | select fill, activity dot (when sounding) | 4 channel cells |
| Z2 | 2 | 2 | 7 | 16 cell backgrounds, up to 16 bars, playhead (playing) | 16 step cells (white keys), header |
| Z3 | 20 | 18 | 20 | live-row fills: 5 (tag half + 4 knob cells) | 8 knob cells + 2 tag labels |
| Z4 | 21 | 18 | 21 | 10 separators (5 per row), 4 hold marks | 12 buttons |
| Z5 | 11 | 9 | 11 | 11 wells | 11 black keys |
| Z6 | 1 | 1 | 1 | none (toast: 1 fill) | 0 |
| **Total** | **69** | **62** | **75** | | |

- Distinct text items on the PERF screen: **69** text draws (62 distinct strings) in the default state; **75** while playing (adds `>`, `>P1` lane-header marker and lane glyphs of the playing pattern).
- Distinct text colours (fg RGB565): 4 in the default state (0841, 5ACB, 9492, CE59), 5 over all PERF states (0841, 5ACB, 9492, CE59, FFDF). Distinct backgrounds under text: 3 default, 4 over all states.
- Distinct colours of every pixel-drawing call (text fg/bg, rect, frame) in `PERF_default`: **10** (0841, 2945, 39C7, 5ACB, 9492, BDD7, CE59, EF5D, FD80, FFDF); one of them is the fixed amber clock dot, the rest are theme roles that on GREY collapse to the values listed in section 1.
- Distinct theme roles drawn on PERF: T_BG, T_TEXT, T_MID, T_DIM, T_INK, T_SEL, T_RAISE, T_LANE, T_LINE, T_GRID, T_THEME, T_ACCENT (12), plus T_TINT for the toast, plus T_REC on the REC page; plus the fixed amber dot.
- Distinct font faces / sizes used on PERF: **1** (AF_S, 15 px line box, 9 px caps). Text weights: 1 (400).
- Items per zone as a reader meets them: Z0 6 text + dot + 4-cell map (header slots H1..H6); Z1 4 cells x 2 lines; Z2 1 header line + 16 cells; Z3 8 knob cells (label + value) + 2 tag labels; Z4 12 buttons (12 taps + 9 sub-labels at ACID); Z5 11 legend labels; Z6 1 line.
- Controls with an on-screen role label at once: 4 channel cells + 16 step cells + 2 tag + 8 knob + 12 buttons + 11 black keys = **53** labelled controls on one screen (the control map has 49 controls: 16 white keys, 11 black keys, 14 buttons, 7 encoders incl. SELECT/ALGO/PRESET, MASTER).
- Distinct text line tops (screen y) on PERF default: 16 (3, 4, 22, 34, 49, 95, 109, 121, 135, 147, 161, 173, 187, 199, 212, 221).

## 7. Observed differences between the spec text and the current render (facts)

| # | Spec says | Render does |
| --- | --- | --- |
| 1 | SCREEN_SPEC 2.4: PERF tag column `OCT-` over `h:peek`, `OCT+` over `h:peek` | `button_modes.tsv` / `control_map.tsv` label B_OCTDN / B_OCTUP `ROW 1` / `ROW 2`; the tag draws `ROW` over `1` and `ROW` over `2` (two lines each, 27 px); no `h:peek` sub-label |
| 2 | SCREEN_SPEC 5: DENS / HITS at 0 shows `KEYS`; SEQUENCER_UX 2.1 mock: `DIR FWD` | knob values are the raw morph integer (`DIR 0`, `DENS 0`); `uisq_draw_z3` does no value formatting |
| 3 | SCREEN_SPEC 2.1 H2: INT n / EXT -- / EXT n / WAIT; dot colours amber / cyan / cyan ring | `INT <bpm>` and the amber solid dot only (`uisq_h2_text`, `uisq_dot`), marked a placeholder in the code |
| 4 | SCREEN_SPEC 2.3: lane header `PG 1/2  FOLLOW  hold=EDIT        >P2` | FOLLOW off draws the word `hold`: `PG 1/2  hold  hold=EDIT`; `>Pn` is right-aligned at x=236 only while playing |
| 5 | SEQUENCER_UX 11.2 row 14 alternates every 4 s; row 15 `ROW 2: OCT- = back to row 1` while ROW 2 is live and playing | `uisq_cue_text` draws row 14's first string `TAP STEP=ON/OFF  HOLD=EDIT NOTE` (203 px) whenever PERF is playing; no alternation, no row 15 |
| 6 | SCREEN_SPEC 2.2: MIDI OFF drawn T_ACCENT for 3 s after displacement, then T_TEXT | line 2 `MIDI OFF` is T_ACCENT for as long as the channel is unmuted, unselected and MIDI OFF (no timer) |
| 7 | SCREEN_SPEC 2.6: legend cell inverted while the key is down | no key-down state is read; legend cells are T_RAISE wells with T_TEXT / T_DIM labels |
| 8 | SCREEN_SPEC 2.6 / F3: labels over 20 px are cut with an ellipsis, "the full word is in HELP" | 6 PERF legend labels (CHN, SOL, ROL, PG-, PG+, RST) draw as one letter plus the ellipsis; HELP page is a stub (`?`) |
| 9 | SCREEN_SPEC 2.4: the live row is filled (T_SEL); "line 2 the value (T_THEME, T_ACCENT while the knob is hot)" | the live row's value is always T_INK on T_SEL; the hot colour T_ACCENT is applied to the value of the **other** (non-live) row's cell in that column (`hot` is indexed by column only) |
| 10 | SCREEN_SPEC 7.1 / 3: NOTE_EDIT, CHAN, SET, SCALE, REC, RESET, HELP bodies | see section 5: values absent on all non-PERF pages; HELP box, RESET tags, CHAN page-map cells, SCALE strip, RIG wires, REC cursor not drawn |
| 11 | SCREEN_SPEC 2.5: cell widths data per page, `h:` replaced by a hold mark | hold mark drawn as the 5x7 down-triangle, widths per page and algorithm computed from `text_w` (test asserts sum = 240 on every page x algorithm) |

## 8. Reproducing

```sh
sh docs/design/ui_review/measure.sh build/ui_review     # compiles the instrumented harness, writes build/ui_review/*.tsv
python3 docs/design/ui_review/analyze.py                  # elements.tsv, truncations.tsv, text_pairs.tsv, copies palettes/fonts here
python3 docs/design/ui_review/contrast.py                 # the contrast tables of section 1
python3 docs/design/ui_review/build_evidence.py           # regenerates this file
```
