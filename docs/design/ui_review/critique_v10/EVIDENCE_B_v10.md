# EVIDENCE B (v10): measured facts, no design advice

Proposal `proposals/v10/v10.py`, palette HI-CON, tool `mock/mock.py` (real AF_S / AF_M / AF_L). Every number below is produced by a script in `critique_v10/scripts/` (raw output follows the summary; `build_md.sh` regenerates this file). Optics: 30 mm panel = 240 px (8 px/mm), 35 cm, arcmin = mm / 350 * 3438 (1 px = 1.23 arcmin). ASSESSMENT_A_v10.md was not read.

## Key numbers

1. Reproduction: all 28 scenes re-rendered with the CLI and compared to `out/v10_<scene>.png`: 0 differing pixels in every scene.
2. Text inventory (strings M / S / L, icons): playing 14 / 4 / 4 = 22 strings, 18 icons; dice 17 / 4 / 4 = 25, 18; turn_select 15 / 5 / 4 = 24, 17; hold_save 15 / 4 / 4 = 23, 18; midi_off_after 13 / 4 / 4 = 21, 17; rec_playing 13 / 2 / 3 = 18, 12. No text crosses y=120 in any of the 28 scenes.
3. Contrast: every text pair in the 28 scenes is at or above 4.73 (lowest: WHITE on OFF_RED 4.73, AF_S and AF_M); lowest AF_L pair 8.27 (WHITE on the CH1 knob fill); no text pair is flagged. Non-text pairs below 3:1 (HI-CON): see section 3b; the main ones are SURF vs BG 1.43 (tiles, white keys, muted chip), DICE_BG vs BG 2.32, DICE_BG vs SURF 1.62, PHC (playhead column) vs BG 2.63, knob fill vs empty tile 1.47 (CH4) / 1.78 (CH1) / 1.80 (CH2) / 2.63 (CH3), OFF_RED bar on the playhead column 1.69, CH4 on the playhead column 2.61, OFF_RED cap vs the CH2 chip fill 2.09 (vs CH4 chip 1.55), absent die DIM on DICE_BG 1.66, position dot current vs inactive 2.39 (inactive vs SURF 2.69), page dot active vs inactive 2.11, black key vs BG 2.03 / vs white key 1.42, diced WHITE vs hatch CH1 2.29. Channel colours on BG: 9.19 / 9.28 / 16.31 / 6.87 (on SURF 6.43 / 6.49 / 11.41 / 4.80). Colour-vision simulation (Machado 2009, severity 1.0): smallest CIEDE2000 between channel colours 12.6 (CH1-CH3, deuteranopia), 15.4 (CH2-CH4, protanopia), 17.6 (CH2-CH4, deuteranopia); normal vision smallest 22.2 (CH1-CH3); CH1 and CH2 have equal L* (70).
4. Geometry (playing): non-empty row bands y 1-19 header chips, 21-22 selection bar, 24-39 knob labels, 41-76 knob bodies, 79-91 status, 93-140 music (playhead bar 93-94), 146-162 and 164-180 buttons, 186-220 keybed, 224-236 message line; gaps between bands 1, 1, 1, 2, 1, 5, 1, 5, 3 px. Button cell 38x17 at pitch 40x18 (2 px side gap, 1 px row gap); icons 13x13 (dice, 2 px top/bottom margin) and 13x11 (3 px). Music bar maximum height 44 / 40 / 36 / 32 px for 1 / 2 / 3 / 4 channels, hit strips 3 px high at pitch 4; an accent cap on the top-pitch bar touches the playhead white bar (0 px gap). Pending pill "P0n?" is 1 px above the playhead white bar and overlaps it in x for steps 1-3 only (row 92 stays empty, measured). Font cap heights at 35 cm: AF_S 11.1', AF_M 13.5', AF_L 25.8'; 2 px marks 2.5'; 1 px lines 1.2'.
5. Sweeps: 1572 renders of every ORDER algorithm (ACID EUCLID SNAKE STAGE ARP PATTERNS DEMO) x rows 1-3 x every value (row 3 = 4 x 128 values under the first algorithm, 4 values under the others): 0 of 84 knob entries fail (value ink always inside the tile; narrowest margins 3 px: ARP SHAPE "THUMB", ARP DIR "CHORD", PATTERNS PICK "ACD 10", WIRE "100" 3 px right); font is stable per knob except the deliberate zero word KEYS in AF_M on 6 DENS/HITS knobs; `algos_v10.check()` returns `[]`; value-list knobs: 9, with 3-7 dots, dot width 5-8 px. 546 state renders (15 channel sets with every selection, muted/OFF/all-rest/REC/playhead variants, every algorithm on every channel): 0 warnings, 0 exceptions.
6. Extensibility: v10.py drawing code names no algorithm; names appear at v10.py 29-38 (DEMO), 46-49 (fixtures), 391-405 (scenes); algos_v10.py 28-38 and 142. 19 stress algorithms registered: `check()` catches the label, value-text, format, callback and zero-word cases and the 8-char short name; it does not catch a value list of 14+ entries (dot width 1 px; tested 16 and 17) or 18+ entries (dot width 0; tested 20 and 25: the current-value dot has 0 channel-colour pixels); dot shapes are hollow-vs-filled only up to 10 entries (dot width >= 3), at 11-13 entries (dot width 2) current and inactive differ by colour only, and the 8-char short name it flags renders without any problem. Details in section 6.
7. Accuracy vs TSVs: 72 claims checked, 51 match, 5 mismatch, 16 partial / not shown / conflict / unverifiable. Mismatches: (a) turn_algo draws the new algorithm's header and knobs while the toast says "AT NEXT STEP" (knob_map: lands at the next step); (b) ROW 3 tiles are generic WIRE1-4 0..127, knob_map R1-R4 says the cell shows the device's name for the parameter; (c) REC lane has no write cursor (page_states REC: lane with the write cursor); (d) toast "MIDI OFF  CHN=SET": key_map CHN tap opens the CHAN page, SET is the GLO page; (e) button picture line 2 (hold sub-labels h:RST h:HARM h:HLP/MENU h:UNDO L, dice sub-label) is not drawn. Non-matching but not "mismatch": SELECT past the last channel adds a channel (v10 docstring / "+" chip) vs knob_map "clamped, no wrap" and no TSV row for adding; PRESETS toast fixed to "AT NEXT BAR" (stopped = 1.5 s after the last detent), "Pnn>" armed header state not drawn; no OCT- toast, no "LAST ROW: OCT- = ROW 1" toast, no REC PRESET-inert toast; HOME hold text follows the TSV, not owner decision 118 (HANDOFF/02_CONSTRAINTS s3); header lacks clock dot, tempo source and LENGTH; M1-M4 hold (PREVIEW) and CHN hold (RIG) not shown; dice absent-layer dimming is set only for DEMO (`load()` sets absent=() for all six real algorithms; the TSVs carry no per-algorithm layer-presence column); AlgoDesc.name is `char[8]` ("<= 6 chars") and PATTERNS is 8 characters.
8. Coverage of PERF control x gesture rows (137 rows: button_modes 30, key_map 48, knob_map 59): shown persistently 114 of 129 rows that have an action = 88 % (119 = 92 % counting hold hints / toasts). By source: buttons 12 of 22 (55 %), 17 of 22 (77 %) with hold hints/toast; keys 43 of 48 (90 %); knobs 59 of 59 (48 of those are the per-algorithm K/S label rows). 8 hold rows are "no hold role" (excluded). Rows shown nowhere (10): OCT- tap, OCT- hold (peek), OCT+ hold (peek), OCT- and OCT+ hold-both (update mode), M1-M4 hold (PREVIEW, 4 rows), CHN hold (RIG).
9. Firmware cost: 21 distinct icon bitmaps are drawn across the 28 scenes (397 B row-packed at 1 bpp, 320 B bit-packed); one more defined icon (`midi`) is never drawn. 12 flat colours are outside HI-CON tokens: CH1-CH4 (Okabe-Ito), 4 mix(CH,120/255) knob fills, WHITE #ffffff, PHC (80,80,80), DICE_BG (72,72,72), OFF_RED (220,40,40); HI-CON tokens drawn: DIM INK KEY MID RAISE REC SURF TEXT; HI-CON THEME, ACCENT, SEL, TINT, LINE, LANE, GRID never drawn. v10 hard-codes INK = black, equal to BG in HI-CON only (not under GREY).

Notes on method: text is captured by wrapping `el.P.text` (every string funnels into `Canvas.text_flags`), rects/icons by wrapping `el.P.rect`, `el.P.frame`, `el.P.art`; the 240x240 "CV_MAX" canvas warning printed by `render_state` comes from the tool's initial `begin()` and is not a v10 draw. Sweeps that need a particular classification (section 8) state it per row.

---



---

<!-- scripts/s1_repro.py -->
## 1 reproduction: CLI render (--out /tmp/claude-0/v10b) vs proposals/v10/out/v10_<scene>.png, pixel diff via mock.diff_images  (scripts/s1_repro.py)
playing          0 differing px []
two_channels     0 differing px []
one_channel      0 differing px []
sel_muted        0 differing px []
dice             0 differing px []
turning_k2       0 differing px []
hold_edit        0 differing px []
hold_home        0 differing px []
rec_armed        0 differing px []
midi_off         0 differing px []
channel_added    0 differing px []
midi_off_after   0 differing px []
pending_p02      0 differing px []
turn_select      0 differing px []
turn_algo        0 differing px []
turn_presets     0 differing px []
oct_row2         0 differing px []
row3_rig         0 differing px []
hold_glo         0 differing px []
hold_save        0 differing px []
rec_playing      0 differing px []
arp_row2         0 differing px []
demo_algo        0 differing px []
stage_heard      0 differing px []
sheet_row1_a     0 differing px []
sheet_row1_b     0 differing px []
sheet_row2_a     0 differing px []
sheet_row2_b     0 differing px []
TOTAL differing px 0


---

<!-- scripts/s2_text.py -->

