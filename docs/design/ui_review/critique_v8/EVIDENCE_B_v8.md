# EVIDENCE_B_v8: measured facts about the v8 screen (240x240, HI-CON)

Facts only (measurements, counts, mismatches). No design advice. Scripts: `critique_v8/scripts/` (`run_all.sh` re-runs all; `common.py` hooks `Canvas.text_flags` and `el.P.art` to record every text and icon draw). Palette HI-CON. Panel 30 mm / 240 px, so 1 px = 0.125 mm = 1.23 arcmin at 35 cm (arcmin = mm/350*3438).
Scene names below are the STATE scenes of `proposals/v8/v8.py`. "Main scenes" = playing, two_channels, one_channel, sel_muted, dice, turning_k2, rec_armed, midi_off.

## 1. Reproduction (s1_repro.py)
- `mock.py render v8.py` exits 0, no warnings printed, 15 scenes written.
- Pixel diff of the 15 fresh PNGs against `proposals/v8/out/v8_<scene>.png`: 0 differing pixels in every scene (all 240x240).

## 2. Text inventory (s2_text.py)
Every text draw goes through `Canvas.text_flags`; ink boxes are screen coordinates (x0,y0,x1,y1 exclusive).

| scene | text draws | AF_M draws/chars | AF_S draws/chars | AF_L draws/chars | distinct (font,fg,bg) roles | distinct strings | icon instances / distinct |
|---|---|---|---|---|---|---|---|
| playing | 22 | 12 / 33 | 6 / 40 | 4 / 5 | 11 | 19 | 18 / 18 |
| two_channels | 21 | 11 / 32 | 6 / 40 | 4 / 5 | 11 | 20 | 17 / 17 |
| one_channel | 20 | 10 / 31 | 6 / 40 | 4 / 5 | 10 | 20 | 17 / 17 |
| sel_muted | 22 | 12 / 32 | 6 / 40 | 4 / 6 | 11 | 20 | 18 / 18 |
| dice | 24 (9 header texts are overpainted by the toast rect y0..20) | 14 / 53 | 6 / 40 | 4 / 5 | 13 | 22 | 18 / 17 (dice_notes twice: tile + toast) |
| turning_k2 | 22 | 12 / 33 | 6 / 40 | 4 / 5 | 12 | 20 | 17 / 17 |
| rec_armed | 17 | 11 / 32 | 2 / 28 | 4 / 5 | 10 | 16 | 12 / 12 |
| midi_off | 21 | 11 / 37 | 6 / 40 | 4 / 4 | 11 | 20 | 17 / 17 |

Icon instance = recorded draws / 2 (the painter runs each draw once per 120 px strip; strip 2 clips). Icons per scene: 12 button icons + header play_s/stop_s (rec_armed: header REC disc is 3 rects, no bitmap) + black-key glyphs (midi_s, left, right, rst, x7 when a used channel is muted; unmuted M keys are a 6x7 rect, not a bitmap).

playing scene, full string list (font, fg on bg, ink box):
- header: `120` AF_M TEXT/BG (17,4,43,17); chips `1` AF_M INK/CH1 (49,5,54,16), `2` CH2/BG (62,4,71,16), `3` MID/SURF (76,4,85,17), `4` CH4/BG (90,5,99,16); `ACID` AF_M CH1/BG (105,4,138,17); `MIDI` AF_S MID/BG (142,7,165,16); `2` AF_M WHITE/BG (167,4,176,16); `P01` AF_M TEXT/BG (209,4,233,17).
- knob tiles: labels AF_M MID/SURF `DENS` (11,26,49,39), `SLIDE` (70,26,108,39), `JUMP` (130,27,170,39), `ACCNT` (185,26,235,39); values AF_L WHITE/SURF `12` (15,48,47,69), `4` (82,48,98,69), `5` (143,48,158,70), `3` (202,48,218,70).
- keybed: `S` `F` `R` AF_S TEXT/RAISE (y 194..204); footer `KEYS = STEPS` AF_S TEXT/BG (2,227,78,237); `HOLD = EDIT  PAGE 1/2` AF_S MID/BG (115,226,238,238).
Other scenes differ by: chips (`+` AF_M MID/BG at the first unused chip), algo name (`EUCLID` ink 105..155, `SNAKE` 105..152), MIDI value (`--` AF_M REC/BG (185,10,197,13) when OFF), toast (`NOTES DICED` AF_M INK/WHITE (22,4,115,17), `EDIT=UNDO` (153,4,236,17)), rec footer `REC: ALL KEYS PLAY NOTES` AF_S TEXT/BG (10,227,160,237).
Distinct fonts used: all three (AF_S, AF_M, AF_L). AF_L is used only for knob values; AF_S only for `MIDI`, black-key letters and footer.
Cap heights: AF_S 9 px = 1.12 mm = 11.1 arcmin; AF_M 11 px = 1.38 mm = 13.5 arcmin; AF_L 21 px = 2.62 mm = 25.8 arcmin.

