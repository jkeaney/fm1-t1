# SCREEN_SPEC: the 240x240 screen, zone by zone (P2-003)

Status: **proposed**, for the owner's human-review gate. Nothing here is approved until the owner writes it in the
P2-003 log. It makes VISION rule 1 concrete: at every moment the screen shows what the device is **doing** and the
current role of **every** knob and button. It adds pixels to the locked UX ([SEQUENCER_UX.md](SEQUENCER_UX.md), P1-017
approved); it changes none of its decisions. Where a locked mock-up cannot be drawn as printed, section 9 lists the
finding and the resolution chosen here, and the owner can overrule it.

Inputs: SEQUENCER_UX sections 2, 4, 11; `page_states.tsv`, `button_modes.tsv`, `knob_map.tsv`, `key_map.tsv`,
`control_map.tsv`; CONTROL_MAP; PERFORMANCE_DESIGN 6; FULL_CONTROL; `firmware/src/gfx.c` (canvas, themes, fonts);
FB-001 (tempo source and clock out, owner decision 2026-11-10: v1 sends clock as master).

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
| H2 | 12,4,56,12 | **tempo source and clock out** (section 6) | `120 EXT?` 50, `EXT 126` 45, `INT 120` 42 + out-mark 9 |
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

Cell widths are **data per page**, computed as the widest label of the cell (tap label, dice sub-label or hold
sub-label) plus 3 px and then widened to fill 240. PERF widths (the tightest page; measured): row 1 = 24, 41, 42, 47,
36, 50; row 2 = 47, 46, 46, 45, 30, 26 (+ separators drawn as a 1 px T_LINE column inside the cell). The hold
sub-label carries a 5x7 px **hold mark** (a filled down-triangle) in place of the printed `h:`, which saves 9 px
each and makes row 2 fit (section 9, finding F2). A button whose layer is absent for the algorithm is dim with a
dash. A button whose tap label differs from the printed word (RST, CANCEL, BACK) is drawn in T_ACCENT.

### 2.6 Z5 LEGEND and Z6 CUE

- Z5: 11 cells of 20 px at pitch 21 in the physical 3-2-3-2-1 grouping with a 2 px gap between groups:
  `LEG_X` = 0, 21, 42, 65, 86, 109, 130, 151, 174, 195, 218 (the last cell ends at 238); each cell shows the page's label for that black key (max 6
  characters); inverted while the key is down; muted-channel cells dim.
- Z6: one line, 34 characters, priority table SEQUENCER_UX 11.2 (plus the two rows added in section 6.5).

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
| `EN_SELECT` | 1 | Z1 the selected cell (inverted) | label `CH n` |
| `EN_ALGO` | 1 | Z1 the selected cell, line 1 algorithm name; Z0 H4 | label `ALGO` |
| `EN_PRESET` | 1 | Z0 H3 | label `Pnn` |
| `KEY_W01..KEY_W16` (16 white) | 16 | Z2 cells 1..16 (the lane IS the legend); on SCALE the Z2 area shows the 27-key picture and each key shows its pitch | `STEP` / `NOTE` / `KEY` / `CLR` |
| `KEY_B01..KEY_B11` (11 black) | 11 | Z5 cells 1..11 | |
| `MASTER` | (not a panel control) | none | |

Per page (the nine contexts of the tables), the slots that change meaning:

| Page | Z1 | Z2 | Z3 band | Z4 | Z5 | Z6 |
| --- | --- | --- | --- | --- | --- | --- |
| PERF | 4 cells | lane + header | tag `OCT-`/`OCT+` `h:peek`; ROW 1 live (K1..K4) + ROW 2 dim (S1..S4) | dice row + holds | M1..M4 CHN SOL FIL ROL PG- PG+ RST | rows 13/14/15 of 11.2 |
| NOTE_EDIT | banner replaces cells: `NOTE EDIT STEP 21` | edited cell boxed | tag `OCT-`/`OCT+` (step octave); K1..K4 PITCH GATE VELO RATCH, value + `AUTO`/`*` | dice-this-step labels (ALL NOTE ACCENT PROB UNDO SET / HOME SAVE SLIDE ON/OFF PLAY REC) | as PERF | `STEP n: KNOBS EDIT  HOME=done` |
| CHAN | 4 cells | page map cells (4) | tag `-PAGE`/`+PAGE`; K1..K4 LENGTH MIDI SOURCE FOLLOW; status lines: page-source summary, `D#4 = BACK` | as PERF | D#4 reads `BACK` | row 6 |
| SET | 4 cells | lane as PERF | tag `-`/`-` (inert); K1..K4 BPM SWING GATE DIV; **status lines: tempo source and clock out (6.4)** | `GLO` reads `BACK`, hold `SCALE` | as PERF | row 8 (or 8a, 6.5) |
| SCALE | 4 cells | 27-key picture, scale tones lit, root boxed | tag `OCT-`/`OCT+` = `OCTAVE -`/`+`; K1..K4 SCALE KEY OCTAVE `-` | as SET | all 27 keys read `KEY` | row 7 |
| REC | banner `REC CH1 writing to page 2` (T_REC) | write cursor `^` | tag `OCT-`/`OCT+` = `OCT -`/`+`; K1..K4 CURSOR ACC SLIDE OCT; status `KEYS = NOTES (all 27)` | dice labels live (performing while recording is allowed) | `KEYS = NOTES` | row 4 |
| RESET | the page beneath, banner `RESET` | every resettable cell tagged `RST` | K1..K4 = `RST` over the value being reset | `RST AL`, `RST NT`... (two-line wrap, 9) | white keys `CLR`; D#4 = re-copy | row 2 |
| HELP | the page beneath dimmed | dimmed | dimmed | dimmed | dimmed | 2-line help box over Z3..Z4 (y=108..212, x=8..232) naming the touched control | row 1 |
| MENU | whole screen: Felucca menu | | | | | own title |

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
| H2 tempo | AF_S | 8 | 56 | the string table of 6.2 only; no other text |
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

