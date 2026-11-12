# EVIDENCE_B_v11 (measured evidence, facts only)

Subject: proposals/v11/v11.py (29 scenes), palette HI-CON, mock tool docs/design/ui_review/mock/mock.py (real AF_S/AF_M/AF_L, Canvas.text_flags). Every number below is produced by a script in `scripts/` (outputs kept beside them as `scripts/out_*.txt`); `scripts/lib.py` instruments `Canvas.text_flags` and `P.art` to capture each draw. ASSESSMENT_A_v11.md was not read. Viewing assumption (given): 30 mm cap, 8 px/mm, 35 cm, arcmin = cap_mm/350*3438. Conversion for reference: AF_S cap 9 px = 1.125 mm = 11.05 arcmin; AF_M cap 11 px = 1.375 mm = 13.5 arcmin; AF_L cap 21 px = 2.625 mm = 25.8 arcmin (the tool exposes cap heights 9/11/21 via el.CAP; 30 mm / 8 px/mm is the stated scale, applied here without re-measuring).

## Headline numbers
- Reproduction: 29 of 29 scenes render pixel-identical (0 differing px) to proposals/v11/out/v11_<scene>.png.
- Text: playing scene = 26 strings / 77 chars (AF_M 15, AF_S 7, AF_L 4); 20 distinct icon bitmaps (379 bytes at 1 bpp) used across all scenes.
- Text contrast: 25 distinct visible (font, fg, bg) pairs over all scenes; lowest passed pair 4.73 (white on OFF_RED, AF_S/AF_M); lowest AF_L pair 8.27. No text pair below 4.5 on its declared background. Three places where ink meets a different colour under the bbox (see 3a).
- Non-text: 88 pairs measured, 26 below 3:1 (list in 3b). Largest groups: knob meter fill vs empty tile (1.38 to 1.80), tile/key surfaces vs BG (1.43 to 2.32), CH colour vs white bar / OFF_RED / hatch white (1.37 to 3.46), playhead column vs BG 2.63.
- CVD (CIEDE2000, Machado 2009, severity 1.0): pairs < 20: deuteranopia CH2-CH4 17.6, CH3-CH4 14.3; protanopia CH1-CH3 18.0, CH2-CH4 15.4; tritanopia CH1-CH4 10.4, CH2-CH3 13.0. Normal vision minimum is CH2-CH3 35.1. Luminance ratio between channel colours is 1.01 to 1.53.
- Geometry: no text crosses the y=120 seam in any scene; message panel is x104..239 (136 px) y0..21 and covers header algorithm name + MIDI field while shown.
- Sweeps: A.check() = [] for all 7 algorithms; 1476 knob-value renders and 8704 full-screen renders: 0 warnings/exceptions, 0 values outside their tile; value font is stable per knob (only the by-design zero word 'KEYS' drops to AF_M). 75 message strings: 15 cut (list in 5c), all are strings the scenes do not contain.
- Coverage of PERF control x gesture rows: 68/81 = 84% persistent, 4/81 = 5% only on touch/press/hold, 9/81 = 11% not shown.

## MISMATCHES and measured defects (complete list)
Mismatches against the TSVs / firmware docs:
1. SELECT past the last channel adds one (scenes channel_added, added_after; toast NEW ALGO=PICK). knob_map.tsv PERF SELECT: "turn: select channel CH1..CH4 (clamped, no wrap)". Contradicts the TSV as it stands.
2. ALGO turn: page_states.tsv ALGO_PICK says the cue strip lists all six algorithms with the current one bracketed (ACID EUCL SNAK STAG [ARP] PATT, 185 px). Scene turn_algo shows only the toast `ALGO / SNAKE AT STEP`; the cue strip still reads KEYS = STEPS / HOLD = EDIT.
3. Pending preset: knob_map.tsv PRESET says the pending marker is in the header (Pnn?, then Pnn> while armed to load at the bar). v11 draws `P02?` in the status line (y80), not the header, and has no `Pnn>` state. Turn range in the TSV is P01..P16; the toast generator accepts any number (tested P99).
4. REC K1 CURSOR: knob_map.tsv `1..LENGTH, wraps` (LENGTH up to 64; the fixture state has pages=2 i.e. 32 steps). REC_ROW hardcodes min 1, max 16 and the frame is drawn on a 16-cell lane.
5. PERF page_states `screen`: header = play, clock dot + BPM with tempo source (INT / EXT n / EXT -- / WAIT), preset, CH, algorithm, MIDI, page map, length; v11 header has play/stop glyph, BPM, four channel chips, algorithm short name, MIDI field; no clock dot, no tempo source, no length; preset is in the status line; page map is the text `1/2`.
6. PERF page_states / button_modes: button picture "line 1 tap label; line 2 hold sub-label (h:RST h:HARM h:HLP/MENU h:UNDO L) or the dice sub-label (algorithm sub-label under it)". v11 tiles are icon only; the BUTTONS hold column (RESET, HARM, HLP/MN, UNDO L) is never drawn on a tile; dice sub-labels (AlgoDesc.sub) are not drawn anywhere.
7. PERF page_states: preview speaker icon on the channel cell, and line 2 `MUTE n / MIDI OFF / CUE n` text. Not drawn (PREVIEW is the hold of M1..M4).
8. REC page_states `screen`: "banner REC"; v11 uses a red disc in the header with no word REC (cue strip: `PLAY KEYS TO WRITE` left, `REC = DONE` right; TSV cue `REC: PLAY KEYS TO WRITE  REC=done`).
9. Cue `CHN KEY = MIDI`: key_map.tsv PERF D#4 tap = "open the CHAN page (LENGTH MIDI SOURCE FOLLOW)", hold = RIG; FIRST_RUN step text is `D#4 K2: MIDI CHANNEL`. The cue names only the MIDI field of that page.
10. HOME hold panel reads `HELP / MENU` with the bar filling to 100; TSV: from 0.25 s the strip reads `HOME: touch=HELP  let go=MENU`; menu fires on release after 0.7 s. Hold fractions in the scenes (EDIT 60, GLO 60, SAVE 80, HOME 100) are fixed numbers, not derived from the TSV times (0.4 / 0.4 / 0.7 / 0.7 s).
11. ROW 3 DEV wires: TSV "DEV: inert, toast names it"; v11 has no toast for it, and OCT+ on ROW 3 "inert, toast LAST ROW: OCT- = ROW 1" is the only text longer than the panel (x1=285, cut at 5c).
Measured defects (not TSV mismatches): text '48(32)' (STAGE TOTAL with heard) overlaps the full-colour level-edge pixels (2.29 vs amber); muted chip digit is crossed by the same-colour diagonal (1.00); header digit '2' top ink row touches the OFF_RED cap row on the MIDI OFF chip (4.44 at those pixels); playhead white cap (y93..94) is 0 px below the ink of `1/2` at steps 15 and 16 and 1 px below `P01`/`PAGE` at steps 1, 2, 13, 14, 15; REC cursor frame at step 1 starts at x=-1 (1 px clipped); pitched-bar accent cap on the tallest bar sits at y95, directly below the playhead cap (cap y93..94, accent cap y95..96: 0 px gap) when the playhead is on that step; full-name PATTERNS messages (CH1 PATTERNS, PATTERNS AT STEP) cut at 241/249 px; the colours in v11.py other than INK are not HI-CON tokens (12 of 20 non-text colours).


## 1 Reproduction diff vs proposals/v11/out
```
playing 0 
two_channels 0 
one_channel 0 
sel_muted 0 
dice 0 
turning_k2 0 
hold_edit 0 
hold_home 0 
rec_armed 0 
midi_off 0 
channel_added 0 
midi_off_after 0 
added_after 0 
pending_p02 0 
turn_select 0 
turn_algo 0 
turn_presets 0 
oct_row2 0 
row3_rig 0 
hold_glo 0 
hold_save 0 
rec_playing 0 
arp_row2 0 
demo_algo 0 
stage_heard 0 
sheet_row1_a 0 
sheet_row1_b 0 
sheet_row2_a 0 
sheet_row2_b 0 
scenes 29 total diff px 0
```

## 2 Text inventory (6 scenes)
Columns: string, font, fg, bg as passed to the draw call, ink bbox in screen px (x0,y0,x1,y1; right/bottom exclusive). Strings later painted over by a top-bar message are still listed here (they are drawn); section 3a excludes them. Icons are counted as `p.art` calls (bitmap blits, including the 12 button tiles) and do not include rectangle-built marks (REC disc/play mark, chips, meters).

Totals per font and scene (strings / chars): 
| scene | AF_M | AF_S | AF_L | strings | chars | p.art icons |
|--|--|--|--|--|--|--|
| playing | 15 / 59 | 7 / 13 | 4 / 5 | 26 | 77 | 36 |
| dice | 16 / 60 | 9 / 27 | 4 / 5 | 29 | 92 | 34 |
| turn_algo | 15 / 59 | 9 / 30 | 4 / 5 | 28 | 94 | 34 |
| hold_home | 16 / 70 | 7 / 13 | 4 / 5 | 27 | 88 | 36 |
| added_after | 14 / 57 | 7 / 13 | 4 / 4 | 25 | 74 | 34 |
| rec_playing | 14 / 53 | 3 / 18 | 3 / 7 | 20 | 78 | 24 |
Smallest font on screen is AF_S (cap 9 px). Strings in AF_S: MIDI label, row numbers 1..3, PAGE, black-key letters S F R, encoder names and hints in the top bar, REC = DONE.

