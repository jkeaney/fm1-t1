# SCREEN_SPEC: the 240x240 screen, zone by zone (P2-003)

Status: **proposed**, for the owner's human-review gate. Nothing here is approved until the owner writes it in the
P2-003 log. It makes VISION rule 1 concrete: at every moment the screen shows what the device is **doing** and the
current role of **every** knob and button. It adds pixels to the locked UX ([SEQUENCER_UX.md](SEQUENCER_UX.md), P1-017
approved); it changes none of its decisions (SET K1 stays as locked: ignored while an external clock runs). Where a locked mock-up cannot be drawn as printed, section 9 lists the
finding and the resolution chosen here, and the owner can overrule it.

Inputs: SEQUENCER_UX sections 2, 4, 11; `page_states.tsv`, `button_modes.tsv`, `knob_map.tsv`, `key_map.tsv`,
`control_map.tsv`; CONTROL_MAP; PERFORMANCE_DESIGN 6; FULL_CONTROL; `firmware/src/gfx.c` (canvas, themes, fonts);
FB-001 (tempo source and clock out, owner decision 2026-11-10: v1 sends clock as master); P2-010 / [TIMING_SPEC.md](TIMING_SPEC.md) (the clock architecture and the owner rulings of 2026-11-10: section 6 below is rewritten to them and supersedes the first draft).

## 1. Drawing facts this spec is measured against

| Fact | Value | Source |
| --- | --- | --- |
| Screen | 240x240 RGB565 | CLAUDE.md |
| Canvas | one blit is at most 240x124 px (`CV_MAX`) | gfx.c |
| Fonts | `AF_S` 12 px line, caps 9 px; `AF_M` 15 px line, caps 11 px; `AF_L` 28 px line, caps 21 px | build/gen/ui_fonts.h |
| Text widths (px, measured with the real `text_w` on the real glyph tables) | AF_S: `0` 7, `W` 11, `EXT 126` 45, `INT 120` 42, `ACCENT` 46, `RESTRT` 42, `SET` 21; AF_M: `EXT 126` 57, `ACCENT` 58; AF_L: `120` 54 | probe built from gfx.c |
| Theme colours | `T_BG T_SURF T_TEXT T_THEME T_ACCENT T_MID T_DIM T_LINE T_SEL T_TINT T_INK T_REC T_RAISE T_KEY T_LANE T_GRID` | gfx.c |
| Redraw rule | redraw a zone only when its state signature changes or `ui.force` is set | CLAUDE.md |

**Consequence.** The mock-ups of SEQUENCER_UX 2 are "40 columns of about 6 px". The real AF_S average is 6.5 px per
character and AF_M is wider still (section 9), so every label in this spec is drawn in **AF_S** and checked by width.
AF_M is used only for one-word banners (REC, RESET, NOTE EDIT).

## 2. The zones (the data table; a layout change is an edit of these rows)

`Z` rows tile 240x240 exactly: each row starts where the one above ends, every row is 240 wide, no overlap, no gap.

| Zone | Name | x,y,w,h | Font | Holds |
| --- | --- | --- | --- | --- |
| Z0 | HEAD | 0,0,240,20 | AF_S | transport, tempo source, preset, channel and algorithm, page map, length |
| Z1 | CHANNELS | 0,20,240,28 | AF_S | 4 channel cells (60x28 each): the encoder SELECT / ALGO targets |
| Z2 | LANE | 0,48,240,60 | AF_S, AF_M | lane header line (14 px) + 16 step cells (15x46 each) |
| Z3 | BAND | 0,108,240,52 | AF_S | the knob band: tag column (OCT labels) + 4 knob cells (+ a second row on PERF) |
| Z4 | BUTTONS | 0,160,240,52 | AF_S | the button picture: 2 rows x 6 buttons, tap label line + sub-label line |
| Z5 | LEGEND | 0,212,240,12 | AF_S | the 11 black keys, 3-2-3-2-1 grouping |
| Z6 | CUE | 0,224,240,12 | AF_S | the cue strip (34 characters at most) |
| Z7 | MARGIN | 0,236,240,4 | none | blank (T_BG): the bezel overscan; nothing is drawn |

Areas: 4800 + 6720 + 14400 + 12480 + 12480 + 2880 + 2880 + 960 = 57600 = 240x240. Blits (each at most 240x124): A =
Z0-Z2 (240x108), B = Z3-Z4 (240x104), C = Z5-Z7 (240x28). A zone is redrawn into its blit's canvas and the blit is
sent when any zone in it changed.

MENU, HELP and SCALE replace parts of this layout; section 7 lists what each page changes.

### 2.1 Z0 HEAD slots

Slots are contiguous from x=2 (a 2 px inner margin each side). Text is left-aligned in its slot unless noted.

