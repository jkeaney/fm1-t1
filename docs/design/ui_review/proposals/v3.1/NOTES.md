<!-- SPDX-License-Identifier: GPL-3.0-only -->
# UI PROPOSAL v3.1: the glance screen, refined

Source: `proposal_v3_1.py` (zones, scenes, fixtures; starts from `../v3-glance/proposal_v3.py`), `report_v3_1.py` (every table below), `render_all.py` (all PNGs and the
before/after sheets). Palette GREY unless a scene says otherwise. Per scene `proposal_v3_1_SCENE.png` (1x), `_4x.png`, `_grid.png` (zones and every text ink box outlined),
and `SCENE_v3_vs_v3_1.png` (v3-glance left, v3.1 right, both 2x). Not hardware-tested: the mock is pixel-exact for fonts and palettes only; it says nothing about viewing angle or glare.
The tool printed no warning (no cut text, no ellipsis, no strip over 240x124) for any of the 34 scenes.

**Scenes.** Asked for: `glance_idle glance_playing row2 mutes stress stress_row2 toast_algo toast_preset_pending toast_dice_notes toast_undo map map_menu_countdown`,
plus the stress render on `stress_pal_paper`, `stress_pal_amber`, `stress_pal_ice` (and, because it was cheap, green, violet, rose, night, mono). Extras that carry an argument:
`map_touch` (header showing a touched control's help), `snake_path`, `glance_playing_mono`, `toast_*_header`, `toast_*_lane`, `toast_algo_full` (the placements compared in change 7),
`stress_selL` (change 9), `gallery_path`, `gallery_arp`, `gallery_curve` (all pictured enumerations, change 5).

**The stress scene changed.** It is now what the brief asked: ARP selected AND muted AND MIDI OFF (channel 4), EXT 126, P16, L64 on page 4/4, REC, with the enumerated knobs SHAPE (OCTV) and
RATE (2X) in ROW 1 and DIR (CHORD) in ROW 2; channel 3 is OFF alone. The v3 stress (STAGE selected) is not the same state, so its left-hand side in the sheets differs in content as well as look.

## Verdict table (every required change, judged)

| # | Change | Verdict |
| --- | --- | --- |
| 1 | HOME gesture, map + MENU countdown | Done; the MAP got denser (see 1) |
| 2 | ROW 1 / ROW 2 labels | Done; better, costs 8 px of knob-cell height |
| 3 | MUTE and OFF both visible | Done; but OFF is now a pictogram, not a word: partly worse |
| 4 | LANE KEY on the MAP | Done; one mark (edited diamond) is new and small |
| 5 | SNAKE PATH as a picture; other pictures | Done and clearly better than invented names; pictures for 36 enum values |
| 6 | Square corners | Done (v3 never called a rounded primitive; see 6) |
| 7 | Toast behaviour | Spec written; recommendation changed to a 32 px band at the bottom |
| 8 | Palettes | Tokens for all 9; v3's literals FAIL on PAPER (1.1 to 1.6:1); fixed |
| 9 | Algorithm names in AF_L | Does not fit; experiment rendered; recommend keeping AF_M |
| 10 | Own-render fixes | 13 fixes listed below, plus 2 changes that cost something |

## 1. HOME gesture (owner decision)

Hold HOME: the MAP shows at once. Keep holding: at `MENU_HOLD_MS = 1500` the header swaps to a MENU countdown (the word `MENU` in AF_M, a 187 x 9 px outlined bar that fills left to right, scene
`map_menu_countdown` at 60 %); at `MENU_COUNT_MS = 1000` later it is full and the menu opens. Let go before that: back to the main screen. The old `LET GO = MENU` text is gone.
**My reading of "about 1.5 s":** the countdown appears at 1.5 s and then runs 1 s, so the menu opens at about 2.5 s. If the owner meant the whole thing takes 1.5 s, set `MENU_HOLD_MS = 0`,
`MENU_COUNT_MS = 1500` (the bar is then on screen from the first frame). The constants are in the file; only the 60 % render depends on neither.
The header while nothing is touched: `TOUCH A CONTROL TO SEE WHAT IT DOES` (AF_S, 230 px ink in 240: 5 px margins, no room for a longer sentence). While a control is touched the header becomes that
control's first help line from `docs/design/help_text.tsv` (scene `map_touch`: `ALL: re-roll every layer`) and the touched tile on the diagram is inverted; the second help line (`FX; edited steps are kept`) is not shown.
**What got worse:** to make room for the lane key and a channel key (the MAP is 234 px tall, as in v3) its tiles lost 1 px of leading: printed name and role now sit 1 px apart on the buttons and
the encoder tiles (v3: 2 px). Still no cut or ellipsised text, but it is a tighter page. While the countdown shows, the touch text is hidden; spec: a touch during the countdown cancels the menu and shows the help.
I did not verify that every control has a `help_text.tsv` line in the PERF context (only `B_FX` was rendered; the encoder control names in the file were not checked).

## 2. ROW indicator

Two 118 x 16 px halves under the cell pairs: `ROW 1` (left, OCT-) and `ROW 2` (right, OCT+) in AF_M (9.5 arcmin; cap 11 in 16 px leaves 3 above and 2 below); the live row is a solid half with the label in ink, the other
a 1 px outline with the label in MID (6.4:1). The ticks were the numerals `I` (one bar) and `II` (two bars): they carried exactly what the label says now, so they are dropped.
Cost: the bar grew 6 px, so the knob cells shrank from 66 to 58 px (label/value/meter moved up; pictured cells have no meter). Weakness: the bar does not say which physical button latches it (OCT- / OCT+); that is on the MAP only.

## 3. MUTE and MIDI OFF are independent marks

Channel cell 58 x 44. Slots (all 14 px, 4 px from the top, 4 px from the right): the **right-most slot** shows the channel's sound state: a solid `M` chip when muted, else the amber activity square when it
just sounded; the slot **to its left** shows MIDI OFF: a ring with a slash (the "no" sign, 14 x 14). Muted also strikes the channel number (3 px bar) and, when not selected, turns the cell hollow with a 2 px frame; selected cells stay inverted
and draw the chip, ring and strike in ink. A channel that is both shows chip + ring + strike (channel 4 in `stress`, selected: reads; channel 4 in `mutes`, unselected: reads).
**Honest trade-off:** v3's `OFF` was a word; now it is a symbol. The word does not fit: `OFF` is 27 px in AF_M, the `M` chip 14, the number 16, and the cell is 58 with 4 px padding. The ring is learned, not self-explaining.
Mitigation: the MAP has a channel key (`M MUTE`, ring `OFF`). If the owner wants the word, the only way is to drop the algorithm name from unselected cells or to widen the cell (not possible: four cells).

## 4. LANE KEY on the MAP

One row at the bottom of the MAP (y 219..233, AF_S), the real marks at real size, one word each: accent triangle `ACCENT`, slide cap `SLIDE`, ratchet comb `RATCH`, edited-step diamond `EDITED`.
No tie/hold mark exists in the lane (a held note is not drawn in v3), so none is shown. The rest dash is not in the key (not asked; one word would fit if the owner wants `REST`: 28 px).
**New mark:** v3 had dropped the edited-step dot; the key needs it, so the lane now draws a 5 x 5 diamond at the top of the column of each hand-entered step (fixture field `ed`, dimmed on a muted channel).
It is the smallest shape on the screen (0.6 mm); see Least sure. To make room the accent triangle moved down 4 px (y 7) and the slide cap to y 14, and the tallest bar is 2 px shorter.
A second key row (right of the OCT tiles) draws the channel marks: `HOLD` box, `M MUTE`, ring `OFF` (the ring is 12 px there, 14 on the cell: no room).

## 5. SNAKE PATH and the other enumerations

**PATH is a picture** (`gallery_path`, `snake_path`): a 31 x 31 px route on a 4 x 4 node grid (9 px pitch) in the knob cell, with the path number (`1`..`14`, AF_M, the spec's own PATH 1..14) beside it.
It reads at that size: the five families (rows, columns, column serpentines, row serpentines and diagonals, spirals) are visible at once, and mirrored pairs differ by the start marker (solid 5 x 5 at step 1; hollow 5 x 5 at step 16).
Faithfulness: the route is computed at render time from the `SHAPE[14][16]` table in `docs/design/algorithms/snake/proto.c` (the file asserts each row is a permutation of 1..16; cell k = reading order of the 4x4 grid; `order[s]` is the cell at step s).
Steps between orthogonal neighbours are 3 px strokes; non-adjacent steps (the "jumping orders" of snake-algorithms 03 section 3.4: paths 1 to 4, 9, 10) are 1 px dim lines, so the solid strokes alone show the contiguous paths 5 to 8 and 11 to 14, exactly the set the spec calls the true snakes.
Weak spots: paths 9 and 10 (diagonal zig-zag) are busy; paths 11 vs 12 and 13 vs 14 differ by a mirror, told only by the start square. No invented name remains: the numeral is the real name.

**Other enumerations drawn as pictures** (`gallery_arp`, `gallery_curve`), each with its **firmware name in AF_M** under the picture (so no invented abbreviation is left; v3 had about 20): ARP `SHAPE` (6; dot plots of the figures in arp SPEC 6.8,
for a 3-tone chord: LINE `0 1 2`, SKIP `0 2 1`, THUMB `0 1 0 2`, OCTV `0 +oct 1 ...`, REPT `0 0 1 1`, WAVE `0 1 2 1 2 0 2 0 1`), ARP `DIR` (6), `DIR` of ACID / SNAKE / STAGE (3: the FWD / BWD / PEND picture family is the
same as UP / DOWN / UPDN), EUCLID `CURVE` (7: AUTO is drawn jagged because it is seeded). Not geometric enough: `RATE` (`1/4` to `4X`, stays AF_L text), numbers, `KEY`.
`P-P` is **gone**: `PING` is 62 px in AF_L (limit 56), `P-P` 44 px but does not say pendulum; the picture of PEND (rise and fall) does. `UP`/`DN`/`ORD`/`CRD`/`RP+` invented forms are gone with it.
**Cost:** a pictured cell's value is now 9.5 arcmin (the AF_M name) plus a picture instead of an 18 arcmin word; the picture is the glance read, the name is the check. RAND and ASPL pictures look alike (both scatter): the name separates them.
Per-picture scaling fills a 42 x 20 px box, so the same pitch is not kept across pictures (a 3-level LINE is drawn taller per step than a 6-level UP).

## 6. Square corners

No `rrect` is called anywhere in v3.1, and **v3's source did not call it either** (grep: 0 hits): every cell, bar and toast was already `rect()` / `frame()`. If the owner saw round corners they come from the AF_L letter shapes,
the REC disc and the diagonal ends of the play triangle, not from the boxes. I cannot tell from here which one the owner meant; all of v3.1's boxes are square, the selected cell is still the inverted block.

## 7. Toast behaviour (spec)

Toasts exist to confirm a press or a turn. A toast must never hide a number the user is changing, and must not hide the playhead.

**Events**

| event | what happens to the toast |
| --- | --- |
| any new toast | replaces the current one at once (one slot, no queue); timer restarts |
| turn of K1..K4 | dismisses an ALGORITHM, DICE or UNDO toast **immediately** (the values must never be hidden by a stale toast); a PRESET-pending toast stays (its countdown is the information) |
| SELECT turn (other channel) | dismisses ALGORITHM, DICE, UNDO (they name `CH 1`); a PRESET toast stays |
| ALGO turn | replaces with the ALGORITHM toast (the title follows the encoder detent by detent) |
| PRESETS turn | PRESET-pending toast; a further turn updates the number and restarts the 1.5 s countdown |
| SAVE (while pending) | does what the toast says (save into the slot); the toast then leaves after a short confirmation (not designed here); EDIT or HOME cancels and dismisses |
| HOME held (map) | dismisses the toast; none is shown while the MAP is up |
| MENU open | dismisses |
| transport start / stop | no effect on a running toast |

**Maximum duration**: ALGORITHM, DICE, UNDO: 1.2 s. PRESET pending: until the load settles (1.5 s after the last turn), then it leaves; nothing else exceeds 1.5 s, and nothing is ever longer than 2.0 s except a pending preset the user keeps turning.
**Which toasts may cover the knob row: none.** The knob row (four labels and four values) is never covered; that is the screen's reason to exist.

**Placements compared** (same scene content, PNGs `toast_algo*`, `toast_preset_*`, `toast_dice_*`):

| placement | hides | reads? | verdict |
| --- | --- | --- | --- |
| v3: block over knobs + cue (92 px) `toast_algo_full` | **the four knob values**, ROW bar, cue | big and clear, but the values you are turning vanish; would have to be cancelled by every knob turn | rejected |
| lane block (70 px) `toast_algo_lane`, `toast_preset_lane`, `toast_dice_lane` | **the pattern and the playhead** (the second owner fact) | big, central, very legible: title AF_L, detail AF_M, shape | rejected for playing; usable only while stopped |
| header banner (24 px) `toast_*_header` | transport shape, BPM, preset, page boxes (the playing/stop state) | AF_L cap 21 in 24 px: 1 px above, 2 below; legible but jammed against the screen edge; the long details (`CH 1  NEW PITCHES`) need the short form | acceptable fallback |
| **bottom band (32 px)** `toast_algo`, `toast_preset_pending`, `toast_dice_notes`, `toast_undo` | ROW bar and the `HOLD HOME = MAP` cue only | AF_L title with 5 px above and 6 px below, AF_M detail right-aligned; the preset countdown is a 2 px bar on the band's edge; the header keeps showing `P04?` too | **recommended** |

What the bottom band costs: the ROW bar is hidden for 1.2 s (the knob labels still say which row is live), and the six-pip list position of v3's algorithm toast is gone (it needed 14 px more; the new algorithm's name is the information).
The long detail is dropped to its short form when it would touch the title (`CH 1 ALGORITHM` to `ALGORITHM`, `CH 1  NEW PITCHES` to `NEW PITCHES`, `SAVE = SAVE HERE` to `SAVE HERE`); in the shown scenes the long form fits except dice.

## 8. Palettes: tokens for every palette

`mock.py palette` lists 9: GREY GREEN AMBER ICE VIOLET ROSE PAPER NIGHT MONO. (`tools/gen_ui_palettes.py` and `palettes.tsv` also have HI-CON, 10 in all; the generated `build/gen/ui_palettes.h` the mock reads has 9, so
HI-CON was not rendered. Its BG is black and TEXT white like NIGHT/MONO; I expect the dark set to work, but this is untested.)
**Finding:** v3's three literals were designed and checked only on GREY, and they **fail on PAPER**: amber `0xFD80` is 1.57:1 against the PAPER background and 1.13:1 against its lane; cyan `0x565F` 1.63 / 1.17; red `0xFA49` 2.91:1.
All three pass on the seven dark palettes unchanged, so the table has three families. `T_REC` already exists in every colour palette (`0xFA49`, PAPER `0xB8C5`); only GREY and MONO have it white. So v3.1 needs **two** new palette tokens, not three, plus one GREY override.

| palette | T_PLAYHEAD | T_ACCSTEP | T_REC | note |
| --- | --- | --- | --- | --- |
| GREY | `0xFD80` amber | `0x565F` cyan | `0xFA49` red | T_REC white in the palette table today; GREY needs the literal (or its T_REC changed) |
| GREEN AMBER ICE VIOLET ROSE NIGHT | `0xFD80` | `0x565F` | `0xFA49` (= the palette's T_REC) | |
| PAPER | `0x82C2` ochre (131,89,16) | `0x1310` teal (16,97,131) | `0xB8C5` (= T_REC) | dark tokens for a light surface |
| MONO | `T_TEXT` | `T_TEXT` | `T_TEXT` | the monochrome fallback: playhead = white column with ink bars; accent = triangle only; REC = disc shape |

Contrast, every palette (non-text 3:1; all 9 pass; lowest is PAPER's playhead on a RAISE cell, 3.52):

| palette | PLAY | ACC | REC | PLAY/BG | PLAY/LANE | PLAY/RAISE | PLAY/SURF | ACC/BG | ACC/LANE | REC/BG | BG on PLAY | min | verdict |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| GREY | FD80 | 565F | FA49 | 11.08 | 6.46 | 6.46 | 9.02 | 10.73 | 6.25 | 5.99 | 11.08 | 5.99 | ok |
| GREEN | FD80 | 565F | FA49 | 10.78 | 7.15 | 6.21 | 8.68 | 10.44 | 6.92 | 5.83 | 10.78 | 5.83 | ok |
| AMBER | FD80 | 565F | FA49 | 10.78 | 7.30 | 6.38 | 8.88 | 10.44 | 7.06 | 5.82 | 10.78 | 5.82 | ok |
| ICE | FD80 | 565F | FA49 | 10.59 | 7.23 | 6.32 | 8.81 | 10.25 | 7.00 | 5.72 | 10.59 | 5.72 | ok |
| VIOLET | FD80 | 565F | FA49 | 10.68 | 7.58 | 6.50 | 9.03 | 10.34 | 7.34 | 5.77 | 10.68 | 5.77 | ok |
| ROSE | FD80 | 565F | FA49 | 10.75 | 8.01 | 6.73 | 9.20 | 10.41 | 7.76 | 5.81 | 10.75 | 5.81 | ok |
| PAPER | 82C2 | 1310 | B8C5 | 5.38 | 3.87 | 3.52 | 4.49 | 5.98 | 4.30 | 5.50 | 5.38 | 3.52 | ok |
| NIGHT | FD80 | 565F | FA49 | 11.62 | 8.43 | 7.97 | 10.25 | 11.25 | 8.16 | 6.28 | 11.62 | 6.28 | ok |
| MONO | FFFF | FFFF | FFFF | 21.00 | 17.76 | 16.29 | 21.00 | 21.00 | 17.76 | 21.00 | 21.00 | 16.29 | ok |

Every other pair the screen draws, per palette (text 4.5:1, shapes 3:1): **none under its threshold**. Nearest misses: selected cell vs a normal cell (`SEL` on `RAISE`, shape, 3:1) is 3.07 on PAPER and 3.08 on VIOLET, `INK` on `SEL` (text) 4.70 on PAPER.
Selection is therefore **by tone, and thin on PAPER, VIOLET and ROSE**; the inversion is the only cue, which is why the selected-cell worst case in change 3 must stay readable in ink.

| pair | need | use | GREY | GREEN | AMBER | ICE | VIOLET | ROSE | PAPER | NIGHT | MONO |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| TEXT on BG | 4.5 | text | 12.33 | 17.34 | 16.88 | 16.86 | 16.67 | 16.98 | 14.74 | 14.17 | 21.00 |
| TEXT on RAISE | 4.5 | text | 7.19 | 9.98 | 9.99 | 10.06 | 10.15 | 10.62 | 9.65 | 9.72 | 16.29 |
| TEXT on SURF | 4.5 | text | 10.03 | 13.96 | 13.91 | 14.04 | 14.09 | 14.53 | 12.31 | 12.50 | 21.00 |
| INK on SEL | 4.5 | text | 10.42 | 6.92 | 6.67 | 6.00 | 5.06 | 5.17 | 4.70 | 7.62 | 12.93 |
| MID on BG | 4.5 | inactive ROW label, shapes | 6.42 | 8.80 | 8.51 | 8.53 | 8.66 | 8.51 | 5.70 | 7.00 | 9.97 |
| MID on SURF | 4.5 | map name text | 5.23 | 7.08 | 7.01 | 7.10 | 7.33 | 7.29 | 4.76 | 6.17 | 9.97 |
| INK on MID | 4.5 | M chip letter | 6.42 | 8.80 | 8.51 | 8.53 | 8.66 | 8.51 | 5.70 | 7.00 | 9.97 |
| MID on RAISE | 3.0 | shape: rest dash, hollow bar | 3.75 | 5.07 | 5.03 | 5.09 | 5.27 | 5.33 | 3.73 | 4.80 | 7.73 |
| MID on LANE | 3.0 | shape: rest dash, hollow bar, beat pip | 3.75 | 5.83 | 5.76 | 5.83 | 6.15 | 6.35 | 4.10 | 5.08 | 8.43 |
| TEXT on LANE | 3.0 | shape: bars | 7.19 | 11.50 | 11.43 | 11.52 | 11.83 | 12.66 | 10.61 | 10.29 | 17.76 |
| SEL on RAISE | 3.0 | selected vs normal cell | 6.07 | 3.98 | 3.95 | 3.58 | 3.08 | 3.23 | 3.07 | 5.23 | 10.03 |
| SEL on BG | 3.0 | selected vs muted (hollow) cell | 10.42 | 6.92 | 6.67 | 6.00 | 5.06 | 5.17 | 4.70 | 7.62 | 12.93 |
| INK on TEXT | 4.5 | ROW label on the live half | 12.33 | 17.34 | 16.88 | 16.86 | 16.67 | 16.98 | 14.74 | 14.17 | 21.00 |
| MID on BG | 3.0 | hollow cell frame | 6.42 | 8.80 | 8.51 | 8.53 | 8.66 | 8.51 | 5.70 | 7.00 | 9.97 |

Colour separation between tokens and the bars (not contrast): cyan vs `T_TEXT` is dE 37 on ICE and 40 on GREY (the lowest; 46 to 60 elsewhere), so on ICE an accent bar is only a modest tint away from a normal bar; the accent triangle carries the fact in shape.
On PAPER, ochre vs teal dE 73, ochre vs ink-dark TEXT 52, teal vs TEXT 42. In MONO the surfaces SURF and BG are both black, so the knob cells have no card (only the hot frame and the picture/number show them): legible, but flatter (`stress_pal_mono`).
**An alternative with zero new tokens** (palette-native: playhead = `T_ACCENT`, accent step = `T_THEME`): all pairs pass too (minimum 3.60 on PAPER) and the playhead would be white on GREY/NIGHT/MONO, yellow on GREEN/ROSE, cyan on AMBER, orange on ICE, mint on VIOLET.
I did not render it; it saves the palette-table rows but gives up one fixed "live = amber" meaning.

**Palette-native alternative, contrast:**

| palette | T_ACCENT | T_THEME | ACCENT/BG | ACCENT/LANE | ACCENT/RAISE | THEME/BG | THEME/LANE | min |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| GREY | FFDF | EF5D | 19.43 | 11.32 | 11.32 | 16.82 | 9.81 | 9.81 |
| GREEN | FEAB | 56AF | 13.94 | 9.24 | 8.02 | 10.45 | 6.93 | 6.93 |
| AMBER | 66BC | FD45 | 11.39 | 7.71 | 6.74 | 10.23 | 6.93 | 6.74 |
| ICE | FC6C | 55DD | 8.42 | 5.75 | 5.02 | 8.76 | 5.99 | 5.02 |
| VIOLET | 6F35 | B45E | 12.44 | 8.82 | 7.57 | 7.22 | 5.12 | 5.12 |
| ROSE | FEAD | F396 | 13.95 | 10.40 | 8.73 | 7.34 | 5.48 | 5.48 |
| PAPER | A9C1 | 0AA8 | 5.50 | 3.96 | 3.60 | 7.67 | 5.52 | 3.60 |
| NIGHT | FFDF | 3EEC | 20.37 | 14.78 | 13.97 | 11.80 | 8.56 | 8.56 |
| MONO | FFFF | FFFF | 21.00 | 17.76 | 16.29 | 21.00 | 17.76 | 16.29 |

## 9. Algorithm names at AF_L

Measured (ink px; cell 58, limit 56 for AF_L):

| name | AF_M | AF_L | fits AF_L 56? |
| --- | --- | --- | --- |
| ACID | 33 | 63 | NO |
| EUCL | 37 | 69 | NO |
| SNAK | 39 | 74 | NO |
| STAG | 36 | 70 | NO |
| ARP | 28 | 53 | yes |
| PATT | 34 | 65 | NO |
| ACD | 30 | 57 | NO |
| EUC | 29 | 54 | yes |
| SNK | 29 | 55 | yes |
| STG | 28 | 53 | yes |
| PAT | 25 | 47 | yes |
| EUCLID | 50 | 95 | NO |
| SNAKE | 47 | 89 | NO |
| STAGE | 45 | 85 | NO |

enum text widths in AF_L (56): DIR3 UP 35 DN 37 P-P 44 PING 62

No 4-letter form fits AF_L in 56 px (63 to 74 px; only `ARP` is 53). 3-letter forms fit except `ACD` (57). Whether the selected cell alone could use AF_L: it holds the channel number (AF_L, 21 px tall) and the name (needs 21 px more) in 44 px: impossible side by side (60 px) or stacked.
**The experiment** (`stress_selL`, `stress_selL_v3_vs_v3_1.png`): the selected cell shows the name in AF_L as a 3-letter form (`ARP`, `STG`, `EUC`, `SNK`, `PAT`, `ACD`) and demotes its channel number to AF_M. Result: the algorithm reads big (18 arcmin), but
(a) the channel number drops to 9.5 arcmin in the one cell that matters, (b) the mute strike moves onto the name, (c) the forms are invented abbreviations and `ACD` is 57 px in a 58 px cell, (d) selected and unselected cells no longer share a layout.
**Recommendation: keep AF_M (9.5 arcmin, 33 to 50 px) names in all four cells.** The only real fix for "just under the 10 arcmin glance mark" is a font size between AF_M and AF_L (about 13 to 14 px caps, an extra font in flash; not measured).
v3.1 did not change the names (`PATT` stays 34 px; `PATTRN` is 54 px).

## 10. Other fixes from re-reading v3 at 4x with the grid

1. **One horizontal grid.** v3 cells were 58 px at pitch 60 (margin 1, gap 2) but lane columns 14 px at pitch 15 (margin 0 left, 1 right), so the four columns of a lane group did not share the cell's edges. Now columns are 13 px with a 2 px gap and 1 px margin: each group spans x 1..58 like a cell, and the knob/ROW halves and the white-key tiles on the MAP use the same grid.
2. **Vertical gaps all 2 px**: header to channel cells, cells to lane (cells 26..70, lane 72..142), lane to knobs (cells start 144).
3. **Transport shape**: play triangle 15 x 16, REC disc 17 px, stop square 15 x 16 differed; now 15 x 15 each, on y 4..18, left edge x 1 (it was x 6, off the cell edge).
4. **Header items** floated at x 30 and 110: BPM now starts at x 22 (after the shape), the preset on the left edge of cell 3 (x 121), and the page boxes' right edge is x 239 (was 234), all on the cell grid.
5. **Channel cells**: the number was left-aligned in a box and the algorithm name centred; both now start their ink at x+4. The strike bar was 20 px starting at x+6 (past the digit's edge); now 18 px at x+4.
6. **OFF chip text** sat low in its frame (2 px of inner space above the capitals, 1 px below); that chip no longer exists (change 3).
7. **Knob cell vertical rhythm**: label top 6 / value top 21 / meter top 50 / bottom pad 6 in a 66 px cell; now 5 / 19 / 45 and 5 below the meter in 58, so top and bottom padding are equal. Meter 46 x 10 became 48 x 8, centred (x+5).
8. **Beat pips** were 6 px in a 14 px column (half a pixel off centre); now 5 px in a 13 px column, exactly centred.
9. **Ratchet combs**: v3's x3 and x4 combs had uneven gaps (1,2 and 1,2,1 px, from rounding); now 4/3/2 px bars with even gaps (3 / 1,1 / 1,1,1 px) that end flush with the 11 px bar.
10. **Playhead column** 14 px wide drawn over a 15 px pitch: 13 px now, 2 px gaps, same as every other column.
11. **Lane marks** rows moved to y 7 and 14, so the new edited diamond (y 0..4) does not touch the accent triangle (y 7..12), and the slide cap ends 1 px above the tallest bar (top y 20).
12. **MAP header** y: v3's title sat 3 px from the top and its rule 14; now the text cap starts at y 3 and the rule is at y 13 (header 13 px).
13. **ROW bar halves** (1..118 and 121..238) and the MAP white-key tiles (x = 1 + 15 n) are on the same grid as the cells and lane.

**Changes that cost something:** the MAP white-key tiles are now 13 px wide (v3: 14), so the two-digit numerals `10`..`16` (13 px ink) touch the tile edges, 1 px worse than v3; and the knob cells are 8 px shorter (change 2).

Not fixed: `OFFSET` (54 px) and `LEGATO` (55 px) fill a 58 px cell with 1.5 px each side; `ACCNT` on the MAP touches its neighbours; `ALGORITHM` fills its 65 px MAP tile exactly.

## (a) Every text item on the glance screen

Cap heights from the real fonts (AF_S 9 px, AF_M 11, AF_L 21) at 8 px/mm, arcmin at 50 cm. "need" = 4.5:1, or 3:1 for AF_L (large). Contrast is the worst instance in any glance scene on GREY.

| role | font | strings drawn (all scenes) | cap px | cap mm | arcmin @50 cm | worst contrast | need | instances in busiest scene |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| bpm | AF_M | `120, EXT 126` | 11 | 1.38 | 9.5 | 12.33:1 | 4.5 ok | 1 |
| preset | AF_M | `P01, P16, P04?` | 11 | 1.38 | 9.5 | 12.33:1 | 4.5 ok | 1 |
| chan number | AF_L | `1, 2, 3, 4` | 21 | 2.62 | 18.0 | 6.42:1 | 3.0 ok | 4 |
| algo name | AF_M | `ARP, ACID, PATT, SNAKE, STAGE, EUCLID` | 11 | 1.38 | 9.5 | 6.42:1 | 4.5 ok | 4 |
| MUTE chip | AF_M | `M` | 11 | 1.38 | 9.5 | 6.42:1 | 4.5 ok | 2 |
| knob label | AF_M | `DIR, DYN, LEN, DENS, HITS, JUMP, OCTS ...` | 11 | 1.38 | 9.5 | 10.03:1 | 4.5 ok | 4 |
| knob value | AF_L | `0, 2, 3, 4, 5, 6, 7 ...` | 21 | 2.62 | 18.0 | 10.03:1 | 3.0 ok | 4 |
| row label | AF_M | `ROW 1, ROW 2` | 11 | 1.38 | 9.5 | 6.42:1 | 4.5 ok | 2 |
| cue | AF_M | `HOLD HOME = MAP` | 11 | 1.38 | 9.5 | 12.33:1 | 4.5 ok | 1 |
| knob value (M) | AF_M | `7, FWD, OCTV, CHORD` | 11 | 1.38 | 9.5 | 10.03:1 | 4.5 ok | 1 |
| toast title | AF_L | `P04, UNDO, NOTES, EUCLID` | 21 | 2.62 | 18.0 | 10.42:1 | 3.0 ok | 1 |
| toast detail | AF_M | `DICE NOTES, NEW PITCHES, CH 1 ALGORITHM, SAVE = SAVE HERE` | 11 | 1.38 | 9.5 | 10.42:1 | 4.5 ok | 1 |

roles (distinct text kinds) on the glance screen: 12
- glance_idle: 23 strings drawn (L x8, M x15)
- glance_playing: 22 strings drawn (L x8, M x14)
- row2: 23 strings drawn (L x7, M x16)
- mutes: 22 strings drawn (L x7, M x15)
- stress: 21 strings drawn (L x7, M x14)
- stress_row2: 21 strings drawn (L x7, M x14)
- toast_algo: 22 strings drawn (L x9, M x13)
- toast_preset_pending: 22 strings drawn (L x9, M x13)
- toast_dice_notes: 22 strings drawn (L x9, M x13)
- toast_undo: 22 strings drawn (L x9, M x13)
- snake_path: 21 strings drawn (L x7, M x14)

**Item count, honestly.** Distinct text roles on screen at once while playing: 8 (BPM, preset, channel number, algorithm name, knob label, knob value, ROW label, and the conditional chip letter `M`), 9 when a pictured enum (its name) is on screen, one more when stopped (the cue). String instances drawn:
**22 playing, 23 idle** (8 AF_L + 14 to 15 AF_M), 22 under a toast. AF_S strings: **0**. That is 2 more than v3 (20 playing): two `ROW` labels and the `M` chip of the muted+OFF channel (it showed only `OFF` in v3), minus the `OFF` word.

## (b) The MAP: every text item

AF_S (7.7 arcmin) except the `MENU` word (AF_M) in the countdown. It is a page you bring your face to; the glance rule (10 arcmin) applies to the main screen only.

| role | font | strings drawn (all scenes) | cap px | cap mm | arcmin @50 cm | worst contrast | need | instances in busiest scene |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| map title | AF_S | `TOUCH A CONTROL TO SEE WHAT IT DOES` | 9 | 1.12 | 7.7 | 12.33:1 | 4.5 ok | 1 |
| map encoder name | AF_S | `MASTER, SELECT, PRESETS, ALGORITHM` | 9 | 1.12 | 7.7 | 5.23:1 | 4.5 ok | 4 |
| map encoder role | AF_S | `VOLUME, CHANNEL, OF CH 1, P01-P16` | 9 | 1.12 | 7.7 | 10.03:1 | 4.5 ok | 4 |
| map OCT name | AF_S | `OCT+, OCT-` | 9 | 1.12 | 7.7 | 5.23:1 | 4.5 ok | 2 |
| map OCT role | AF_S | `ROW 1, ROW 2` | 9 | 1.12 | 7.7 | 10.03:1 | 4.5 ok | 2 |
| map knob id | AF_S | `K1, K2, K3, K4` | 9 | 1.12 | 7.7 | 6.42:1 | 4.5 ok | 4 |
| map knob row1 | AF_S | `DENS, JUMP, ACCNT, SLIDE` | 9 | 1.12 | 7.7 | 12.33:1 | 4.5 ok | 4 |
| map knob row2 | AF_S | `DIR, PEDAL, PUNCH, OFFSET` | 9 | 1.12 | 7.7 | 6.42:1 | 4.5 ok | 4 |
| map legend | AF_S | `= HOLD` | 9 | 1.12 | 7.7 | 12.33:1 | 4.5 ok | 1 |
| map key word | AF_S | `M, OFF, MUTE, RATCH, SLIDE, ACCENT, EDITED` | 9 | 1.12 | 7.7 | 6.42:1 | 4.5 ok | 7 |
| map button printed | AF_S | `FX, ARP, ENV, GLO, LFO, REC, SCL ...` | 9 | 1.12 | 7.7 | 5.23:1 | 4.5 ok | 12 |
| map button tap | AF_S | `ALL, REC, SET, HITS, HOME, PLAY, SAVE ...` | 9 | 1.12 | 7.7 | 10.03:1 | 4.5 ok | 12 |
| map button hold | AF_S | `BACK, HARM, MENU, RESET` | 9 | 1.12 | 7.7 | 10.03:1 | 4.5 ok | 4 |
| map black key | AF_S | `M1, M2, M3, M4, CHN, PG+, PG- ...` | 9 | 1.12 | 7.7 | 12.33:1 | 4.5 ok | 8 |
| map black key hold | AF_S | `FIL, ROL, SOL` | 9 | 1.12 | 7.7 | 12.33:1 | 4.5 ok | 3 |
| map caption | AF_S | `STEPS 1-16: TAP ON/OFF  HOLD EDIT` | 9 | 1.12 | 7.7 | 12.33:1 | 4.5 ok | 1 |
| map white key | AF_S | `1, 2, 3, 4, 5, 6, 7 ...` | 9 | 1.12 | 7.7 | 7.19:1 | 4.5 ok | 16 |
| map menu word | AF_M | `MENU` | 11 | 1.38 | 9.5 | 12.33:1 | 4.5 ok | 1 |
| map help line | AF_S | `ALL: re-roll every layer` | 9 | 1.12 | 7.7 | 12.33:1 | 4.5 ok | 1 |
- map: 89 strings drawn
- map_menu_countdown: 89 strings drawn
- map_touch: 89 strings drawn

MAP roles: 19, strings drawn in `map`: 89

Tool warnings: none. Tight spots, not cut: `TOUCH A CONTROL TO SEE WHAT IT DOES` 230 of 240 px; `ALGORITHM` exactly fills its tile; steps 10..16 ink 13 px in 13 px tiles. Not on the MAP (no room): the M1..M4 and CHN hold roles, the OCT hold-peek, `GLO+SELECT`, `GLO+PLAY`.

## (c) Enumerated-knob names

Names are the firmware strings (`knob_map.tsv`). "drawn as": pictured values show the firmware name in AF_M under the picture; `RATE` is plain AF_L text; PATH shows the route and its number.

| enum | value | firmware string | drawn as | name font | name ink px |
| --- | --- | --- | --- | --- | --- |
| DIR3 | 0 | FWD | picture 46x24 + name | AF_M | 32 |
| DIR3 | 1 | BWD | picture 46x24 + name | AF_M | 33 |
| DIR3 | 2 | PEND | picture 46x24 + name | AF_M | 38 |
| ARPDIR | 0 | UP | picture 46x24 + name | AF_M | 20 |
| ARPDIR | 1 | DOWN | picture 46x24 + name | AF_M | 45 |
| ARPDIR | 2 | UPDN | picture 46x24 + name | AF_M | 40 |
| ARPDIR | 3 | RAND | picture 46x24 + name | AF_M | 40 |
| ARPDIR | 4 | ASPL | picture 46x24 + name | AF_M | 36 |
| ARPDIR | 5 | CHORD | picture 46x24 + name | AF_M | 51 |
| CURVE | 0 | AUTO | picture 46x24 + name | AF_M | 40 |
| CURVE | 1 | SINE | picture 46x24 + name | AF_M | 31 |
| CURVE | 2 | TRI | picture 46x24 + name | AF_M | 21 |
| CURVE | 3 | SAW+ | picture 46x24 + name | AF_M | 40 |
| CURVE | 4 | SAW- | picture 46x24 + name | AF_M | 38 |
| CURVE | 5 | ARCH | picture 46x24 + name | AF_M | 39 |
| CURVE | 6 | INV | picture 46x24 + name | AF_M | 24 |
| SHAPE | 0 | LINE | picture 46x24 + name | AF_M | 30 |
| SHAPE | 1 | SKIP | picture 46x24 + name | AF_M | 31 |
| SHAPE | 2 | THUMB | picture 46x24 + name | AF_M | 52 |
| SHAPE | 3 | OCTV | picture 46x24 + name | AF_M | 40 |
| SHAPE | 4 | REPT | picture 46x24 + name | AF_M | 36 |
| SHAPE | 5 | WAVE | picture 46x24 + name | AF_M | 40 |
| RATE | 0 | 1/4 | text | AF_L | 42 |
| RATE | 1 | 1/2 | text | AF_L | 42 |
| RATE | 2 | 1X | text | AF_L | 33 |
| RATE | 3 | 2X | text | AF_L | 34 |
| RATE | 4 | 3T | text | AF_L | 34 |
| RATE | 5 | 4X | text | AF_L | 35 |
| PATH | 0 | path 1 | route picture 31x31 (from snake/proto.c SHAPE) + numeral | AF_M | 5 |
| PATH | 1 | path 2 | route picture 31x31 (from snake/proto.c SHAPE) + numeral | AF_M | 9 |
| PATH | 2 | path 3 | route picture 31x31 (from snake/proto.c SHAPE) + numeral | AF_M | 9 |
| PATH | 3 | path 4 | route picture 31x31 (from snake/proto.c SHAPE) + numeral | AF_M | 9 |
| PATH | 4 | path 5 | route picture 31x31 (from snake/proto.c SHAPE) + numeral | AF_M | 9 |
| PATH | 5 | path 6 | route picture 31x31 (from snake/proto.c SHAPE) + numeral | AF_M | 9 |
| PATH | 6 | path 7 | route picture 31x31 (from snake/proto.c SHAPE) + numeral | AF_M | 9 |
| PATH | 7 | path 8 | route picture 31x31 (from snake/proto.c SHAPE) + numeral | AF_M | 9 |
| PATH | 8 | path 9 | route picture 31x31 (from snake/proto.c SHAPE) + numeral | AF_M | 9 |
| PATH | 9 | path 10 | route picture 31x31 (from snake/proto.c SHAPE) + numeral | AF_M | 17 |
| PATH | 10 | path 11 | route picture 31x31 (from snake/proto.c SHAPE) + numeral | AF_M | 14 |
| PATH | 11 | path 12 | route picture 31x31 (from snake/proto.c SHAPE) + numeral | AF_M | 17 |
| PATH | 12 | path 13 | route picture 31x31 (from snake/proto.c SHAPE) + numeral | AF_M | 17 |
| PATH | 13 | path 14 | route picture 31x31 (from snake/proto.c SHAPE) + numeral | AF_M | 17 |

Numbers that are not enums stay numbers (AF_L; widest `64` / `120` are 34 / 50 px) with a 48 x 8 meter. `DENS` / `HITS` at 0 read `KEY` (52 px AF_L) with an empty meter. `SPAN` of ARP is labelled `OCTS` and shows `1`..`4`. PATTERNS `PICK` shows the category on the label line (`HOUSE`) and the number in the category as the value (six pips for the category).

## Firmware cost

- **`ui_sq.c`**: all zone draws rewritten to the table above (zone heights 24/48/72/76/16/4; blits A 240x72, B 240x72, C 240x96, all under CV_MAX 240x124): header with 15 x 15 transport shapes, channel cell (new right-slot logic, ring-slash and diamond routines),
  lane columns 13 px at pitch 15 with the edited diamond, knob cell with three value renderers (AF_L number + meter; picture + AF_M name; route + numeral), ROW bar with text, toast band in blit C with the short/long detail choice and the preset countdown.
  New primitives (all 1 px spans or `cv_rect`): up triangle, right triangle, disc, ring-slash, diamond, comb bar, dot-plot with a 2 px joining line, polyline curve, route renderer. No bitmap font.
- **State the screen needs** that may not exist yet: knob `lo/hi` and enum id per (algorithm, knob); activity per channel (`snd0`, exists); MIDI OFF per channel (exists: `midi_ch` 16); the edited flag per step (`ed`, exists in the fixture); REC armed; tempo source; the settle fraction of a pending preset.
- **Generated tables**: `knob_map.tsv` gains `lo`, `hi`, `enum`; a picture table (36 values: 6 + 6 + 3 + 7 point lists of at most 10 points, roughly 250 bytes, not built, plus the 14 x 16 SNAKE `SHAPE` table if the firmware does not already expose it: 224 bytes; the route is derived at draw time);
  `tools/gen_ui_tables.py` emits them; palette table gains 2 x 9 RGB565 values (36 bytes) and a GREY T_REC; `PICK` category names (6).
- **`widths.tsv`** rows for every new string: the knob names in AF_M, `ROW 1` `ROW 2`, `MENU`, `TOUCH A CONTROL TO SEE WHAT IT DOES`, the toast titles and details (long and short), the lane-key words, `MUTE`, `OFF`.
- **Value-name tables**: firmware names kept, so no new name table; `OCTS` for `SPAN` of ARP, `KEY` for DENS 0 are the only relabels.
- **Glyph / icon routines**: ring-slash (14 and 12 px), diamond, comb, die and undo arrow (2x of 9 x 9 and 11 x 7 bitmaps), a drawn `?` (AF_L has none: 15 x 21 from 5 x 7 at x3; alternative one glyph in AF_L, not measured).
- **Palette tables**: T_PLAYHEAD and T_ACCSTEP per palette (PAPER differs; MONO falls back to T_TEXT), GREY T_REC.
- **Behaviour**: HOME-hold map and countdown (two timers), help-line lookup per control for the MAP header, the toast state machine of section 7 (kind, title, detail long/short, shape, countdown, dismiss rules), a MAP channel key.
- **Memory**: no new buffers; the largest blit is 240 x 96 (23040 px).

## Least sure about

1. **The ring-slash OFF.** It replaced a word with a symbol the owner has not seen; it could be misread as "disabled step" or as "mute". The MAP key helps only after the fact. This is the change I trust least.
2. **The edited diamond** is 5 x 5 px (0.6 mm, 3.6 arcmin at 50 cm): it will not be seen at a glance, and it sits where the eye does not look (top of a column). It is a state you check, not a state you see.
3. **Pictures at 1x and at 50 cm.** The PATH route (31 px, 3 px strokes) and the 4 px dots of the dot plots are 3.9 mm and 0.5 mm: fine at 4x on a monitor, unproven on the panel. The 9.5 arcmin name under each picture is the safety net.
4. **The MENU timing** (appears at 1.5 s, opens at 2.5 s) is my reading of the brief.
5. **Selection by tone only** (inverted cell): 3.07:1 against a normal cell on PAPER and 3.08:1 on VIOLET. Unchanged from v3, still the first thing to test on the panel in daylight.
6. **The MAP at 7.7 arcmin and 1 px leading**: denser than v3. It is the page you bring your face to, but the tiles are now cramped.
7. **Bottom toast band** hides the ROW bar; if the user toggles OCT during a toast the new row will not show until the toast ends (labels change at once, though).
8. **Per-picture scaling** means the same musical step is drawn with different heights in different pictures; two pictures (RAND, ASPL) look alike.
9. **HI-CON** palette not rendered (not in the generated header the mock reads).

## Owner questions

1. Menu timing: is "map at once, countdown appears at about 1.5 s, menu 1 s later" what you meant, or should the whole countdown take 1.5 s from the first frame?
2. OFF as a symbol (ring-slash) or the word `OFF` (then the algorithm name would have to go from unselected cells)?
3. Bottom toast band (recommended), header banner, or lane block?
4. Pictures plus firmware names for DIR / SHAPE / CURVE / PATH, instead of big text abbreviations: accepted? (RATE stays text.)
5. Two new palette tokens per palette (amber / cyan, dark ochre / teal for PAPER) plus a GREY T_REC, or the zero-token palette-native alternative?
6. Is the edited-step diamond worth drawing (it is the only mark for hand-entered steps), or should edited steps show elsewhere (e.g. a thicker bar)?
7. Should the ROW labels also name the button that latches them (`ROW 1  OCT-`, 88 px of the 118 px half in AF_M: it fits)? It adds two more words.
8. Is a font between AF_M and AF_L worth its flash cost for algorithm names and knob labels (change 9)?
9. Should the MAP header, once a control is touched, show both help lines (needs 12 px more height)?