## 6. Tempo source and clock out (FB-001, owner decision 2026-11-10)

### 6.1 State fields the screen reads (data, one place)

| Field | Where it lives (ISR writes, main loop reads) | Values |
| --- | --- | --- |
| `clk_src` | `song.g[G_CLOCK]` (`INT` 0, `USB` 1, `TRS` 2), set in MENU > CLOCK, set-and-forget | 0, 1, 2 |
| `clk_live` | derived each block: the last accepted clock pulse is at most `CLK_TIMEOUT_MS` ago | 0 / 1 |
| `clk_bpm` | measured: `60000000 / (pulse interval in us * 24)`, rounded, valid after 6 pulses (`midi_clock.tempo_valid`) | 20..300, 0 = not measured |
| `clk_out` | derived: `clk_src == INT` (master: sends clock) | 0 / 1 |

`CLK_TIMEOUT_MS = 500`, the value the firmware already uses to stop the transport when the cable is lost
(`seq.c`: `fm1_ms - last > 500u`) [V]. 500 ms is 12 clock pulses at 120 BPM and 3.6 beats at 30 BPM, so a slow
tempo is never mistaken for a stop. It is a constant in the flex table (section 8).

### 6.2 The four tempo states (slot H2, and the same words on the SET page)

| State | Condition | H2 text | Colour | BPM value the user reads |
| --- | --- | --- | --- | --- |
| INT | `clk_src == INT` | `INT 120` + out-mark | T_TEXT | the internal BPM (K1 on SET) |
| EXT LIVE | `clk_src != INT`, `clk_live`, `clk_bpm > 0` | `EXT 126` as an inverted chip (T_ACCENT fill, T_INK) | accent | the incoming BPM, integer; updates at most 4 times a second, +-1 hysteresis so it does not flicker |
| EXT SYNC | `clk_src != INT`, `clk_live`, `clk_bpm == 0` (first quarter note) | `EXT --` chip | accent | none yet |
| EXT WAIT | `clk_src != INT`, not `clk_live` (never started, or silent for `CLK_TIMEOUT_MS`) | `120 EXT?` (the internal BPM, then a dim `EXT?`) | T_TEXT + T_MID | the internal BPM; it **reverts** to this readout when the clock stops, which is the timeout rule |

The out-mark is the 9x9 px glyph "arrow leaving a bar" in T_THEME, drawn at H2 x+46. It is shown only when `clk_out`
and absent in every EXT state. The internal BPM (`song.g[G_BPM]`) is never overwritten by the external tempo; EXT
only changes what `beat_samples()` returns [V, fx.c], so leaving EXT restores the user's own tempo.

When the external clock stops while playing the existing rule stops the transport (500 ms), the header glyph H1
shows `.` and H2 returns to `EXT WAIT`. Start/Stop/Continue from the host move the transport as today.

### 6.3 Clock out: master and follow

Owner decision (2026-11-10): v1 sends 24 pulses per quarter note plus Start / Stop / Continue over USB when the FM-1 is
the master, and does not echo the clock when it is following.

| Mode | Condition | What goes out |
| --- | --- | --- |
| MASTER | `clk_src == INT` | `0xF8` x24 per quarter at the internal tempo (one per 8 ticks of the 192-PPQN grid), `0xFA` on PLAY from the start, `0xFC` on stop, `0xFB` on a continue |
| FOLLOW | `clk_src != INT` (including EXT WAIT) | nothing: no clock, no start, no stop. The state follows the **selected source**, not the live pulse, so a silent host can never be fed its own clock back |

