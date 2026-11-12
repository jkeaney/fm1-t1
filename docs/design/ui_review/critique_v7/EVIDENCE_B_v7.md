# EVIDENCE_B_v7: measured evidence for proposals/v7 (240x240, MONO palette, facts only)

Method: every number below comes from scripts in `scripts/` (run `python3 scripts/s0_build.py` to regenerate this file). They import `mock.py` and `proposals/v7/v7.py`, wrap `el.P.{rect,frame,art,hatch,line,text}` and `Canvas.text_flags` to log every draw (text, font, fg, bg, position, caller), and re-render via `mock.render_state`/`v7.draw`. Cap heights: AF_S 9 px, AF_M 11 px, AF_L 21 px; 8 px/mm; arcmin = cap_mm / 350 * 3438.

## Key numbers and mismatches (index)

TOOLING FACT (affects every colour number):
- `mock.palettes()` parses `build/gen/ui_palettes.h` with `\w+`, which does not match the row name "HI-CON". The mock therefore has 9 palettes and "MONO" lands at index 8, while `UI_BW_INDEX` is 9. `Theme.set` applies the black-and-white special case only at index 9, so the PNGs in `v7/out` were produced with a MONO that is NOT the header's MONO. As rendered by the mock: MID #b4b2b4, DIM #6a696a, LINE #312c31, RAISE #202020, TINT #202020, KEY #c5c6c5, REC #ff484a. Per the header (idx 9): MID/SEL/KEY/REC = TEXT white, DIM/LINE/RAISE/LANE = UI_BW_GREY 0x528a (#525052), TINT/GRID = BG. Sections 3 report both ("mock" and "fw"). Pixels differing between the two renderings per scene: playing 17559, one_channel 17307, two_channels_stopped 17353, dice 17198, turning_k2 16779, rec_armed 18545 (of 57600).

