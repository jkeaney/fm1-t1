
### note_open

#### texts: band, lit pixel extents (x0,y0..x1,y1 inclusive), column index of the ink left edge (15 px columns), offset into that column, distance from the right edge
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

### note_set

#### texts: band, lit pixel extents (x0,y0..x1,y1 inclusive), column index of the ink left edge (15 px columns), offset into that column, distance from the right edge
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

### note_dice

#### texts: band, lit pixel extents (x0,y0..x1,y1 inclusive), column index of the ink left edge (15 px columns), offset into that column, distance from the right edge
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

### note_final

#### texts: band, lit pixel extents (x0,y0..x1,y1 inclusive), column index of the ink left edge (15 px columns), offset into that column, distance from the right edge
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

### worst_w1

#### texts: band, lit pixel extents (x0,y0..x1,y1 inclusive), column index of the ink left edge (15 px columns), offset into that column, distance from the right edge
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

### worst_w2

#### texts: band, lit pixel extents (x0,y0..x1,y1 inclusive), column index of the ink left edge (15 px columns), offset into that column, distance from the right edge
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

## geometry of the NOTE_EDIT-specific elements vs the grid (from the op log)

### note_open
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

### note_set
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

### note_dice
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

### note_final
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

### worst_w1
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

### worst_w2
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

## status line capacity arithmetic (firmware formula uisq_ne_status_layout; text_w from the mock = firmware advance)
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
