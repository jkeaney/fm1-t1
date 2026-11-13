# S01: TB-3 first run, from power-on to a playing bassline

Walks SEQUENCER_UX 7 W1 and 11.3/11.4 against `control_map.tsv`, `knob_map.tsv`, `page_states.tsv` and SCREEN_SPEC.
Setup: FM-1 -> USB-C -> a USB-MIDI host box -> DIN -> Roland TB-3 MIDI IN (CLAUDE.md hardware facts). The TB-3 listens on
MIDI channel 2. The FM-1 is on its factory state, first power-on (first-run bit clear).

Notation. Zones follow SCREEN_SPEC 2: HD = header Z0, CHB = channel band Z1, LN = lane header Z2, K1R / K2R = knob band
ROW 1 / ROW 2 (Z3), BTN = button picture Z4, LEG = black-key legend Z5, CUE = cue strip Z6. `o` is the 8x8 clock dot.
A knob cell is written `LABEL value`. MIDI is written `ch2 ON n v` for note on, `ch2 OFF n` for note off. Pitches and
exact velocities come from the ACID golden files of P2-008, so this file states the shape of the MIDI, not the number.

Reference frame F-PERF-INT (PERF, stopped, power-on, nothing else touched):

| Zone | Exact text |
| --- | --- |
| HD | `.` `o`amber `INT 120` `P01` `1 ACID M1` `[1]. . .` `L16` |
| CHB | `1 ACID` / `MIDI 1` (inverted); `2 EUCL` / `MUTE 2` (dim cell; `MIDI 2 MUTE` was 68 px in a 56 px ink width, fixed by P2-011 F-04); `3 SNAK` / `MUTE 3`; `4 STAG` / `MUTE 4` |
| LN | `PG 1/1  FOLLOW  hold=EDIT` |
| K1R | `DENS 12` `SLIDE 4` `JUMP 5` `ACCNT 3` (live row, filled); tag column `OCT-` over `h:peek` |
| K2R | `OFFSET 0` `PEDAL 6` `PUNCH 40` `DIR FWD` (dim); tag column `OCT+` over `h:peek` |
| BTN row 1 | `ALL` `NOTES` `ACCNT` `SHAPE` (sub `OCTAVE`) `UNDO` (hold mark `RST`) `SET` (hold mark `HARM`) |
| BTN row 2 | `HOME` (hold mark `HLP/MN`) `SAVE` (hold mark `UNDO L`) `ARTIC` (sub `GT+SLD`) `HITS` `PLAY` `REC` |
| LEG | `M1 M2 M3 M4 CHN SOL FIL ROL PG- PG+ RST` |
| CUE | `CHN KEY, K2 = MIDI` (first-run step 1, priority 12 beats stopped, row 13) |

| Step | User action | Exact screen text | MIDI out | Row ids |
| --- | --- | --- | --- | --- |
| 1 | Power on the FM-1 | Splash for 400 ms, then F-PERF-INT. The amber dot blinks on the internal beat although stopped | none; no clock out while stopped (TIMING_SPEC M7) | - |
| 2 | Tap black key D#4 (CHN) | Page CHAN. Banner `CHANNEL 1   ACID   MIDI 1`; page map with page 1 `OWN` (dim); K1R `LENGTH 16` `MIDI 1` `SOURCE OWN` `FOLLOW ON`; tag column `-PAGE` / `+PAGE`; LEG cell D#4 reads `BACK`; footer `K2 = MIDI` / `CHN = BACK` (the CHAN footer; V2: it says the first-run step 1 already); status line 1 `PAGES  1 OWN  2 -  3 -  4 -`, line 2 `D#4 = BACK` (SCREEN_SPEC 7.1) | none | KEY_B05_D#4/PERF |
| 3 | Turn K2 one detent clockwise | K2 cell `MIDI 2`; HD `1 ACID M2`; CHB cell 1 line 2 `MIDI 2`; the first-run strip advances to `PRESS PLAY` (row 5a: it is shown on CHAN too) | none (stopped); notes sent after this go to MIDI channel 2; no other channel is in use and CH2..CH4 hold MIDI OFF (P4-044), so there is no clash and no toast (SEQUENCER_UX 4.4); EDIT would take the change back | EN_K2/CHAN |
| 4 | Tap HOME | PERF. HD `1 ACID M2`; CUE `PRESS PLAY`. A HOME tap on a page other than PERF returns to PERF | none | B_HOME/CHAN |
| 5 | Press PLAY | HD transport `>`; amber dot keeps blinking on the beat; lane playhead underline moves 1..16; CHB cell 1 activity dot flashes on each note out; CUE `TURN K1 TO MORPH IT`; PLAY green LED mid steady with a beat flash | `FA`, `F8` x24 per quarter at 120 BPM (clock out MASTER, R12); then `ch2 ON/OFF` pairs on the ACID pattern, one per hit, DENS 12 hits shaped as the golden; PREVIEW is OFF at power-on, so the speaker is silent (hold F#3 0.4 s to hear CH1: the preview then plays the same notes) | B_PLAY/PERF |
| 6 | Turn K2 (SLIDE) two detents clockwise | K1R cell `SLIDE 6`; lane glyph `~` appears on slid steps; the value is shown in the same frame | from the next step: on steps with a glide the next `ch2 ON` is sent before the previous `ch2 OFF` (TB-3 slide), more of them | EN_K2/PERF |
| 7 | Turn K1 (DENS) two detents clockwise | K1R `DENS 14`; more lane cells lit; first-run strip advances to `PRESS FX: NEW PATTERN` | from the next step: 14 hits in 16 steps | EN_K1/PERF |
| 8 | Press SCL (NOTES) until the notes please: one press | Toast `NOTES` with sub-label; CUE `EDIT takes it back` for 2 s; the lane keeps its rhythm | the same rhythm, new pitches from the next step, still scale notes | B_SCL/PERF |
| 9 | Press EDIT | Toast `UNDO` with the layer name; the old pitches return | the previous pitches from the next step | B_EDIT/PERF |
| 10 | Press PLAY | HD transport `.`; CUE `PLAY: start   K1-K4: morph` | `FC`; `ch2 OFF` for any sounding note; `B1 7B 00` (CC123 on channel 2) | B_PLAY/PERF |

Gesture count to the first sound: D#4, K2, HOME, PLAY = 4, or 3 when PLAY is pressed on CHAN (PLAY is live on every
page, `B_PLAY/CHAN`), which is the "3 gestures" of W1. Nothing here opens a menu. The two-gesture function is the MIDI
channel (page open + turn); see FINDINGS Q1.

Observed against the locked tables (details in FINDINGS.md), both fixed by P2-011: the first-run prompt was lost on CHAN
because the CHAN strip (priority 6) outranked first-run (priority 12) (F-09, now cue row 5a); the CHAN page
status lines had no text (F-05, now SCREEN_SPEC 7.1).