## 3. Contrast (s3_contrast.py, s3b_nontext.py)
HI-CON values: BG/INK #000000, SURF #292829, TEXT #fffaff, MID #b4aeb4, DIM #6a696a, RAISE #414041, KEY #c5c2c5, REC #ff484a. v8 colours: CH1 #e69d00, CH2 #52b6ee, CH3 #f6e641, CH4 #cd79a4, WHITE #ffffff, PHC #525052, DICE_BG #392c08, DICE_FG #ffca52; knob fill mix(CH,120/255) = #6a4800 #20556a #736918 #5a384a.

### 3.1 Text pairs
Background taken from a no-glyph render of the same scene, all distinct colours under each ink box. Flag thresholds: <4.5, AF_L <3.
- Failing: (1) AF_M MID on {SURF, MID}: the muted chip digit `3` (sel_muted, playing): the strike line (T.MID, 13x19 diagonal) crosses the digit and is the same colour as the digit, ratio 1.00 at the overlapped pixels (SURF side 6.75).
- Toast-occluded header texts in the dice scene (TEXT/CH1/CH2/CH4/MID on WHITE, 1.03 to 3.06) are drawn but overpainted; not visible.
- All other pairs pass. Lowest visible passes: AF_L WHITE on SURF 14.69 / on the CH3 knob fill #736918 5.58 (lowest AF_L); AF_M REC (`--`) on BG 6.28; AF_M MID on SURF 6.75 (knob labels); AF_M CH4 on BG 6.87; AF_M INK on CH1 9.19, CH2 9.28; AF_S MID on BG 9.65; AF_S TEXT on RAISE 10.01; AF_M CH3 on BG 16.31; TEXT on BG 20.37; INK/WHITE 21.00.
- AF_L values straddling the fill edge (fill height <30 px): the text bg argument is SURF while the fill lies under part of the glyph box. Anti-aliasing pixels computed against SURF that sit on the fill: `12` 117 px (fill 27 px), `4` 45 px (fill 18), `5` 10 px (fill 9), `3` 28 px (fill 13). Fill height >=30 uses the fill as bg.