| Slot | x,y,w,h | Content (AF_S) | Longest string and width |
| --- | --- | --- | --- |
| H1 | 2,4,10,12 | transport: `>` playing, `.` stopped, `R` REC armed (T_REC); a lost external clock shows `.` too (6.2) | 1 glyph |
| H2 | 12,4,56,12 | **clock dot (8x8 at x=12,y=6) and tempo source** (section 6): dot + text from x=23 | `EXT 126` 45 (23+45 = 68), `INT 120` 42, `EXT --` 34, `WAIT` 28 |
| H3 | 68,4,28,12 | preset: `P01`, `P05?` pending, `P05>` loading | `P05?` 27 |
| H4 | 96,4,76,12 | `CH1 ACID M1` channel, algorithm (4 letters), MIDI channel | `CH1 ACID M1` 72 |
| H5 | 172,4,42,12 | page map `[1][2]. .` (viewed outlined, playing filled) | 38 |
| H6 | 214,4,24,12 | `L32` pattern length | `L64` 20 |

The header is on every page except MENU (own title) and HELP's dimmed copy. 2+10+56+28+76+42+24 = 238, plus 2 inner
margin at the right = 240.

### 2.2 Z1 CHANNELS cells

4 cells at x = 0, 60, 120, 180, each 60x28. Line 1 (y+2): channel number and algorithm name (`1 ACID`). Line 2 (y+14):
`MIDI 2` and, when muted, `MUTE` (dim). The selected cell is drawn inverted (T_SEL fill, T_INK text); a 4x4 activity dot
at the cell's right edge flashes on each note out. The SELECT encoder picks the cell, the ALGO encoder renames its
algorithm field, the M1..M4 black keys mute a cell.

### 2.3 Z2 LANE

- Lane header line, y=48..62: `PG 1/2  FOLLOW  hold=EDIT        >P2` (viewed page / pages, FOLLOW, the white-key hold,
  the playhead page). One line, 14 px, AF_S.
- 16 step cells, y=62..108, each x = 15 n, w=15 (n=0..15), 14 px of ink and a 1 px gap. Each cell: top 32 px a velocity
  bar (lit = a step plays, height = velocity), bottom 14 px one glyph row: `'` edited, `!` accent, `~` slide, `x2` ratchet.
  Cell n is drawn in the same left-to-right order as white key n+1, so the lane is the labels of the 16 white keys.
- The playhead is a T_ACCENT underline of the cell. Cells past LENGTH are dim.

### 2.4 Z3 BAND slots

The band has a **tag column** and **4 knob cells**; 36 + 4x51 = 240.

| Slot | x,y,w,h | Content |
| --- | --- | --- |
| BT | 0,108,36,52 | tag column = the OCT- (upper half, y=108..134) and OCT+ (lower half, y=134..160) button labels of the page; see 7. PERF: `OCT-` over `h:peek`, `OCT+` over `h:peek` |
| BK1..BK4 | 36+51(i-1), 108, 51, 26 or 52 | one knob cell: line 1 the label (AF_S, T_MID), line 2 the value (AF_S, T_THEME, T_ACCENT while the knob is hot); width 51 holds the longest knob label `FOLLOW` (47) |
| BS1..BS4 | 36+51(i-1), 134, 51, 26 | PERF only: ROW 2 (S1..S4), drawn dim; the live row is filled (T_SEL) |

On single-row pages (NOTE_EDIT, CHAN, SET, SCALE, REC) the knob cells use the full 52 px: label (y+2), value (y+16),
and the lower 24 px hold two **status lines** (AF_S, 12 px each) at y=136 and y=148 spanning x=36..240 (204 px). The
status lines are table data per page (7).

### 2.5 Z4 BUTTONS

2 rows of 6 buttons, row 1 at y=160..186, row 2 at y=186..212 (26 px each; tap label y+1, sub-label y+13).

| Row | Buttons left to right | Notes |
| --- | --- | --- |
| 1 | FX SCL ENV LFO EDIT GLO | the six dice and the two page buttons |
| 2 | HOME SAVE ARP SEQ PLAY REC | OCT- / OCT+ are labelled in the band tag column BT, not here |

Cell widths are **data per page**: the widest label of the cell (tap label, dice sub-label or hold sub-label, with
the hold mark counted) plus a padding of 1 to 3 px, then widened to fill 240. PERF is the tightest page; measured with
the real `text_w` (AF_S):

| PERF row 1 | FX | SCL | ENV | LFO | EDIT | GLO | sum |
| --- | --- | --- | --- | --- | --- | --- | --- |
| widest label | `ALL` 20 | `NOTES` 38 | `ACCNT` 39 | `SHAPE` 37 | `UNDO` 33 | mark 6 + `SCALE` 36 = 42 | 209 |
| cell width (padding 3, spare given to the last two) | 24 | 41 | 42 | 47 | 36 | 50 | 240 |