## playing
| string | font | x | cap-y | advance w | fg | bg | contrast |
|---|---|---|---|---|---|---|---|
| `120` | M | 16 | 5 | 27 | TEXT(#fffaff) | BG(#000000) | 20.37 |
| `1` | M | 48 | 5 | 9 | BG(#000000) | CH1(#e69d00) | 9.19 |
| `2` | M | 62 | 5 | 9 | CH2(#52b6ee) | BG(#000000) | 9.28 |
| `3` | M | 76 | 5 | 9 | MID(#b4aeb4) | SURF(#292829) | 6.75 |
| `4` | M | 90 | 5 | 9 | CH4(#cd79a4) | BG(#000000) | 6.87 |
| `ACID` | M | 105 | 5 | 33 | CH1(#e69d00) | BG(#000000) | 9.19 |
| `2` | M | 168 | 5 | 9 | WHITE(#ffffff) | BG(#000000) | 21.00 |
| `MIDI` | S | 143 | 7 | 23 | MID(#b4aeb4) | BG(#000000) | 9.65 |
| `DENS` | M | 11 | 27 | 38 | MID(#b4aeb4) | SURF(#292829) | 6.75 |
| `SLIDE` | M | 70 | 27 | 39 | MID(#b4aeb4) | SURF(#292829) | 6.75 |
| `JUMP` | M | 130 | 27 | 40 | MID(#b4aeb4) | SURF(#292829) | 6.75 |
| `ACCNT` | M | 185 | 27 | 50 | MID(#b4aeb4) | SURF(#292829) | 6.75 |
| `12` | L | 12 | 48 | 36 | WHITE(#ffffff) | SURF(#292829) | 14.69 |
| `4` | L | 81 | 48 | 18 | WHITE(#ffffff) | SURF(#292829) | 14.69 |
| `5` | L | 141 | 48 | 18 | WHITE(#ffffff) | SURF(#292829) | 14.69 |
| `3` | L | 201 | 48 | 18 | WHITE(#ffffff) | SURF(#292829) | 14.69 |
| `P01` | M | 3 | 80 | 27 | TEXT(#fffaff) | BG(#000000) | 20.37 |
| `S` | S | 116 | 192 | 7 | TEXT(#fffaff) | RAISE(#414041) | 10.01 |
| `F` | S | 131 | 192 | 7 | TEXT(#fffaff) | RAISE(#414041) | 10.01 |
| `R` | S | 146 | 192 | 7 | TEXT(#fffaff) | RAISE(#414041) | 10.01 |
| `KEYS = STEPS` | M | 2 | 225 | 97 | TEXT(#fffaff) | BG(#000000) | 20.37 |
| `HOLD = EDIT` | M | 151 | 225 | 87 | MID(#b4aeb4) | BG(#000000) | 9.65 |
totals strings by font: {'M': 14, 'S': 4, 'L': 4}  chars: {'M': 56, 'S': 7, 'L': 5}  total strings 22
icons drawn (distinct x,y,bitmap): 18 ; button icons 12 ; other 6 ; lit pixels total 943
  non-button: play_s@3,6 9x9, x7@42,193 7x7, midi_s@87,194 7x5, left@178,193 5x7, right@193,193 5x7, rst@223,193 5x7
draw warnings (excl. the initial 240x240 begin): []

## dice
| string | font | x | cap-y | advance w | fg | bg | contrast |
|---|---|---|---|---|---|---|---|
| `120` | M | 16 | 5 | 27 | TEXT(#fffaff) | BG(#000000) | 20.37 |
| `NOTES` | M | 38 | 5 | 48 | BG(#000000) | WHITE(#ffffff) | 21.00 |
| `1` | M | 48 | 5 | 9 | BG(#000000) | CH1(#e69d00) | 9.19 |
| `2` | M | 62 | 5 | 9 | CH2(#52b6ee) | BG(#000000) | 9.28 |
| `+` | M | 76 | 5 | 9 | MID(#b4aeb4) | BG(#000000) | 9.65 |
| `4` | M | 90 | 5 | 9 | CH4(#cd79a4) | BG(#000000) | 6.87 |
| `ACID` | M | 105 | 5 | 33 | CH1(#e69d00) | BG(#000000) | 9.19 |
| `EDIT=UNDO` | M | 153 | 5 | 83 | BG(#000000) | WHITE(#ffffff) | 21.00 |
| `2` | M | 168 | 5 | 9 | WHITE(#ffffff) | BG(#000000) | 21.00 |
| `1` | M | 6 | 6 | 9 | BG(#000000) | CH1(#e69d00) | 9.19 |
| `MIDI` | S | 143 | 7 | 23 | MID(#b4aeb4) | BG(#000000) | 9.65 |
| `DENS` | M | 11 | 27 | 38 | MID(#b4aeb4) | SURF(#292829) | 6.75 |
| `SLIDE` | M | 70 | 27 | 39 | MID(#b4aeb4) | SURF(#292829) | 6.75 |
| `JUMP` | M | 130 | 27 | 40 | MID(#b4aeb4) | SURF(#292829) | 6.75 |
| `ACCNT` | M | 185 | 27 | 50 | MID(#b4aeb4) | SURF(#292829) | 6.75 |
| `12` | L | 12 | 48 | 36 | WHITE(#ffffff) | SURF(#292829) | 14.69 |
| `4` | L | 81 | 48 | 18 | WHITE(#ffffff) | SURF(#292829) | 14.69 |
| `5` | L | 141 | 48 | 18 | WHITE(#ffffff) | SURF(#292829) | 14.69 |
| `3` | L | 201 | 48 | 18 | WHITE(#ffffff) | SURF(#292829) | 14.69 |
| `P01` | M | 3 | 80 | 27 | TEXT(#fffaff) | BG(#000000) | 20.37 |
| `S` | S | 116 | 192 | 7 | TEXT(#fffaff) | RAISE(#414041) | 10.01 |
| `F` | S | 131 | 192 | 7 | TEXT(#fffaff) | RAISE(#414041) | 10.01 |
| `R` | S | 146 | 192 | 7 | TEXT(#fffaff) | RAISE(#414041) | 10.01 |
| `KEYS = STEPS` | M | 2 | 225 | 97 | TEXT(#fffaff) | BG(#000000) | 20.37 |
| `HOLD = EDIT` | M | 151 | 225 | 87 | MID(#b4aeb4) | BG(#000000) | 9.65 |
totals strings by font: {'M': 17, 'S': 4, 'L': 4}  chars: {'M': 71, 'S': 7, 'L': 5}  total strings 25
icons drawn (distinct x,y,bitmap): 18 ; button icons 12 ; other 6 ; lit pixels total 978
  non-button: play_s@3,6 9x9, midi_s@87,194 7x5, left@178,193 5x7, right@193,193 5x7, rst@223,193 5x7, dice_notes@21,5 13x13
draw warnings (excl. the initial 240x240 begin): []

## turn_select
| string | font | x | cap-y | advance w | fg | bg | contrast |
|---|---|---|---|---|---|---|---|
| `120` | M | 16 | 5 | 27 | TEXT(#fffaff) | BG(#000000) | 20.37 |
| `1` | M | 48 | 5 | 9 | CH1(#e69d00) | BG(#000000) | 9.19 |
| `CH2  EUCLID` | M | 55 | 5 | 87 | BG(#000000) | TEXT(#fffaff) | 20.37 |
| `2` | M | 62 | 5 | 9 | BG(#000000) | CH2(#52b6ee) | 9.28 |
| `+` | M | 76 | 5 | 9 | MID(#b4aeb4) | BG(#000000) | 9.65 |
| `4` | M | 90 | 5 | 9 | CH4(#cd79a4) | BG(#000000) | 6.87 |
| `EUCLID` | M | 105 | 5 | 51 | CH2(#52b6ee) | BG(#000000) | 9.28 |
| `10` | M | 186 | 5 | 18 | WHITE(#ffffff) | BG(#000000) | 21.00 |
| `SELECT` | S | 5 | 7 | 42 | TEXT(#fffaff) | BG(#000000) | 20.37 |
| `MIDI` | S | 161 | 7 | 23 | MID(#b4aeb4) | BG(#000000) | 9.65 |
| `HITS` | M | 14 | 27 | 32 | MID(#b4aeb4) | SURF(#292829) | 6.75 |
| `OFFSET` | M | 63 | 27 | 54 | MID(#b4aeb4) | SURF(#292829) | 6.75 |
| `RATCH` | M | 126 | 27 | 47 | MID(#b4aeb4) | SURF(#292829) | 6.75 |
| `ACCNT` | M | 185 | 27 | 50 | MID(#b4aeb4) | SURF(#292829) | 6.75 |
| `5` | L | 21 | 48 | 18 | WHITE(#ffffff) | SURF(#292829) | 14.69 |
| `0` | L | 81 | 48 | 18 | WHITE(#ffffff) | SURF(#292829) | 14.69 |
| `4` | L | 141 | 48 | 18 | WHITE(#ffffff) | SURF(#292829) | 14.69 |
| `2` | L | 201 | 48 | 18 | WHITE(#ffffff) | SURF(#292829) | 14.69 |
| `P01` | M | 3 | 80 | 27 | TEXT(#fffaff) | BG(#000000) | 20.37 |
| `S` | S | 116 | 192 | 7 | TEXT(#fffaff) | RAISE(#414041) | 10.01 |
| `F` | S | 131 | 192 | 7 | TEXT(#fffaff) | RAISE(#414041) | 10.01 |
| `R` | S | 146 | 192 | 7 | TEXT(#fffaff) | RAISE(#414041) | 10.01 |
| `KEYS = STEPS` | M | 2 | 225 | 97 | TEXT(#fffaff) | BG(#000000) | 20.37 |
| `HOLD = EDIT` | M | 151 | 225 | 87 | MID(#b4aeb4) | BG(#000000) | 9.65 |
totals strings by font: {'M': 15, 'S': 5, 'L': 4}  chars: {'M': 72, 'S': 13, 'L': 4}  total strings 24
icons drawn (distinct x,y,bitmap): 17 ; button icons 12 ; other 5 ; lit pixels total 910
  non-button: play_s@3,6 9x9, midi_s@87,194 7x5, left@178,193 5x7, right@193,193 5x7, rst@223,193 5x7
draw warnings (excl. the initial 240x240 begin): []

## hold_save
| string | font | x | cap-y | advance w | fg | bg | contrast |
|---|---|---|---|---|---|---|---|
| `120` | M | 16 | 5 | 27 | TEXT(#fffaff) | BG(#000000) | 20.37 |
| `HOLD SAVE: UNDO LOAD` | M | 22 | 5 | 169 | BG(#000000) | TEXT(#fffaff) | 20.37 |
| `1` | M | 48 | 5 | 9 | BG(#000000) | CH1(#e69d00) | 9.19 |
| `2` | M | 62 | 5 | 9 | CH2(#52b6ee) | BG(#000000) | 9.28 |
| `+` | M | 76 | 5 | 9 | MID(#b4aeb4) | BG(#000000) | 9.65 |
| `4` | M | 90 | 5 | 9 | CH4(#cd79a4) | BG(#000000) | 6.87 |
| `ACID` | M | 105 | 5 | 33 | CH1(#e69d00) | BG(#000000) | 9.19 |
| `2` | M | 168 | 5 | 9 | WHITE(#ffffff) | BG(#000000) | 21.00 |
| `MIDI` | S | 143 | 7 | 23 | MID(#b4aeb4) | BG(#000000) | 9.65 |
| `DENS` | M | 11 | 27 | 38 | MID(#b4aeb4) | SURF(#292829) | 6.75 |
| `SLIDE` | M | 70 | 27 | 39 | MID(#b4aeb4) | SURF(#292829) | 6.75 |
| `JUMP` | M | 130 | 27 | 40 | MID(#b4aeb4) | SURF(#292829) | 6.75 |
| `ACCNT` | M | 185 | 27 | 50 | MID(#b4aeb4) | SURF(#292829) | 6.75 |
| `12` | L | 12 | 48 | 36 | WHITE(#ffffff) | SURF(#292829) | 14.69 |
| `4` | L | 81 | 48 | 18 | WHITE(#ffffff) | SURF(#292829) | 14.69 |
| `5` | L | 141 | 48 | 18 | WHITE(#ffffff) | SURF(#292829) | 14.69 |
| `3` | L | 201 | 48 | 18 | WHITE(#ffffff) | SURF(#292829) | 14.69 |
| `P01` | M | 3 | 80 | 27 | TEXT(#fffaff) | BG(#000000) | 20.37 |
| `S` | S | 116 | 192 | 7 | TEXT(#fffaff) | RAISE(#414041) | 10.01 |
| `F` | S | 131 | 192 | 7 | TEXT(#fffaff) | RAISE(#414041) | 10.01 |
| `R` | S | 146 | 192 | 7 | TEXT(#fffaff) | RAISE(#414041) | 10.01 |
| `KEYS = STEPS` | M | 2 | 225 | 97 | TEXT(#fffaff) | BG(#000000) | 20.37 |
| `HOLD = EDIT` | M | 151 | 225 | 87 | MID(#b4aeb4) | BG(#000000) | 9.65 |
totals strings by font: {'M': 15, 'S': 4, 'L': 4}  chars: {'M': 76, 'S': 7, 'L': 5}  total strings 23
icons drawn (distinct x,y,bitmap): 18 ; button icons 12 ; other 6 ; lit pixels total 993
  non-button: play_s@3,6 9x9, midi_s@87,194 7x5, left@178,193 5x7, right@193,193 5x7, rst@223,193 5x7, save@4,4 13x11
draw warnings (excl. the initial 240x240 begin): []

## midi_off_after
| string | font | x | cap-y | advance w | fg | bg | contrast |
|---|---|---|---|---|---|---|---|
| `120` | M | 16 | 5 | 27 | TEXT(#fffaff) | BG(#000000) | 20.37 |
| `1` | M | 48 | 5 | 9 | CH1(#e69d00) | BG(#000000) | 9.19 |
| `2` | M | 62 | 5 | 9 | BG(#000000) | CH2(#52b6ee) | 9.28 |
| `+` | M | 76 | 5 | 9 | MID(#b4aeb4) | BG(#000000) | 9.65 |
| `EUCLID` | M | 105 | 5 | 51 | CH2(#52b6ee) | BG(#000000) | 9.28 |
| `OFF` | M | 189 | 5 | 27 | WHITE(#ffffff) | OFF_RED(#de2829) | 4.73 |
| `MIDI` | S | 163 | 7 | 23 | WHITE(#ffffff) | OFF_RED(#de2829) | 4.73 |
| `HITS` | M | 14 | 27 | 32 | MID(#b4aeb4) | SURF(#292829) | 6.75 |
| `OFFSET` | M | 63 | 27 | 54 | MID(#b4aeb4) | SURF(#292829) | 6.75 |
| `RATCH` | M | 126 | 27 | 47 | MID(#b4aeb4) | SURF(#292829) | 6.75 |
| `ACCNT` | M | 185 | 27 | 50 | MID(#b4aeb4) | SURF(#292829) | 6.75 |
| `5` | L | 21 | 48 | 18 | WHITE(#ffffff) | SURF(#292829) | 14.69 |
| `0` | L | 81 | 48 | 18 | WHITE(#ffffff) | SURF(#292829) | 14.69 |
| `4` | L | 141 | 48 | 18 | WHITE(#ffffff) | SURF(#292829) | 14.69 |
| `2` | L | 201 | 48 | 18 | WHITE(#ffffff) | SURF(#292829) | 14.69 |
| `P01` | M | 3 | 80 | 27 | TEXT(#fffaff) | BG(#000000) | 20.37 |
| `S` | S | 116 | 192 | 7 | TEXT(#fffaff) | RAISE(#414041) | 10.01 |
| `F` | S | 131 | 192 | 7 | TEXT(#fffaff) | RAISE(#414041) | 10.01 |
| `R` | S | 146 | 192 | 7 | TEXT(#fffaff) | RAISE(#414041) | 10.01 |
| `KEYS = STEPS` | M | 2 | 225 | 97 | TEXT(#fffaff) | BG(#000000) | 20.37 |
| `HOLD = EDIT` | M | 151 | 225 | 87 | MID(#b4aeb4) | BG(#000000) | 9.65 |
totals strings by font: {'M': 13, 'S': 4, 'L': 4}  chars: {'M': 61, 'S': 7, 'L': 4}  total strings 21
icons drawn (distinct x,y,bitmap): 17 ; button icons 12 ; other 5 ; lit pixels total 910
  non-button: play_s@3,6 9x9, midi_s@87,194 7x5, left@178,193 5x7, right@193,193 5x7, rst@223,193 5x7
draw warnings (excl. the initial 240x240 begin): []

## rec_playing
| string | font | x | cap-y | advance w | fg | bg | contrast |
|---|---|---|---|---|---|---|---|
| `120` | M | 16 | 5 | 27 | TEXT(#fffaff) | BG(#000000) | 20.37 |
| `1` | M | 48 | 5 | 9 | BG(#000000) | CH1(#e69d00) | 9.19 |
| `2` | M | 62 | 5 | 9 | CH2(#52b6ee) | BG(#000000) | 9.28 |
| `+` | M | 76 | 5 | 9 | MID(#b4aeb4) | BG(#000000) | 9.65 |
| `ACID` | M | 105 | 5 | 33 | CH1(#e69d00) | BG(#000000) | 9.19 |
| `2` | M | 168 | 5 | 9 | WHITE(#ffffff) | BG(#000000) | 21.00 |
| `MIDI` | S | 143 | 7 | 23 | MID(#b4aeb4) | BG(#000000) | 9.65 |
| `CURSOR` | M | 0 | 27 | 59 | MID(#b4aeb4) | SURF(#292829) | 6.75 |
| `ACC` | M | 75 | 27 | 30 | MID(#b4aeb4) | SURF(#292829) | 6.75 |
| `SLIDE` | M | 130 | 27 | 39 | MID(#b4aeb4) | SURF(#292829) | 6.75 |
| `OCT` | M | 195 | 27 | 30 | MID(#b4aeb4) | SURF(#292829) | 6.75 |
| `1` | L | 21 | 48 | 18 | WHITE(#ffffff) | SURF(#292829) | 14.69 |
| `OFF` | L | 64 | 48 | 51 | WHITE(#ffffff) | SURF(#292829) | 14.69 |
| `OFF` | L | 124 | 48 | 51 | WHITE(#ffffff) | SURF(#292829) | 14.69 |
| `0` | M | 205 | 50 | 9 | WHITE(#ffffff) | SURF(#292829) | 14.69 |
| `P01` | M | 3 | 80 | 27 | TEXT(#fffaff) | BG(#000000) | 20.37 |
| `PLAY KEYS TO WRITE` | M | 2 | 225 | 146 | TEXT(#fffaff) | BG(#000000) | 20.37 |
| `REC = DONE` | S | 171 | 226 | 67 | MID(#b4aeb4) | BG(#000000) | 9.65 |
totals strings by font: {'M': 13, 'S': 2, 'L': 3}  chars: {'M': 50, 'S': 14, 'L': 7}  total strings 18
icons drawn (distinct x,y,bitmap): 12 ; button icons 12 ; other 0 ; lit pixels total 838
  non-button: 
draw warnings (excl. the initial 240x240 begin): []


---

<!-- scripts/s3_contrast.py -->
## HI-CON tokens  (scripts/s3_contrast.py)
  bg #000000
  surf #292829
  text #fffaff
  theme #ffea00
  accent #00e6ff
  mid #b4aeb4
  dim #6a696a
  line #312c31
  sel #cdba00
  tint #201c00
  ink #000000
  rec #ff484a
  raise_ #414041
  key #c5c2c5
  lane #413c20
  grid #393439

## 3a text pairs, all 28 scenes (distinct fg/bg/font)  (scripts/s3_contrast.py)
| fg | bg | font | ratio | threshold | verdict | n strings | examples |
|---|---|---|---|---|---|---|---|
| WHITE(#ffffff) | OFF_RED(#de2829) | S | 4.73 | 4.5 | ok | 2 | MIDI |
| WHITE(#ffffff) | OFF_RED(#de2829) | M | 4.73 | 4.5 | ok | 2 | OFF |
| MID(#b4aeb4) | SURF(#292829) | M | 6.75 | 4.5 | ok | 154 | 3, ACC, ACCNT, BITS, CTONE |
| CH4(#cd79a4) | BG(#000000) | M | 6.87 | 4.5 | ok | 15 | 4 |
| WHITE(#ffffff) | ?(#6a4800) | L | 8.27 | 4.5 | ok | 12 | 10, 16, 4, 40 |
| BG(#000000) | CH1(#e69d00) | M | 9.19 | 4.5 | ok | 20 | 1 |
| CH1(#e69d00) | BG(#000000) | M | 9.19 | 4.5 | ok | 24 | 1, ACID, ARP, DEMO, SNAKE |
| CH2(#52b6ee) | BG(#000000) | M | 9.28 | 4.5 | ok | 23 | 2, EUCLID |
| BG(#000000) | CH2(#52b6ee) | M | 9.28 | 4.5 | ok | 6 | 2 |
| MID(#b4aeb4) | BG(#000000) | S | 9.65 | 4.5 | ok | 24 | MIDI, REC = DONE |
| MID(#b4aeb4) | BG(#000000) | M | 9.65 | 4.5 | ok | 44 | +, HOLD = EDIT |
| TEXT(#fffaff) | RAISE(#414041) | S | 10.01 | 4.5 | ok | 66 | F, R, S |
| WHITE(#ffffff) | SURF(#292829) | L | 14.69 | 4.5 | ok | 119 | 0, 1, 10, 12, 2 |
| WHITE(#ffffff) | SURF(#292829) | M | 14.69 | 4.5 | ok | 21 | 0, 16, 1X, 2 OCT, 48(32) |
| CH3(#f6e641) | BG(#000000) | M | 16.31 | 4.5 | ok | 1 | SNAKE |
| TEXT(#fffaff) | BG(#000000) | M | 20.37 | 4.5 | ok | 70 | 120, KEYS = STEPS, P01, PLAY KEYS TO WRITE |
| BG(#000000) | TEXT(#fffaff) | M | 20.37 | 4.5 | ok | 9 | CH2  EUCLID, HOLD EDIT: RESET, HOLD GLO: HARMONY, HOLD SAVE: UNDO LOAD, P02 AT NEXT BAR |
| TEXT(#fffaff) | BG(#000000) | S | 20.37 | 4.5 | ok | 5 | ALGO, OCT+, PRESETS, SELECT |
| WHITE(#ffffff) | BG(#000000) | M | 21.00 | 4.5 | ok | 22 | 10, 2, 3 |
| BG(#000000) | WHITE(#ffffff) | M | 21.00 | 4.5 | ok | 8 | ALGO=PICK, CHN=SET, EDIT=UNDO, MIDI OFF, NEW |

## 3b meaning-bearing non-text pairs (flag <3:1)  (scripts/s3_contrast.py)
| item | fg | bg | ratio | <3:1 |
|---|---|---|---|---|
| channel 1 colour on BG (header chip frame, music bar, hit strip) | CH1(#e69d00) | BG(#000000) | 9.19 |  |
| channel 1 colour on SURF (knob tile dots, keybed lane block) | CH1(#e69d00) | SURF(#292829) | 6.43 |  |
| channel 1 colour on PLAYHEAD column PHC | CH1(#e69d00) | PHC(#525052) | 3.50 |  |
| channel 1 header chip digit INK on filled chip | BG(#000000) | CH1(#e69d00) | 9.19 |  |
| channel 2 colour on BG (header chip frame, music bar, hit strip) | CH2(#52b6ee) | BG(#000000) | 9.28 |  |
| channel 2 colour on SURF (knob tile dots, keybed lane block) | CH2(#52b6ee) | SURF(#292829) | 6.49 |  |
| channel 2 colour on PLAYHEAD column PHC | CH2(#52b6ee) | PHC(#525052) | 3.53 |  |
| channel 2 header chip digit INK on filled chip | BG(#000000) | CH2(#52b6ee) | 9.28 |  |
| channel 3 colour on BG (header chip frame, music bar, hit strip) | CH3(#f6e641) | BG(#000000) | 16.31 |  |
| channel 3 colour on SURF (knob tile dots, keybed lane block) | CH3(#f6e641) | SURF(#292829) | 11.41 |  |
| channel 3 colour on PLAYHEAD column PHC | CH3(#f6e641) | PHC(#525052) | 6.21 |  |
| channel 3 header chip digit INK on filled chip | BG(#000000) | CH3(#f6e641) | 16.31 |  |
| channel 4 colour on BG (header chip frame, music bar, hit strip) | CH4(#cd79a4) | BG(#000000) | 6.87 |  |
| channel 4 colour on SURF (knob tile dots, keybed lane block) | CH4(#cd79a4) | SURF(#292829) | 4.80 |  |
| channel 4 colour on PLAYHEAD column PHC | CH4(#cd79a4) | PHC(#525052) | 2.61 | **FLAG** |
| channel 4 header chip digit INK on filled chip | BG(#000000) | CH4(#cd79a4) | 6.87 |  |
| hit strip dash (muted/off other channel) DIM on BG | DIM(#6a696a) | BG(#000000) | 3.84 |  |
| hollow muted bar frame MID on BG | MID(#b4aeb4) | BG(#000000) | 9.65 |  |
| hollow muted bar frame MID on PHC (playhead) | MID(#b4aeb4) | PHC(#525052) | 3.67 |  |
| hollow MIDI-OFF bar frame OFF_RED on BG | OFF_RED(#de2829) | BG(#000000) | 4.44 |  |
| hollow MIDI-OFF bar frame OFF_RED on PHC | OFF_RED(#de2829) | PHC(#525052) | 1.69 | **FLAG** |
| rest tick DIM (3x1) on BG | DIM(#6a696a) | BG(#000000) | 3.84 |  |
| OFF_RED cap on chip vs CH2 chip fill | OFF_RED(#de2829) | CH2(#52b6ee) | 2.09 | **FLAG** |
| OFF_RED cap on chip vs BG | OFF_RED(#de2829) | BG(#000000) | 4.44 |  |
| OFF_RED header chip vs BG | OFF_RED(#de2829) | BG(#000000) | 4.44 |  |
| OFF_RED cap vs CH1 / CH3 / CH4 chip (min over) | OFF_RED(#de2829) | CH4(#cd79a4) | 1.55 | **FLAG** |
| playhead column PHC vs BG | PHC(#525052) | BG(#000000) | 2.63 | **FLAG** |
| playhead white bar vs BG | WHITE(#ffffff) | BG(#000000) | 21.00 |  |
| playhead white bar vs PHC | WHITE(#ffffff) | PHC(#525052) | 7.99 |  |
| dice tile DICE_BG vs BG | DICE_BG(#4a484a) | BG(#000000) | 2.32 | **FLAG** |
| plain tile SURF vs BG | SURF(#292829) | BG(#000000) | 1.43 | **FLAG** |
| dice tile DICE_BG vs plain tile SURF | DICE_BG(#4a484a) | SURF(#292829) | 1.62 | **FLAG** |
| dice icon WHITE on DICE_BG | WHITE(#ffffff) | DICE_BG(#4a484a) | 9.06 |  |
| plain icon TEXT on SURF | TEXT(#fffaff) | SURF(#292829) | 14.25 |  |
| absent die DIM on DICE_BG | DIM(#6a696a) | DICE_BG(#4a484a) | 1.66 | **FLAG** |
| lit button INK on WHITE | BG(#000000) | WHITE(#ffffff) | 21.00 |  |
| lit button WHITE vs BG | WHITE(#ffffff) | BG(#000000) | 21.00 |  |
| REC icon T.REC on SURF | REC(#ff484a) | SURF(#292829) | 4.39 |  |
| REC armed icon vs BG | REC(#ff484a) | BG(#000000) | 6.28 |  |
| selection bar WHITE vs BG | WHITE(#ffffff) | BG(#000000) | 21.00 |  |
| top-bar message TEXT vs header BG | TEXT(#fffaff) | BG(#000000) | 20.37 |  |
| top-bar toast WHITE vs header BG | WHITE(#ffffff) | BG(#000000) | 21.00 |  |
| top-bar name pill INK on TEXT bar | BG(#000000) | TEXT(#fffaff) | 20.37 |  |
| hold progress bar INK on TEXT | BG(#000000) | TEXT(#fffaff) | 20.37 |  |
| hold progress OFF_RED (>=100%) on TEXT | OFF_RED(#de2829) | TEXT(#fffaff) | 4.59 |  |
| hold progress track TEXT vs BG | TEXT(#fffaff) | BG(#000000) | 20.37 |  |
| row marker active (CH1) vs inactive MID frame (on BG) | CH1(#e69d00) | MID(#b4aeb4) | 1.05 | **FLAG** |
| row marker inactive MID on BG | MID(#b4aeb4) | BG(#000000) | 9.65 |  |
| row marker active CH2 on BG | CH2(#52b6ee) | BG(#000000) | 9.28 |  |
| page dots active TEXT vs inactive MID | TEXT(#fffaff) | MID(#b4aeb4) | 2.11 | **FLAG** |
| page dots inactive MID on BG | MID(#b4aeb4) | BG(#000000) | 9.65 |  |
| knob fill (mix 120/255) vs empty SURF, ch1 | ?(#6a4800) | SURF(#292829) | 1.78 | **FLAG** |
| knob level edge ch1 full vs fill | CH1(#e69d00) | ?(#6a4800) | 3.62 |  |
| knob fill (mix 120/255) vs empty SURF, ch2 | ?(#20556a) | SURF(#292829) | 1.80 | **FLAG** |
| knob level edge ch2 full vs fill | CH2(#52b6ee) | ?(#20556a) | 3.61 |  |
| knob fill (mix 120/255) vs empty SURF, ch3 | ?(#736918) | SURF(#292829) | 2.63 | **FLAG** |
| knob level edge ch3 full vs fill | CH3(#f6e641) | ?(#736918) | 4.34 |  |
| knob fill (mix 120/255) vs empty SURF, ch4 | ?(#5a384a) | SURF(#292829) | 1.47 | **FLAG** |
| knob level edge ch4 full vs fill | CH4(#cd79a4) | ?(#5a384a) | 3.27 |  |
| knob fill WHITE(REC) vs empty | ?(#737573) | SURF(#292829) | 3.16 |  |
| knob level edge WHITE (REC) vs its fill | WHITE(#ffffff) | ?(#737573) | 4.64 |  |
| turning frame/ghost WHITE vs fill (ch1) | WHITE(#ffffff) | ?(#6a4800) | 8.27 |  |
| position dot current ch1 vs inactive DIM frame | CH1(#e69d00) | DIM(#6a696a) | 2.39 | **FLAG** |
| position dot inactive DIM on SURF | DIM(#6a696a) | SURF(#292829) | 2.69 | **FLAG** |
| knob tile SURF vs BG | SURF(#292829) | BG(#000000) | 1.43 | **FLAG** |
| keybed white key SURF vs BG | SURF(#292829) | BG(#000000) | 1.43 | **FLAG** |
| black key RAISE vs BG | RAISE(#414041) | BG(#000000) | 2.03 | **FLAG** |
| black key RAISE vs white key SURF | RAISE(#414041) | SURF(#292829) | 1.42 | **FLAG** |
| REC white key KEY vs BG | KEY(#c5c2c5) | BG(#000000) | 11.90 |  |
| REC black key RAISE vs KEY | RAISE(#414041) | KEY(#c5c2c5) | 5.85 |  |
| slide bridge ch1 vs BG | CH1(#e69d00) | BG(#000000) | 9.19 |  |
| accent cap WHITE vs BG | WHITE(#ffffff) | BG(#000000) | 21.00 |  |
| diced bar WHITE vs hatch ch1 | WHITE(#ffffff) | CH1(#e69d00) | 2.29 | **FLAG** |
| muted chip: MID digit on SURF | MID(#b4aeb4) | SURF(#292829) | 6.75 |  |
| muted chip diagonal MID on SURF | MID(#b4aeb4) | SURF(#292829) | 6.75 |  |
| muted chip SURF vs BG | SURF(#292829) | BG(#000000) | 1.43 | **FLAG** |
| muted x7 glyph MID on RAISE | MID(#b4aeb4) | RAISE(#414041) | 4.75 |  |
| pending pill INK on WHITE | BG(#000000) | WHITE(#ffffff) | 21.00 |  |
| pending pill WHITE vs BG | WHITE(#ffffff) | BG(#000000) | 21.00 |  |

## 3c CVD: Machado et al. 2009, severity 1.0, applied in linear sRGB; Lab D65; deltaE76 and CIEDE2000 (JND ~2.3)  (scripts/s3_contrast.py)

## normal
simulated: CH1 #e69d00, CH2 #52b6ee, CH3 #f6e641, CH4 #cd79a4
| pair | dE76 | dE2000 | lightness L* a / b |
|---|---|---|---|
| CH1-CH2 | 114.8 | 54.3 | 70 / 70 |
| CH1-CH3 | 35.7 | 22.2 | 70 / 90 |
| CH1-CH4 | 86.2 | 47.7 | 70 / 61 |
| CH2-CH3 | 115.1 | 58.6 | 70 / 90 |
| CH2-CH4 | 58.4 | 46.6 | 70 / 61 |
| CH3-CH4 | 103.5 | 61.7 | 90 / 61 |

## deuteranopia
simulated: CH1 #c9b310, CH2 #86a5ed, CH3 #ffe64d, CH4 #9699a2
| pair | dE76 | dE2000 | lightness L* a / b |
|---|---|---|---|
| CH1-CH2 | 113.0 | 54.9 | 73 / 68 |
| CH1-CH3 | 18.5 | 12.6 | 73 / 91 |
| CH1-CH4 | 78.8 | 34.4 | 73 / 63 |
| CH2-CH3 | 116.4 | 57.4 | 68 / 91 |
| CH2-CH4 | 35.2 | 17.6 | 68 / 63 |
| CH3-CH4 | 84.2 | 39.2 | 91 / 63 |

## protanopia
simulated: CH1 #b7a100, CH2 #9bb5f1, CH3 #fbde1f, CH4 #818ba6
| pair | dE76 | dE2000 | lightness L* a / b |
|---|---|---|---|
| CH1-CH2 | 102.6 | 51.3 | 66 / 74 |
| CH1-CH3 | 27.0 | 16.2 | 66 / 88 |
| CH1-CH4 | 85.4 | 42.1 | 66 / 58 |
| CH2-CH3 | 118.4 | 54.2 | 74 / 88 |
| CH2-CH4 | 23.6 | 15.4 | 74 / 58 |
| CH3-CH4 | 104.8 | 49.2 | 88 / 58 |


---

<!-- scripts/s4_geom.py -->
## 4 geometry (HI-CON, 240x240)  (scripts/s4_geom.py)

## non-empty row bands, scene playing: [(1, 19), (21, 22), (24, 39), (41, 76), (79, 91), (93, 140), (146, 162), (164, 180), (186, 220), (224, 236)]
   gaps between bands (px of pure BG): [1, 1, 1, 2, 1, 5, 1, 5, 3]

## non-empty row bands, scene one_channel: [(1, 19), (21, 22), (24, 39), (41, 76), (79, 91), (99, 139), (146, 162), (164, 180), (186, 218), (224, 236)]
   gaps between bands (px of pure BG): [1, 1, 1, 2, 7, 6, 1, 5, 5]

## non-empty row bands, scene rec_armed: [(1, 19), (21, 22), (24, 39), (41, 76), (79, 91), (99, 135), (138, 140), (146, 162), (164, 180), (186, 218), (224, 236)]
   gaps between bands (px of pure BG): [1, 1, 1, 2, 7, 2, 5, 1, 5, 5]

## non-empty row bands, scene dice: [(0, 22), (24, 39), (41, 76), (79, 91), (93, 140), (146, 162), (164, 180), (186, 220), (224, 236)]
   gaps between bands (px of pure BG): [1, 1, 2, 1, 5, 1, 5, 3]

## non-empty row bands, scene hold_glo: [(0, 22), (24, 39), (41, 76), (79, 91), (93, 140), (146, 162), (164, 180), (186, 220), (224, 236)]
   gaps between bands (px of pure BG): [1, 1, 2, 1, 5, 1, 5, 3]

## element y-ranges (inclusive), scene playing
header chips/bar: y 1..22
knob tiles: label tile y 24..39, body tile y 41..76, row gap between = 1 px, tile gap horizontally = 2 px (x step 60, w 58)
status line: text cap y 80..90 (AF_M cap 11); row markers y 82..88; page dots y 82..88; pending pill y 78..91 (h14)
music region: code y0=96 y1=141; playhead column y 95..140 (PHC), white bar y 93..94; bars end y 140 at most
buttons: row1 y 146..162, row2 y 164..180 (cell 38x17, pitch 40x18: 2 px gap horizontally, 1 px vertically), x 1..39 .. 201..239
keybed: white keys y 186..218 (h33, w14, pitch 15 = 1px gap), lane block y 207..218 (h12), playhead bar y 219..220; black keys y 186..205 (10x19 + 1px BG halo)
message line: M cap 11 at y 225..235 ; S cap 9 at y 226..234
top bar (toast/hold): y 0..21 full width; hold progress bar y 20..21 (2 px)
measured gaps (BG rows) in playing: header chips 1..19, selection bar 21..22, knob label 24..39, knob body 41..76, status 79..91, music 93..140, buttons 146..162 / 164..180, keybed 186..220, message line 224..236; gaps = [1,1,1,2,1,5,1,5,3] (see bands above); pending: see pill section

texts whose cap box crosses y=120 (playing): none
all 28 scenes: texts whose cap box straddles a firmware strip boundary (120 for screens; 116/232 for sheet scenes): none
rects spanning y=119/120 boundary (playing), type/w/h/colour:count -> {('rect', 13, 18, 'CH1(#e69d00)'): 2, ('rect', 13, 11, 'CH1(#e69d00)'): 2, ('rect', 2, 3, 'CH1(#e69d00)'): 1, ('rect', 15, 46, 'PHC(#525052)'): 1, ('rect', 13, 14, 'CH1(#e69d00)'): 2, ('rect', 13, 23, 'CH1(#e69d00)'): 1, ('rect', 13, 29, 'CH1(#e69d00)'): 1}

## buttons: cell 38x17 (x=c*40+1, y=146+r*18), 12 cells
| button | icon | w x h | margin L/R | margin T/B | lit px | stroke notes |
|---|---|---|---|---|---|---|
| FX | dice_all | 13x13 | 12/13 | 2/2 | 65 | TIGHT (<3 px margin T/B) |
| SCL | dice_notes | 13x13 | 12/13 | 2/2 | 68 | TIGHT (<3 px margin T/B) |
| ENV | dice_accnt | 13x13 | 12/13 | 2/2 | 75 | TIGHT (<3 px margin T/B) |
| LFO | dice_shape | 13x13 | 12/13 | 2/2 | 64 | TIGHT (<3 px margin T/B) |
| EDIT | undo | 13x11 | 12/13 | 3/3 | 26 |  |
| GLO | settings | 13x11 | 12/13 | 3/3 | 70 |  |
| HOME | home | 13x11 | 12/13 | 3/3 | 59 |  |
| SAVE | save | 13x11 | 12/13 | 3/3 | 83 |  |
| ARP | dice_artic | 13x13 | 12/13 | 2/2 | 71 | TIGHT (<3 px margin T/B) |
| SEQ | dice_hits | 13x13 | 12/13 | 2/2 | 74 | TIGHT (<3 px margin T/B) |
| PLAY | play | 13x11 | 12/13 | 3/3 | 72 |  |
| REC | rec_ring | 13x11 | 12/13 | 3/3 | 60 |  |

## music bars/strips per channel count (code: y0=96,y1=141, strip_h=3, pitch 4)
| channels | others | bh_max | pitched bar h range | single bar h | bar y-top (tallest) | strip rows y | strip bottom | accent cap top (tallest bar) | gap cap-top to playhead white bar (rows 93-94) |
|---|---|---|---|---|---|---|---|---|---|
| 1 | 0 | 44 | 6..41 | 8 | 99 | - | - | 95 | 0 |
| 2 | 1 | 40 | 6..37 | 8 | 99 | [138] | 140 | 95 | 0 |
| 3 | 2 | 36 | 6..33 | 8 | 99 | [134, 138] | 140 | 95 | 0 |
| 4 | 3 | 32 | 6..29 | 8 | 99 | [130, 134, 138] | 140 | 95 | 0 |
  (gap 0 = the accent cap on the top-pitch bar touches the playhead's white bar when that step is under the playhead; bars: w13 pitch15 -> 2 px gap; slide bridge 2x3; strip 13x3, dash 5x3)

## pending pill P0n? (y 78..91) vs playhead white bar (y 93..94) / column (y 95..)
pill P02?: x 1..40 (w 40), vertical gap to playhead white bar = 92-91 = 1 px (row 92); steps whose playhead column x-range overlaps the pill in x: [1, 2, 3]
pill P09?: x 1..40 (w 40), vertical gap to playhead white bar = 92-91 = 1 px (row 92); steps whose playhead column x-range overlaps the pill in x: [1, 2, 3]
pill P12?: x 1..40 (w 40), vertical gap to playhead white bar = 92-91 = 1 px (row 92); steps whose playhead column x-range overlaps the pill in x: [1, 2, 3]
the pill is 1 px above the playhead white bar for steps [1, 2, 3]; for all other steps no x-overlap.
measured per playhead step (step, non-BG pixel on row 92 under pill, white px on row 93 in x<40): [(1, False, 15), (2, False, 15), (3, False, 10), (4, False, 0), (5, False, 0), (6, False, 0), (7, False, 0), (8, False, 0), (9, False, 0), (10, False, 0), (11, False, 0), (12, False, 0), (13, False, 0), (14, False, 0), (15, False, 0), (16, False, 0)]

## rect pairs of different colours exactly 1 px apart (playing): (colourA, colourB, axis, minDimA, minDimB): count
   ('CH1', 'BG', 'v', 12, 12) 13
   ('CH1', 'SURF', 'h', 12, 14) 10
   ('SURF', 'CH1', 'h', 14, 12) 9
   ('CH2', 'DIM', 'v', 3, 3) 3
   ('DIM', 'CH4', 'v', 3, 3) 3
   ('DICE_BG', 'SURF', 'v', 17, 17) 2
   ('CH1', 'WHITE', 'v', 13, 2) 1
   ('WHITE', 'SURF', 'v', 2, 16) 1
   ('CH1', 'PHC', 'h', 11, 15) 1
   ('CH4', 'PHC', 'h', 3, 15) 1
   ('PHC', 'CH2', 'h', 15, 3) 1
   ('PHC', 'DIM', 'h', 15, 3) 1
(1px gaps are by construction: header chips 13w pitch 14; music-column/bar; hit strips pitch 4 h3; button rows pitch 18 h17; knob label->body; selection bar under chip; pending pill->playhead)


---

<!-- scripts/s4b_arcmin.py -->
## 4b visual size at 35 cm: 30 mm panel = 240 px -> 8 px/mm; arcmin = size_mm / 350 * 3438  (scripts/s4b_arcmin.py)
1 px = 0.125 mm = 1.23 arcmin

## measured ink heights of capital 'H' per font (text_ink), advance of 'M'
AF_S: cap ink height 9 px = 1.12 mm = 11.1 arcmin ; stroke (est) -- ; 'M' advance 10
AF_M: cap ink height 11 px = 1.38 mm = 13.5 arcmin ; stroke (est) -- ; 'M' advance 13
AF_L: cap ink height 21 px = 2.62 mm = 25.8 arcmin ; stroke (est) -- ; 'M' advance 24

## feature sizes used for meaning (px -> mm -> arcmin)
  hit strip height (3)                3 px  0.375 mm  3.7 arcmin
  rest tick (3x1) height              1 px  0.125 mm  1.2 arcmin
  accent cap height (2)               2 px  0.250 mm  2.5 arcmin
  slide bridge (2x3)                  2 px  0.250 mm  2.5 arcmin
  playhead white bar height (2)       2 px  0.250 mm  2.5 arcmin
  selection bar under chip (2)        2 px  0.250 mm  2.5 arcmin
  OFF_RED cap on chip (3 high)        3 px  0.375 mm  3.7 arcmin
  level edge (2 high x 7 wide)        2 px  0.250 mm  2.5 arcmin
  position dot (3 high)               3 px  0.375 mm  3.7 arcmin
  row marker (7x7)                    7 px  0.875 mm  8.6 arcmin
  page dot (9x7)                      7 px  0.875 mm  8.6 arcmin
  muted diagonal (1 px line)          1 px  0.125 mm  1.2 arcmin
  hollow bar frame (1 px line)        1 px  0.125 mm  1.2 arcmin
  bar gap (2)                         2 px  0.250 mm  2.5 arcmin
  button gap vertical (1)             1 px  0.125 mm  1.2 arcmin
  button gap horizontal (2)           2 px  0.250 mm  2.5 arcmin
  button cell (38x17) height         17 px  2.125 mm  20.9 arcmin
  white key gap (1)                   1 px  0.125 mm  1.2 arcmin
  hit-strip dash (5 wide)             5 px  0.625 mm  6.1 arcmin
  hit-strip (13 wide)                13 px  1.625 mm  16.0 arcmin
  icon 13x13                         13 px  1.625 mm  16.0 arcmin
  black-key glyph 7x7                 7 px  0.875 mm  8.6 arcmin
  x7 muted glyph                      7 px  0.875 mm  8.6 arcmin
  hold progress bar (2 high)          2 px  0.250 mm  2.5 arcmin
  header chip 13x19 width            13 px  1.625 mm  16.0 arcmin


---

<!-- scripts/s5_sweeps.py -->
## 5 sweeps (HI-CON)  (scripts/s5_sweeps.py)
algos_v10.ORDER = ['ACID', 'EUCLID', 'SNAKE', 'STAGE', 'ARP', 'PATTERNS', 'DEMO']

## algos_v10.check(text_w, AF_M, AF_L) -> [] (no problems)

## 5a every algorithm x row x knob x value (renders: all values of rows 1-2; row 3 = RIG_ROW 0..127 all values)
renders: 1572, 20.2s
| algo | row | K | label | #values | font(non-zero) | zero-word font | widest value (advance px) | min left margin to tile | max right overflow | stable & fits |
|---|---|---|---|---|---|---|---|---|---|---|
| ACID | 1 | 1 | DENS | 17 | L | +zero:M | KEYS (36) | 11 | -11 | yes |
| ACID | 1 | 2 | SLIDE | 9 | L |  | 0 (18) | 21 | -21 | yes |
| ACID | 1 | 3 | JUMP | 21 | L |  | 10 (36) | 13 | -12 | yes |
| ACID | 1 | 4 | ACCNT | 9 | L |  | 0 (18) | 21 | -21 | yes |
| ACID | 2 | 1 | OFFSET | 16 | L |  | 10 (36) | 14 | -12 | yes |
| ACID | 2 | 2 | PEDAL | 13 | L |  | 10 (36) | 14 | -12 | yes |
| ACID | 2 | 3 | PUNCH | 48 | L |  | 10 (36) | 12 | -12 | yes |
| ACID | 2 | 4 | DIR | 3 | M |  | PEND (38) | 10 | -10 | yes |
| ACID | 3 | 1 | WIRE1 | 128 | L |  | 100 (54) | 5 | -3 | yes |
| ACID | 3 | 2 | WIRE2 | 128 | L |  | 100 (54) | 5 | -3 | yes |
| ACID | 3 | 3 | WIRE3 | 128 | L |  | 100 (54) | 5 | -3 | yes |
| ACID | 3 | 4 | WIRE4 | 128 | L |  | 100 (54) | 5 | -3 | yes |
| EUCLID | 1 | 1 | HITS | 17 | L | +zero:M | KEYS (36) | 11 | -11 | yes |
| EUCLID | 1 | 2 | OFFSET | 16 | L |  | 10 (36) | 14 | -12 | yes |
| EUCLID | 1 | 3 | RATCH | 9 | L |  | 0 (18) | 21 | -21 | yes |
| EUCLID | 1 | 4 | ACCNT | 9 | L |  | 0 (18) | 21 | -21 | yes |
| EUCLID | 2 | 1 | LEN | 15 | L |  | 10 (36) | 14 | -12 | yes |
| EUCLID | 2 | 2 | SPAN | 25 | L |  | 10 (36) | 13 | -12 | yes |
| EUCLID | 2 | 3 | PUNCH | 48 | L |  | 10 (36) | 12 | -12 | yes |
| EUCLID | 2 | 4 | CURVE | 7 | M |  | SAW+ (41) | 8 | -9 | yes |
| EUCLID | 3 | 1 | WIRE1 | 4 | L |  | 100 (54) | 5 | -3 | yes |
| EUCLID | 3 | 2 | WIRE2 | 4 | L |  | 100 (54) | 5 | -3 | yes |
| EUCLID | 3 | 3 | WIRE3 | 4 | L |  | 100 (54) | 5 | -3 | yes |
| EUCLID | 3 | 4 | WIRE4 | 4 | L |  | 100 (54) | 5 | -3 | yes |
| SNAKE | 1 | 1 | DENS | 17 | L | +zero:M | KEYS (36) | 11 | -11 | yes |
| SNAKE | 1 | 2 | LEN | 16 | L |  | 10 (36) | 14 | -12 | yes |
| SNAKE | 1 | 3 | RANGE | 49 | L |  | 10 (36) | 12 | -12 | yes |
| SNAKE | 1 | 4 | PATH | 14 | L |  | 10 (36) | 14 | -12 | yes |
| SNAKE | 2 | 1 | OFFSET | 16 | L |  | 10 (36) | 14 | -12 | yes |
| SNAKE | 2 | 2 | GLEN | 16 | L |  | 10 (36) | 14 | -12 | yes |
| SNAKE | 2 | 3 | DYN | 11 | L |  | 10 (36) | 14 | -12 | yes |
| SNAKE | 2 | 4 | DIR | 3 | M |  | PEND (38) | 10 | -10 | yes |
| SNAKE | 3 | 1 | WIRE1 | 4 | L |  | 100 (54) | 5 | -3 | yes |
| SNAKE | 3 | 2 | WIRE2 | 4 | L |  | 100 (54) | 5 | -3 | yes |
| SNAKE | 3 | 3 | WIRE3 | 4 | L |  | 100 (54) | 5 | -3 | yes |
| SNAKE | 3 | 4 | WIRE4 | 4 | L |  | 100 (54) | 5 | -3 | yes |
| STAGE | 1 | 1 | DENS | 9 | L | +zero:M | KEYS (36) | 11 | -11 | yes |
| STAGE | 1 | 2 | TOTAL | 57 | M |  | 10 (18) | 20 | -20 | yes |
| STAGE | 1 | 3 | DIR | 3 | M |  | PEND (38) | 10 | -10 | yes |
| STAGE | 1 | 4 | RATCH | 9 | L |  | 0 (18) | 21 | -21 | yes |
| STAGE | 2 | 1 | OFFSET | 64 | L |  | 10 (36) | 12 | -12 | yes |
| STAGE | 2 | 2 | RDIV | 4 | L |  | 1 (18) | 21 | -21 | yes |
| STAGE | 2 | 3 | DYN | 11 | L |  | 10 (36) | 14 | -12 | yes |
| STAGE | 2 | 4 | RANGE | 37 | L |  | 10 (36) | 12 | -12 | yes |
| STAGE | 3 | 1 | WIRE1 | 4 | L |  | 100 (54) | 5 | -3 | yes |
| STAGE | 3 | 2 | WIRE2 | 4 | L |  | 100 (54) | 5 | -3 | yes |
| STAGE | 3 | 3 | WIRE3 | 4 | L |  | 100 (54) | 5 | -3 | yes |
| STAGE | 3 | 4 | WIRE4 | 4 | L |  | 100 (54) | 5 | -3 | yes |
| ARP | 1 | 1 | DENS | 17 | L | +zero:M | KEYS (36) | 11 | -11 | yes |
| ARP | 1 | 2 | SHAPE | 6 | M |  | THUMB (52) | 3 | -3 | yes |
| ARP | 1 | 3 | SPAN | 4 | M |  | 1 OCT (43) | 7 | -8 | yes |
| ARP | 1 | 4 | RATE | 6 | M |  | 1/4 (23) | 18 | -18 | yes |
| ARP | 2 | 1 | OFFSET | 16 | L |  | 10 (36) | 14 | -12 | yes |
| ARP | 2 | 2 | LEGATO | 9 | L |  | 0 (18) | 21 | -21 | yes |
| ARP | 2 | 3 | DYN | 11 | L |  | 10 (36) | 14 | -12 | yes |
| ARP | 2 | 4 | DIR | 6 | M |  | CHORD (51) | 3 | -4 | yes |
| ARP | 3 | 1 | WIRE1 | 4 | L |  | 100 (54) | 5 | -3 | yes |
| ARP | 3 | 2 | WIRE2 | 4 | L |  | 100 (54) | 5 | -3 | yes |
| ARP | 3 | 3 | WIRE3 | 4 | L |  | 100 (54) | 5 | -3 | yes |
| ARP | 3 | 4 | WIRE4 | 4 | L |  | 100 (54) | 5 | -3 | yes |
| PATTERNS | 1 | 1 | DENS | 17 | L | +zero:M | KEYS (36) | 11 | -11 | yes |
| PATTERNS | 1 | 2 | VARY | 9 | L |  | 0 (18) | 21 | -21 | yes |
| PATTERNS | 1 | 3 | SPAN | 4 | L |  | 1 (18) | 21 | -21 | yes |
| PATTERNS | 1 | 4 | ACCNT | 9 | L |  | 0 (18) | 21 | -21 | yes |
| PATTERNS | 2 | 1 | PICK | 120 | M |  | ACD 10 (51) | 3 | -4 | yes |
| PATTERNS | 2 | 2 | SLIDE | 9 | L |  | 0 (18) | 21 | -21 | yes |
| PATTERNS | 2 | 3 | DYN | 15 | L |  | 10 (36) | 14 | -12 | yes |
| PATTERNS | 2 | 4 | CTONE | 9 | L |  | 0 (18) | 21 | -21 | yes |
| PATTERNS | 3 | 1 | WIRE1 | 4 | L |  | 100 (54) | 5 | -3 | yes |
| PATTERNS | 3 | 2 | WIRE2 | 4 | L |  | 100 (54) | 5 | -3 | yes |
| PATTERNS | 3 | 3 | WIRE3 | 4 | L |  | 100 (54) | 5 | -3 | yes |
| PATTERNS | 3 | 4 | WIRE4 | 4 | L |  | 100 (54) | 5 | -3 | yes |
| DEMO | 1 | 1 | LOCK | 17 | L |  | 10 (36) | 14 | -12 | yes |
| DEMO | 1 | 2 | BITS | 16 | L |  | 10 (36) | 14 | -12 | yes |
| DEMO | 1 | 3 | SCALE | 4 | M |  | PENT (37) | 10 | -11 | yes |
| DEMO | 1 | 4 | SPRD | 4 | M |  | 1 OCT (43) | 7 | -8 | yes |
| DEMO | 2 | 1 | OFFSET | 16 | L |  | 10 (36) | 14 | -12 | yes |
| DEMO | 2 | 2 | GATE | 8 | L |  | 1 (18) | 21 | -21 | yes |
| DEMO | 2 | 3 | DYN | 11 | L |  | 10 (36) | 14 | -12 | yes |
| DEMO | 2 | 4 | DIR | 3 | M |  | PEND (38) | 10 | -10 | yes |
| DEMO | 3 | 1 | WIRE1 | 4 | L |  | 100 (54) | 5 | -3 | yes |
| DEMO | 3 | 2 | WIRE2 | 4 | L |  | 100 (54) | 5 | -3 | yes |
| DEMO | 3 | 3 | WIRE3 | 4 | L |  | 100 (54) | 5 | -3 | yes |
| DEMO | 3 | 4 | WIRE4 | 4 | L |  | 100 (54) | 5 | -3 | yes |

issues found by the sweep (0):
knobs failing: 0 of 84

## value_font decisions (AF_L for whole knob when widest non-zero value <=54 px and only digits/caps/space)
  knobs drawn in AF_M although all values are digits/caps (width forced):  [('ACID', 2, 'DIR', 'PEND', 72), ('SNAKE', 2, 'DIR', 'PEND', 72), ('STAGE', 1, 'DIR', 'PEND', 72), ('ARP', 1, 'SHAPE', 'THUMB', 98), ('ARP', 1, 'SPAN', '1 OCT', 82), ('ARP', 2, 'DIR', 'CHORD', 97), ('PATTERNS', 2, 'PICK', 'ACD 10', 99), ('DEMO', 1, 'SCALE', 'PENT', 70), ('DEMO', 1, 'SPRD', '1 OCT', 82), ('DEMO', 2, 'DIR', 'PEND', 72)]
  knobs drawn in AF_M for other reasons (lowercase/punct in widest value):  [('EUCLID', 2, 'CURVE', 'SAW+'), ('STAGE', 1, 'TOTAL', '64(63)'), ('ARP', 1, 'RATE', '1/4')]

## 5b value-list knobs (position dots): n dots, dot width dw=min(8,(50-(n-1)*2)//n), total width, tile 58
| algo | row | label | n | dw | total w | dot y 47..49 |
|---|---|---|---|---|---|---|
| ACID | 2 | DIR | 3 | 8 | 28 |  |
| EUCLID | 2 | CURVE | 7 | 5 | 47 |  |
| SNAKE | 2 | DIR | 3 | 8 | 28 |  |
| STAGE | 1 | DIR | 3 | 8 | 28 |  |
| ARP | 1 | SHAPE | 6 | 6 | 46 |  |
| ARP | 1 | RATE | 6 | 6 | 46 |  |
| ARP | 2 | DIR | 6 | 6 | 46 |  |
| DEMO | 1 | SCALE | 4 | 8 | 38 |  |
| DEMO | 2 | DIR | 3 | 8 | 28 |  |
distribution of list lengths: {3: 4, 4: 1, 6: 3, 7: 1}
lists with n>=2 whose dw<2: [14, 15, 16] (first n with dw<2; dw<=0 from n=18)

## 5c per-algorithm header/toast widths
| algo | short | AF_M w | header 'MIDI n' ends x | MIDI OFF chip ends x | ALGO toast end x (`<NAME> AT NEXT STEP`) | SELECT toast end x (`CH2  <NAME>`) |
|---|---|---|---|---|---|---|
| ACID | ACID | 33 | 186 | 202 | 183 | 131 |
| EUCLID | EUCLID | 51 | 204 | 220 | 200 | 148 |
| SNAKE | SNAKE | 47 | 200 | 216 | 197 | 145 |
| STAGE | STAGE | 45 | 198 | 214 | 195 | 143 |
| ARP | ARP | 28 | 181 | 197 | 178 | 125 |
| PATTERNS | PATT | 35 | 188 | 204 | 221 | 169 |
| DEMO | DEMO | 42 | 195 | 211 | 192 | 140 |

## 5d channel-set / state sweep
subsets of used channels: 15 ; renders: 546 ; renders with warnings or exceptions: 0 ; time 26.2s


---

<!-- scripts/s5b_extra.py -->
## 5e extra: header chip layout for every channel set; STAGE TOTAL with 'heard'; PATTERNS PICK  (scripts/s5b_extra.py)
| used | '+' drawn at slot (x) | chip slots drawn | note |
|---|---|---|---|
| [0] | [62] | [48] |  |
| [1] | [48] | [62] | non-contiguous: empty slot(s) [0, 2, 3] |
| [2] | [48] | [76] | non-contiguous: empty slot(s) [0, 1, 3] |
| [3] | [48] | [90] | non-contiguous: empty slot(s) [0, 1, 2] |
| [0, 1] | [76] | [48, 62] |  |
| [0, 2] | [62] | [48, 76] | non-contiguous: empty slot(s) [1, 3] |
| [0, 3] | [62] | [48, 90] | non-contiguous: empty slot(s) [1, 2] |
| [1, 2] | [48] | [62, 76] | non-contiguous: empty slot(s) [0, 3] |
| [1, 3] | [48] | [62, 90] | non-contiguous: empty slot(s) [0, 2] |
| [2, 3] | [48] | [76, 90] | non-contiguous: empty slot(s) [0, 1] |
| [0, 1, 2] | [90] | [48, 62, 76] |  |
| [0, 1, 3] | [76] | [48, 62, 90] | non-contiguous: empty slot(s) [2] |
| [0, 2, 3] | [62] | [48, 76, 90] | non-contiguous: empty slot(s) [1] |
| [1, 2, 3] | [48] | [62, 76, 90] | non-contiguous: empty slot(s) [0] |
| [0, 1, 2, 3] | none | [48, 62, 76, 90] |  |

STAGE TOTAL 8..64 x heard in {none, v-1, v}: widest text advance 46 px; value ink leaving tile 2: none
PATTERNS PICK 1..120: widest 'ACD 10' 51 px in AF_M


---

<!-- scripts/s6_extend.py -->
## 6 extensibility  (scripts/s6_extend.py)

## 6a where v10.py / algos_v10.py name individual algorithms (grep, line numbers)
v10.py:29-38   A.register("DEMO", ...) demo algorithm with its own rows; absent=("ACCNT",) at line 38
v10.py:46-49   CHANS fixture: algo names ACID / EUCLID / SNAKE / STAGE (data, not drawing code)
v10.py:391-405 scene table: names SNAKE (392), EUCLID (391), ARP (399), DEMO (400), STAGE (401), sheet lists (402-405)
v10.py drawing functions (header 93, knob_row 113, music 191/205, buttons 232) read A.ALGOS[...] fields only: short, rows, lane=="single", absent
algos_v10.py:28-34  patt_pick (PATTERNS) and stage_total (STAGE) value callbacks; :36-38 UI_EXTRA keys PATTERNS (short PATT, PICK fn), EUCLID (lane single), STAGE (TOTAL fn); :142 ORDER list of the six + DEMO appended by register()
algos_v10.py:93  load() sets absent=() for every TSV-parsed algorithm (the dimmed-die hint is never read from a TSV; only DEMO sets it, v10.py:38)
firmware sq_types.h: AlgoDesc.layer[SQ_NLAYER=5] n=0 -> absent, sub[5][8] '-' ; SQ_NALGO 6; AlgoDesc.name[8] (comment '<= 6 chars')

## 6b registered test algorithms: one knob (K1 row 1) stressed each; check() output and render sweep
A.check() returned 8 problem(s):
   check> X_LONGLBL LONGLABEL: label too wide
   check> X_LBL7 ABCDEFG: label too wide
   check> X_WIDELBL MMMMMM: label too wide
   check> X_LONGVAL VAL: value 'SEVENTEEN' too wide even in AF_M
   check> X_FMT FMT: value '10 SEMIS' too wide even in AF_M
   check> X_FN FNV: value 'VERY LONG VALUE 0' too wide even in AF_M
   check> X_SHORT: short name 'LONGNAME' does not fit the header
   check> X_ZERO ZRO: value 'SILENTLY' too wide even in AF_M

| case | stressed property | check() flags it | render: warnings / exception | value ink vs tile (K1) | verdict |
|---|---|---|---|---|---|
| X_LONGLBL | label 9 chars (83 px ink, tile 58) | yes: X_LONGLBL LONGLABEL: label too wide | text 'LONGLABEL' is cut by the canvas (ink -12,26..71,39) | L 21 / R -21 font L | BREAKS |
| X_LBL7 | label 7 chars (67 px) | yes: X_LBL7 ABCDEFG: label too wide | text 'ABCDEFG' is cut by the canvas (ink -4,26..63,39) | L 21 / R -21 font L | BREAKS |
| X_WIDELBL | label 6 wide chars (77 px) | yes: X_WIDELBL MMMMMM: label too wide | text 'MMMMMM' is cut by the canvas (ink -9,27..68,38) | L 21 / R -21 font L | BREAKS |
| X_LONGVAL | list value 'SEVENTEEN' (9 chars); dot width dw=8; pixels on dot row y=72: 14 non-SURF, 8 in channel colour (current dot) | yes: X_LONGVAL VAL: value 'SEVENTEEN' too wide even in AF_M | text 'SEVENTEEN' is cut by the canvas (ink -12,49..70,62) | L -13 / R +11 font M | BREAKS |
| X_FMT | fmt '%d SEMIS' | yes: X_FMT FMT: value '10 SEMIS' too wide even in AF_M | text '94 SEMIS' is cut by the canvas (ink -2,49..62,62); text '95 SEMIS' is cut by the can | L -3 / R +3 font M | BREAKS |
| X_LIST20 | 20-entry list; dot width dw=0; pixels on dot row y=72: 38 non-SURF, 0 in channel colour (current dot) | no | none | L 14 / R -15 font M | BREAKS and check() silent |
| X_LIST8 | 8-entry list; dot width dw=4; pixels on dot row y=72: 18 non-SURF, 4 in channel colour (current dot) | no | none | L 10 / R -12 font L | ok |
| X_LIST16 | 16-entry list; dot width dw=1; pixels on dot row y=72: 16 non-SURF, 1 in channel colour (current dot) | no | none | L 15 / R -15 font M | BREAKS and check() silent |
| X_LIST17 | 17-entry list; dot width dw=1; pixels on dot row y=72: 17 non-SURF, 1 in channel colour (current dot) | no | none | L 15 / R -15 font M | BREAKS and check() silent |
| X_LIST25 | 25-entry list; dot width dw=0; pixels on dot row y=72: 48 non-SURF, 0 in channel colour (current dot) | no | none | L 15 / R -15 font M | BREAKS and check() silent |
| X_FN | callback returns 17-char string | yes: X_FN FNV: value 'VERY LONG VALUE 0' too wide even in AF_M | text 'VERY LONG VALUE 0' is cut by the canvas (ink -40,49..100,62); text 'VERY LONG VALUE  | L -41 / R +41 font M | BREAKS |
| X_SHORT | short name 'LONGNAME' (8 chars; header ends x=186) | yes: X_SHORT: short name 'LONGNAME' does not fit the header | none | L 21 / R -21 font L | ok |
| X_NEG | range -20..20 | no | none | L 17 / R -17 font M | ok |
| X_BIG | range 0..9999 (4 digits) | no | none | L 11 / R -11 font M | ok |
| X_LOWER | lowercase value names; dot width dw=8; pixels on dot row y=72: 12 non-SURF, 8 in channel colour (current dot) | no | none | L 17 / R -17 font M | ok |
| X_ONEVAL | min == max | no | none | L 22 / R -21 font L | ok |
| X_BADLANE | lane='chord' (unknown) | no | none | L 21 / R -21 font L | ok |
| X_ABSENT | absent=('ZZZ',) (unknown layer) | no | none | L 21 / R -21 font L | ok |
| X_ZERO | zero word 'SILENTLY' (8 chars) | yes: X_ZERO ZRO: value 'SILENTLY' too wide even in AF_M | text 'SILENTLY' is cut by the canvas (ink -3,49..62,62) | L -4 / R +3 font L/M | BREAKS |

## 6c summary of what check() does NOT test (derived above)
- number of list entries / dot width (position dots): X_LIST20 gives dw=0; check() silent: True
- header room for the MIDI OFF chip and the ALGO toast with the FULL name; check() tests only AF_M width of 'short' <= 60 and len(short) <= 6
- value font consistency / the rendered position of the value over the knob meter
- 'lane' value, 'absent' layer names, rows with fewer than 4 knobs are only partly tested (missing knob None is tested)


---

<!-- scripts/s7_accuracy.py -->
## 7 accuracy vs TSVs: 72 claims checked; 51 MATCH; 5 MISMATCH; 16 other (partial / not shown / conflict / unverifiable)  (scripts/s7_accuracy.py)
| # | area | v10 | TSV / source | verdict | note |
|---|---|---|---|---|---|
| 1 | button grid order (2x6) | FX SCL ENV LFO EDIT GLO HOME SAVE ARP SEQ PLAY REC | HANDOFF/02_CONSTRAINTS.md s2: row1 FX SCL ENV LFO EDIT GLO, row2 HOME SAVE ARP SEQ PLAY/STOP REC | MATCH |  |
| 2 | PERF FX tap label | role ALL (icon dice_all) | button_modes PERF B_FX tap label ALL | MATCH |  |
| 3 | PERF FX hold | no hold role | hold: no hold role: acted once on press | MATCH |  |
| 4 | PERF SCL tap label | role NOTES (icon dice_notes) | button_modes PERF B_SCL tap label NOTES | MATCH |  |
| 5 | PERF SCL hold | no hold role | hold: no hold role: acted once on press | MATCH |  |
| 6 | PERF ENV tap label | role ACCNT (icon dice_accnt) | button_modes PERF B_ENV tap label ACCNT | MATCH |  |
| 7 | PERF ENV hold | no hold role | hold: no hold role: acted once on press | MATCH |  |
| 8 | PERF LFO tap label | role SHAPE (icon dice_shape) | button_modes PERF B_LFO tap label SHAPE | MATCH |  |
| 9 | PERF LFO hold | no hold role | hold: no hold role: acted once on press | MATCH |  |
| 10 | PERF EDIT tap label | role UNDO (icon undo) | button_modes PERF B_EDIT tap label UNDO | MATCH |  |
| 11 | PERF EDIT hold label | hold role RESET | hold label RESET | MATCH |  |
| 12 | PERF GLO tap label | role SET (icon settings) | button_modes PERF B_GLO tap label SET | MATCH |  |
| 13 | PERF GLO hold label | hold role HARM | hold label HARM | MATCH |  |
| 14 | PERF HOME tap label | role HOME (icon home) | button_modes PERF B_HOME tap label HOME | MATCH |  |
| 15 | PERF HOME hold label | hold role HLP/MN | hold label HLP/MN | MATCH |  |
| 16 | PERF SAVE tap label | role SAVE (icon save) | button_modes PERF B_SAVE tap label SAVE | MATCH |  |
| 17 | PERF SAVE hold label | hold role UNDO L | hold label UNDO L | MATCH |  |
| 18 | PERF ARP tap label | role ARTIC (icon dice_artic) | button_modes PERF B_ARP tap label ARTIC | MATCH |  |
| 19 | PERF ARP hold | no hold role | hold: no hold role: acted once on press | MATCH |  |
| 20 | PERF SEQ tap label | role HITS (icon dice_hits) | button_modes PERF B_SEQ tap label HITS | MATCH |  |
| 21 | PERF SEQ hold | no hold role | hold: no hold role: acted once on press | MATCH |  |
| 22 | PERF PLAY tap label | role PLAY (icon play) | button_modes PERF B_PLAY tap label PLAY | MATCH |  |
| 23 | PERF PLAY hold | no hold role | hold: no hold role: acted once on press | MATCH |  |
| 24 | PERF REC tap label | role REC (icon rec_ring) | button_modes PERF B_REC tap label REC | MATCH |  |
| 25 | PERF REC hold | no hold role | hold: no hold role: acted once on press | MATCH |  |
| 26 | which buttons are dice | FX,SCL,ENV,LFO,ARP,SEQ | 02_CONSTRAINTS: FX SCL ENV LFO ARP SEQ = six dice | MATCH |  |
| 27 | black key at x=15 (F#3) | M1 | key_map PERF KEY_B..F#3 label M1 (gestures: tap/hold) | MATCH |  |
| 28 | black key at x=30 (G#3) | M2 | key_map PERF KEY_B..G#3 label M2 (gestures: tap/hold) | MATCH |  |
| 29 | black key at x=45 (A#3) | M3 | key_map PERF KEY_B..A#3 label M3 (gestures: tap/hold) | MATCH |  |
| 30 | black key at x=75 (C#4) | M4 | key_map PERF KEY_B..C#4 label M4 (gestures: tap/hold) | MATCH |  |
| 31 | black key at x=90 (D#4) | CHN | key_map PERF KEY_B..D#4 label CHN (gestures: tap/hold) | MATCH |  |
| 32 | black key at x=120 (F#4) | SOL | key_map PERF KEY_B..F#4 label SOL (gestures: hold) | MATCH |  |
| 33 | black key at x=135 (G#4) | FIL | key_map PERF KEY_B..G#4 label FIL (gestures: hold) | MATCH |  |
| 34 | black key at x=150 (A#4) | ROL | key_map PERF KEY_B..A#4 label ROL (gestures: hold) | MATCH |  |
| 35 | black key at x=180 (C#5) | PG- | key_map PERF KEY_B..C#5 label PG- (gestures: tap) | MATCH |  |
| 36 | black key at x=195 (D#5) | PG+ | key_map PERF KEY_B..D#5 label PG+ (gestures: tap) | MATCH |  |
| 37 | black key at x=225 (F#5) | RST | key_map PERF KEY_B..F#5 label RST (gestures: tap) | MATCH |  |
| 38 | M1..M4 glyph | channel-colour 6x7 block if channel used; x7 glyph (MID) if muted; nothing if unused | key_map: tap = mute/unmute CHn; hold = PREVIEW | shown: tap-state only | hold PREVIEW (M1..M4) has no mark; unused-channel key is blank though tap still mutes CHn |
| 39 | CHN glyph | midi_s DIN 7x5 icon | key_map: tap opens CHAN page (LENGTH MIDI SOURCE FOLLOW); hold = RIG | partial | icon says MIDI; page also holds LENGTH SOURCE FOLLOW; RIG hold not shown |
| 40 | hold EDIT hint | HOLD EDIT: RESET | B_EDIT hold: '0.4 s: open RESET (latched, one shot)' | MATCH | text 'HOLD EDIT: RESET' = label RESET; fill at 60 % = 0.24 s of 0.4 s |
| 41 | hold GLO hint | HOLD GLO: HARMONY | B_GLO hold: '0.4 s: open HARMONY' | MATCH | label HARM spelled HARMONY; 60 % = 0.24 s of 0.4 s |
| 42 | hold SAVE hint | HOLD SAVE: UNDO LOAD | B_SAVE hold: '0.7 s: undo the last preset load...; while a slot is pending: cancel it instead' | MATCH (partial) | pending-slot variant 'cancel it' has no hint text; 80 % = 0.56 s of 0.7 s; no TSV time for hint onset except HOME |
| 43 | hold HOME hint | TOUCH=HELP  LET GO=MENU | B_HOME hold: 'from 0.25 s the strip reads HOME: touch=HELP  let go=MENU' | MATCH (wording) | drops the 'HOME:' prefix and upper-cases; 100 % bar = 0.7 s, drawn OFF_RED |
| 44 | HOME hold semantics | TOUCH=HELP  LET GO=MENU | HANDOFF/02_CONSTRAINTS s3: owner decision 118 changes HOME hold to 'map while held, keep holding to escalate to MENU' (TSV not updated) | TSV-consistent, decision-118-inconsistent | v10 follows the TSV text |
| 45 | SELECT toast | SELECT | CH2  EUCLID | knob_map PERF SELECT: 'turn: select channel CH1..CH4 (clamped, no wrap)', label 'CH n' | MATCH | adds the algorithm name; 'CH2' matches label form |
| 46 | ALGO toast | ALGO | SNAKE AT NEXT STEP | knob_map PERF ALGO: 'lands at the next step; morphs and seeds kept' | MATCH | wording matches |
| 47 | ALGO scene state | header ['SNAKE']; knob labels ['DENS', 'LEN', 'RANGE', 'PATH'] (toast says AT NEXT STEP) | lands at the next step (old algorithm still active until then) | MISMATCH | the scene draws the NEW algorithm's header and knobs while the toast says it has not landed |
| 48 | PRESETS toast (playing) | PRESETS | P02 AT NEXT BAR (scene playing=True) | knob_map PERF PRESET: playing: first 768-tick bar >= 1.5 s after last detent | MATCH |  |
| 49 | PRESETS toast (stopped) | no scene; text is fixed 'AT NEXT BAR' | stopped: loads 1.5 s after the last detent | NOT SHOWN | no stopped-state wording exists in v10 |
| 50 | pending header | pill 'P02?' only | knob_map: header 'Pnn?' then 'Pnn>' once armed to the bar | partial | the 'Pnn>' armed state is not drawn |
| 51 | OCT+ ROW 2 | OCT+ | ROW 2 OF 2 | button_modes B_OCTUP tap label 'ROW 2'; ROW 1->ROW 2 | MATCH | 'OF 2' not in TSV (derived from absence of a RIG) |
| 52 | OCT+ ROW 3 | OCT+ | ROW 3: RIG; markers 3 only when S['rig'] | B_OCTUP tap: ROW 2 -> ROW 3 'only when the channel has a device with a wire that is not DEV' | MATCH |  |
| 53 | OCT+ on last row | no scene | 'on ROW 3 inert, toast LAST ROW: OCT- = ROW 1' | NOT SHOWN |  |
| 54 | OCT- toast | no scene | B_OCTDN tap: latch ROW 1; hold = peek | NOT SHOWN | only OCT+ toasts exist |
| 55 | ROW 3 tile labels | WIRE1..WIRE4 with 0..127 bars | knob_map PERF R1-R4: 'the cell shows the device's name for the parameter' | MISMATCH | generic WIRE n labels, not device parameter names; value range 0..127 generic |
| 56 | SELECT past last channel | docstring: SELECT past the last channel adds one; header '+' chip | knob_map PERF SELECT: 'clamped, no wrap'; SQ_NCH 4 | CONFLICT | no TSV row defines 'add a channel' (grep add-channel in the 4 TSVs: none) |
| 57 | REC K1 | CURSOR 1..16  | label CURSOR; range 1..LENGTH, wraps | MATCH | CURSOR fixed 1..16 vs 1..LENGTH wraps |
| 58 | REC K2 | ACC 0..1 ['OFF', 'ON'] | label ACC; range OFF ON | MATCH |  |
| 59 | REC K3 | SLIDE 0..1 ['OFF', 'ON'] | label SLIDE; range OFF ON | MATCH |  |
| 60 | REC K4 | OCT -2..2 ['-2', '-1', '0', '+1', '+2'] | label OCT; range -2..+2, default 0 | MATCH |  |
| 61 | REC cue strip | PLAY KEYS TO WRITE  /  REC = DONE | page_states REC cue_strip: 'REC: PLAY KEYS TO WRITE  REC=done' | MATCH (wording) | drops 'REC:' prefix; 'REC = DONE' vs 'REC=done' |
| 62 | REC lane | lane = normal playhead lane (no write cursor mark) | page_states REC screen: 'lane with the write cursor' | MISMATCH | the write cursor position is only the K1 CURSOR number in a tile |
| 63 | REC PRESET turn | no scene | knob_map REC PRESET: inert, toast 'REC: tap REC to finish' | NOT SHOWN |  |
| 64 | REC keys | plain keys, keybed lane tints removed, black keys plain | page_states REC: all 27 keys play and write; force level on F3-C5 | MATCH |  |
| 65 | dice toast | NOTES | EDIT=UNDO (lit SCL) | button_modes B_SCL tap: dice NOTES; 'EDIT tap undoes' | MATCH |  |
| 66 | MIDI OFF toast | MIDI OFF | CHN=SET | key_map CHN tap: opens the CHAN page (not SET); SET is the GLO tap page (BPM SWING GATE DIV) | MISMATCH | the hint names SET where the page that holds MIDI is CHAN |
| 67 | channel added toast | NEW | ALGO=PICK | knob_map ALGO turn = algorithm | MATCH (derived) | no TSV row for adding a channel |
| 68 | MIDI OFF header | red chip 'MIDI OFF' | page_states PERF line 2 reads 'MUTE n / MIDI OFF' | MATCH | text MIDI OFF equal |
| 69 | header content | play/stop, BPM, 4 channel chips, algorithm, MIDI n, preset, row markers, page dots | page_states PERF header: play, clock dot + BPM with source INT/EXT n/EXT --/WAIT, preset, CH, algorithm, MIDI, page map, length | partial | absent: clock dot, tempo source, LENGTH; present extras: row markers |
| 70 | hold-sub-labels on buttons | none drawn (hold named only while held) | page_states PERF: button picture line 2 = hold sub-label h:RST h:HARM h:HLP/MENU h:UNDO L / dice sub-label | MISMATCH |  |
| 71 | dice absent-layer dimming | only DEMO has absent=('ACCNT',); algos load() sets absent=() for all 6 real algorithms | button_modes B_ARP: 'dim if absent'; sq_types.h AlgoDesc.layer[].n=0 absent | unverifiable from TSVs | TSVs carry no per-algorithm layer-presence column |
| 72 | AlgoDesc.name | PATTERNS is 8 chars; header uses 'PATT' | sq_types.h AlgoDesc.name[8] comment '<= 6 chars' | n/a | PATTERNS (8) does not fit name[8] with NUL; UI_EXTRA short=PATT handles the header only; toast 'PATTERNS AT NEXT STEP' uses the full name |


---

<!-- scripts/s8_coverage.py -->
## 8 coverage of PERF control x gesture rows (classification stated per row below)  (scripts/s8_coverage.py)
classes: yes = a persistent element on the PERF screen identifies it; touch/press/hold-only = appears only while touching/pressing/holding (toast, hold hint, lit tile); no = nowhere; n/a = TSV says no hold role / act on press (excluded from the denominator)
ALL (buttons + keys + knobs): 137 rows; yes 114, touch/press/hold-only 5, no 10, n/a (no hold role) 8 -> coverage 'yes' = 114/129 = 88%; yes + touch = 119/129 = 92%
buttons (button_modes PERF): 30 rows; yes 12, touch/press/hold-only 5, no 5, n/a (no hold role) 8 -> coverage 'yes' = 12/22 = 55%; yes + touch = 17/22 = 77%
keys (key_map PERF): 48 rows; yes 43, touch/press/hold-only 0, no 5, n/a (no hold role) 0 -> coverage 'yes' = 43/48 = 90%; yes + touch = 43/48 = 90%
knobs (knob_map PERF, incl. 48 per-algorithm K/S rows): 59 rows; yes 59, touch/press/hold-only 0, no 0, n/a (no hold role) 0 -> coverage 'yes' = 59/59 = 100%; yes + touch = 59/59 = 100%
knobs, '*' rows only (K1-4, R1-4, SELECT, ALGO, PRESET): 11 rows; yes 11, touch/press/hold-only 0, no 0, n/a (no hold role) 0 -> coverage 'yes' = 11/11 = 100%; yes + touch = 11/11 = 100%

rows classified 'no':
  - button_modes B_OCTDN tap | no OCT- toast scene; row markers only show the current row
  - button_modes B_OCTDN hold | peek not shown
  - button_modes B_OCTDN hold-both | update mode not shown
  - button_modes B_OCTUP hold | peek not shown
  - button_modes B_OCTUP hold-both | update mode not shown
  - key_map KEY_B01_F#3 hold | PREVIEW hold not shown
  - key_map KEY_B02_G#3 hold | PREVIEW hold not shown
  - key_map KEY_B03_A#3 hold | PREVIEW hold not shown
  - key_map KEY_B04_C#4 hold | PREVIEW hold not shown
  - key_map KEY_B05_D#4 hold | RIG hold not shown

rows classified 'touch/press/hold only':
  - button_modes B_EDIT hold | hold hint HOLD EDIT: RESET
  - button_modes B_GLO hold | hold hint HOLD GLO: HARMONY
  - button_modes B_HOME hold | hold hint TOUCH=HELP LET GO=MENU
  - button_modes B_SAVE hold | hold hint HOLD SAVE: UNDO LOAD
  - button_modes B_OCTUP tap | no OCT+ button on screen; row markers show the row (persistent) but the action/label ROW n appears only as the OCT+ toast

full row table: 137 rows
| source | control | gesture | class | evidence |
|---|---|---|---|---|
| button_modes | B_FX | tap | yes | dice_all icon |
| button_modes | B_FX | hold | n/a (no action) | no hold role: acted once on press |
| button_modes | B_SCL | tap | yes | dice_notes icon (algorithm sub-label under it: no) |
| button_modes | B_SCL | hold | n/a (no action) | no hold role: acted once on press |
| button_modes | B_ENV | tap | yes | dice_accnt icon (dimmed if absent: DEMO only) |
| button_modes | B_ENV | hold | n/a (no action) | no hold role: acted once on press |
| button_modes | B_LFO | tap | yes | dice_shape icon |
| button_modes | B_LFO | hold | n/a (no action) | no hold role: acted once on press |
| button_modes | B_EDIT | tap | yes | undo icon; toast EDIT=UNDO after a dice |
| button_modes | B_EDIT | hold | touch/press/hold only | hold hint HOLD EDIT: RESET |
| button_modes | B_GLO | tap | yes | settings icon |
| button_modes | B_GLO | hold | touch/press/hold only | hold hint HOLD GLO: HARMONY |
| button_modes | B_HOME | tap | yes | home icon |
| button_modes | B_HOME | hold | touch/press/hold only | hold hint TOUCH=HELP LET GO=MENU |
| button_modes | B_SAVE | tap | yes | save icon |
| button_modes | B_SAVE | hold | touch/press/hold only | hold hint HOLD SAVE: UNDO LOAD |
| button_modes | B_ARP | tap | yes | dice_artic icon |
| button_modes | B_ARP | hold | n/a (no action) | no hold role: acted once on press |
| button_modes | B_SEQ | tap | yes | dice_hits icon |
| button_modes | B_SEQ | hold | n/a (no action) | no hold role: acted once on press |
| button_modes | B_PLAY | tap | yes | play icon + header play/stop mark |
| button_modes | B_PLAY | hold | n/a (no action) | no hold role: acted once on press |
| button_modes | B_REC | tap | yes | rec ring icon; REC state in header + button |
| button_modes | B_REC | hold | n/a (no action) | no hold role: acted once on press |
| button_modes | B_OCTDN | tap | no | no OCT- toast scene; row markers only show the current row |
| button_modes | B_OCTDN | hold | no | peek not shown |
| button_modes | B_OCTDN | hold-both | no | update mode not shown |
| button_modes | B_OCTUP | tap | touch/press/hold only | no OCT+ button on screen; row markers show the row (persistent) but the action/label ROW n appears only as the OCT+ toast |
| button_modes | B_OCTUP | hold | no | peek not shown |
| button_modes | B_OCTUP | hold-both | no | update mode not shown |
| key_map | KEY_W01_F3 | tap | yes | bar/key lane block shows step on/off |
| key_map | KEY_W01_F3 | hold | yes | message line 'HOLD = EDIT' (static text) |
| key_map | KEY_W02_G3 | tap | yes | bar/key lane block shows step on/off |
| key_map | KEY_W02_G3 | hold | yes | message line 'HOLD = EDIT' (static text) |
| key_map | KEY_W03_A3 | tap | yes | bar/key lane block shows step on/off |
| key_map | KEY_W03_A3 | hold | yes | message line 'HOLD = EDIT' (static text) |
| key_map | KEY_W04_B3 | tap | yes | bar/key lane block shows step on/off |
| key_map | KEY_W04_B3 | hold | yes | message line 'HOLD = EDIT' (static text) |
| key_map | KEY_W05_C4 | tap | yes | bar/key lane block shows step on/off |
| key_map | KEY_W05_C4 | hold | yes | message line 'HOLD = EDIT' (static text) |
| key_map | KEY_W06_D4 | tap | yes | bar/key lane block shows step on/off |
| key_map | KEY_W06_D4 | hold | yes | message line 'HOLD = EDIT' (static text) |
| key_map | KEY_W07_E4 | tap | yes | bar/key lane block shows step on/off |
| key_map | KEY_W07_E4 | hold | yes | message line 'HOLD = EDIT' (static text) |
| key_map | KEY_W08_F4 | tap | yes | bar/key lane block shows step on/off |
| key_map | KEY_W08_F4 | hold | yes | message line 'HOLD = EDIT' (static text) |
| key_map | KEY_W09_G4 | tap | yes | bar/key lane block shows step on/off |
| key_map | KEY_W09_G4 | hold | yes | message line 'HOLD = EDIT' (static text) |
| key_map | KEY_W10_A4 | tap | yes | bar/key lane block shows step on/off |
| key_map | KEY_W10_A4 | hold | yes | message line 'HOLD = EDIT' (static text) |
| key_map | KEY_W11_B4 | tap | yes | bar/key lane block shows step on/off |
| key_map | KEY_W11_B4 | hold | yes | message line 'HOLD = EDIT' (static text) |
| key_map | KEY_W12_C5 | tap | yes | bar/key lane block shows step on/off |
| key_map | KEY_W12_C5 | hold | yes | message line 'HOLD = EDIT' (static text) |
| key_map | KEY_W13_D5 | tap | yes | bar/key lane block shows step on/off |
| key_map | KEY_W13_D5 | hold | yes | message line 'HOLD = EDIT' (static text) |
| key_map | KEY_W14_E5 | tap | yes | bar/key lane block shows step on/off |
| key_map | KEY_W14_E5 | hold | yes | message line 'HOLD = EDIT' (static text) |
| key_map | KEY_W15_F5 | tap | yes | bar/key lane block shows step on/off |
| key_map | KEY_W15_F5 | hold | yes | message line 'HOLD = EDIT' (static text) |
| key_map | KEY_W16_G5 | tap | yes | bar/key lane block shows step on/off |
| key_map | KEY_W16_G5 | hold | yes | message line 'HOLD = EDIT' (static text) |
| key_map | KEY_B01_F#3 | tap | yes | black-key block / x7 glyph (blank for an unused channel) |
| key_map | KEY_B01_F#3 | hold | no | PREVIEW hold not shown |
| key_map | KEY_B02_G#3 | tap | yes | black-key block / x7 glyph (blank for an unused channel) |
| key_map | KEY_B02_G#3 | hold | no | PREVIEW hold not shown |
| key_map | KEY_B03_A#3 | tap | yes | black-key block / x7 glyph (blank for an unused channel) |
| key_map | KEY_B03_A#3 | hold | no | PREVIEW hold not shown |
| key_map | KEY_B04_C#4 | tap | yes | black-key block / x7 glyph (blank for an unused channel) |
| key_map | KEY_B04_C#4 | hold | no | PREVIEW hold not shown |
| key_map | KEY_B05_D#4 | tap | yes | DIN glyph |
| key_map | KEY_B05_D#4 | hold | no | RIG hold not shown |
| key_map | KEY_B06_F#4 | hold | yes | glyph/letter SOL |
| key_map | KEY_B07_G#4 | hold | yes | glyph/letter FIL |
| key_map | KEY_B08_A#4 | hold | yes | glyph/letter ROL |
| key_map | KEY_B09_C#5 | tap | yes | glyph/letter PG- |
| key_map | KEY_B10_D#5 | tap | yes | glyph/letter PG+ |
| key_map | KEY_B11_F#5 | tap | yes | glyph/letter RST |
| knob_map | */K1 | turn | yes | tile 1 |
| knob_map | */K2 | turn | yes | tile 2 |
| knob_map | */K3 | turn | yes | tile 3 |
| knob_map | */K4 | turn | yes | tile 4 |
| knob_map | */SELECT | turn | yes | channel chips + selection bar (+ toast) |
| knob_map | */ALGO | turn | yes | algorithm name in header (+ toast) |
| knob_map | */PRESET | turn | yes | P01 text / P02? pill |
| knob_map | */R1 | turn | yes | tile WIRE1 (generic label) |
| knob_map | */R2 | turn | yes | tile WIRE2 (generic label) |
| knob_map | */R3 | turn | yes | tile WIRE3 (generic label) |
| knob_map | */R4 | turn | yes | tile WIRE4 (generic label) |
| knob_map | 48 per-algorithm K/S rows | turn | yes | tiles ROW 1 / ROW 2 |


---

<!-- scripts/s9_cost.py -->
## 9 firmware cost (HI-CON)  (scripts/s9_cost.py)

## icon bitmaps drawn in the 28 scenes: 21 distinct
| icon | w x h | lit px | bytes row-packed (ceil(w/8)*h) | bytes bit-packed (ceil(w*h/8)) |
|---|---|---|---|---|
| play_s | 9x9 | 49 | 18 | 11 |
| dice_all | 13x13 | 65 | 26 | 22 |
| dice_notes | 13x13 | 68 | 26 | 22 |
| dice_accnt | 13x13 | 75 | 26 | 22 |
| dice_shape | 13x13 | 64 | 26 | 22 |
| undo | 13x11 | 26 | 22 | 18 |
| settings | 13x11 | 70 | 22 | 18 |
| home | 13x11 | 59 | 22 | 18 |
| save | 13x11 | 83 | 22 | 18 |
| dice_artic | 13x13 | 71 | 26 | 22 |
| dice_hits | 13x13 | 74 | 26 | 22 |
| play | 13x11 | 72 | 22 | 18 |
| rec_ring | 13x11 | 60 | 22 | 18 |
| x7 | 7x7 | 33 | 7 | 7 |
| midi_s | 7x5 | 19 | 5 | 5 |
| left | 5x7 | 16 | 7 | 5 |
| right | 5x7 | 16 | 7 | 5 |
| rst | 5x7 | 23 | 7 | 5 |
| stop_s | 9x9 | 81 | 18 | 11 |
| rec | 13x11 | 111 | 22 | 18 |
| din | 11x9 | 28 | 18 | 13 |
TOTAL distinct icons 21 : 397 bytes row-packed, 320 bytes bit-packed (1 bpp, no per-icon header)
v10-specific icons (icons_v10.py new/overridden keys): ['dice_accnt', 'dice_artic', 'dice_hits', 'dice_shape', 'din', 'midi', 'midi_s', 'play_s', 'rec_ring', 'stop_s', 'x7']
button icons: 12 distinct of 12 buttons; rec_ring + `rec` (armed) -> True
icons in the table that no scene draws: ['midi']
(din/midi/stop_s are defined but drawn? din drawn: yes ; midi: no ; stop_s: yes ; rec: yes ; undo/home/save/settings in toasts)

## colours drawn as flat fills / icon pixels (rect, frame, art) in the 28 scenes that are NOT a HI-CON token
HI-CON tokens: INK #000000, SURF #292829, TEXT #fffaff, THEME #ffea00, ACCENT #00e6ff, MID #b4aeb4, DIM #6a696a, LINE #312c31, SEL #cdba00, TINT #201c00, REC #ff484a, RAISE #414041, KEY #c5c2c5, LANE #413c20, GRID #393439
| RGB565 | hex | first scene | draws | what it is |
|---|---|---|---|---|
| 0xE4E0 | #e69d00 | playing | 723 | CH1 Okabe-Ito |
| 0xFFFF | #ffffff | playing | 313 | WHITE (255,255,255); HI-CON TEXT is #fffaff |
| 0x55BD | #52b6ee | playing | 191 | CH2 Okabe-Ito |
| 0x4A49 | #4a484a | playing | 143 | DICE_BG (72,72,72) |
| 0x6A40 | #6a4800 | playing | 111 | mix(CH1,120/255) knob fill |
| 0xCBD4 | #cd79a4 | playing | 90 | CH4 Okabe-Ito |
| 0x528A | #525052 | playing | 22 | PHC playhead column (80,80,80) |
| 0xF728 | #f6e641 | sel_muted | 17 | CH3 Okabe-Ito |
| 0x22AD | #20556a | midi_off | 16 | mix(CH2,120/255) knob fill |
| 0xD945 | #de2829 | hold_home | 15 | OFF_RED (220,40,40); HI-CON REC is #ff484a |
| 0x7343 | #736918 | sel_muted | 4 | mix(CH3,120/255) knob fill |
| 0x73AE | #737573 | rec_armed | 2 | mix(WHITE,120/255) REC knob fill |
count of off-palette flat colours: 12  (of 20 distinct flat/icon colours drawn)
off-palette flat colours that are channel identity (4) / fills derived by mix (4) / neutrals+red (WHITE, PHC, DICE_BG, OFF_RED = 4)
HI-CON tokens used by v10: DIM, INK, KEY, MID, RAISE, REC, SURF, TEXT
HI-CON tokens never drawn as flat fill: ACCENT, GRID, LANE, LINE, SEL, THEME, TINT
note: text anti-aliasing ramps (Canvas.ramp) add intermediate colours; the same count would arise on device from the gfx ramp, not counted here
distinct RGB values in the rendered 'playing' frame (incl. AA ramps): 108

module PALETTE = HI-CON ; hard-coded colours (not theme tokens) in v10.py: CH (4), WHITE, INK(=black, equals HI-CON BG only), PHC, DICE_BG, OFF_RED
  INK == T.BG under HI-CON: True
  INK == T.BG under GREY: False