### 3.2 Non-text pairs (WCAG ratio, flag <3)
| pair | ratio |
|---|---|
| CH1 / CH2 / CH3 / CH4 on BG | 9.19 / 9.28 / 16.31 / 6.87 |
| CH1 / CH2 / CH3 / CH4 on SURF (keybed lane step) | 6.43 / 6.49 / 11.41 / 4.80 |
| CH1 / CH2 / CH3 / CH4 bar on PHC (playhead column) | 3.50 / 3.53 / 6.21 / **2.61** |
| knob fill on empty tile SURF, CH1..4 | **1.78 / 1.80 / 2.63 / 1.47** (all four <3) |
| knob fill on BG, CH1..4 | **2.54 / 2.57** / 3.76 / **2.10** |
| muted dashed DIM on BG | 3.84 |
| muted dashed DIM on PHC (muted bar under playhead) | **1.46** |
| empty-step tick DIM on BG / on PHC | 3.84 / **1.46** |
| muted header strike MID on SURF | 6.75 (MID on MID digit: **1.00**) |
| playhead column PHC on BG / on SURF | **2.63 / 1.84** |
| playhead white bar on BG / on PHC | 21.00 / 7.99 |
| accent white cap on BG / on PHC | 21.00 / 7.99 |
| dice icon DICE_FG on DICE_BG | 8.99 |
| DICE_BG tile on BG / vs plain SURF tile | **1.54** / **1.08** |
| plain tile SURF on BG | **1.43** |
| hold notch MID on SURF / on DICE_BG / INK on WHITE (lit) | 6.75 / 6.28 / 21.00 |
| mute X glyph MID on RAISE | 4.75 |
| black key RAISE on BG / vs SURF white-key tile | **2.03 / 1.42** |
| black-key glyph TEXT on RAISE | 10.01 |
| REC mode: key tile KEY on BG / DIM black key on KEY | 11.90 / 3.10 |
| row marker active CH on BG / inactive MID on BG | 9.19 / 9.65 |
| turning frame CH2 on BG; white tick on SURF; WHITE value on fills | 9.28; 14.69; 8.17, 8.27, 5.58, 10.00 |
| diced hatch CH1..4 on WHITE fill | **2.29 / 2.26 / 1.29** / 3.06 |
| header REC indicator on BG | 6.28 |
Notes: a lit button tile, pending frame, toast, playing bar are WHITE on BG (21.00). Meaning-bearing items under 3:1: knob fill vs empty tile (4 of 4), playhead column vs BG, muted bar and empty-step tick on the playhead column, CH4 bar on the playhead column, button/key bodies vs BG, diced hatch on white (CH1-CH3).

### 3.3 Colour-vision simulation
Machado 2009 severity 1.0 in linear RGB (cross-checked with Vienot 1999). Luminance ratio is WCAG; dE = CIEDE2000.
| pair | normal L / dE | deuteranopia L / dE | protanopia L / dE |
|---|---|---|---|
| CH1-CH2 | 1.01 / 54.3 | 1.14 / 54.9 | 1.27 / 51.3 |
| CH1-CH3 | 1.78 / 22.2 | 1.69 / **12.6** | 1.92 / 16.2 |
| CH1-CH4 | 1.34 / 47.7 | 1.35 / 34.4 | 1.32 / 42.1 |
| CH2-CH3 | 1.76 / 58.6 | 1.93 / 57.4 | 1.52 / 54.2 |
| CH2-CH4 | 1.35 / 46.6 | 1.18 / **17.6** | 1.67 / **15.4** |
| CH3-CH4 | 2.38 / 61.7 | 2.27 / 39.2 | 2.54 / 49.2 |
Simulated sRGB (deut): #c9b310 #86a5ed #ffe64d #9699a2; (prot): #b7a100 #9bb5f1 #fbde1f #818ba6. Vienot cross-check: deut CH1-CH3 12.7, CH2-CH4 18.4 (L 1.20); prot CH1-CH3 16.4, CH2-CH4 15.6. Simulated contrast on BG: 5.9 to 16.7. Luminance ratio between the four colours is <=2.7 in every case; CH1 and CH2 differ by 1.01 in luminance under normal vision.