| PERF row 2 | HOME | SAVE | ARP | SEQ | PLAY | REC | sum |
| --- | --- | --- | --- | --- | --- | --- | --- |
| widest label | mark 6 + `HLP/MN` 44 = 50 | mark 6 + `UNDO L` 42 = 48 | `GT+SLD` 43 | `ROTATE` 42 | `PLAY` 28 | `REC` 22 | 233 |
| cell width (padding 1, the spare 1 px on the last) | 51 | 49 | 44 | 43 | 29 | 24 | 240 |

Without the hold mark the printed row needs `h:HLP/MN` 53 + `h:UNDO L` 52 + 43 + 42 + 28 + 22 = 240 of label ink alone
plus the OCT cells and padding (about 326 px with `h:peek` x2 and 3 px padding, F2). The hold sub-label carries a 5x7 px
**hold mark** (a filled down-triangle) plus a 1 px gap (6 px) in place of the printed `h:` (9 px): it saves **3 px**
per hold sub-label, not 9. What makes row 2 fit is moving the OCT cells to the band and cutting the padding to 1 px on
this row (`233 + 6 x 1 = 239 <= 240`). The cell separator is a 1 px T_LINE column inside the padding. A button whose layer is absent for the algorithm is dim with a
dash. A button whose tap label differs from the printed word (RST, CANCEL, BACK) is drawn in T_ACCENT.

### 2.6 Z5 LEGEND and Z6 CUE

- Z5: 11 cells of 20 px at pitch 21 in the physical 3-2-3-2-1 grouping with a 3 px gap between groups (the cells are 20 px wide at a 21 px pitch, so the gap
  between groups is 1 px of pitch gap plus 2 px = 3 px from one cell's right edge to the next group's left edge):
  `LEG_X` = 0, 21, 42, 65, 86, 109, 130, 151, 174, 195, 218 (cell 3 ends at 62, cell 4 starts at 65; the last cell ends at 238); each cell shows the page's label for that black key (max 6
  characters); inverted while the key is down; muted-channel cells dim.
- Z6: one line, 34 characters, priority table SEQUENCER_UX 11.2 (plus the rows 8a, 13a, 13b added in section 6.6).

## 3. Control labels: where every control shows its role

Every one of the 49 controls of CONTROL_MAP has a label slot on every page. The label text is the `on_screen_label` of
`control_map.tsv`; this table says **where** it is drawn (the zone slot) and in which font. A page change changes the
text, never the slot.

| Control group (CONTROL_MAP) | Count | Label slot | Notes |
| --- | --- | --- | --- |
| `B_FX B_SCL B_ENV B_LFO B_EDIT B_GLO` | 6 | Z4 row 1 cell 1..6: tap label (line 1), dice or hold sub-label (line 2) | |
| `B_HOME B_SAVE B_ARP B_SEQ B_PLAY B_REC` | 6 | Z4 row 2 cell 1..6 | |
| `B_OCTDN B_OCTUP` | 2 | Z3 tag column BT, upper and lower half | tap label on line 1, hold sub-label on line 2 |
| `EN_K1..EN_K4` | 4 | Z3 BK1..BK4 (label line); PERF also BS1..BS4 for the S row | |
| `EN_SELECT` | 1 | Z1 the selected cell (inverted); where Z1 is replaced by a banner (NOTE_EDIT, REC) the label `CH n` is the first token of the banner | label `CH n` |
| `EN_ALGO` | 1 | Z1 the selected cell, line 1 algorithm name; Z0 H4 | label `ALGO` |
| `EN_PRESET` | 1 | Z0 H3 | label `Pnn` |
| `KEY_W01..KEY_W16` (16 white) | 16 | Z2 cells 1..16 (the lane IS the legend); on SCALE the Z2 area shows the 27-key picture and each key shows its pitch | `STEP` / `NOTE` / `KEY` / `CLR` |
| `KEY_B01..KEY_B11` (11 black) | 11 | Z5 cells 1..11 | |
| `MASTER` | (not a panel control) | none | |

Per page (the nine contexts of the tables), the slots that change meaning:

