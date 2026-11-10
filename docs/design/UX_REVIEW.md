# UX review: hardware controls x algorithms x workflow, with an ease score

> P2-009: the chord row of the CONTROL tab is removed (owner decision 39). CONTROL tab is 4 rows; 28 menu rows in total (SYSTEM_MAP section 7).

> P1-014 revision (printed-word pass, scheme B2; for owner review): supersedes the layer-order note below. The buttons follow their printed words: `FX` ALL, `SCL` NOTES, `SEQ` HITS, `ENV` ACCNT, `ARP` ARTIC, `LFO` SHAPE (all on press), `EDIT` = UNDO / RESET, `GLO` = SET, `REC` = RECORD (new context), MUTATE removed; hold a step key + another white key = its pitch. Sections 1.2 and 1.3 are regenerated from control_map.tsv (294 rows = 49 controls x 6 contexts); sections 2 and 3.3 follow; the scores of 3.2 are the history of P1-012 / P1-013, the live scores are PERFORMANCE_DESIGN.md 7.2 (honest L; B2 576 of 648, no failure).

> P1-014 (performance-first, owner eighth round; for owner review): the control layout is re-organised by ONE principle (top button row = the six dice in layer order, bottom row = safety and system, white keys = the steps, black keys = performance moves) and the rubric gains a fourth sub-score **L (learnable without a manual)**, section 3.3. **Superseded here:** sections 1.2 and 1.3 are regenerated from the new control_map.tsv (five contexts `PERF STEP_HELD SET RESET MENU`); the dice positions in 1.1 / 5.2 and the workflows W4 / W7 follow the new map. The score tables in 3.2 and the iteration table in 4 stay as the P1-013 history (the "proposed" column there is the approved P1-013 scheme); the current scores are PERFORMANCE_DESIGN.md 7.2. Source of truth for the map: [CONTROL_MAP.md](CONTROL_MAP.md) and [control_map.tsv](control_map.tsv).

> P1-013 (owner decision "Shift"): the knobs now have **two rows** (up to 8 morphs per algorithm). OCT- / OCT+ become the row buttons (tap latches ROW 1 / ROW 2; hold and turn is momentary); the keyboard octave moves to SCL held + OCT-/OCT+. Changed here: 1.1 and 1.3 rows, 1.4 knob band, W5/W6/W14, scores F30..F35 and totals, R1 note, 5.1 knob table, 5.4. Design: [FULL_CONTROL.md](FULL_CONTROL.md). Every score row below that is not named above is unchanged.

Task P1-012 (owner directive 2026-11-10). Status: **for owner review** (gate `human-review`; approval also re-approves
P1-010). Design only: no firmware is changed by this document.

Inputs: VISION.md, OWNER_QUESTIONS.md (decisions 2026-11-09 and 2026-11-10, S1-S7), ADR-0004 and its flexibility
addendum, CONTROL_INVENTORY.md (current behaviour, with file:line), ALGORITHM_SET.md (P1-010 draft), the section 6 of
sting.md, t1.md, snake.md, ml185.md, `firmware/src/panel.c` and the panel photo `docs/controls.jpg`.

The machine-readable form of section 1 is [control_map.tsv](control_map.tsv) (343 rows: 49 controls x 7 contexts). The
table in 1.3 and the TSV come from one generator, so they agree row for row.

## 0. The idea in one paragraph

The FM-1 boots into the sequencer. One main screen (PERF) shows the four channels, the selected channel's algorithm,
its 16 steps, the four knob roles and a 2 x 6 picture of the button block with each button's current role. The
three printed encoders do what their printing says: **SELECT selects the channel, ALGORITHM picks the algorithm,
PRESETS picks the preset.** KNOB 1-4 always morph the selected channel (density, form, pitch width, spice). Five
buttons dice the five shared layers, FX dices all of them, EDIT undoes. REC enters steps on the 16 white keys. The
black keys mute and select channels. Three modifier buttons whose printing names a setting open it while held: SCL
(global scale and key), EDIT (channel MIDI channels), GLO (gate, division, swing, tempo). HOME held is the menu, and
the menu holds only set-and-forget settings.

## 1. Hardware map

### 1.1 Physical controls and their printing

From `panel.c` (enums `B_*`, `EN_*`, `B_NAME`, `E_NAME`) and `docs/controls.jpg`. Physical layout, left to right:
encoder block (top row MASTER, SELECT; bottom row PRESETS, ALGORITHM) with OCT- OCT+ under it; the screen; the knob
row KNOB 1..KNOB 4; under the knobs the 2 x 6 button block; the 27-key keyboard along the bottom.