```

## scene playing
| # | string | font | fg | bg | ink bbox (x0,y0,x1,y1) |
|--|--|--|--|--|--|
| 0 | '120' | AF_M | #fffaff | #000000 | (17, 4, 43, 17) |
| 1 | '1' | AF_M | #000000 | #e69d00 | (49, 5, 54, 16) |
| 2 | '2' | AF_M | #52b6ee | #000000 | (62, 4, 71, 16) |
| 3 | '3' | AF_M | #b4aeb4 | #292829 | (76, 4, 85, 17) |
| 4 | '4' | AF_M | #cd79a4 | #000000 | (90, 5, 99, 16) |
| 5 | 'ACID' | AF_M | #e69d00 | #000000 | (105, 4, 138, 17) |
| 6 | 'MIDI' | AF_S | #b4aeb4 | #000000 | (143, 7, 166, 16) |
| 7 | '2' | AF_M | #ffffff | #000000 | (168, 4, 177, 16) |
| 8 | 'DENS' | AF_M | #b4aeb4 | #292829 | (11, 26, 49, 39) |
| 9 | '12' | AF_L | #ffffff | #292829 | (15, 48, 47, 69) |
| 10 | 'SLIDE' | AF_M | #b4aeb4 | #292829 | (70, 26, 108, 39) |
| 11 | '4' | AF_L | #ffffff | #292829 | (82, 48, 98, 69) |
| 12 | 'JUMP' | AF_M | #b4aeb4 | #292829 | (130, 27, 170, 39) |
| 13 | '5' | AF_L | #ffffff | #292829 | (143, 48, 158, 70) |
| 14 | 'ACCNT' | AF_M | #b4aeb4 | #292829 | (185, 26, 235, 39) |
| 15 | '3' | AF_L | #ffffff | #292829 | (202, 48, 218, 70) |
| 16 | 'P01' | AF_M | #fffaff | #000000 | (3, 79, 27, 92) |
| 17 | '1' | AF_S | #000000 | #e69d00 | (110, 81, 114, 90) |
| 18 | '2' | AF_S | #b4aeb4 | #000000 | (122, 81, 129, 90) |
| 19 | '1/2' | AF_M | #fffaff | #000000 | (216, 79, 238, 93) |
| 20 | 'PAGE' | AF_S | #b4aeb4 | #000000 | (185, 82, 213, 92) |
| 21 | 'S' | AF_S | #fffaff | #414041 | (116, 192, 123, 202) |
| 22 | 'F' | AF_S | #fffaff | #414041 | (131, 192, 137, 201) |
| 23 | 'R' | AF_S | #fffaff | #414041 | (146, 192, 153, 201) |
| 24 | 'KEYS = STEPS' | AF_M | #fffaff | #000000 | (2, 224, 99, 237) |
| 25 | 'HOLD = EDIT' | AF_M | #b4aeb4 | #000000 | (151, 224, 237, 237) |
totals strings: {'AF_M': 15, 'AF_S': 7, 'AF_L': 4} chars: {'AF_M': 59, 'AF_S': 13, 'AF_L': 5} all strings 26 total chars 77
icon art calls (p.art): 36  (+ drawn-by-rect symbols: REC dot/play mark if rec header, tile glyphs excluded)

## scene dice
| # | string | font | fg | bg | ink bbox (x0,y0,x1,y1) |
|--|--|--|--|--|--|
| 0 | '120' | AF_M | #fffaff | #000000 | (17, 4, 43, 17) |
| 1 | '1' | AF_M | #000000 | #e69d00 | (49, 5, 54, 16) |
| 2 | '2' | AF_M | #52b6ee | #000000 | (62, 4, 71, 16) |
| 3 | '+' | AF_M | #b4aeb4 | #000000 | (77, 8, 85, 16) |
| 4 | '4' | AF_M | #cd79a4 | #000000 | (90, 5, 99, 16) |
| 5 | 'ACID' | AF_M | #e69d00 | #000000 | (105, 4, 138, 17) |
| 6 | 'MIDI' | AF_S | #b4aeb4 | #000000 | (143, 7, 166, 16) |
| 7 | '2' | AF_M | #ffffff | #000000 | (168, 4, 177, 16) |
| 8 | 'DENS' | AF_M | #b4aeb4 | #292829 | (11, 26, 49, 39) |
| 9 | '12' | AF_L | #ffffff | #292829 | (15, 48, 47, 69) |
| 10 | 'SLIDE' | AF_M | #b4aeb4 | #292829 | (70, 26, 108, 39) |
| 11 | '4' | AF_L | #ffffff | #292829 | (82, 48, 98, 69) |
| 12 | 'JUMP' | AF_M | #b4aeb4 | #292829 | (130, 27, 170, 39) |
| 13 | '5' | AF_L | #ffffff | #292829 | (143, 48, 158, 70) |
| 14 | 'ACCNT' | AF_M | #b4aeb4 | #292829 | (185, 26, 235, 39) |
| 15 | '3' | AF_L | #ffffff | #292829 | (202, 48, 218, 70) |
| 16 | 'P01' | AF_M | #fffaff | #000000 | (3, 79, 27, 92) |
| 17 | '1' | AF_S | #000000 | #e69d00 | (110, 81, 114, 90) |
| 18 | '2' | AF_S | #b4aeb4 | #000000 | (122, 81, 129, 90) |
| 19 | '1/2' | AF_M | #fffaff | #000000 | (216, 79, 238, 93) |
| 20 | 'PAGE' | AF_S | #b4aeb4 | #000000 | (185, 82, 213, 92) |
| 21 | '1' | AF_M | #000000 | #e69d00 | (110, 6, 115, 17) |
| 22 | 'NOTES' | AF_S | #000000 | #ffffff | (124, 7, 161, 17) |
| 23 | 'EDIT=UNDO' | AF_S | #000000 | #ffffff | (172, 7, 237, 17) |
| 24 | 'S' | AF_S | #fffaff | #414041 | (116, 192, 123, 202) |
| 25 | 'F' | AF_S | #fffaff | #414041 | (131, 192, 137, 201) |
| 26 | 'R' | AF_S | #fffaff | #414041 | (146, 192, 153, 201) |
| 27 | 'KEYS = STEPS' | AF_M | #fffaff | #000000 | (2, 224, 99, 237) |
| 28 | 'HOLD = EDIT' | AF_M | #b4aeb4 | #000000 | (151, 224, 237, 237) |
totals strings: {'AF_M': 16, 'AF_S': 9, 'AF_L': 4} chars: {'AF_M': 60, 'AF_S': 27, 'AF_L': 5} all strings 29 total chars 92
icon art calls (p.art): 34  (+ drawn-by-rect symbols: REC dot/play mark if rec header, tile glyphs excluded)

## scene turn_algo
| # | string | font | fg | bg | ink bbox (x0,y0,x1,y1) |
|--|--|--|--|--|--|
| 0 | '120' | AF_M | #fffaff | #000000 | (17, 4, 43, 17) |
| 1 | '1' | AF_M | #000000 | #e69d00 | (49, 5, 54, 16) |
| 2 | '2' | AF_M | #52b6ee | #000000 | (62, 4, 71, 16) |
| 3 | '+' | AF_M | #b4aeb4 | #000000 | (77, 8, 85, 16) |
| 4 | '4' | AF_M | #cd79a4 | #000000 | (90, 5, 99, 16) |
| 5 | 'ACID' | AF_M | #e69d00 | #000000 | (105, 4, 138, 17) |
| 6 | 'MIDI' | AF_S | #b4aeb4 | #000000 | (143, 7, 166, 16) |
| 7 | '2' | AF_M | #ffffff | #000000 | (168, 4, 177, 16) |
| 8 | 'DENS' | AF_M | #b4aeb4 | #292829 | (11, 26, 49, 39) |
| 9 | '12' | AF_L | #ffffff | #292829 | (15, 48, 47, 69) |
| 10 | 'SLIDE' | AF_M | #b4aeb4 | #292829 | (70, 26, 108, 39) |
| 11 | '4' | AF_L | #ffffff | #292829 | (82, 48, 98, 69) |
| 12 | 'JUMP' | AF_M | #b4aeb4 | #292829 | (130, 27, 170, 39) |
| 13 | '5' | AF_L | #ffffff | #292829 | (143, 48, 158, 70) |
| 14 | 'ACCNT' | AF_M | #b4aeb4 | #292829 | (185, 26, 235, 39) |
| 15 | '3' | AF_L | #ffffff | #292829 | (202, 48, 218, 70) |
| 16 | 'P01' | AF_M | #fffaff | #000000 | (3, 79, 27, 92) |
| 17 | '1' | AF_S | #000000 | #e69d00 | (110, 81, 114, 90) |
| 18 | '2' | AF_S | #b4aeb4 | #000000 | (122, 81, 129, 90) |
| 19 | '1/2' | AF_M | #fffaff | #000000 | (216, 79, 238, 93) |
| 20 | 'PAGE' | AF_S | #b4aeb4 | #000000 | (185, 82, 213, 92) |
| 21 | 'ALGO' | AF_S | #fffaff | #000000 | (109, 7, 139, 17) |
| 22 | 'SNAKE AT STEP' | AF_S | #000000 | #fffaff | (146, 7, 230, 17) |
| 23 | 'S' | AF_S | #fffaff | #414041 | (116, 192, 123, 202) |
| 24 | 'F' | AF_S | #fffaff | #414041 | (131, 192, 137, 201) |
| 25 | 'R' | AF_S | #fffaff | #414041 | (146, 192, 153, 201) |
| 26 | 'KEYS = STEPS' | AF_M | #fffaff | #000000 | (2, 224, 99, 237) |
| 27 | 'HOLD = EDIT' | AF_M | #b4aeb4 | #000000 | (151, 224, 237, 237) |
totals strings: {'AF_M': 15, 'AF_S': 9, 'AF_L': 4} chars: {'AF_M': 59, 'AF_S': 30, 'AF_L': 5} all strings 28 total chars 94
icon art calls (p.art): 34  (+ drawn-by-rect symbols: REC dot/play mark if rec header, tile glyphs excluded)

## scene hold_home
| # | string | font | fg | bg | ink bbox (x0,y0,x1,y1) |
|--|--|--|--|--|--|
| 0 | '120' | AF_M | #fffaff | #000000 | (17, 4, 43, 17) |
| 1 | '1' | AF_M | #000000 | #e69d00 | (49, 5, 54, 16) |
| 2 | '2' | AF_M | #52b6ee | #000000 | (62, 4, 71, 16) |
| 3 | '+' | AF_M | #b4aeb4 | #000000 | (77, 8, 85, 16) |
| 4 | '4' | AF_M | #cd79a4 | #000000 | (90, 5, 99, 16) |
| 5 | 'ACID' | AF_M | #e69d00 | #000000 | (105, 4, 138, 17) |
| 6 | 'MIDI' | AF_S | #b4aeb4 | #000000 | (143, 7, 166, 16) |
| 7 | '2' | AF_M | #ffffff | #000000 | (168, 4, 177, 16) |
| 8 | 'DENS' | AF_M | #b4aeb4 | #292829 | (11, 26, 49, 39) |
| 9 | '12' | AF_L | #ffffff | #292829 | (15, 48, 47, 69) |
| 10 | 'SLIDE' | AF_M | #b4aeb4 | #292829 | (70, 26, 108, 39) |
| 11 | '4' | AF_L | #ffffff | #292829 | (82, 48, 98, 69) |
| 12 | 'JUMP' | AF_M | #b4aeb4 | #292829 | (130, 27, 170, 39) |
| 13 | '5' | AF_L | #ffffff | #292829 | (143, 48, 158, 70) |
| 14 | 'ACCNT' | AF_M | #b4aeb4 | #292829 | (185, 26, 235, 39) |
| 15 | '3' | AF_L | #ffffff | #292829 | (202, 48, 218, 70) |
| 16 | 'P01' | AF_M | #fffaff | #000000 | (3, 79, 27, 92) |
| 17 | '1' | AF_S | #000000 | #e69d00 | (110, 81, 114, 90) |
| 18 | '2' | AF_S | #b4aeb4 | #000000 | (122, 81, 129, 90) |
| 19 | '1/2' | AF_M | #fffaff | #000000 | (216, 79, 238, 93) |
| 20 | 'PAGE' | AF_S | #b4aeb4 | #000000 | (185, 82, 213, 92) |
| 21 | 'HELP / MENU' | AF_M | #000000 | #fffaff | (124, 4, 213, 18) |
| 22 | 'S' | AF_S | #fffaff | #414041 | (116, 192, 123, 202) |
| 23 | 'F' | AF_S | #fffaff | #414041 | (131, 192, 137, 201) |
| 24 | 'R' | AF_S | #fffaff | #414041 | (146, 192, 153, 201) |
| 25 | 'KEYS = STEPS' | AF_M | #fffaff | #000000 | (2, 224, 99, 237) |
| 26 | 'HOLD = EDIT' | AF_M | #b4aeb4 | #000000 | (151, 224, 237, 237) |
totals strings: {'AF_M': 16, 'AF_S': 7, 'AF_L': 4} chars: {'AF_M': 70, 'AF_S': 13, 'AF_L': 5} all strings 27 total chars 88
icon art calls (p.art): 36  (+ drawn-by-rect symbols: REC dot/play mark if rec header, tile glyphs excluded)

## scene added_after
| # | string | font | fg | bg | ink bbox (x0,y0,x1,y1) |
|--|--|--|--|--|--|
| 0 | '120' | AF_M | #fffaff | #000000 | (17, 4, 43, 17) |
| 1 | '1' | AF_M | #e69d00 | #000000 | (49, 5, 54, 16) |
| 2 | '2' | AF_M | #000000 | #52b6ee | (62, 4, 71, 16) |
| 3 | '+' | AF_M | #b4aeb4 | #000000 | (77, 8, 85, 16) |
| 4 | 'EUCLID' | AF_M | #52b6ee | #000000 | (105, 4, 155, 17) |
| 5 | 'MIDI' | AF_S | #b4aeb4 | #000000 | (161, 7, 184, 16) |
| 6 | '10' | AF_M | #ffffff | #000000 | (187, 4, 204, 17) |
| 7 | 'HITS' | AF_M | #b4aeb4 | #292829 | (14, 26, 46, 39) |
| 8 | '5' | AF_L | #ffffff | #292829 | (23, 48, 38, 70) |
| 9 | 'OFFSET' | AF_M | #b4aeb4 | #292829 | (63, 26, 117, 39) |
| 10 | '0' | AF_L | #ffffff | #292829 | (82, 48, 98, 70) |
| 11 | 'RATCH' | AF_M | #b4aeb4 | #292829 | (126, 26, 173, 39) |
| 12 | '4' | AF_L | #ffffff | #292829 | (142, 48, 158, 69) |
| 13 | 'ACCNT' | AF_M | #b4aeb4 | #292829 | (185, 26, 235, 39) |
| 14 | '2' | AF_L | #ffffff | #292829 | (203, 48, 218, 69) |
| 15 | 'P01' | AF_M | #fffaff | #000000 | (3, 79, 27, 92) |
| 16 | '1' | AF_S | #000000 | #52b6ee | (110, 81, 114, 90) |
| 17 | '2' | AF_S | #b4aeb4 | #000000 | (122, 81, 129, 90) |
| 18 | '1/2' | AF_M | #fffaff | #000000 | (216, 79, 238, 93) |
| 19 | 'PAGE' | AF_S | #b4aeb4 | #000000 | (185, 82, 213, 92) |
| 20 | 'S' | AF_S | #fffaff | #414041 | (116, 192, 123, 202) |
| 21 | 'F' | AF_S | #fffaff | #414041 | (131, 192, 137, 201) |
| 22 | 'R' | AF_S | #fffaff | #414041 | (146, 192, 153, 201) |
| 23 | 'CHN KEY = MIDI' | AF_M | #000000 | #ffffff | (4, 224, 111, 237) |
| 24 | 'CH2' | AF_M | #000000 | #ffffff | (206, 224, 236, 237) |
totals strings: {'AF_M': 14, 'AF_S': 7, 'AF_L': 4} chars: {'AF_M': 57, 'AF_S': 13, 'AF_L': 4} all strings 25 total chars 74
icon art calls (p.art): 34  (+ drawn-by-rect symbols: REC dot/play mark if rec header, tile glyphs excluded)

## scene rec_playing
| # | string | font | fg | bg | ink bbox (x0,y0,x1,y1) |
|--|--|--|--|--|--|
| 0 | '120' | AF_M | #fffaff | #000000 | (17, 4, 43, 17) |
| 1 | '1' | AF_M | #000000 | #e69d00 | (49, 5, 54, 16) |
| 2 | '2' | AF_M | #52b6ee | #000000 | (62, 4, 71, 16) |
| 3 | '+' | AF_M | #b4aeb4 | #000000 | (77, 8, 85, 16) |
| 4 | 'ACID' | AF_M | #e69d00 | #000000 | (105, 4, 138, 17) |
| 5 | 'MIDI' | AF_S | #b4aeb4 | #000000 | (143, 7, 166, 16) |
| 6 | '2' | AF_M | #ffffff | #000000 | (168, 4, 177, 16) |
| 7 | 'CURSOR' | AF_M | #b4aeb4 | #292829 | (0, 26, 59, 39) |
| 8 | '4' | AF_L | #ffffff | #292829 | (22, 48, 38, 69) |
| 9 | 'ACC' | AF_M | #b4aeb4 | #292829 | (75, 26, 105, 39) |
| 10 | 'OFF' | AF_L | #ffffff | #292829 | (65, 48, 115, 70) |
| 11 | 'SLIDE' | AF_M | #b4aeb4 | #292829 | (130, 26, 168, 39) |
| 12 | 'OFF' | AF_L | #ffffff | #292829 | (125, 48, 175, 70) |
| 13 | 'OCT' | AF_M | #b4aeb4 | #292829 | (195, 26, 225, 39) |
| 14 | '0' | AF_M | #ffffff | #292829 | (205, 49, 214, 62) |
| 15 | 'P01' | AF_M | #fffaff | #000000 | (3, 79, 27, 92) |
| 16 | '1/2' | AF_M | #fffaff | #000000 | (216, 79, 238, 93) |
| 17 | 'PAGE' | AF_S | #b4aeb4 | #000000 | (185, 82, 213, 92) |
| 18 | 'PLAY KEYS TO WRITE' | AF_M | #fffaff | #000000 | (2, 224, 147, 237) |
| 19 | 'REC = DONE' | AF_S | #b4aeb4 | #000000 | (171, 226, 238, 236) |
totals strings: {'AF_M': 14, 'AF_S': 3, 'AF_L': 3} chars: {'AF_M': 53, 'AF_S': 18, 'AF_L': 7} all strings 20 total chars 78
icon art calls (p.art): 24  (+ drawn-by-rect symbols: REC dot/play mark if rec header, tile glyphs excluded)
```