| Page | Z1 | Z2 | Z3 band | Z4 | Z5 | Z6 |
| --- | --- | --- | --- | --- | --- | --- |
| PERF | 4 cells | lane + header | tag `OCT-`/`OCT+` `h:peek`; ROW 1 live (K1..K4) + ROW 2 dim (S1..S4) | dice row + holds | M1..M4 CHN SOL FIL ROL PG- PG+ RST | rows 13/14/15 of 11.2 |
| NOTE_EDIT | banner replaces cells: `CH1 NOTE EDIT STEP 21` (`CH n` = the EN_SELECT label) | edited cell boxed | tag `OCT-`/`OCT+` (step octave); K1..K4 PITCH GATE VELO RATCH, value + `AUTO`/`*` | dice-this-step labels (ALL NOTE ACCENT PROB UNDO SET / HOME SAVE SLIDE ON/OFF PLAY REC) | as PERF | `STEP n: KNOBS EDIT  HOME=done` |
| CHAN | 4 cells | page map cells (4) | tag `-PAGE`/`+PAGE`; K1..K4 LENGTH MIDI SOURCE FOLLOW; status lines: page-source summary, `D#4 = BACK` | as PERF | D#4 reads `BACK` | row 6 |
| SET | 4 cells | lane as PERF | tag `-`/`-` (inert); K1..K4 BPM SWING GATE DIV; **status lines: tempo source and clock out (6.5)** | `GLO` reads `BACK`, hold `SCALE` | as PERF | row 8 (or 8a, 6.6) |
| SCALE | 4 cells | 27-key picture, scale tones lit, root boxed | tag `OCT-`/`OCT+` = `OCTAVE -`/`+`; K1..K4 SCALE KEY OCTAVE `-` | as SET | all 27 keys read `KEY` | row 7 |
| REC | banner `CH1 REC writing to page 2` (T_REC; `CH n` = the EN_SELECT label) | write cursor `^` | tag `OCT-`/`OCT+` = `OCT -`/`+`; K1..K4 CURSOR ACC SLIDE OCT; status `KEYS = NOTES (all 27)` | dice labels live (performing while recording is allowed) | `KEYS = NOTES` | row 4 |
| RESET | the page beneath, banner `RESET`; ALGO / PRESET encoders read `-` (no role) in the Z0 H3 / H4 slots | every resettable cell tagged `RST` | K1..K4 = `RST` over the value being reset; tag column BT: `RST R1` (OCT-) over `RST R2` (OCT+) | `RST AL`, `RST NT`... (two-line wrap, 9) | white keys `CLR`; D#4 = re-copy | row 2 |
| HELP | the page beneath dimmed | dimmed | dimmed | dimmed | dimmed | 2-line help box over Z3..Z4 (y=108..212, x=8..232) naming the touched control | row 1 |
| MENU | whole screen: Felucca menu. Control labels (control_map): OCT- / OCT+ and the page buttons read `close` (HOME), the SELECT encoder `tab`, the ALGO encoder `row`, K1..K4 the row's value; they are drawn by `ui_menu.c` in its footer line (AF_S, 12 px, y=224..236): `HOME close  SELECT tab  ALGO row`. | | | | | footer line above (the slot is the same Z6 rectangle) |

## 4. How modifier-held states relabel

A control whose role changes shows its new role **while** it is changed (VISION rule 1). The mechanism is one rule:
the slot of a control is repainted with the held-role text, the page beneath is untouched.

| Held | Relabelled slots | Text |
| --- | --- | --- |
| `HOME` (HELP, 0.25 s after press) | all of Z3, Z4 dimmed; help box over y=108..212; Z6 | the touched control's line (`HITS: re-roll where notes fall`) |
| `EDIT` hold 0.4 s (RESET) | every resettable cell gains a `RST` tag (T_ACCENT) in the corner of its slot; Z6 | `RESET: TOUCH WHAT TO RESET  EDIT=no` |
| `GLO` hold 0.4 s (SCALE) | Z2 becomes the 27-key picture; Z3 becomes SCALE band | `SCALE: PRESS A KEY = KEY  HOME=back` |
| `OCT-` / `OCT+` hold (peek) | the other knob row's cells are drawn filled for as long as the hold lasts | none |
| a white key held 0.4 s | NOTE_EDIT opens (page change, not a relabel) | |
| `SOLO` / `FILL` / `ROLL` black key held | the held Z5 cell inverted; FILL / ROLL cell shows the live amount | `ROLL 3x: let go to stop` |
| `PG+` while on the last page | ghost page cell in Z2 header row | `ADD PAGE 2 = COPY OF 1? PG+ = yes` |
| pending preset (`Pnn?`) | H3 shows `P05?`; SAVE (Z4 row 2 cell 2) label reads `SAVE` still | `P05: SAVE = save here` |

## 5. Text length limits (per zone, with fonts)

| Zone / slot | Font | Max characters | Max width (px) | Overflow rule |
| --- | --- | --- | --- | --- |
| H2 tempo | AF_S | 7 | 45 (after the 8 px dot and 3 px gap) | the string table of 6.2 only; no other text |
| H3 preset | AF_S | 4 | 28 | |
| H4 | AF_S | 12 | 76 | algorithm name is the 4-letter short form |
| H5 / H6 | AF_S | 9 / 3 | 42 / 24 | pages beyond 4 never occur |
| Z1 cell line | AF_S | 8 | 56 | ellipsis glyph `\x85` after the cut |
| Lane header | AF_S | 40 | 240 | |
| Knob label | AF_S | 6 | 51 | locked by `check_sequencer_ux.py` (6 characters) |
| Knob value | AF_S | 8 | 51 | |
| Band status line | AF_S | 34 | 204 | |
| Button tap / sub-label | AF_S | 6 | the cell width of 2.5 | a label with a space may wrap onto the two lines (9, F2) on pages that have no sub-labels |
| Black-key label | AF_S | 6 | 20 | `RESTRT` is 42 px and does not fit a 20 px cell: labels over 20 px are cut to what fits plus the ellipsis glyph and the full word is in HELP (9, F3) |
| Cue strip | AF_S | 34 | 240 | the widest locked strings, `TAP STEP=ON/OFF  HOLD=EDIT NOTE` is 203 px |

