<!-- SPDX-License-Identifier: GPL-3.0-only -->
# UI PROPOSAL v3-GLANCE: FM-1 sequencer screen

Source: `proposal_v3.py` (the whole proposal; zones, scenes and fixtures), `report_v3.py` (the tables below: `proposal_v3.py --report`),
`before_v3.py` + `make_sheets_v3.py` (the before/after sheets against the shipping render, `mock/current_perf.py`). Palette GREY, the default.
Per scene: `proposal_v3_SCENE.png` (1x), `_4x.png`, `_grid.png` (zones and every text ink box outlined) and `SCENE_before_after.png`
(CURRENT left, PROPOSAL right, both 2x). Scenes: glance_idle, glance_playing, row2, mutes, stress, stress_row2, toast_algo,
toast_preset_pending, toast_dice_notes, toast_undo, map, plus two monochrome-fallback renders (glance_playing_mono, stress_mono).
Not hardware-tested: the mock is pixel-exact for fonts and palette only.

## What the screen is

The main screen shows live state only, and shows it big. Nothing on it is AF_S. The four facts the owner picked, and where they are:

| Owner fact | Where | How it is drawn |
| --- | --- | --- |
| Four knob values | Z3, four 60 px cells | label AF_M (9.5 arcmin), VALUE AF_L (18.0 arcmin), a meter shape under it (fill bar; pips for enums) |
| Where the pattern is | Z2, 16 cells of 15 px (the 16 white keys) | bars; the playing step is a full-height 14x70 column (amber `T_PLAYHEAD`, white in the monochrome fallback) with the bar inverted inside it |
| Which channels are muted (all four) | Z1, four 60 px cells | muted = hollow cell (2 px frame, no fill) + the channel number struck through + a solid `M` chip; MIDI OFF = hollow `OFF` chip |
| Selected channel and its algorithm | Z1 | selected = inverted block; number AF_L (18.0), algorithm AF_M (9.5) |

