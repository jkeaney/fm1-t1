# S02: dial in an ACID line on channel 1 and perform it (the target workflow)

Continues S01 after step 9: PLAY is running, CH1 ACID on MIDI channel 2 (the TB-3), SCALE MINPEN, key C, BPM `INT 120`,
DENS 14, SLIDE 6. The goal of the reference workflow of the vision: pick the algorithm, dial a sequence in with the knobs and
dice buttons, then perform it without leaving the main screen. Notation as S01.

| Step | User action | Exact screen text | MIDI out | Row ids |
| --- | --- | --- | --- | --- |
| 1 | Turn ALGORITHM one detent counter-clockwise, then one clockwise (ACID is already the algorithm of CH1; this proves the encoder returns it) | HD `CH1 EUCL M2` then `CH1 ACID M2`; CHB cell 1 `1 EUCL` then `1 ACID`; K1R labels `HITS OFFSET RATCH ACCNT` then `DENS SLIDE JUMP ACCNT`; morphs and seeds of ACID are kept, so DENS reads `14` again | the algorithm lands at the next step; no note is cut; the pattern returns to the same ACID notes (deterministic) | EN_ALGO/PERF |
| 2 | Turn K1 (DENS) four detents counter-clockwise | K1R `DENS 10` | from the next step: 10 hits in 16 steps | EN_K1/PERF |
| 3 | Turn K3 (JUMP) four detents clockwise | K1R `JUMP 9` | register jumps (octave moves) appear in the `ch2 ON` notes from the next step | EN_K3/PERF |
| 4 | Turn K4 (ACCNT) two detents clockwise | K1R `ACCNT 5` | more `ch2 ON` with velocity at or above 100, the TB-3 accent threshold | EN_K4/PERF |
| 5 | Tap OCT+ (row 2 latch) | ROW 2 is filled and live, ROW 1 dim; OCT+ cell inverted; toast `ROW 2`; CUE `ROW 2: OCT- = back to row 1` | none | B_OCTUP/PERF |
| 6 | Turn K2 (PEDAL, row 2) three detents counter-clockwise | K2R `PEDAL 3` (fewer steps sit on the root note) | from the next step: fewer repeats of the root note | EN_K2/PERF |
| 7 | Turn K4 (DIR, row 2) one detent clockwise | K2R `DIR BWD` | the 16 steps play in reverse order from the next step | EN_K4/PERF |
| 8 | Turn K1 (OFFSET, row 2) three detents clockwise | K2R `OFFSET 3` | the pattern is rotated by 3 steps | EN_K1/PERF |
| 9 | Turn K3 (PUNCH, row 2) one detent clockwise | K2R `PUNCH 41` | the velocity contrast changes | EN_K3/PERF |
| 10 | Tap OCT- (row 1 latch) | ROW 1 live again; toast `ROW 1`; CUE `TAP STEP=ON/OFF  HOLD=EDIT NOTE` | none | B_OCTDN/PERF |
| 11 | Hold OCT+ and turn K2 one detent (momentary peek), then release | while held the row 2 cells are drawn filled; `PEDAL 4`; on release row 1 is live again | from the next step: PEDAL 4 | B_OCTUP/PERF, EN_K2/PERF |
| 12 | Press SEQ (HITS dice) | Toast `HITS`; CUE `EDIT takes it back` (2 s); lane redraws with new hit places | next step: new rhythm, the same note set | B_SEQ/PERF |
| 13 | Press SCL (NOTES dice) | Toast `NOTES`; sub-label under the button `NOTES` | next step: same rhythm, new pitches | B_SCL/PERF |
| 14 | Press ENV (ACCNT dice) | Toast `ACCNT`; lane `!` glyphs move | next step: accents land on different steps | B_ENV/PERF |
| 15 | Press ARP (ARTIC dice) | Toast `ARTIC` with sub-label `GT+SLD`; lane `~` glyphs move | next step: new gate lengths and glides together | B_ARP/PERF |
| 16 | Press LFO (SHAPE dice) | Toast `SHAPE` with sub-label `OCTAVE`; lane register changes | next step: new octave jumps | B_LFO/PERF |
| 17 | Press FX (dice all) | Toast `ALL`; every layer is new | next step: a new line | B_FX/PERF |
| 18 | Press EDIT | Toast `UNDO` naming the dice-all; the previous line is back | previous line from the next step | B_EDIT/PERF |
| 19 | Press EDIT again | Toast `REDO`; the dice-all line returns | the dice-all line from the next step | B_EDIT/PERF |
| 20 | Press the black key F#4 (SOL) and hold it for one bar | key cell inverted; CUE `SOLO CH1: let go`; only CH1 sounds | only channel 2 notes (the other channels are muted already) | KEY_B06_F#4/PERF |
| 21 | Hold black key G#4 (FIL) with a firm press, then release | cell inverted with the live amount `FILL 80%` (the only locked readout string; how the percentage maps to force is not specified, FINDINGS F-11); DENS rises toward the maximum while held; on release DENS reads `10` again | extra hits while held; none after release | KEY_B07_G#4/PERF |
| 22 | Hold black key A#4 (ROL) with medium pressure, then release | `ROLL 3x`; CUE `ROLL 3x: let go to stop` | each step sends 3 `ch2 ON/OFF` pairs inside the step while held | KEY_B08_A#4/PERF |
| 23 | Tap black key F#5 (RST) | playhead returns to step 1 at the next step | the pattern restarts from step 1 on the next step | KEY_B11_F#5/PERF |
| 24 | Turn K1 down to zero (eleven detents counter-clockwise from DENS 10 with fast steps counted as one each) | K1R `DENS KEYS ONLY` (the value text is 62 px against a 51 px cell: FINDINGS F-04); CUE `TAP STEP=ON/OFF  HOLD=EDIT NOTE` | no generated hits; only steps turned on by hand play | EN_K1/PERF |

The performance loop used no menu and no page: 24 steps, each one gesture. Row 2 morphs need a latch tap first (two
gestures for the first row 2 morph, then one per turn); that is the locked R-5 count.