## 6. Tempo source, clock out and the clock dot (FB-001; owner rulings 2026-11-10, final form from P2-010)

> Rewritten by P2-010 to the owner rulings of 2026-11-10 (AUTO source, K1 inert in EXT, clock out automatic, a labelled
> wait for the host start, blinking colour-coded dots). The mechanisms, numbers and tests behind it are in
> [TIMING_SPEC.md](TIMING_SPEC.md); this section states only what the screen shows and reads. Source lines cited
> there. Status: proposed until the owner approves P2-010.

### 6.1 State fields the screen reads (data, one place)

| Field | Where it lives (ISR writes, main loop reads) | Values |
| --- | --- | --- |
| `bpm_own` | `song.g[G_BPM]`: **the user's tempo; the host clock never writes it** (today `midi_clock.c:65` overwrites it; P2-010 removes that) | 40..240 |
| `clk_src` | derived: `INT` (default) or `EXT` (a host clock is accepted, TIMING_SPEC 4.2 R2) | 0 / 1 |
| `clk_port` | which port the accepted clock came from, `USB` or `TRS` (shown on the SET page only) | 1 / 2 |
| `clk_known` | at least 7 pulses measured (the first BPM estimate, `midi_clock.c:36-45`) | 0 / 1 |
| `clk_ext_bpm` | the smoothed host tempo (TIMING_SPEC M11), shown as an integer, at most 4 updates a second, +-1 hysteresis | 40..240 |
| `clk_armed` | PLAY pressed while `EXT` and the host has not started (WAIT) | 0 / 1 |
| `clk_pulse_n` | free-running pulse count, the dot's host beat | 0..255 |

`CLK_TIMEOUT_MS = 500`, the value the firmware already uses to stop the transport when the cable is lost
(`seq.c:1410`) [V]: 24 pulses at 120 BPM, 8 at 40 BPM, so a slow clock is never mistaken for a stop. The new
derivation also runs while **stopped** (today it runs only while playing, `seq.c:1406-1416`), from the timestamp of the
last accepted pulse. The rules that move the source between `INT` and `EXT` (R1..R15) are TIMING_SPEC 4.2; the three
the user can observe: a clock heard while stopped makes the source `EXT`; 500 ms of silence returns it to `INT` at the
user's own tempo; a clock that arrives while the FM-1 is playing as master is ignored.

### 6.2 The four states (slot H2 and the clock dot; the same words on the SET page)

The dot (6.3) and the text always change together. Text widths are measured with the real `text_w` (AF_S).

| State | Condition | Dot | H2 text | Width | BPM the user reads |
| --- | --- | --- | --- | --- | --- |
| INT | `clk_src == INT`, not armed | amber, solid, flashes on the internal beat (also while stopped) | `INT 120` (T_TEXT) | 42 | `bpm_own` (K1 on SET) |
| EXT SYNC | `clk_src == EXT`, not `clk_known` (the first 7 pulses, 146 ms at 120 BPM) | cyan with a centre hole, steady at the off level | `EXT --` (T_ACCENT) | 34 | none yet |
| EXT LIVE | `clk_src == EXT`, `clk_known` | cyan with a centre hole, flashes on the host beat | `EXT 126` (T_ACCENT) | 45 | the host tempo, integer |
| WAIT | `clk_armed` | cyan ring (4 x 4 hole), blinks at 2 Hz | `WAIT` (T_ACCENT) | 28 | the host tempo if known, else none |

**There is no "last host tempo" state any more.** When the host clock stops the source returns to `INT` and the
display returns to `INT <bpm_own>`; the user's own tempo was never overwritten. The transport glyph H1 shows `.` when
stopped. When the clock stops **while playing** the transport stops (TIMING_SPEC R6, as `seq.c:1410-1411` does today)
and the cue strip says `CLOCK LOST: STOPPED` for 3 s.

### 6.3 The clock dot (owner ruling: blinking, colour-coded dots)

An 8 x 8 px dot at x=12, y=6 in H2, the text beginning at x=23 (8 + 3 px gap; `EXT 126` ends at 68, the slot's right
edge). It replaces the out-mark glyph of the first draft.

