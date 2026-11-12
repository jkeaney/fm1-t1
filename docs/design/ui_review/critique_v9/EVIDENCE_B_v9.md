# EVIDENCE_B_v9: measured evidence for proposals/v9 (facts only)

Method: all numbers come from scripts in `scripts/` (common.py patches `Canvas.text_flags` to log every text draw with font/fg/bg; `render_state` from mock.py; palette HI-CON). Raw outputs: `scripts/out_s2.txt` .. `out_s9.txt`. Screen y values are absolute (strip 2 starts at y=120). ASSESSMENT_A and NOTES.md were not read.

## 1. Reproduction
- `mock.py render v9.py --out /tmp/claude-0/v9b`: 20 scenes x 3 PNGs = 60 files, 0 warnings printed.
- Pixel diff of all 60 files against `proposals/v9/out/`: 0 differing files. Separate in-process render of the 20 scenes vs `v9_<scene>.png`: 0 differing pixels in every scene (`scripts/s1_repro.py`).

## 2. Text inventory (HI-CON; every row: string, font, fg on bg, contrast, ink box in screen px)
Totals per scene (text draws; fonts M/S/L; icon `art` draws incl. black-key and button glyphs):

| scene | text draws | AF_M | AF_S | AF_L | art draws |
|---|---|---|---|---|---|
| playing | 22 | 14 | 4 | 4 | 36 |
| dice | 25 | 17 | 4 | 4 | 36 |
| hold_edit | 23 | 15 | 4 | 4 | 36 |
| midi_off_after | 21 | 13 | 4 | 4 | 34 |
| rec_armed | 17 | 13 | 1 | 3 | 24 |
| pending_p02 | 22 | 14 | 4 | 4 | 34 |

Fixed icons per screen: 12 button icons + 1 header play/stop (rec dot is rects, not a bitmap) + black-key glyphs (M1-M4 squares are rects; x7 for muted; DIN; arrows; RST).
- No text ink crosses y=120 in any of the 14 non-sheet scenes. The only ink touching the screen edge is rec_armed `CURSOR` (x 0..59, tile label ink starts at x=0) and the bottom line (`KEYS = STEPS` ink ends y=239).
- Full per-string list (all six scenes) is in section 11 below (verbatim `out_s2.txt`).

## 3. Contrast (WCAG 2 ratio; thresholds: text <4.5, AF_L <3, non-text <3)
Text pairs below threshold (all six scenes):
- `3` (muted channel digit) AF_M #6a696a on SURF #292829 = **2.69** (<4.5). Appears in `playing`.
- `MIDI` AF_S / `OFF` AF_M #ffffff on OFF_RED #de2829 = **4.73** (passes 4.5, 0.23 margin) (midi_off_after).
- Every other logged text pair is >= 6.75 (MID label on SURF 6.75; ch4 pink digit on BG 6.87). No AF_L pair < 14.69.

Meaning-bearing non-text, flags <3:1 (full list `out_s3.txt`):
| element | pair | ratio |
|---|---|---|
| knob fill (channel colour x120/255) vs empty tile SURF | ch1 1.78, ch2 1.80, ch3 2.63, ch4 1.47 | all <3 |
| same fill vs BG | ch1 2.54, ch2 2.57, ch4 2.10 (ch3 3.76) | <3 |
| position dot OFF (DIM frame) vs SURF | 2.69 | <3 |
| OFF_RED cap on selected chip: over ch2 sky / ch1 amber | 2.09 / 2.07 | <3 |
| playhead column PHC vs BG | 2.63 | <3 |
| ch4 pink bar on PHC column | 2.61 | <3 (ch1 3.50, ch2 3.53, ch3 6.21) |
| dice tile DICE_BG vs plain tile SURF | **1.62** | <3 |
| dice tile vs BG | 2.32 | <3 |
| plain tile SURF vs BG | 1.43 | <3 |
| absent die icon DIM vs DICE_BG | 1.66 | <3 |
| muted chip SURF vs BG | 1.43 | <3 |
| black key RAISE vs BG | 2.03 | <3 |
| hatch (channel colour) on white diced bar | ch1 2.29, ch2 2.26, ch3 1.29 (ch4 3.06) | <3 |
Passing (>=3): channel colours on BG 9.19/9.28/16.31/6.87; on SURF 6.43/6.49/11.41/4.80; hit strips on BG = channel colour on BG; silent dash DIM on BG 3.84; muted hollow frame MID on BG 9.65; OFF_RED hollow frame/chip on BG 4.44; white playhead bar on BG 21.0, on PHC 7.99; white selection bar 21.0; position dot ON vs SURF 4.80..11.41 (white 14.69); row marker active/inactive 9.19/9.65; REC dot 6.28; OFF_RED progress bar on TEXT-bg 4.59.
Luminance note: ch1 amber and ch2 sky have relative luminance 0.409 / 0.414 (lum contrast 1.01 between them).

Colour-vision simulation (Vienot 1999 LMS projection on linear sRGB, protan/deutan matrices; dE = CIE76 on D65 Lab; scripts/s3_contrast.py):
| | normal | deutan | protan |
|---|---|---|---|
| ch1 amber | #e69d00 | #b6b600 | #a7a706 |
| ch2 sky | #52b6ee | #a1a1ef | #aeaeee |
| ch3 yellow | #f6e641 | #ebeb3f | #e8e841 |
| ch4 pink | #cd79a4 | #9898a2 | #8686a4 |
Pairwise dE76 (1-2, 1-3, 1-4, 2-3, 2-4, 3-4): normal 114.8, 35.7, 86.3, 115.1, 58.4, 103.6; deutan 117.9, **19.1**, 81.5, 124.2, 37.8, 89.7; protan 104.3, **24.3**, 87.6, 113.4, **23.7**, 100.7. Smallest: ch1-ch3 (amber vs yellow) 19.1 deutan, ch2-ch4 23.7 protan. Luminance contrast between channel pairs under simulation is 1.01 to 2.69 (ch1-ch2 1.01 normal, 1.10 deutan; ch1-ch4 1.32-1.38; ch2-ch4 1.20-1.70).

