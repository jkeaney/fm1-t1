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

### (a) text pairs (nominal fg/bg as passed to the draw call); '!' = under 4.5:1
| render | string | font | fg | bg passed | ratio | actual dominant colour under the ink box | ratio vs that | flag |
|---|---|---|---|---|---|---|---|---|
| note_open | `120` | M | TEXT | BG | 20.37 | BG | 20.37 |  |
| note_open | `1` | M | BG | CH1 | 9.19 | CH1 | 9.19 |  |
| note_open | `2` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_open | `3` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_open | `4` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_open | `ACID` | M | CH1 | BG | 9.19 | BG | 9.19 |  |
| note_open | `MIDI` | S | MID | BG | 9.65 | BG | 9.65 |  |
| note_open | `1` | M | WHITE | BG | 21.00 | BG | 21.00 |  |
| note_open | `PITCH` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_open | `AUTO` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_open | `GEN C2` | S | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_open | `GATE` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_open | `AUTO` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_open | `GEN 40` | S | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_open | `VELO` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_open | `AUTO` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_open | `GEN 43` | S | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_open | `RATCH` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_open | `AUTO` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_open | `GEN 1X` | S | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_open | `STEP` | S | MID | BG | 9.65 | BG | 9.65 |  |
| note_open | `9` | M | WHITE | BG | 21.00 | BG | 21.00 |  |
| note_open | `= SET BY YOU` | S | MID | BG | 9.65 | BG | 9.65 |  |
| note_open | `S` | S | TEXT | RAISE | 10.01 | RAISE | 10.01 |  |
| note_open | `F` | S | TEXT | RAISE | 10.01 | RAISE | 10.01 |  |
| note_open | `R` | S | TEXT | RAISE | 10.01 | RAISE | 10.01 |  |
| note_open | `KNOBS EDIT` | M | TEXT | BG | 20.37 | BG | 20.37 |  |
| note_open | `HOME = DONE` | M | MID | BG | 9.65 | BG | 9.65 |  |
| note_set | `120` | M | TEXT | BG | 20.37 | BG | 20.37 |  |
| note_set | `1` | M | BG | CH1 | 9.19 | CH1 | 9.19 |  |
| note_set | `2` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_set | `3` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_set | `4` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_set | `ACID` | M | CH1 | BG | 9.19 | BG | 9.19 |  |
| note_set | `MIDI` | S | MID | BG | 9.65 | BG | 9.65 |  |
| note_set | `1` | M | WHITE | BG | 21.00 | BG | 21.00 |  |
| note_set | `PITCH` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_set | `AUTO` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_set | `GEN C2` | S | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_set | `GATE` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_set | `AUTO` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_set | `GEN 40` | S | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_set | `VELO` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_set | `AUTO` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_set | `GEN 43` | S | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_set | `RATCH` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_set | `AUTO` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_set | `GEN 1X` | S | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_set | `STEP` | S | MID | BG | 9.65 | BG | 9.65 |  |
| note_set | `9` | M | WHITE | BG | 21.00 | BG | 21.00 |  |
| note_set | `SLIDE` | S | BG | CH1 | 9.19 | CH1 | 9.19 |  |
| note_set | `PROB 50` | S | BG | CH1 | 9.19 | CH1 | 9.19 |  |
| note_set | `OCT-2` | S | BG | CH1 | 9.19 | CH1 | 9.19 |  |
| note_set | `S` | S | TEXT | RAISE | 10.01 | RAISE | 10.01 |  |
| note_set | `F` | S | TEXT | RAISE | 10.01 | RAISE | 10.01 |  |
| note_set | `R` | S | TEXT | RAISE | 10.01 | RAISE | 10.01 |  |
| note_set | `KNOBS EDIT` | M | TEXT | BG | 20.37 | BG | 20.37 |  |
| note_set | `HOME = DONE` | M | MID | BG | 9.65 | BG | 9.65 |  |
| note_dice | `120` | M | TEXT | BG | 20.37 | BG | 20.37 |  |
| note_dice | `1` | M | BG | CH1 | 9.19 | CH1 | 9.19 |  |
| note_dice | `2` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_dice | `3` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_dice | `4` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_dice | `PITCH` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_dice | `F2` | L | WHITE | CHF1 | 8.27 | CHF1 | 8.27 |  |
| note_dice | `GEN C2` | S | TEXT | CHF1 | 8.02 | CHF1 | 8.02 |  |
| note_dice | `GATE` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_dice | `100` | L | WHITE | CHF1 | 8.27 | CHF1 | 8.27 |  |
| note_dice | `GEN 40` | S | TEXT | CHF1 | 8.02 | CHF1 | 8.02 |  |
| note_dice | `VELO` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_dice | `80` | L | WHITE | CHF1 | 8.27 | CHF1 | 8.27 |  |
| note_dice | `GEN 43` | S | TEXT | CHF1 | 8.02 | CHF1 | 8.02 |  |
| note_dice | `RATCH` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_dice | `AUTO` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_dice | `GEN 1X` | S | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_dice | `STEP` | S | MID | BG | 9.65 | BG | 9.65 |  |
| note_dice | `9` | M | WHITE | BG | 21.00 | BG | 21.00 |  |
| note_dice | `NO SLIDE` | S | BG | CH1 | 9.19 | CH1 | 9.19 |  |
| note_dice | `PROB 50` | S | BG | CH1 | 9.19 | CH1 | 9.19 |  |
| note_dice | `OCT-2` | S | BG | CH1 | 9.19 | CH1 | 9.19 |  |
| note_dice | `S` | S | TEXT | RAISE | 10.01 | RAISE | 10.01 |  |
| note_dice | `F` | S | TEXT | RAISE | 10.01 | RAISE | 10.01 |  |
| note_dice | `R` | S | TEXT | RAISE | 10.01 | RAISE | 10.01 |  |
| note_dice | `KNOBS EDIT` | M | TEXT | BG | 20.37 | BG | 20.37 |  |
| note_dice | `HOME = DONE` | M | MID | BG | 9.65 | BG | 9.65 |  |
| note_dice | `1` | M | BG | CH1 | 9.19 | CH1 | 9.19 |  |
| note_dice | `ALL` | M | BG | WHITE | 21.00 | WHITE | 21.00 |  |
| note_dice | `EDIT=UNDO` | S | BG | WHITE | 21.00 | WHITE | 21.00 |  |
| note_final | `120` | M | TEXT | BG | 20.37 | BG | 20.37 |  |
| note_final | `1` | M | BG | CH1 | 9.19 | CH1 | 9.19 |  |
| note_final | `2` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_final | `3` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_final | `4` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_final | `ACID` | M | CH1 | BG | 9.19 | BG | 9.19 |  |
| note_final | `MIDI` | S | MID | BG | 9.65 | BG | 9.65 |  |
| note_final | `1` | M | WHITE | BG | 21.00 | BG | 21.00 |  |
| note_final | `PITCH` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_final | `AUTO` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_final | `GEN C2` | S | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_final | `GATE` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_final | `AUTO` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_final | `GEN 40` | S | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_final | `VELO` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_final | `83` | L | WHITE | CHF1 | 8.27 | CHF1 | 8.27 |  |
| note_final | `GEN 43` | S | TEXT | CHF1 | 8.02 | CHF1 | 8.02 |  |
| note_final | `RATCH` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_final | `AUTO` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_final | `GEN 1X` | S | MID | SURF | 6.75 | SURF | 6.75 |  |
| note_final | `STEP` | S | MID | BG | 9.65 | BG | 9.65 |  |
| note_final | `9` | M | WHITE | BG | 21.00 | BG | 21.00 |  |
| note_final | `= SET BY YOU` | S | MID | BG | 9.65 | BG | 9.65 |  |
| note_final | `S` | S | TEXT | RAISE | 10.01 | RAISE | 10.01 |  |
| note_final | `F` | S | TEXT | RAISE | 10.01 | RAISE | 10.01 |  |
| note_final | `R` | S | TEXT | RAISE | 10.01 | RAISE | 10.01 |  |
| note_final | `KNOBS EDIT` | M | TEXT | BG | 20.37 | BG | 20.37 |  |
| note_final | `HOME = DONE` | M | MID | BG | 9.65 | BG | 9.65 |  |
| worst_w1 | `120` | M | TEXT | BG | 20.37 | BG | 20.37 |  |
| worst_w1 | `1` | M | BG | CH1 | 9.19 | CH1 | 9.19 |  |
| worst_w1 | `2` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| worst_w1 | `3` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| worst_w1 | `4` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| worst_w1 | `ACID` | M | CH1 | BG | 9.19 | BG | 9.19 |  |
| worst_w1 | `MIDI` | S | MID | BG | 9.65 | BG | 9.65 |  |
| worst_w1 | `1` | M | WHITE | BG | 21.00 | BG | 21.00 |  |
| worst_w1 | `PITCH` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| worst_w1 | `C#-1` | M | WHITE | CHF1 | 8.27 | CHF1 | 8.27 |  |
| worst_w1 | `GEN G#-1` | S | TEXT | CHF1 | 8.02 | CHF1 | 8.02 |  |
| worst_w1 | `GATE` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| worst_w1 | `100` | L | WHITE | CHF1 | 8.27 | CHF1 | 8.27 |  |
| worst_w1 | `GEN 100` | S | TEXT | CHF1 | 8.02 | CHF1 | 8.02 |  |
| worst_w1 | `VELO` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| worst_w1 | `ACC` | M | WHITE | CHF1 | 8.27 | CHF1 | 8.27 |  |
| worst_w1 | `GEN 99` | S | TEXT | CHF1 | 8.02 | CHF1 | 8.02 |  |
| worst_w1 | `RATCH` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| worst_w1 | `4X` | L | WHITE | CHF1 | 8.27 | CHF1 | 8.27 |  |
| worst_w1 | `GEN 4X` | S | TEXT | CHF1 | 8.02 | CHF1 | 8.02 |  |
| worst_w1 | `STEP` | S | MID | BG | 9.65 | BG | 9.65 |  |
| worst_w1 | `64` | M | WHITE | BG | 21.00 | BG | 21.00 |  |
| worst_w1 | `NO SLIDE` | S | BG | CH1 | 9.19 | CH1 | 9.19 |  |
| worst_w1 | `PROB 50` | S | BG | CH1 | 9.19 | CH1 | 9.19 |  |
| worst_w1 | `OCT+2` | S | BG | CH1 | 9.19 | CH1 | 9.19 |  |
| worst_w1 | `OFF` | S | BG | CH1 | 9.19 | CH1 | 9.19 |  |
| worst_w1 | `S` | S | TEXT | RAISE | 10.01 | RAISE | 10.01 |  |
| worst_w1 | `F` | S | TEXT | RAISE | 10.01 | RAISE | 10.01 |  |
| worst_w1 | `R` | S | TEXT | RAISE | 10.01 | RAISE | 10.01 |  |
| worst_w1 | `KNOBS EDIT` | M | TEXT | BG | 20.37 | BG | 20.37 |  |
| worst_w1 | `HOME = DONE` | M | MID | BG | 9.65 | BG | 9.65 |  |
| worst_w2 | `120` | M | TEXT | BG | 20.37 | BG | 20.37 |  |
| worst_w2 | `1` | M | BG | CH1 | 9.19 | CH1 | 9.19 |  |
| worst_w2 | `2` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| worst_w2 | `3` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| worst_w2 | `4` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| worst_w2 | `ACID` | M | CH1 | BG | 9.19 | BG | 9.19 |  |
| worst_w2 | `MIDI` | S | MID | BG | 9.65 | BG | 9.65 |  |
| worst_w2 | `1` | M | WHITE | BG | 21.00 | BG | 21.00 |  |
| worst_w2 | `PITCH` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| worst_w2 | `A#-1` | M | WHITE | CHF1 | 8.27 | CHF1 | 8.27 |  |
| worst_w2 | `GEN D#-1` | S | TEXT | CHF1 | 8.02 | CHF1 | 8.02 |  |
| worst_w2 | `GATE` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| worst_w2 | `TIE` | L | WHITE | CHF1 | 8.27 | CHF1 | 8.27 |  |
| worst_w2 | `GEN TIE` | S | TEXT | CHF1 | 8.02 | CHF1 | 8.02 |  |
| worst_w2 | `VELO` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| worst_w2 | `ACC` | M | WHITE | CHF1 | 8.27 | CHF1 | 8.27 |  |
| worst_w2 | `GEN ACC` | S | TEXT | CHF1 | 8.02 | CHF1 | 8.02 |  |
| worst_w2 | `RATCH` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| worst_w2 | `AUTO` | M | MID | SURF | 6.75 | SURF | 6.75 |  |
| worst_w2 | `GEN 3X` | S | MID | SURF | 6.75 | SURF | 6.75 |  |
| worst_w2 | `STEP` | S | MID | BG | 9.65 | BG | 9.65 |  |
| worst_w2 | `1` | M | WHITE | BG | 21.00 | BG | 21.00 |  |
| worst_w2 | `= SET BY YOU` | S | MID | BG | 9.65 | BG | 9.65 |  |
| worst_w2 | `S` | S | TEXT | RAISE | 10.01 | RAISE | 10.01 |  |
| worst_w2 | `F` | S | TEXT | RAISE | 10.01 | RAISE | 10.01 |  |
| worst_w2 | `R` | S | TEXT | RAISE | 10.01 | RAISE | 10.01 |  |
| worst_w2 | `KNOBS EDIT` | M | TEXT | BG | 20.37 | BG | 20.37 |  |
| worst_w2 | `HOME = DONE` | M | MID | BG | 9.65 | BG | 9.65 |  |

### (a2) distinct text colour pairs across the four renders + 2 worst cases, with every user
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

### (b) non-text touching colour pairs on the final frame (4-neighbour edges; text ink boxes masked); '!' = under 3:1

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