| Item | Specification |
| --- | --- |
| Meaning of the blink | the **beat**: on for one sixth of a beat (clamped to 50..120 ms), the bar's first beat twice as long; off level = the colour blended 65 % toward `T_BG`, so the dot is always visible |
| Meaning of the colour and shape (never colour alone) | **amber solid = the FM-1's own clock (internal, and it is sending clock out)**; **cyan with a 2 x 2 centre hole = a host clock drives the tempo**; **cyan ring (4 x 4 hole) blinking at 2 Hz = armed, waiting for the host to start** |
| Colours, RGB565, fixed (not theme roles) | dark background: amber `0xFD80`, cyan `0x071F`; light background (luminance of `T_BG` over 50 %, PAPER): amber `0xCB80`, cyan `0x03D4` |
| Source of the beat | INT: the internal beat counter (`beat_pos`, `beat_n`, `seq.c:350,1437-1441`), running also while stopped; EXT: `clk_pulse_n % 24 < 4`, the host beat |
| On every screen | TIMING_SPEC 5.3: H2 on all pages; undimmed in HELP; after the title in MENU; the reserved corner cell (4, 4, 8 x 8) on full-screen modes without a header; none in UBOOT |
| LEDs | TIMING_SPEC 5.4: PLAY green steady at mid level with a beat flash while playing; 2 Hz while armed; PLAY's own LED flashes on the host beat when stopped with a clock heard |

### 6.4 Clock out: automatic, no menu row (owner ruling 2)

| Condition | What goes out |
| --- | --- |
| `clk_src == INT` and playing | `0xF8` x24 per quarter at the internal tempo (one per 8 ticks of the 192-PPQN grid), `0xFA` on a start from step 0, `0xF2` then `0xFB` on a continue, `0xFC` on stop; each pulse stamped to its exact sample (TIMING_SPEC M7) |
| `clk_src == EXT`, WAIT, or stopped | nothing: no clock, no start, no stop. A host can therefore never be fed its own clock back |

There is no `CLOCK OUT` setting. The CLOCK row of MENU keeps its place and reduces to the single question of
TIMING_SPEC question 1 (AUTO only, or AUTO / INTERNAL ONLY).

### 6.5 SET page rendering

The SET band (Z3) rows in the four states (strings are table data; widths measured, AF_S). **SET K1 is locked**
(`knob_map.tsv`: "tempo (internal clock; ignored while an external MIDI clock runs, value shows EXT)"): in every EXT
state K1 does nothing and its cell is a read-only mirror. The user's own tempo is kept and returns when the host stops.

| State | K1 cell | Status line 1 (y=136) | Status line 2 (y=148) |
| --- | --- | --- | --- |
| INT | `BPM` / `120` | `TEMPO INT 120` (84)    `OUT MASTER` (72) | `host clock takes over` (109) |
| EXT SYNC | `BPM` / `EXT --` | `TEMPO EXT -- USB` (101)    `OUT FOLLOW` (73) | `waiting for the tempo` (110) |
| EXT LIVE | `BPM` / `EXT 126` (read-only, T_ACCENT; K1 ignored) | `TEMPO EXT 126 USB` (112)    `OUT FOLLOW` (73) | `K1 off: the host sets tempo` |
| WAIT | `BPM` / `EXT 126` or `EXT --` | `TEMPO EXT 126 USB`    `OUT FOLLOW` | `WAITING FOR HOST START` (144, T_ACCENT) |

`USB` is `TRS` when the clock comes from the 3.5 mm jack (`TEMPO EXT 126 TRS`, 111). Turning K1 in an EXT state changes
nothing and shows nothing new; the cue strip (8a) says why.

### 6.6 Cue strip rows added (SEQUENCER_UX 11.2)

| Priority | State | Text |
| --- | --- | --- |
| 8a | SET page, any EXT state | `EXT: K1 OFF, HOST SETS TEMPO` (28 characters) |
| 13a | `clk_armed` (any page except the ones above it) | `WAIT FOR HOST START  PLAY=cancel` (32 characters, 198 px) |
| 13b | the clock was lost, 3 s after R6 / R9 | `CLOCK LOST: STOPPED` while playing, `CLOCK LOST: PLAY TO START` when armed |

### 6.7 Main-screen rule

On every screen the user can read, without a menu, whether the tempo is the FM-1's own (amber dot, `INT`), a host's
(cyan dot with hole, `EXT` and its BPM), or the FM-1 is waiting for a host start (cyan ring, `WAIT`), and the dot
blinks on the beat of whichever clock is in charge.

## 7. Mock-ups (40 columns; section 2 holds the pixels)

PERF, INT (power-on):