Zones (they tile 240 exactly; `assert` in the file): Z0 head y0 h24, Z1 channels y24 h48, Z2 lane y72 h72, Z3 knobs y144 h76, Z4 cue y220 h16, Z5 bezel
margin y236 h4. Blits: A = Z0+Z1 240x72, B = Z2 240x72, C = Z3+Z4+Z5 240x96 (all under the 240x124 strip; the shipping screen's biggest is 240x108).

- **Z0.** Transport is a drawn 16x16 shape: play triangle (amber), stop square (grey), REC disc (red). BPM in AF_M (`EXT 126` when external). Preset in AF_M
  (`P04?` pending). Page boxes are shapes: viewed = solid, other existing pages = double outline, the playing page carries a 4x4 marker.
- **Z1.** Cell 58x44 inside a 60x48 slot. Number AF_L, algorithm AF_M. Activity: a solid 14x14 square (amber; ink on the selected cell) while the channel just sounded.
- **Z2.** Mark row A (y3..8): accent = filled triangle 11x6. Mark row B (y11..16): slide = a 15x6 tie bar (a run of slides chains into one line). Ratchet = the
  bar itself becomes a comb of 2, 3 or 4 bars (x2: 5 px wide, x3: 3, x4+: 2). Accent also turns the bar cyan (`T_ACCSTEP`), but the triangle carries it alone in
  monochrome. Rests = a low flat dash. Beat pips under steps 1, 5, 9, 13. A muted selected channel draws its bars hollow; the playhead still runs.
- **Z3.** Cell 58x66: label AF_M at cap y+6, value AF_L at cap y+21, meter at y+50 (46x10). Under the cells a 10 px **ROW bar**: two halves, left = OCT- = ROW 1,
  right = OCT+ = ROW 2; the latched half is solid with `I` or `II` drawn in ink. Only the latched row's cells are drawn. A just-touched knob gets a 2 px frame.
- **Z4.** One AF_M phrase, `HOLD HOME = MAP`, only while stopped. Verdict: it earns its place only as the MAP's discoverability; it is blank while playing. It is the first thing to cut.
- **Toasts** replace Z3+Z4 (92 px) with an inverted block: AF_L title, AF_M detail, one shape. ALGO: `EUCLID` + `CH 1 ALGORITHM` + six pips (chosen one tall).
  PRESETS: `P04` + a drawn `?` (AF_L has no `?`) + `SAVE = SAVE HERE` + the 1.5 s settle countdown as a bar. Dice: drawn die + layer name `NOTES` + `CH 1  NEW PITCHES`.
  UNDO: drawn arrow + `UNDO` + what was undone. SELECT needs no toast: the inverted cell is the answer.
- **MAP** (HOME held): a full-screen AF_S diagram in panel order, section below.

## (a) Every text item on the glance screen

Cap heights from the real fonts (AF_S 9 px, AF_M 11, AF_L 21) at 8 px/mm, arcmin at 50 cm. "need" = 4.5:1, or 3:1 for AF_L (large). Contrast is the worst
instance in any scene, measured with `mock.contrast_ratio` on the real RGB565 values.

| role | font | strings drawn (all scenes) | cap px | cap mm | arcmin @50 cm | worst contrast | need | instances in busiest scene |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| bpm | AF_M | `120, EXT 126` | 11 | 1.38 | 9.5 | 12.33:1 | 4.5 ok | 1 |
| preset | AF_M | `P01, P16, P04?` | 11 | 1.38 | 9.5 | 12.33:1 | 4.5 ok | 1 |
| chan number | AF_L | `1, 2, 3, 4` | 21 | 2.62 | 18.0 | 6.42:1 | 3.0 ok | 4 |
| algo name | AF_M | `ARP, ACID, PATT, SNAKE, STAGE, EUCLID` | 11 | 1.38 | 9.5 | 6.42:1 | 4.5 ok | 4 |
| OFF chip | AF_M | `OFF` | 11 | 1.38 | 9.5 | 6.42:1 | 4.5 ok | 1 |
| MUTE chip | AF_M | `M` | 11 | 1.38 | 9.5 | 6.42:1 | 4.5 ok | 2 |
| knob label | AF_M | `DIR, DYN, LEN, DENS, JUMP, PATH, RDIV ...` | 11 | 1.38 | 9.5 | 10.03:1 | 4.5 ok | 4 |
| knob value | AF_L | `0, 3, 4, 5, 6, 7, 8 ...` | 21 | 2.62 | 18.0 | 10.03:1 | 3.0 ok | 4 |
| cue | AF_M | `HOLD HOME = MAP` | 11 | 1.38 | 9.5 | 12.33:1 | 4.5 ok | 1 |
| knob value (M) | AF_M | `SNK3` | 11 | 1.38 | 9.5 | 10.03:1 | 4.5 ok | 1 |
| toast title | AF_L | `P04, UNDO, NOTES, EUCLID` | 21 | 2.62 | 18.0 | 10.42:1 | 3.0 ok | 1 |
| toast detail | AF_M | `DICE NOTES, CH 1 ALGORITHM, SAVE = SAVE HERE, CH 1  NEW PITCHES` | 11 | 1.38 | 9.5 | 10.42:1 | 4.5 ok | 1 |

roles (distinct text kinds) on the glance screen: 12
- glance_idle: 21 strings drawn (L x8, M x13)
- glance_playing: 20 strings drawn (L x8, M x12)
- row2: 21 strings drawn (L x8, M x13)
- mutes: 20 strings drawn (L x7, M x13)
- stress: 20 strings drawn (L x8, M x12)
- toast_algo: 14 strings drawn (L x5, M x9)
- toast_preset_pending: 14 strings drawn (L x5, M x9)
- toast_dice_notes: 14 strings drawn (L x5, M x9)
- toast_undo: 14 strings drawn (L x5, M x9)

**Item count, honestly.** Distinct text roles on screen at once while playing: **8** (BPM, preset, channel number, algorithm name, knob label, knob value, and the two
conditional chip words `OFF` and `M`); 9 when stopped (the cue). String instances drawn: **20 playing, 21 idle** (8 AF_L + 12-13 AF_M), 14 under a toast. AF_S strings: **0**.
The shipping screen draws about 55 (assessment A) to 75 (owner) AF_S strings. I could not reach a literal 8 strings: the four facts alone need 4 knob values + 4 knob names + 4 channel numbers + 1 algorithm = 13.
The extra 7 are: three more algorithm names (cells of the unselected channels), BPM, preset, and the chips. Dropping the three extra algorithm names gives 17.

## (b) The MAP (HOME held)

AF_S only; temporary and dense by design. Order is the panel's: row 1 = encoder block 2x2 (MASTER SELECT / PRESETS ALGORITHM) on the left and K1..K4 on the right
(ROW 1 name, ROW 2 name, the live row bright), row 2 = OCT- OCT+ under the encoders (the latched ROW is the inverted cell), then the 2x6 buttons (printed word,
CURRENT tap role, hold role in an outline box, from `button_modes.tsv`), then the 11 black keys (picture + two staggered label tiers with leader lines: AF_S labels
of 20-25 px do not fit a 20 px pitch) and the 16 white keys as steps 1..16.

| role | font | strings drawn (all scenes) | cap px | cap mm | arcmin @50 cm | worst contrast | need | instances in busiest scene |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| map title | AF_S | `TOUCH = HELP   LET GO = MENU` | 9 | 1.12 | 7.7 | 12.33:1 | 4.5 ok | 1 |
| map encoder name | AF_S | `MASTER, SELECT, PRESETS, ALGORITHM` | 9 | 1.12 | 7.7 | 5.23:1 | 4.5 ok | 4 |
| map encoder role | AF_S | `VOLUME, CHANNEL, OF CH 1, P01-P16` | 9 | 1.12 | 7.7 | 10.03:1 | 4.5 ok | 4 |
| map OCT name | AF_S | `OCT+, OCT-` | 9 | 1.12 | 7.7 | 5.23:1 | 4.5 ok | 2 |
| map OCT role | AF_S | `ROW 1, ROW 2` | 9 | 1.12 | 7.7 | 10.03:1 | 4.5 ok | 2 |
| map knob id | AF_S | `K1, K2, K3, K4` | 9 | 1.12 | 7.7 | 6.42:1 | 4.5 ok | 4 |
| map knob row1 | AF_S | `DENS, JUMP, ACCNT, SLIDE` | 9 | 1.12 | 7.7 | 12.33:1 | 4.5 ok | 4 |
| map knob row2 | AF_S | `DIR, PEDAL, PUNCH, OFFSET` | 9 | 1.12 | 7.7 | 6.42:1 | 4.5 ok | 4 |
| map legend | AF_S | `= HOLD` | 9 | 1.12 | 7.7 | 12.33:1 | 4.5 ok | 1 |
| map button printed | AF_S | `FX, ARP, ENV, GLO, LFO, REC, SCL ...` | 9 | 1.12 | 7.7 | 5.23:1 | 4.5 ok | 12 |
| map button tap | AF_S | `ALL, REC, SET, HITS, HOME, PLAY, SAVE ...` | 9 | 1.12 | 7.7 | 10.03:1 | 4.5 ok | 12 |
| map button hold | AF_S | `BACK, HARM, MENU, RESET` | 9 | 1.12 | 7.7 | 10.03:1 | 4.5 ok | 4 |
| map black key | AF_S | `M1, M2, M3, M4, CHN, PG+, PG- ...` | 9 | 1.12 | 7.7 | 12.33:1 | 4.5 ok | 8 |
| map black key hold | AF_S | `FIL, ROL, SOL` | 9 | 1.12 | 7.7 | 12.33:1 | 4.5 ok | 3 |
| map caption | AF_S | `STEPS 1-16: TAP ON/OFF  HOLD EDIT` | 9 | 1.12 | 7.7 | 12.33:1 | 4.5 ok | 1 |
| map white key | AF_S | `1, 2, 3, 4, 5, 6, 7 ...` | 9 | 1.12 | 7.7 | 7.19:1 | 4.5 ok | 16 |

MAP: 16 roles, **82 strings**, all AF_S (7.7 arcmin: it is a page you bring your face to). Tool warnings: none, no cut and no ellipsised text. Tight spots, not cut: `ALGORITHM` fills
its 65 px cell exactly (ink 65); `ACCNT` is 40 px in a 38 px button cell and touches `NOTES` and `SHAPE`; steps 10..16 are 13 px ink in 14 px cells.
Not on the MAP (it does not fit): hold roles of M1..M4 (`PRV1..4` preview) and CHN (`RIG`), the OCT hold-peek, `GLO+SELECT` tempo, `GLO+PLAY` restart, both-OCT update mode.
Renamed for the MAP: SAVE hold `UNDO L` becomes `BACK` (undo the last preset load); HOME hold `HLP/MN` becomes `MENU` (help is the touch-while-held in the title).

## (c) Enumerated knobs: names

Rule: a value is drawn in AF_L when its ink fits 56 px of the 60 px cell, and **one font per enum** (a cell must not change size while you turn it): if one name of the enum does not fit,
all of its names go to AF_M. Names are at most 4 letters (AF_L ones 3). Source of ranges and the firmware strings: `knob_map.tsv`, algorithm SPEC.md files.
`DIR` appears as ROW 2 K4 of ACID and SNAKE, ROW 1 K3 of STAGE, ROW 2 K4 of ARP; `CURVE` is EUCLID ROW 2 K4; `SHAPE`/`RATE` are ARP ROW 1 K2/K4; `PATH` is SNAKE ROW 1 K4.

| enum | value | firmware string | proposed | font (60 px cell) | ink px |
| --- | --- | --- | --- | --- | --- |
| DIR3 | 0 | FWD | `UP` | AF_L | 35 |
| DIR3 | 1 | BWD | `DN` | AF_L | 37 |
| DIR3 | 2 | PEND | `P-P` | AF_L | 44 |
| ARPDIR | 0 | UP | `UP` | AF_L | 35 |
| ARPDIR | 1 | DOWN | `DN` | AF_L | 37 |
| ARPDIR | 2 | UPDN | `U-D` | AF_L | 49 |
| ARPDIR | 3 | RAND | `RND` | AF_L | 54 |
| ARPDIR | 4 | ASPL | `ORD` | AF_L | 55 |
| ARPDIR | 5 | CHORD | `CRD` | AF_L | 55 |
| CURVE | 0 | AUTO | `AUT` | AF_L | 56 |
| CURVE | 1 | SINE | `SIN` | AF_L | 42 |
| CURVE | 2 | TRI | `TRI` | AF_L | 40 |
| CURVE | 3 | SAW+ | `RP+` | AF_L | 48 |
| CURVE | 4 | SAW- | `RP-` | AF_L | 43 |
| CURVE | 5 | ARCH | `ARC` | AF_L | 55 |
| CURVE | 6 | INV | `INV` | AF_L | 44 |
| SHAPE | 0 | LINE | `LINE` | AF_M | 30 |
| SHAPE | 1 | SKIP | `SKIP` | AF_M | 31 |
| SHAPE | 2 | THUMB | `THMB` | AF_M | 42 |
| SHAPE | 3 | OCTV | `OCTV` | AF_M | 40 |
| SHAPE | 4 | REPT | `REPT` | AF_M | 36 |
| SHAPE | 5 | WAVE | `WAVE` | AF_M | 40 |
| RATE | 0 | 1/4 | `1/4` | AF_L | 42 |
| RATE | 1 | 1/2 | `1/2` | AF_L | 42 |
| RATE | 2 | 1X | `1X` | AF_L | 33 |
| RATE | 3 | 2X | `2X` | AF_L | 34 |
| RATE | 4 | 3T | `3T` | AF_L | 34 |
| RATE | 5 | 4X | `4X` | AF_L | 35 |
| PATH | 0 | path 1 | `ROW1` | AF_M | 40 |
| PATH | 1 | path 2 | `ROW2` | AF_M | 43 |
| PATH | 2 | path 3 | `COL1` | AF_M | 35 |
| PATH | 3 | path 4 | `COL2` | AF_M | 38 |
| PATH | 4 | path 5 | `SNK1` | AF_M | 35 |
| PATH | 5 | path 6 | `SNK2` | AF_M | 38 |
| PATH | 6 | path 7 | `SNK3` | AF_M | 38 |
| PATH | 7 | path 8 | `SNK4` | AF_M | 38 |
| PATH | 8 | path 9 | `DIA1` | AF_M | 29 |
| PATH | 9 | path 10 | `DIA2` | AF_M | 32 |
| PATH | 10 | path 11 | `SPI1` | AF_M | 27 |
| PATH | 11 | path 12 | `SPI2` | AF_M | 30 |
| PATH | 12 | path 13 | `SPI3` | AF_M | 30 |
| PATH | 13 | path 14 | `SPI4` | AF_M | 30 |

- `PATH` names are **mine**: snake.md gives only the 14 permutations (6.3), no names. Families: ROW = raster rows (1: top-down, 2: bottom-up), COL = raster columns, SNK1..4 = the four
  contiguous serpentines (5,6 by columns, 7,8 by rows; the default path 7 reads `SNK3`), DIA = the two diagonals (9, 10), SPI1..4 = the four spirals (11..14). Checked against the SHAPE table
  for contiguity only; the direction of each variant is not verified.
- `DENS` / `HITS` at 0 read `KEY` (firmware `KEYS`, 4 letters = 70 px in AF_L, `KEY` 52 px) with an empty meter.
- Numbers that are not enums stay numbers (`DENS 12`, `TOTAL 64`, `OFFSET 63`, the widest are 3 digits = 50 px). The meter shows their position in the knob's range (lo..hi from `knob_map.tsv`).
- ARP K3 `SPAN` (firmware `1 OCT`..`4 OCT`, 82 px in AF_L) is relabelled `OCTS` with the value `1`..`4`.
- PATTERNS `PICK` (1..120): the label line shows the category (`ACID DUB FUNK HOUSE MEL TECH`: `MELODY` and `TECHNO` are 59-60 px in AF_M), the value is the number in the category (`HOUSE 3`), six pips for the category.
- STAGE `TOTAL`: the firmware cell also reads `48 (32)` (heard steps); the glance cell shows `48` only (the bracket does not fit AF_L).

## Palette tokens (GREY is mono: `T_ACCENT = T_REC = T_TEXT`)

At most 3 new, literal RGB565 in the mock; each has a monochrome fallback that is still readable (renders `glance_playing_mono`, `stress_mono`):

| Token | RGB565 | Used for | Fallback |
| --- | --- | --- | --- |
| `T_PLAYHEAD` | 0xFD80 (255,176,0) | playhead column, playing triangle, just-sounded square, playing page marker | `T_TEXT` (white column with ink bars) |
| `T_ACCSTEP` | 0x565F (80,200,255) | accented bars | `T_TEXT` (the triangle mark still says accent) |
| `T_RECRED` | 0xFA49 (the REC colour of the 8 colour palettes) | REC disc | `T_TEXT` (disc vs triangle vs square) |

Contrast of what is drawn (GREY): TEXT on BG: 12.33:1; TEXT on RAISE: 7.19:1; TEXT on SURF: 10.03:1; INK on SEL: 10.42:1; MID on BG: 6.42:1; MID on RAISE: 3.75:1; MID on LANE: 3.75:1; TEXT on LANE: 7.19:1; MID on SURF: 5.23:1; INK on MID: 6.42:1; T_PLAYHEAD 0xFD80: on BG 11.08, on LANE 6.46, on RAISE 6.46; BG ink on it 11.08; T_ACCSTEP 0x565F: on BG 10.73, on LANE 6.25, on RAISE 6.25; BG ink on it 10.73; T_RECRED 0xFA49: on BG 5.99, on LANE 3.49, on RAISE 3.49; BG ink on it 5.99; bar (TEXT) on LANE 7.19; playhead column T_PLAYHEAD vs LANE column 6.46; mono playhead TEXT vs LANE 7.19. MID on RAISE / LANE (3.75) is used for shapes only (hollow bars, strike, frames; the 3:1 shape floor), never for text

Colour is always redundant: mute = hollow + strike + chip; selected = inverted; accent = triangle; slide = tie bar; ratchet = comb; REC = disc; playhead = full-height column.

## (d) Firmware, tokens, fonts, tables: cost

- **Fonts.** No new size. AF_L has only `# + - . / 0-9 A-Y`: no `?`, `Z`, `:`, space beyond 32. Every AF_L string here is digits and capitals from that set. The drawn `?` is a 5x7 bitmap at x3 (15x21) in the toast; the alternative is one new glyph in AF_L
  (a few hundred bytes, not measured). If the owner wants AF_M names larger than 9.5 arcmin the only route is a new font size between AF_M and AF_L (flash cost not measured).
- **`ui_sq.c`.** All six zone draws are rewritten (`uisq_draw_z0..z6`): new zone table (24/48/72/76/16/4), blits A/B/C as above, new primitives (right triangle, up triangle, disc, comb bar, strike, outlined box)
  as 1-px-row spans (no bitmap font), a value formatter (range, enum table, `KEY`, `PICK`), a row-latch bar, a drawn `?`, and a **toast model** with kind + title + detail + shape + countdown (today a toast is a string in the Z6 strip).
  A knob turn should clear a toast at once (not specified in the mock).
- **State the screen needs that it may not have yet:** knob `lo/hi` per (algorithm, knob), the enum id, activity per channel (`snd0`, exists), REC armed, tempo source (exist), settle fraction of a pending preset load.
- **Generated tables.** `knob_map.tsv` gains `lo`, `hi`, `enum`; a new enum-name table (6 enums, 14 `PATH` names, 6 `PICK` categories, about 150 bytes); `tools/gen_ui_tables.py`; `button_modes.tsv` hold labels (`HLP/MN` to `MENU`, `UNDO L` to `BACK`); `widths.tsv`
  rows for every new string (3 or 4 letter enum names in AF_L / AF_M, `EUCLID` .. `PATT`, `HOLD HOME = MAP`, toast details, the MAP labels). `SCREEN_SPEC.md` 2.1 to 2.7 and 5 are replaced; `tests/ui_sq_test.c` goldens change.
- **The MAP** is a new full-screen page for the HOME-held state (the shipping one only changes the cue strip), drawn from the existing tables (AF_S, 82 strings).
- **Behaviour / spec decisions hidden in this mock:** the black-key legend and button grid leave the main screen; pattern length `Lnn`, FOLLOW/pinned, the edited-step dot, the clock dot, the MIDI channel number (only OFF is shown), tempo-source words other than EXT are dropped from the glance screen;
  the cue is blank while playing; ratchet is capped at 4 combs; marks are drawn only for steps that are ON; a muted channel keeps its algorithm name and number but loses its activity square.
- **Memory.** No new buffers; the largest blit is 240x96 (23040 px, under CV_MAX 29760).

## (e) Tool warnings

Proposal scenes: **none** (no cut, no ellipsised text, no strip over 240x124), 13 scenes. Warnings in the **before** renders (the shipping screen, on the same states): `HLP/MN` is cut by the canvas, and the
black-key legend `CHN SOL ROL PG- PG+ RST` is ellipsised to `x…` in 18 px: the shipping screen still has those defects.
What the tool does **not** check: the 8 other palettes (T_ACCENT differs there: in AMBER the playhead amber would be the theme colour itself), viewing angle, whether a 14 px column reads as a playhead in daylight, and the contrast of shapes
(I computed those by hand with the tool's `contrast`, table above). The before screens for the toast scenes draw the shipping Z6 strip from SCREEN_SPEC 2.7, not from firmware; the MAP "before" is the shipping screen with its HOME-held cue strip.

## Self-critique against the four owner facts

1. **Knob values.** Good: all numbers and 22 of the 42 enum names are AF_L (18 arcmin), with a meter. Weak: the other 20 (SNAKE `PATH` 14, ARP `SHAPE` 6) are AF_M (9.5 arcmin) with names I invented; names (`DENS`, `OFFSET`) are AF_M, below the 10 arcmin brief-glance mark, and `OFFSET` / `LEGATO` fill 54-55 of 58 px.
2. **Where the pattern is.** Good for the viewed page. **Weak when FOLLOW is off or the pattern is longer than 16**: if the playhead is on another page the lane shows no playhead and the only cue is a 4x4 marker in a 14 px header box. An edge arrow on the lane should be added.
3. **Mutes.** All four read at 1 m: hollow + strike + `M`. But the chip is the letter `M` (the M1..M4 key), not the word MUTE; a muted AND MIDI OFF channel shows `OFF` and no `M`.
4. **Selected channel and algorithm.** Number AF_L, algorithm name AF_M (9.5 arcmin): the number is big, the algorithm is not.
Other: amber is used for four different live things (playhead, playing triangle, activity, playing page): one "live" colour, but also one more thing for the eye to confuse in the monochrome fallback, where it is all white.

## Least sure about

1. A toast that replaces the whole knob row for ~2 s hides the values you are touching; if a knob turn does not cancel it, it fights the reason the screen exists.
2. AF_M at 9.5 arcmin is under the 10 arcmin glance threshold and carries the algorithm name, knob labels, BPM and the cue. Nothing between AF_M and AF_L exists.
3. The enum renames (`FWD BWD PEND` to `UP DN P-P`, `ASPL` to `ORD`, `SAW+` to `RP+`, `PATH` names) are readable shorthand I made up; `ORD` and `RP+` are the weakest.
4. The row bar (two halves with `I` / `II` in 6 px) is a shape I did not test with a user; the knob labels changing is the real cue.
5. Selection reads by tone only (inverted light cell against dark grey ones); in a bright room the selected cell versus a muted hollow cell is the one comparison I would test first on the panel.
6. The 8-slot count: I could not stay under 13 strings (see above); the 20 strings are mostly the same items repeated four times.
7. The MAP omits the PRV / RIG hold roles of the M keys and CHN, and the OCT hold-peek.

## Owner questions

1. Keep the algorithm name on all four cells (20 strings), or only on the selected cell (17)?
2. May a knob turn cancel a toast at once? May toasts cover the knob row?
3. Are the proposed enum names acceptable, especially the invented `PATH` families? Or keep the firmware strings in AF_M for enums that do not fit?
4. Accept three new palette tokens (amber, cyan, red) with the white fallback, and per-palette values for the other 8 palettes (not designed)?
5. Is `HOLD HOME = MAP` while stopped wanted, or drop the cue line entirely?
6. Is `OFF` (no MIDI channel number) enough on the glance screen, or should the selected cell show its MIDI channel (one more AF_M string)?
7. Is a new font size between AF_M and AF_L worth its flash for algorithm names and knob labels?