1. Reproduction: 0 differing pixels in all six 1x scenes (playing, one_channel, two_channels_stopped, dice, turning_k2, rec_armed) against `out/v7_<scene>.png` (as rendered by the mock's MONO).
2. Text: draws per scene 20 / 18 / 19 / 21 / 22 / 15 (playing / one_channel / two_channels_stopped / dice / turning_k2 / rec_armed). In `dice` 6 header texts are fully overpainted by the toast (visible 15); in `turning_k2` 4 tile texts are overpainted by the turning box (visible 18). Fonts in `playing`: AF_S 5, AF_M 11, AF_L 4 (73 characters). Cap heights: AF_S 1.125 mm = 11.1 arcmin, AF_M 1.375 mm = 13.5 arcmin, AF_L 2.625 mm = 25.8 arcmin. Distinct text roles (caller, font, fg token) over the six scenes: 17. Icon bitmaps drawn per scene: 17 / 17 / 17 / 18 / 18 / 15 (12 button icons each).
3. Contrast, text: 16 distinct (font, fg, bg) pairs in "mock" MONO, 14 in "fw" MONO; none below 4.5:1 (AF_L 3:1). Lowest text pair 7.73 (mock, MID on RAISE, turning_k2) / 7.75 (fw, WHITE on #525052). In `playing` the ink boxes of label DENS and values 5 and 3 cross the meter-fill top edge (two background colours under the ink); worst underlying colour #6a3c10, ratio 8.99. Contrast, 61 non-text pairs tested, 24 below 3:1 in each variant (list in 3b). Selected:
   - knob meter fill mix(CH,110) vs empty RAISE tile: mock 1.76 / 1.86 / 2.14 / 1.61; fw 1.16 / 1.10 / 1.05 / 1.27 (CH1..CH4).
   - WHITE (diced-bar base, playhead outline) vs channel colours: 2.13 / 2.03 / 1.59 / 2.52.
   - T.DIM muted-bar outline and rest dash vs BG: mock 3.84, fw 2.63; vs PH column: mock 1.81, fw 1.24.
   - PH playhead column vs BG: 2.13. Dice tint vs BG 1.44; vs RAISE: mock 1.12, fw 1.82. Dice amber icon on tint: 9.87.
   - Black-key body LINE vs BG: mock 1.54, fw 2.63; RAISE vs BG: mock 1.29, fw 2.63.
   - REC scene: white playhead outline vs white key body T.KEY: mock 1.66, fw 1.03.
   - Hatched diced bar: 130 of 260 pixels white, 130 channel colour (checker, step 2).
   - Deuteranopia (Machado 2009, severity 1.0): simulated pair luminance delta CH2-CH4 0.0087 (WCAG ratio 1.02, CIE76 dE 42.5); CH1-CH3 hue dE 15.1 (lum delta 0.095, ratio 1.18). Normal-vision channel luminance ratios between pairs range 1.05 to 1.58.
4. Geometry (ink, half-open): header 1..20, knob tiles 24..76, row marker 80..85, music row 90..124 (ticks to 127), buttons 132..182 (rows 132..156 and 158..182), keys 188..222, caption 224..236 (rec: 225..235); band gaps 4, 4, 5, 5, 6 px; key-to-caption gap 2 px (rec 3). Margin top 1 px (0 with toast), bottom 4 px. No text ink box crosses y=120 and the mock raises no cut/ellipsis warning in the six scenes; the music row bars (y 90..123) straddle y=120 (19 rect/frame draws in `playing`, drawn in both strips). 1 px gaps: all 16 white keys (pitch 15, width 14), every adjacent music bar, tile-to-turning-box. Button cell 38x24 px (4.75 x 3.0 mm), pitch 40x26, gap 2 px; tiles 58x52 (gap 2); chips 13x19 (gap 2). Icons 13x13 (6 dice) and 13x11 (7 others; ink 9x9 stop, 11x9 undo ... see table); glyphs 5x7 (3), 11x11 (3), algorithm glyphs 11x6 / 9x7 / 9x5.
5. Edge sweep: 48 knob tiles (6 algorithms x K1-K4 + S1-S4). Labels (AF_M) max 55 px (LEGATO): all fit 58. AF_L widest values wider than 58 px: 15 of 48 tiles (KEYS 70 in all 6 K1/HITS cells that carry it, FWD/BWD/PEND 62/64/72, CURVE up to 78, TOTAL "48 (32)" 90, SHAPE up to 98, SPAN "1 OCT" 82, ARP DIR up to 97, PICK "HOUSE 3" 117 / "HOUSE 120" 153). AF_L has no "(" or ")" (only space # + - . / 0-9 A-Y); they render as glyph 0. Through `v7.screen` with the TSV widest values injected: 11 of 18 (algorithm x row) renders raise "text ... is cut by the canvas" (KEYS x=-4..64, PEND to 245, SAW+ to 248, CHORD to 257, HOUSE 120 from -46). As shipped, `el.KN` defines only ACID (2 rows) and EUCLID (1 row): row index 2 (ROW 3) raises IndexError for every algorithm, and EUCLID row index 1 (S1-S4) raises IndexError, and SNAKE/STAGE/ARP/PATTERNS draw ACID labels: 26 of 48 labels differ from knob_map.tsv (EUCLID 4, SNAKE 5, STAGE 6, ARP 5, PATTERNS 6). Header: `algo[:6]` draws "PATTER" for PATTERNS (full name 72 px would end x=199, over the pending frame at x=196); the other five names fit (ends x=155..178). Turning box: new value in AF_L overlaps the scale at x+66 for 2-digit values (11->12: right edge x0+67) and for any text value; old value (AF_M) crosses the arrow at x0+20 for 2+ characters; the box always reads row-1 labels. Channel sets: all 25 (set, selected) combinations rendered with no warnings or text overlaps; "+" absent in 7 sets with fewer than 4 channels ([2], [1,3], [2,3], [3,4], [1,2,4], [1,3,4], [2,3,4]); a used channel with an all-rest lane raises ValueError (min of empty list in `v6.music_slim`); muting the selected channel changes 1820 px, all in the music row (1784) and key glyphs (36), none in the header, tiles or chips; MIDI channel 16 vs 2: 0 px differ (field never drawn); 4 channels on one step: bars 2 px wide, hatch 2 px wide.
6. Accuracy: 65 claims checked against the TSVs (section 6). Mismatches / gaps: header "PATTER" vs ALGO name PATTERNS (knob_map.tsv:87; page_states.tsv:14 short form PATT); "Pnn>" queued-preset form (knob_map.tsv:88) not drawn; button hold roles (EDIT=RESET :75, GLO=HARM :93, HOME=HLP/MN :111, SAVE=UNDO L :129) not drawn; black-key hold roles PRV1-4 (:211 :224 :237 :250) and RIG (:263) not drawn; header stopped-state glyph is the REC circle; "PAGE 1/2" is a constant string; BPM 120 is a constant; EUCLID second row and ROW 3 data missing (knob_map.tsv:14-17, 113-116); MIDI, clock/tempo source, page map, length not drawn (page_states.tsv:2); REC-armed button colour T.REC is distinct from T.TEXT only in the mock's MONO (fw MONO: identical, 0 px differ in the REC cell). All 12 button tap roles, all 11 black-key roles/positions, 'KEYS = STEPS', 'HOLD = EDIT', 'ALL KEYS PLAY NOTES' (27 REC rows, 27 NOTE), ACID/EUCLID K-row labels, defaults and fill percentages, 'NOTES DICED' and 'EDIT=UNDO' match.
7. Coverage of PERF control x gesture (strict, shown in at least one scene): rows in the three TSVs 137 (button_modes 30, key_map 48, knob_map 59): yes 66 = 48.2%; excluding the 8 "no hold role" rows 66 of 129 = 51.2%. Control-level (knob rows collapsed to 15 controls): 93 units, yes 62 = 66.7%; excluding the 8: 62 of 85 = 72.9%. Buttons 12 of 30 rows, keys 43 of 48, knobs 11 of 59 rows (7 of 15 controls). Not shown at all: OCT- / OCT+ buttons (6 rows), S1-S4 (no scene with row 2), R1-R4, hold roles of EDIT/GLO/HOME/SAVE, PRV1-4, RIG.
8. Firmware cost: 13 button bitmaps in icons_v7 (310 bytes row-padded, 252 bytes packed); 20 distinct bitmaps drawn in the six scenes (409 bytes padded, 319 packed); 25 reachable with the six algorithm glyphs and the six `el.IC` bitmaps. Colours not palette tokens: 13 RGB565 values (12 not equal to any GREY token; 10 chromatic): channel 0xfca5 0x45ff 0x96ea 0xfb76, WHITE 0xffdf, INK 0x0000, PH 0x4228, DICE_BG 0x3142, amber 0xfe6b, knob fills 0x69e2 0x1a8d 0x3ae4 0x6969. GREY palette (mono=1): `_ramp` uses only the red 5-bit channel of fg and bg, so coloured text becomes neutral grey: CH1 and CH4 both peak #fffaff (contrast vs BG 19.43), CH3 #949194 (6.42), CH2 #414041 (1.94). 0 of 3874 text-ink pixels are chromatic; 14450 of 57600 pixels in the GREY render are still chromatic (rect/frame/bar fills).



## 1. Reproduction (1x PNG pixel diff vs proposals/v7/out)
- playing: size (240, 240) vs ref (240, 240), differing pixels = 0
- one_channel: size (240, 240) vs ref (240, 240), differing pixels = 0
- two_channels_stopped: size (240, 240) vs ref (240, 240), differing pixels = 0
- dice: size (240, 240) vs ref (240, 240), differing pixels = 0
- turning_k2: size (240, 240) vs ref (240, 240), differing pixels = 0
- rec_armed: size (240, 240) vs ref (240, 240), differing pixels = 0
- icons (sheet, 1x re-render; reference only exists at 4x: (960, 960)): not diffed at 1x

## 2. Text inventory
### scene playing: 20 text draws; warnings: none
| text | font | fg (token) | bg (token) | x | y(cap top) | ink bbox | cap mm | arcmin | caller | overpainted by later rect |
|---|---|---|---|---|---|---|---|---|---|---|
| '120' | AF_M | #ffffff TEXT | #000000 BG/INK | 16 | 5 | (17, 4, 43, 17) | 1.375 | 13.5 | enc_row | no |
| '1' | AF_M | #000000 BG/INK | #ff9529 CH1 | 52 | 5 | (53, 5, 58, 16) | 1.375 | 13.5 | enc_row | no |
| '2' | AF_M | #41beff CH2 | #000000 BG/INK | 67 | 5 | (67, 4, 76, 16) | 1.375 | 13.5 | enc_row | no |
| '3' | AF_M | #94de52 CH3 | #000000 BG/INK | 82 | 5 | (82, 4, 91, 17) | 1.375 | 13.5 | enc_row | no |
| '4' | AF_M | #ff6db4 CH4 | #000000 BG/INK | 97 | 5 | (97, 5, 106, 16) | 1.375 | 13.5 | enc_row | no |
| 'ACID' | AF_M | #ff9529 CH1 | #000000 BG/INK | 127 | 5 | (127, 4, 160, 17) | 1.375 | 13.5 | enc_row | no |
| 'P01' | AF_M | #ffffff TEXT | #000000 BG/INK | 209 | 5 | (209, 4, 233, 17) | 1.375 | 13.5 | enc_row | no |
| 'DENS' | AF_M | #fffaff WHITE | #202020 RAISE | 11 | 29 | (11, 28, 49, 41) | 1.375 | 13.5 | knobs_fill | no |
| 'SLIDE' | AF_M | #fffaff WHITE | #202020 RAISE | 70 | 29 | (70, 28, 108, 41) | 1.375 | 13.5 | knobs_fill | no |
| 'JUMP' | AF_M | #fffaff WHITE | #202020 RAISE | 130 | 29 | (130, 29, 170, 41) | 1.375 | 13.5 | knobs_fill | no |
| 'ACCNT' | AF_M | #fffaff WHITE | #202020 RAISE | 185 | 29 | (185, 28, 235, 41) | 1.375 | 13.5 | knobs_fill | no |
| '12' | AF_L | #fffaff WHITE | #6a3c10 mix | 12 | 46 | (15, 46, 47, 67) | 2.625 | 25.8 | knobs_fill | no |
| '4' | AF_L | #fffaff WHITE | #6a3c10 mix | 81 | 46 | (82, 46, 98, 67) | 2.625 | 25.8 | knobs_fill | no |
| '5' | AF_L | #fffaff WHITE | #202020 RAISE | 141 | 46 | (143, 46, 158, 68) | 2.625 | 25.8 | knobs_fill | no |
| '3' | AF_L | #fffaff WHITE | #202020 RAISE | 201 | 46 | (202, 46, 218, 68) | 2.625 | 25.8 | knobs_fill | no |
| 'S' | AF_S | #ffffff TEXT | #312c31 LINE | 116 | 195 | (116, 195, 123, 205) | 1.125 | 11.1 | footer_keybed | no |
| 'F' | AF_S | #ffffff TEXT | #312c31 LINE | 131 | 195 | (131, 195, 137, 204) | 1.125 | 11.1 | footer_keybed | no |
| 'R' | AF_S | #ffffff TEXT | #312c31 LINE | 146 | 195 | (146, 195, 153, 204) | 1.125 | 11.1 | footer_keybed | no |
| 'KEYS = STEPS' | AF_S | #ffffff TEXT | #000000 BG/INK | 2 | 225 | (2, 225, 78, 235) | 1.125 | 11.1 | footer_keybed | no |
| 'HOLD = EDIT  PAGE 1/2' | AF_S | #b4b2b4 MID | #000000 BG/INK | 115 | 225 | (115, 224, 238, 236) | 1.125 | 11.1 | footer_keybed | no |
totals per font: AF_L=4, AF_M=11, AF_S=5; total=20; characters=73
visible (not overpainted) text per font: AF_L=4, AF_M=11, AF_S=5; visible total=20
icon (art) draws: 17; sizes: {'11x11': 1, '11x6': 1, '13x13': 6, '13x11': 6, '5x7': 3}
icons by caller: {'enc_row': 2, 'buttons7': 12, 'footer_keybed': 3}
button icons=12; keybed icon bitmaps=3 (art draws in footer_keybed); header/other icon draws=2

### scene one_channel: 18 text draws; warnings: none
| text | font | fg (token) | bg (token) | x | y(cap top) | ink bbox | cap mm | arcmin | caller | overpainted by later rect |
|---|---|---|---|---|---|---|---|---|---|---|
| '120' | AF_M | #ffffff TEXT | #000000 BG/INK | 16 | 5 | (17, 4, 43, 17) | 1.375 | 13.5 | enc_row | no |
| '1' | AF_M | #000000 BG/INK | #ff9529 CH1 | 52 | 5 | (53, 5, 58, 16) | 1.375 | 13.5 | enc_row | no |
| '+' | AF_M | #b4b2b4 MID | #000000 BG/INK | 67 | 5 | (68, 8, 76, 16) | 1.375 | 13.5 | enc_row | no |
| 'ACID' | AF_M | #ff9529 CH1 | #000000 BG/INK | 127 | 5 | (127, 4, 160, 17) | 1.375 | 13.5 | enc_row | no |
| 'P01' | AF_M | #ffffff TEXT | #000000 BG/INK | 209 | 5 | (209, 4, 233, 17) | 1.375 | 13.5 | enc_row | no |
| 'DENS' | AF_M | #fffaff WHITE | #202020 RAISE | 11 | 29 | (11, 28, 49, 41) | 1.375 | 13.5 | knobs_fill | no |
| 'SLIDE' | AF_M | #fffaff WHITE | #202020 RAISE | 70 | 29 | (70, 28, 108, 41) | 1.375 | 13.5 | knobs_fill | no |
| 'JUMP' | AF_M | #fffaff WHITE | #202020 RAISE | 130 | 29 | (130, 29, 170, 41) | 1.375 | 13.5 | knobs_fill | no |
| 'ACCNT' | AF_M | #fffaff WHITE | #202020 RAISE | 185 | 29 | (185, 28, 235, 41) | 1.375 | 13.5 | knobs_fill | no |
| '12' | AF_L | #fffaff WHITE | #6a3c10 mix | 12 | 46 | (15, 46, 47, 67) | 2.625 | 25.8 | knobs_fill | no |
| '4' | AF_L | #fffaff WHITE | #6a3c10 mix | 81 | 46 | (82, 46, 98, 67) | 2.625 | 25.8 | knobs_fill | no |
| '5' | AF_L | #fffaff WHITE | #202020 RAISE | 141 | 46 | (143, 46, 158, 68) | 2.625 | 25.8 | knobs_fill | no |
| '3' | AF_L | #fffaff WHITE | #202020 RAISE | 201 | 46 | (202, 46, 218, 68) | 2.625 | 25.8 | knobs_fill | no |
| 'S' | AF_S | #ffffff TEXT | #312c31 LINE | 116 | 195 | (116, 195, 123, 205) | 1.125 | 11.1 | footer_keybed | no |
| 'F' | AF_S | #ffffff TEXT | #312c31 LINE | 131 | 195 | (131, 195, 137, 204) | 1.125 | 11.1 | footer_keybed | no |
| 'R' | AF_S | #ffffff TEXT | #312c31 LINE | 146 | 195 | (146, 195, 153, 204) | 1.125 | 11.1 | footer_keybed | no |
| 'KEYS = STEPS' | AF_S | #ffffff TEXT | #000000 BG/INK | 2 | 225 | (2, 225, 78, 235) | 1.125 | 11.1 | footer_keybed | no |
| 'HOLD = EDIT  PAGE 1/2' | AF_S | #b4b2b4 MID | #000000 BG/INK | 115 | 225 | (115, 224, 238, 236) | 1.125 | 11.1 | footer_keybed | no |
totals per font: AF_L=4, AF_M=9, AF_S=5; total=18; characters=71
visible (not overpainted) text per font: AF_L=4, AF_M=9, AF_S=5; visible total=18
icon (art) draws: 17; sizes: {'11x11': 1, '11x6': 1, '13x13': 6, '13x11': 6, '5x7': 3}
icons by caller: {'enc_row': 2, 'buttons7': 12, 'footer_keybed': 3}
button icons=12; keybed icon bitmaps=3 (art draws in footer_keybed); header/other icon draws=2

### scene two_channels_stopped: 19 text draws; warnings: none
| text | font | fg (token) | bg (token) | x | y(cap top) | ink bbox | cap mm | arcmin | caller | overpainted by later rect |
|---|---|---|---|---|---|---|---|---|---|---|
| '120' | AF_M | #ffffff TEXT | #000000 BG/INK | 16 | 5 | (17, 4, 43, 17) | 1.375 | 13.5 | enc_row | no |
| '1' | AF_M | #000000 BG/INK | #ff9529 CH1 | 52 | 5 | (53, 5, 58, 16) | 1.375 | 13.5 | enc_row | no |
| '2' | AF_M | #41beff CH2 | #000000 BG/INK | 67 | 5 | (67, 4, 76, 16) | 1.375 | 13.5 | enc_row | no |
| '+' | AF_M | #b4b2b4 MID | #000000 BG/INK | 82 | 5 | (83, 8, 91, 16) | 1.375 | 13.5 | enc_row | no |
| 'ACID' | AF_M | #ff9529 CH1 | #000000 BG/INK | 127 | 5 | (127, 4, 160, 17) | 1.375 | 13.5 | enc_row | no |
| 'P01' | AF_M | #ffffff TEXT | #000000 BG/INK | 209 | 5 | (209, 4, 233, 17) | 1.375 | 13.5 | enc_row | no |
| 'DENS' | AF_M | #fffaff WHITE | #202020 RAISE | 11 | 29 | (11, 28, 49, 41) | 1.375 | 13.5 | knobs_fill | no |
| 'SLIDE' | AF_M | #fffaff WHITE | #202020 RAISE | 70 | 29 | (70, 28, 108, 41) | 1.375 | 13.5 | knobs_fill | no |
| 'JUMP' | AF_M | #fffaff WHITE | #202020 RAISE | 130 | 29 | (130, 29, 170, 41) | 1.375 | 13.5 | knobs_fill | no |
| 'ACCNT' | AF_M | #fffaff WHITE | #202020 RAISE | 185 | 29 | (185, 28, 235, 41) | 1.375 | 13.5 | knobs_fill | no |
| '12' | AF_L | #fffaff WHITE | #6a3c10 mix | 12 | 46 | (15, 46, 47, 67) | 2.625 | 25.8 | knobs_fill | no |
| '4' | AF_L | #fffaff WHITE | #6a3c10 mix | 81 | 46 | (82, 46, 98, 67) | 2.625 | 25.8 | knobs_fill | no |
| '5' | AF_L | #fffaff WHITE | #202020 RAISE | 141 | 46 | (143, 46, 158, 68) | 2.625 | 25.8 | knobs_fill | no |
| '3' | AF_L | #fffaff WHITE | #202020 RAISE | 201 | 46 | (202, 46, 218, 68) | 2.625 | 25.8 | knobs_fill | no |
| 'S' | AF_S | #ffffff TEXT | #312c31 LINE | 116 | 195 | (116, 195, 123, 205) | 1.125 | 11.1 | footer_keybed | no |
| 'F' | AF_S | #ffffff TEXT | #312c31 LINE | 131 | 195 | (131, 195, 137, 204) | 1.125 | 11.1 | footer_keybed | no |
| 'R' | AF_S | #ffffff TEXT | #312c31 LINE | 146 | 195 | (146, 195, 153, 204) | 1.125 | 11.1 | footer_keybed | no |
| 'KEYS = STEPS' | AF_S | #ffffff TEXT | #000000 BG/INK | 2 | 225 | (2, 225, 78, 235) | 1.125 | 11.1 | footer_keybed | no |
| 'HOLD = EDIT  PAGE 1/2' | AF_S | #b4b2b4 MID | #000000 BG/INK | 115 | 225 | (115, 224, 238, 236) | 1.125 | 11.1 | footer_keybed | no |
totals per font: AF_L=4, AF_M=10, AF_S=5; total=19; characters=72
visible (not overpainted) text per font: AF_L=4, AF_M=10, AF_S=5; visible total=19
icon (art) draws: 17; sizes: {'11x11': 1, '11x6': 1, '13x13': 6, '13x11': 6, '5x7': 3}
icons by caller: {'enc_row': 2, 'buttons7': 12, 'footer_keybed': 3}
button icons=12; keybed icon bitmaps=3 (art draws in footer_keybed); header/other icon draws=2

### scene dice: 21 text draws; warnings: none
| text | font | fg (token) | bg (token) | x | y(cap top) | ink bbox | cap mm | arcmin | caller | overpainted by later rect |
|---|---|---|---|---|---|---|---|---|---|---|
| '120' | AF_M | #ffffff TEXT | #000000 BG/INK | 16 | 5 | (17, 4, 43, 17) | 1.375 | 13.5 | enc_row | YES |
| 'NOTES DICED' | AF_M | #000000 BG/INK | #fffaff WHITE | 20 | 5 | (20, 4, 113, 17) | 1.375 | 13.5 | toast_top | no |
| '1' | AF_M | #000000 BG/INK | #ff9529 CH1 | 52 | 5 | (53, 5, 58, 16) | 1.375 | 13.5 | enc_row | YES |
| '2' | AF_M | #41beff CH2 | #000000 BG/INK | 67 | 5 | (67, 4, 76, 16) | 1.375 | 13.5 | enc_row | YES |
| '4' | AF_M | #ff6db4 CH4 | #000000 BG/INK | 97 | 5 | (97, 5, 106, 16) | 1.375 | 13.5 | enc_row | YES |
| 'ACID' | AF_M | #ff9529 CH1 | #000000 BG/INK | 127 | 5 | (127, 4, 160, 17) | 1.375 | 13.5 | enc_row | YES |
| 'EDIT=UNDO' | AF_M | #000000 BG/INK | #fffaff WHITE | 153 | 5 | (153, 4, 236, 17) | 1.375 | 13.5 | toast_top | no |
| 'P01' | AF_M | #ffffff TEXT | #000000 BG/INK | 209 | 5 | (209, 4, 233, 17) | 1.375 | 13.5 | enc_row | YES |
| 'DENS' | AF_M | #fffaff WHITE | #202020 RAISE | 11 | 29 | (11, 28, 49, 41) | 1.375 | 13.5 | knobs_fill | no |
| 'SLIDE' | AF_M | #fffaff WHITE | #202020 RAISE | 70 | 29 | (70, 28, 108, 41) | 1.375 | 13.5 | knobs_fill | no |
| 'JUMP' | AF_M | #fffaff WHITE | #202020 RAISE | 130 | 29 | (130, 29, 170, 41) | 1.375 | 13.5 | knobs_fill | no |
| 'ACCNT' | AF_M | #fffaff WHITE | #202020 RAISE | 185 | 29 | (185, 28, 235, 41) | 1.375 | 13.5 | knobs_fill | no |
| '12' | AF_L | #fffaff WHITE | #6a3c10 mix | 12 | 46 | (15, 46, 47, 67) | 2.625 | 25.8 | knobs_fill | no |
| '4' | AF_L | #fffaff WHITE | #6a3c10 mix | 81 | 46 | (82, 46, 98, 67) | 2.625 | 25.8 | knobs_fill | no |
| '5' | AF_L | #fffaff WHITE | #202020 RAISE | 141 | 46 | (143, 46, 158, 68) | 2.625 | 25.8 | knobs_fill | no |
| '3' | AF_L | #fffaff WHITE | #202020 RAISE | 201 | 46 | (202, 46, 218, 68) | 2.625 | 25.8 | knobs_fill | no |
| 'S' | AF_S | #ffffff TEXT | #312c31 LINE | 116 | 195 | (116, 195, 123, 205) | 1.125 | 11.1 | footer_keybed | no |
| 'F' | AF_S | #ffffff TEXT | #312c31 LINE | 131 | 195 | (131, 195, 137, 204) | 1.125 | 11.1 | footer_keybed | no |
| 'R' | AF_S | #ffffff TEXT | #312c31 LINE | 146 | 195 | (146, 195, 153, 204) | 1.125 | 11.1 | footer_keybed | no |
| 'KEYS = STEPS' | AF_S | #ffffff TEXT | #000000 BG/INK | 2 | 225 | (2, 225, 78, 235) | 1.125 | 11.1 | footer_keybed | no |
| 'HOLD = EDIT  PAGE 1/2' | AF_S | #b4b2b4 MID | #000000 BG/INK | 115 | 225 | (115, 224, 238, 236) | 1.125 | 11.1 | footer_keybed | no |
totals per font: AF_L=4, AF_M=12, AF_S=5; total=21; characters=92
visible (not overpainted) text per font: AF_L=4, AF_M=6, AF_S=5; visible total=15
icon (art) draws: 18; sizes: {'11x11': 2, '11x6': 1, '13x13': 6, '13x11': 6, '5x7': 3}
icons by caller: {'enc_row': 2, 'buttons7': 12, 'toast_top': 1, 'footer_keybed': 3}
button icons=12; keybed icon bitmaps=3 (art draws in footer_keybed); header/other icon draws=3

### scene turning_k2: 22 text draws; warnings: none
| text | font | fg (token) | bg (token) | x | y(cap top) | ink bbox | cap mm | arcmin | caller | overpainted by later rect |
|---|---|---|---|---|---|---|---|---|---|---|
| '120' | AF_M | #ffffff TEXT | #000000 BG/INK | 16 | 5 | (17, 4, 43, 17) | 1.375 | 13.5 | enc_row | no |
| '1' | AF_M | #000000 BG/INK | #ff9529 CH1 | 52 | 5 | (53, 5, 58, 16) | 1.375 | 13.5 | enc_row | no |
| '2' | AF_M | #41beff CH2 | #000000 BG/INK | 67 | 5 | (67, 4, 76, 16) | 1.375 | 13.5 | enc_row | no |
| '4' | AF_M | #ff6db4 CH4 | #000000 BG/INK | 97 | 5 | (97, 5, 106, 16) | 1.375 | 13.5 | enc_row | no |
| 'ACID' | AF_M | #ff9529 CH1 | #000000 BG/INK | 127 | 5 | (127, 4, 160, 17) | 1.375 | 13.5 | enc_row | no |
| 'P01' | AF_M | #ffffff TEXT | #000000 BG/INK | 209 | 5 | (209, 4, 233, 17) | 1.375 | 13.5 | enc_row | no |
| 'K2 SLIDE' | AF_M | #ff9529 CH1 | #000000 BG/INK | 66 | 27 | (66, 26, 126, 39) | 1.375 | 13.5 | knob_turning | no |
| 'DENS' | AF_M | #b4b2b4 MID | #202020 RAISE | 11 | 28 | (11, 27, 49, 40) | 1.375 | 13.5 | knobs_num | no |
| 'SLIDE' | AF_M | #b4b2b4 MID | #202020 RAISE | 70 | 28 | (70, 27, 108, 40) | 1.375 | 13.5 | knobs_num | YES |
| 'JUMP' | AF_M | #b4b2b4 MID | #202020 RAISE | 130 | 28 | (130, 28, 170, 40) | 1.375 | 13.5 | knobs_num | YES |
| 'ACCNT' | AF_M | #b4b2b4 MID | #202020 RAISE | 185 | 28 | (185, 27, 235, 40) | 1.375 | 13.5 | knobs_num | no |
| '6' | AF_L | #fffaff WHITE | #000000 BG/INK | 92 | 39 | (93, 39, 109, 61) | 2.625 | 25.8 | knob_turning | no |
| '12' | AF_L | #fffaff WHITE | #202020 RAISE | 12 | 44 | (15, 44, 47, 65) | 2.625 | 25.8 | knobs_num | no |
| '4' | AF_M | #b4b2b4 MID | #000000 BG/INK | 66 | 44 | (66, 44, 75, 55) | 1.375 | 13.5 | knob_turning | no |
| '4' | AF_L | #fffaff WHITE | #202020 RAISE | 81 | 44 | (82, 44, 98, 65) | 2.625 | 25.8 | knobs_num | YES |
| '5' | AF_L | #fffaff WHITE | #202020 RAISE | 141 | 44 | (143, 44, 158, 66) | 2.625 | 25.8 | knobs_num | YES |
| '3' | AF_L | #fffaff WHITE | #202020 RAISE | 201 | 44 | (202, 44, 218, 66) | 2.625 | 25.8 | knobs_num | no |
| 'S' | AF_S | #ffffff TEXT | #312c31 LINE | 116 | 195 | (116, 195, 123, 205) | 1.125 | 11.1 | footer_keybed | no |
| 'F' | AF_S | #ffffff TEXT | #312c31 LINE | 131 | 195 | (131, 195, 137, 204) | 1.125 | 11.1 | footer_keybed | no |
| 'R' | AF_S | #ffffff TEXT | #312c31 LINE | 146 | 195 | (146, 195, 153, 204) | 1.125 | 11.1 | footer_keybed | no |
| 'KEYS = STEPS' | AF_S | #ffffff TEXT | #000000 BG/INK | 2 | 225 | (2, 225, 78, 235) | 1.125 | 11.1 | footer_keybed | no |
| 'HOLD = EDIT  PAGE 1/2' | AF_S | #b4b2b4 MID | #000000 BG/INK | 115 | 225 | (115, 224, 238, 236) | 1.125 | 11.1 | footer_keybed | no |
totals per font: AF_L=5, AF_M=12, AF_S=5; total=22; characters=82
visible (not overpainted) text per font: AF_L=3, AF_M=10, AF_S=5; visible total=18
icon (art) draws: 18; sizes: {'11x11': 1, '11x6': 1, '5x7': 4, '13x13': 6, '13x11': 6}
icons by caller: {'enc_row': 2, 'knob_turning': 1, 'buttons7': 12, 'footer_keybed': 3}
button icons=12; keybed icon bitmaps=3 (art draws in footer_keybed); header/other icon draws=3

### scene rec_armed: 15 text draws; warnings: none
| text | font | fg (token) | bg (token) | x | y(cap top) | ink bbox | cap mm | arcmin | caller | overpainted by later rect |
|---|---|---|---|---|---|---|---|---|---|---|
| '120' | AF_M | #ffffff TEXT | #000000 BG/INK | 16 | 5 | (17, 4, 43, 17) | 1.375 | 13.5 | enc_row | no |
| '1' | AF_M | #000000 BG/INK | #ff9529 CH1 | 52 | 5 | (53, 5, 58, 16) | 1.375 | 13.5 | enc_row | no |
| '2' | AF_M | #41beff CH2 | #000000 BG/INK | 67 | 5 | (67, 4, 76, 16) | 1.375 | 13.5 | enc_row | no |
| '4' | AF_M | #ff6db4 CH4 | #000000 BG/INK | 97 | 5 | (97, 5, 106, 16) | 1.375 | 13.5 | enc_row | no |
| 'ACID' | AF_M | #ff9529 CH1 | #000000 BG/INK | 127 | 5 | (127, 4, 160, 17) | 1.375 | 13.5 | enc_row | no |
| 'P01' | AF_M | #ffffff TEXT | #000000 BG/INK | 209 | 5 | (209, 4, 233, 17) | 1.375 | 13.5 | enc_row | no |
| 'DENS' | AF_M | #fffaff WHITE | #202020 RAISE | 11 | 29 | (11, 28, 49, 41) | 1.375 | 13.5 | knobs_fill | no |
| 'SLIDE' | AF_M | #fffaff WHITE | #202020 RAISE | 70 | 29 | (70, 28, 108, 41) | 1.375 | 13.5 | knobs_fill | no |
| 'JUMP' | AF_M | #fffaff WHITE | #202020 RAISE | 130 | 29 | (130, 29, 170, 41) | 1.375 | 13.5 | knobs_fill | no |
| 'ACCNT' | AF_M | #fffaff WHITE | #202020 RAISE | 185 | 29 | (185, 28, 235, 41) | 1.375 | 13.5 | knobs_fill | no |
| '12' | AF_L | #fffaff WHITE | #6a3c10 mix | 12 | 46 | (15, 46, 47, 67) | 2.625 | 25.8 | knobs_fill | no |
| '4' | AF_L | #fffaff WHITE | #6a3c10 mix | 81 | 46 | (82, 46, 98, 67) | 2.625 | 25.8 | knobs_fill | no |
| '5' | AF_L | #fffaff WHITE | #202020 RAISE | 141 | 46 | (143, 46, 158, 68) | 2.625 | 25.8 | knobs_fill | no |
| '3' | AF_L | #fffaff WHITE | #202020 RAISE | 201 | 46 | (202, 46, 218, 68) | 2.625 | 25.8 | knobs_fill | no |
| 'ALL KEYS PLAY NOTES' | AF_S | #ffffff TEXT | #000000 BG/INK | 16 | 225 | (16, 225, 138, 235) | 1.125 | 11.1 | footer_keybed | no |
totals per font: AF_L=4, AF_M=10, AF_S=1; total=15; characters=55
visible (not overpainted) text per font: AF_L=4, AF_M=10, AF_S=1; visible total=15
icon (art) draws: 15; sizes: {'11x11': 2, '11x6': 1, '13x13': 6, '13x11': 6}
icons by caller: {'enc_row': 2, 'buttons7': 12, 'footer_keybed': 1}
button icons=12; keybed icon bitmaps=1 (art draws in footer_keybed); header/other icon draws=2

### distinct text roles (caller function, font, fg token) over all six scenes: 17
- enc_row AF_M TEXT : in ['one_channel', 'playing', 'rec_armed', 'turning_k2', 'two_channels_stopped']
- enc_row AF_M BG/INK : in ['one_channel', 'playing', 'rec_armed', 'turning_k2', 'two_channels_stopped']
- enc_row AF_M CH2 : in ['playing', 'rec_armed', 'turning_k2', 'two_channels_stopped']
- enc_row AF_M CH3 : in ['playing']
- enc_row AF_M CH4 : in ['playing', 'rec_armed', 'turning_k2']
- enc_row AF_M CH1 : in ['one_channel', 'playing', 'rec_armed', 'turning_k2', 'two_channels_stopped']
- knobs_fill AF_M WHITE : in ['dice', 'one_channel', 'playing', 'rec_armed', 'two_channels_stopped']
- knobs_fill AF_L WHITE : in ['dice', 'one_channel', 'playing', 'rec_armed', 'two_channels_stopped']
- footer_keybed AF_S TEXT : in ['dice', 'one_channel', 'playing', 'rec_armed', 'turning_k2', 'two_channels_stopped']
- footer_keybed AF_S MID : in ['dice', 'one_channel', 'playing', 'turning_k2', 'two_channels_stopped']
- enc_row AF_M MID : in ['one_channel', 'two_channels_stopped']
- toast_top AF_M BG/INK : in ['dice']
- knob_turning AF_M CH1 : in ['turning_k2']
- knobs_num AF_M MID : in ['turning_k2']
- knob_turning AF_L WHITE : in ['turning_k2']
- knobs_num AF_L WHITE : in ['turning_k2']
- knob_turning AF_M MID : in ['turning_k2']

## 2b. non-text marks per scene (drawn ops by caller; hidden-by-overpaint ops not removed; art = bitmap icon draws)
| scene | art icons (button / header / key / toast / other) | key-role glyph marks on black keys (squares / 2x2 dots / letters / arrows+rst) | row-marker squares | channel chips (rect+frame) | knob tiles | music bars (rect/frame, h>=4) | rest dashes | hatch regions | lit white-key fronts |
|---|---|---|---|---|---|---|---|---|---|
| playing | 17 (12/2/3/0/0) | 4 / 4 / 3 / 3 | 3 | 4 | 4 | 27 | 2 | 0 | 10 |
| one_channel | 17 (12/2/3/0/0) | 1 / 4 / 3 / 3 | 3 | 1 | 4 | 10 | 6 | 0 | 10 |
| two_channels_stopped | 17 (12/2/3/0/0) | 2 / 4 / 3 / 3 | 3 | 2 | 4 | 15 | 4 | 0 | 10 |
| dice | 18 (12/2/3/1/0) | 3 / 4 / 3 / 3 | 3 | 3 | 4 | 19 | 3 | 6 | 10 |
| turning_k2 | 18 (12/2/3/0/1) | 3 / 4 / 3 / 3 | 3 | 3 | 4 | 19 | 3 | 0 | 10 |
| rec_armed | 15 (12/2/1/0/0) | 0 / 0 / 0 / 0 | 3 | 3 | 4 | 19 | 3 | 0 | 0 |

button icon draws: 12 per scene (6 dice-frame icons, undo, settings, home, save, play-or-stop, rec); header glyphs: transport icon + algorithm glyph; black-key glyphs in non-REC scenes: up to 4 mute squares + 1 CHN 4-dot + 3 letters + 3 5x7 icons = 11 keys with a mark; in REC scene: 0 black-key glyphs plus 1 rec icon in the caption line

## 2c. text margins inside the 58 px knob tiles (scene playing; tile k spans x=60k+1 .. 60k+58)
- 'DENS' AF_M ink x 11..49 in tile 1 (1..59): left margin 10, right margin 10
- '12' AF_L ink x 15..47 in tile 1 (1..59): left margin 14, right margin 12
- 'SLIDE' AF_M ink x 70..108 in tile 2 (61..119): left margin 9, right margin 11
- '4' AF_L ink x 82..98 in tile 2 (61..119): left margin 21, right margin 21
- 'JUMP' AF_M ink x 130..170 in tile 3 (121..179): left margin 9, right margin 9
- '5' AF_L ink x 143..158 in tile 3 (121..179): left margin 22, right margin 21
- 'ACCNT' AF_M ink x 185..235 in tile 4 (181..239): left margin 4, right margin 4
- '3' AF_L ink x 202..218 in tile 4 (181..239): left margin 21, right margin 21

# 3. Contrast (evidence)
Note: variant 'mock' is what mock.py renders for palette 'MONO' (its palette regex skips the HI-CON row, so MONO is index 8, not UI_BW_INDEX 9, and the BW special case in Theme.set is not applied). Variant 'fw' re-parses ui_palettes.h including HI-CON so MONO is index 9 (mid/sel/key/rec = text; dim/line/raise/lane = UI_BW_GREY 0x528a; tint/grid = bg). out/*.png were produced by 'mock'.

# VARIANT mock : palette=MONO mono_flag=False DIM=#6a696a LINE=#312c31 RAISE=#202020 MID=#b4b2b4 KEY=#c5c6c5 REC=#ff484a TINT=#202020
## 3a. text fg/bg pairs actually drawn (WCAG ratio; threshold 4.5, AF_L 3.0), visible texts only, all six scenes
| font | fg | bg | ratio | threshold | flag | texts | scenes |
|---|---|---|---|---|---|---|---|
| AF_M | #b4b2b4 MID | #202020 RAISE | 7.73 | 4.5 | ok | ['ACCNT', 'DENS'] | ['turning_k2'] |
| AF_M | #ff6db4 CH4 | #000000 BG/INK | 8.10 | 4.5 | ok | ['4'] | ['playing', 'rec_armed', 'turning_k2'] |
| AF_L | #fffaff WHITE | #6a3c10 mix | 8.99 | 3.0 | ok | ['12', '4'] | ['dice', 'one_channel', 'playing', 'rec_armed', 'two_channels_stopped'] |
| AF_M | #000000 BG/INK | #ff9529 CH1 | 9.58 | 4.5 | ok | ['1'] | ['one_channel', 'playing', 'rec_armed', 'turning_k2', 'two_channels_stopped'] |
| AF_M | #ff9529 CH1 | #000000 BG/INK | 9.58 | 4.5 | ok | ['ACID', 'K2 SLIDE'] | ['one_channel', 'playing', 'rec_armed', 'turning_k2', 'two_channels_stopped'] |
| AF_S | #b4b2b4 MID | #000000 BG/INK | 9.97 | 4.5 | ok | ['HOLD = EDIT  PAGE 1/2'] | ['dice', 'one_channel', 'playing', 'turning_k2', 'two_channels_stopped'] |
| AF_M | #b4b2b4 MID | #000000 BG/INK | 9.97 | 4.5 | ok | ['+', '4'] | ['one_channel', 'turning_k2', 'two_channels_stopped'] |
| AF_M | #41beff CH2 | #000000 BG/INK | 10.03 | 4.5 | ok | ['2'] | ['playing', 'rec_armed', 'turning_k2', 'two_channels_stopped'] |
| AF_M | #94de52 CH3 | #000000 BG/INK | 12.83 | 4.5 | ok | ['3'] | ['playing'] |
| AF_S | #ffffff TEXT | #312c31 LINE | 13.68 | 4.5 | ok | ['F', 'R', 'S'] | ['dice', 'one_channel', 'playing', 'turning_k2', 'two_channels_stopped'] |
| AF_M | #fffaff WHITE | #202020 RAISE | 15.80 | 4.5 | ok | ['ACCNT', 'DENS', 'JUMP', 'SLIDE'] | ['dice', 'one_channel', 'playing', 'rec_armed', 'two_channels_stopped'] |
| AF_L | #fffaff WHITE | #202020 RAISE | 15.80 | 3.0 | ok | ['12', '3', '5'] | ['dice', 'one_channel', 'playing', 'rec_armed', 'turning_k2', 'two_channels_stopped'] |
| AF_M | #000000 BG/INK | #fffaff WHITE | 20.37 | 4.5 | ok | ['EDIT=UNDO', 'NOTES DICED'] | ['dice'] |
| AF_L | #fffaff WHITE | #000000 BG/INK | 20.37 | 3.0 | ok | ['6'] | ['turning_k2'] |
| AF_M | #ffffff TEXT | #000000 BG/INK | 21.00 | 4.5 | ok | ['120', 'P01'] | ['one_channel', 'playing', 'rec_armed', 'turning_k2', 'two_channels_stopped'] |
| AF_S | #ffffff TEXT | #000000 BG/INK | 21.00 | 4.5 | ok | ['ALL KEYS PLAY NOTES', 'KEYS = STEPS'] | ['dice', 'one_channel', 'playing', 'rec_armed', 'turning_k2', 'two_channels_stopped'] |
distinct text pairs=16, below threshold=0

text whose ink box covers a pixel colour that differs from the declared bg and gives a lower ratio (no-text render underneath):
- scene playing text 'DENS' AF_M fg #fffaff declared bg #202020 worst underlying #6a3c10 ratio 8.99 (distinct bg colours under ink: 2)
- scene playing text '5' AF_L fg #fffaff declared bg #202020 worst underlying #6a3c10 ratio 8.99 (distinct bg colours under ink: 2)
- scene playing text '3' AF_L fg #fffaff declared bg #202020 worst underlying #6a3c10 ratio 8.99 (distinct bg colours under ink: 2)

## 3b. meaning-bearing non-text pairs (WCAG ratio; flag < 3:1)
| pair | A | B | ratio | flag |
|---|---|---|---|---|
| CH1 vs BG (chip frame, bars, algo glyph) | #ff9529 | #000000 | 9.58 | ok |
| CH1 vs RAISE (knob tile / unlit key / button) | #ff9529 | #202020 | 7.44 | ok |
| CH1 vs LINE (glyph on black-key body) | #ff9529 | #312c31 | 6.24 | ok |
| CH1 vs PH (bar on playhead column) | #ff9529 | #414441 | 4.50 | ok |
| CH1 knob-fill mix(CH,110) vs RAISE (meter fill vs empty) | #6a3c10 | #202020 | 1.76 | BELOW 3:1 |
| CH1 vs WHITE (diced bar white base / playhead outline on lit key) | #ff9529 | #fffaff | 2.13 | BELOW 3:1 |
| CH1 vs T.MID (row-marker filled vs outline colour) | #ff9529 | #b4b2b4 | 1.04 | BELOW 3:1 |
| CH1 vs T.DIM | #ff9529 | #6a696a | 2.49 | BELOW 3:1 |
| CH2 vs BG (chip frame, bars, algo glyph) | #41beff | #000000 | 10.03 | ok |
| CH2 vs RAISE (knob tile / unlit key / button) | #41beff | #202020 | 7.79 | ok |
| CH2 vs LINE (glyph on black-key body) | #41beff | #312c31 | 6.54 | ok |
| CH2 vs PH (bar on playhead column) | #41beff | #414441 | 4.72 | ok |
| CH2 knob-fill mix(CH,110) vs RAISE (meter fill vs empty) | #18506a | #202020 | 1.86 | BELOW 3:1 |
| CH2 vs WHITE (diced bar white base / playhead outline on lit key) | #41beff | #fffaff | 2.03 | BELOW 3:1 |
| CH2 vs T.MID (row-marker filled vs outline colour) | #41beff | #b4b2b4 | 1.01 | BELOW 3:1 |
| CH2 vs T.DIM | #41beff | #6a696a | 2.61 | BELOW 3:1 |
| CH3 vs BG (chip frame, bars, algo glyph) | #94de52 | #000000 | 12.83 | ok |
| CH3 vs RAISE (knob tile / unlit key / button) | #94de52 | #202020 | 9.95 | ok |
| CH3 vs LINE (glyph on black-key body) | #94de52 | #312c31 | 8.36 | ok |
| CH3 vs PH (bar on playhead column) | #94de52 | #414441 | 6.03 | ok |
| CH3 knob-fill mix(CH,110) vs RAISE (meter fill vs empty) | #395d20 | #202020 | 2.14 | BELOW 3:1 |
| CH3 vs WHITE (diced bar white base / playhead outline on lit key) | #94de52 | #fffaff | 1.59 | BELOW 3:1 |
| CH3 vs T.MID (row-marker filled vs outline colour) | #94de52 | #b4b2b4 | 1.29 | BELOW 3:1 |
| CH3 vs T.DIM | #94de52 | #6a696a | 3.34 | ok |
| CH4 vs BG (chip frame, bars, algo glyph) | #ff6db4 | #000000 | 8.10 | ok |
| CH4 vs RAISE (knob tile / unlit key / button) | #ff6db4 | #202020 | 6.28 | ok |
| CH4 vs LINE (glyph on black-key body) | #ff6db4 | #312c31 | 5.28 | ok |
| CH4 vs PH (bar on playhead column) | #ff6db4 | #414441 | 3.81 | ok |
| CH4 knob-fill mix(CH,110) vs RAISE (meter fill vs empty) | #6a2c4a | #202020 | 1.61 | BELOW 3:1 |
| CH4 vs WHITE (diced bar white base / playhead outline on lit key) | #ff6db4 | #fffaff | 2.52 | BELOW 3:1 |
| CH4 vs T.MID (row-marker filled vs outline colour) | #ff6db4 | #b4b2b4 | 1.23 | BELOW 3:1 |
| CH4 vs T.DIM | #ff6db4 | #6a696a | 2.11 | BELOW 3:1 |
| T.DIM (muted bar outline) vs BG | #6a696a | #000000 | 3.84 | ok |
| T.DIM (muted bar outline) vs PH (muted bar on playhead col) | #6a696a | #414441 | 1.81 | BELOW 3:1 |
| T.DIM (rest dash, 3x1 px) vs BG | #6a696a | #000000 | 3.84 | ok |
| T.DIM (rest dash) vs PH | #6a696a | #414441 | 1.81 | BELOW 3:1 |
| T.MID (row-marker outline, page tick marks) vs BG | #b4b2b4 | #000000 | 9.97 | ok |
| T.MID (muted-channel key glyph frame) vs T.LINE (black-key body) | #b4b2b4 | #312c31 | 6.49 | ok |
| T.LINE (black-key body) vs BG | #312c31 | #000000 | 1.54 | BELOW 3:1 |
| T.RAISE (white-key body / knob tile / button) vs BG | #202020 | #000000 | 1.29 | BELOW 3:1 |
| T.KEY (white-key body in REC) vs BG | #c5c6c5 | #000000 | 12.26 | ok |
| T.DIM (black-key body in REC) vs BG | #6a696a | #000000 | 3.84 | ok |
| PH (playhead column) vs BG | #414441 | #000000 | 2.13 | BELOW 3:1 |
| DICE icon amber (255,205,90) vs DICE_BG | #ffce5a | #312810 | 9.87 | ok |
| DICE_BG tint vs BG (button group tint) | #312810 | #000000 | 1.44 | BELOW 3:1 |
| DICE_BG tint vs RAISE (dice vs non-dice button cell) | #312810 | #202020 | 1.12 | BELOW 3:1 |
| TEXT icon vs RAISE (non-dice button icon) | #ffffff | #202020 | 16.29 | ok |
| INK icon vs WHITE (lit button) | #000000 | #fffaff | 20.37 | ok |
| WHITE playhead outline vs RAISE (unlit key) | #fffaff | #202020 | 15.80 | ok |
| WHITE playhead outline vs BG | #fffaff | #000000 | 20.37 | ok |
| WHITE accent cap (2 px) vs BG | #fffaff | #000000 | 20.37 | ok |
| WHITE accent cap vs PH | #fffaff | #414441 | 9.57 | ok |
| lit-key front CH1 vs unlit key RAISE (step on/off) | #ff9529 | #202020 | 7.44 | ok |
| T.LINE (turning-box scale track) vs BG | #312c31 | #000000 | 1.54 | BELOW 3:1 |
| T.MID (turning-box ticks) vs BG | #b4b2b4 | #000000 | 9.97 | ok |
| pending-preset frame WHITE vs BG | #fffaff | #000000 | 20.37 | ok |
| REC button fg T.REC vs RAISE (rec armed) | #ff484a | #202020 | 4.87 | ok |
| WHITE playhead outline vs T.KEY (white key body in REC scene; outline still drawn when playing) | #fffaff | #c5c6c5 | 1.66 | BELOW 3:1 |
| T.DIM black-key body vs T.KEY white key (REC) | #6a696a | #c5c6c5 | 3.19 | ok |
| REC key-caption icon CH1 vs BG | #ff9529 | #000000 | 9.58 | ok |
| TEXT vs T.LINE (S/F/R letters on black key) | #ffffff | #312c31 | 13.68 | ok |
pairs=61 below 3:1=24
hatched diced bar sample (13x20 at 31,104): pixel colours {'#ff9529 CH1': 130, '#fffaff WHITE': 130} of 260
hatch ops in dice scene: 6; hatch step values: [2]

# VARIANT fw : palette=MONO mono_flag=False DIM=#525052 LINE=#525052 RAISE=#525052 MID=#ffffff KEY=#ffffff REC=#ffffff TINT=#000000
## 3a. text fg/bg pairs actually drawn (WCAG ratio; threshold 4.5, AF_L 3.0), visible texts only, all six scenes
| font | fg | bg | ratio | threshold | flag | texts | scenes |
|---|---|---|---|---|---|---|---|
| AF_M | #fffaff WHITE | #525052 DIM/LINE/RAISE | 7.75 | 4.5 | ok | ['ACCNT', 'DENS', 'JUMP', 'SLIDE'] | ['dice', 'one_channel', 'playing', 'rec_armed', 'two_channels_stopped'] |
| AF_L | #fffaff WHITE | #525052 DIM/LINE/RAISE | 7.75 | 3.0 | ok | ['12', '3', '5'] | ['dice', 'one_channel', 'playing', 'rec_armed', 'turning_k2', 'two_channels_stopped'] |
| AF_S | #ffffff TEXT/MID/KEY/REC | #525052 DIM/LINE/RAISE | 7.99 | 4.5 | ok | ['F', 'R', 'S'] | ['dice', 'one_channel', 'playing', 'turning_k2', 'two_channels_stopped'] |
| AF_M | #ffffff TEXT/MID/KEY/REC | #525052 DIM/LINE/RAISE | 7.99 | 4.5 | ok | ['ACCNT', 'DENS'] | ['turning_k2'] |
| AF_M | #ff6db4 CH4 | #000000 BG/INK | 8.10 | 4.5 | ok | ['4'] | ['playing', 'rec_armed', 'turning_k2'] |
| AF_L | #fffaff WHITE | #6a3c10 mix | 8.99 | 3.0 | ok | ['12', '4'] | ['dice', 'one_channel', 'playing', 'rec_armed', 'two_channels_stopped'] |
| AF_M | #000000 BG/INK | #ff9529 CH1 | 9.58 | 4.5 | ok | ['1'] | ['one_channel', 'playing', 'rec_armed', 'turning_k2', 'two_channels_stopped'] |
| AF_M | #ff9529 CH1 | #000000 BG/INK | 9.58 | 4.5 | ok | ['ACID', 'K2 SLIDE'] | ['one_channel', 'playing', 'rec_armed', 'turning_k2', 'two_channels_stopped'] |
| AF_M | #41beff CH2 | #000000 BG/INK | 10.03 | 4.5 | ok | ['2'] | ['playing', 'rec_armed', 'turning_k2', 'two_channels_stopped'] |
| AF_M | #94de52 CH3 | #000000 BG/INK | 12.83 | 4.5 | ok | ['3'] | ['playing'] |
| AF_M | #000000 BG/INK | #fffaff WHITE | 20.37 | 4.5 | ok | ['EDIT=UNDO', 'NOTES DICED'] | ['dice'] |
| AF_L | #fffaff WHITE | #000000 BG/INK | 20.37 | 3.0 | ok | ['6'] | ['turning_k2'] |
| AF_M | #ffffff TEXT/MID/KEY/REC | #000000 BG/INK | 21.00 | 4.5 | ok | ['+', '120', '4', 'P01'] | ['one_channel', 'playing', 'rec_armed', 'turning_k2', 'two_channels_stopped'] |
| AF_S | #ffffff TEXT/MID/KEY/REC | #000000 BG/INK | 21.00 | 4.5 | ok | ['ALL KEYS PLAY NOTES', 'HOLD = EDIT  PAGE 1/2', 'KEYS = STEPS'] | ['dice', 'one_channel', 'playing', 'rec_armed', 'turning_k2', 'two_channels_stopped'] |
distinct text pairs=14, below threshold=0

text whose ink box covers a pixel colour that differs from the declared bg and gives a lower ratio (no-text render underneath):
- scene playing text '4' AF_L fg #fffaff declared bg #6a3c10 worst underlying #525052 ratio 7.75 (distinct bg colours under ink: 2)

## 3b. meaning-bearing non-text pairs (WCAG ratio; flag < 3:1)
| pair | A | B | ratio | flag |
|---|---|---|---|---|
| CH1 vs BG (chip frame, bars, algo glyph) | #ff9529 | #000000 | 9.58 | ok |
| CH1 vs RAISE (knob tile / unlit key / button) | #ff9529 | #525052 | 3.65 | ok |
| CH1 vs LINE (glyph on black-key body) | #ff9529 | #525052 | 3.65 | ok |
| CH1 vs PH (bar on playhead column) | #ff9529 | #414441 | 4.50 | ok |
| CH1 knob-fill mix(CH,110) vs RAISE (meter fill vs empty) | #6a3c10 | #525052 | 1.16 | BELOW 3:1 |
| CH1 vs WHITE (diced bar white base / playhead outline on lit key) | #ff9529 | #fffaff | 2.13 | BELOW 3:1 |
| CH1 vs T.MID (row-marker filled vs outline colour) | #ff9529 | #ffffff | 2.19 | BELOW 3:1 |
| CH1 vs T.DIM | #ff9529 | #525052 | 3.65 | ok |
| CH2 vs BG (chip frame, bars, algo glyph) | #41beff | #000000 | 10.03 | ok |
| CH2 vs RAISE (knob tile / unlit key / button) | #41beff | #525052 | 3.82 | ok |
| CH2 vs LINE (glyph on black-key body) | #41beff | #525052 | 3.82 | ok |
| CH2 vs PH (bar on playhead column) | #41beff | #414441 | 4.72 | ok |
| CH2 knob-fill mix(CH,110) vs RAISE (meter fill vs empty) | #18506a | #525052 | 1.10 | BELOW 3:1 |
| CH2 vs WHITE (diced bar white base / playhead outline on lit key) | #41beff | #fffaff | 2.03 | BELOW 3:1 |
| CH2 vs T.MID (row-marker filled vs outline colour) | #41beff | #ffffff | 2.09 | BELOW 3:1 |
| CH2 vs T.DIM | #41beff | #525052 | 3.82 | ok |
| CH3 vs BG (chip frame, bars, algo glyph) | #94de52 | #000000 | 12.83 | ok |
| CH3 vs RAISE (knob tile / unlit key / button) | #94de52 | #525052 | 4.88 | ok |
| CH3 vs LINE (glyph on black-key body) | #94de52 | #525052 | 4.88 | ok |
| CH3 vs PH (bar on playhead column) | #94de52 | #414441 | 6.03 | ok |
| CH3 knob-fill mix(CH,110) vs RAISE (meter fill vs empty) | #395d20 | #525052 | 1.05 | BELOW 3:1 |
| CH3 vs WHITE (diced bar white base / playhead outline on lit key) | #94de52 | #fffaff | 1.59 | BELOW 3:1 |
| CH3 vs T.MID (row-marker filled vs outline colour) | #94de52 | #ffffff | 1.64 | BELOW 3:1 |
| CH3 vs T.DIM | #94de52 | #525052 | 4.88 | ok |
| CH4 vs BG (chip frame, bars, algo glyph) | #ff6db4 | #000000 | 8.10 | ok |
| CH4 vs RAISE (knob tile / unlit key / button) | #ff6db4 | #525052 | 3.08 | ok |
| CH4 vs LINE (glyph on black-key body) | #ff6db4 | #525052 | 3.08 | ok |
| CH4 vs PH (bar on playhead column) | #ff6db4 | #414441 | 3.81 | ok |
| CH4 knob-fill mix(CH,110) vs RAISE (meter fill vs empty) | #6a2c4a | #525052 | 1.27 | BELOW 3:1 |
| CH4 vs WHITE (diced bar white base / playhead outline on lit key) | #ff6db4 | #fffaff | 2.52 | BELOW 3:1 |
| CH4 vs T.MID (row-marker filled vs outline colour) | #ff6db4 | #ffffff | 2.59 | BELOW 3:1 |
| CH4 vs T.DIM | #ff6db4 | #525052 | 3.08 | ok |
| T.DIM (muted bar outline) vs BG | #525052 | #000000 | 2.63 | BELOW 3:1 |
| T.DIM (muted bar outline) vs PH (muted bar on playhead col) | #525052 | #414441 | 1.24 | BELOW 3:1 |
| T.DIM (rest dash, 3x1 px) vs BG | #525052 | #000000 | 2.63 | BELOW 3:1 |
| T.DIM (rest dash) vs PH | #525052 | #414441 | 1.24 | BELOW 3:1 |
| T.MID (row-marker outline, page tick marks) vs BG | #ffffff | #000000 | 21.00 | ok |
| T.MID (muted-channel key glyph frame) vs T.LINE (black-key body) | #ffffff | #525052 | 7.99 | ok |
| T.LINE (black-key body) vs BG | #525052 | #000000 | 2.63 | BELOW 3:1 |
| T.RAISE (white-key body / knob tile / button) vs BG | #525052 | #000000 | 2.63 | BELOW 3:1 |
| T.KEY (white-key body in REC) vs BG | #ffffff | #000000 | 21.00 | ok |
| T.DIM (black-key body in REC) vs BG | #525052 | #000000 | 2.63 | BELOW 3:1 |
| PH (playhead column) vs BG | #414441 | #000000 | 2.13 | BELOW 3:1 |
| DICE icon amber (255,205,90) vs DICE_BG | #ffce5a | #312810 | 9.87 | ok |
| DICE_BG tint vs BG (button group tint) | #312810 | #000000 | 1.44 | BELOW 3:1 |
| DICE_BG tint vs RAISE (dice vs non-dice button cell) | #312810 | #525052 | 1.82 | BELOW 3:1 |
| TEXT icon vs RAISE (non-dice button icon) | #ffffff | #525052 | 7.99 | ok |
| INK icon vs WHITE (lit button) | #000000 | #fffaff | 20.37 | ok |
| WHITE playhead outline vs RAISE (unlit key) | #fffaff | #525052 | 7.75 | ok |
| WHITE playhead outline vs BG | #fffaff | #000000 | 20.37 | ok |
| WHITE accent cap (2 px) vs BG | #fffaff | #000000 | 20.37 | ok |
| WHITE accent cap vs PH | #fffaff | #414441 | 9.57 | ok |
| lit-key front CH1 vs unlit key RAISE (step on/off) | #ff9529 | #525052 | 3.65 | ok |
| T.LINE (turning-box scale track) vs BG | #525052 | #000000 | 2.63 | BELOW 3:1 |
| T.MID (turning-box ticks) vs BG | #ffffff | #000000 | 21.00 | ok |
| pending-preset frame WHITE vs BG | #fffaff | #000000 | 20.37 | ok |
| REC button fg T.REC vs RAISE (rec armed) | #ffffff | #525052 | 7.99 | ok |
| WHITE playhead outline vs T.KEY (white key body in REC scene; outline still drawn when playing) | #fffaff | #ffffff | 1.03 | BELOW 3:1 |
| T.DIM black-key body vs T.KEY white key (REC) | #525052 | #ffffff | 7.99 | ok |
| REC key-caption icon CH1 vs BG | #ff9529 | #000000 | 9.58 | ok |
| TEXT vs T.LINE (S/F/R letters on black key) | #ffffff | #525052 | 7.99 | ok |
pairs=61 below 3:1=24
hatched diced bar sample (13x20 at 31,104): pixel colours {'#ff9529 CH1': 130, '#fffaff WHITE': 130} of 260
hatch ops in dice scene: 6; hatch step values: [2]

## 3c. deuteranopia simulation of the four channel colours
Method: expand RGB565 to 8-bit as mock.to_rgb888, sRGB->linear, apply Machado, Oliveira & Fernandes (2009) deuteranopia matrix at severity 1.0 [[0.367322,0.860646,-0.227968],[0.280085,0.672501,0.047413],[-0.011820,0.042940,0.968881]] in linear RGB, clip 0..1, back to sRGB. Luminance = WCAG relative luminance (0.2126R+0.7152G+0.0722B, linear). deltaE = CIE76 on the simulated colours (D65).
| ch | RGB565->RGB888 | normal rel. luminance | simulated RGB | simulated rel. luminance |
|---|---|---|---|---|
| CH1 | (255, 149, 41) #ff9529 | 0.4291 | (207,185,41) | 0.4793 |
| CH2 | (65, 190, 255) #41beff | 0.4517 | (133,171,254) | 0.4135 |
| CH3 | (148, 222, 82) #94de52 | 0.5915 | (220,200,93) | 0.5741 |
| CH4 | (255, 109, 180) #ff6db4 | 0.3549 | (169,170,177) | 0.4048 |
| pair | normal: dL (rel lum) | normal: WCAG ratio | deut: dL | deut: WCAG ratio | deut: deltaE76 | normal: deltaE76 |
|---|---|---|---|---|---|---|
| CH1-CH2 | 0.0226 | 1.05 | 0.0657 | 1.14 | 115.2 | 119.5 |
| CH1-CH3 | 0.1623 | 1.34 | 0.0949 | 1.18 | 15.1 | 79.4 |
| CH1-CH4 | 0.0742 | 1.18 | 0.0744 | 1.16 | 72.9 | 83.8 |
| CH2-CH3 | 0.1398 | 1.28 | 0.1606 | 1.35 | 101.7 | 106.7 |
| CH2-CH4 | 0.0968 | 1.24 | 0.0087 | 1.02 | 42.5 | 82.5 |
| CH3-CH4 | 0.2366 | 1.58 | 0.1693 | 1.37 | 59.7 | 129.2 |

Simulated channel colours vs black BG (WCAG): CH1 10.59, CH2 9.27, CH3 12.48, CH4 9.10

# 4. Geometry

## scene playing
| band (caller fn) | x-range | y-range (ink, half-open) | height px |
|---|---|---|---|
| enc_row | 3..233 | 1..20 | 19 |
| knobs_fill | 1..239 | 24..76 | 52 |
| row_marker | 109..128 | 80..85 | 5 |
| music_slim | 1..239 | 90..127 | 37 |
| buttons7 | 1..239 | 132..182 | 50 |
| footer_keybed | 1..240 | 188..236 | 48 |
gaps between consecutive bands (empty px rows between bbox end and next bbox start; negative = overlap):
- enc_row -> knobs_fill : 4 px
- knobs_fill -> row_marker : 4 px
- row_marker -> music_slim : 5 px
- music_slim -> buttons7 : 5 px
- buttons7 -> footer_keybed : 6 px
rendered pixel rows with any non-BG pixel, runs (half-open): [(1, 20), (24, 76), (80, 85), (90, 124), (125, 127), (132, 156), (158, 182), (188, 222), (224, 236)] ; empty-row gaps: [(20, 24, 4), (76, 80, 4), (85, 90, 5), (124, 125, 1), (127, 132, 5), (156, 158, 2), (182, 188, 6), (222, 224, 2)]
rows 0..239 empty at top: 1, bottom: 4
text ink boxes crossing y=120: none; text cut warnings: none; mock warnings: none
nearest visible text to y=120: [(63.0, '3', (202, 46, 218, 68)), (63.0, '5', (143, 46, 158, 68))]
ops (rect/frame/art) straddling y=120 (drawn in both strips): {'music_slim:frame': 7, 'music_slim:rect': 12}
element pairs (non-contained) with 0 <= gap < 2 px (gap = empty px between bboxes; 0 = touching), grouped:
- footer_keybed/frame ~ footer_keybed/rect gap=1 : 3 pair(s), e.g. ((76, 188, 90, 222), (91, 188, 105, 222))
- footer_keybed/rect ~ footer_keybed/frame gap=1 : 2 pair(s), e.g. ((61, 188, 75, 222), (76, 188, 90, 222))
- footer_keybed/rect ~ footer_keybed/rect gap=1 : 53 pair(s), e.g. ((1, 188, 15, 222), (16, 188, 30, 222))
music_slim intra-row pairs with gap<2: {'rect~rect gap=1': 16, 'rect~frame gap=1': 8, 'frame~rect gap=1': 4}
partially overlapping (not contained) element pairs, grouped:
- footer_keybed/frame ~ footer_keybed/rect : 10 pair(s), e.g. ((76, 188, 90, 222), (69, 188, 81, 209))
- footer_keybed/rect ~ footer_keybed/art : 6 pair(s), e.g. ((166, 188, 180, 222), (179, 195, 183, 202))
- footer_keybed/rect ~ footer_keybed/frame : 3 pair(s), e.g. ((31, 188, 45, 222), (42, 196, 48, 202))
- footer_keybed/rect ~ footer_keybed/rect : 50 pair(s), e.g. ((1, 188, 15, 222), (9, 188, 21, 209))
- footer_keybed/rect ~ footer_keybed/text : 6 pair(s), e.g. ((106, 188, 120, 222), (116, 195, 123, 205))
- knobs_fill/rect ~ knobs_fill/text : 4 pair(s), e.g. ((1, 37, 59, 76), (11, 28, 49, 41))

## scene one_channel
| band (caller fn) | x-range | y-range (ink, half-open) | height px |
|---|---|---|---|
| enc_row | 3..233 | 1..20 | 19 |
| knobs_fill | 1..239 | 24..76 | 52 |
| row_marker | 109..128 | 80..85 | 5 |
| music_slim | 1..234 | 90..127 | 37 |
| buttons7 | 1..239 | 132..182 | 50 |
| footer_keybed | 1..240 | 188..236 | 48 |
gaps between consecutive bands (empty px rows between bbox end and next bbox start; negative = overlap):
- enc_row -> knobs_fill : 4 px
- knobs_fill -> row_marker : 4 px
- row_marker -> music_slim : 5 px
- music_slim -> buttons7 : 5 px
- buttons7 -> footer_keybed : 6 px
rendered pixel rows with any non-BG pixel, runs (half-open): [(1, 20), (24, 76), (80, 85), (90, 124), (125, 127), (132, 156), (158, 182), (188, 222), (224, 236)] ; empty-row gaps: [(20, 24, 4), (76, 80, 4), (85, 90, 5), (124, 125, 1), (127, 132, 5), (156, 158, 2), (182, 188, 6), (222, 224, 2)]
rows 0..239 empty at top: 1, bottom: 4
text ink boxes crossing y=120: none; text cut warnings: none; mock warnings: none
nearest visible text to y=120: [(63.0, '3', (202, 46, 218, 68)), (63.0, '5', (143, 46, 158, 68))]
ops (rect/frame/art) straddling y=120 (drawn in both strips): {'music_slim:rect': 9}
element pairs (non-contained) with 0 <= gap < 2 px (gap = empty px between bboxes; 0 = touching), grouped:
- footer_keybed/frame ~ footer_keybed/rect gap=1 : 3 pair(s), e.g. ((76, 188, 90, 222), (91, 188, 105, 222))
- footer_keybed/rect ~ footer_keybed/frame gap=1 : 2 pair(s), e.g. ((61, 188, 75, 222), (76, 188, 90, 222))
- footer_keybed/rect ~ footer_keybed/rect gap=1 : 53 pair(s), e.g. ((1, 188, 15, 222), (16, 188, 30, 222))
music_slim intra-row pairs with gap<2: {'rect~rect gap=1': 8}
partially overlapping (not contained) element pairs, grouped:
- footer_keybed/frame ~ footer_keybed/rect : 8 pair(s), e.g. ((76, 188, 90, 222), (69, 188, 81, 209))
- footer_keybed/rect ~ footer_keybed/art : 6 pair(s), e.g. ((166, 188, 180, 222), (179, 195, 183, 202))
- footer_keybed/rect ~ footer_keybed/frame : 1 pair(s), e.g. ((76, 210, 90, 222), (77, 189, 89, 221))
- footer_keybed/rect ~ footer_keybed/rect : 46 pair(s), e.g. ((1, 188, 15, 222), (9, 188, 21, 209))
- footer_keybed/rect ~ footer_keybed/text : 6 pair(s), e.g. ((106, 188, 120, 222), (116, 195, 123, 205))
- knobs_fill/rect ~ knobs_fill/text : 4 pair(s), e.g. ((1, 37, 59, 76), (11, 28, 49, 41))

## scene two_channels_stopped
| band (caller fn) | x-range | y-range (ink, half-open) | height px |
|---|---|---|---|
| enc_row | 4..233 | 1..20 | 19 |
| knobs_fill | 1..239 | 24..76 | 52 |
| row_marker | 109..128 | 80..85 | 5 |
| music_slim | 1..234 | 90..127 | 37 |
| buttons7 | 1..239 | 132..182 | 50 |
| footer_keybed | 1..240 | 188..236 | 48 |
gaps between consecutive bands (empty px rows between bbox end and next bbox start; negative = overlap):
- enc_row -> knobs_fill : 4 px
- knobs_fill -> row_marker : 4 px
- row_marker -> music_slim : 5 px
- music_slim -> buttons7 : 5 px
- buttons7 -> footer_keybed : 6 px
rendered pixel rows with any non-BG pixel, runs (half-open): [(1, 20), (24, 76), (80, 85), (90, 124), (125, 127), (132, 156), (158, 182), (188, 222), (224, 236)] ; empty-row gaps: [(20, 24, 4), (76, 80, 4), (85, 90, 5), (124, 125, 1), (127, 132, 5), (156, 158, 2), (182, 188, 6), (222, 224, 2)]
rows 0..239 empty at top: 1, bottom: 4
text ink boxes crossing y=120: none; text cut warnings: none; mock warnings: none
nearest visible text to y=120: [(63.0, '3', (202, 46, 218, 68)), (63.0, '5', (143, 46, 158, 68))]
ops (rect/frame/art) straddling y=120 (drawn in both strips): {'music_slim:rect': 9}
element pairs (non-contained) with 0 <= gap < 2 px (gap = empty px between bboxes; 0 = touching), grouped:
- footer_keybed/rect ~ footer_keybed/rect gap=1 : 53 pair(s), e.g. ((1, 188, 15, 222), (16, 188, 30, 222))
music_slim intra-row pairs with gap<2: {'rect~rect gap=1': 17}
partially overlapping (not contained) element pairs, grouped:
- footer_keybed/rect ~ footer_keybed/art : 6 pair(s), e.g. ((166, 188, 180, 222), (179, 195, 183, 202))
- footer_keybed/rect ~ footer_keybed/rect : 48 pair(s), e.g. ((1, 188, 15, 222), (9, 188, 21, 209))
- footer_keybed/rect ~ footer_keybed/text : 6 pair(s), e.g. ((106, 188, 120, 222), (116, 195, 123, 205))
- knobs_fill/rect ~ knobs_fill/text : 4 pair(s), e.g. ((1, 37, 59, 76), (11, 28, 49, 41))

## scene dice
| band (caller fn) | x-range | y-range (ink, half-open) | height px |
|---|---|---|---|
| toast_top | 0..240 | 0..20 | 20 |
| enc_row | 3..125 | 1..20 | 19 |
| knobs_fill | 1..239 | 24..76 | 52 |
| row_marker | 109..128 | 80..85 | 5 |
| music_slim | 1..234 | 90..127 | 37 |
| buttons7 | 1..239 | 132..182 | 50 |
| footer_keybed | 1..240 | 188..236 | 48 |
gaps between consecutive bands (empty px rows between bbox end and next bbox start; negative = overlap):
- toast_top -> enc_row : -19 px
- enc_row -> knobs_fill : 4 px
- knobs_fill -> row_marker : 4 px
- row_marker -> music_slim : 5 px
- music_slim -> buttons7 : 5 px
- buttons7 -> footer_keybed : 6 px
rendered pixel rows with any non-BG pixel, runs (half-open): [(0, 20), (24, 76), (80, 85), (90, 124), (125, 127), (132, 156), (158, 182), (188, 222), (224, 236)] ; empty-row gaps: [(20, 24, 4), (76, 80, 4), (85, 90, 5), (124, 125, 1), (127, 132, 5), (156, 158, 2), (182, 188, 6), (222, 224, 2)]
rows 0..239 empty at top: 0, bottom: 4
text ink boxes crossing y=120: none; text cut warnings: none; mock warnings: none
nearest visible text to y=120: [(63.0, '3', (202, 46, 218, 68)), (63.0, '5', (143, 46, 158, 68))]
ops (rect/frame/art) straddling y=120 (drawn in both strips): {'music_slim:rect': 12}
element pairs (non-contained) with 0 <= gap < 2 px (gap = empty px between bboxes; 0 = touching), grouped:
- enc_row/art ~ toast_top/text gap=1 : 1 pair(s), e.g. ((114, 7, 125, 13), (20, 4, 113, 17))
- footer_keybed/frame ~ footer_keybed/rect gap=1 : 3 pair(s), e.g. ((76, 188, 90, 222), (91, 188, 105, 222))
- footer_keybed/rect ~ footer_keybed/frame gap=1 : 2 pair(s), e.g. ((61, 188, 75, 222), (76, 188, 90, 222))
- footer_keybed/rect ~ footer_keybed/rect gap=1 : 53 pair(s), e.g. ((1, 188, 15, 222), (16, 188, 30, 222))
music_slim intra-row pairs with gap<2: {'rect~rect gap=1': 22}
partially overlapping (not contained) element pairs, grouped:
- enc_row/art ~ toast_top/art : 1 pair(s), e.g. ((3, 5, 13, 14), (5, 4, 13, 14))
- enc_row/frame ~ toast_top/text : 2 pair(s), e.g. ((65, 1, 78, 20), (20, 4, 113, 17))
- enc_row/rect ~ toast_top/text : 1 pair(s), e.g. ((50, 1, 63, 20), (20, 4, 113, 17))
- footer_keybed/frame ~ footer_keybed/rect : 10 pair(s), e.g. ((76, 188, 90, 222), (69, 188, 81, 209))
- footer_keybed/rect ~ footer_keybed/art : 6 pair(s), e.g. ((166, 188, 180, 222), (179, 195, 183, 202))
- footer_keybed/rect ~ footer_keybed/frame : 1 pair(s), e.g. ((76, 210, 90, 222), (77, 189, 89, 221))
- footer_keybed/rect ~ footer_keybed/rect : 50 pair(s), e.g. ((1, 188, 15, 222), (9, 188, 21, 209))
- footer_keybed/rect ~ footer_keybed/text : 6 pair(s), e.g. ((106, 188, 120, 222), (116, 195, 123, 205))
- knobs_fill/rect ~ knobs_fill/text : 4 pair(s), e.g. ((1, 37, 59, 76), (11, 28, 49, 41))

## scene turning_k2
| band (caller fn) | x-range | y-range (ink, half-open) | height px |
|---|---|---|---|
| enc_row | 3..233 | 1..20 | 19 |
| knob_turning | 60..180 | 22..78 | 56 |
| knobs_num | 1..239 | 24..76 | 52 |
| row_marker | 109..128 | 80..85 | 5 |
| music_slim | 1..234 | 90..127 | 37 |
| buttons7 | 1..239 | 132..182 | 50 |
| footer_keybed | 1..240 | 188..236 | 48 |
gaps between consecutive bands (empty px rows between bbox end and next bbox start; negative = overlap):
- enc_row -> knob_turning : 2 px
- knob_turning -> knobs_num : -54 px
- knobs_num -> row_marker : 4 px
- row_marker -> music_slim : 5 px
- music_slim -> buttons7 : 5 px
- buttons7 -> footer_keybed : 6 px
rendered pixel rows with any non-BG pixel, runs (half-open): [(1, 20), (22, 78), (80, 85), (90, 124), (125, 127), (132, 156), (158, 182), (188, 222), (224, 236)] ; empty-row gaps: [(20, 22, 2), (78, 80, 2), (85, 90, 5), (124, 125, 1), (127, 132, 5), (156, 158, 2), (182, 188, 6), (222, 224, 2)]
rows 0..239 empty at top: 1, bottom: 4
text ink boxes crossing y=120: none; text cut warnings: none; mock warnings: none
nearest visible text to y=120: [(65.0, '3', (202, 44, 218, 66)), (65.5, '12', (15, 44, 47, 65))]
ops (rect/frame/art) straddling y=120 (drawn in both strips): {'music_slim:rect': 12}
element pairs (non-contained) with 0 <= gap < 2 px (gap = empty px between bboxes; 0 = touching), grouped:
- footer_keybed/frame ~ footer_keybed/rect gap=1 : 3 pair(s), e.g. ((76, 188, 90, 222), (91, 188, 105, 222))
- footer_keybed/rect ~ footer_keybed/frame gap=1 : 2 pair(s), e.g. ((61, 188, 75, 222), (76, 188, 90, 222))
- footer_keybed/rect ~ footer_keybed/rect gap=1 : 53 pair(s), e.g. ((1, 188, 15, 222), (16, 188, 30, 222))
- knob_turning/text ~ knob_turning/text gap=0 : 1 pair(s), e.g. ((66, 26, 126, 39), (93, 39, 109, 61))
- knobs_num/rect ~ knob_turning/frame gap=1 : 2 pair(s), e.g. ((1, 24, 59, 76), (60, 22, 180, 78))
- knobs_num/rect ~ knob_turning/rect gap=0 : 4 pair(s), e.g. ((126, 70, 174, 73), (126, 67, 174, 70))
- knobs_num/rect ~ knob_turning/rect gap=1 : 2 pair(s), e.g. ((1, 24, 59, 76), (60, 22, 180, 78))
music_slim intra-row pairs with gap<2: {'rect~rect gap=1': 22}
partially overlapping (not contained) element pairs, grouped:
- footer_keybed/frame ~ footer_keybed/rect : 10 pair(s), e.g. ((76, 188, 90, 222), (69, 188, 81, 209))
- footer_keybed/rect ~ footer_keybed/art : 6 pair(s), e.g. ((166, 188, 180, 222), (179, 195, 183, 202))
- footer_keybed/rect ~ footer_keybed/frame : 1 pair(s), e.g. ((76, 210, 90, 222), (77, 189, 89, 221))
- footer_keybed/rect ~ footer_keybed/rect : 50 pair(s), e.g. ((1, 188, 15, 222), (9, 188, 21, 209))
- footer_keybed/rect ~ footer_keybed/text : 6 pair(s), e.g. ((106, 188, 120, 222), (116, 195, 123, 205))
- knobs_num/rect ~ knob_turning/text : 2 pair(s), e.g. ((61, 24, 119, 76), (66, 26, 126, 39))

## scene rec_armed
| band (caller fn) | x-range | y-range (ink, half-open) | height px |
|---|---|---|---|
| enc_row | 3..233 | 1..20 | 19 |
| knobs_fill | 1..239 | 24..76 | 52 |
| row_marker | 109..128 | 80..85 | 5 |
| music_slim | 1..234 | 90..127 | 37 |
| buttons7 | 1..239 | 132..182 | 50 |
| footer_keybed | 1..240 | 188..235 | 47 |
gaps between consecutive bands (empty px rows between bbox end and next bbox start; negative = overlap):
- enc_row -> knobs_fill : 4 px
- knobs_fill -> row_marker : 4 px
- row_marker -> music_slim : 5 px
- music_slim -> buttons7 : 5 px
- buttons7 -> footer_keybed : 6 px
rendered pixel rows with any non-BG pixel, runs (half-open): [(1, 20), (24, 76), (80, 85), (90, 124), (125, 127), (132, 156), (158, 182), (188, 222), (225, 235)] ; empty-row gaps: [(20, 24, 4), (76, 80, 4), (85, 90, 5), (124, 125, 1), (127, 132, 5), (156, 158, 2), (182, 188, 6), (222, 225, 3)]
rows 0..239 empty at top: 1, bottom: 5
text ink boxes crossing y=120: none; text cut warnings: none; mock warnings: none
nearest visible text to y=120: [(63.0, '3', (202, 46, 218, 68)), (63.0, '5', (143, 46, 158, 68))]
ops (rect/frame/art) straddling y=120 (drawn in both strips): {'music_slim:rect': 12}
element pairs (non-contained) with 0 <= gap < 2 px (gap = empty px between bboxes; 0 = touching), grouped:
- footer_keybed/frame ~ footer_keybed/rect gap=1 : 1 pair(s), e.g. ((76, 188, 90, 222), (91, 188, 105, 222))
- footer_keybed/rect ~ footer_keybed/frame gap=1 : 1 pair(s), e.g. ((61, 188, 75, 222), (76, 188, 90, 222))
- footer_keybed/rect ~ footer_keybed/rect gap=1 : 15 pair(s), e.g. ((1, 188, 15, 222), (16, 188, 30, 222))
music_slim intra-row pairs with gap<2: {'rect~rect gap=1': 22}
partially overlapping (not contained) element pairs, grouped:
- footer_keybed/frame ~ footer_keybed/rect : 8 pair(s), e.g. ((76, 188, 90, 222), (69, 188, 81, 209))
- footer_keybed/rect ~ footer_keybed/rect : 44 pair(s), e.g. ((1, 188, 15, 222), (9, 188, 21, 209))
- knobs_fill/rect ~ knobs_fill/text : 4 pair(s), e.g. ((1, 37, 59, 76), (11, 28, 49, 41))

## icon pixel sizes (icons_v7.ICONS, 1 bit/px)
| icon | rows x cols (nominal) | ink bbox w x h | ink px (set) | bytes at 1 bpp (rows*ceil(cols/8)) |
|---|---|---|---|---|
| dice_all | 13x13 | 13x13 | 65 | 26 |
| dice_notes | 13x13 | 13x13 | 68 | 26 |
| dice_accnt | 13x13 | 13x13 | 58 | 26 |
| dice_shape | 13x13 | 13x13 | 56 | 26 |
| dice_artic | 13x13 | 13x13 | 57 | 26 |
| dice_hits | 13x13 | 13x13 | 65 | 26 |
| undo | 13x11 | 11x9 | 26 | 22 |
| settings | 13x11 | 13x11 | 70 | 22 |
| home | 13x11 | 11x11 | 59 | 22 |
| save | 13x11 | 13x11 | 83 | 22 |
| play | 13x11 | 12x11 | 72 | 22 |
| stop | 13x11 | 9x9 | 81 | 22 |
| rec | 13x11 | 13x11 | 111 | 22 |
total distinct button icon bitmaps=13, bytes (row-padded to byte) = 310, bits (w*h) = 2015
distinct art strings drawn across the six scenes: 20
| bitmap used | nominal cols x rows | bytes@1bpp |
| ALGO.ACID | 11x6 | 12 |
| IC.left | 5x7 | 7 |
| IC.note | 11x11 | 22 |
| IC.play | 11x11 | 22 |
| IC.rec | 11x11 | 22 |
| IC.right | 5x7 | 7 |
| IC.rst | 5x7 | 7 |
| dice_accnt | 13x13 | 26 |
| dice_all | 13x13 | 26 |
| dice_artic | 13x13 | 26 |
| dice_hits | 13x13 | 26 |
| dice_notes | 13x13 | 26 |
| dice_shape | 13x13 | 26 |
| home | 13x11 | 22 |
| play | 13x11 | 22 |
| rec | 13x11 | 22 |
| save | 13x11 | 22 |
| settings | 13x11 | 22 |
| stop | 13x11 | 22 |
| undo | 13x11 | 22 |
total bytes of bitmaps drawn (row-padded) = 409

## button cells
cells: 12; sizes {(38, 24)}; x starts [1, 41, 81, 121, 161, 201]; y starts [132, 158]
cell 38x24 px = 4.75 x 3.00 mm at 8 px/mm; pitch 40 x 26 px; gaps between cells 2 px (x) and 2 px (y); block spans y 132..182
- cell(1,132) icon ink bbox (13, 137, 26, 150) margins L12 R13 T5 B6 (ink 13x13)
- cell(41,132) icon ink bbox (53, 137, 66, 150) margins L12 R13 T5 B6 (ink 13x13)
- cell(81,132) icon ink bbox (93, 137, 106, 150) margins L12 R13 T5 B6 (ink 13x13)
- cell(121,132) icon ink bbox (133, 137, 146, 150) margins L12 R13 T5 B6 (ink 13x13)
- cell(161,132) icon ink bbox (174, 139, 185, 148) margins L13 R14 T7 B8 (ink 11x9)
- cell(201,132) icon ink bbox (213, 138, 226, 149) margins L12 R13 T6 B7 (ink 13x11)
- cell(1,158) icon ink bbox (14, 164, 25, 175) margins L13 R14 T6 B7 (ink 11x11)
- cell(41,158) icon ink bbox (53, 164, 66, 175) margins L12 R13 T6 B7 (ink 13x11)
- cell(81,158) icon ink bbox (93, 163, 106, 176) margins L12 R13 T5 B6 (ink 13x13)
- cell(121,158) icon ink bbox (133, 163, 146, 176) margins L12 R13 T5 B6 (ink 13x13)
- cell(161,158) icon ink bbox (174, 165, 183, 174) margins L13 R16 T7 B8 (ink 9x9)
- cell(201,158) icon ink bbox (213, 164, 226, 175) margins L12 R13 T6 B7 (ink 13x11)

## keybed vs the 15 px column grid (playing scene)
white-key rects (w=14): 26; x starts [1, 1, 16, 31] ... pitch [15]; widths {14}; heights {34, 12}
black-key bodies (w=10): 11; x starts [10, 25, 40, 70, 85, 115, 130, 145, 175, 190, 220]; centres (x+5) [15, 30, 45, 75, 90, 120, 135, 150, 180, 195, 225]
black-key BG surrounds (w=12): 11 at x [9, 24, 39, 69, 84, 114, 129, 144, 174, 189, 219]
white-key x spans: [(1, 14), (1, 14), (16, 29), (31, 44), (31, 44), (46, 59), (61, 74), (61, 74), (76, 89), (76, 89), (91, 104), (106, 119), (106, 119), (121, 134), (121, 134), (136, 149), (151, 164), (151, 164), (166, 179), (166, 179), (181, 194), (196, 209), (196, 209), (211, 224), (211, 224), (226, 239)]
1 px empty column between adjacent white keys (pitch 15, width 14); black-key BODY 10 px wide = 0.67 of pitch; body+surround 12 px = 0.8 of pitch; white key 14 px = 1.75 mm, black body 10 px = 1.25 mm
black-key centre = boundary between white keys N and N+1 (white keys N at x=15(N-1)+1..15(N-1)+14, the empty column at x=15N): body spans cx-5..cx+4, i.e. centred 0.5 px left of the boundary column
music row column pitch 15 px (x=i*15+1), inner 13 px; bars per step widths used (playing): {2: 6, 6: 8, 13: 7, 3: 10, 15: 1, 1: 3}
scene playing: bar width histogram (px:count) {1: 3, 2: 6, 3: 10, 6: 8, 13: 7}; min bar height 2; muted outline frames at width<=2 (no interior): 1; hatch ops 0 (widths [])
scene one_channel: bar width histogram (px:count) {1: 3, 13: 13}; min bar height 2; muted outline frames at width<=2 (no interior): 0; hatch ops 0 (widths [])
scene two_channels_stopped: bar width histogram (px:count) {1: 3, 6: 9, 13: 10}; min bar height 2; muted outline frames at width<=2 (no interior): 0; hatch ops 0 (widths [])
scene dice: bar width histogram (px:count) {1: 3, 3: 5, 6: 9, 13: 9}; min bar height 2; muted outline frames at width<=2 (no interior): 0; hatch ops 6 (widths [6, 13])
scene turning_k2: bar width histogram (px:count) {1: 3, 3: 5, 6: 9, 13: 9}; min bar height 2; muted outline frames at width<=2 (no interior): 0; hatch ops 0 (widths [])
scene rec_armed: bar width histogram (px:count) {1: 3, 3: 5, 6: 9, 13: 9}; min bar height 2; muted outline frames at width<=2 (no interior): 0; hatch ops 0 (widths [])

## 4b. music row bar statistics (bars = music_slim rect/frame ops, 90<=y<124, h>=4, w<15; accent caps/ticks/dashes excluded)
- playing: steps with sound=14 (bars per step histogram {1: 6, 2: 4, 3: 3, 4: 1}); bar width px histogram {2: 4, 3: 9, 6: 8, 13: 6}; bar height range 4..32; muted frames=8 (widths {2: 1, 6: 3, 3: 3, 13: 1}); accent caps=4 (widths [2, 3, 13]); rest dashes=2 (3x1 px); bar-width-2 solid bars=3; hatched=0
  step sets: used=[0, 1, 2, 3]; all-rest steps (no bars): [1, 9]
  bars with width<=3 : 13 of 27; colours among w<=3: ['CH1', 'CH2', 'CH4', 'DIM']
  bar widths in mm: [0.25, 0.375, 0.75, 1.625]
- one_channel: steps with sound=10 (bars per step histogram {1: 10}); bar width px histogram {13: 10}; bar height range 4..32; muted frames=0 (widths {}); accent caps=3 (widths [13]); rest dashes=6 (3x1 px); bar-width-2 solid bars=0; hatched=0
  step sets: used=[0]; all-rest steps (no bars): [1, 3, 6, 9, 12, 15]
  bars with width<=3 : 0 of 10; colours among w<=3: []
  bar widths in mm: [1.625]
- two_channels_stopped: steps with sound=12 (bars per step histogram {1: 9, 2: 3}); bar width px histogram {6: 6, 13: 9}; bar height range 4..32; muted frames=0 (widths {}); accent caps=4 (widths [6, 13]); rest dashes=4 (3x1 px); bar-width-2 solid bars=0; hatched=0
  step sets: used=[0, 1]; all-rest steps (no bars): [1, 9, 12, 15]
  bars with width<=3 : 0 of 15; colours among w<=3: []
  bar widths in mm: [0.75, 1.625]
- dice: steps with sound=13 (bars per step histogram {1: 8, 2: 4, 3: 1}); bar width px histogram {3: 3, 6: 8, 13: 8}; bar height range 4..32; muted frames=0 (widths {}); accent caps=4 (widths [3, 6, 13]); rest dashes=3 (3x1 px); bar-width-2 solid bars=0; hatched=6
  step sets: used=[0, 1, 3]; all-rest steps (no bars): [1, 9, 15]
  bars with width<=3 : 3 of 19; colours among w<=3: ['CH1', 'CH2', 'CH4']
  bar widths in mm: [0.375, 0.75, 1.625]
- turning_k2: steps with sound=13 (bars per step histogram {1: 8, 2: 4, 3: 1}); bar width px histogram {3: 3, 6: 8, 13: 8}; bar height range 4..32; muted frames=0 (widths {}); accent caps=4 (widths [3, 6, 13]); rest dashes=3 (3x1 px); bar-width-2 solid bars=0; hatched=0
  step sets: used=[0, 1, 3]; all-rest steps (no bars): [1, 9, 15]
  bars with width<=3 : 3 of 19; colours among w<=3: ['CH1', 'CH2', 'CH4']
  bar widths in mm: [0.375, 0.75, 1.625]
- rec_armed: steps with sound=13 (bars per step histogram {1: 8, 2: 4, 3: 1}); bar width px histogram {3: 3, 6: 8, 13: 8}; bar height range 4..32; muted frames=0 (widths {}); accent caps=4 (widths [3, 6, 13]); rest dashes=3 (3x1 px); bar-width-2 solid bars=0; hatched=0
  step sets: used=[0, 1, 3]; all-rest steps (no bars): [1, 9, 15]
  bars with width<=3 : 3 of 19; colours among w<=3: ['CH1', 'CH2', 'CH4']
  bar widths in mm: [0.375, 0.75, 1.625]

# 5. Edge-case sweep
## 5a. knob tile fit (tile 58 px wide; label AF_M, value AF_L as knobs_fill draws them; AF_L has glyphs for space # + - . / 0-9 A-Y only)
| algo | knob | label | label px (AF_M) | widest value | value px (AF_L) | fits 58? | glyphs missing in AF_L (drawn as glyph 0) | note |
|---|---|---|---|---|---|---|---|---|
| ACID | K1 | DENS | 38 | KEYS | 70 | NO | - | range from TSV: den 0..16 def 12; 0 = KEYS ONLY (the cell value reads KEYS) |
| ACID | K2 | SLIDE | 39 | 0 | 18 | yes | - | range from TSV: sl 0..8 def 4 (was S2) |
| ACID | K3 | JUMP | 40 | 10 | 36 | yes | - | range from TSV: jmp 0..20 def 5 |
| ACID | K4 | ACCNT | 50 | 0 | 18 | yes | - | range from TSV: acc 0..8 def 3 |
| ACID | S1 | OFFSET | 54 | 10 | 36 | yes | - | range from TSV: off 0..15 def 0 |
| ACID | S2 | PEDAL | 45 | 10 | 36 | yes | - | range from TSV: typ 0..12 def 6 (was K2) |
| ACID | S3 | PUNCH | 51 | 10 | 36 | yes | - | range from TSV: pun 0..47 def 40 fast 4 |
| ACID | S4 | DIR | 23 | PEND | 72 | NO | - | range from TSV: dir FWD BWD PEND |
| EUCLID | K1 | HITS | 32 | KEYS | 70 | NO | - | range from TSV: hit 0..16 def 5; 0 = KEYS ONLY (the cell value reads KEYS) |
| EUCLID | K2 | OFFSET | 54 | 10 | 36 | yes | - | range from TSV: off 0..15 def 0 (rotation; was S1) |
| EUCLID | K3 | RATCH | 47 | 0 | 18 | yes | - | range from TSV: rat 0..8 def 4 (was S2) |
| EUCLID | K4 | ACCNT | 50 | 0 | 18 | yes | - | range from TSV: acc 0..8 def 2 |
| EUCLID | S1 | LEN | 27 | 10 | 36 | yes | - | range from TSV: len 2..16 def 16 (was K2) |
| EUCLID | S2 | SPAN | 37 | 10 | 36 | yes | - | range from TSV: spn 0..24 def 12 (was K3) |
| EUCLID | S3 | PUNCH | 51 | 10 | 36 | yes | - | range from TSV: pun 0..47 def 40 fast 4 |
| EUCLID | S4 | CURVE | 48 | SAW+ | 78 | NO | - | range from TSV: crv AUTO SINE TRI SAW+ SAW- ARCH INV |
| SNAKE | K1 | DENS | 38 | KEYS | 70 | NO | - | range from TSV: den 0..16 def 10; 0 = KEYS ONLY (the cell value reads KEYS) |
| SNAKE | K2 | LEN | 27 | 10 | 36 | yes | - | range from TSV: len 1..16 def 7 |
| SNAKE | K3 | RANGE | 49 | 10 | 36 | yes | - | range from TSV: rng 0..48 def 12 fast 4 |
| SNAKE | K4 | PATH | 36 | 10 | 36 | yes | - | range from TSV: shp 1..14 def 7 |
| SNAKE | S1 | OFFSET | 54 | 10 | 36 | yes | - | range from TSV: off 0..15 def 0 |
| SNAKE | S2 | GLEN | 37 | 10 | 36 | yes | - | range from TSV: gl 1..16 def 16 |
| SNAKE | S3 | DYN | 30 | 10 | 36 | yes | - | range from TSV: dyn 0..10 def 10 |
| SNAKE | S4 | DIR | 23 | PEND | 72 | NO | - | range from TSV: dir FWD BWD PEND |
| STAGE | K1 | DENS | 38 | KEYS | 70 | NO | - | range from TSV: den 0..8 def 6; 0 = KEYS ONLY (the cell value reads KEYS) |
| STAGE | K2 | TOTAL | 45 | 48 (32) | 90 | NO | () | range from TSV: tot 8..64 def 16 fast 4; cell adds (n), the number of steps  |
| STAGE | K3 | DIR | 23 | PEND | 72 | NO | - | range from TSV: dir FWD BWD PEND (was S4) |
| STAGE | K4 | RATCH | 47 | 0 | 18 | yes | - | range from TSV: rat 0..8 def 2 |
| STAGE | S1 | OFFSET | 54 | 10 | 36 | yes | - | range from TSV: off 0..63 def 0 fast 4 |
| STAGE | S2 | RDIV | 33 | 1 | 18 | yes | - | range from TSV: rdiv 1..4 def 4 |
| STAGE | S3 | DYN | 30 | 10 | 36 | yes | - | range from TSV: dyn 0..10 def 10 |
| STAGE | S4 | RANGE | 49 | 10 | 36 | yes | - | range from TSV: rng 0..36 def 12 fast 4 (was K3) |
| ARP | K1 | DENS | 38 | KEYS | 70 | NO | - | range from TSV: den 0..16 def 12; 0 = KEYS ONLY (the cell value reads KEYS) |
| ARP | K2 | SHAPE | 47 | THUMB | 98 | NO | - | range from TSV: shp 0..5 def 0 LINE SKIP THUMB OCTV REPT WAVE |
| ARP | K3 | SPAN | 37 | 1 OCT | 82 | NO | - | range from TSV: spn 1..4 def 2 (cell reads 1 OCT..4 OCT) |
| ARP | K4 | RATE | 35 | 1/4 | 46 | yes | - | range from TSV: rat 0..5 def 2 1/4 1/2 1X 2X 3T 4X |
| ARP | S1 | OFFSET | 54 | 10 | 36 | yes | - | range from TSV: off 0..15 def 0 |
| ARP | S2 | LEGATO | 55 | 0 | 18 | yes | - | range from TSV: lg 0..8 def 3 |
| ARP | S3 | DYN | 30 | 10 | 36 | yes | - | range from TSV: dyn 0..10 def 10 |
| ARP | S4 | DIR | 23 | CHORD | 97 | NO | - | range from TSV: dir UP DOWN UPDN RAND ASPL CHORD def UP |
| PATTERNS | K1 | DENS | 38 | KEYS | 70 | NO | - | range from TSV: dens 0..16 def 8 (as written); 0 = KEYS ONLY (the cell reads |
| PATTERNS | K2 | VARY | 37 | 0 | 18 | yes | - | range from TSV: vary 0..8 def 0 |
| PATTERNS | K3 | SPAN | 37 | 1 | 18 | yes | - | range from TSV: span 1..4 def 4 (as written) |
| PATTERNS | K4 | ACCNT | 50 | 0 | 18 | yes | - | range from TSV: acc 0..8 def 4 (as written) |
| PATTERNS | S1 | PICK | 32 | HOUSE 120 | 153 | NO | - | range from TSV: pick 1..120 def 1 fast = category; the cell reads CAT nn, fo |
| PATTERNS | S2 | SLIDE | 39 | 0 | 18 | yes | - | range from TSV: sl 0..8 def 4 (as written) |
| PATTERNS | S3 | DYN | 30 | 10 | 36 | yes | - | range from TSV: dyn 0..14 def 10 |
| PATTERNS | S4 | CTONE | 49 | 0 | 18 | yes | - | range from TSV: ct 0..8 def 2 |

tiles checked=48; label or widest value > 58 px = 15; knobs whose value set uses an AF_L-missing glyph = 1
values (any, not only widest) whose AF_L advance exceeds 58 px or that need a glyph AF_L lacks:
- ACID K1 DENS value 'KEYS': 70 px AF_L; missing glyphs []
- ACID S4 DIR value 'FWD': 62 px AF_L; missing glyphs []
- ACID S4 DIR value 'BWD': 64 px AF_L; missing glyphs []
- ACID S4 DIR value 'PEND': 72 px AF_L; missing glyphs []
- EUCLID K1 HITS value 'KEYS': 70 px AF_L; missing glyphs []
- EUCLID S4 CURVE value 'AUTO': 76 px AF_L; missing glyphs []
- EUCLID S4 CURVE value 'SINE': 59 px AF_L; missing glyphs []
- EUCLID S4 CURVE value 'SAW+': 78 px AF_L; missing glyphs []
- EUCLID S4 CURVE value 'SAW-': 73 px AF_L; missing glyphs []
- EUCLID S4 CURVE value 'ARCH': 76 px AF_L; missing glyphs []
- SNAKE K1 DENS value 'KEYS': 70 px AF_L; missing glyphs []
- SNAKE S4 DIR value 'FWD': 62 px AF_L; missing glyphs []
- SNAKE S4 DIR value 'BWD': 64 px AF_L; missing glyphs []
- SNAKE S4 DIR value 'PEND': 72 px AF_L; missing glyphs []
- STAGE K1 DENS value 'KEYS': 70 px AF_L; missing glyphs []
- STAGE K2 TOTAL value '48 (32)': 90 px AF_L; missing glyphs ['(', ')']
- STAGE K2 TOTAL value '64 (32)': 90 px AF_L; missing glyphs ['(', ')']
- STAGE K3 DIR value 'FWD': 62 px AF_L; missing glyphs []
- STAGE K3 DIR value 'BWD': 64 px AF_L; missing glyphs []
- STAGE K3 DIR value 'PEND': 72 px AF_L; missing glyphs []
- ARP K1 DENS value 'KEYS': 70 px AF_L; missing glyphs []
- ARP K2 SHAPE value 'SKIP': 59 px AF_L; missing glyphs []
- ARP K2 SHAPE value 'THUMB': 98 px AF_L; missing glyphs []
- ARP K2 SHAPE value 'OCTV': 77 px AF_L; missing glyphs []
- ARP K2 SHAPE value 'REPT': 68 px AF_L; missing glyphs []
- ARP K2 SHAPE value 'WAVE': 77 px AF_L; missing glyphs []
- ARP K3 SPAN value '1 OCT': 82 px AF_L; missing glyphs []
- ARP K3 SPAN value '2 OCT': 82 px AF_L; missing glyphs []
- ARP K3 SPAN value '3 OCT': 82 px AF_L; missing glyphs []
- ARP K3 SPAN value '4 OCT': 82 px AF_L; missing glyphs []
- ARP S4 DIR value 'DOWN': 86 px AF_L; missing glyphs []
- ARP S4 DIR value 'UPDN': 75 px AF_L; missing glyphs []
- ARP S4 DIR value 'RAND': 75 px AF_L; missing glyphs []
- ARP S4 DIR value 'ASPL': 68 px AF_L; missing glyphs []
- ARP S4 DIR value 'CHORD': 97 px AF_L; missing glyphs []
- PATTERNS K1 DENS value 'KEYS': 70 px AF_L; missing glyphs []
- PATTERNS S1 PICK value 'HOUSE 3': 117 px AF_L; missing glyphs []
- PATTERNS S1 PICK value 'HOUSE 120': 153 px AF_L; missing glyphs []
- PATTERNS S1 PICK value 'HOUSE 99': 135 px AF_L; missing glyphs []

labels (AF_M) wider than 58: none; widest label: (55, 'LEGATO')
PERF * rows: R1-R4 'WIRE1'..'WIRE4' (ROW 3 labels): AF_M width WIRE1=44; the knob_map R rows say the cell shows the device's name for the parameter (not enumerated, not testable)

## 5b. v7.screen with TSV-derived knob rows injected into el.KN (algo names, labels, widest values), and as-shipped KN
el.KN as shipped has keys: ['ACID', 'EUCLID'] ; ACID rows: 2 ; EUCLID rows: 1
- as shipped: algo ACID row 0: rendered; warnings=none; knob labels drawn=['DENS', 'SLIDE', 'JUMP', 'ACCNT'] (algo in KN: True)
- as shipped: algo ACID row 1: rendered; warnings=none; knob labels drawn=['OFFSET', 'PEDAL', 'PUNCH', 'DIR'] (algo in KN: True)
- as shipped: algo ACID row 2: EXCEPTION IndexError: list index out of range; warnings=none; knob labels drawn=None (algo in KN: True)
- as shipped: algo EUCLID row 0: rendered; warnings=none; knob labels drawn=['HITS', 'OFFSET', 'RATCH', 'ACCNT'] (algo in KN: True)
- as shipped: algo EUCLID row 1: EXCEPTION IndexError: list index out of range; warnings=none; knob labels drawn=None (algo in KN: True)
- as shipped: algo EUCLID row 2: EXCEPTION IndexError: list index out of range; warnings=none; knob labels drawn=None (algo in KN: True)
- as shipped: algo SNAKE row 0: rendered; warnings=none; knob labels drawn=['DENS', 'SLIDE', 'JUMP', 'ACCNT'] (algo in KN: False)
- as shipped: algo SNAKE row 1: rendered; warnings=none; knob labels drawn=['OFFSET', 'PEDAL', 'PUNCH', 'DIR'] (algo in KN: False)
- as shipped: algo SNAKE row 2: EXCEPTION IndexError: list index out of range; warnings=none; knob labels drawn=None (algo in KN: False)
- as shipped: algo STAGE row 0: rendered; warnings=none; knob labels drawn=['DENS', 'SLIDE', 'JUMP', 'ACCNT'] (algo in KN: False)
- as shipped: algo STAGE row 1: rendered; warnings=none; knob labels drawn=['OFFSET', 'PEDAL', 'PUNCH', 'DIR'] (algo in KN: False)
- as shipped: algo STAGE row 2: EXCEPTION IndexError: list index out of range; warnings=none; knob labels drawn=None (algo in KN: False)
- as shipped: algo ARP row 0: rendered; warnings=none; knob labels drawn=['DENS', 'SLIDE', 'JUMP', 'ACCNT'] (algo in KN: False)
- as shipped: algo ARP row 1: rendered; warnings=none; knob labels drawn=['OFFSET', 'PEDAL', 'PUNCH', 'DIR'] (algo in KN: False)
- as shipped: algo ARP row 2: EXCEPTION IndexError: list index out of range; warnings=none; knob labels drawn=None (algo in KN: False)
- as shipped: algo PATTERNS row 0: rendered; warnings=none; knob labels drawn=['DENS', 'SLIDE', 'JUMP', 'ACCNT'] (algo in KN: False)
- as shipped: algo PATTERNS row 1: rendered; warnings=none; knob labels drawn=['OFFSET', 'PEDAL', 'PUNCH', 'DIR'] (algo in KN: False)
- as shipped: algo PATTERNS row 2: EXCEPTION IndexError: list index out of range; warnings=none; knob labels drawn=None (algo in KN: False)
- TSV rows injected: algo ACID row 0: rendered; warnings=["warning: text 'KEYS' is cut by the canvas (ink -4,46..64,68)"]
    cut texts=[('KEYS', 'AF_L', (-4, 46, 64, 68))] ; knob texts wider than 58=[('KEYS', 'AF_L', 70)]
- TSV rows injected: algo ACID row 1: rendered; warnings=["warning: text 'PEND' is cut by the canvas (ink 175,46..245,67)"]
    cut texts=[('PEND', 'AF_L', (175, 46, 245, 67))] ; knob texts wider than 58=[('PEND', 'AF_L', 72)]
- TSV rows injected: algo ACID row 2: rendered; warnings=none
    cut texts=none ; knob texts wider than 58=none
- TSV rows injected: algo EUCLID row 0: rendered; warnings=["warning: text 'KEYS' is cut by the canvas (ink -4,46..64,68)"]
    cut texts=[('KEYS', 'AF_L', (-4, 46, 64, 68))] ; knob texts wider than 58=[('KEYS', 'AF_L', 70)]
- TSV rows injected: algo EUCLID row 1: rendered; warnings=["warning: text 'SAW+' is cut by the canvas (ink 171,46..248,68)"]
    cut texts=[('SAW+', 'AF_L', (171, 46, 248, 68))] ; knob texts wider than 58=[('SAW+', 'AF_L', 78)]
- TSV rows injected: algo EUCLID row 2: rendered; warnings=none
    cut texts=none ; knob texts wider than 58=none
- TSV rows injected: algo SNAKE row 0: rendered; warnings=["warning: text 'KEYS' is cut by the canvas (ink -4,46..64,68)"]
    cut texts=[('KEYS', 'AF_L', (-4, 46, 64, 68))] ; knob texts wider than 58=[('KEYS', 'AF_L', 70)]
- TSV rows injected: algo SNAKE row 1: rendered; warnings=["warning: text 'PEND' is cut by the canvas (ink 175,46..245,67)"]
    cut texts=[('PEND', 'AF_L', (175, 46, 245, 67))] ; knob texts wider than 58=[('PEND', 'AF_L', 72)]
- TSV rows injected: algo SNAKE row 2: rendered; warnings=none
    cut texts=none ; knob texts wider than 58=none
- TSV rows injected: algo STAGE row 0: rendered; warnings=["warning: text 'KEYS' is cut by the canvas (ink -4,46..64,68)"]
    cut texts=[('KEYS', 'AF_L', (-4, 46, 64, 68))] ; knob texts wider than 58=[('KEYS', 'AF_L', 70), ('48 (32)', 'AF_L', 90), ('PEND', 'AF_L', 72)]
- TSV rows injected: algo STAGE row 1: rendered; warnings=none
    cut texts=none ; knob texts wider than 58=none
- TSV rows injected: algo STAGE row 2: rendered; warnings=none
    cut texts=none ; knob texts wider than 58=none
- TSV rows injected: algo ARP row 0: rendered; warnings=["warning: text 'KEYS' is cut by the canvas (ink -4,46..64,68)"]
    cut texts=[('KEYS', 'AF_L', (-4, 46, 64, 68))] ; knob texts wider than 58=[('KEYS', 'AF_L', 70), ('THUMB', 'AF_L', 98), ('1 OCT', 'AF_L', 82)]
- TSV rows injected: algo ARP row 1: rendered; warnings=["warning: text 'CHORD' is cut by the canvas (ink 162,46..257,68)"]
    cut texts=[('CHORD', 'AF_L', (162, 46, 257, 68))] ; knob texts wider than 58=[('CHORD', 'AF_L', 97)]
- TSV rows injected: algo ARP row 2: rendered; warnings=none
    cut texts=none ; knob texts wider than 58=none
- TSV rows injected: algo PATTERNS row 0: rendered; warnings=["warning: text 'KEYS' is cut by the canvas (ink -4,46..64,68)"]
    cut texts=[('KEYS', 'AF_L', (-4, 46, 64, 68))] ; knob texts wider than 58=[('KEYS', 'AF_L', 70)]
- TSV rows injected: algo PATTERNS row 1: rendered; warnings=["warning: text 'HOUSE 120' is cut by the canvas (ink -46,46..105,68)"]
    cut texts=[('HOUSE 120', 'AF_L', (-46, 46, 105, 68))] ; knob texts wider than 58=[('HOUSE 120', 'AF_L', 153)]
- TSV rows injected: algo PATTERNS row 2: rendered; warnings=none
    cut texts=none ; knob texts wider than 58=none

## 5c. header: algorithm name where it is drawn (enc_row: icon x=114..125, text x=127 with algo[:6]; preset text right-aligned to x=236 ('P01' starts x=209); pending frame x=196..238)
| algo | name | drawn [:6] | full width AF_M | drawn width | x end drawn | full x end | room to P01 text (x=209) | room to pending frame (x=196) | ALGO_IC size |
|---|---|---|---|---|---|---|---|---|---|
| ACID | ACID | ACID | 33 | 33 | 160 | 160 | 49 | 36 | 11x6 |
| EUCLID | EUCLID | EUCLID | 51 | 51 | 178 | 178 | 31 | 18 | 9x7 |
| SNAKE | SNAKE | SNAKE | 47 | 47 | 174 | 174 | 35 | 22 | 9x5 |
| STAGE | STAGE | STAGE | 45 | 45 | 172 | 172 | 37 | 24 | 9x5 |
| ARP | ARP | ARP | 28 | 28 | 155 | 155 | 54 | 41 | 9x5 |
| PATTERNS | PATTERNS | PATTER | 72 | 52 | 179 | 199 | 10 | -3 | 9x5 |
preset text 'P16' width AF_M=27 (x start 209); 'P16?' with frame: width 34 (x start 202, frame 196..238)
BPM text x=16: '120' width 27 (ends x=43), '300' width 27, '40' width 18; first channel chip starts x=50 (room for '300' = 7 px)
ALGO_PICK cue strip form per page_states.tsv uses 4-letter short names ACID EUCL SNAK STAG ARP PATT; the header here draws the first 6 letters of the long name: ACID->ACID, EUCLID->EUCLID, SNAKE->SNAKE, STAGE->STAGE, ARP->ARP, PATTERNS->PATTER
- header render algo ACID pend=None: algo text drawn 'ACID' ink (127, 4, 160, 17); preset text 'P01' ink (209, 4, 233, 17); algo ink right edge 160 vs preset ink left 209; overlap=False; warnings=none
- header render algo ACID pend=16: algo text drawn 'ACID' ink (127, 4, 160, 17); preset text 'P16?' ink (202, 4, 236, 17); algo ink right edge 160 vs preset ink left 202; overlap=False; warnings=none
- header render algo EUCLID pend=None: algo text drawn 'EUCLID' ink (127, 4, 177, 17); preset text 'P01' ink (209, 4, 233, 17); algo ink right edge 177 vs preset ink left 209; overlap=False; warnings=none
- header render algo EUCLID pend=16: algo text drawn 'EUCLID' ink (127, 4, 177, 17); preset text 'P16?' ink (202, 4, 236, 17); algo ink right edge 177 vs preset ink left 202; overlap=False; warnings=none
- header render algo SNAKE pend=None: algo text drawn 'SNAKE' ink (127, 4, 174, 17); preset text 'P01' ink (209, 4, 233, 17); algo ink right edge 174 vs preset ink left 209; overlap=False; warnings=none
- header render algo SNAKE pend=16: algo text drawn 'SNAKE' ink (127, 4, 174, 17); preset text 'P16?' ink (202, 4, 236, 17); algo ink right edge 174 vs preset ink left 202; overlap=False; warnings=none
- header render algo STAGE pend=None: algo text drawn 'STAGE' ink (127, 4, 172, 17); preset text 'P01' ink (209, 4, 233, 17); algo ink right edge 172 vs preset ink left 209; overlap=False; warnings=none
- header render algo STAGE pend=16: algo text drawn 'STAGE' ink (127, 4, 172, 17); preset text 'P16?' ink (202, 4, 236, 17); algo ink right edge 172 vs preset ink left 202; overlap=False; warnings=none
- header render algo ARP pend=None: algo text drawn 'ARP' ink (127, 5, 155, 16); preset text 'P01' ink (209, 4, 233, 17); algo ink right edge 155 vs preset ink left 209; overlap=False; warnings=none
- header render algo ARP pend=16: algo text drawn 'ARP' ink (127, 5, 155, 16); preset text 'P16?' ink (202, 4, 236, 17); algo ink right edge 155 vs preset ink left 202; overlap=False; warnings=none
- header render algo PATTERNS pend=None: algo text drawn 'PATTER' ink (127, 5, 179, 16); preset text 'P01' ink (209, 4, 233, 17); algo ink right edge 179 vs preset ink left 209; overlap=False; warnings=none
- header render algo PATTERNS pend=16: algo text drawn 'PATTER' ink (127, 5, 179, 16); preset text 'P16?' ink (202, 4, 236, 17); algo ink right edge 179 vs preset ink left 202; overlap=False; warnings=none

## 5d. channel combinations (render_state through v7.draw, palette MONO as the mock parses it)
- used=[1] sel=1: rendered; mock warnings=none; text overlaps=none; cut texts=none; header chip digits=['1']; '+' drawn at x=[67]
- used=[2] sel=2: rendered; mock warnings=none; text overlaps=none; cut texts=none; header chip digits=['2']; '+' drawn at x=none
- used=[3] sel=3: rendered; mock warnings=none; text overlaps=none; cut texts=none; header chip digits=['3']; '+' drawn at x=[67]
- used=[4] sel=4: rendered; mock warnings=none; text overlaps=none; cut texts=none; header chip digits=['4']; '+' drawn at x=[67]
- used=[1, 2] sel=1: rendered; mock warnings=none; text overlaps=none; cut texts=none; header chip digits=['1', '2']; '+' drawn at x=[82]
- used=[1, 2] sel=2: rendered; mock warnings=none; text overlaps=none; cut texts=none; header chip digits=['1', '2']; '+' drawn at x=[82]
- used=[1, 3] sel=1: rendered; mock warnings=none; text overlaps=none; cut texts=none; header chip digits=['1', '3']; '+' drawn at x=none
- used=[1, 3] sel=3: rendered; mock warnings=none; text overlaps=none; cut texts=none; header chip digits=['1', '3']; '+' drawn at x=none
- used=[1, 4] sel=1: rendered; mock warnings=none; text overlaps=none; cut texts=none; header chip digits=['1', '4']; '+' drawn at x=[82]
- used=[1, 4] sel=4: rendered; mock warnings=none; text overlaps=none; cut texts=none; header chip digits=['1', '4']; '+' drawn at x=[82]
- used=[2, 3] sel=2: rendered; mock warnings=none; text overlaps=none; cut texts=none; header chip digits=['2', '3']; '+' drawn at x=none
- used=[2, 3] sel=3: rendered; mock warnings=none; text overlaps=none; cut texts=none; header chip digits=['2', '3']; '+' drawn at x=none
- used=[2, 4] sel=2: rendered; mock warnings=none; text overlaps=none; cut texts=none; header chip digits=['2', '4']; '+' drawn at x=[82]
- used=[2, 4] sel=4: rendered; mock warnings=none; text overlaps=none; cut texts=none; header chip digits=['2', '4']; '+' drawn at x=[82]
- used=[3, 4] sel=3: rendered; mock warnings=none; text overlaps=none; cut texts=none; header chip digits=['3', '4']; '+' drawn at x=none
- used=[3, 4] sel=4: rendered; mock warnings=none; text overlaps=none; cut texts=none; header chip digits=['3', '4']; '+' drawn at x=none
- used=[1, 2, 3] sel=1: rendered; mock warnings=none; text overlaps=none; cut texts=none; header chip digits=['1', '2', '3']; '+' drawn at x=[97]
- used=[1, 2, 3] sel=3: rendered; mock warnings=none; text overlaps=none; cut texts=none; header chip digits=['1', '2', '3']; '+' drawn at x=[97]
- used=[1, 2, 4] sel=1: rendered; mock warnings=none; text overlaps=none; cut texts=none; header chip digits=['1', '2', '4']; '+' drawn at x=none
- used=[1, 2, 4] sel=4: rendered; mock warnings=none; text overlaps=none; cut texts=none; header chip digits=['1', '2', '4']; '+' drawn at x=none
- used=[1, 3, 4] sel=1: rendered; mock warnings=none; text overlaps=none; cut texts=none; header chip digits=['1', '3', '4']; '+' drawn at x=none
- used=[1, 3, 4] sel=4: rendered; mock warnings=none; text overlaps=none; cut texts=none; header chip digits=['1', '3', '4']; '+' drawn at x=none
- used=[2, 3, 4] sel=2: rendered; mock warnings=none; text overlaps=none; cut texts=none; header chip digits=['2', '3', '4']; '+' drawn at x=none
- used=[2, 3, 4] sel=4: rendered; mock warnings=none; text overlaps=none; cut texts=none; header chip digits=['2', '3', '4']; '+' drawn at x=none
- used=[1, 2, 3, 4] sel=1: rendered; mock warnings=none; text overlaps=none; cut texts=none; header chip digits=['1', '2', '3', '4']; '+' drawn at x=none
- used=[1, 2, 3, 4] sel=4: rendered; mock warnings=none; text overlaps=none; cut texts=none; header chip digits=['1', '2', '3', '4']; '+' drawn at x=none

### special cases
- selected channel (CH1) muted, used 1,2,4: rendered; mock warnings=none; text overlaps=none; cut texts=none; header chip digits=['1', '2', '4']; '+' drawn at x=none
  pixels changed vs unmuted: 1820; y-range of changes 92..201; changed outside music row (y 90..127) and key glyph rows (y 188..222): 0
  changed pixels by band: {'music': 1784, 'keys': 36}
- CH2 lane all rests (used 1,2): EXCEPTION ValueError: min() iterable argument is empty
- selected CH2 lane all rests (used 1,2, sel 2): EXCEPTION ValueError: min() iterable argument is empty
- only channel, all rests: EXCEPTION ValueError: min() iterable argument is empty
- all 4 channels play steps 0 and 1 (steps 0: accent on CH1, dice on CH1+CH3): rendered; mock warnings=none; text overlaps=none; cut texts=none; header chip digits=['1', '2', '3', '4']; '+' drawn at x=none
  step 0 bars: [(1, 2, 11, 'WHITE'), (4, 2, 26, 'CH2'), (7, 2, 4, 'WHITE'), (10, 2, 7, 'CH4')] ; hatch ops on step 0: [(1, 2, 11), (7, 2, 4)]
- all four muted: rendered; mock warnings=none; text overlaps=none; cut texts=none; header chip digits=['1', '2', '3', '4']; '+' drawn at x=none
- MIDI channel 16 on CH1 vs 2: pixels differing = 0; texts containing '16'/'MIDI'/'CH' = none (the chans[].midi field is not read by v5/v6/v7 drawing code)
- sel=CH4 not in used {1,2}: rendered; mock warnings=none; text overlaps=none; cut texts=none; header chip digits=['1', '2']; '+' drawn at x=[82]
- ACID row=0 (row_marker squares drawn: 3, filled index 0): rendered
- ACID row=1 (row_marker squares drawn: 3, filled index 1): rendered
- ACID row=2 (row_marker squares drawn: 3, filled index 2): EXCEPTION IndexError: list index out of range

## 5e. turning-knob box (v5.knob_turning as v7 calls it: box 120x56 at x=60*min(k,2); title 'K%d LABEL' AF_M at x+6; old AF_M at x+6; arrow 5x7 at x+20; new AF_L at x+32; scale track starts x+66, 48 px; reads kn[0][k] = row 1 labels always)
- K1 old='4' new='6': box x=0..119; old ink right=15, new ink right=49; flags=none; warnings=none
- K2 old='4' new='6': box x=60..179; old ink right=75, new ink right=109; flags=none; warnings=none
- K3 old='4' new='6': box x=120..239; old ink right=135, new ink right=169; flags=none; warnings=none
- K4 old='4' new='6': box x=120..239; old ink right=135, new ink right=169; flags=none; warnings=none
- K1 old='11' new='12': box x=0..119; old ink right=21, new ink right=67; flags=['old text right edge 21 > arrow x 20', 'new text right edge 67 > scale x 66']; warnings=none
- K2 old='11' new='12': box x=60..179; old ink right=81, new ink right=127; flags=['old text right edge 81 > arrow x 80', 'new text right edge 127 > scale x 126']; warnings=none
- K3 old='11' new='12': box x=120..239; old ink right=141, new ink right=187; flags=['old text right edge 141 > arrow x 140', 'new text right edge 187 > scale x 186']; warnings=none
- K4 old='11' new='12': box x=120..239; old ink right=141, new ink right=187; flags=['old text right edge 141 > arrow x 140', 'new text right edge 187 > scale x 186']; warnings=none
- K1 old='46' new='47': box x=0..119; old ink right=24, new ink right=66; flags=['old text right edge 24 > arrow x 20']; warnings=none
- K2 old='46' new='47': box x=60..179; old ink right=84, new ink right=126; flags=['old text right edge 84 > arrow x 80']; warnings=none
- K3 old='46' new='47': box x=120..239; old ink right=144, new ink right=186; flags=['old text right edge 144 > arrow x 140']; warnings=none
- K4 old='46' new='47': box x=120..239; old ink right=144, new ink right=186; flags=['old text right edge 144 > arrow x 140']; warnings=none
- K1 old='KEYS' new='1': box x=0..119; old ink right=42, new ink right=45; flags=['old text right edge 42 > arrow x 20']; warnings=none
- K2 old='KEYS' new='1': box x=60..179; old ink right=102, new ink right=105; flags=['old text right edge 102 > arrow x 80']; warnings=none
- K3 old='KEYS' new='1': box x=120..239; old ink right=162, new ink right=165; flags=['old text right edge 162 > arrow x 140']; warnings=none
- K4 old='KEYS' new='1': box x=120..239; old ink right=162, new ink right=165; flags=['old text right edge 162 > arrow x 140']; warnings=none
- K1 old='FWD' new='BWD': box x=0..119; old ink right=38, new ink right=95; flags=['old text right edge 38 > arrow x 20', 'new text right edge 95 > scale x 66']; warnings=none
- K2 old='FWD' new='BWD': box x=60..179; old ink right=98, new ink right=155; flags=['old text right edge 98 > arrow x 80', 'new text right edge 155 > scale x 126']; warnings=none
- K3 old='FWD' new='BWD': box x=120..239; old ink right=158, new ink right=215; flags=['old text right edge 158 > arrow x 140', 'new text right edge 215 > scale x 186']; warnings=none
- K4 old='FWD' new='BWD': box x=120..239; old ink right=158, new ink right=215; flags=['old text right edge 158 > arrow x 140', 'new text right edge 215 > scale x 186']; warnings=none
- K1 old='PEND' new='FWD': box x=0..119; old ink right=44, new ink right=93; flags=['old text right edge 44 > arrow x 20', 'new text right edge 93 > scale x 66']; warnings=none
- K2 old='PEND' new='FWD': box x=60..179; old ink right=104, new ink right=153; flags=['old text right edge 104 > arrow x 80', 'new text right edge 153 > scale x 126']; warnings=none
- K3 old='PEND' new='FWD': box x=120..239; old ink right=164, new ink right=213; flags=['old text right edge 164 > arrow x 140', 'new text right edge 213 > scale x 186']; warnings=none
- K4 old='PEND' new='FWD': box x=120..239; old ink right=164, new ink right=213; flags=['old text right edge 164 > arrow x 140', 'new text right edge 213 > scale x 186']; warnings=none
- K1 old='1 OCT' new='2 OCT': box x=0..119; old ink right=49, new ink right=114; flags=['old text right edge 49 > arrow x 20', 'new text right edge 114 > scale x 66']; warnings=none
- K2 old='1 OCT' new='2 OCT': box x=60..179; old ink right=109, new ink right=174; flags=['old text right edge 109 > arrow x 80', 'new text right edge 174 > scale x 126']; warnings=none
- K3 old='1 OCT' new='2 OCT': box x=120..239; old ink right=169, new ink right=234; flags=['old text right edge 169 > arrow x 140', 'new text right edge 234 > scale x 186']; warnings=none
- K4 old='1 OCT' new='2 OCT': box x=120..239; old ink right=169, new ink right=234; flags=['old text right edge 169 > arrow x 140', 'new text right edge 234 > scale x 186']; warnings=none
- K1 old='48 (32)' new='50 (32)': box x=0..119; old ink right=54, new ink right=115; flags=['old text right edge 54 > arrow x 20', 'new text right edge 115 > scale x 66']; warnings=none
- K2 old='48 (32)' new='50 (32)': box x=60..179; old ink right=114, new ink right=175; flags=['old text right edge 114 > arrow x 80', 'new text right edge 175 > scale x 126']; warnings=none
- K3 old='48 (32)' new='50 (32)': box x=120..239; old ink right=174, new ink right=235; flags=['old text right edge 174 > arrow x 140', 'new text right edge 235 > scale x 186']; warnings=none
- K4 old='48 (32)' new='50 (32)': box x=120..239; old ink right=174, new ink right=235; flags=['old text right edge 174 > arrow x 140', 'new text right edge 235 > scale x 186']; warnings=none
- K1 old='HOUSE 3' new='HOUSE 4': box x=0..119; old ink right=68, new ink right=148; flags=['old text right edge 68 > arrow x 20', 'new text right edge 148 > scale x 66', 'new text right edge 148 > box right 119']; warnings=none
- K2 old='HOUSE 3' new='HOUSE 4': box x=60..179; old ink right=128, new ink right=208; flags=['old text right edge 128 > arrow x 80', 'new text right edge 208 > scale x 126', 'new text right edge 208 > box right 179']; warnings=none
- K3 old='HOUSE 3' new='HOUSE 4': box x=120..239; old ink right=188, new ink right=268; flags=['old text right edge 188 > arrow x 140', 'new text right edge 268 > scale x 186', 'new text right edge 268 > box right 239']; warnings=["warning: text 'HOUSE 4' is cut by the canvas (ink 153,39..268,61)"]
- K4 old='HOUSE 3' new='HOUSE 4': box x=120..239; old ink right=188, new ink right=268; flags=['old text right edge 188 > arrow x 140', 'new text right edge 268 > scale x 186', 'new text right edge 268 > box right 239']; warnings=["warning: text 'HOUSE 4' is cut by the canvas (ink 153,39..268,61)"]
title widths 'Kn LABEL' for K1-K4 labels: max (75, 'K2 OFFSET', 'EUCLID') (box inner width 108)


## 5f. labels drawn from el.KN versus knob_map.tsv, all 6 algorithms x 8 knobs (el.KN lookup is KN.get(algo, KN['ACID']); EUCLID has one row)
tiles = 48; label differs or no data = 26; matches = 22
per algorithm mismatches: ACID 0, EUCLID 4, SNAKE 5, STAGE 6, ARP 5, PATTERNS 6
- EUCLID S1: drawn None, TSV LEN
- EUCLID S2: drawn None, TSV SPAN
- EUCLID S3: drawn None, TSV PUNCH
- EUCLID S4: drawn None, TSV CURVE
- SNAKE K2: drawn SLIDE, TSV LEN
- SNAKE K3: drawn JUMP, TSV RANGE
- SNAKE K4: drawn ACCNT, TSV PATH
- SNAKE S2: drawn PEDAL, TSV GLEN
- SNAKE S3: drawn PUNCH, TSV DYN
- STAGE K2: drawn SLIDE, TSV TOTAL
- STAGE K3: drawn JUMP, TSV DIR
- STAGE K4: drawn ACCNT, TSV RATCH
- STAGE S2: drawn PEDAL, TSV RDIV
- STAGE S3: drawn PUNCH, TSV DYN
- STAGE S4: drawn DIR, TSV RANGE
- ARP K2: drawn SLIDE, TSV SHAPE
- ARP K3: drawn JUMP, TSV SPAN
- ARP K4: drawn ACCNT, TSV RATE
- ARP S2: drawn PEDAL, TSV LEGATO
- ARP S3: drawn PUNCH, TSV DYN
- PATTERNS K2: drawn SLIDE, TSV VARY
- PATTERNS K3: drawn JUMP, TSV SPAN
- PATTERNS S1: drawn OFFSET, TSV PICK
- PATTERNS S2: drawn PEDAL, TSV SLIDE
- PATTERNS S3: drawn PUNCH, TSV DYN
- PATTERNS S4: drawn DIR, TSV CTONE

knob_turning (v5) always reads kn[0][k] (row 1 labels) and draws 'K%d <label>' even when ROW 2 is live (S1-S4); v7.screen passes the same kn for both

# 6. Accuracy vs the TSVs (each row: on-screen claim | evidence in code/render | TSV line | verdict)
| # | on-screen claim | what the render/code shows | TSV line | verdict |
|---|---|---|---|---|
| 1 | button FX shows icon 'dice_all' (die + 4 corner pips + centre cross); code tuple role/hold = ALL/ALL | icon drawn, role/hold text NOT drawn (buttons7 ignores role and hold) | button_modes.tsv:2 tap label ALL; :3 hold label ALL | tap role match; code hold label matches TSV; hold role on screen: no |
| 2 | button SCL shows icon 'dice_notes' (die + note glyph); code tuple role/hold = NOTES/NOTES | icon drawn, role/hold text NOT drawn (buttons7 ignores role and hold) | button_modes.tsv:20 tap label NOTES; :21 hold label NOTES | tap role match; code hold label matches TSV; hold role on screen: no |
| 3 | button ENV shows icon 'dice_accnt' (die + rising/falling chevron '>'); code tuple role/hold = ACCNT/ACCNT | icon drawn, role/hold text NOT drawn (buttons7 ignores role and hold) | button_modes.tsv:38 tap label ACCNT; :39 hold label ACCNT | tap role match; code hold label matches TSV; hold role on screen: no |
| 4 | button LFO shows icon 'dice_shape' (die + sine-like wave); code tuple role/hold = SHAPE/SHAPE | icon drawn, role/hold text NOT drawn (buttons7 ignores role and hold) | button_modes.tsv:56 tap label SHAPE; :57 hold label SHAPE | tap role match; code hold label matches TSV; hold role on screen: no |
| 5 | button EDIT shows icon 'undo' (undo arrow); code tuple role/hold = UNDO/RESET | icon drawn, role/hold text NOT drawn (buttons7 ignores role and hold) | button_modes.tsv:74 tap label UNDO; :75 hold label RESET | tap role match; code hold label matches TSV; hold role on screen: no |
| 6 | button GLO shows icon 'settings' (gear); code tuple role/hold = SET/HARM | icon drawn, role/hold text NOT drawn (buttons7 ignores role and hold) | button_modes.tsv:92 tap label SET; :93 hold label HARM | tap role match; code hold label matches TSV; hold role on screen: no |
| 7 | button HOME shows icon 'home' (house); code tuple role/hold = HOME/HLP/MN | icon drawn, role/hold text NOT drawn (buttons7 ignores role and hold) | button_modes.tsv:110 tap label HOME; :111 hold label HLP/MN | tap role match; code hold label matches TSV; hold role on screen: no |
| 8 | button SAVE shows icon 'save' (floppy); code tuple role/hold = SAVE/UNDO L | icon drawn, role/hold text NOT drawn (buttons7 ignores role and hold) | button_modes.tsv:128 tap label SAVE; :129 hold label UNDO L | tap role match; code hold label matches TSV; hold role on screen: no |
| 9 | button ARP shows icon 'dice_artic' (die + bar over block (articulation)); code tuple role/hold = ARTIC/ARTIC | icon drawn, role/hold text NOT drawn (buttons7 ignores role and hold) | button_modes.tsv:146 tap label ARTIC; :147 hold label ARTIC | tap role match; code hold label matches TSV; hold role on screen: no |
| 10 | button SEQ shows icon 'dice_hits' (die + 3 dot columns + base line (hits)); code tuple role/hold = HITS/HITS | icon drawn, role/hold text NOT drawn (buttons7 ignores role and hold) | button_modes.tsv:164 tap label HITS; :165 hold label HITS | tap role match; code hold label matches TSV; hold role on screen: no |
| 11 | button PLAY shows icon 'play' (triangle (stopped) / square (playing)); code tuple role/hold = PLAY/PLAY | icon drawn, role/hold text NOT drawn (buttons7 ignores role and hold) | button_modes.tsv:182 tap label PLAY; :183 hold label PLAY | tap role match; code hold label matches TSV; hold role on screen: no |
| 12 | button REC shows icon 'rec' (filled circle); code tuple role/hold = REC/REC | icon drawn, role/hold text NOT drawn (buttons7 ignores role and hold) | button_modes.tsv:200 tap label REC; :201 hold label REC | tap role match; code hold label matches TSV; hold role on screen: no |
| 13 | black key F#3 (x centre 15) glyph = square in CH1 colour (filled; outline in T.MID when muted; absent if channel unused) | role code M1 at position 1; tap role on TSV | key_map.tsv:210 (KEY_B01_F#3 tap -> M1) | match |
| 14 | black key G#3 (x centre 30) glyph = square CH2 | role code M2 at position 2; tap role on TSV | key_map.tsv:223 (KEY_B02_G#3 tap -> M2) | match |
| 15 | black key A#3 (x centre 45) glyph = square CH3 | role code M3 at position 3; tap role on TSV | key_map.tsv:236 (KEY_B03_A#3 tap -> M3) | match |
| 16 | black key C#4 (x centre 75) glyph = square CH4 | role code M4 at position 4; tap role on TSV | key_map.tsv:249 (KEY_B04_C#4 tap -> M4) | match |
| 17 | black key D#4 (x centre 90) glyph = 2x2 dots in the four channel colours | role code CHN at position 5; tap role on TSV | key_map.tsv:262 (KEY_B05_D#4 tap -> CHN) | match |
| 18 | black key F#4 (x centre 120) glyph = letter S | role code SOL at position 6; hold role on TSV | key_map.tsv:272 (KEY_B06_F#4 hold -> SOL) | match |
| 19 | black key G#4 (x centre 135) glyph = letter F | role code FIL at position 7; hold role on TSV | key_map.tsv:281 (KEY_B07_G#4 hold -> FIL) | match |
| 20 | black key A#4 (x centre 150) glyph = letter R | role code ROL at position 8; hold role on TSV | key_map.tsv:290 (KEY_B08_A#4 hold -> ROL) | match |
| 21 | black key C#5 (x centre 180) glyph = left triangle 5x7 | role code PG- at position 9; tap role on TSV | key_map.tsv:299 (KEY_B09_C#5 tap -> PG-) | match |
| 22 | black key D#5 (x centre 195) glyph = right triangle 5x7 | role code PG+ at position 10; tap role on TSV | key_map.tsv:308 (KEY_B10_D#5 tap -> PG+) | match |
| 23 | black key F#5 (x centre 225) glyph = bar+left-triangle 5x7 | role code RST at position 11; tap role on TSV | key_map.tsv:317 (KEY_B11_F#5 tap -> RST) | match |
| 24 | footer 'KEYS = STEPS' (PERF) | AF_S at x=2 y=225; white keys lit in the selected channel colour when its step is on | key_map.tsv:2 W01..W16 tap = step n ON/OFF (label STEP) | match |
| 25 | footer 'HOLD = EDIT' (PERF) | AF_S MID right-aligned x=238 y=225, same string also contains 'PAGE 1/2' | key_map.tsv:3 hold = open NOTE EDIT (label EDIT) | match for HOLD = EDIT |
| 26 | footer 'PAGE 1/2' | constant string in footer_keybed (no state field read); S has no page key | key_map.tsv:299 PG-/PG+ (view prev/next page); page_states.tsv:2 header page map | static string: no state source in S (the string is the same in every scene) |
| 27 | REC footer 'ALL KEYS PLAY NOTES' | AF_S x=16 y=225 + rec glyph; white keys drawn T.KEY, black keys T.DIM, glyphs hidden | key_map.tsv:11..322: 27 REC rows, 27 with label NOTE (all tap) | match |
| 28 | ACID K1 tile label 'DENS' value '12' fill 75% | label from el.KN | knob_map.tsv:2 label DENS range den 0..16 def 12; 0 = KEYS ONLY (the cel | label match |
| 29 | ACID K2 tile label 'SLIDE' value '4' fill 50% | label from el.KN | knob_map.tsv:3 label SLIDE range sl 0..8 def 4 (was S2) | label match |
| 30 | ACID K3 tile label 'JUMP' value '5' fill 25% | label from el.KN | knob_map.tsv:4 label JUMP range jmp 0..20 def 5 | label match |
| 31 | ACID K4 tile label 'ACCNT' value '3' fill 37% | label from el.KN | knob_map.tsv:5 label ACCNT range acc 0..8 def 3 | label match |
| 32 | ACID S1 tile label 'OFFSET' value '0' fill 0% | label from el.KN | knob_map.tsv:6 label OFFSET range off 0..15 def 0 | label match |
| 33 | ACID S2 tile label 'PEDAL' value '6' fill 50% | label from el.KN | knob_map.tsv:7 label PEDAL range typ 0..12 def 6 (was K2) | label match |
| 34 | ACID S3 tile label 'PUNCH' value '40' fill 85% | label from el.KN | knob_map.tsv:8 label PUNCH range pun 0..47 def 40 fast 4 | label match |
| 35 | ACID S4 tile label 'DIR' value 'FWD' fill 0% | label from el.KN | knob_map.tsv:9 label DIR range dir FWD BWD PEND | label match |
| 36 | EUCLID K1 tile label 'HITS' value '5' fill 31% | label from el.KN | knob_map.tsv:10 label HITS range hit 0..16 def 5; 0 = KEYS ONLY (the cell | label match |
| 37 | EUCLID K2 tile label 'OFFSET' value '0' fill 0% | label from el.KN | knob_map.tsv:11 label OFFSET range off 0..15 def 0 (rotation; was S1) | label match |
| 38 | EUCLID K3 tile label 'RATCH' value '4' fill 50% | label from el.KN | knob_map.tsv:12 label RATCH range rat 0..8 def 4 (was S2) | label match |
| 39 | EUCLID K4 tile label 'ACCNT' value '2' fill 25% | label from el.KN | knob_map.tsv:13 label ACCNT range acc 0..8 def 2 | label match |
| 40 | KN[EUCLID] second row (S1-S4) | el.KN has no row 2 for EUCLID (only 1 row) | knob_map.tsv:14,15,16,17 S1-S4 | MISSING data (render IndexError at row=1) |
| 41 | ACID K1 DENS shows value 12, fill 75% | (value-lo)/(hi-lo) = 75%; TSV default 12 | knob_map.tsv:2 | value==default; fill matches |
| 42 | ACID K2 SLIDE shows value 4, fill 50% | (value-lo)/(hi-lo) = 50%; TSV default 4 | knob_map.tsv:3 | value==default; fill matches |
| 43 | ACID K3 JUMP shows value 5, fill 25% | (value-lo)/(hi-lo) = 25%; TSV default 5 | knob_map.tsv:4 | value==default; fill matches |
| 44 | ACID K4 ACCNT shows value 3, fill 37% | (value-lo)/(hi-lo) = 37%; TSV default 3 | knob_map.tsv:5 | value==default; fill matches |
| 45 | ACID S1 OFFSET shows value 0, fill 0% | (value-lo)/(hi-lo) = 0%; TSV default 0 | knob_map.tsv:6 | value==default; fill matches |
| 46 | ACID S2 PEDAL shows value 6, fill 50% | (value-lo)/(hi-lo) = 50%; TSV default 6 | knob_map.tsv:7 | value==default; fill matches |
| 47 | ACID S3 PUNCH shows value 40, fill 85% | (value-lo)/(hi-lo) = 85%; TSV default 40 | knob_map.tsv:8 | value==default; fill matches |
| 48 | EUCLID K1 HITS shows value 5, fill 31% | (value-lo)/(hi-lo) = 31%; TSV default 5 | knob_map.tsv:10 | value==default; fill matches |
| 49 | EUCLID K2 OFFSET shows value 0, fill 0% | (value-lo)/(hi-lo) = 0%; TSV default 0 | knob_map.tsv:11 | value==default; fill matches |
| 50 | EUCLID K3 RATCH shows value 4, fill 50% | (value-lo)/(hi-lo) = 50%; TSV default 4 | knob_map.tsv:12 | value==default; fill matches |
| 51 | EUCLID K4 ACCNT shows value 2, fill 25% | (value-lo)/(hi-lo) = 25%; TSV default 2 | knob_map.tsv:13 | value==default; fill matches |
| 52 | ACID S4 DIR value 'FWD' fill 0 | el.KN | knob_map.tsv:9 dir FWD BWD PEND | value in set; fill 0 for an enum (FWD = first) |
| 53 | toast 'NOTES DICED' with SCL button lit | toast_top word; SCL cell drawn WHITE bg/INK icon; hatched bars on CH1 steps | button_modes.tsv:20 B_SCL tap = dice the NOTES layer | match |
| 54 | toast hint 'EDIT=UNDO' | toast_top hint AF_M right-aligned | button_modes.tsv:74 B_EDIT tap = UNDO / REDO the last dice | match (tap); EDIT hold = RESET (:75) not mentioned |
| 55 | header algo name drawn as algo[:6] | ACID/EUCLID/SNAKE/STAGE/ARP unchanged; PATTERNS -> 'PATTER' | knob_map.tsv:87 ALGO names ACID EUCLID SNAKE STAGE ARP PATTERNS; page_states.tsv:14 short names ACID EUCL SNAK STAG ARP PATT | MISMATCH for PATTERNS (PATTER is neither name nor the PATT short form) |
| 56 | header preset 'P01' / pending 'P01?' in a white frame | pend frame x=196..238; text 'Pnn?' | knob_map.tsv:88 PRESET: header 'Pnn?' then 'Pnn>' while playing | 'Pnn?' match; 'Pnn>' (queued while playing) not drawn |
| 57 | header transport icon: play triangle when playing, circle (REC glyph, T.MID) when stopped | enc_row uses IC['play'] if playing else IC['rec']; the PLAY button shows a stop square while playing | button_modes.tsv:182 B_PLAY tap start/stop; :200 B_REC tap open REC | stopped-state header glyph is the filled-circle bitmap IC[rec] (11x11, T.MID), the same shape as the REC button icon; playing-state header glyph is the play triangle while the PLAY button then shows a stop square |
| 58 | dice-group tint (DICE_BG + amber icon) on 6 buttons | FX SCL ENV LFO ARP SEQ | button_modes.tsv:2,20,38,56,146,164 tap text contains 'dice' | match (6 of 6 TSV dice buttons found) |
| 59 | 16 white-key cells at 15 px pitch, 11 black keys at x=15 30 45; 75 90; 120 135 150; 180 195; 225 | counts from render: 16 white, 11 black | key_map.tsv PERF: KEY_W01..W16 (32 rows), KEY_B01..B11 | match |
| 60 | muted channel bars drawn as T.DIM outlines; M-key glyph as T.MID outline | music_slim/footer_keybed muted branch | key_map.tsv:210 M1 tap = mute / unmute CH1 (latched) | match |
| 61 | accent = 2 px WHITE cap above the bar | music_slim acc flag | key_map.tsv:2 hard tap turning a step ON = ACCENT (velocity 120) | match in meaning; TSV has no on-screen form |
| 62 | BPM '120' in header | enc_row bpm default 120; v7 passes no BPM from S | page_states.tsv:2 header: clock dot + BPM with tempo source INT / EXT n / EXT -- / WAIT | static value; tempo-source word and clock dot not drawn |
| 63 | REC armed: REC button icon fg = T.REC | buttons7 rec=True; mock-MONO T.REC=#ff484a, firmware-MONO T.REC=text white (see 3 / 8) | button_modes.tsv:200 B_REC tap open REC (latched, LED lit) | mock-MONO distinct from unarmed; firmware-MONO identical colour to T.TEXT |
| 64 | channel chips 1..4 (selected = filled) | chip digits; '+' at slot len(used) only | knob_map.tsv:86 SELECT CH1..CH4 | match; '+' missing for non-contiguous sets (see 5d) |
| 65 | MIDI channel (page_states.tsv:2 header lists MIDI) | not drawn anywhere; chans[].midi never read (16 vs 2: 0 px differ) | page_states.tsv:2 PERF screen: header ... MIDI | not shown |

# 7. Coverage of PERF controls x gestures (strict: shown in at least one of the six scenes; 'only on press' = appears only as press/turn feedback)

## row-level (every PERF row in the three TSVs)
- button_modes: 30 rows: yes 12, no 10, n/a (no role) 8, only on press 0
- key_map: 48 rows: yes 43, no 5, n/a (no role) 0, only on press 0
- knob_map: 59 rows: yes 11, no 48, n/a (no role) 0, only on press 0
- ALL: 137 rows: yes 66 (48.2%), no 63, n/a 8; excluding the 8 'no hold role' rows: yes 66 of 129 = 51.2%

## control-level (knob rows collapsed to one control per knob name: K1-4, S1-4, R1-4, SELECT, ALGO, PRESET = 15 controls x turn)
knob controls: {'K1': 'yes', 'K2': 'yes', 'K3': 'yes', 'K4': 'yes', 'SELECT': 'yes', 'ALGO': 'yes', 'PRESET': 'yes', 'R1': 'no', 'R2': 'no', 'R3': 'no', 'R4': 'no', 'S1': 'no', 'S2': 'no', 'S3': 'no', 'S4': 'no'}
units = 30 button rows + 48 key rows + 15 knob controls = 93; yes = 62 (66.7%); excluding 8 'no hold role' units: 62 of 85 = 72.9%

## detail (file:line control gesture role -> shown / how)
| file:line | control | gesture | TSV label | shown | how |
|---|---|---|---|---|---|
| button_modes:2 | B_FX | tap | ALL | yes | icon in the 2x6 block |
| button_modes:3 | B_FX | hold | ALL | n/a | TSV: no hold role |
| button_modes:20 | B_SCL | tap | NOTES | yes | icon in the 2x6 block |
| button_modes:21 | B_SCL | hold | NOTES | n/a | TSV: no hold role |
| button_modes:38 | B_ENV | tap | ACCNT | yes | icon in the 2x6 block |
| button_modes:39 | B_ENV | hold | ACCNT | n/a | TSV: no hold role |
| button_modes:56 | B_LFO | tap | SHAPE | yes | icon in the 2x6 block |
| button_modes:57 | B_LFO | hold | SHAPE | n/a | TSV: no hold role |
| button_modes:74 | B_EDIT | tap | UNDO | yes | icon in the 2x6 block |
| button_modes:75 | B_EDIT | hold | RESET | no | hold role label (RESET) is in the code tuple but not drawn |
| button_modes:92 | B_GLO | tap | SET | yes | icon in the 2x6 block |
| button_modes:93 | B_GLO | hold | HARM | no | hold role label (HARM) is in the code tuple but not drawn |
| button_modes:110 | B_HOME | tap | HOME | yes | icon in the 2x6 block |
| button_modes:111 | B_HOME | hold | HLP/MN | no | hold role label (HLP/MN) is in the code tuple but not drawn |
| button_modes:128 | B_SAVE | tap | SAVE | yes | icon in the 2x6 block |
| button_modes:129 | B_SAVE | hold | UNDO L | no | hold role label (UNDO L) is in the code tuple but not drawn |
| button_modes:146 | B_ARP | tap | ARTIC | yes | icon in the 2x6 block |
| button_modes:147 | B_ARP | hold | ARTIC | n/a | TSV: no hold role |
| button_modes:164 | B_SEQ | tap | HITS | yes | icon in the 2x6 block |
| button_modes:165 | B_SEQ | hold | HITS | n/a | TSV: no hold role |
| button_modes:182 | B_PLAY | tap | PLAY | yes | icon in the 2x6 block |
| button_modes:183 | B_PLAY | hold | PLAY | n/a | TSV: no hold role |
| button_modes:200 | B_REC | tap | REC | yes | icon in the 2x6 block |
| button_modes:201 | B_REC | hold | REC | n/a | TSV: no hold role |
| button_modes:218 | B_OCTDN | tap | ROW 1 | no | button not drawn; only its result (row marker, 3 squares) is drawn |
| button_modes:219 | B_OCTDN | hold | ROW 1 | no | hold role label (ROW 1) is in the code tuple but not drawn |
| button_modes:220 | B_OCTDN | hold-both | UPDATE | no | hold role label (UPDATE) is in the code tuple but not drawn |
| button_modes:237 | B_OCTUP | tap | ROW 2 | no | button not drawn; only its result (row marker, 3 squares) is drawn |
| button_modes:238 | B_OCTUP | hold | ROW 2 | no | hold role label (ROW 2) is in the code tuple but not drawn |
| button_modes:239 | B_OCTUP | hold-both | UPDATE | no | hold role label (UPDATE) is in the code tuple but not drawn |
| key_map:2 | KEY_W01_F3 | tap | STEP | yes | caption 'KEYS = STEPS'; lit key = step on |
| key_map:3 | KEY_W01_F3 | hold | EDIT | yes | caption 'HOLD = EDIT'; lit key = step on |
| key_map:197 | KEY_W16_G5 | tap | STEP | yes | caption 'KEYS = STEPS'; lit key = step on |
| key_map:198 | KEY_W16_G5 | hold | EDIT | yes | caption 'HOLD = EDIT'; lit key = step on |
| key_map:210 | KEY_B01_F#3 | tap | M1 | yes | square glyph (absent when channel not in use) |
| key_map:211 | KEY_B01_F#3 | hold | PRV1 | no | no preview indicator on screen |
| key_map:223 | KEY_B02_G#3 | tap | M2 | yes | square glyph (absent when channel not in use) |
| key_map:224 | KEY_B02_G#3 | hold | PRV2 | no | no preview indicator on screen |
| key_map:236 | KEY_B03_A#3 | tap | M3 | yes | square glyph (absent when channel not in use) |
| key_map:237 | KEY_B03_A#3 | hold | PRV3 | no | no preview indicator on screen |
| key_map:249 | KEY_B04_C#4 | tap | M4 | yes | square glyph (absent when channel not in use) |
| key_map:250 | KEY_B04_C#4 | hold | PRV4 | no | no preview indicator on screen |
| key_map:262 | KEY_B05_D#4 | tap | CHN | yes | 2x2 colour-dot glyph |
| key_map:263 | KEY_B05_D#4 | hold | RIG | no | hold role not drawn |
| key_map:272 | KEY_B06_F#4 | hold | SOL | yes | glyph |
| key_map:281 | KEY_B07_G#4 | hold | FIL | yes | glyph |
| key_map:290 | KEY_B08_A#4 | hold | ROL | yes | glyph |
| key_map:299 | KEY_B09_C#5 | tap | PG- | yes | glyph |
| key_map:308 | KEY_B10_D#5 | tap | PG+ | yes | glyph |
| key_map:317 | KEY_B11_F#5 | tap | RST | yes | glyph |
| knob_map:2 | ACID/K1 | turn | DENS | yes | ACID tile DENS in all 5 non-turning scenes |
| knob_map:3 | ACID/K2 | turn | SLIDE | yes | ACID tile SLIDE in all 5 non-turning scenes |
| knob_map:4 | ACID/K3 | turn | JUMP | yes | ACID tile JUMP in all 5 non-turning scenes |
| knob_map:5 | ACID/K4 | turn | ACCNT | yes | ACID tile ACCNT in all 5 non-turning scenes |
| knob_map:6 | ACID/S1 | turn | OFFSET | no | row 2 data exists in el.KN but no scene has row=1 |
| knob_map:7 | ACID/S2 | turn | PEDAL | no | row 2 data exists in el.KN but no scene has row=1 |
| knob_map:8 | ACID/S3 | turn | PUNCH | no | row 2 data exists in el.KN but no scene has row=1 |
| knob_map:9 | ACID/S4 | turn | DIR | no | row 2 data exists in el.KN but no scene has row=1 |
| knob_map:10 | EUCLID/K1 | turn | HITS | no | el.KN has EUCLID row 1 but no scene selects CH2 (EUCLID) |
| knob_map:11 | EUCLID/K2 | turn | OFFSET | no | el.KN has EUCLID row 1 but no scene selects CH2 (EUCLID) |
| knob_map:12 | EUCLID/K3 | turn | RATCH | no | el.KN has EUCLID row 1 but no scene selects CH2 (EUCLID) |
| knob_map:13 | EUCLID/K4 | turn | ACCNT | no | el.KN has EUCLID row 1 but no scene selects CH2 (EUCLID) |
| knob_map:14 | EUCLID/S1 | turn | LEN | no | no row 2 in el.KN (IndexError) |
| knob_map:15 | EUCLID/S2 | turn | SPAN | no | no row 2 in el.KN (IndexError) |
| knob_map:16 | EUCLID/S3 | turn | PUNCH | no | no row 2 in el.KN (IndexError) |
| knob_map:17 | EUCLID/S4 | turn | CURVE | no | no row 2 in el.KN (IndexError) |
| knob_map:82 | */K1 | turn | morph | yes | tile row 1 |
| knob_map:83 | */K2 | turn | morph | yes | tile row 1 |
| knob_map:84 | */K3 | turn | morph | yes | tile row 1 |
| knob_map:85 | */K4 | turn | morph | yes | tile row 1 |
| knob_map:86 | */SELECT | turn | CH n | yes | channel chips |
| knob_map:87 | */ALGO | turn | ALGO | yes | algo glyph + name (PATTERNS cut to PATTER) |
| knob_map:88 | */PRESET | turn | Pnn | yes | Pnn text; Pnn? frame only while pending |
| knob_map:113 | */R1 | turn | WIRE1 | no | ROW 3 (WIRE n) not drawn; el.KN has no 3rd row (IndexError) |
| knob_map:114 | */R2 | turn | WIRE2 | no | ROW 3 (WIRE n) not drawn; el.KN has no 3rd row (IndexError) |
| knob_map:115 | */R3 | turn | WIRE3 | no | ROW 3 (WIRE n) not drawn; el.KN has no 3rd row (IndexError) |
| knob_map:116 | */R4 | turn | WIRE4 | no | ROW 3 (WIRE n) not drawn; el.KN has no 3rd row (IndexError) |
(white keys W02..W15 and the 4 other algorithms' knob rows are collapsed in this listing; counted above)

# 8. Firmware-cost facts
## 8a. bitmaps
icons_v7.ICONS: 13 bitmaps (6 dice 13x13, 7 others 13x11); bytes at 1 bpp row-padded to byte = 310; packed bits = 2015 (= 252 bytes)
el.ALGO_IC (header algorithm glyphs): 6 bitmaps, sizes {'ACID': '11x6', 'EUCLID': '9x7', 'SNAKE': '9x5', 'STAGE': '9x5', 'ARP': '9x5', 'PATTERNS': '9x5'}, row-padded bytes = 66
el.IC bitmaps drawn by the v7 code paths in the six scenes: ['left', 'note', 'play', 'rec', 'right', 'rst'] sizes {'left': '5x7', 'note': '11x11', 'play': '11x11', 'rec': '11x11', 'right': '5x7', 'rst': '5x7'}, row-padded bytes = 87
distinct bitmaps actually drawn across the six scenes: 20; row-padded bytes = 409; packed bits = 2549 (319 bytes)
bitmap draws per scene (art ops): playing=17, one_channel=17, two_channels_stopped=17, dice=18, turning_k2=18, rec_armed=15
duplicates by shape: IC.play (11x11) vs icons_v7.play (13x11) and IC.rec (11x11) vs icons_v7.rec (13x11) are separate bitmaps; both pairs drawn in the same scene

## 8b. colours that are not palette tokens (RGB565)
| RGB565 | RGB888 | role in v7 | equals a palette token in GREY (default) | MONO as mock parses | MONO per ui_palettes.h (idx 9) |
|---|---|---|---|---|---|
| 0xfca5 | #ff9529 | CH_COL[0] (channel 1) | no | no | no |
| 0x45ff | #41beff | CH_COL[1] (channel 2) | no | no | no |
| 0x96ea | #94de52 | CH_COL[2] (channel 3) | no | no | no |
| 0xfb76 | #ff6db4 | CH_COL[3] (channel 4) | no | no | no |
| 0xffdf | #fffaff | WHITE rgb565(250,250,250) | accent,rec | no | no |
| 0x0000 | #000000 | INK rgb565(0,0,0) | no | bg,surf,ink | bg,surf,tint,ink,grid |
| 0x4228 | #414441 | PH rgb565(70,70,70) playhead column | no | no | no |
| 0x3142 | #312810 | DICE_BG rgb565(48,40,16) | no | no | no |
| 0xfe6b | #ffce5a | DICE amber rgb565(255,205,90) icon fg | no | no | no |
| 0x69e2 | #6a3c10 | knob fill mix(CH_COL[0],110) | no | no | no |
| 0x1a8d | #18506a | knob fill mix(CH_COL[1],110) | no | no | no |
| 0x3ae4 | #395d20 | knob fill mix(CH_COL[2],110) | no | no | no |
| 0x6969 | #6a2c4a | knob fill mix(CH_COL[3],110) | no | no | no |
distinct non-token colour values (vs GREY tokens): 12; of them chromatic (max-min channel > 6): 10
colour constants defined in el.py/icons_v7.py/v7.py and not taken from T: CH_COL x4, WHITE, INK, PH, DICE_BG, amber, plus 4 derived mixes = 13 values

## 8c. GREY palette (firmware default, mono=1): coloured text
mock._ramp mono branch (gfx.c ramp mirrored): `if ux.mono: x = bg >> 11; v.append(ux.gray(x + ((((fg >> 11) - x) * w + 128) >> 8)))` : only the red 5-bit channel of fg and bg is used, output is gray(x5) (r=g=b)
GREY tokens: bg #080808 surf #202020 text #cdcacd mid #949194 dim #5a595a line #292829 raise #393839; mono flag=True
| colour text | fg RGB | red5 used | GREY ramp peak (nibble 15) | peak vs bg pixel contrast | in v7 as |
|---|---|---|---|---|---|
| CH1 | #ff9529 | 31/31 | #fffaff (r=g=b) | 19.43 | chip digit / algo name / turning title on BG |
| CH2 | #41beff | 8/31 | #414041 (r=g=b) | 1.94 | chip digit / algo name / turning title on BG |
| CH3 | #94de52 | 18/31 | #949194 (r=g=b) | 6.42 | chip digit / algo name / turning title on BG |
| CH4 | #ff6db4 | 31/31 | #fffaff (r=g=b) | 19.43 | chip digit / algo name / turning title on BG |
| WHITE | #fffaff | 31/31 | #fffaff | 19.43 (vs real bg #080808) | - |
| T.TEXT | #cdcacd | 25/31 | #cdcacd | 12.33 (vs real bg #080808) | - |
| T.MID | #949194 | 18/31 | #949194 | 6.42 (vs real bg #080808) | - |
| INK on CH1 | #000000 | 0/31 | #000000 | 9.58 (vs real bg #ff9529) | - |

GREY render of scene 'playing': text draws and the brightest pixel inside each ink box (screen after render) vs the pixel colour behind it (no-text render):
| text | font | fg declared | brightest ink pixel in render | contrast vs declared bg (as drawn) | contrast vs pixel behind |
|---|---|---|---|---|---|
| '120' | AF_M | #cdcacd TEXT | #cdcacd | 12.33 | 12.33 |
| '1' | AF_M | #000000 INK | #000000 | 9.58 | 9.58 |
| '2' | AF_M | #41beff CH2 | #414041 | 1.94 | 1.94 |
| '3' | AF_M | #94de52 CH3 | #949194 | 6.42 | 6.42 |
| '4' | AF_M | #ff6db4 CH4 | #fffaff | 19.43 | 19.43 |
| 'ACID' | AF_M | #ff9529 CH1 | #fffaff | 19.43 | 19.43 |
| 'P01' | AF_M | #cdcacd TEXT | #cdcacd | 12.33 | 12.33 |
| 'DENS' | AF_M | #fffaff REC/WHITE | #fffaff | 11.32 | 11.32 |
| 'SLIDE' | AF_M | #fffaff REC/WHITE | #fffaff | 11.32 | 11.32 |
| 'JUMP' | AF_M | #fffaff REC/WHITE | #fffaff | 11.32 | 11.32 |
| 'ACCNT' | AF_M | #fffaff REC/WHITE | #fffaff | 11.32 | 11.32 |
| '12' | AF_L | #fffaff REC/WHITE | #fffaff | 8.99 | 8.99 |
| '4' | AF_L | #fffaff REC/WHITE | #fffaff | 8.99 | 11.32 |
| '5' | AF_L | #fffaff REC/WHITE | #fffaff | 11.32 | 11.32 |
| '3' | AF_L | #fffaff REC/WHITE | #fffaff | 11.32 | 11.32 |
| 'S' | AF_S | #cdcacd TEXT | #c5c2c5 | 8.32 | 8.32 |
| 'F' | AF_S | #cdcacd TEXT | #c5c2c5 | 8.32 | 8.32 |
| 'R' | AF_S | #cdcacd TEXT | #cdcacd | 9.04 | 9.04 |
| 'KEYS = STEPS' | AF_S | #cdcacd TEXT | #cdcacd | 12.33 | 12.33 |
| 'HOLD = EDIT  PAGE 1/2' | AF_S | #949194 MID | #949194 | 6.42 | 6.42 |

GREY render 'playing': 14450 of 57600 pixels are non-neutral (chromatic): channel-colour rects/frames/bars/chips are still drawn in colour; text pixels are neutral grey ramps
text-ink pixels (changed by text) = 3874; chromatic among them = 0

## 8d. REC-armed button colour under each palette
- mock-MONO: T.REC=#ff484a T.TEXT=#ffffff; REC cell pixels differing armed vs not = 111
- fw-MONO (idx 9): T.REC=#ffffff T.TEXT=#ffffff; REC cell pixels differing armed vs not = 0
- GREY: T.REC=#fffaff T.TEXT=#cdcacd; REC cell pixels differing armed vs not = 111