```
+----------------------------------------+ y
|. oINT 120 P01 CH1 ACID M1 [1][2]. . L32| 0..20   H1 H2 (o = amber clock dot) H3 H4 H5 H6
|[1 ACID ]| 2 EUCL  | 3 SNAK  | 4 STAG   | 20..48  Z1 (selected inverted; line 2 MIDI n / MUTE)
| PG 1/2  FOLLOW  hold=EDIT        >P2   | 48..62
| #  .  #' #  .  #! .  #~ #  .  #  .  #x2 | 62..108 16 cells of 15 px
|OCT-|DENS 12|SLIDE 4|JUMP 5 |ACCNT 3|   | 108..134 tag + ROW 1
|h:pk|OFFS 0 |PEDAL 6|PUNCH40|DIR FWD|   | 134..160 tag + ROW 2 (dim)
| ALL |NOTES|ACCNT|OCTAVE|UNDO| SET      | 160..186 row 1 taps
|     |notes|acc  |oct   |h:RST|h:SCALE  | (hold mark in place of h:)
| HOME  SAVE  ARTIC HITS PLAY REC        | 186..212 row 2
| M1 M2 M3|M4 CHN|SOL FIL ROL|PG- PG+|RST| 212..224
| PLAY: start   K1-K4: morph             | 224..236
+----------------------------------------+ 236..240 blank
```

SET, EXT LIVE (Ableton over USB):

```
+----------------------------------------+
|> oEXT 126  P01 CH1 ACID M1 [1][2]. . L32|   H2 = cyan clock dot with a hole + accent text
|[1 ACID ]| 2 EUCL  | 3 SNAK  | 4 STAG   |
| lane as PERF (steps stay live)         |
| ...                                    |
|  - |BPM    |SWING 50|GATE 50 |DIV 1/16 |   tag inert
|  - |EXT 126|        |        |         |   K1 value = incoming BPM (read-only, K1 ignored)
| TEMPO EXT 126 USB      OUT FOLLOW       |   status line 1
| K1 off: the host sets tempo             |   status line 2
| ALL NOTES ACCNT SHAPE UNDO BACK         |   GLO = BACK
| EXT: K1 OFF, HOST SETS TEMPO           |
+----------------------------------------+
```

SET, WAIT (PLAY pressed while a host clock is heard and the host has not started):

```
|. oWAIT     P01 CH1 ACID M1 [1][2]. . L32|   o = cyan ring blinking 2 Hz, transport not yet running
| TEMPO EXT 126 USB      OUT FOLLOW       |
| WAITING FOR HOST START                  |   (T_ACCENT)
| WAIT FOR HOST START  PLAY=cancel        |   cue strip 13a
```

## 8. Flex points (table edits, no code)

| Table or constant | What a change does |
| --- | --- |
| Zone table (2) | any zone's x,y,w,h, as long as the rows still tile 240x240 (the area sum above is a check) |
| Slot tables 2.1, 2.4 | slot widths and order; the sum of the widths in a row must stay 240 |
| `LEG_X` (2.6) | the 11 legend x positions |
| Button cell widths per page (2.5) | recomputed from the measured widths, never from the mock-up |
| String tables 6.2, 6.5, 6.6 | every tempo / clock-out text, the status lines |
| Clock dot (6.3, TIMING_SPEC 5.1) | size, colours, flash length, the WAIT blink rate |
| `CLK_TIMEOUT_MS` (500) | the host-silence timeout after which the source returns to INT |
| BPM readout update rate (4/s) and hysteresis (+-1) | readout stability |
| The hold mark (2.5) | `h:` text instead of the glyph (9 px instead of 6: row 2 no longer fits; needs a label edit, see F2) |
| Row padding (2.5) | 1 px on PERF row 2, 3 px on row 1 |
| Theme colour per element | any `T_*` token |
| Cue strip rows 8a, 13a | text and priority |

Code (not a table edit): the fonts, the 240x124 canvas, the 3-blit structure, the order of redraw.

## 9. Findings from measuring the locked mock-ups against real glyph widths

These are the places where the printed mock-ups of SEQUENCER_UX 2 are not drawable as printed. None changes a
decision; each is resolved here and the owner may overrule.