## 4. Geometry (s4_geom.py, s4b_slots.py)
Vertical bands (rows with any non-BG pixel, playing scene): header 1-19 | gap 20-23 (4) | knob label band 24-39 | gap 40 (1) | value tile 41-76 | gap 77-80 (4) | row markers 81-85 | gap 86 (1) | playhead white bar 87-88 | gap 89-90 (2) | music row 91-123 | gap 124 (1) | quarter ticks 125-127 | gap 128-131 (4) | button row 1 132-155 | gap 156-157 (2) | button row 2 158-181 | gap 182-187 (6) | keybed 188-223 (keys 188-221, playing bar 222-223) | gap 224-225 (2) | footer ink 226-237 (rec_armed 227-236). Dice scene: toast 0-20 over the header. turning_k2: frame spans 23-77.
Strips: (0,120) and (120,240), each 120 px (<=124). Strip edge y=120 crosses the music row (91-123) and nothing else; text ink boxes crossing y=120: none in 11 scenes (nearest: AF_L values y48..70).
Element sizes: header chips 13x19 on 14 px pitch (1 px between chips; x=46+14i); knob tile 58 wide (label band 58x16, value tile 58x36, 53 tall incl. 1 px gap), x=1,61,121,181 (2 px between tiles, 1 px to each screen edge); music cell content 13 px on 15 px pitch (playhead column 15x33 from x-1); button tiles 38x24 (2 px gaps, 40 px pitch); white keys 14x34 (1 px gap); black keys 10x20 with a 1 px BG halo (12x21); row markers 5x5 at x=109,117,125 (3 px apart); hold notch 4x4 triangle (4 lines, x+32..35, y..y+3); accent cap 2 px white.
Icon sizes: dice x6 13x13; undo/settings/home/save/play/rec_ring/rec 13x11; play_s/stop_s 9x9; x7 7x7; midi_s 7x5; left/right/rst 5x7; header REC disc 11x11 from 3 rects.
Music-row slots (offset,width in the 13 px cell): 1 channel (0,13); 2: (0,6)(7,6), 1 px between; 3: (0,4)(5,4)(10,4), 1 px between, last slot ends at offset 13 so the pixel run reaches offset 14 and the gap to the next step is 1 px (other layouts leave 2 px); 4: (0,3)(3,3)(7,3)(10,3), gaps 0 / 1 / 0 px (CH1|CH2 and CH3|CH4 touch). Pixel check at step 1, y=123: 4 ch `.111222.ddd444..`, 2 ch `.111111.222222..`, 1 ch `.1111111111111..`, 3 ch (dice, step 5) `.1W1W......4444p`. Slot index = position in `used`, not channel number (used=[1,3]: CH2 slot 0, CH4 slot 1). Bar height: pitched 4..30 px (pitch range shared across all pitched channels); single-lane (EUCLID) 6 px; same-pitch lane 4 px.
Slot widths in arcmin: 3 px = 3.7', 4 px = 4.9', 6 px = 7.4', 13 px = 16.0'; 2 px stroke 2.5'; 1 px gap 1.2'; 5 px marker 6.1'; hold notch 4.9'; mute X 8.6'.
Separations under 2 px: label band to value tile 1 px; header chips 1 px; music slots 0/1 px (above); accent cap to bar 1 px; white keys 1 px; row marker to playhead bar 1 px (y86); bars to quarter ticks 1 px (y124); footer right text ink ends at x=238 exclusive (pixel 237; 2 px to the last column 239); tiles are 1 px from the screen edges.
Text margins in tiles (tile 58): `ACCNT` ink 185..235 in 181..238 leaves 4 px each side; widest label `LEGATO` advance 55 (>54 usable, <=58 tile), `OFFSET` 54. Value ink inside tile: 0 of 523 knob renders ink outside the tile (all registered TSV algorithms).