The master / follow state is automatic: no new control, no new menu row. The source is the one existing set-and-
forget setting. (Open question for the owner, section 10: an optional MENU switch `CLOCK OUT ON/OFF` for a gear
that must not receive clock; adding it changes the tab row counts asserted in `tests/ui_test.c`, so it is not
assumed here.)

### 6.4 SET page rendering

The SET band (Z3) rows in the four states (strings are table data; widths measured, AF_S):

| State | K1 cell | Status line 1 (y=136) | Status line 2 (y=148) |
| --- | --- | --- | --- |
| INT | `BPM` / `120` | `TEMPO INT 120` (84)    `OUT MASTER` (72) | `MENU>CLK sets source` (125) |
| EXT LIVE | `BPM` / `EXT 126` (read-only mirror, T_ACCENT) | `TEMPO EXT 126 USB` (112)    `OUT FOLLOW` (73) | `K1 sets INT BPM` |
| EXT SYNC | `BPM` / `EXT --` | `TEMPO EXT -- USB` (101)    `OUT FOLLOW` | `waiting for the 1st beat` |
| EXT WAIT | `BPM` / `120` dim | `TEMPO 120 EXT? USB` (118)    `OUT FOLLOW` | `NO CLOCK FROM USB` (T_ACCENT) |

Turning K1 while an EXT state shows: the **internal** BPM changes (it is what returns when the source goes back to
INT) and the K1 cell shows `INT 121` for 2 s, then the EXT value again; the cue strip reads `EXT: K1 SETS INT BPM  HOME=back`
(31 characters). SWING, GATE and DIV are unaffected by the source.

### 6.5 Cue strip rows added (SEQUENCER_UX 11.2)

| Priority | State | Text |
| --- | --- | --- |
| 8a | SET page, any EXT state | `EXT: K1 SETS INT BPM  HOME=back` |
| 13a | stopped, PERF, `clk_src != INT` and not live | `EXT CLOCK: WAITING FOR USB` (`TRS` when the source is TRS) |

### 6.6 Main-screen rule

On every page except MENU the header slot H2 shows the tempo state of 6.2. The user can therefore always read on the
main screen whether the tempo is the FM-1's own, a host's (with its BPM), or a host that went quiet.

## 7. Mock-ups (40 columns; section 2 holds the pixels)

PERF, INT (power-on):

```
+----------------------------------------+ y
|. INT 120> P01 CH1 ACID M1 [1][2]. . L32| 0..20   H1 H2 (out-mark >) H3 H4 H5 H6
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
|> [EXT 126]  P01 CH1 ACID M1 [1][2]. . L32|   H2 = accent chip, no out-mark
|[1 ACID ]| 2 EUCL  | 3 SNAK  | 4 STAG   |
| lane as PERF (steps stay live)         |
| ...                                    |
|  - |BPM    |SWING 50|GATE 50 |DIV 1/16 |   tag inert
|  - |EXT 126|        |        |         |   K1 value = incoming BPM (read-only mirror)
| TEMPO EXT 126 USB      OUT FOLLOW       |   status line 1
| K1 sets INT BPM                         |   status line 2
| ALL NOTES ACCNT SHAPE UNDO BACK         |   GLO = BACK
| EXT: K1 SETS INT BPM  HOME=back        |
+----------------------------------------+
```

SET, EXT WAIT (host stopped sending clock):

```
|. 120 EXT?  P01 CH1 ACID M1 [1][2]. . L32|   internal BPM back, EXT? dim, transport stopped
| TEMPO 120 EXT? USB     OUT FOLLOW       |
| NO CLOCK FROM USB                       |   (T_ACCENT)
```

## 8. Flex points (table edits, no code)

| Table or constant | What a change does |
| --- | --- |
| Zone table (2) | any zone's x,y,w,h, as long as the rows still tile 240x240 (the area sum above is a check) |
| Slot tables 2.1, 2.4 | slot widths and order; the sum of the widths in a row must stay 240 |
| `LEG_X` (2.6) | the 11 legend x positions |
| Button cell widths per page (2.5) | recomputed from the measured widths, never from the mock-up |
| String tables 6.2, 6.4, 6.5 | every tempo / clock-out text, the out-mark, the status lines |
| `CLK_TIMEOUT_MS` (500) | the EXT to EXT WAIT timeout |
| BPM readout update rate (4/s) and hysteresis (+-1) | readout stability |
| The hold mark (2.5) | `h:` text instead of the glyph (needs the row widths reduced by an edit of the label text, see F2) |
| Theme colour per element | any `T_*` token |
| Cue strip rows 8a, 13a | text and priority |