| Control | Printed | Kind | Proposed role (PERF) | Printing fit | Page or owns the sequence |
| --- | --- | --- | --- | --- | --- |
| `MASTER` | MASTER | analogue pot (not an `EN_*`) | preview-synth and USB-audio volume | exact | device |
| `EN_SELECT` | SELECT | relative encoder | **select channel** CH1..CH4 | exact ("select") | page (what the knobs act on) |
| `EN_ALGO` | ALGORITHM | relative encoder | **algorithm of the selected channel** | exact (it was the track selector before) | owns |
| `EN_PRESET` | PRESETS | relative encoder | **preset slot** P01..P16 (whole-device state) | exact | owns |
| `EN_K1`..`EN_K4` | KNOB 1..4 | relative encoders | the four morphs of the selected channel, K1 left to K4 right, drawn in the same order on screen | exact (position = column) | owns |
| `B_FX` | FX | button, row 1 col 1 | DICE ALL | new meaning; screen label `ALL` | owns |
| `B_SCL` | SCL | row 1 col 2 | press: dice NOTES (P1-014 revision: no hold role; the scale and key are on the SET page) | fair (scale -> the pitch layer) | owns |
| `B_ENV` | ENV | row 1 col 3 | dice ACCNT | envelope = dynamics; screen label `ACCNT` | owns |
| `B_LFO` | LFO | row 1 col 4 | dice SHAPE | LFO = shape over time; screen label `SHAPE` | owns |
| `B_EDIT` | EDIT | row 1 col 5 | tap UNDO/REDO; hold = the RESET layer (P1-014 revision; the MIDI channel moved to SET) | weak (edit -> undo) | owns (tap) / page (hold) |
| `B_GLO` | GLO | row 1 col 6 | the SET page: GATE, DIV, SWING, BPM; SCALE, OCTAVE, MIDI, LENGTH (P1-014 revision) | exact (global) | page |
| `B_HOME` | HOME | row 2 col 1 | tap = back to PERF from anywhere; hold = HELP while held (touch a control), MENU on release after 0.7 s with nothing touched | exact (stock: tap home, hold menu) | page |
| `B_SAVE` | SAVE | row 2 col 2 | tap save to the shown slot; hold 0.7 s undo the last load | exact (hold-undo as today) | owns |
| `B_ARP` | ARP | row 2 col 3 | dice ARTIC | arpeggio = articulation (gate, glide, repeats); screen label `ARTIC` | owns |
| `B_SEQ` | SEQ | row 2 col 4 | dice HITS | sequencer steps = which steps sound; screen label `HITS` | owns |
| `B_PLAY` | PLAY/STOP | row 2 col 5 | start / stop all channels | exact | owns |
| `B_REC` | REC | row 2 col 6 | tap = RECORD (latched, LED): the 27 keys play pitches and write them (P1-014 revision; MUTATE removed) | exact ("arm to record") | page |
| `B_OCTDN`, `B_OCTUP` | OCT-, OCT+ | buttons under the encoders | knob ROW 1 / ROW 2 (tap latches, hold and turn is momentary); with a step held, the step band row; in SET, the SET row | override (printed: octave; owner decision 48) | owns |
| 16 white keys F3..G5 | none | keys, place 0..15 | the 16 steps of the viewed page (hold + another white key = that step's pitch); in REC they play pitches F3..G5 | n/a | owns |
| 11 black keys F#3..F#5 | `OP1 OP2 OP3 OP4 OP5 OP6 PIT GLD MONO POLY` (last: blank) | keys, place 0..10 | F#3 G#3 A#3 C#4 mute CH1-4 (OP1..OP4); D#4 PAGE; F#4 SOLO; G#4 FILL; A#4 ROLL; C#5 / D#5 SCENE A / B; F#5 RESTART; in REC they play pitches | weak (OP1..OP4 = parts) | owns |
| Hidden maintenance | - | OCT- + OCT+ | both 5 s = UPDATE MODE; both at power-on = CALIBRATION (unchanged) | n/a | device |

Why this layout:
- **The dice cluster is spatial.** Row 1 holds the pitch and dynamics layers (SCL NOTES, ENV ACCNT, LFO SHAPE), row 2
  the time layers (ARP ARTIC, SEQ HITS) next to PLAY. FX (ALL) and EDIT (UNDO) flank the row-1 dice: "new everything"
  on the left, "take it back" on the right.
- **P1-014 revision:** the buttons that name a setting now open it by a tap (GLO = SET) or have none (SCL = NOTES dice, EDIT = UNDO with RESET on hold); the old holds (SCL layer, EDIT = MIDI channels) moved to the SET page so every dice button acts on press.
- **Synth-only printing (FX ENV LFO ARP SEQ) becomes dice.** Rule 2: the synth is only a preview, so those labels
  have no job; the screen's 2 x 6 button picture shows the real role under each printed position.
- Felucca's groovebox pages (ENV, LFO, FX, ARP, SONG, sound editing) are not part of this map. They are not reachable
  at all: decision 32b removes the groovebox from the v1 UI (there is no mode switch and no menu route to it; the
  device boots straight into the sequencer).

### 1.2 Contexts (P1-014 revision: six)

| Context | Entered by | Left by | Screen |
| --- | --- | --- | --- |
| `PERF` | power-on default | n/a | main screen (1.4): header, channels, lane with `PG n/M`, two knob rows, button picture, black-key legend, cue strip |
| `STEP_HELD` | hold a white key (a tap toggles the pin) | let go of the key | step band: row 1 `NOTE ACC SLIDE RATCH`, row 2 `TIE MUTE PROB OCT`; another white key = that step's pitch |
| `SET` | tap GLO (latched) or hold GLO | tap GLO, tap HOME, or release | SET page: row 1 `GATE DIV SWING BPM`, row 2 `SCALE OCTAVE MIDI LENGTH` |
| `RESET` | hold EDIT (UNDO) past HOLD, or press a control while it is held | let go of EDIT | cells tagged `RST`, buttons relabelled |
| `REC` | tap REC (latched, LED) | tap REC or HOME | banner `REC`, one-row band `CURSOR ACC SLIDE OCT`, the 27 keys play pitches and write them |
| `MENU` | hold HOME 0.7 s, release | HOME tap | Felucca menu (1.5) |

Precedence: MENU > SET > RESET > REC > STEP_HELD > PERF. HOME tap leaves every state for PERF; HOME held shows `touch = HELP, let go = MENU` and acts on release. EDIT and GLO use the layer time (MENU > HOLD, default 0.4 s); SAVE and the scene keys use 0.7 s.

### 1.3 The control map (every control x every context, regenerated from control_map.tsv, P1-014)

Each cell is the on-screen label and the function ids scored in PERFORMANCE_DESIGN.md 7.2. Full action text per row: [control_map.tsv](control_map.tsv).

| Control | PERF | STEP_HELD | SET | RESET | REC | MENU |
| --- | --- | --- | --- | --- | --- | --- |
| B_FX | `ALL`  | `ALL`  | `ALL`  | `RST ALL` F45 9 | `ALL`  | `close` F29 11 |
| B_SCL | `NOTES +sub`  | `NOTES +sub`  | `NOTES +sub`  | `RST NOT` F45 9 | `NOTES +sub`  | `close` F29 11 |
| B_ENV | `ACCNT +sub`  | `ACCNT +sub`  | `ACCNT +sub`  | `RST ACC` F45 9 | `ACCNT +sub`  | `close` F29 11 |
| B_LFO | `SHAPE +sub`  | `SHAPE +sub`  | `SHAPE +sub`  | `RST SHP` F45 9 | `SHAPE +sub`  | `close` F29 11 |
| B_EDIT | `UNDO` F15 11 F43 9 | `UNDO` F15 11 | `UNDO` F15 11 | `RESET` F43 9 | `UNDO` F15 11 | `close` F29 11 |
| B_GLO | `SET` F16 11 | `-` F05 10 | `BACK` F47 12 | `-` F43 9 | `-`  | `close` F29 11 |
| B_HOME | `HOME` F47 12 F29 11 F46 9 | `HOME` F47 12 F29 11 F46 9 | `HOME` F47 12 F29 11 F46 9 | `HOME` F47 12 F46 9 | `HOME` F47 12 F29 11 F46 9 | `BACK` F29 11 F47 12 |
| B_SAVE | `SAVE Pnn` F22 12 F24 11 | `SAVE Pnn` F22 12 F24 11 | `SAVE Pnn` F22 12 F24 11 | `SAVE Pnn` F22 12 F24 11 | `SAVE Pnn` F22 12 F24 11 | `close` F29 11 |
| B_ARP | `ARTIC +sub`  | `ARTIC +sub`  | `ARTIC +sub`  | `RST ART` F45 9 | `ARTIC +sub`  | `close` F29 11 |
| B_SEQ | `HITS +sub`  | `HITS +sub`  | `HITS +sub`  | `RST HIT` F45 9 | `HITS +sub`  | `close` F29 11 |
| B_PLAY | `PLAY / STOP` F25 12 | `PLAY / STOP` F25 12 | `PLAY / STOP` F25 12 | `PLAY / STOP` F25 12 | `PLAY / STOP` F25 12 | `PLAY / STOP` F25 12 |
| B_REC | `REC` F53 12 F26 10 | `REC` F53 12 | `REC` F53 12 | `CLR PIN` F06 10 | `REC` F53 12 | `close` F29 11 |
| B_OCTDN | `ROW 1` F30 11 | `STEP 1` F51 9 | `SET 1` F16 11 | `RST R1` F44 9 | `-`  | `-` F29 11 |
| B_OCTUP | `ROW 2` F30 11 | `STEP 2` F51 9 | `SET 2` F16 11 | `RST R2` F44 9 | `-`  | `-` F29 11 |
| EN_SELECT | `CH n` F01 12 | `CH n` F01 12 | `CH n` F01 12 | `CH n` F01 12 | `CH n` F01 12 | `CH n` F01 12 |
| EN_ALGO | `ALGO name` F03 12 | `ALGO name` F03 12 | `ALGO name` F03 12 | `ALGO name` F03 12 | `ALGO name` F03 12 | `tab` F29 11 |
| EN_PRESET | `Pnn` F23 10 | `Pnn` F23 10 | `Pnn` F23 10 | `Pnn` F23 10 | `-`  | `row` F29 11 |
| MASTER | `-`  | `-`  | `-`  | `-`  | `-`  | `-`  |
| EN_K1 | `algo label` F08 12 F31 10 | `NOTE / TIE` F05 10 F51 9 | `GATE / SCALE` F16 11 F19 9 | `RST` F43 9 | `CURSOR` F53 12 | `value` F29 11 |
| EN_K2 | `algo label` F08 12 F32 10 | `ACC / MUTE` F42 9 F51 9 | `DIV / OCTAVE` F16 11 F27 9 | `RST` F43 9 | `ACC` F53 12 | `value` F29 11 |
| EN_K3 | `algo label` F08 12 F33 10 | `SLIDE / PROB` F42 9 F51 9 | `SWING / MIDI` F16 11 F02 9 | `RST` F43 9 | `SLIDE` F53 12 | `value` F29 11 |
| EN_K4 | `algo label` F08 12 F34 10 | `RATCH / OCT` F42 9 F05 10 | `BPM / LENGTH` F17 11 F50 9 | `RST` F43 9 | `OCT` F53 12 | `value` F29 11 |
| KEY_W01_F3 | `step 1` F04 12 F26 10 | `pitch / step 1` F05 10 F42 9 | `KEY / step` F18 10 | `CLR 1` F06 10 | `note` F53 12 F26 10 | `-`  |
| KEY_W02_G3 | `step 2` F04 12 F26 10 | `pitch` F05 10 F42 9 | `KEY / step` F18 10 | `CLR 2` F06 10 | `note` F53 12 F26 10 | `-`  |
| KEY_W03_A3 | `step 3` F04 12 F26 10 | `pitch` F05 10 F42 9 | `KEY / step` F18 10 | `CLR 3` F06 10 | `note` F53 12 F26 10 | `-`  |
| KEY_W04_B3 | `step 4` F04 12 F26 10 | `pitch` F05 10 F42 9 | `KEY / step` F18 10 | `CLR 4` F06 10 | `note` F53 12 F26 10 | `-`  |
| KEY_W05_C4 | `step 5` F04 12 F26 10 | `pitch` F05 10 F42 9 | `KEY / step` F18 10 | `CLR 5` F06 10 | `note` F53 12 F26 10 | `-`  |
| KEY_W06_D4 | `step 6` F04 12 F26 10 | `pitch` F05 10 F42 9 | `KEY / step` F18 10 | `CLR 6` F06 10 | `note` F53 12 F26 10 | `-`  |
| KEY_W07_E4 | `step 7` F04 12 F26 10 | `pitch` F05 10 F42 9 | `KEY / step` F18 10 | `CLR 7` F06 10 | `note` F53 12 F26 10 | `-`  |
| KEY_W08_F4 | `step 8` F04 12 F26 10 | `pitch` F05 10 F42 9 | `KEY / step` F18 10 | `CLR 8` F06 10 | `note` F53 12 F26 10 | `-`  |
| KEY_W09_G4 | `step 9` F04 12 F26 10 | `pitch` F05 10 F42 9 | `KEY / step` F18 10 | `CLR 9` F06 10 | `note` F53 12 F26 10 | `-`  |
| KEY_W10_A4 | `step 10` F04 12 F26 10 | `pitch` F05 10 F42 9 | `KEY / step` F18 10 | `CLR 10` F06 10 | `note` F53 12 F26 10 | `-`  |
| KEY_W11_B4 | `step 11` F04 12 F26 10 | `pitch` F05 10 F42 9 | `KEY / step` F18 10 | `CLR 11` F06 10 | `note` F53 12 F26 10 | `-`  |
| KEY_W12_C5 | `step 12` F04 12 F26 10 | `pitch` F05 10 F42 9 | `KEY / step` F18 10 | `CLR 12` F06 10 | `note` F53 12 F26 10 | `-`  |
| KEY_W13_D5 | `step 13` F04 12 F26 10 | `pitch` F05 10 F42 9 | `KEY / step` F18 10 | `CLR 13` F06 10 | `note` F53 12 F26 10 | `-`  |
| KEY_W14_E5 | `step 14` F04 12 F26 10 | `pitch` F05 10 F42 9 | `KEY / step` F18 10 | `CLR 14` F06 10 | `note` F53 12 F26 10 | `-`  |
| KEY_W15_F5 | `step 15` F04 12 F26 10 | `pitch` F05 10 F42 9 | `KEY / step` F18 10 | `CLR 15` F06 10 | `note` F53 12 F26 10 | `-`  |
| KEY_W16_G5 | `step 16` F04 12 F26 10 | `pitch` F05 10 F42 9 | `KEY / step` F18 10 | `CLR 16` F06 10 | `note` F53 12 F26 10 | `-`  |
| KEY_B01_F#3 | `MUTE1` F20 11 | `MUTE1` F20 11 | `KEY / MUTE1` F18 10 F20 11 | `MUTE1` F20 11 | `note` F53 12 F26 10 | `-`  |
| KEY_B02_G#3 | `MUTE2` F20 11 | `MUTE2` F20 11 | `KEY / MUTE2` F18 10 F20 11 | `MUTE2` F20 11 | `note` F53 12 F26 10 | `-`  |
| KEY_B03_A#3 | `MUTE3` F20 11 | `MUTE3` F20 11 | `KEY / MUTE3` F18 10 F20 11 | `MUTE3` F20 11 | `note` F53 12 F26 10 | `-`  |
| KEY_B04_C#4 | `MUTE4` F20 11 | `MUTE4` F20 11 | `KEY / MUTE4` F18 10 F20 11 | `MUTE4` F20 11 | `note` F53 12 F26 10 | `-`  |
| KEY_B05_D#4 | `PAGE` F49 11 F50 9 | `-`  | `KEY / PAGE` F18 10 F49 11 | `PAGE` F49 11 F50 9 | `note` F53 12 F26 10 | `-`  |
| KEY_B06_F#4 | `SOLO` F39 11 | `SOLO` F39 11 | `KEY / SOLO` F18 10 F39 11 | `SOLO` F39 11 | `note` F53 12 F26 10 | `-`  |
| KEY_B07_G#4 | `FILL` F37 11 | `FILL` F37 11 | `KEY / FILL` F18 10 F37 11 | `FILL` F37 11 | `note` F53 12 F26 10 | `-`  |
| KEY_B08_A#4 | `ROLL` F38 11 | `ROLL` F38 11 | `KEY / ROLL` F18 10 F38 11 | `ROLL` F38 11 | `note` F53 12 F26 10 | `-`  |
| KEY_B09_C#5 | `SCN A` F40 11 F41 10 | `SCN A` F40 11 F41 10 | `KEY / SCN A` F18 10 F40 11 | `SCN A` F40 11 F41 10 | `note` F53 12 F26 10 | `-`  |
| KEY_B10_D#5 | `SCN B` F40 11 F41 10 | `SCN B` F40 11 F41 10 | `KEY / SCN B` F18 10 F40 11 | `SCN B` F40 11 F41 10 | `note` F53 12 F26 10 | `-`  |
| KEY_B11_F#5 | `RESTRT` F28 11 | `RESTRT` F28 11 | `KEY / RESTRT` F18 10 F28 11 | `RESTRT` F28 11 | `note` F53 12 F26 10 | `-`  |

Conflict check (done on the TSV): in every context each control has exactly one action; the only deliberate duplicates are none (SELECT is the only channel select; TEMPO is SET K4 only). White keys are inert in MENU.

### 1.4 What the main screen shows (so every role is labelled where the eye is)

Exact pixel layout belongs to the screen spec (P2-003); these are the fixed contents and their order, top to bottom,
mirroring the panel (knob row above the button block):

| Band (240 px wide) | Content |
| --- | --- |
| Header, 24 px | transport glyph and BPM (`EXT` when slaved), global `KEY SCALE` (for example `C MINPEN`), preset `P01` (`P03>` while a load is queued, `P05 EMPTY` on an empty slot), `STEP` banner in STEP |
| Channels, 32 px | four cells `1 ACID 2` (channel, algorithm, MIDI channel); selected cell inverted; muted cell dimmed with `M` |
| Sequence, 68 px (was 80) | the selected channel's algorithm name in large type and its 16-step lane: hits, accents, pins (outlined), playhead; in STEP the white-key number under each step; STAGE draws stage bars over its TOTAL pulses |
| Knobs, 52 px (P1-013; the sequence band is 68 px) | two rows of four cells, each 26 px high: ROW 1 K1..K4 above ROW 2 S1..S4, a 12 px gutter with the row number `1` / `2`; each cell has a label (max 6 characters) and a value; the live row is filled, the other dimmed; a cell turned in the last 1 s is inverted. Exact zone and states: FULL_CONTROL.md section 2 |
| Buttons, 64 px | 2 x 6 grid in the panel's order (FX SCL ENV LFO EDIT GLO / HOME SAVE ARP SEQ PLAY REC): current label, second line `+sub` for dice buttons; absent layer drawn dimmed with `-` |

The KEYS legend (HOME tap on PERF) replaces the sequence band with the 27-key map: black keys `MUTE1..4`, `CH1..4`,
`ALL ON`, `RESTRT`, `TAP`; white keys `NOTES` (PERF) or `1..16` (STEP).

### 1.5 The menu (hold HOME): set-and-forget only

Kept: DISPLAY tab (COLOR, STYLE, LARGE, ANIM, LEDS, SCREEN OFF, SCOPE, HOME view), CONTROL tab (HOLD, ACCEL, SCALE
LEDS, STEP PREVIEW), AUDIO tab (SPEAKER EQ, USB LEVEL, CLICK, CLICK LEVEL, COUNT-IN, TUNE), MIDI tab (MIDI
IN routing, AUTO SELECT, SCREEN WAKE, LEARN CLEAR), SYSTEM tab (USB SERIAL,
RESTORE LAST, HELP, CALIBRATION, INFO, ABOUT).

Removed from the menu because they are musical functions now on the panel: `MI_TCH1..4` (CHAN layer), `MI_SELECT`
(SELECT is fixed to channel select), `MI_BPMLOCK` (tempo is no longer on SELECT), `MI_T1SEQ` (the sequencer is always on,
decision 32b), `MI_LATCH` (the FX layer does not exist). Tab row counts change,
so the dev task that edits `menu_items.c` updates `tests/ui_test.c`.

## 2. Workflows (exact controls, from power-on)

Defaults at power-on (first boot or after INIT): CH1 ACID MIDI 1 unmuted; CH2 EUCLID MIDI 2, CH3 SNAKE MIDI 3, CH4
STAGE MIDI 4, all three muted; global KEY C, SCALE MINPEN, OCTAVE 0, GATE 50 %, DIV 1/16, SWING 50 %, BPM 120;
preset P01; keyboard OCT 0. The preview synth plays the selected channel; MASTER is its volume.

| W | Workflow | Steps (controls pressed) | Function ids |
| --- | --- | --- | --- |
| W1 | Select channel 1 | 1. Turn SELECT until the header cell `1` is inverted, (black keys no longer select, P1-014). | F01, F21 |
| W2 | Set its MIDI channel (TB-3 = 2) | (P1-014 revision) 1. Tap GLO (the SET page opens), tap OCT+ (row 2). 2. Turn K3 until `MIDI` reads `2`. 3. Tap HOME. The next note goes out on MIDI channel 2. | F02 |
| W3 | Pick an algorithm | 1. Turn ALGORITHM: `ACID > EUCLID > SNAKE > STAGE`. The new algorithm plays from the next step with its own stored morphs and seeds (S7). | F03 |
| W4 | Enter steps on the 16 white keys | (P1-014 revision) 1. Turn K1 fully down (`KEYS ONLY`) if only typed steps should play. 2. Tap white key n: step n is pinned and sounds (no STEP mode). 3. Pitches: tap REC and play the line on the keys (they write at the cursor / nearest step; tap REC or HOME to leave), or hold the step key and press another white key (= that scale degree). 4. Per-step feel: hold the key, K2 `ACC`, K3 `SLIDE`, K4 `RATCH`; OCT+ then `TIE MUTE PROB OCT`. 5. Tap a pinned key to clear; hold EDIT + REC clears all. | F04, F05, F06, F42 |
| W5 | Dial a sequence in with the knobs | 1. Press PLAY. 2. Turn K1 (density), K2 (form), K3 (pitch width), K4 (spice). For the second row, tap OCT+ (the band fills ROW 2) and turn the same four knobs: S1 start (`OFFSET`), S2 articulation, S3 dynamics, S4 read order; tap OCT- to go back (or hold OCT+ while turning for a momentary ROW 2). Each detent is heard from the next step and its cell shows the value. | F07, F25 |
| W6 | Morph live | Same controls as W5 while playing. Every morph is monotonic and reversible: turning back restores the exact pattern. | F08 |
| W7 | Dice each layer | (P1-014 revision) FX = all, SCL = NOTES, SEQ = HITS, ENV = ACCNT, ARP = ARTIC, LFO = SHAPE, all on press. EDIT tap = undo / redo. | F09-F15 |
| W8 | Tune the algorithm | The algorithm's own parameters are the four morphs (W5). The global feel it reads: (P1-014) 1. Tap GLO (SET page, row 1). 2. Turn K1 GATE, K2 DIV, K3 SWING, K4 BPM. 3. Tap HOME (or GLO) to go back. | F16 |
| W9 | Set the global scale and key (S5) | (P1-014 revision) 1. Tap GLO, tap OCT+ (SET row 2). 2. Press any key for the KEY; turn K1 (SCALE), K2 (OCTAVE). 3. Tap HOME. Every channel re-snaps from the next step; the header shows `KEY SCALE` on every screen. | F18, F19, F27 |  <!-- superseded by HARMONY.md 5 (P2-012): hold GLO = HARMONY; the KEY is K2, OCTAVE is OCT-/OCT+ on SET -->
| W10 | Mute / switch channels | (P1-014) Mute: tap F#3 / G#3 / A#3 / C#4 for CH1-4 (toggle). Switch: turn SELECT (black keys no longer select; there is no `ALL ON`). Restart all from step 1: tap F#5. | F20, F21, F28 |
| W11 | Save / load | Save: tap SAVE (`SAVED P01`). Save to another slot: turn PRESETS to an empty slot (`P05 EMPTY`), tap SAVE. Load: turn PRESETS to a filled slot; it loads at the next loop start (immediately when stopped), the header shows `P03>` until then. Undo a load: hold SAVE 0.7 s (again: redo). | F22, F23, F24 |
| W12 | Tempo | (P1-014 revision) Tap GLO, turn K4 (BPM); the TAP key is given up for PAGE. | F17 |
| W13 | Transport | PLAY starts / stops every channel. | F25 |
| W14 | Audition notes | (P1-014 revision) Tap REC: every key (F3..G5, chromatic) sounds on the selected channel and writes a pin; EDIT tap takes the last note back. Outside REC a white key press pins and sounds step n; a held step key + another white key sounds that scale degree; K1 `NOTE` sounds each detent. | F26 |
| W15 | Set-and-forget settings | Hold HOME 0.7 s; ALGORITHM = tab, PRESETS = row, K1-K4 = value; HOME tap closes. | F29 |

The reference workflow (VISION) is therefore: SELECT to `1` -> tap GLO, OCT+, K3 to `2`, HOME -> ALGORITHM to `ACID` -> PLAY ->
K1-K4 and the dice buttons. That is 5 gestures to sound on the TB-3, all on the main screen except the SET page (tap GLO, OCT+).

## 3. Ease score

### 3.1 Rubric

Each function is scored 0-3 on three axes, total /9.

| Score | (a) VISUAL CUE | (b) INPUT EFFORT | (c) OUTPUT FEEDBACK |
| --- | --- | --- | --- |
| 0 | the role is not shown | menu dive or hidden combo | none |
| 1 | shown on another page | modifier + control, or more than 2 presses | delayed more than 1 step, or not shown |
| 2 | shown but small or ambiguous | two actions | audible at the next step (screen does not show the value) |
| 3 | labelled on the main screen where the eye already is | one direct control | audible within one step AND the screen shows the new value |

Rules used while scoring:
- A held layer's own screen counts as "where the eye is" for the controls it labels (the layer appears on the press).
- A hold of one control (0.7 s) is a two-phase gesture: input 2.
- For a function with nothing to hear (save, select), output 3 = the screen confirms within one frame.
- CURRENT scores assume T1 mode is already ON (MENU > SYSTEM > T1 SEQ); reaching it is scored separately as F00.

**Pass rule.** Performance functions (P: morph, dice, mute, select algorithm, and the other live gestures marked P)
need >= 8 with no sub-score below 2. Setup functions (S) need >= 6 with no 0.

### 3.2 Scores (current firmware vs proposed)

Current = CONTROL_INVENTORY.md, T1 mode. V/I/O = visual / input / output.

| Id | Function | Class | Current control (CONTROL_INVENTORY) | Current V/I/O = total | Proposed control | Proposed V/I/O = total | Delta | Proposed pass |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| F00 | Reach the sequencer | S | hold HOME, SYSTEM tab, T1 SEQ row, K to ON | 1/0/1 = 2 | power-on default | 3/3/3 = 9 | +7 | yes |
| F01 | Select channel | P | white keys 1-6 in T1 mode, unlabelled (ui_t1.c:85-99) | 1/3/2 = 6 | SELECT encoder | 3/3/3 = 9 | +3 | yes |
| F02 | Set the channel's MIDI channel | S | SEQ x3 to MOD/CC, hold OCT+, turn K1 (torso_t1_ui.c MOD/CC shift) | 1/1/2 = 4 | hold EDIT + K1..K4 | 3/1/3 = 7 | +3 | yes |
| F03 | Pick an algorithm | P | no algorithm concept; only T1 SEQ on/off in the menu | 0/0/1 = 1 | ALGORITHM encoder | 3/3/3 = 9 | +8 | yes |
| F04 | Enter a step (pin) | S | unreachable: `edit_step` never set, Step Mod has no UI | 0/0/0 = 0 | REC tap, then white key | 3/2/3 = 8 | +8 | yes |
| F05 | Set a pin's pitch | S | unreachable (P-LOCK) | 0/0/0 = 0 | hold step key + K3 / OCT | 3/1/3 = 7 | +7 | yes |
| F06 | Clear pins | S | unreachable | 0/0/0 = 0 | hold REC 0.7 s (EDIT tap undoes) | 3/2/3 = 8 | +8 | yes |
| F07 | Dial a sequence in with knobs | S | four T-1 pages via SEQ / OCT-, OCT+ shift for the second row | 3/1/2 = 6 | K1-K4 on PERF | 3/3/3 = 9 | +3 | yes |
| F08 | Morph live | P | as F07; density on RHYTHM, pitch on HARMONY, page changes needed | 3/2/2 = 7 | K1-K4 on PERF | 3/3/3 = 9 | +2 | yes |
| F09 | Dice NOTES | P | no dice | 0/0/0 = 0 | SCL tap | 3/3/3 = 9 | +9 | yes |
| F10 | Dice HITS | P | no dice | 0/0/0 = 0 | SEQ press | 3/3/3 = 9 | +9 | yes |
| F11 | Dice ACCNT | P | no dice | 0/0/0 = 0 | ENV press | 3/3/3 = 9 | +9 | yes |
| F12 | Dice ARTIC | P | no dice | 0/0/0 = 0 | ARP press | 3/3/3 = 9 | +9 | yes |
| F13 | Dice SHAPE | P | no dice | 0/0/0 = 0 | LFO press | 3/3/3 = 9 | +9 | yes |
| F14 | Dice all | P | no dice | 0/0/0 = 0 | FX press | 3/3/3 = 9 | +9 | yes |
| F15 | Undo / redo dice | P | TEMP (hold SAVE or LFO, OCT+ commits) undoes knob edits only | 1/1/2 = 4 | EDIT tap | 3/3/3 = 9 | +5 | yes |
| F16 | Tune: GATE, DIV, SWING | S | per track: RHYTHM K3 DIV, shift K3 GATE; swing not reachable | 2/1/2 = 5 | GLO tap + K1..K3 | 3/2/3 = 8 | +3 | yes |
| F17 | Tempo | S | SELECT turn (BPM) | 2/3/3 = 8 | GLO tap + K4, or F#5 TAP | 3/2/3 = 8 | 0 | yes |
| F18 | Set global key | S | per track HARMONY K2 ROOT, not global | 1/1/2 = 4 | hold SCL + any key or K1 | 3/1/3 = 7 | +3 | yes |
| F19 | Set global scale | S | per track HARMONY shift K2 SCALE | 1/1/2 = 4 | hold SCL + K2 | 3/1/3 = 7 | +3 | yes |
| F20 | Mute channel | P | black keys 1-6, unlabelled | 1/3/2 = 6 | black F#3..C#4; C#5 all on | 2/3/3 = 8 | +2 | yes |
| F21 | Switch channel | P | white keys 1-6, unlabelled | 1/3/2 = 6 | SELECT or black D#4..A#4 | 3/3/3 = 9 | +3 | yes |
| F22 | Save | S | T-1 flash save not wired | 0/0/0 = 0 | SAVE tap | 3/3/3 = 9 | +9 | yes |
| F23 | Load | S | not wired | 0/0/0 = 0 | PRESETS turn (queued to loop start) | 3/3/1 = 7 | +7 | yes |
| F24 | Undo a load | S | not wired | 0/0/0 = 0 | hold SAVE 0.7 s | 3/2/3 = 8 | +8 | yes |
| F25 | Start / stop | P | PLAY | 3/3/3 = 9 | PLAY | 3/3/3 = 9 | 0 | yes |
| F26 | Audition notes on keys | S | keys silent in T1 mode (song.grid 2) | 0/0/0 = 0 | white keys in PERF | 2/3/3 = 8 | +8 | yes |
| F27 | Global output octave | S | per track HARMONY K1 PITCH | 1/1/2 = 4 | hold SCL + K3 | 3/1/3 = 7 | +3 | yes |
| F28 | Restart all from step 1 | P | GLO + PLAY in Felucca only; none in T1 mode | 0/0/0 = 0 | black D#5 | 2/3/3 = 8 | +8 | yes |
| F29 | Set-and-forget settings | S | hold HOME 0.7 s | 3/2/3 = 8 | hold HOME 0.7 s | 3/2/3 = 8 | 0 | yes |
| F30 | Switch the knob row | P | unlabelled OCT+ SHIFT, SEQ pages (T-1) | 1/1/2 = 4 | tap OCT+ / OCT-, or hold and turn | 3/3/3 = 9 | +5 | yes |
| F31 | Turn S1 START (`OFFSET`) | P | cut (not reachable) | 0/0/0 = 0 | tap OCT+, turn K1 | 3/2/3 = 8 | +8 | yes |
| F32 | Turn S2 ARTIC (`SLIDE` `RATCH` `GLEN` `RDIV`) | P | cut | 0/0/0 = 0 | tap OCT+, turn K2 | 3/2/3 = 8 | +8 | yes |
| F33 | Turn S3 DYNAMICS (`PUNCH` `DYN`) | P | cut | 0/0/0 = 0 | tap OCT+, turn K3 | 3/2/3 = 8 | +8 | yes |
| F34 | Turn S4 READ (`DIR` `CURVE`) | P | cut | 0/0/0 = 0 | tap OCT+, turn K4 | 3/2/3 = 8 | +8 | yes |
| F35 | Keyboard octave (moved by P1-013) | S | keys silent in T1 mode | 0/0/0 = 0 | hold SCL, then OCT- / OCT+ | 3/1/3 = 7 | +7 | yes |

Totals (P1-013, 36 functions): current 88 / 324 (the F30-F35 current column adds 4), proposed 297 / 324. Current: 32 of 36 functions fail. Proposed: 36 of 36 pass. (Before P1-013: 84 / 270 and 249 / 270.)
Performance functions proposed: F30 = 9; F31-F34 = 8 (input 2: tap OCT+, then turn); F01 F03 F08-F15 F21 F25 = 9 each; F20 and F28 = 8 (V2: the black-key labels are on the
KEYS legend, not on the main screen; reviewer correction, they had been scored V3); none has a sub-score below 2.

### 3.3 The L sub-score and the extended pass rule (P1-014)

**L = learnable without a manual** (0 needs the manual; 1 guessable only from other controls; 2 role visible on screen but must be discovered, including a printed word that only loosely fits or contradicts the role; 3 role obvious from the printed word or position without the screen, and the cue strip names it). Totals are out of 12. Pass: performance functions >= 11 with no sub-score below 2; setup functions >= 9 with no 0. Every function F00-F55, approved (P1-013) versus B (the first P1-014 version, L corrected) versus B2 (final), with V/I/O/L and the pass result, is the table in [PERFORMANCE_DESIGN.md](PERFORMANCE_DESIGN.md) section 7.2: under the extended rule the approved scheme fails 25 of 56 functions; B2 passes all 54 live functions (576 of 648) with one disclosed classification (F31-F34, the ROW 2 morphs, are setup dial-ins at 10: owner question 60).

## 4. Redesign iterations

Every CURRENT failure in 3.2 is redesigned by the proposed column. These are the alternatives that were scored on the
way and rejected; each line gives the score that settled it.

| # | Function | Tried | Score | Kept | Score | Why |
| --- | --- | --- | --- | --- | --- | --- |
| R1 | F02 MIDI channel | OCT+ held as SHIFT + SELECT on PERF | 1/1/3 = 5 (fail, shift role unlabelled; P1-013 reuses OCT as a labelled, latched row switch for the knobs, F30 = 9) | hold EDIT, K1..K4 per channel | 7 | The layer screen labels all four channels; the knob under the label is the channel's column. |
| R2 | F02 MIDI channel | EDIT tap opens a latched CHAN page | 3/2/3 = 8 | hold EDIT | 7 | EDIT's tap is UNDO (F15 needs 9, a performance function); setup at 7 passes. |
| R3 | F14 / F15 | FX tap = UNDO, FX hold = DICE ALL | ALL 3/2/3 = 8 | FX press = ALL, EDIT tap = UNDO | 9 / 9 | Dice-all is the fastest way to a new sequence (rule 5): it gets a direct press. |
| R4 | F12 + slide | ARP press = GATE+SLIDE; ARP hold = SLIDE only | slide-only 2/2/3 = 7 (fail as P) | one ARP press dices GATE and SLIDE (ACID) | 9 | See S1 (5.3). |
| R5 | F19 scale | PRESETS encoder = scale | 3/3/3 = 9 | hold SCL + K2 | 7 | PRESETS printing would lie and save/load would lose its encoder (F23 drops to 0). Scale is setup; 7 passes. |
| R6 | F23 load | load on PRESETS detent immediately | 3/3/3 = 9 | queue to the loop start | 3/3/1 = 7 | An immediate load cuts the bar mid-phrase and one stray detent replaces the live state. Setup rule allows the 1. |
| R7 | F04 entry | piano-roll entry (keys = notes, cursor advances) | 2/2/3 = 7 | white key = step (TR style) | 8 | VISION names the 16 white keys as steps; the lane under the keys is the visual cue. |
| R8 | F17 tempo | SELECT = tempo (today) | 8 | GLO K4 or TAP | 8 | Same score; SELECT's printing is "select", which F01 needs at 9. |
| R9 | F01 / F21 | white keys 1-4 select, black keys mute (T1 today) | 1/3/2 = 6 | SELECT + black D#4..A#4 | 9 | White keys are reserved for steps (rule 3); the KEYS legend shows each channel's key. |
| R10 | F13 SHAPE label | label `SHAPE` alone | 2/3/3 = 8 | `SHAPE` + algorithm sub-label | 9 | See S2 (5.3). |
| R11 | F08 ACID K2 | label `TYPE` | 2/3/3 = 8 | label `PEDAL` | 9 | See S3 (5.3). |

**Unavoidable failures: none.** Accepted low sub-scores (setup only, allowed by the pass rule): F23 output 1 (queued
load, R6); input 1 on F02, F05, F18, F19, F27 (modifier + control: a held layer is the price of keeping settings off
the main screen without a menu).

## 5. Algorithm alignment

### 5.1 Knobs (K1 DENSITY, K2 FORM, K3 PITCH, K4 SPICE)

Each cell: on-screen label, range, default. Score is the F08 score of that cell (V/I/O). K1 now starts at 0 on every
algorithm: 0 = `KEYS ONLY`, no generated hits, only pins play (rank-order density with `n = 0` plays nothing;
`t1_euclidean_bjorklund(steps, 0)` already returns 0). This is a range edit only.

| Algorithm | K1 | K2 | K3 | K4 | Score per knob |
| --- | --- | --- | --- | --- | --- |
| `ACID` | `DENS` 0..16, 12 | `PEDAL` 0..12, 6 (was `TYPE`) | `JUMP` 0..20, 5 | `ACCNT` 0..8, 3 | 3/3/3 = 9 each |
| `EUCLID` | `HITS` 0..16, 5 | `LEN` 2..16, 16 | `SPAN` 0..24, 12 | `ACCNT` 0..8, 2 | 9 each |
| `SNAKE` | `DENS` 0..16, 10 | `LEN` 1..16, 7 | `RANGE` 0..48, **12** | `PATH` 1..14, 7 | 9 each |
| `STAGE` | `DENS` 0..8, 6 | `TOTAL` 8..64, 16 (step 4; 1 when turned slowly) | `RANGE` 0..36, 12 | `RATCH` 0..8, 2 | 9 each |

**ROW 2 (P1-013).** The same four physical knobs after tapping OCT+ (or while OCT+ is held). Label, range, default:

| Algorithm | S1 START | S2 ARTIC | S3 DYNAMICS | S4 READ | Score per knob |
| --- | --- | --- | --- | --- | --- |
| `ACID` | `OFFSET` 0..15, 0 | `SLIDE` 0..8, 4 | `PUNCH` 0..47, 40 | `DIR` 0..2, 0 | 3/2/3 = 8 each |
| `EUCLID` | `OFFSET` 0..15, 0 | `RATCH` 0..8, 4 | `PUNCH` 0..47, 40 | `CURVE` 0..6, 0 | 8 each |
| `SNAKE` | `OFFSET` 0..15, 0 | `GLEN` 1..16, 16 | `DYN` 0..10, 10 | `DIR` 0..2, 0 | 8 each |
| `STAGE` | `OFFSET` 0..63, 0 | `RDIV` 1..4, 4 | `DYN` 0..10, 10 | `DIR` 0..2, 0 | 8 each |

Formulas, restored source parameters and the parameters that stay cut: FULL_CONTROL.md section 3. Step rule (1 per
detent, 4 per detent when turned fast on ranges over 32 values): FULL_CONTROL.md 5.1.

### 5.2 Dice buttons

> P1-014 revision: the button positions in this section are the P1-012 / P1-013 ones and are **current again** (scheme B2): `FX` ALL, `SCL` NOTES, `SEQ` HITS, `ENV` ACCNT, `ARP` ARTIC, `LFO` SHAPE (all press-only), `EDIT` tap = UNDO. (The first P1-014 version had moved them into layer order; withdrawn.) Layer meanings, seeds and sub-labels below are unchanged.

Main label (always the shared name) / sub-label (the algorithm's own layer, max 6 characters) / seeds diced.
`-` = absent: the button is drawn dimmed as `SHAPE -`, a press changes nothing and shows `NO SHAPE IN EUCLID` for
0.5 s (ALGORITHM_SET 4.4).

| Button (printed) | Layer | ACID | EUCLID | SNAKE | STAGE | Score |
| --- | --- | --- | --- | --- | --- | --- |
| SCL tap | `NOTES` | `NOTES` s_notes | `NOTES` s_notes | `NOTES` s_notes | `NOTES` s_notes | 9 |
| SEQ | `HITS` | `HITS` s_hits | `ROTATE` s_rot | `GATES` s_gates | `RESTS` s_rests | 9 |
| ENV | `ACCNT` | `ACCNT` s_acc | `ACCNT` s_acc | `VELO` s_vel | `VELO` s_vel | 9 |
| ARP | `ARTIC` | `GT+SLD` s_gate + s_slide | `RATCH` s_rat | `-` | `RT+HLD` s_mode | 9 (present) |
| LFO | `SHAPE` | `OCTAVE` s_oct | `-` | `DRIFT` s_poly | `LONG` s_lens | 9 (present) |
| FX | `ALL` | 6 seeds | 4 | 4 | 5 | 9 |
| EDIT tap | `UNDO` | swaps every seed of the last dice action with its previous seed | same | same | same | 9 |

### 5.3 Owner items S1-S4, S6 resolved by the score

| Item | Resolution | Score evidence |
| --- | --- | --- |
| **S1** slide on ARTIC | ACID's ARTIC press (ARP) dices GATE and SLIDE together, each with its own seed and previous seed (undo restores both). No separate slide gesture in v1. Sub-label `GT+SLD` says so. | One press: 3/3/3 = 9. A slide-only hold on ARP scores 2/2/3 = 7 and fails the performance rule (R4); it would also force ARP to act on release for all four algorithms. A slide-only binding stays a table row (flex point 7.2). |
| **S2** SHAPE naming and scope | Keep the layer and the name `SHAPE`; scope = "structure that is none of the other four". Its ambiguity is resolved on screen by the sub-label (`OCTAVE`, `DRIFT`, `LONG`, `-`) and the dice toast. The sub-label rule applies to all five buttons. | `SHAPE` alone: visual 2 (ambiguous) = 8; with sub-label 3 = 9 (R10). A rename alone does not help: the three algorithms' SHAPE contents differ, only the sub-label names each honestly. |
| **S3** ACID TYPE on K2 | Keep on K2 (FORM: how much of the loop repeats versus varies) and relabel `PEDAL` (0 = raw motif, 12 = the root hammered on every step: a pedal note). | `TYPE`: visual 2 = 8; `PEDAL`: 9 (R11). Moving it to K4 would push ACCNT off the SPICE slot, which ACID and EUCLID share. |
| **S3** SNAKE PATH on K4 | Keep on K4. LEN stays on K2 (FORM). | Both slots score 9 (labels are explicit). The slot rule is "K4 = the twist on top"; PATH re-orders the same notes and rhythm, which is that twist, while LEN is the loop length that every K2 is. Swapping would break "K2 = length or loop character" on 3 of 4 algorithms. |
| **S4** scale snap rule | Every note of every algorithm, **pins and live keys included**, is snapped at output by SP6: nearest scale tone by circular pitch-class distance, ties to the lower one, applied after span mapping (SP7). The unsnapped value (`pitch_orig`, a pin's `pin_note`) is kept, so changing the scale back restores the exact notes. `CHROM` passes notes through. | Scale or key change: heard on every channel from the next step, header shows it: output 3 (F18, F19). Snap-down (today's `t1_harmony_quantize`) moves a note up to 2 semitones on MINPEN versus 1 for nearest, and flattens low SPAN values (t1 risk 7). Pins entered with K3 move by scale degree, so they are in scale at entry anyway. |
| **S6** defaults | Confirmed: SCALE `MINPEN`, KEY C, OCTAVE 0; SNAKE `RANGE` 12. | Rule 5 / W5: PLAY on the defaults gives an in-scale, one-octave bass line on the first step (F25 9, F07 9). RANGE 24 spans two octaves, which on a TB-3 reads as a lead, not a bass (owner question 5). |

### 5.4 Algorithm doc edits that follow (the ROW 1 slot edits are applied by P1-013 with the ROW 2 parameters; the rest by P2-008)

| File | Edit |
| --- | --- |
| sting.md 6, 6.1 | Knob slots K1 `DENS`, K2 `PEDAL` (rename of `TYPE`), K3 `JUMP`, K4 `ACCNT`; `den` range 0..16. Shared-layer column; ARTIC = GATE + SLIDE (one press); sub-labels `GT+SLD`, `OCTAVE`. Pins as `pin_note[16]` (ALGORITHM_SET 5.4). |
| t1.md 6, 6.1 | `hit` range 0..16 (0 = keys only). Sub-labels `ROTATE` (HITS), `RATCH` (ARTIC); SHAPE absent. |
| snake.md 6, 6.1 | Slots K1 `DENS` 0..16, K2 `LEN`, K3 `RANGE` default 12, K4 `PATH`. Sub-labels `GATES`, `VELO`, `DRIFT`; ARTIC absent. |
| ml185.md 6, 6.1 | Slots K1 `DENS` 0..8, K2 `TOTAL`, K3 `RANGE`, K4 `RATCH`. Sub-labels `RESTS`, `VELO`, `RT+HLD`, `LONG`. |
| all four 6.5 | Step entry gesture is now specified (P1-014): white key = step (no REC tap), hold + K1 = pitch by scale degree, OCT+ then K4 = octave (this document W4). Pins are snapped at output (S4). |

## 6. Global scale and key (S5)

| Aspect | Specification |
| --- | --- |
| How many | One KEY, one SCALE and one OCTAVE for the whole device (owner decisions 3 and S5). No channel or algorithm owns a scale. |
| Where it is set | Hold SCL: any key (white or black) sets KEY to that key's pitch class; K1 KEY (C..B), K2 SCALE, K3 OCTAVE (-2..+2), K4 stays the morph. No menu. |
| Scale list (K2 order = `T1_SCALE_MASKS` index) | `CHROM MAJOR MINOR DORIAN PHRYG LYDIAN MIXO AEOL LOCRI MAJPEN MINPEN BLUES WHOLE HARMIN MELMIN DIM` (the stale `T1_SCALE_NAMES` order is deleted, ALGORITHM_SET section 9). |
| Defaults | KEY C, SCALE `MINPEN` (index 10), OCTAVE 0. |
| How each algorithm adheres | The pitch layer of every algorithm produces a semitone offset; SP9 maps it `midi = clamp(36 + KEY + 12*OCTAVE + offset, 0, 127)` and SP6 snaps it to SCALE (rule S4). ACID: motif and JUMP octaves; EUCLID: contour times SPAN; SNAKE and STAGE: RANGE span (SP7) then snap; pins: `pin_note`. Live keys in PERF use the same snap. |
| When it lands | The next step on every channel (all caches rebuild on a scale change, ALGORITHM_SET SP12). |
| Always on screen | Header of every screen except MENU: `C MINPEN` (plus `+1` when OCTAVE is not 0). The SCL layer shows the 12 pitch classes with the scale's notes lit; with MENU > SCALE LEDS on, the keys glow the scale. |
| Saved | In every preset (W11), with the channels. |

## 7. Flex points (ADR-0004 addendum)

### 7.1 Remappable by editing a table row

| Table | Row / field | What a row edit changes |
| --- | --- | --- |
| `control_map.tsv` (control x context) | action id, label | Any button, encoder, knob or key role in any context, for example swap FX and EDIT, move UNDO to OCT-, move mutes to other black keys. |
| Context table (1.2) | hold thresholds, enter/leave gesture | Make GLOB hold-only, change REC hold time. |
| Knob table (5.1) | slot, label, range, default, step | Remap K1..K4 per algorithm, rename `PEDAL`, change SNAKE RANGE default, STAGE TOTAL coarse step. |
| Dice table (5.2) | button -> layer, sub-label, seeds | Bind SLIDE alone to its own gesture (S1 fallback), rename a sub-label, mark a layer absent. |
| Global table (6) | scale list order, defaults | Default scale, key, octave; scale order on K2. |
| Black-key table | key -> MUTEn / CHn / ALL ON / RESTRT / TAP | Re-place channel keys if the channel count changes (owner question 31). |
| Defaults (section 2) | channel algorithms, MIDI channels, mutes | Power-on state. |

### 7.2 Not changeable without code

| Item | Why |
| --- | --- |
| The number of contexts and their precedence (1.2) | Input dispatch order in `ui_input()`. |
| Tap-on-release for buttons with a hold role | Gesture engine behaviour (`btn_hold`, `ui_layer.c`). |
| The five shared layers and their meanings | ALGORITHM_SET 10 (dice contract). |
| Snap rule SP6 and the output order (span, then snap) | Defines golden files. |
| Queued preset load at the loop start | Needs a scheduler hook in the ISR. |
| The main-screen bands (1.4) | Screen code; the contents of each band are table driven. |