| Id | Finding | Resolution here |
| --- | --- | --- |
| F1 | the printed AF_M (13 px) tap labels do not fit: AF_M `ACCENT` is 58 px, the cell is at most 51 | every label is AF_S (12 px line, 46 px for `ACCENT`) |
| F2 | PERF row 2 with all eight hold / dice sub-labels needs about 326 px in one 240 px row: cells HOME `h:HLP/MN` 53, SAVE `h:UNDO L` 52, ARP `GT+SLD` 43, SEQ `ROTATE` 42, PLAY 28, REC 22, OCT- `h:peek` and OCT+ `h:peek` (each about 31) plus 3 px padding on every cell | OCT- / OCT+ labels move to the band tag column BT, and the printed `h:` (9 px) becomes a 5x7 px hold mark plus a 1 px gap (6 px, saving 3 px per hold sub-label). Row 2 is then 50+48+43+42+28+22 = 233 px of label plus 1 px padding per cell (6) = 239 <= 240 (table in 2.5). On the other pages row 1 and 2 sub-labels are absent, so `RST AL`, `RST NT` wrap onto two lines |
| F3 | black-key label `RESTRT` is 42 px; the legend pitch is 21 px (PERFORMANCE_DESIGN 6 expected P2-003 to check this) | labels over 20 px are cut to what fits plus the ellipsis glyph in the legend and shown in full by HELP; the locked labels in `key_map.tsv` are unchanged. Owner may prefer a taller legend (it would cost the cue strip or band 12 px) |
| F4 | header text of the locked mock-up is 202 px with `120`; adding the source word and the clock dot needs the slot widths of 2.1 | measured: all six slots fit in 238 px |
| F5 | lane glyphs `x2` fit a 15 px cell (AF_S 12 px); the velocity bar uses the other 32 px | as 2.3 |

The widths come from a probe built on the real `text_w` and `ui_fonts.h`. **The implementing task must add a host test
that runs the same measurement over every label of `control_map.tsv` and every string of 6.2, 6.5 and 6.6 against the
slot widths of this spec** (otherwise a longer label silently breaks a row).

## 10. Boot and first use (owner decision 32b)

| Step | What the screen shows | Where it is specified |
| --- | --- | --- |
| Power-on | splash 400 ms (`draw_splash`), then the PERF main screen straight away: channel 1 selected, stopped, no menu, no mode or groovebox choice, no page reached by a menu | ADR-0003 12 |
| Defaults on screen | CH1 `ACID` MIDI 1 unmuted; CH2..CH4 `EUCL SNAK STAG` MIDI 2..4 muted; scale `MINPEN`, key `C`, octave 0, BPM `INT 120` (the header, clock out MASTER), length `L16` | ADR-0003 12 `SQ_DEFAULTS` |
| Audible with no setup | PLAY starts CH1's acid line on the preview synth with default morphs; nothing needs setting first | ADR-0003 12 step 7 |
| First-run cue strip (once) | `D#4 K2: MIDI CHANNEL`, then `PRESS PLAY`, `TURN K1 TO MORPH IT`, `PRESS FX: NEW PATTERN  EDIT=undo`, `TAP A WHITE KEY: STEP ON/OFF`, `HOLD A WHITE KEY: EDIT THAT NOTE`, `PG+ TWICE: ADD PAGE 2 (A COPY)`; HOME skips | SEQUENCER_UX 11.3 |
| The tempo | power-on is INT, so the header reads `INT 120` with the amber clock dot, blinking on the beat; a host that sends clock is picked up by itself (AUTO): the dot turns cyan with a hole and the header shows `EXT --` then `EXT <bpm>`; when the host stops the FM-1 returns to its own tempo | 6 |

> **Owner rulings 2026-11-10 (applied by P2-010; section 6 and TIMING_SPEC carry them in full):**
> 1. Tempo source is AUTO. Default is the internal clock (the user's own tempo, kept separately and never overwritten). If a MIDI clock is heard, that clock becomes the tempo source; when it stops, return to internal at the user's own tempo. K1 stays inert while external clock drives.
> 2. Clock out is automatic: sent whenever the FM-1 is the tempo master (internal). No CLOCK OUT menu row.
> 3. PLAY while the source is external and the host has not started = a labelled wait for the clock to start (no silent fall back): header `WAIT`, ring dot, cue `WAIT FOR HOST START  PLAY=cancel`.
> 4. Clock visibility is BLINKING DOTS: amber solid = internal, cyan with a centre hole = external, cyan ring = waiting; they blink on the beat. Exact colours, size, placement and LED behaviour: section 6.3 and TIMING_SPEC 5. The hold-mark glyph (question 4) stays as proposed.

## 11. Questions for the owner (new; the locked questions 65-73 are untouched)

1. **Header tempo word.** *Answered by the owner's rulings (2026-11-10): the out-mark is replaced by the blinking clock
   dot (6.3).*
2. **Clock-out switch.** *Answered (ruling 2): clock out is automatic, no menu row.*
3. **EXT WAIT and the transport.** *Answered (ruling 3): a labelled wait (`WAIT`, `WAIT FOR HOST START  PLAY=cancel`),
   no silent fall back. Open questions about the escape from WAIT are in TIMING_SPEC section 11.*
4. **Hold mark** (F2). Accept the glyph in place of `h:` on the PERF button sub-labels?
5. **Tempo while following.** *Answered (ruling 1 and the P2-003 approval): the user's own tempo is kept in `song.g[G_BPM]`
   and never overwritten (the host tempo has its own field, 6.1); K1 stays inert in EXT.* The remaining clock questions
   (the CLOCK menu row after AUTO, clock lost while playing, the PLAY LED) are TIMING_SPEC section 11.