## 5. Edge sweep (s5_sweep.py, s5b_sets.py)
- `algos_v8.check(text_w, AF_M, AF_L)` returns 1 problem: `PATTERNS PICK: value 'CAT 100' too wide even in AF_M`.
- Value sweep: ORDER = 7 algorithms (6 TSV + DEMO registered by v8.py), 2 rows x 4 knobs, every value in range: 965 value strings; 800 fit AF_L (advance <=54; widest accepted `MIN` 51); 165 fall back to AF_M; 21 fit neither (AF_M advance 58 > 54 usable, = tile width 58): PATTERNS ROW2 PICK values `CAT 100`..`CAT 120`. AF_M fallbacks other than PICK: ARP SHAPE `THUMB` (52), ARP DIR `CHORD` (51); PICK `CAT 10`..`CAT 99` (49). Real render of knobs() for 523 value steps: 0 canvas warnings, 0 ink outside the 58 px tile (`CAT 100` ink 58 px wide, flush with the tile).
- Header sweep: 7 algorithm short names x MIDI OFF,1..16 x (P01 | pending P16?) = 238 combos. Without a pending preset: 0 overlaps (longest group end x=203: EUCLID with MIDI 10-16, vs `P01`/`P16` ink starting x=209). With the pending frame (x=197..238, `P16?` ink 202..236; group end x is the exclusive ink edge): EUCLID OFF ends 197 (0 px to the frame); EUCLID MIDI 10..16 ends 200-203 (ink crosses the frame by 3-6 px; ink overlaps `P16?` for MIDI 10,12,13,14,15,16); SNAKE MIDI 10,12..16 ends 199 (crosses the frame by 2 px, no `P16?` ink overlap); STAGE MIDI 10,12..16 ends 197 (0 px to the frame). 43 flagged combos in total (end >= 197). Group end x (OFF / 16 / 1): ACID 179/185/173, EUCLID 197/203/191, SNAKE 193/199/187, STAGE 191/197/185, ARP 174/180/168, PATT 181/187/175, DEMO 188/194/182. No scene in STATE sets `pend`.
- Channel sets: 15 non-empty subsets of 4 channels x selected channel 0..3 x selected-muted off/on = 120 renders: 0 warnings, 0 exceptions. 28 (set, sel) combinations have the selected channel outside `used`: renders without error; the header then shows that channel's algo name and MIDI with no highlighted chip. Non-contiguous used=[1,3]: header chips `+` (at chip 1), `2`, `4` (no chip 3). Further 20 renders (all-rest channel in a 3-ch set, only channel all-rest, 4 channels all-rest, 4 channels on every step, EUCLID-only single lane, same-pitch lane, EUCLID muted, pending/rec/toast combos, 7 extra scenes): 0 warnings, 0 exceptions. Total 140 renders. 4 channels on every step bottom row: `.111222.333444..` repeated, no overlap.
- `S['row']=2` (ROW 3 / RIG): `IndexError: list index out of range`.

## 6. Extensibility facts (s6_extend.py)
- Algorithm names in v8.py: lines 26 and 34 (DEMO registration), 42-45 (scene data CHANS: ACID, EUCLID, SNAKE, STAGE), 292-295 (sheet scene lists), 299-300 (arp, demo_algo scene overrides). No `if`/`==` on a name anywhere in draw code.
- Per-algorithm data read by draw code: `["short"]` (line 81), `["rows"]` (line 95), `["lane"]` (lines 135, 149: the only behavioural branch, `== "single"`). `algos_v8.py` name-keyed entries: `UI_EXTRA` lines 24 (PATTERNS short PATT) and 25 (EUCLID lane single).
- AlgoDesc (`firmware/src/sq_types.h`) vs mirror: name[8] mirrored (+ hand-kept `short`); morph label/min/max/def mirrored from TSV text by regex; knob_morph remap pre-applied by the TSV rows; value-name lists exist only in TSV text (MorphDesc has no names field); morph step/fast not mirrored; layer[5] (absent-layer dim) not mirrored and the screen draws six dice icons identically for every algorithm; sub[5][8] dice sub-labels not mirrored (the docstring says algos_v8 supplies them, no field or code does); loop_len/pages not mirrored (footer `PAGE 1/2` is a literal); ROW 3 (RIG) not mirrored (`rows` holds 2 lists); `lane` exists only in the mirror; id, n_seed, flags, cache_bytes, hi, sub_note, mod not used.
- Registering NEWALG from outside (6-char labels such as MMMMMM/WWWWWW, enum values WWWWWW, INTERLEAVED, `%d OCTAVE`): no change needed in v8.py, icons or el.py; `ORDER` is appended. Result: `check()` reports label/value too wide for MMMMMM (AF_M advance 85), WWWWWW, INTERLEAVED (94), `127 / 127` (66) etc.; render completes, canvas warnings "text 'MMMMMM' is cut by the canvas (ink -9,27..68,38)" and "text 'STUVWX' is cut" (ink to x=241); ink falls outside the 58 px tile for 3 of 8 texts per row; knobs() has no `text_fit`/ellipsis path (value drawn with AF_M unclamped when AF_L does not fit). A `short` of 8 characters is flagged by `check()` (header slot 60 px / 6 chars). Existing-registry worst label (`LEGATO` 55) is the widest realistic one.