Code (not a table edit): the fonts, the 240x124 canvas, the 3-blit structure, the order of redraw.

## 9. Findings from measuring the locked mock-ups against real glyph widths

These are the places where the printed mock-ups of SEQUENCER_UX 2 are not drawable as printed. None changes a
decision; each is resolved here and the owner may overrule.

| Id | Finding | Resolution here |
| --- | --- | --- |
| F1 | the printed AF_M (13 px) tap labels do not fit: AF_M `ACCENT` is 58 px, the cell is at most 51 | every label is AF_S (12 px line, 46 px for `ACCENT`) |
| F2 | PERF row 2 with all eight hold / dice sub-labels needs about 326 px in one 240 px row (`h:HLP/MN` 53, `h:UNDO L` 52, `GT+SLD` 43, `ROTATE` 42, and the OCT cells) | OCT- / OCT+ labels move to the band tag column BT; the printed `h:` prefix becomes a 5 px hold mark (saves 9 px per hold sub-label): row 2 = 47+46+46+45+30+26 = 240. On the other pages row 1 and 2 sub-labels are absent, so `RST AL`, `RST NT` wrap onto two lines |
| F3 | black-key label `RESTRT` is 42 px; the legend pitch is 21 px (PERFORMANCE_DESIGN 6 expected P2-003 to check this) | labels over 20 px are cut to what fits plus the ellipsis glyph in the legend and shown in full by HELP; the locked labels in `key_map.tsv` are unchanged. Owner may prefer a taller legend (it would cost the cue strip or band 12 px) |
| F4 | header text of the locked mock-up is 202 px with `120`; adding the source word and out-mark needs the slot widths of 2.1 | measured: all six slots fit in 238 px |
| F5 | lane glyphs `x2` fit a 15 px cell (AF_S 12 px); the velocity bar uses the other 32 px | as 2.3 |

The widths come from a probe built on the real `text_w` and `ui_fonts.h`. **The implementing task must add a host test
that runs the same measurement over every label of `control_map.tsv` and every string of 6.2, 6.4 and 6.5 against the
slot widths of this spec** (otherwise a longer label silently breaks a row).

## 10. Boot and first use (owner decision 32b)

| Step | What the screen shows | Where it is specified |
| --- | --- | --- |
| Power-on | splash 400 ms (`draw_splash`), then the PERF main screen straight away: channel 1 selected, stopped, no menu, no mode or groovebox choice, no page reached by a menu | ADR-0003 12 |
| Defaults on screen | CH1 `ACID` MIDI 1 unmuted; CH2..CH4 `EUCL SNAK STAG` MIDI 2..4 muted; scale `MINPEN`, key `C`, octave 0, BPM `INT 120` (the header, clock out MASTER), length `L16` | ADR-0003 12 `SQ_DEFAULTS` |
| Audible with no setup | PLAY starts CH1's acid line on the preview synth with default morphs; nothing needs setting first | ADR-0003 12 step 7 |
| First-run cue strip (once) | `D#4 K2: MIDI CHANNEL`, then `PRESS PLAY`, `TURN K1 TO MORPH IT`, `PRESS FX: NEW PATTERN  EDIT=undo`, `TAP A WHITE KEY: STEP ON/OFF`, `HOLD A WHITE KEY: EDIT THAT NOTE`, `PG+ TWICE: ADD PAGE 2 (A COPY)`; HOME skips | SEQUENCER_UX 11.3 |
| The tempo | power-on is INT, so the header reads `INT 120` with the out-mark; a host that sends clock is picked up only after the one-time setting MENU > CLOCK (`USB` or `TRS`); then the header shows `EXT --` and `EXT <bpm>` | 6 |

## 11. Questions for the owner (new; the locked questions 65-73 are untouched)

1. **Header tempo word.** On the main screen INT is shown as `INT 120` with an out-mark. The out-mark can instead be
   the words `OUT` if the owner prefers it spelled (24 px; needs the page map narrowed). Recommended: out-mark.
2. **Clock-out switch.** v1 as decided sends clock whenever the source is INT. Add an optional MENU row
   `CLOCK OUT ON/OFF`? Recommended: no for v1 (no new setting); revisit if the TB-3 or another unit misbehaves with a
   clock it did not ask for.
3. **EXT WAIT and the transport.** With `CLK USB` selected and no host clock the sequencer cannot run (existing
   behaviour: PLAY waits). The screen says so (`EXT CLOCK: WAITING FOR USB`). Alternative: fall back to internal after the
   timeout. Not chosen, because a sequencer that silently switches tempo master mid-set is worse than a labelled wait.
4. **Hold mark** (F2). Accept the glyph in place of `h:` on the PERF button sub-labels?
