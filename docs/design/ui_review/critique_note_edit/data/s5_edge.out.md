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