## 7. Accuracy vs TSVs (s7_accuracy.py, s7b_enums.py): 42 claims, 34 match, 8 mismatch
Buttons (12 claims, all match): tap roles ALL NOTES ACCNT SHAPE UNDO SET HOME SAVE ARTIC HITS PLAY REC equal the button_modes.tsv PERF labels; hold notch present exactly on EDIT (RESET), GLO (HARM), HOME (HLP/MN), SAVE (UNDO L), which are the only 4 of 12 buttons whose hold row is not "no hold role"; rendered notch pixel verified on all 12 tiles. The role strings in `BUTTONS` are never drawn (icons only). B_OCTDN and B_OCTUP (PERF rows tap, hold, hold-both) are not drawn.
Black keys (11 claims, all match): x centres 15 30 45 75 90 120 135 150 180 195 225 and roles M1 M2 M3 M4 CHN SOL FIL ROL PG- PG+ RST equal key_map.tsv F#3 G#3 A#3 C#4 D#4 F#4 G#4 A#4 C#5 D#5 F#5 positions and tap/hold labels. Hold roles PRV1..PRV4 and RIG have no mark on the screen; M glyph of an unused channel is blank.
Footer/rec: `KEYS = STEPS` matches white-key tap STEP (match); `HOLD = EDIT` matches white-key hold EDIT (match); `PAGE 1/2` is a literal (MISMATCH: no viewed-page or page-count state); rec footer `REC: ALL KEYS PLAY NOTES` vs page_states REC cue `REC: PLAY KEYS TO WRITE  REC=done` (MISMATCH); rec_armed keeps the PERF knob band while page_states REC knob band is CURSOR ACC SLIDE OCT (MISMATCH).
Knobs: labels, ranges and numeric defaults of all 48 slots equal an independent parse of knob_map.tsv (match); ACID DENS 0 reads `KEYS` (match); ARP SPAN reads `1 OCT`..`4 OCT` (match); ARP RATE names and default `1X` (match); EUCLID CURVE names (match); PATTERNS DENS 0 `KEYS` (match). Mismatches: PATTERNS PICK prints `CAT n` where the TSV says the cell reads a category name plus number (example `HOUSE 3`); STAGE TOTAL prints the bare number where the TSV adds `(n)`; ARP ROW2 DIR parses to 7 values `UP DOWN UPDN RAND ASPL CHORD UP` (the trailing `def UP` token is taken as a name; range 0..6 instead of 0..5; meter at CHORD 83%, TSV lists 6 directions).
Toast/header: dice toast `NOTES DICED` + `EDIT=UNDO` uses the TSV labels NOTES (B_SCL tap) and UNDO (B_EDIT tap); the sentence itself appears in no TSV or firmware file (match on roles); one toast tuple exists, for SCL only; ALL/ACCNT/SHAPE/ARTIC/HITS have no toast text (counted as a mismatch of coverage). Header MIDI: ChanSave.midi_ch 0..15, OFF = 16 (SQ_MIDI_OFF) vs screen scene value 1..16 or None drawn `--` in REC colour (mapping is in scene data; the string `16` and the stored value 16 mean different things). Header preset `Pnn` and pending `Pnn?` drawn, `Pnn>` (load queued at the bar) not drawn. Header algorithm names equal ALGO list except PATTERNS shown as `PATT` (MISMATCH; sq_types.h comments name[8] "<= 6 chars" while `PATTERNS` is 8).
page_states.tsv PERF elements absent from the main screen: tempo source INT/EXT/WAIT and clock dot; page map; length; button-picture line 1 tap label and line 2 hold/dice sub-label; preview speaker icon; MUTE n / MIDI OFF text line. The B_ARP/B_SEQ "dim if absent" and "algorithm sub-label under it" (B_SCL, B_ENV, B_LFO, B_ARP, B_SEQ) have no counterpart.