## 4. Geometry
Bands (non-BG rows, `playing`): header chips 1..19; selection bar 21..22; knob label tiles 24..39; value tiles 41..76; status + music 79..126 (status text/pill from 77; playhead white bar y90..91; PHC column y92..126); buttons row 1 132..155, row 2 158..181; keybed 188..223 (steps) ; cue line 226..238.
Gaps: 1 px (chip-bar), 1 (bar-knob label), 1 (label-value tile), 2 (value tile to status), 5 (music to buttons), 2 (button rows), 6 (buttons to keybed), 2 (keybed to cue line). Everything <2 px: header chip bottom to selection bar (1 px at y20), selection bar to knob tile (1 px at y23), knob label tile to value tile (1 px at y40), hit-strip pitch gap (1 px between 2 px strips), slide bridge 2x3 in a 2 px column gap, bar-to-bar column gap 2 px.
Music row by channel count (y0=94, y1=127, strip 2 px + 1 gap):
| channels | bar area h (y) | strips y | pitched bar h range | single |
|---|---|---|---|---|
| 1 | 32 (94..125) | none | 6..29 | 8 |
| 2 | 29 (94..122) | 125-126 | 6..26 | 8 |
| 3 | 26 (94..119) | 122-123, 125-126 | 6..23 | 8 |
| 4 | 23 (94..116) | 119-120, 122-123, 125-126 | 6..20 | 8 |
Hit strips 13x2 (hit) or 5x2 (silent dash). With 4 channels the first strip (y119-120) straddles the y=120 strip boundary (no text there). Accent cap 13x2 at bar top -4.
Header: chips x 46..100 (4 x 13 px, pitch 14); BPM ink ends x=43 for 3 digits (3 px to chip 1); name starts x=105 (4 px after chip 4). Sweep, 7 algorithms x MIDI 1..16 + OFF: rightmost ink max = 202 ACID, 220 EUCLID, 216 SNAKE, 214 STAGE, 197 ARP, 204 PATT, 211 DEMO (OFF chip, EUCLID worst, 20 px margin). 119 renders: 0 cut, 0 overlap with the chips, 0 off-screen.
Overlap found: with `pend` set, the white `P02?` pill (x1..41, y77..93) overlaps the playhead white bar (y90..91) and column (y92+) when the playhead is on step 1, 2 or 3; music() draws after status(), the column overwrites pill rows 92-93 and the white bar merges with the white pill (verified by pixel: playhead 0 -> (2,92)=#525052; playhead 3 -> (2,92)=#ffffff pill).

## 5. Sweeps
- `algos_v9.check(text_w, AF_M, AF_L)`: [] (no problems) for ORDER = ACID, EUCLID, SNAKE, STAGE, ARP, PATTERNS, DEMO.
- 964 values (7 algorithms x 2 rows x 4 knobs x every value): widest value ink 52 px (ARP SHAPE `THUMB`, AF_M) and 51 (ARP DIR `CHORD`, PATTERNS PICK `ACD 10`); none >54 (tile). 964 full renders (turning overlay on): 0 exceptions, 0 cut text, 0 warnings.
- Label widths: ARP `LEGATO` 55 px and OFFSET 54 px (tile 58, label limit in check() = 56; sweep's 54 limit exceeded only by LEGATO).
- Font stable per knob across range: yes for every knob except the K1 density knobs (DENS/HITS: ACID, EUCLID, SNAKE, STAGE, ARP, PATTERNS): AF_L for 1..max, AF_M for the value `0` shown as `KEYS` (by design in code). All others one font.
- Position dots vs value-list sizes: dots = max-min+1 = len(names) for every list (ACID/SNAKE/STAGE DIR n=3 dot 8 px; EUCLID CURVE n=7 dot 5; ARP SHAPE/RATE/DIR n=6 dot 6; DEMO SCALE n=4 dot 8, DIR n=3). Max total 47 px of the 50 px budget.
- Channel sets: 832 renders (every non-empty subset of 4 channels, every selected, +/- one muted, +/- one MIDI OFF, REC on/off): 0 exceptions, 0 warnings, 0 cut text. All-rest channel (playing or stopped): renders. `sel` outside `used` renders without exception.

## 6. Extensibility
v9.py names algorithms only in sample data (lines 46-49 CHANS: ACID/EUCLID/SNAKE/STAGE; 29,38 DEMO register; 373-379 scene states). Per-algorithm attribute reads: line 91 `short`, 106 `rows`, 195 `a["lane"] == "single"`, 222 `absent`. No `if algo ==` branch.
Registered `LONGALGO` (short 8 chars, label LONGLABEL, values `100 STEPS`, `MMMMMMM`, 12-value and 20-value lists, range 0..9999, -100..100):
- `check()` reports 4: short name too wide; LONGLABEL too wide; `100 STEPS` too wide; `MMMMMMM` too wide. It does not flag the dot problems below.
- Header: name ink 105..183 fits; with MIDI OFF the OFF text ink reaches x=243 (cut, canvas warning).
- LONGLABEL (AF_M) ink -12..71: cut at the left edge and 11 px into the K2 tile.
- `100 STEPS` ink 173..247: cut at right edge, overlaps the neighbouring column.
- Lists: 12 values -> dot width 2 px (total 46); 20 values -> dot width **0 px** (nothing drawn; formula `(50-(n-1)*2)//n`).
- Other failures (IndexError): state `row=2` (ROW 3/RIG) in knobs(); lane shorter than 16; 5 used channels (header `S["chans"][i]`). `pages=17` draws page dots from x=51; no warning.
- STAGE TOTAL with heard text widest `64(63)` = 46 px.

## 7. Accuracy vs TSV / firmware (83 claims checked, 64 match, 19 mismatch or absent)
Full table with v9 value vs TSV value: section 12 (verbatim `out_s7s8.txt`). Mismatches:
28 GLO hold hint (HARM) never rendered; 29 SAVE hold hint (UNDO L) never rendered; 52 CHN: glyph only, RIG hold not shown; 53 B01-B04 hold PREVIEW not shown; 58 REC CURSOR range fixed 1..16 vs 1..LENGTH; 60 REC cue strip omits `  REC=done` (full string 258 px at AF_M, wider than the 240 px screen); 61 REC lane has no write cursor/banner (music() has no `rec` branch); 62 no `KEYS = NOTES` legend; 64 toasts omit the algorithm sub-label; 68/69 comment "nothing is sounding" vs page_states (preview still plays, speaker icon never dimmed) and the speaker icon is not drawn; 70 header lacks tempo source, clock dot, LENGTH; 71 4-tile live row vs 8-cell band; 72 ROW 3 raises IndexError; 73 no h: hold sub-labels on buttons; 75 PATTERNS shown `PATT` (AlgoDesc.name[8] holds `PATTERNS`, comment says <=6; no `short` field in AlgoDesc); 76 PICK shows `DUB 1`/`ACD 10` vs TSV "CAT nn, e.g. HOUSE 3"; 77 STAGE `48(32)` vs TSV `48 (32)`; 82 docstring says adding an algorithm needs only AlgoDesc + TSV rows, but short/lane/value_fn live in python `UI_EXTRA`.
Matches include: all 12 button tap roles and the 4 non-empty hold roles (RESET, HARM, HLP/MN, UNDO L) and 8 no-hold-role buttons; 11 black-key roles (M1-M4, CHN, SOL, FIL, ROL, PG-, PG+, RST); all 11 black-key x positions equal the right edge of the matching white key; REC labels CURSOR ACC SLIDE OCT; OCT -2..+2; ALGO list; EUCLID CURVE and ARP DIR value lists; DENS 0 = KEYS; ARP SPAN `1 OCT`; MIDI OFF = midi_ch 16 (`SQ_MIDI_OFF`), CHAN K2 MIDI 1..16/OFF, toast `CHN=SET`; button grid order FX SCL ENV LFO EDIT GLO / HOME SAVE ARP SEQ PLAY REC (HANDOFF/02_CONSTRAINTS.md line 28); SQ_NCH 4; hold bar 60% = 0.24 s vs 0.25 s hint start.

## 8. Coverage of PERF control x gesture rows
89 TSV rows (key_map PERF 48 + button_modes PERF 30 + knob_map PERF `*` 11). 8 rows are "no hold role" (n/a). Of the remaining 81: **62 yes (77%)**, **2 on press or hold (2%: EDIT hold, HOME hold)**, **17 no (21%)**. By group: keys 43/48, buttons 14/22, knobs 7/11.
"no" rows: B01 hold, B02 hold, B03 hold, B04 hold (PREVIEW); B05 CHN hold (RIG); B_GLO hold; B_SAVE hold; B_OCTDN tap / hold / hold-both; B_OCTUP tap / hold / hold-both (no OCT buttons on screen); knob R1, R2, R3, R4 turn (ROW 3).

## 9. Firmware cost
- Distinct icon bitmaps drawn across the 20 scenes: 21 (6 dice 13x13; 7 at 13x11 (home, play, rec, rec_ring, save, settings, undo); din 11x9; play_s/stop_s 9x9; midi_s 7x5; x7 7x7; left/right/rst 5x7). 1-bpp packed: **320 B**; row-byte-aligned: **397 B**.
- Colours outside HI-CON tokens (16 tokens): 9 constants: CH1 #e69d00, CH2 #52b6ee, CH3 #f6e641, CH4 #cd79a4, WHITE #ffffff, PHC #525052, DICE_BG #4a484a, OFF_RED #de2829 (INK #000000 equals BG token); plus 4 derived knob fills (channel x120/255): #6a4800, #20556a, #736918, #5a384a. WHITE differs from TEXT #fffaff. Remaining non-token colours on screen (105 of 113 in `playing`) are anti-aliasing ramps of text.

## 11. Text inventory, verbatim (string|font|fg|bg|contrast|ink box)
```
scene|string|font|fg|bg|contrast|box

## playing: 22 text draws, icon draws 36 (art calls, incl. glyphs)
per font: {'M': 14, 'S': 4, 'L': 4} total 22
|120|M|#fffaff|#000000|20.37|(17, 4, 43, 17)
|1|M|#000000|#e69d00|9.19|(49, 5, 54, 16)
|2|M|#52b6ee|#000000|9.28|(62, 4, 71, 16)
|3|M|#6a696a|#292829|2.69|(76, 4, 85, 17)
|4|M|#cd79a4|#000000|6.87|(90, 5, 99, 16)
|ACID|M|#e69d00|#000000|9.19|(105, 4, 138, 17)
|MIDI|S|#b4aeb4|#000000|9.65|(143, 7, 166, 16)
|2|M|#ffffff|#000000|21.00|(168, 4, 177, 16)
|DENS|M|#b4aeb4|#292829|6.75|(11, 26, 49, 39)
|12|L|#ffffff|#292829|14.69|(15, 48, 47, 69)
|SLIDE|M|#b4aeb4|#292829|6.75|(70, 26, 108, 39)
|4|L|#ffffff|#292829|14.69|(82, 48, 98, 69)
|JUMP|M|#b4aeb4|#292829|6.75|(130, 27, 170, 39)
|5|L|#ffffff|#292829|14.69|(143, 48, 158, 70)
|ACCNT|M|#b4aeb4|#292829|6.75|(185, 26, 235, 39)
|3|L|#ffffff|#292829|14.69|(202, 48, 218, 70)
|P01|M|#fffaff|#000000|20.37|(3, 79, 27, 92)
|S|S|#fffaff|#414041|10.01|(116, 194, 123, 204)
|F|S|#fffaff|#414041|10.01|(131, 194, 137, 203)
|R|S|#fffaff|#414041|10.01|(146, 194, 153, 203)
|KEYS = STEPS|M|#fffaff|#000000|20.37|(2, 226, 99, 239)
|HOLD = EDIT|M|#b4aeb4|#000000|9.65|(151, 226, 237, 239)

## dice: 25 text draws, icon draws 36 (art calls, incl. glyphs)
per font: {'M': 17, 'S': 4, 'L': 4} total 25
|120|M|#fffaff|#000000|20.37|(17, 4, 43, 17)
|1|M|#000000|#e69d00|9.19|(49, 5, 54, 16)
|2|M|#52b6ee|#000000|9.28|(62, 4, 71, 16)
|+|M|#b4aeb4|#000000|9.65|(77, 8, 85, 16)
|4|M|#cd79a4|#000000|6.87|(90, 5, 99, 16)
|ACID|M|#e69d00|#000000|9.19|(105, 4, 138, 17)
|MIDI|S|#b4aeb4|#000000|9.65|(143, 7, 166, 16)
|2|M|#ffffff|#000000|21.00|(168, 4, 177, 16)
|DENS|M|#b4aeb4|#292829|6.75|(11, 26, 49, 39)
|12|L|#ffffff|#292829|14.69|(15, 48, 47, 69)
|SLIDE|M|#b4aeb4|#292829|6.75|(70, 26, 108, 39)
|4|L|#ffffff|#292829|14.69|(82, 48, 98, 69)
|JUMP|M|#b4aeb4|#292829|6.75|(130, 27, 170, 39)
|5|L|#ffffff|#292829|14.69|(143, 48, 158, 70)
|ACCNT|M|#b4aeb4|#292829|6.75|(185, 26, 235, 39)
|3|L|#ffffff|#292829|14.69|(202, 48, 218, 70)
|P01|M|#fffaff|#000000|20.37|(3, 79, 27, 92)
|1|M|#000000|#e69d00|9.19|(7, 6, 12, 17)
|NOTES|M|#000000|#ffffff|21.00|(38, 4, 85, 17)
|EDIT=UNDO|M|#000000|#ffffff|21.00|(153, 4, 236, 17)
|S|S|#fffaff|#414041|10.01|(116, 194, 123, 204)
|F|S|#fffaff|#414041|10.01|(131, 194, 137, 203)
|R|S|#fffaff|#414041|10.01|(146, 194, 153, 203)
|KEYS = STEPS|M|#fffaff|#000000|20.37|(2, 226, 99, 239)
|HOLD = EDIT|M|#b4aeb4|#000000|9.65|(151, 226, 237, 239)

## hold_edit: 23 text draws, icon draws 36 (art calls, incl. glyphs)
per font: {'M': 15, 'S': 4, 'L': 4} total 23
|120|M|#fffaff|#000000|20.37|(17, 4, 43, 17)
|1|M|#000000|#e69d00|9.19|(49, 5, 54, 16)
|2|M|#52b6ee|#000000|9.28|(62, 4, 71, 16)
|+|M|#b4aeb4|#000000|9.65|(77, 8, 85, 16)
|4|M|#cd79a4|#000000|6.87|(90, 5, 99, 16)
|ACID|M|#e69d00|#000000|9.19|(105, 4, 138, 17)
|MIDI|S|#b4aeb4|#000000|9.65|(143, 7, 166, 16)
|2|M|#ffffff|#000000|21.00|(168, 4, 177, 16)
|DENS|M|#b4aeb4|#292829|6.75|(11, 26, 49, 39)
|12|L|#ffffff|#292829|14.69|(15, 48, 47, 69)
|SLIDE|M|#b4aeb4|#292829|6.75|(70, 26, 108, 39)
|4|L|#ffffff|#292829|14.69|(82, 48, 98, 69)
|JUMP|M|#b4aeb4|#292829|6.75|(130, 27, 170, 39)
|5|L|#ffffff|#292829|14.69|(143, 48, 158, 70)
|ACCNT|M|#b4aeb4|#292829|6.75|(185, 26, 235, 39)
|3|L|#ffffff|#292829|14.69|(202, 48, 218, 70)
|P01|M|#fffaff|#000000|20.37|(3, 79, 27, 92)
|HOLD EDIT: RESET|M|#000000|#fffaff|20.37|(22, 4, 147, 17)
|S|S|#fffaff|#414041|10.01|(116, 194, 123, 204)
|F|S|#fffaff|#414041|10.01|(131, 194, 137, 203)
|R|S|#fffaff|#414041|10.01|(146, 194, 153, 203)
|KEYS = STEPS|M|#fffaff|#000000|20.37|(2, 226, 99, 239)
|HOLD = EDIT|M|#b4aeb4|#000000|9.65|(151, 226, 237, 239)

## midi_off_after: 21 text draws, icon draws 34 (art calls, incl. glyphs)
per font: {'M': 13, 'S': 4, 'L': 4} total 21
|120|M|#fffaff|#000000|20.37|(17, 4, 43, 17)
|1|M|#e69d00|#000000|9.19|(49, 5, 54, 16)
|2|M|#000000|#52b6ee|9.28|(62, 4, 71, 16)
|+|M|#b4aeb4|#000000|9.65|(77, 8, 85, 16)
|EUCLID|M|#52b6ee|#000000|9.28|(105, 4, 155, 17)
|MIDI|S|#ffffff|#de2829|4.73|(163, 7, 186, 16)
|OFF|M|#ffffff|#de2829|4.73|(189, 4, 216, 17)
|HITS|M|#b4aeb4|#292829|6.75|(14, 26, 46, 39)
|5|L|#ffffff|#292829|14.69|(23, 48, 38, 70)
|OFFSET|M|#b4aeb4|#292829|6.75|(63, 26, 117, 39)
|0|L|#ffffff|#292829|14.69|(82, 48, 98, 70)
|RATCH|M|#b4aeb4|#292829|6.75|(126, 26, 173, 39)
|4|L|#ffffff|#292829|14.69|(142, 48, 158, 69)
|ACCNT|M|#b4aeb4|#292829|6.75|(185, 26, 235, 39)
|2|L|#ffffff|#292829|14.69|(203, 48, 218, 69)
|P01|M|#fffaff|#000000|20.37|(3, 79, 27, 92)
|S|S|#fffaff|#414041|10.01|(116, 194, 123, 204)
|F|S|#fffaff|#414041|10.01|(131, 194, 137, 203)
|R|S|#fffaff|#414041|10.01|(146, 194, 153, 203)
|KEYS = STEPS|M|#fffaff|#000000|20.37|(2, 226, 99, 239)
|HOLD = EDIT|M|#b4aeb4|#000000|9.65|(151, 226, 237, 239)

## rec_armed: 17 text draws, icon draws 24 (art calls, incl. glyphs)
per font: {'M': 13, 'S': 1, 'L': 3} total 17
|120|M|#fffaff|#000000|20.37|(17, 4, 43, 17)
|1|M|#000000|#e69d00|9.19|(49, 5, 54, 16)
|2|M|#52b6ee|#000000|9.28|(62, 4, 71, 16)
|+|M|#b4aeb4|#000000|9.65|(77, 8, 85, 16)
|ACID|M|#e69d00|#000000|9.19|(105, 4, 138, 17)
|MIDI|S|#b4aeb4|#000000|9.65|(143, 7, 166, 16)
|2|M|#ffffff|#000000|21.00|(168, 4, 177, 16)
|CURSOR|M|#b4aeb4|#292829|6.75|(0, 26, 59, 39)
|1|L|#ffffff|#292829|14.69|(24, 48, 34, 69)
|ACC|M|#b4aeb4|#292829|6.75|(75, 26, 105, 39)
|OFF|L|#ffffff|#292829|14.69|(65, 48, 115, 70)
|SLIDE|M|#b4aeb4|#292829|6.75|(130, 26, 168, 39)
|OFF|L|#ffffff|#292829|14.69|(125, 48, 175, 70)
|OCT|M|#b4aeb4|#292829|6.75|(195, 26, 225, 39)
|0|M|#ffffff|#292829|14.69|(205, 49, 214, 62)
|P01|M|#fffaff|#000000|20.37|(3, 79, 27, 92)
|REC: PLAY KEYS TO WRITE|M|#fffaff|#000000|20.37|(2, 226, 183, 239)

## pending_p02: 22 text draws, icon draws 34 (art calls, incl. glyphs)
per font: {'M': 14, 'S': 4, 'L': 4} total 22
|120|M|#fffaff|#000000|20.37|(17, 4, 43, 17)
|1|M|#000000|#e69d00|9.19|(49, 5, 54, 16)
|2|M|#52b6ee|#000000|9.28|(62, 4, 71, 16)
|+|M|#b4aeb4|#000000|9.65|(77, 8, 85, 16)
|4|M|#cd79a4|#000000|6.87|(90, 5, 99, 16)
|ACID|M|#e69d00|#000000|9.19|(105, 4, 138, 17)
|MIDI|S|#b4aeb4|#000000|9.65|(143, 7, 166, 16)
|2|M|#ffffff|#000000|21.00|(168, 4, 177, 16)
|DENS|M|#b4aeb4|#292829|6.75|(11, 26, 49, 39)
|12|L|#ffffff|#292829|14.69|(15, 48, 47, 69)
|SLIDE|M|#b4aeb4|#292829|6.75|(70, 26, 108, 39)
|4|L|#ffffff|#292829|14.69|(82, 48, 98, 69)
|JUMP|M|#b4aeb4|#292829|6.75|(130, 27, 170, 39)
|5|L|#ffffff|#292829|14.69|(143, 48, 158, 70)
|ACCNT|M|#b4aeb4|#292829|6.75|(185, 26, 235, 39)
|3|L|#ffffff|#292829|14.69|(202, 48, 218, 70)
|P02?|M|#000000|#ffffff|21.00|(4, 79, 38, 92)
|S|S|#fffaff|#414041|10.01|(116, 194, 123, 204)
|F|S|#fffaff|#414041|10.01|(131, 194, 137, 203)
|R|S|#fffaff|#414041|10.01|(146, 194, 153, 203)
|KEYS = STEPS|M|#fffaff|#000000|20.37|(2, 226, 99, 239)
|HOLD = EDIT|M|#b4aeb4|#000000|9.65|(151, 226, 237, 239)
```

## 12. Accuracy table, verbatim and coverage
```
n|claim|v9|tsv/firmware|match
1|button FX tap role|ALL|ALL|MATCH
2|button FX hold role|None|ALL (no hold role)|MATCH
3|button SCL tap role|NOTES|NOTES|MATCH
4|button SCL hold role|None|NOTES (no hold role)|MATCH
5|button ENV tap role|ACCNT|ACCNT|MATCH
6|button ENV hold role|None|ACCNT (no hold role)|MATCH
7|button LFO tap role|SHAPE|SHAPE|MATCH
8|button LFO hold role|None|SHAPE (no hold role)|MATCH
9|button EDIT tap role|UNDO|UNDO|MATCH
10|button EDIT hold role|RESET|RESET (hold)|MATCH
11|button GLO tap role|SET|SET|MATCH
12|button GLO hold role|HARM|HARM (hold)|MATCH
13|button HOME tap role|HOME|HOME|MATCH
14|button HOME hold role|HLP/MN|HLP/MN (hold)|MATCH
15|button SAVE tap role|SAVE|SAVE|MATCH
16|button SAVE hold role|UNDO L|UNDO L (hold)|MATCH
17|button ARP tap role|ARTIC|ARTIC|MATCH
18|button ARP hold role|None|ARTIC (no hold role)|MATCH
19|button SEQ tap role|HITS|HITS|MATCH
20|button SEQ hold role|None|HITS (no hold role)|MATCH
21|button PLAY tap role|PLAY|PLAY|MATCH
22|button PLAY hold role|None|PLAY (no hold role)|MATCH
23|button REC tap role|REC|REC|MATCH
24|button REC hold role|None|REC (no hold role)|MATCH
25|hold_edit hint text|HOLD EDIT: RESET|EDIT hold 0.4 s: open RESET (label RESET)|MATCH
26|hold_home hint text|TOUCH=HELP  LET GO=MENU|strip reads 'HOME: touch=HELP  let go=MENU' from 0.25 s|MATCH
27|hold_edit bar fraction (60%) vs 0.25 s hint / 0.4 s fire|60% =0.24 s|fires 0.4 s|MATCH
28|GLO hold hint (HARM) rendered anywhere|no scene, no string in v9.py|hold 0.4 s opens HARMONY|MISMATCH/ABSENT
29|SAVE hold hint (UNDO L) rendered anywhere|no scene, no string in v9.py|hold 0.7 s undo last preset load|MISMATCH/ABSENT
30|black key B01 glyph role|M1|[('tap', 'M1'), ('hold', 'PRV1')]|MATCH
31|black key B02 glyph role|M2|[('tap', 'M2'), ('hold', 'PRV2')]|MATCH
32|black key B03 glyph role|M3|[('tap', 'M3'), ('hold', 'PRV3')]|MATCH
33|black key B04 glyph role|M4|[('tap', 'M4'), ('hold', 'PRV4')]|MATCH
34|black key B05 glyph role|CHN|[('tap', 'CHN'), ('hold', 'RIG')]|MATCH
35|black key B06 glyph role|SOL|[('hold', 'SOL')]|MATCH
36|black key B07 glyph role|FIL|[('hold', 'FIL')]|MATCH
37|black key B08 glyph role|ROL|[('hold', 'ROL')]|MATCH
38|black key B09 glyph role|PG-|[('tap', 'PG-')]|MATCH
39|black key B10 glyph role|PG+|[('tap', 'PG+')]|MATCH
40|black key B11 glyph role|RST|[('tap', 'RST')]|MATCH
41|black key B01 x centre = right edge of white F3|15|15|MATCH
42|black key B02 x centre = right edge of white G3|30|30|MATCH
43|black key B03 x centre = right edge of white A3|45|45|MATCH
44|black key B04 x centre = right edge of white C4|75|75|MATCH
45|black key B05 x centre = right edge of white D4|90|90|MATCH
46|black key B06 x centre = right edge of white F4|120|120|MATCH
47|black key B07 x centre = right edge of white G4|135|135|MATCH
48|black key B08 x centre = right edge of white A4|150|150|MATCH
49|black key B09 x centre = right edge of white C5|180|180|MATCH
50|black key B10 x centre = right edge of white D5|195|195|MATCH
51|black key B11 x centre = right edge of white F5|225|225|MATCH
52|CHN tap/hold|glyph DIN only (tap CHAN; hold RIG not shown)|tap CHAN page (LENGTH MIDI SOURCE FOLLOW); hold RIG|MISMATCH/ABSENT
53|B01-B04 hold = PREVIEW (PRV1-4)|not shown|hold 0.4 s PREVIEW on/off|MISMATCH/ABSENT
54|REC knob K1 label|CURSOR|CURSOR|MATCH
55|REC knob K2 label|ACC|ACC|MATCH
56|REC knob K3 label|SLIDE|SLIDE|MATCH
57|REC knob K4 label|OCT|OCT|MATCH
58|REC CURSOR range|1..16 (REC_ROW max 16)|1..LENGTH, wraps (LENGTH up to 64)|MISMATCH/ABSENT
59|REC OCT range|-2..+2 default 0|-2..+2, default 0|MATCH
60|REC cue strip text|REC: PLAY KEYS TO WRITE (181 px)|REC: PLAY KEYS TO WRITE  REC=done (258 px at AF_M) | tsv='REC: PLAY KEYS TO WRITE  REC=done'|MISMATCH/ABSENT
61|REC lane write cursor|no rec branch in music(); lane identical to PERF|banner REC + lane with the write cursor|MISMATCH/ABSENT
62|REC: PRESET inert toast / REC band 'KEYS = NOTES legend'|not shown|KEYS = NOTES legend|MISMATCH/ABSENT
63|SCL toast word|NOTES|B_SCL tap label NOTES|MATCH
64|toast carries algorithm sub-label (AlgoDesc.sub)|no (word + EDIT=UNDO only)|'algorithm sub-label under it' (B_SCL/B_ENV/B_LFO/B_ARP/B_SEQ tap)|MISMATCH/ABSENT
65|toast hint EDIT=UNDO|EDIT=UNDO|PERF B_EDIT tap UNDO/REDO last dice|MATCH
66|MIDI OFF toast|MIDI OFF / CHN=SET|CHN = D#4 tap opens CHAN page (MIDI is K2 there)|MATCH
67|MIDI OFF value|header OFF chip; None=OFF, 1..16|ChanSave.midi_ch 0..15, 16=OFF (SQ_MIDI_OFF); knob_map CHAN K2 1..16, OFF|MATCH
68|MIDI OFF bars hollow 'nothing is sounding'|hollow OFF_RED frames|PERF: preview speaker icon never dimmed by mute/MIDI OFF; preview still plays (decision 111)|MISMATCH/ABSENT
69|preview speaker icon on channel chip|not drawn|6x6 speaker while PREVIEW ON|MISMATCH/ABSENT
70|header tempo source / clock dot / LENGTH|absent|PERF header: clock dot + BPM + tempo source INT/EXT n/EXT --/WAIT, ..., length|MISMATCH/ABSENT
71|knob band|4 tiles of live row (row dots 2 or 3)|8-cell knob band (live row filled); ROW 3 exists with a RIG|MISMATCH/ABSENT
72|ROW 3 (RIG) knob tiles|status draws 3 dots when S['rig']; knobs() IndexError on row 2|ROW 3 = LEVEL of 4 wires|MISMATCH/ABSENT
73|button line 2 hold sub-labels (h:RST h:HARM ...)|not drawn|PERF button picture line 2|MISMATCH/ABSENT
74|ALGO list|ORDER[:6]=['ACID', 'EUCLID', 'SNAKE', 'STAGE', 'ARP', 'PATTERNS']|ACID EUCLID SNAKE STAGE ARP PATTERNS|MATCH
75|PATTERNS header name|PATT (UI_EXTRA short)|AlgoDesc.name[8] 'PATTERNS' (8 chars; comment says <= 6); AlgoDesc has no short field|MISMATCH/ABSENT
76|PATTERNS PICK cell text|DUB 1|cell reads CAT nn, e.g. HOUSE 3|MISMATCH/ABSENT
77|STAGE TOTAL cell text|48(32)|value reads 48 (32)|MISMATCH/ABSENT
78|ARP SPAN cell|1 OCT|1 OCT..4 OCT|MATCH
79|DENS 0 reads KEYS|KEYS|0 = KEYS ONLY (cell reads KEYS)|MATCH
80|EUCLID CURVE list|['AUTO', 'SINE', 'TRI', 'SAW+', 'SAW-', 'ARCH', 'INV']|AUTO SINE TRI SAW+ SAW- ARCH INV|MATCH
81|ARP DIR list|['UP', 'DOWN', 'UPDN', 'RAND', 'ASPL', 'CHORD']|UP DOWN UPDN RAND ASPL CHORD|MATCH
82|'adding an algorithm = AlgoDesc row + TSV rows' (docstring)|short/lane/value_fn from UI_EXTRA (python only)|AlgoDesc fields: name,id,n_seed,flags,knob_morph,morph,layer,sub,fns - no short/lane/format|MISMATCH/ABSENT
83|channels max|4|SQ_NCH=4 (CH1..CH4 clamped)|MATCH
matches 64 of 83

coverage rows 89 {'yes': 62, 'no': 17, 'n/a': 8, 'on press or hold': 2}
excluding n/a: 81 yes 77% | on press or hold 2% | no 21%
NO list:
   key B01 hold - PRV preview not shown
   key B02 hold - PRV preview not shown
   key B03 hold - PRV preview not shown
   key B04 hold - PRV preview not shown
   key B05 CHN hold - RIG not shown
   button B_GLO hold - no hint string/scene
   button B_SAVE hold - no hint string/scene
   button B_OCTDN tap - no OCT buttons on the screen
   button B_OCTDN hold - no OCT buttons on the screen
   button B_OCTDN hold-both - no OCT buttons on the screen
   button B_OCTUP tap - no OCT buttons on the screen
   button B_OCTUP hold - no OCT buttons on the screen
   button B_OCTUP hold-both - no OCT buttons on the screen
   knob R1 turn - ROW 3 not drawn (IndexError)
   knob R2 turn - ROW 3 not drawn (IndexError)
   knob R3 turn - ROW 3 not drawn (IndexError)
   knob R4 turn - ROW 3 not drawn (IndexError)
key 48 rows; yes+hold: 43
button 22 rows; yes+hold: 14
knob 11 rows; yes+hold: 7
```

## 13. Raw contrast, geometry, sweep, extensibility outputs
### out_s3
```
tokens: {'BG': '#000000', 'SURF': '#292829', 'TEXT': '#fffaff', 'MID': '#b4aeb4', 'DIM': '#6a696a', 'REC': '#ff484a', 'RAISE': '#414041', 'KEY': '#c5c2c5'}

## channel colours: hex, contrast vs BG, vs SURF, vs dice tile
CH1 amber #e69d00 BG 9.19 SURF 6.43 DICE 3.97 RAISE 4.52
CH2 sky #52b6ee BG 9.28 SURF 6.49 DICE 4.01 RAISE 4.56
CH3 yellow #f6e641 BG 16.31 SURF 11.41 DICE 7.04 RAISE 8.02
CH4 pink #cd79a4 BG 6.87 SURF 4.80 DICE 2.96 RAISE 3.38

## non-text meaning-bearing pairs (flag <3)
ch1 hit strip (13x2) on BG                                     #e69d00 on #000000  9.19 
ch1 knob fill mix120 vs SURF (empty)                           #6a4800 on #292829  1.78 FLAG<3.0
ch1 knob fill mix120 vs BG                                     #6a4800 on #000000  2.54 FLAG<3.0
ch1 selected chip fill on BG                                   #e69d00 on #000000  9.19 
ch1 position dot ON vs SURF                                    #e69d00 on #292829  6.43 
ch2 hit strip (13x2) on BG                                     #52b6ee on #000000  9.28 
ch2 knob fill mix120 vs SURF (empty)                           #20556a on #292829  1.80 FLAG<3.0
ch2 knob fill mix120 vs BG                                     #20556a on #000000  2.57 FLAG<3.0
ch2 selected chip fill on BG                                   #52b6ee on #000000  9.28 
ch2 position dot ON vs SURF                                    #52b6ee on #292829  6.49 
ch3 hit strip (13x2) on BG                                     #f6e641 on #000000  16.31 
ch3 knob fill mix120 vs SURF (empty)                           #736918 on #292829  2.63 FLAG<3.0
ch3 knob fill mix120 vs BG                                     #736918 on #000000  3.76 
ch3 selected chip fill on BG                                   #f6e641 on #000000  16.31 
ch3 position dot ON vs SURF                                    #f6e641 on #292829  11.41 
ch4 hit strip (13x2) on BG                                     #cd79a4 on #000000  6.87 
ch4 knob fill mix120 vs SURF (empty)                           #5a384a on #292829  1.47 FLAG<3.0
ch4 knob fill mix120 vs BG                                     #5a384a on #000000  2.10 FLAG<3.0
ch4 selected chip fill on BG                                   #cd79a4 on #000000  6.87 
ch4 position dot ON vs SURF                                    #cd79a4 on #292829  4.80 
white pos dot ON (REC) vs SURF                                 #ffffff on #292829  14.69 
position dot OFF (DIM frame) vs SURF                           #6a696a on #292829  2.69 FLAG<3.0
muted/silent hit strip dash DIM on BG                          #6a696a on #000000  3.84 
muted hollow bar frame MID on BG                               #b4aeb4 on #000000  9.65 
MIDI OFF hollow bar frame OFF_RED on BG                        #de2829 on #000000  4.44 
OFF_RED cap on chip over ch2 sky (chip selected fill)          #de2829 on #52b6ee  2.09 FLAG<3.0
OFF_RED cap on chip over BG (unselected frame)                 #de2829 on #000000  4.44 
OFF_RED cap over CH1 amber                                     #de2829 on #e69d00  2.07 FLAG<3.0
OFF_RED chip on BG                                             #de2829 on #000000  4.44 
white MIDI/OFF text on OFF_RED (text, 4.5 threshold)           #ffffff on #de2829  4.73 
playhead column PHC on BG                                      #525052 on #000000  2.63 FLAG<3.0
playhead white bar on BG                                       #ffffff on #000000  21.00 
playhead white bar on PHC                                      #ffffff on #525052  7.99 
ch1 bar on PHC (playhead col)                                  #e69d00 on #525052  3.50 
ch2 bar on PHC (playhead col)                                  #52b6ee on #525052  3.53 
ch3 bar on PHC (playhead col)                                  #f6e641 on #525052  6.21 
ch4 bar on PHC (playhead col)                                  #cd79a4 on #525052  2.61 FLAG<3.0
white accent cap on BG                                         #ffffff on #000000  21.00 
white accent cap on PHC                                        #ffffff on #525052  7.99 
dice tile DICE_BG vs plain tile SURF                           #4a484a on #292829  1.62 FLAG<3.0
dice tile DICE_BG vs BG                                        #4a484a on #000000  2.32 FLAG<3.0
plain tile SURF vs BG                                          #292829 on #000000  1.43 FLAG<3.0
tile (SURF) vs BG knob tiles                                   #292829 on #000000  1.43 FLAG<3.0
dice icon WHITE on DICE_BG                                     #ffffff on #4a484a  9.06 
absent die icon DIM on DICE_BG                                 #6a696a on #4a484a  1.66 FLAG<3.0
plain icon TEXT on SURF                                        #fffaff on #292829  14.25 
row marker active (ch1) vs BG                                  #e69d00 on #000000  9.19 
row marker inactive MID frame vs BG                            #b4aeb4 on #000000  9.65 
page dot active TEXT vs BG                                     #fffaff on #000000  20.37 
page dot inactive MID vs BG                                    #b4aeb4 on #000000  9.65 
selection bar WHITE under chip vs BG                           #ffffff on #000000  21.00 
muted chip SURF vs BG                                          #292829 on #000000  1.43 FLAG<3.0
muted diagonal MID on SURF                                     #b4aeb4 on #292829  6.75 
muted digit DIM on SURF (text)                                 #6a696a on #292829  2.69 FLAG<4.5
unselected chip frame ch4 on BG                                #cd79a4 on #000000  6.87 
selected chip ink digit INK on ch1 (text)                      #000000 on #e69d00  9.19 
REC dot T.REC on BG                                            #ff484a on #000000  6.28 
hold hint progress bar INK on TEXT-bg                          #000000 on #fffaff  20.37 
hold hint progress bar OFF_RED(100%) on TEXT-bg                #de2829 on #fffaff  4.59 
hatch (dice-diced) CH on WHITE ch1                             #e69d00 on #ffffff  2.29 FLAG<3.0
hatch ch2 on WHITE                                             #52b6ee on #ffffff  2.26 FLAG<3.0
hatch ch3 yellow on WHITE                                      #f6e641 on #ffffff  1.29 FLAG<3.0
hatch ch4 on WHITE                                             #cd79a4 on #ffffff  3.06 
diced bar (hatched WHITE) vs BG                                #ffffff on #000000  21.00 
black key RAISE vs BG gap                                      #414041 on #000000  2.03 FLAG<3.0
black key glyph ch colour vs RAISE: ch1                        #e69d00 on #414041  4.52 
black key glyph x7 MID on RAISE                                #b4aeb4 on #414041  4.75 
white key SURF (lane step)                                     #292829 on #000000  1.43 FLAG<3.0
keybed step marker ch1 on SURF                                 #e69d00 on #292829  6.43 
rec keys T.KEY vs BG                                           #c5c2c5 on #000000  11.90 
keybed playhead WHITE bar vs BG                                #ffffff on #000000  21.00 
knob value-fill ch3 yellow-mix vs WHITE value text (text 4.5)  #736918 on #ffffff  5.58 
value text WHITE on fill ch1 mix120 (text)                     #ffffff on #6a4800  8.27 
value text WHITE on fill ch2 mix120 (text)                     #ffffff on #20556a  8.17 
value text WHITE on fill ch3 mix120 (text)                     #ffffff on #736918  5.58 
value text WHITE on fill ch4 mix120 (text)                     #ffffff on #5a384a  10.00 
empty-lane rest tick DIM on BG                                 #6a696a on #000000  3.84 
turning frame ch on SURF/BG: ch1                               #e69d00 on #000000  9.19 
turning white pegs vs fill ch3                                 #ffffff on #736918  5.58 

## deuteranopia / protanopia simulation of 4 channel colours
none ['#e69d00', '#52b6ee', '#f6e641', '#cd79a4']
  pairwise dE76 (CIELAB): 1-2:114.8 1-3:35.7 1-4:86.3 2-3:115.1 2-4:58.4 3-4:103.6
  luminance contrast pairwise: 
deut ['#b6b600', '#a1a1ef', '#ebeb3f', '#9898a2']
  pairwise dE76 (CIELAB): 1-2:117.9 1-3:19.1 1-4:81.5 2-3:124.2 2-4:37.8 3-4:89.7
  luminance contrast pairwise: 
prot ['#a7a706', '#aeaeee', '#e8e841', '#8686a4']
  pairwise dE76 (CIELAB): 1-2:104.3 1-3:24.3 1-4:87.6 2-3:113.4 2-4:23.7 3-4:100.7
  luminance contrast pairwise: 
none relative lum: ['0.409', '0.414', '0.766', '0.293']  pairwise lum-contrast: 1-2:1.01 1-3:1.78 1-4:1.34 2-3:1.76 2-4:1.35 3-4:2.38
deut relative lum: ['0.436', '0.392', '0.773', '0.317']  pairwise lum-contrast: 1-2:1.10 1-3:1.69 1-4:1.32 2-3:1.86 2-4:1.20 3-4:2.24
prot relative lum: ['0.360', '0.456', '0.752', '0.248']  pairwise lum-contrast: 1-2:1.23 1-3:1.95 1-4:1.38 2-3:1.59 2-4:1.70 3-4:2.69
```
### out_s4
```
playing non-BG row runs: [(1, 19), (21, 22), (24, 39), (41, 76), (79, 126), (132, 155), (158, 181), (188, 223), (226, 238)] gaps: [1, 1, 1, 2, 5, 2, 6, 2]
one_channel non-BG row runs: [(1, 19), (21, 22), (24, 39), (41, 76), (79, 91), (97, 125), (132, 155), (158, 181), (188, 221), (226, 238)] gaps: [1, 1, 1, 2, 5, 6, 2, 6, 4]
two_channels non-BG row runs: [(1, 19), (21, 22), (24, 39), (41, 76), (79, 126), (132, 155), (158, 181), (188, 223), (226, 238)] gaps: [1, 1, 1, 2, 5, 2, 6, 2]

## text crossing y=120 / touching screen edge, all scenes
rec_armed EDGE (0, 26, 59, 39, 'CURSOR', 0)
sheet_row1_a CROSS120 (11, 118, 49, 131, 'DENS', 0)
sheet_row1_a CROSS120 (76, 119, 102, 130, 'LEN', 0)
sheet_row1_a CROSS120 (125, 118, 173, 131, 'RANGE', 0)
sheet_row1_a CROSS120 (192, 119, 227, 130, 'PATH', 0)
sheet_row1_b CROSS120 (11, 118, 49, 131, 'LOCK', 0)
sheet_row1_b CROSS120 (74, 118, 104, 131, 'BITS', 0)
sheet_row1_b CROSS120 (127, 118, 172, 131, 'SCALE', 0)
sheet_row1_b CROSS120 (191, 118, 228, 131, 'SPRD', 0)
sheet_row2_a CROSS120 (3, 118, 57, 131, 'OFFSET', 0)
sheet_row2_a CROSS120 (71, 118, 108, 131, 'GLEN', 0)
sheet_row2_a CROSS120 (135, 119, 164, 130, 'DYN', 0)
sheet_row2_a CROSS120 (198, 119, 221, 130, 'DIR', 0)
sheet_row2_b CROSS120 (3, 118, 57, 131, 'OFFSET', 0)
sheet_row2_b CROSS120 (72, 118, 108, 131, 'GATE', 0)
sheet_row2_b CROSS120 (135, 119, 164, 130, 'DYN', 0)
sheet_row2_b CROSS120 (198, 119, 221, 130, 'DIR', 0)

## music geometry per channel count (code constants: y0=94,y1=127,strip_h=2,gap 1)
channels 1 bar area h 32 bar y 94 .. 125 strips y [] strip last row None playhead col y 92..126; strips cross y=120: []
  min bar h 6 single=8; pitched h=6+(bhmax-9)*frac -> range 6 .. 29
channels 2 bar area h 29 bar y 94 .. 122 strips y [(125, 126)] strip last row 126 playhead col y 92..126; strips cross y=120: []
  pitched bar range 6 .. 26 ; single 8; accent cap 4px above bar (y by-4..by-3); strips 13x2 hit /5x2 dash
channels 3 bar area h 26 bar y 94 .. 119 strips y [(122, 123), (125, 126)] strip last row 126 playhead col y 92..126; strips cross y=120: []
  pitched bar range 6 .. 23 ; single 8; accent cap 4px above bar (y by-4..by-3); strips 13x2 hit /5x2 dash
channels 4 bar area h 23 bar y 94 .. 116 strips y [(119, 120), (122, 123), (125, 126)] strip last row 126 playhead col y 92..126; strips cross y=120: [(119, 120)]
  pitched bar range 6 .. 20 ; single 8; accent cap 4px above bar (y by-4..by-3); strips 13x2 hit /5x2 dash

## header sweep
ACID      short=ACID   nameEnd=138  max right edge over MIDI1..16/OFF=202  (>=240 off-screen? False)  gap chips->name=4
EUCLID    short=EUCLID nameEnd=156  max right edge over MIDI1..16/OFF=220  (>=240 off-screen? False)  gap chips->name=4
SNAKE     short=SNAKE  nameEnd=152  max right edge over MIDI1..16/OFF=216  (>=240 off-screen? False)  gap chips->name=4
STAGE     short=STAGE  nameEnd=150  max right edge over MIDI1..16/OFF=214  (>=240 off-screen? False)  gap chips->name=4
ARP       short=ARP    nameEnd=133  max right edge over MIDI1..16/OFF=197  (>=240 off-screen? False)  gap chips->name=4
PATTERNS  short=PATT   nameEnd=140  max right edge over MIDI1..16/OFF=204  (>=240 off-screen? False)  gap chips->name=4
DEMO      short=DEMO   nameEnd=147  max right edge over MIDI1..16/OFF=211  (>=240 off-screen? False)  gap chips->name=4

## header render sweep: cut/warn
renders 119 cut/overflow 0
bpm text widths AF_M: {9: 9, 10: 18, 99: 18, 100: 27, 120: 27, 300: 27}  bpm end x for 3 digits = 43  chip1 x=46
DEMO/ACID header chip right 101 name x=105
```
### out_s5a
```
A.check(): []
ORDER: ['ACID', 'EUCLID', 'SNAKE', 'STAGE', 'ARP', 'PATTERNS', 'DEMO']

## knob sweep: algo row knob label font[min..max widths] fits(54 / 58) font-stable dots
ACID      r1 K1 DENS    label_w=38 font=L range 0..16 maxw=36 ('KEYS') fonts_used=['L', 'M']
ACID      r1 K2 SLIDE   label_w=39 font=L range 0..8 maxw=18 ('0') fonts_used=['L']
ACID      r1 K3 JUMP    label_w=40 font=L range 0..20 maxw=36 ('10') fonts_used=['L']
ACID      r1 K4 ACCNT   label_w=50 font=L range 0..8 maxw=18 ('0') fonts_used=['L']
ACID      r2 K1 OFFSET  label_w=54 font=L range 0..15 maxw=36 ('10') fonts_used=['L']
ACID      r2 K2 PEDAL   label_w=45 font=L range 0..12 maxw=36 ('10') fonts_used=['L']
ACID      r2 K3 PUNCH   label_w=51 font=L range 0..47 maxw=36 ('10') fonts_used=['L']
ACID      r2 K4 DIR     label_w=23 font=M range 0..2 maxw=38 ('PEND') fonts_used=['M'] LIST n=3(names 3) dot_w=8 total=28
EUCLID    r1 K1 HITS    label_w=32 font=L range 0..16 maxw=36 ('KEYS') fonts_used=['L', 'M']
EUCLID    r1 K2 OFFSET  label_w=54 font=L range 0..15 maxw=36 ('10') fonts_used=['L']
EUCLID    r1 K3 RATCH   label_w=47 font=L range 0..8 maxw=18 ('0') fonts_used=['L']
EUCLID    r1 K4 ACCNT   label_w=50 font=L range 0..8 maxw=18 ('0') fonts_used=['L']
EUCLID    r2 K1 LEN     label_w=27 font=L range 2..16 maxw=36 ('10') fonts_used=['L']
EUCLID    r2 K2 SPAN    label_w=37 font=L range 0..24 maxw=36 ('10') fonts_used=['L']
EUCLID    r2 K3 PUNCH   label_w=51 font=L range 0..47 maxw=36 ('10') fonts_used=['L']
EUCLID    r2 K4 CURVE   label_w=48 font=M range 0..6 maxw=41 ('SAW+') fonts_used=['M'] LIST n=7(names 7) dot_w=5 total=47
SNAKE     r1 K1 DENS    label_w=38 font=L range 0..16 maxw=36 ('KEYS') fonts_used=['L', 'M']
SNAKE     r1 K2 LEN     label_w=27 font=L range 1..16 maxw=36 ('10') fonts_used=['L']
SNAKE     r1 K3 RANGE   label_w=49 font=L range 0..48 maxw=36 ('10') fonts_used=['L']
SNAKE     r1 K4 PATH    label_w=36 font=L range 1..14 maxw=36 ('10') fonts_used=['L']
SNAKE     r2 K1 OFFSET  label_w=54 font=L range 0..15 maxw=36 ('10') fonts_used=['L']
SNAKE     r2 K2 GLEN    label_w=37 font=L range 1..16 maxw=36 ('10') fonts_used=['L']
SNAKE     r2 K3 DYN     label_w=30 font=L range 0..10 maxw=36 ('10') fonts_used=['L']
SNAKE     r2 K4 DIR     label_w=23 font=M range 0..2 maxw=38 ('PEND') fonts_used=['M'] LIST n=3(names 3) dot_w=8 total=28
STAGE     r1 K1 DENS    label_w=38 font=L range 0..8 maxw=36 ('KEYS') fonts_used=['L', 'M']
STAGE     r1 K2 TOTAL   label_w=45 font=M range 8..64 maxw=18 ('10') fonts_used=['M']
STAGE     r1 K3 DIR     label_w=23 font=M range 0..2 maxw=38 ('PEND') fonts_used=['M'] LIST n=3(names 3) dot_w=8 total=28
STAGE     r1 K4 RATCH   label_w=47 font=L range 0..8 maxw=18 ('0') fonts_used=['L']
STAGE     r2 K1 OFFSET  label_w=54 font=L range 0..63 maxw=36 ('10') fonts_used=['L']
STAGE     r2 K2 RDIV    label_w=33 font=L range 1..4 maxw=18 ('1') fonts_used=['L']
STAGE     r2 K3 DYN     label_w=30 font=L range 0..10 maxw=36 ('10') fonts_used=['L']
STAGE     r2 K4 RANGE   label_w=49 font=L range 0..36 maxw=36 ('10') fonts_used=['L']
ARP       r1 K1 DENS    label_w=38 font=L range 0..16 maxw=36 ('KEYS') fonts_used=['L', 'M']
ARP       r1 K2 SHAPE   label_w=47 font=M range 0..5 maxw=52 ('THUMB') fonts_used=['M'] LIST n=6(names 6) dot_w=6 total=46
ARP       r1 K3 SPAN    label_w=37 font=M range 1..4 maxw=43 ('1 OCT') fonts_used=['M']
ARP       r1 K4 RATE    label_w=35 font=M range 0..5 maxw=23 ('1/4') fonts_used=['M'] LIST n=6(names 6) dot_w=6 total=46
ARP       r2 K1 OFFSET  label_w=54 font=L range 0..15 maxw=36 ('10') fonts_used=['L']
ARP       r2 K2 LEGATO  label_w=55 font=L range 0..8 maxw=18 ('0') fonts_used=['L']
ARP       r2 K3 DYN     label_w=30 font=L range 0..10 maxw=36 ('10') fonts_used=['L']
ARP       r2 K4 DIR     label_w=23 font=M range 0..5 maxw=51 ('CHORD') fonts_used=['M'] LIST n=6(names 6) dot_w=6 total=46
PATTERNS  r1 K1 DENS    label_w=38 font=L range 0..16 maxw=36 ('KEYS') fonts_used=['L', 'M']
PATTERNS  r1 K2 VARY    label_w=37 font=L range 0..8 maxw=18 ('0') fonts_used=['L']
PATTERNS  r1 K3 SPAN    label_w=37 font=L range 1..4 maxw=18 ('1') fonts_used=['L']
PATTERNS  r1 K4 ACCNT   label_w=50 font=L range 0..8 maxw=18 ('0') fonts_used=['L']
PATTERNS  r2 K1 PICK    label_w=32 font=M range 1..120 maxw=51 ('ACD 10') fonts_used=['M']
PATTERNS  r2 K2 SLIDE   label_w=39 font=L range 0..8 maxw=18 ('0') fonts_used=['L']
PATTERNS  r2 K3 DYN     label_w=30 font=L range 0..14 maxw=36 ('10') fonts_used=['L']
PATTERNS  r2 K4 CTONE   label_w=49 font=L range 0..8 maxw=18 ('0') fonts_used=['L']
DEMO      r1 K1 LOCK    label_w=38 font=L range 0..16 maxw=36 ('10') fonts_used=['L']
DEMO      r1 K2 BITS    label_w=31 font=L range 1..16 maxw=36 ('10') fonts_used=['L']
DEMO      r1 K3 SCALE   label_w=45 font=M range 0..3 maxw=37 ('PENT') fonts_used=['M'] LIST n=4(names 4) dot_w=8 total=38
DEMO      r1 K4 SPRD    label_w=37 font=M range 1..4 maxw=43 ('1 OCT') fonts_used=['M']
DEMO      r2 K1 OFFSET  label_w=54 font=L range 0..15 maxw=36 ('10') fonts_used=['L']
DEMO      r2 K2 GATE    label_w=36 font=L range 1..8 maxw=18 ('1') fonts_used=['L']
DEMO      r2 K3 DYN     label_w=30 font=L range 0..10 maxw=36 ('10') fonts_used=['L']
DEMO      r2 K4 DIR     label_w=23 font=M range 0..2 maxw=38 ('PEND') fonts_used=['M'] LIST n=3(names 3) dot_w=8 total=28
values swept 964
ISSUES:
ARP label LEGATO w=55>54
full renders 964 exceptions 0 cut texts 0 []
captured stdout warnings: [] n= 0
```
### out_s5b
```
channel-set renders 832 exceptions 0 cut texts 0 []
all-rest ch1 selected: OK
sel not in used: rendered (no exception)
P02? pill x 1 .. 41 y 77..93 ; playhead white bar y90..91 and column y92.. at x=15*i..15*i+14; overlap for playhead cols: [0, 1, 2]
pixel(5,91) with playhead0 + pending: #ffffff ; pixel(5,85) pill: #000000
P12? width 40
one_channel lit rows ok
warnings stdout: 0 
```
### out_s6
```
A.check() with LONGALGO:
   LONGALGO: short name 'LONGALGO' does not fit the header
   LONGALGO LONGLABEL: label too wide
   LONGALGO STEPS: value '100 STEPS' too wide even in AF_M
   LONGALGO WIDE: value 'MMMMMMM' too wide even in AF_M
r1 K1 LONGLABEL font=L widest '10' w=36 (tile58/54)
r1 K2 TEMPO     font=M widest '1000' w=36 (tile58/54)
r1 K3 MODE      font=M widest 'FOXTRT' w=56 (tile58/54) n=12 dot_w=2 total=46
r1 K4 STEPS     font=M widest '100 STEPS' w=75 (tile58/54)
r2 K1 X         font=M widest 'N10' w=29 (tile58/54) n=20 dot_w=0 total=38
r2 K2 WIDE      font=M widest 'MMMMMMM' w=90 (tile58/54) n=3 dot_w=8 total=28
r2 K3 NEG       font=M widest '-100' w=33 (tile58/54)
r2 K4 POS       font=M widest 'b' w=9 (tile58/54) n=4 dot_w=8 total=38
row 1 midi 1 header name box (105, 4, 183, 17) rightmost header ink x 219 cut [(-12, 26, 71, 39, 'LONGLABEL', 2)]
row 1 midi 16 header name box (105, 4, 183, 17) rightmost header ink x 231 cut [(-12, 26, 71, 39, 'LONGLABEL', 2)]
row 1 midi None header name box (105, 4, 183, 17) rightmost header ink x 243 cut [(216, 4, 243, 17, 'OFF', 2), (-12, 26, 71, 39, 'LONGLABEL', 2)]
row 2 midi 1 header name box (105, 4, 183, 17) rightmost header ink x 219 cut []
row 2 midi 16 header name box (105, 4, 183, 17) rightmost header ink x 231 cut []
row 2 midi None header name box (105, 4, 183, 17) rightmost header ink x 243 cut [(216, 4, 243, 17, 'OFF', 2)]
stdout warnings: ["  warning: text 'LONGLABEL' is cut by the canvas (ink -12,26..71,39)", "  warning: text 'LONGLABEL' is cut by the canvas (ink -12,26..71,39)", "  warning: text 'OFF' is cut by the canvas (ink 216,4..243,17)", "  warning: text 'LONGLABEL' is cut by the canvas (ink -12,26..71,39)", "  warning: text 'OFF' is cut by the canvas (ink 216,4..243,17)"]
  warning: text 'OFF' is cut by the canvas (ink 216,4..243,17)
  warning: text 'LONGLABEL' is cut by the canvas (ink -12,26..71,39)
  warning: text '100 STEPS' is cut by the canvas (ink 173,49..247,62)
tile text off-screen (-12, 26, 71, 39, 'LONGLABEL', 2)
tile text off-screen (173, 49, 247, 62, '100 STEPS', 2)
tile text overflows its 58px tile: LONGLABEL (-12, 26, 71, 39) tile (181, 239)
tile text overflows its 58px tile: 100 STEPS (173, 49, 247, 62) tile (121, 179)
rig row index 2 -> EXC IndexError list index out of range
empty lane -> rendered; page-dot leftmost x 216
lane len 8 -> EXC IndexError list index out of range
5 channels used -> EXC IndexError list index out of range
pages=5 -> rendered; page-dot leftmost x 183
pages=17 dots -> rendered; page-dot leftmost x 51
widest STAGE TOTAL with heard: ('64(63)', 46)
```
### out_s9
```
icons drawn in the 20 scenes (unique bitmaps):
  dice_accnt   13x13  packed  22 B  row-aligned  26 B
  dice_all     13x13  packed  22 B  row-aligned  26 B
  dice_artic   13x13  packed  22 B  row-aligned  26 B
  dice_hits    13x13  packed  22 B  row-aligned  26 B
  dice_notes   13x13  packed  22 B  row-aligned  26 B
  dice_shape   13x13  packed  22 B  row-aligned  26 B
  din          11x9   packed  13 B  row-aligned  18 B
  home         13x11  packed  18 B  row-aligned  22 B
  left          5x7   packed   5 B  row-aligned   7 B
  midi_s        7x5   packed   5 B  row-aligned   5 B
  play         13x11  packed  18 B  row-aligned  22 B
  play_s        9x9   packed  11 B  row-aligned  18 B
  rec          13x11  packed  18 B  row-aligned  22 B
  rec_ring     13x11  packed  18 B  row-aligned  22 B
  right         5x7   packed   5 B  row-aligned   7 B
  rst           5x7   packed   5 B  row-aligned   7 B
  save         13x11  packed  18 B  row-aligned  22 B
  settings     13x11  packed  18 B  row-aligned  22 B
  stop_s        9x9   packed  11 B  row-aligned  18 B
  undo         13x11  packed  18 B  row-aligned  22 B
  x7            7x7   packed   7 B  row-aligned   7 B
count 21 total packed 320 B, row-aligned 397 B
code-referenced ICONS keys: 18 ['dice_accnt', 'dice_all', 'dice_artic', 'dice_hits', 'dice_notes', 'dice_shape', 'din', 'home', 'midi_s', 'play', 'play_s', 'rec', 'rec_ring', 'save', 'settings', 'stop_s', 'undo', 'x7']
el.IC referenced: left right rst
ICONS dict total entries (v7+v9): 20

HI-CON tokens:
{'BG': '#000000', 'SURF': '#292829', 'TEXT': '#fffaff', 'THEME': '#ffea00', 'ACCENT': '#00e6ff', 'MID': '#b4aeb4', 'DIM': '#6a696a', 'LINE': '#312c31', 'SEL': '#cdba00', 'TINT': '#201c00', 'INK': '#000000', 'REC': '#ff484a', 'RAISE': '#414041', 'KEY': '#c5c2c5', 'LANE': '#413c20', 'GRID': '#393439'}
CH1 #e69d00 NOT a HI-CON token
CH2 #52b6ee NOT a HI-CON token
CH3 #f6e641 NOT a HI-CON token
CH4 #cd79a4 NOT a HI-CON token
WHITE #ffffff NOT a HI-CON token
INK #000000 in HI-CON tokens
PHC #525052 NOT a HI-CON token
DICE_BG #4a484a NOT a HI-CON token
OFF_RED #de2829 NOT a HI-CON token
fill mix120 ch1 #6a4800 derived (not a token)
fill mix120 ch2 #20556a derived (not a token)
fill mix120 ch3 #736918 derived (not a token)
fill mix120 ch4 #5a384a derived (not a token)
playing unique RGB565 colours on screen 113 ; not tokens: 105
dice unique RGB565 colours on screen 73 ; not tokens: 64
hold_edit unique RGB565 colours on screen 63 ; not tokens: 54
rec_armed unique RGB565 colours on screen 89 ; not tokens: 80
midi_off_after unique RGB565 colours on screen 100 ; not tokens: 92
playing non-token colours: ['#081018', '#100800', '#100c10', '#101010', '#102029', '#102c39', '#181000', '#181400', '#181818', '#18384a', '#201418', '#201c20', '#20445a', '#20506a', '#291c00', '#292c29', '#311c29', '#313031', '#313431', '#31698b', '#317594', '#392400', '#393839', '#393c39', '#397da4', '#412431', '#413c41', '#4189b4', '#4195c5', '#4a2c41', '#4a3000', '#4a444a', '#4a484a', '#4a4c4a', '#4aa1cd', '#4aaade', '#523800', '#524c52', '#525052', '#52b6ee', '#5a344a', '#5a3c00', '#5a555a', '#5a595a', '#5a5d5a', '#624400', '#625d62', '#626162', '#6a3c52', '#6a4800', '#6a656a', '#6a6d6a', '#73445a', '#735000', '#736d73', '#737173', '#7b757b', '#7b797b', '#834c6a', '#835900']
```