## 3 Contrast
```
## 3a text pairs (worst-case contrast vs every distinct background pixel under the ink bbox, from a no-text render)
| scene | string | font | fg | bg passed | worst bg under ink | ratio(passed) | ratio(worst) | flag |
|--|--|--|--|--|--|--|--|--|
| playing (x2) | '3' | AF_M | #b4aeb4 | #292829 | #b4aeb4 | 6.75 | 1.00 | FLAG(<4.5) |
| playing (x12) | '12' | AF_L | #ffffff | #292829 | #6a4800 | 14.69 | 8.27 |  |
| playing (x11) | '4' | AF_L | #ffffff | #292829 | #6a4800 | 14.69 | 8.27 |  |
| playing (x12) | '5' | AF_L | #ffffff | #292829 | #6a4800 | 14.69 | 8.27 |  |
| playing (x13) | '3' | AF_L | #ffffff | #292829 | #6a4800 | 14.69 | 8.27 |  |
| sel_muted (x1) | '10' | AF_L | #ffffff | #292829 | #004831 | 14.69 | 10.65 |  |
| sel_muted (x2) | '7' | AF_L | #ffffff | #292829 | #004831 | 14.69 | 10.65 |  |
| sel_muted (x1) | '12' | AF_L | #ffffff | #292829 | #004831 | 14.69 | 10.65 |  |
| turning_k2 (x3) | '6' | AF_L | #ffffff | #292829 | #6a4800 | 14.69 | 8.27 |  |
| midi_off (x2) | '2' | AF_M | #000000 | #52b6ee | #de2829 | 9.28 | 4.44 | FLAG(<4.5) |
| midi_off (x5) | '5' | AF_L | #ffffff | #292829 | #20556a | 14.69 | 8.17 |  |
| midi_off (x5) | '4' | AF_L | #ffffff | #292829 | #20556a | 14.69 | 8.17 |  |
| midi_off (x5) | '2' | AF_L | #ffffff | #292829 | #20556a | 14.69 | 8.17 |  |
| row3_rig (x3) | '64' | AF_L | #ffffff | #292829 | #6a4800 | 14.69 | 8.27 |  |
| row3_rig (x1) | '43' | AF_L | #ffffff | #292829 | #6a4800 | 14.69 | 8.27 |  |
| demo_algo (x1) | '9' | AF_L | #ffffff | #292829 | #6a4800 | 14.69 | 8.27 |  |
| demo_algo (x1) | '8' | AF_L | #ffffff | #292829 | #6a4800 | 14.69 | 8.27 |  |
| stage_heard (x1) | '48(32)' | AF_M | #ffffff | #292829 | #e69d00 | 14.69 | 2.29 | FLAG(<4.5) |
| stage_heard (x1) | '2' | AF_L | #ffffff | #292829 | #6a4800 | 14.69 | 8.27 |  |
texts drawn but painted over by a later element (not counted): 36 [('channel_added', '10'), ('channel_added', 'EUCLID'), ('channel_added', 'MIDI'), ('dice', '2'), ('dice', 'ACID'), ('dice', 'MIDI'), ('hold_edit', '2'), ('hold_edit', 'ACID'), ('hold_edit', 'MIDI'), ('hold_glo', '2'), ('hold_glo', 'ACID'), ('hold_glo', 'MIDI'), ('hold_home', '2'), ('hold_home', 'ACID'), ('hold_home', 'MIDI'), ('hold_save', '2'), ('hold_save', 'ACID'), ('hold_save', 'MIDI'), ('midi_off', 'EUCLID'), ('midi_off', 'MIDI'), ('midi_off', 'OFF'), ('oct_row2', '2'), ('oct_row2', 'ACID'), ('oct_row2', 'MIDI'), ('row3_rig', '2'), ('row3_rig', 'ACID'), ('row3_rig', 'MIDI'), ('turn_algo', '2'), ('turn_algo', 'ACID'), ('turn_algo', 'MIDI'), ('turn_presets', '2'), ('turn_presets', 'ACID'), ('turn_presets', 'MIDI'), ('turn_select', '10'), ('turn_select', 'EUCLID'), ('turn_select', 'MIDI')]
distinct (string,font,fg,bg,worst) combos: 116  flagged: 3
lowest text ratio: 3 AF_M 6.75/1.00 #b4aeb4 on #292829 #b4aeb4

## 3b meaning-bearing non-text pairs (WCAG ratio; flag <3:1)
| pair | ratio | flag |
|--|--|--|
| CH1 amber on BG (strip/bar/chip frame) (#e69d00 / #000000) | 9.19 |  |
| CH1 amber on SURF (tile text / chip? / dots) (#e69d00 / #292829) | 6.43 |  |
| CH1 amber bar on playhead column PHC (#e69d00 / #525052) | 3.50 |  |
| CH1 amber knob fill mix120 vs SURF empty (#6a4800 / #292829) | 1.78 | FLAG <3 |
| CH1 amber level edge (full) vs its fill mix120 (#e69d00 / #6a4800) | 3.62 |  |
| CH1 amber level edge vs SURF (edge above fill) (#e69d00 / #292829) | 6.43 |  |
| CH1 amber live row box fill vs BG (#e69d00 / #000000) | 9.19 |  |
| CH1 amber keybed step key colour vs SURF key (#e69d00 / #292829) | 6.43 |  |
| CH1 amber blk-key mute square vs RAISE (#e69d00 / #414041) | 4.52 |  |
| CH1 amber selected chip fill vs white selection bar (#e69d00 / #ffffff) | 2.29 | FLAG <3 |
| CH1 amber vs OFF_RED cap (MIDI OFF cap on chip) (#e69d00 / #de2829) | 2.07 | FLAG <3 |
| CH1 amber hatch on white (dice step: CH over WHITE) (#e69d00 / #ffffff) | 2.29 | FLAG <3 |
| CH2 sky on BG (strip/bar/chip frame) (#52b6ee / #000000) | 9.28 |  |
| CH2 sky on SURF (tile text / chip? / dots) (#52b6ee / #292829) | 6.49 |  |
| CH2 sky bar on playhead column PHC (#52b6ee / #525052) | 3.53 |  |
| CH2 sky knob fill mix120 vs SURF empty (#20556a / #292829) | 1.80 | FLAG <3 |
| CH2 sky level edge (full) vs its fill mix120 (#52b6ee / #20556a) | 3.61 |  |
| CH2 sky level edge vs SURF (edge above fill) (#52b6ee / #292829) | 6.49 |  |
| CH2 sky live row box fill vs BG (#52b6ee / #000000) | 9.28 |  |
| CH2 sky keybed step key colour vs SURF key (#52b6ee / #292829) | 6.49 |  |
| CH2 sky blk-key mute square vs RAISE (#52b6ee / #414041) | 4.56 |  |
| CH2 sky selected chip fill vs white selection bar (#52b6ee / #ffffff) | 2.26 | FLAG <3 |
| CH2 sky vs OFF_RED cap (MIDI OFF cap on chip) (#52b6ee / #de2829) | 2.09 | FLAG <3 |
| CH2 sky hatch on white (dice step: CH over WHITE) (#52b6ee / #ffffff) | 2.26 | FLAG <3 |
| CH3 green on BG (strip/bar/chip frame) (#009d73 / #000000) | 6.07 |  |
| CH3 green on SURF (tile text / chip? / dots) (#009d73 / #292829) | 4.25 |  |
| CH3 green bar on playhead column PHC (#009d73 / #525052) | 2.31 | FLAG <3 |
| CH3 green knob fill mix120 vs SURF empty (#004831 / #292829) | 1.38 | FLAG <3 |
| CH3 green level edge (full) vs its fill mix120 (#009d73 / #004831) | 3.08 |  |
| CH3 green level edge vs SURF (edge above fill) (#009d73 / #292829) | 4.25 |  |
| CH3 green live row box fill vs BG (#009d73 / #000000) | 6.07 |  |
| CH3 green keybed step key colour vs SURF key (#009d73 / #292829) | 4.25 |  |
| CH3 green blk-key mute square vs RAISE (#009d73 / #414041) | 2.98 | FLAG <3 |
| CH3 green selected chip fill vs white selection bar (#009d73 / #ffffff) | 3.46 |  |
| CH3 green vs OFF_RED cap (MIDI OFF cap on chip) (#009d73 / #de2829) | 1.37 | FLAG <3 |
| CH3 green hatch on white (dice step: CH over WHITE) (#009d73 / #ffffff) | 3.46 |  |
| CH4 pink on BG (strip/bar/chip frame) (#cd79a4 / #000000) | 6.87 |  |
| CH4 pink on SURF (tile text / chip? / dots) (#cd79a4 / #292829) | 4.80 |  |
| CH4 pink bar on playhead column PHC (#cd79a4 / #525052) | 2.61 | FLAG <3 |
| CH4 pink knob fill mix120 vs SURF empty (#5a384a / #292829) | 1.47 | FLAG <3 |
| CH4 pink level edge (full) vs its fill mix120 (#cd79a4 / #5a384a) | 3.27 |  |
| CH4 pink level edge vs SURF (edge above fill) (#cd79a4 / #292829) | 4.80 |  |
| CH4 pink live row box fill vs BG (#cd79a4 / #000000) | 6.87 |  |
| CH4 pink keybed step key colour vs SURF key (#cd79a4 / #292829) | 4.80 |  |
| CH4 pink blk-key mute square vs RAISE (#cd79a4 / #414041) | 3.38 |  |
| CH4 pink selected chip fill vs white selection bar (#cd79a4 / #ffffff) | 3.06 |  |
| CH4 pink vs OFF_RED cap (MIDI OFF cap on chip) (#cd79a4 / #de2829) | 1.55 | FLAG <3 |
| CH4 pink hatch on white (dice step: CH over WHITE) (#cd79a4 / #ffffff) | 3.06 |  |
| hollow muted bars MID on BG (#b4aeb4 / #000000) | 9.65 |  |
| hollow MIDI-OFF bars OFF_RED on BG (#de2829 / #000000) | 4.44 |  |
| muted/off strip dash DIM on BG (#6a696a / #000000) | 3.84 |  |
| empty-step tick DIM on BG (#6a696a / #000000) | 3.84 |  |
| muted chip SURF on BG (#292829 / #000000) | 1.43 | FLAG <3 |
| muted chip diagonal MID on SURF (#b4aeb4 / #292829) | 6.75 |  |
| OFF_RED chip (MIDI OFF) vs BG (#de2829 / #000000) | 4.44 |  |
| playhead column PHC vs BG (#525052 / #000000) | 2.63 | FLAG <3 |
| playhead white bar vs BG (#ffffff / #000000) | 21.00 |  |
| playhead white bar vs PHC (#ffffff / #525052) | 7.99 |  |
| dice tile DICE_BG vs BG (#4a484a / #000000) | 2.32 | FLAG <3 |
| plain tile SURF vs BG (#292829 / #000000) | 1.43 | FLAG <3 |
| dice tile DICE_BG vs plain tile SURF (#4a484a / #292829) | 1.62 | FLAG <3 |
| dice glyph WHITE on DICE_BG (#ffffff / #4a484a) | 9.06 |  |
| dice glyph DIM (absent layer) on DICE_BG (#6a696a / #4a484a) | 1.66 | FLAG <3 |
| plain glyph TEXT on SURF (#fffaff / #292829) | 14.25 |  |
| knob tile SURF vs BG (#292829 / #000000) | 1.43 | FLAG <3 |
| empty list dots frame DIM on SURF (#6a696a / #292829) | 2.69 | FLAG <3 |
| numbered row box frame MID on BG (#b4aeb4 / #000000) | 9.65 |  |
| selection bar WHITE vs BG (#ffffff / #000000) | 21.00 |  |
| message panel WHITE vs header BG (#ffffff / #000000) | 21.00 |  |
| message panel TEXT vs header BG (#fffaff / #000000) | 20.37 |  |
| toast/hold panel INK glyph on TEXT (#000000 / #fffaff) | 20.37 |  |
| hold progress bar INK on TEXT panel (#000000 / #fffaff) | 20.37 |  |
| enc name chip INK vs TEXT panel (#000000 / #fffaff) | 20.37 |  |
| REC cursor frame WHITE vs BG (#ffffff / #000000) | 21.00 |  |
| REC cursor frame WHITE vs PHC column (#ffffff / #525052) | 7.99 |  |
| lit CHN key WHITE vs RAISE (unlit) (#ffffff / #414041) | 10.32 |  |
| lit button WHITE vs SURF (#ffffff / #292829) | 14.69 |  |
| lit button WHITE vs DICE_BG (#ffffff / #4a484a) | 9.06 |  |
| black key RAISE vs BG (#414041 / #000000) | 2.03 | FLAG <3 |
| white key SURF vs BG (#292829 / #000000) | 1.43 | FLAG <3 |
| REC keys KEY vs BG (#c5c2c5 / #000000) | 11.90 |  |
| REC red T.REC vs BG (#ff484a / #000000) | 6.28 |  |
| REC red disc (header) vs BG (#ff484a / #000000) | 6.28 |  |
| accent cap WHITE vs CH (cap sits over BG) (#ffffff / #000000) | 21.00 |  |
| slide bridge CH vs BG (=CH on BG) (#e69d00 / #000000) | 9.19 |  |
| diced step WHITE vs CH hatch (alternating) (#ffffff / #e69d00) | 2.29 | FLAG <3 |
| knob turning frame col vs BG (#e69d00 / #000000) | 9.19 |  |
| pending preset WHITE chip vs BG (#ffffff / #000000) | 21.00 |  |
pairs 88 flagged 26

## 3c colour-vision simulation of the 4 channel colours
method: Machado, Oliveira & Fernandes 2009 matrices, severity 1.0, applied to linear sRGB of the 565-expanded colours; deltaE = CIEDE2000 (skimage) and CIE76 in brackets; Lab D65

normal: CH1 amber sim #e69d00, CH2 sky sim #52b6ee, CH3 green sim #009d73, CH4 pink sim #cd79a4
  CH1-CH2: dE2000 54.3 [dE76 114.8] 
  CH1-CH3: dE2000 43.3 [dE76 89.3] 
  CH1-CH4: dE2000 47.7 [dE76 86.2] 
  CH2-CH3: dE2000 35.1 [dE76 60.4] 
  CH2-CH4: dE2000 46.6 [dE76 58.4] 
  CH3-CH4: dE2000 62.9 [dE76 86.5] 

deuteranopia: CH1 amber sim #c9b310, CH2 sky sim #86a5ed, CH3 green sim #898576, CH4 pink sim #9699a2
  CH1-CH2: dE2000 54.9 [dE76 113.0] 
  CH1-CH3: dE2000 26.7 [dE76 66.4] 
  CH1-CH4: dE2000 34.4 [dE76 78.8] 
  CH2-CH3: dE2000 32.5 [dE76 50.5] 
  CH2-CH4: dE2000 17.6 [dE76 35.2] FLAG <20 (2000)
  CH3-CH4: dE2000 14.3 [dE76 16.0] FLAG <20 (2000)

protanopia: CH1 amber sim #b7a100, CH2 sky sim #9bb5f1, CH3 green sim #999171, CH4 pink sim #818ba6
  CH1-CH2: dE2000 51.3 [dE76 102.6] 
  CH1-CH3: dE2000 18.0 [dE76 51.6] FLAG <20 (2000)
  CH1-CH4: dE2000 42.1 [dE76 85.4] 
  CH2-CH3: dE2000 36.4 [dE76 53.1] 
  CH2-CH4: dE2000 15.4 [dE76 23.6] FLAG <20 (2000)
  CH3-CH4: dE2000 26.5 [dE76 34.0] 

tritanopia: CH1 amber sim #fb8986, CH2 sky sim #00c4c9, CH3 green sim #009d91, CH4 pink sim #d87788
  CH1-CH2: dE2000 54.2 [dE76 88.3] 
  CH1-CH3: dE2000 55.8 [dE76 84.5] 
  CH1-CH4: dE2000 10.4 [dE76 16.6] FLAG <20 (2000)
  CH2-CH3: dE2000 13.0 [dE76 17.2] FLAG <20 (2000)
  CH2-CH4: dE2000 66.4 [dE76 81.3] 
  CH3-CH4: dE2000 55.6 [dE76 77.5] 

luminance ratio between channel colours (normal vision):
  CH1-CH2 1.01
  CH1-CH3 1.51
  CH1-CH4 1.34
  CH2-CH3 1.53
  CH2-CH4 1.35
  CH3-CH4 1.13
```