## 8. Coverage of PERF control x gesture rows (s89_cov_cost.py)
Rows: button_modes 30 + key_map 48 + knob_map 59 = 137. Status: yes = persistent glyph/text/state on the default screen; partial = existence shown, role not named; only-after-OCT+ = ROW 2 knobs (drawn once OCT+ latches the other row); no = not drawn in any scene; n/a = row has no action.
- All 137: yes 86 (63%), only-after-OCT+ 24 (18%), no 13 (9%), n/a 8 (6%), partial 6 (4%). Rows with an action (129): yes 67%, only-after-OCT+ 19%, no 10%, partial 5%.
- Buttons (30): yes 12, n/a 8, partial 6, no 4. Keys (48): yes 43, no 5. Knobs (59): yes 31, only-after-OCT+ 24, no 4.
- "Only on press" (transient scenes): dice toast, lit tile, turning frame with old-value tick, rec glyph swap; no PERF row is covered only by a transient.
- Rows `no` (13): B_OCTDN hold (peek); B_OCTDN hold-both (UPDATE); B_OCTUP hold (peek); B_OCTUP hold-both (UPDATE); KEY_B01_F#3, B02_G#3, B03_A#3, B04_C#4 hold (PRV1..PRV4); KEY_B05_D#4 hold (RIG); knobs R1, R2, R3, R4 (ROW 3 RIG wires).
- Rows `partial` (6): B_EDIT hold (RESET), B_GLO hold (HARM), B_HOME hold (HLP/MN), B_SAVE hold (UNDO L): notch only; B_OCTDN tap, B_OCTUP tap: row markers, no OCT button drawn.
- Rows only-after-OCT+ (24): S1..S4 of ACID, EUCLID, SNAKE, STAGE, ARP, PATTERNS.

## 9. Firmware cost facts
- Distinct icon bitmaps drawn across all 15 scenes: 20 (6 dice 13x13, 7 of 13x11, 2 of 9x9, x7 7x7, midi_s 7x5, 3 of 5x7). Size at 1 bpp: row-padded to bytes 379 B; bit-packed 307 B. The 8 main scenes use the same 20 bitmaps. Drawn from rects instead of bitmaps: header REC disc, hold notch, hatch, bars, frames.
- Colours outside the HI-CON token set (RGB565), 12 total: compile-time 8: 0xE4E0 (CH1), 0x55BD (CH2), 0xF728 (CH3), 0xCBD4 (CH4), 0xFFFF (WHITE), 0x528A (PHC), 0x3961 (DICE_BG), 0xFE4A (DICE_FG); runtime mixes 4 (knob fills, `mix(CHn,120)`): 0x6A40, 0x22AD, 0x7343, 0x59C9. HI-CON tokens used in pixels: BG, SURF, TEXT, MID, DIM, RAISE, KEY, REC, LINE, GRID; unused: THEME, ACCENT, SEL, TINT, LANE. Distinct pixel colours over 15 scenes: 163 (10 palette tokens, 11 v8 constants/fills, 142 text anti-aliasing blends, 0 unexplained).

## Scripts
`common.py` (hooks), `s1_repro.py`, `s2_text.py`, `s3_contrast.py` (text pairs), `s3b_nontext.py` (non-text + colour-vision), `s4_geom.py`, `s4b_slots.py`, `s5_sweep.py` (value + header sweeps), `s5b_sets.py` (channel sets), `s6_extend.py`, `s7_accuracy.py`, `s7b_enums.py`, `s89_cov_cost.py`, `s10_extra.py` (anti-aliasing at fill edge, arcmin), `run_all.sh`.
Limit: icon stroke width was not established (the run-length probe in `s10_extra.py` counts diagonal single pixels as 1 px runs, so it does not separate strokes from corners).