Notes on 3a: 'worst bg under ink' samples every distinct background pixel under the ink bbox from a render with all text suppressed; 'hidden' texts are those a later top-bar panel fully paints over (36 draws, mostly header algorithm name / MIDI field under toasts and holds). Three flagged ink collisions: muted chip digit '3' (diagonal line same colour #b4aeb4), digit '2' on the CH2 chip (OFF_RED cap row), '48(32)' (level-edge pixels #e69d00).


## 4 Geometry
```
## 4a band y-ranges (pixel scan of 'playing' + code constants)
non-BG row runs (y0..y1): [(1, 19), (21, 22), (24, 39), (41, 76), (79, 140), (146, 162), (164, 180), (186, 220), (224, 236)]
gaps between runs: [(20, 20, 1), (23, 23, 1), (40, 40, 1), (77, 78, 2), (141, 145, 5), (163, 163, 1), (181, 185, 5), (221, 223, 3)]
code: header chips y1..19 (+sel bar 21..22), knob tiles y24..76 (label 24..39, body 41..76), status text cap y80 (boxes y79..91), music y96..141 (playhead cap y93..94, column 95..141), buttons y146..181 (2 rows x17, 1 px gap), keybed y186..218 (+playhead bar 219..220), cue strip text y225 (white panel 222..237)
strip seam y=120 (strips (0,120),(120,240))

## 4b texts crossing y=120 (ink bbox y0<120<y1), all scenes
none

## 4c message panel
TB_X = 104 panel x 104 .. 239 width 136 ; y0..21; chips end x: 101 ; header algo name starts x105; MIDI field ends ~177
header text ink in 'playing': [('120', (17, 4, 43, 17)), ('1', (49, 5, 54, 16)), ('2', (62, 4, 71, 16)), ('3', (76, 4, 85, 17)), ('4', (90, 5, 99, 16)), ('ACID', (105, 4, 138, 17)), ('MIDI', (143, 7, 166, 16)), ('2', (168, 4, 177, 16))]
header content overlapped by panel (x>=104): algo name + MIDI field (x105..177) are covered whenever a toast/hold shows; chips (46..99) and BPM stay.

## 4d message text extents per scene (ink bbox of texts drawn inside the panel; panel x 104..239)
dice [('1', 'AF_M', (110, 6, 115, 17)), ('NOTES', 'AF_S', (124, 7, 161, 17)), ('EDIT=UNDO', 'AF_S', (172, 7, 237, 17))] cut-flags: []
hold_edit [('RESET', 'AF_M', (124, 4, 168, 17))] cut-flags: []
hold_home [('HELP / MENU', 'AF_M', (124, 4, 213, 18))] cut-flags: []
hold_glo [('HARMONY', 'AF_M', (124, 4, 198, 17))] cut-flags: []
hold_save [('UNDO LOAD', 'AF_M', (124, 4, 207, 17))] cut-flags: []
midi_off [('2', 'AF_M', (109, 5, 118, 17)), ('MIDI OFF', 'AF_M', (124, 4, 184, 17)), ('TAKEN', 'AF_S', (201, 7, 237, 16))] cut-flags: []
channel_added [('2', 'AF_M', (109, 5, 118, 17)), ('NEW', 'AF_M', (124, 5, 157, 16)), ('ALGO=PICK', 'AF_S', (174, 7, 237, 17))] cut-flags: []
turn_select [('SELECT', 'AF_S', (109, 7, 151, 17)), ('CH2 EUCLID', 'AF_S', (158, 7, 224, 17))] cut-flags: []
turn_algo [('ALGO', 'AF_S', (109, 7, 139, 17)), ('SNAKE AT STEP', 'AF_S', (146, 7, 230, 17))] cut-flags: []
turn_presets [('PRESETS', 'AF_S', (109, 7, 158, 17)), ('P02 AT BAR', 'AF_S', (165, 7, 228, 17))] cut-flags: []
oct_row2 [('OCT+', 'AF_S', (109, 7, 139, 17)), ('ROW 2 OF 2', 'AF_M', (146, 4, 227, 17))] cut-flags: []
row3_rig [('OCT+', 'AF_S', (109, 7, 139, 17)), ('ROW 3 TB-3', 'AF_M', (146, 4, 229, 17))] cut-flags: []

## 4e fixed strip rows (music)
bh_max = 28 bars occupy y96..123; strip rows start y=126: CH1 126..128, CH2 130..132, CH3 134..136, CH4 138..140; spacing 4 px (3 px strip + 1 px gap); lane bottom y1=141 (bars end y123)
bar min height pitched = 6, max = 25 ; single lane bar = 8; accent cap 2 px at by-4 (2 px gap above bar); highest pitched bar top y= 99 ; accent cap on tallest = y 95 (playhead cap at y93..94, column starts 95)

## 4f things < 2 px apart / touching (computed)
knob tiles: x1..58, 61..118, 121..178, 181..238 (each 58 wide), gap 2 px between tiles, 1 px left, 1 px right margin; label cell y24..39 / value cell y41..76: gap 1 px (y40)
header chips: x46..58, 60..72, 74..86, 88..100 (13 wide, 1 px gaps); sel bar y21..22 directly under chip (chip ends y19: gap 1 px); chips y1..19; panel starts x104 (gap 3 px from chip 4 right edge x100? chip4 ends x100)
status row boxes: 11 wide, 2 px gap (x pitch 13); playhead cap y93..94 vs status text ink bottom y<=92 -> 0..1 px
playhead cap (y93..94) x-overlap with status text: [(1, 'P01', (3, 79, 27, 92), 'gap to cap top y93 = 1 px'), (2, 'P01', (3, 79, 27, 92), 'gap to cap top y93 = 1 px'), (8, '1', (110, 81, 114, 90), 'gap to cap top y93 = 3 px'), (9, '2', (122, 81, 129, 90), 'gap to cap top y93 = 3 px'), (13, 'PAGE', (185, 82, 213, 92), 'gap to cap top y93 = 1 px'), (14, 'PAGE', (185, 82, 213, 92), 'gap to cap top y93 = 1 px'), (15, '1/2', (216, 79, 238, 93), 'gap to cap top y93 = 0 px'), (15, 'PAGE', (185, 82, 213, 92), 'gap to cap top y93 = 1 px'), (16, '1/2', (216, 79, 238, 93), 'gap to cap top y93 = 0 px')]
REC cursor frame (y93..141, x pitch 15, 17 wide) overlaps text above (y<93): [(1, 'P01', (3, 79, 27, 92)), (2, 'P01', (3, 79, 27, 92)), (13, 'PAGE', (185, 82, 213, 92)), (14, 'PAGE', (185, 82, 213, 92)), (15, '1/2', (216, 79, 238, 93)), (15, 'PAGE', (185, 82, 213, 92)), (16, '1/2', (216, 79, 238, 93))]
REC cursor frame y93..141 vs buttons y146: 4 px gap; frame vs knob tiles bottom y76: 16 px; frame (2 px thick, 1 px inner gap) double ring x-2..x+14 vs neighbours: adjacent frame pitch 15 so adjacent cursors' outer ring overlaps column neighbours by 2 px
lane column pitch 15: bar 13 wide, 2 px gap between bars; slide bridge 2 px; strip 13 wide
button tiles: x pitch 40, 38 wide, 2 px gap; rows y146..162 and y164..180, 1 px gap between rows; keybed y186: 4 px below buttons
keybed: white keys x pitch 15, 14 wide (1 px gap); black keys 10 wide, 1 px BG halo; keys y186..218, playhead bar y219..220 (0 gap, directly under)
cue strip text ink: KEYS = STEPS y[(2, 224, 99, 237)]; white panel (setup) y222..237
bottom margin: last text ink bottom (playing): 237 screen 240
```

## 5 Sweeps
### 5a/5b algorithm x row x value, channel sets
```
## 5a algorithm x row x value sweep (knob tiles)
renders: 1476
| algo | row | knob | n values | fonts used (count) | widget | font stable |
|--|--|--|--|--|--|--|
| ACID | 1 | DENS | 17 | {'AF_M': 1, 'AF_L': 16} | meter | zero-word AF_M only (by design) |
| ACID | 1 | SLIDE | 9 | {'AF_L': 9} | meter | STABLE |
| ACID | 1 | JUMP | 21 | {'AF_L': 21} | meter | STABLE |
| ACID | 1 | ACCNT | 9 | {'AF_L': 9} | meter | STABLE |
| ACID | 2 | OFFSET | 16 | {'AF_L': 16} | meter | STABLE |
| ACID | 2 | PEDAL | 13 | {'AF_L': 13} | meter | STABLE |
| ACID | 2 | PUNCH | 48 | {'AF_L': 48} | meter | STABLE |
| ACID | 2 | DIR | 3 | {'AF_M': 3} | LIST->dots | STABLE |
| ACID | 3 | CUT | 128 | {'AF_L': 128} | meter | STABLE |
| ACID | 3 | RESO | 128 | {'AF_L': 128} | meter | STABLE |
| ACID | 3 | ENV-X | 128 | {'AF_L': 128} | meter | STABLE |
| ACID | 3 | ENV-Y | 128 | {'AF_L': 128} | meter | STABLE |
| EUCLID | 1 | HITS | 17 | {'AF_M': 1, 'AF_L': 16} | meter | zero-word AF_M only (by design) |
| EUCLID | 1 | OFFSET | 16 | {'AF_L': 16} | meter | STABLE |
| EUCLID | 1 | RATCH | 9 | {'AF_L': 9} | meter | STABLE |
| EUCLID | 1 | ACCNT | 9 | {'AF_L': 9} | meter | STABLE |
| EUCLID | 2 | LEN | 15 | {'AF_L': 15} | meter | STABLE |
| EUCLID | 2 | SPAN | 25 | {'AF_L': 25} | meter | STABLE |
| EUCLID | 2 | PUNCH | 48 | {'AF_L': 48} | meter | STABLE |
| EUCLID | 2 | CURVE | 7 | {'AF_M': 7} | LIST->dots | STABLE |
| SNAKE | 1 | DENS | 17 | {'AF_M': 1, 'AF_L': 16} | meter | zero-word AF_M only (by design) |
| SNAKE | 1 | LEN | 16 | {'AF_L': 16} | meter | STABLE |
| SNAKE | 1 | RANGE | 49 | {'AF_L': 49} | meter | STABLE |
| SNAKE | 1 | PATH | 14 | {'AF_L': 14} | meter | STABLE |
| SNAKE | 2 | OFFSET | 16 | {'AF_L': 16} | meter | STABLE |
| SNAKE | 2 | GLEN | 16 | {'AF_L': 16} | meter | STABLE |
| SNAKE | 2 | DYN | 11 | {'AF_L': 11} | meter | STABLE |
| SNAKE | 2 | DIR | 3 | {'AF_M': 3} | LIST->dots | STABLE |
| STAGE | 1 | DENS | 9 | {'AF_M': 1, 'AF_L': 8} | meter | zero-word AF_M only (by design) |
| STAGE | 1 | TOTAL | 57 | {'AF_M': 57} | meter | STABLE |
| STAGE | 1 | DIR | 3 | {'AF_M': 3} | LIST->dots | STABLE |
| STAGE | 1 | RATCH | 9 | {'AF_L': 9} | meter | STABLE |
| STAGE | 2 | OFFSET | 64 | {'AF_L': 64} | meter | STABLE |
| STAGE | 2 | RDIV | 4 | {'AF_L': 4} | meter | STABLE |
| STAGE | 2 | DYN | 11 | {'AF_L': 11} | meter | STABLE |
| STAGE | 2 | RANGE | 37 | {'AF_L': 37} | meter | STABLE |
| ARP | 1 | DENS | 17 | {'AF_M': 1, 'AF_L': 16} | meter | zero-word AF_M only (by design) |
| ARP | 1 | SHAPE | 6 | {'AF_M': 6} | LIST->dots | STABLE |
| ARP | 1 | SPAN | 4 | {'AF_M': 4} | meter | STABLE |
| ARP | 1 | RATE | 6 | {'AF_M': 6} | LIST->dots | STABLE |
| ARP | 2 | OFFSET | 16 | {'AF_L': 16} | meter | STABLE |
| ARP | 2 | LEGATO | 9 | {'AF_L': 9} | meter | STABLE |
| ARP | 2 | DYN | 11 | {'AF_L': 11} | meter | STABLE |
| ARP | 2 | DIR | 6 | {'AF_M': 6} | LIST->dots | STABLE |
| PATTERNS | 1 | DENS | 17 | {'AF_M': 1, 'AF_L': 16} | meter | zero-word AF_M only (by design) |
| PATTERNS | 1 | VARY | 9 | {'AF_L': 9} | meter | STABLE |
| PATTERNS | 1 | SPAN | 4 | {'AF_L': 4} | meter | STABLE |
| PATTERNS | 1 | ACCNT | 9 | {'AF_L': 9} | meter | STABLE |
| PATTERNS | 2 | PICK | 120 | {'AF_M': 120} | meter | STABLE |
| PATTERNS | 2 | SLIDE | 9 | {'AF_L': 9} | meter | STABLE |
| PATTERNS | 2 | DYN | 15 | {'AF_L': 15} | meter | STABLE |
| PATTERNS | 2 | CTONE | 9 | {'AF_L': 9} | meter | STABLE |
| DEMO | 1 | LOCK | 17 | {'AF_L': 17} | meter | STABLE |
| DEMO | 1 | BITS | 16 | {'AF_L': 16} | meter | STABLE |
| DEMO | 1 | SCALE | 4 | {'AF_M': 4} | LIST->dots | STABLE |
| DEMO | 1 | SPRD | 4 | {'AF_M': 4} | meter | STABLE |
| DEMO | 2 | OFFSET | 16 | {'AF_L': 16} | meter | STABLE |
| DEMO | 2 | GATE | 8 | {'AF_L': 8} | meter | STABLE |
| DEMO | 2 | DYN | 11 | {'AF_L': 11} | meter | STABLE |
| DEMO | 2 | DIR | 3 | {'AF_M': 3} | LIST->dots | STABLE |
problems: 0

A.check(): []
lists > MAX_DOTS (10) fall back to meter: []
lists drawn as dots: [('ACID', 2, 'DIR', 3, 'dot w=8'), ('EUCLID', 2, 'CURVE', 7, 'dot w=5'), ('SNAKE', 2, 'DIR', 3, 'dot w=8'), ('STAGE', 1, 'DIR', 3, 'dot w=8'), ('ARP', 1, 'SHAPE', 6, 'dot w=6'), ('ARP', 1, 'RATE', 6, 'dot w=6'), ('ARP', 2, 'DIR', 6, 'dot w=6'), ('DEMO', 1, 'SCALE', 4, 'dot w=8'), ('DEMO', 2, 'DIR', 3, 'dot w=8')]
MAX_DOTS=10 dot width: 3 px; 11 entries would give 2

## 5b channel-set sweep (full screen)
full-screen renders: 8704 with warnings/exceptions: 0
[]
non-contiguous sets tested: (0,2) (0,3) (1,3) (0,1,3) (0,2,3) (1,2,3 is contiguous)
```

Sweep coverage: algorithms ACID, EUCLID, SNAKE, STAGE, ARP, PATTERNS, DEMO; rows 1 and 2 for each; ROW 3 swept once (device table TB-3 BITE, 0..127) because it is not algorithm-dependent. Channel sets: all 15 non-empty subsets of CH1..CH4 (incl. non-contiguous), every selected channel in the set, selected muted / not, MIDI OFF / not, new_chan / not, playing / stopped, REC off / on with cursor 1..16.

### 5c Top-bar messages (every string the code can produce; the code has no message table, so scene strings plus TSV-derived strings and worst cases were fed through `topbar`)
```
| message | texts in panel | fonts | rightmost ink x (exclusive) | cut? |
|--|--|--|--|--|
| enc SELECT 'CH1 ACID' | ['SELECT', 'CH1 ACID'] | AF_S/AF_M | x1=224 | ok |
| enc SELECT 'CH2 ACID' | ['SELECT', 'CH2 ACID'] | AF_S/AF_M | x1=224 | ok |
| enc SELECT 'CH3 ACID' | ['SELECT', 'CH3 ACID'] | AF_S/AF_M | x1=224 | ok |
| enc SELECT 'CH4 ACID' | ['SELECT', 'CH4 ACID'] | AF_S/AF_M | x1=224 | ok |
| enc ALGO 'ACID AT STEP' (full name) | ['ALGO', 'ACID AT STEP'] | AF_S/AF_S | x1=219 | ok |
| enc SELECT 'CH1 EUCLID' | ['SELECT', 'CH1 EUCLID'] | AF_S/AF_S | x1=224 | ok |
| enc SELECT 'CH2 EUCLID' | ['SELECT', 'CH2 EUCLID'] | AF_S/AF_S | x1=224 | ok |
| enc SELECT 'CH3 EUCLID' | ['SELECT', 'CH3 EUCLID'] | AF_S/AF_S | x1=224 | ok |
| enc SELECT 'CH4 EUCLID' | ['SELECT', 'CH4 EUCLID'] | AF_S/AF_S | x1=224 | ok |
| enc ALGO 'EUCLID AT STEP' (full name) | ['ALGO', 'EUCLID AT STEP'] | AF_S/AF_S | x1=233 | ok |
| enc SELECT 'CH1 SNAKE' | ['SELECT', 'CH1 SNAKE'] | AF_S/AF_M | x1=238 | ok |
| enc SELECT 'CH2 SNAKE' | ['SELECT', 'CH2 SNAKE'] | AF_S/AF_M | x1=238 | ok |
| enc SELECT 'CH3 SNAKE' | ['SELECT', 'CH3 SNAKE'] | AF_S/AF_M | x1=238 | ok |
| enc SELECT 'CH4 SNAKE' | ['SELECT', 'CH4 SNAKE'] | AF_S/AF_M | x1=238 | ok |
| enc ALGO 'SNAKE AT STEP' (full name) | ['ALGO', 'SNAKE AT STEP'] | AF_S/AF_S | x1=230 | ok |
| enc SELECT 'CH1 STAGE' | ['SELECT', 'CH1 STAGE'] | AF_S/AF_M | x1=236 | ok |
| enc SELECT 'CH2 STAGE' | ['SELECT', 'CH2 STAGE'] | AF_S/AF_M | x1=236 | ok |
| enc SELECT 'CH3 STAGE' | ['SELECT', 'CH3 STAGE'] | AF_S/AF_M | x1=236 | ok |
| enc SELECT 'CH4 STAGE' | ['SELECT', 'CH4 STAGE'] | AF_S/AF_M | x1=236 | ok |
| enc ALGO 'STAGE AT STEP' (full name) | ['ALGO', 'STAGE AT STEP'] | AF_S/AF_S | x1=229 | ok |
| enc SELECT 'CH1 ARP' | ['SELECT', 'CH1 ARP'] | AF_S/AF_M | x1=219 | ok |
| enc SELECT 'CH2 ARP' | ['SELECT', 'CH2 ARP'] | AF_S/AF_M | x1=219 | ok |
| enc SELECT 'CH3 ARP' | ['SELECT', 'CH3 ARP'] | AF_S/AF_M | x1=219 | ok |
| enc SELECT 'CH4 ARP' | ['SELECT', 'CH4 ARP'] | AF_S/AF_M | x1=219 | ok |
| enc ALGO 'ARP AT STEP' (full name) | ['ALGO', 'ARP AT STEP'] | AF_S/AF_M | x1=234 | ok |
| enc SELECT 'CH1 PATTERNS' | ['SELECT', 'CH1 PATTERNS'] | AF_S/AF_S | x1=241 | CUT |
| enc SELECT 'CH2 PATTERNS' | ['SELECT', 'CH2 PATTERNS'] | AF_S/AF_S | x1=241 | CUT |
| enc SELECT 'CH3 PATTERNS' | ['SELECT', 'CH3 PATTERNS'] | AF_S/AF_S | x1=241 | CUT |
| enc SELECT 'CH4 PATTERNS' | ['SELECT', 'CH4 PATTERNS'] | AF_S/AF_S | x1=241 | CUT |
| enc ALGO 'PATTERNS AT STEP' (full name) | ['ALGO', 'PATTERNS AT STEP'] | AF_S/AF_S | x1=249 | CUT |
| enc SELECT 'CH1 DEMO' | ['SELECT', 'CH1 DEMO'] | AF_S/AF_M | x1=234 | ok |
| enc SELECT 'CH2 DEMO' | ['SELECT', 'CH2 DEMO'] | AF_S/AF_M | x1=234 | ok |
| enc SELECT 'CH3 DEMO' | ['SELECT', 'CH3 DEMO'] | AF_S/AF_M | x1=234 | ok |
| enc SELECT 'CH4 DEMO' | ['SELECT', 'CH4 DEMO'] | AF_S/AF_M | x1=234 | ok |
| enc ALGO 'DEMO AT STEP' (full name) | ['ALGO', 'DEMO AT STEP'] | AF_S/AF_S | x1=227 | ok |
| enc SELECT 'CH1 PATTERNS' | ['SELECT', 'CH1 PATTERNS'] | AF_S/AF_S | x1=241 | CUT |
| enc SELECT 'CH2 PATTERNS' | ['SELECT', 'CH2 PATTERNS'] | AF_S/AF_S | x1=241 | CUT |
| enc SELECT 'CH3 PATTERNS' | ['SELECT', 'CH3 PATTERNS'] | AF_S/AF_S | x1=241 | CUT |
| enc SELECT 'CH4 PATTERNS' | ['SELECT', 'CH4 PATTERNS'] | AF_S/AF_S | x1=241 | CUT |
| enc ALGO 'PATTERNS AT STEP' (full name) | ['ALGO', 'PATTERNS AT STEP'] | AF_S/AF_S | x1=249 | CUT |
| enc PRESETS 'P02 AT BAR' | ['PRESETS', 'P02 AT BAR'] | AF_S/AF_S | x1=228 | ok |
| enc PRESETS 'P02 LOADS AT BAR' | ['PRESETS', 'P02 LOADS AT BAR'] | AF_S/AF_S | x1=267 | CUT |
| enc PRESETS 'P16 AT BAR' | ['PRESETS', 'P16 AT BAR'] | AF_S/AF_S | x1=228 | ok |
| enc PRESETS 'P16 LOADS AT BAR' | ['PRESETS', 'P16 LOADS AT BAR'] | AF_S/AF_S | x1=267 | CUT |
| enc PRESETS 'P99 AT BAR' | ['PRESETS', 'P99 AT BAR'] | AF_S/AF_S | x1=228 | ok |
| enc PRESETS 'P99 LOADS AT BAR' | ['PRESETS', 'P99 LOADS AT BAR'] | AF_S/AF_S | x1=267 | CUT |
| enc OCT+ 'ROW 2 OF 2' | ['OCT+', 'ROW 2 OF 2'] | AF_S/AF_M | x1=227 | ok |
| enc OCT+ 'ROW 3 TB-3' | ['OCT+', 'ROW 3 TB-3'] | AF_S/AF_M | x1=229 | ok |
| enc OCT+ 'ROW 3 TB-3 BITE' | ['OCT+', 'ROW 3 TB-3 BITE'] | AF_S/AF_S | x1=238 | ok |
| enc OCT+ 'ROW 3 OF 3' | ['OCT+', 'ROW 3 OF 3'] | AF_S/AF_M | x1=227 | ok |
| enc OCT+ 'LAST ROW' | ['OCT+', 'LAST ROW'] | AF_S/AF_M | x1=220 | ok |
| enc OCT+ 'LAST ROW: OCT- = ROW 1' | ['OCT+', 'LAST ROW: OCT- = ROW 1'] | AF_S/AF_S | x1=285 | CUT |
| enc OCT- 'ROW 1 OF 2' | ['OCT-', 'ROW 1 OF 2'] | AF_S/AF_M | x1=225 | ok |
| hold 'RESET' | ['RESET'] | AF_M | x1=168 | ok |
| hold 'HARMONY' | ['HARMONY'] | AF_M | x1=198 | ok |
| hold 'HELP / MENU' | ['HELP / MENU'] | AF_M | x1=213 | ok |
| hold 'UNDO LOAD' | ['UNDO LOAD'] | AF_M | x1=207 | ok |
| hold 'REDO LOAD' | ['REDO LOAD'] | AF_M | x1=204 | ok |
| hold 'HLP/MN' | ['HLP/MN'] | AF_M | x1=179 | ok |
| toast 'MIDI OFF' | 'TAKEN' (chip) | ['4', 'MIDI OFF', 'TAKEN'] | AF_M/AF_M/AF_S | x1=237 | ok |
| toast 'NEW' | 'ALGO=PICK' (chip) | ['4', 'NEW', 'ALGO=PICK'] | AF_M/AF_M/AF_S | x1=237 | ok |
| toast 'NOTES' | 'EDIT=UNDO' (chip) | ['4', 'NOTES', 'EDIT=UNDO'] | AF_M/AF_S/AF_S | x1=237 | ok |
| toast 'ALL' | 'EDIT=UNDO' (chip) | ['4', 'ALL', 'EDIT=UNDO'] | AF_M/AF_M/AF_S | x1=237 | ok |
| toast 'ACCNT' | 'EDIT=UNDO' (chip) | ['4', 'ACCNT', 'EDIT=UNDO'] | AF_M/AF_S/AF_S | x1=237 | ok |
| toast 'SHAPE' | 'EDIT=UNDO' (chip) | ['4', 'SHAPE', 'EDIT=UNDO'] | AF_M/AF_S/AF_S | x1=237 | ok |
| toast 'ARTIC' | 'EDIT=UNDO' (chip) | ['4', 'ARTIC', 'EDIT=UNDO'] | AF_M/AF_S/AF_S | x1=237 | ok |
| toast 'HITS' | 'EDIT=UNDO' (chip) | ['4', 'HITS', 'EDIT=UNDO'] | AF_M/AF_M/AF_S | x1=237 | ok |
| toast 'PAST THE END' | '' (chip) | ['4', 'PAST THE END'] | AF_M/AF_M | x1=223 | ok |
| toast '64 STEPS = MAX' | '' (chip) | ['4', '64 STEPS = MAX'] | AF_M/AF_S | x1=214 | ok |
| toast 'NOTHING TO RESET' | '' (chip) | ['4', 'NOTHING TO RESET'] | AF_M/AF_S | x1=231 | ok |
| toast 'ONE PAGE: K1 = SHORTER' | '' (chip) | ['4', 'ONE PAGE: K1 = SHORTER'] | AF_M/AF_S | x1=264 | CUT |
| toast 'LAST ROW' | '' (chip) | ['4', 'LAST ROW'] | AF_M/AF_M | x1=198 | ok |
| toast 'HOME' | '' (chip) | ['4', 'HOME'] | AF_M/AF_M | x1=166 | ok |
| toast 'MUTE CH3' | '' (chip) | ['4', 'MUTE CH3'] | AF_M/AF_M | x1=198 | ok |
| toast 'PREVIEW CH4' | '' (chip) | ['4', 'PREVIEW CH4'] | AF_M/AF_M | x1=219 | ok |
messages tested: 75 cut: 15
AF_M width of 'HELP / MENU' = 90 'ALGO=PICK' = 80 'EUCLID' = 51 'PATTERNS' = 72
AF_S width of 'HELP / MENU' = 71 'ALGO=PICK' = 63 'EUCLID' = 40 'PATTERNS' = 56
hold text budget (fit maxw) = 114 px from x=124; enc budget = w - name chip - 8; toast budget = 240-4-hint-6-x
```

Facts: `fit()` chooses AF_M if it fits else AF_S and never checks that AF_S fits; there is no ellipsis. Strings that fit nowhere run past x=239 (cut by the panel edge / screen).


## 6 Extensibility
```
A.check() on LONGX:
   LONGX: short name 'LONGERNAME' does not fit the header
   LONGX LONGLABEL: label too wide
   LONGX WIDEWD: label too wide
   LONGX WIDEWD: value 'WWWWWW' too wide even in AF_M
   LONGX LONGFMT: label too wide
   LONGX LONGFMT: value '10 STEPSXXX' too wide even in AF_M
   LONGX ZERO: value 'KEYSONLYLONG' too wide even in AF_M
is_list: LIST20 False LIST10 True LIST11 False
row 1 warnings: ["text '2' is cut by the canvas (ink 234,4..243,16)", "text 'LONGLABEL' is cut by the canvas (ink -12,26..71,39)"]
row 2 warnings: ["text '2' is cut by the canvas (ink 234,4..243,16)", "text 'KEYSONLYLONG' is cut by the canvas (ink 153,49..267,62)"]
text outside its 58 px tile (any value): 213
   (1, 'LONGLABEL', 0, 'LONGLABEL', 'AF_M', (-12, 26, 71, 39))
   (1, 'LONGLABEL', 1, 'LONGLABEL', 'AF_M', (-12, 26, 71, 39))
   (1, 'LONGLABEL', 2, 'LONGLABEL', 'AF_M', (-12, 26, 71, 39))
   (1, 'LONGLABEL', 3, 'LONGLABEL', 'AF_M', (-12, 26, 71, 39))
   (1, 'LONGLABEL', 4, 'LONGLABEL', 'AF_M', (-12, 26, 71, 39))
   (1, 'LONGLABEL', 5, 'LONGLABEL', 'AF_M', (-12, 26, 71, 39))
   (1, 'WIDEWD', 0, 'WIDEWD', 'AF_M', (60, 27, 120, 38))
   (1, 'WIDEWD', 1, 'WIDEWD', 'AF_M', (60, 27, 120, 38))
   (1, 'WIDEWD', 1, 'WWWWWW', 'AF_M', (47, 50, 132, 61))
   (1, 'WIDEWD', 2, 'WIDEWD', 'AF_M', (60, 27, 120, 38))
   (1, 'WIDEWD', 2, 'MMMMMM', 'AF_M', (51, 50, 128, 61))
   (1, 'WIDEWD', 3, 'WIDEWD', 'AF_M', (60, 27, 120, 38))
  warning: text '2' is cut by the canvas (ink 234,4..243,16)
  warning: text 'LONGLABEL' is cut by the canvas (ink -12,26..71,39)
header with short 'LONGERNAME': [('120', (17, 4, 43, 17)), ('1', (49, 5, 54, 16)), ('2', (62, 4, 71, 16)), ('3', (76, 4, 85, 17)), ('4', (90, 5, 99, 16)), ('LONGERNAME', (105, 4, 203, 17)), ('MIDI', (209, 7, 232, 16)), ('2', (234, 4, 243, 16))]
10-entry dots: dw 3 px, row span 48
DEVICES keys: ['TB-3 BITE'] ; rig_row labels: ['CUT', 'RESO', 'ENV-X', 'ENV-Y']
ROW 3 with a 10-char label from a device entry: ink [('CUTOFFLONG', (-18, 26, 78, 39))] tile x1..58; A.check() does not look at DEVICES: True
```

Per-algorithm names/branches (line numbers):
- v11.py: no branch on an algorithm name in any drawing function. Algorithm names occur only as data: CHANS (L46-49), scene states (L438-452), DEMO registration (L29-38, hint `absent=("ACCNT",)`). One structural branch reads a hint: `a["lane"] == "single"` (L227); the dice dim reads `absent` (L254, L259).
- algos_v11.py: UI_EXTRA (L35-38): PATTERNS short "PATT" + value_fn PICK (patt_pick L26), EUCLID lane "single", STAGE value_fn TOTAL (stage_total L30); ORDER L145; MAX_DOTS L131 (is_list L136).
- ROW 3: driven by `DEVICES` (v11.py L105-107; one entry "TB-3 BITE" with four (label, default) pairs), `rig_row` (L110-112), `knob_row` (L116-122 selects it when row==2), `status` shows 3 row boxes when `S["rig"]` is set (L176). All four ROW 3 tiles are 0..127 meters; `A.check()` does not read DEVICES (shown above: result unchanged after adding a 10-character device label, which draws from x=-18).


## 7 Accuracy vs TSVs
```
| # | claim | consistent? | TSV evidence |
|--|--|--|--|
| 1 | button FX tap role 'ALL' | yes | TSV label=ALL |
| 2 | button FX has no hold role | yes | TSV hold: no hold role: acted once on press |
| 3 | button SCL tap role 'NOTES' | yes | TSV label=NOTES |
| 4 | button SCL has no hold role | yes | TSV hold: no hold role: acted once on press |
| 5 | button ENV tap role 'ACCNT' | yes | TSV label=ACCNT |
| 6 | button ENV has no hold role | yes | TSV hold: no hold role: acted once on press |
| 7 | button LFO tap role 'SHAPE' | yes | TSV label=SHAPE |
| 8 | button LFO has no hold role | yes | TSV hold: no hold role: acted once on press |
| 9 | button EDIT tap role 'UNDO' | yes | TSV label=UNDO |
| 10 | button EDIT hold role 'RESET' | yes | TSV hold label=RESET |
| 11 | button GLO tap role 'SET' | yes | TSV label=SET |
| 12 | button GLO hold role 'HARM' | yes | TSV hold label=HARM |
| 13 | button HOME tap role 'HOME' | yes | TSV label=HOME |
| 14 | button HOME hold role 'HLP/MN' | yes | TSV hold label=HLP/MN |
| 15 | button SAVE tap role 'SAVE' | yes | TSV label=SAVE |
| 16 | button SAVE hold role 'UNDO L' | yes | TSV hold label=UNDO L |
| 17 | button ARP tap role 'ARTIC' | yes | TSV label=ARTIC |
| 18 | button ARP has no hold role | yes | TSV hold: no hold role: acted once on press |
| 19 | button SEQ tap role 'HITS' | yes | TSV label=HITS |
| 20 | button SEQ has no hold role | yes | TSV hold: no hold role: acted once on press |
| 21 | button PLAY tap role 'PLAY' | yes | TSV label=PLAY |
| 22 | button PLAY has no hold role | yes | TSV hold: no hold role: acted once on press |
| 23 | button REC tap role 'REC' | yes | TSV label=REC |
| 24 | button REC has no hold role | yes | TSV hold: no hold role: acted once on press |
| 25 | hold panel hold_edit shows 'RESET' | yes | TSV label RESET; action: 0.4 s: open RESET (latched, one shot): the next control touched goes back to its default |
| 26 | hold panel hold_home shows 'HELP / MENU' | yes | TSV label HLP/MN; action: hold (acts on release; owner decisions 57 / 61): from 0.25 s the strip reads HOME: touch=HELP  let go=MENU; to |
| 27 | hold panel hold_glo shows 'HARMONY' | yes | TSV label HARM; action: 0.4 s: open HARMONY (latched): SCALE KEY VOICE MOVE, the progression strip, keys F3-E4 = chords 1-7 |
| 28 | hold panel hold_save shows 'UNDO LOAD' | yes | TSV label UNDO L; action: 0.7 s: undo the last preset load (the state AND the header slot go back), applied at the next step; again = re |

black keys: (x, screen role) vs key_map PERF
  15 M1 <-> KEY_B01_F#3 [('tap', 'M1'), ('hold', 'PRV1')] MATCH
  30 M2 <-> KEY_B02_G#3 [('tap', 'M2'), ('hold', 'PRV2')] MATCH
  45 M3 <-> KEY_B03_A#3 [('tap', 'M3'), ('hold', 'PRV3')] MATCH
  75 M4 <-> KEY_B04_C#4 [('tap', 'M4'), ('hold', 'PRV4')] MATCH
  90 CHN <-> KEY_B05_D#4 [('tap', 'CHN'), ('hold', 'RIG')] MATCH
  120 SOL <-> KEY_B06_F#4 [('hold', 'SOL')] MATCH
  135 FIL <-> KEY_B07_G#4 [('hold', 'FIL')] MATCH
  150 ROL <-> KEY_B08_A#4 [('hold', 'ROL')] MATCH
  180 PG- <-> KEY_B09_C#5 [('tap', 'PG-')] MATCH
  195 PG+ <-> KEY_B10_D#5 [('tap', 'PG+')] MATCH
  225 RST <-> KEY_B11_F#5 [('tap', 'RST')] MATCH
screen BLACK x list: [15, 30, 45, 75, 90, 120, 135, 150, 180, 195, 225]

REC knob_map: [('K1', 'CURSOR', '1..LENGTH, wraps'), ('K2', 'ACC', 'OFF ON'), ('K3', 'SLIDE', 'OFF ON'), ('K4', 'OCT', '-2..+2, default 0')]
screen REC_ROW: [('CURSOR', 1, 16, 1), ('ACC', 0, 1, 0), ('SLIDE', 0, 1, 0), ('OCT', -2, 2, 0)]
page_states REC cue_strip: ['REC: PLAY KEYS TO WRITE  REC=done'] | screen text: 'PLAY KEYS TO WRITE' + 'REC = DONE'
page_states REC screen: ['banner REC + lane with the write cursor + REC band: CURSOR ACC SLIDE OCT + KEYS = NOTES legend']
PERF cue/SELECT row: ['turn: select channel CH1..CH4 (clamped, no wrap)']
REC SELECT row: ['turn: select channel; the cursor goes to step 1']
ALGO_PICK: [('the page beneath, the selected channel cell and header H4 show the new short name, and the cue strip lists all six algorithms with the current one in brackets: ACID EUCL SNAK STAG [ARP] PATT (185 px of 240)', 'ACID EUCL SNAK STAG [ARP] PATT')]
ADD_PAGE: [('PG+ (D#5) while viewing the last page and length < 64', 'ADD PAGE 2 = COPY OF 1? PG+ = yes')]
CHN key PERF rows: [('tap', 'tap: open the CHAN page of the selected channel (LENGTH MIDI SOURCE FOLLOW)'), ('hold', '0.4 s: open the RIG page of the selected channel (latched; stays open when the key is released); nam')]
first-run: [('FIRST_RUN', 'the first-run step text (section 11); step 1 = D#4 K2: MIDI CHANNEL')]
CHAN page knobs: [('K1', 'LENGTH'), ('K2', 'MIDI'), ('K3', 'SOURCE'), ('K4', 'FOLLOW')]
```

Additional checked claims (each from the scenes against the TSV text):
| # | claim in v11 | TSV | result |
|--|--|--|--|
| 29 | SELECT toast `SELECT / CH2 EUCLID` | knob_map PERF SELECT 'CH n' | consistent |
| 30 | ALGO toast `ALGO / SNAKE AT STEP`; header keeps old name | 'lands at the next step' | consistent (toast text carries 'AT STEP') |
| 31 | PRESETS toast `PRESETS / P02 AT BAR`, pending box `P02?` while playing | 'on the first bar >= 1.5 s after the last detent'; stopped: loads 1.5 s after last detent | consistent for playing; no stopped wording; marker in status line, TSV says header (mismatch 3) |
| 32 | OCT+ toast `OCT+ / ROW 2 OF 2` (no RIG) | OCT+ ROW 1->2; ROW 2->3 only with a RIG | consistent |
| 33 | OCT+ toast `OCT+ / ROW 3 TB-3` and three row boxes with a RIG | 'ROW 3 ... only when the channel has a device with a wire that is not DEV' | consistent |
| 34 | OCT+ on ROW 3 | 'inert, toast LAST ROW: OCT- = ROW 1' | no scene; string would be cut (5c, x1=285) |
| 35 | status row boxes hidden in REC | REC OCT-/OCT+ = REC octave | consistent |
| 36 | REC K2 ACC / K3 SLIDE names OFF ON, K4 OCT -2..+2 default 0 | knob_map REC | consistent |
| 37 | `rec_playing` cursor 4, K1 value 4 | K1 CURSOR moves the cursor one step per detent | consistent |
| 38 | black keys M1..M4 = mute, square when sounding, x when muted | key_map tap = mute/unmute CHn | consistent |
| 39 | black key x positions 15,30,45,75,90,120,135,150,180,195,225 | key_map PERF order F#3..F#5 | all 11 consistent |
| 40 | dice taps and absent-layer dimming (`DEMO` ACCNT) | ARP tap 'dim if absent'; LFO 'never dim: every algorithm has it' | consistent (DEMO dims ENV, not LFO) |
| 41 | toast `MIDI OFF / TAKEN` with chip CH2 | knob_map CHAN MIDI: taking a channel sets the other to OFF, toast names it | consistent in intent |
| 42 | dice toast `NOTES / EDIT=UNDO` | B_EDIT tap = UNDO | consistent |
| 43 | white keys: `HOLD = EDIT` | key_map hold 0.4 s opens NOTE EDIT | consistent |


## 8 Coverage of PERF control x gesture rows
```
PERF control x gesture rows counted: 81
yes (persistent): 68 84% | only on touch/press/hold: 4 5% | no: 9 11%
yes + touch/hold: 72 89%
rows = no: [('B_OCTDN', 'hold'), ('B_OCTUP', 'hold'), ('B_OCTDN', 'hold-both'), ('B_OCTUP', 'hold-both'), ('M1', 'hold'), ('M2', 'hold'), ('M3', 'hold'), ('M4', 'hold'), ('CHN', 'hold')]
| B_FX | tap | Y | tile icon (ALL die); dice toast on press |
| B_SCL | tap | Y | tile icon (NOTES die); dice toast on press |
| B_ENV | tap | Y | tile icon (ACCNT die); dice toast on press |
| B_LFO | tap | Y | tile icon (SHAPE die); dice toast on press |
| B_ARP | tap | Y | tile icon (ARTIC die); dice toast on press |
| B_SEQ | tap | Y | tile icon (HITS die); dice toast on press |
| B_EDIT | tap | Y | undo icon |
| B_EDIT | hold | H | hold panel RESET (needs the 0.25 s hold) |
| B_GLO | tap | Y | settings icon |
| B_GLO | hold | H | hold panel HARMONY |
| B_HOME | tap | Y | home icon |
| B_HOME | hold | H | hold panel HELP / MENU |
| B_SAVE | tap | Y | save icon |
| B_SAVE | hold | H | hold panel UNDO LOAD |
| B_PLAY | tap | Y | play icon + header play/stop glyph |
| B_REC | tap | Y | rec ring icon + REC state |
| B_OCTDN | tap | Y | row boxes 1..n in the status line (OCT buttons themselves are not drawn) |
| B_OCTUP | tap | Y | row boxes; OCT+ toast ROW n |
| B_OCTDN | hold | N | peek: nothing shown |
| B_OCTUP | hold | N | peek: nothing shown |
| B_OCTDN | hold-both | N | UPDATE MODE / CALIBRATION |
| B_OCTUP | hold-both | N | UPDATE MODE / CALIBRATION |
| K1 | turn | Y | knob tile (label, value, meter) |
| K2 | turn | Y | knob tile (label, value, meter) |
| K3 | turn | Y | knob tile (label, value, meter) |
| K4 | turn | Y | knob tile (label, value, meter) |
| SELECT | turn | Y | channel chips + SELECT toast |
| ALGO | turn | Y | header algorithm name + ALGO toast (no six-algorithm cue list) |
| PRESET | turn | Y | P01 text in status line; Pnn? pending box |
| R1 | turn | Y | ROW 3 tiles (only when a RIG exists) |
| R2 | turn | Y | ROW 3 tiles (only when a RIG exists) |
| R3 | turn | Y | ROW 3 tiles (only when a RIG exists) |
| R4 | turn | Y | ROW 3 tiles (only when a RIG exists) |
| W01 | tap | Y | lane step bars + keybed step colours |
| W02 | tap | Y | lane step bars + keybed step colours |
| W03 | tap | Y | lane step bars + keybed step colours |
| W04 | tap | Y | lane step bars + keybed step colours |
| W05 | tap | Y | lane step bars + keybed step colours |
| W06 | tap | Y | lane step bars + keybed step colours |
| W07 | tap | Y | lane step bars + keybed step colours |
| W08 | tap | Y | lane step bars + keybed step colours |
| W09 | tap | Y | lane step bars + keybed step colours |
| W10 | tap | Y | lane step bars + keybed step colours |
| W11 | tap | Y | lane step bars + keybed step colours |
| W12 | tap | Y | lane step bars + keybed step colours |
| W13 | tap | Y | lane step bars + keybed step colours |
| W14 | tap | Y | lane step bars + keybed step colours |
| W15 | tap | Y | lane step bars + keybed step colours |
| W16 | tap | Y | lane step bars + keybed step colours |
| W01 | hold | Y | cue strip 'HOLD = EDIT' |
| W02 | hold | Y | cue strip 'HOLD = EDIT' |
| W03 | hold | Y | cue strip 'HOLD = EDIT' |
| W04 | hold | Y | cue strip 'HOLD = EDIT' |
| W05 | hold | Y | cue strip 'HOLD = EDIT' |
| W06 | hold | Y | cue strip 'HOLD = EDIT' |
| W07 | hold | Y | cue strip 'HOLD = EDIT' |
| W08 | hold | Y | cue strip 'HOLD = EDIT' |
| W09 | hold | Y | cue strip 'HOLD = EDIT' |
| W10 | hold | Y | cue strip 'HOLD = EDIT' |
| W11 | hold | Y | cue strip 'HOLD = EDIT' |
| W12 | hold | Y | cue strip 'HOLD = EDIT' |
| W13 | hold | Y | cue strip 'HOLD = EDIT' |
| W14 | hold | Y | cue strip 'HOLD = EDIT' |
| W15 | hold | Y | cue strip 'HOLD = EDIT' |
| W16 | hold | Y | cue strip 'HOLD = EDIT' |
| M1 | tap | Y | black-key square/x + chip diagonal |
| M2 | tap | Y | black-key square/x + chip diagonal |
| M3 | tap | Y | black-key square/x + chip diagonal |
| M4 | tap | Y | black-key square/x + chip diagonal |
| M1 | hold | N | PREVIEW (speaker icon / PRVn) not drawn |
| M2 | hold | N | PREVIEW (speaker icon / PRVn) not drawn |
| M3 | hold | N | PREVIEW (speaker icon / PRVn) not drawn |
| M4 | hold | N | PREVIEW (speaker icon / PRVn) not drawn |
| CHN | tap | Y | MIDI glyph on the key (lit white + cue when MIDI OFF/new channel) |
| CHN | hold | N | RIG page entry not drawn |
| SOL | hold | Y | letter S / F / R on the key |
| FIL | hold | Y | letter S / F / R on the key |
| ROL | hold | Y | letter S / F / R on the key |
| PG- | tap | Y | arrow / reset glyph on the key |
| PG+ | tap | Y | arrow / reset glyph on the key |
| RST | tap | Y | arrow / reset glyph on the key |
```

Method: rows = button_modes PERF (excluding rows whose action is 'no hold role'), knob_map PERF (K1..K4, SELECT, ALGO, PRESET, R1..R4), key_map PERF (16 white keys x tap/hold, M1..M4 x tap/hold, CHN tap/hold, SOL/FIL/ROL hold, PG-, PG+, RST). 'Y' = visible on the persistent PERF screen; 'H' = visible only while the control is touched / pressed / held; 'N' = not visible in any v11 scene. R1..R4 count as Y only when a RIG exists.


## 9 Firmware cost
```
distinct icon bitmaps drawn across all 29 scenes: 20 | bytes 1bpp (row-padded): 379 | bytes 8bpp: 2366
[('dice_accnt', '13x13', 26), ('dice_all', '13x13', 26), ('dice_artic', '13x13', 26), ('dice_hits', '13x13', 26), ('dice_notes', '13x13', 26), ('dice_shape', '13x13', 26), ('home', '13x11', 22), ('left', '5x7', 7), ('midi_s', '7x5', 5), ('play', '13x11', 22), ('play_s', '9x9', 18), ('rec', '13x11', 22), ('rec_ring', '13x11', 22), ('right', '5x7', 7), ('rst', '5x7', 7), ('save', '13x11', 22), ('settings', '13x11', 22), ('stop_s', '9x9', 18), ('undo', '13x11', 22), ('x7', '7x7', 7)]
distinct bitmaps defined in ICONS+IC (incl. unused): 38
HI-CON tokens: {'BG': '#000000', 'SURF': '#292829', 'MID': '#b4aeb4', 'DIM': '#6a696a', 'TEXT': '#fffaff', 'RAISE': '#414041', 'KEY': '#c5c2c5', 'REC': '#ff484a', 'ACCENT': '#00e6ff'}
colours defined in v11.py vs HI-CON tokens:
  CH1            #e69d00 OUTSIDE tokens
  CH2            #52b6ee OUTSIDE tokens
  CH3            #009d73 OUTSIDE tokens
  CH4            #cd79a4 OUTSIDE tokens
  WHITE          #ffffff OUTSIDE tokens
  INK            #000000 = a token
  PHC            #525052 OUTSIDE tokens
  DICE_BG        #4a484a OUTSIDE tokens
  OFF_RED        #de2829 OUTSIDE tokens
  mix(CH1,120)   #6a4800 OUTSIDE tokens
  mix(CH2,120)   #20556a OUTSIDE tokens
  mix(CH3,120)   #004831 OUTSIDE tokens
  mix(CH4,120)   #5a384a OUTSIDE tokens
distinct RGB565 values over 29 scenes incl. text ramps: 180 | in no-text renders: 20 | of those not a HI-CON token: 12
['#004831', '#009d73', '#20556a', '#4a484a', '#525052', '#52b6ee', '#6a4800', '#737573', '#cd79a4', '#de2829', '#e69d00', '#ffffff']
```

Notes: bytes are row-padded 1 bpp for the drawn glyph grids (width x height from the ASCII art); 8 bpp column is the unpadded pixel count. Colours: CH1..CH4, WHITE (#ffffff), PHC, DICE_BG, OFF_RED and the four mix(CH,120) fills are not HI-CON theme tokens (token set above); INK equals BG. `#737573` appears in the no-text renders and is not defined in v11.py (source not traced).
