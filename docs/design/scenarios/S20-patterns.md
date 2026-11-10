# S20: PATTERNS, from the picker to a performed library pattern on a chord progression (P2-014)

docs/design/algorithms/patterns/SPEC.md 4 to 7; numbers are the output of `tests/design/patterns_proto.py` (MINPEN, key C, octave 0, seeds 1 2 3 4 5; MIDI base note 36 + s).
Start: PLAY is running, CH1 ACID on MIDI channel 2 (the TB-3), CH2..CH4 muted, scale MINPEN, key C, HARMONY inert (`1---`, every ROLE AUTO), BPM `INT 120`. Notation as S01.

| Step | User action | Exact screen text | MIDI out | Row ids |
| --- | --- | --- | --- | --- |
| 1 | Turn ALGO five detents clockwise | for 1.2 s the lane header line shows the picker `ACID EUCL SNAK STAG ARP PATT` in six 40 px slots with `PATT` inverted; HD `1 PATT M2`; CHB cell 1 `1 PATT`; K1R labels `DENS VARY SPAN ACCNT`; lane header `ACD 1   16 STEPS  PG 1/1  hold=EDIT`; CUE `PICK: HOLD OCT+ AND TURN K1` | the algorithm lands at the next step; no note is cut; then the `ACID 1` line: `ch2 ON 36` accented (velocity 120) on step 1, `36` on step 2, `48` accented on step 5 | EN_ALGO/PERF |
| 2 | Tap OCT+ | ROW 2 is live: `PICK ACD 1`, `SLIDE 4`, `DYN 10`, `CTONE 2`; toast `ROW 2` | none | B_OCTUP/PERF |
| 3 | Turn K1 (PICK) two detents clockwise within 30 ms | the first detent steps to `ACD 2`, the second (under 40 ms later) jumps to the first pattern of the next category: cell `HSE 1`, lane header `HSE 1   16 STEPS`, toast `PATTERNS: HOUSE 1  LEN 16` | from the next step the offbeat bass: `ch2 ON 36` on steps 3, 7, 11 and 15 | EN_K1/PERF |
| 4 | Turn K1 two detents clockwise, one second apart | `HSE 2` then `HSE 3`; toast `PATTERNS: HOUSE 3  LEN 16` | from the next step the rolling line `. 36 36 36 . 36 36 36 . 36 36 36 . 36 36 48` (step 16 is the octave) | EN_K1/PERF |
| 5 | Tap OCT- | ROW 1 is live; toast `ROW 1` | none | B_OCTDN/PERF |
| 6 | Turn K1 (DENS) three detents counter-clockwise | K1R `DENS 5`; the lane shows the thinned steps dim | from the next step: `. 36 . 36 . 36 . 36 . 36 36 36 . 36 36 48` (the in-between 16ths of rank 3 are gone, the anchors stay) | EN_K1/PERF |
| 7 | Turn K2 (VARY) four detents clockwise | K2R `VARY 4` | step 11 becomes `46` (a scale neighbour above the root in MINPEN): `. 36 . 36 . 36 . 36 . 36 46 36 . 36 36 48` | EN_K2/PERF |
| 8 | Turn K3 (SPAN) three detents counter-clockwise | K3R `SPAN 1` | the octave folds into the first octave: step 16 plays `36` | EN_K3/PERF |
| 9 | Press SEQ (HITS dice) | toast `HITS`, sub-label `ORDER` under the button; CUE `EDIT takes it back` | which rank-3 steps DENS keeps changes from the next step (the same notes, other steps); no pitch is re-rolled | B_SEQ/PERF |
| 10 | Hold GLO for 0.4 s, then tap F3, B3, C4, F3 | page HARMONY; strip `1451`; names `C G Bb C` (each tap auditions its chord) | each tap auditions its chord on `ch2`; the pattern keeps playing | B_GLO/PERF, KEY_W01_F3/SCALE, KEY_W04_B3/SCALE, KEY_W05_C4/SCALE |
| 11 | Tap HOME | PERF; chord chip `I` follows the bar; lane header unchanged | the one `C`-flagged step (step 2) follows the chord: `36` in bar 1, `39` in bar 2 (chord G Bb Eb), `39` in bar 3, `36` in bar 4; every other step is unchanged | B_HOME/SCALE |
| 12 | Hold GLO for 0.4 s, turn K1 (SCALE) one detent counter-clockwise, tap HOME | K1R `SCALE MAJPEN`; PERF | from the next step: the notes are re-quantised, not re-picked: the VARY note of step 11 plays `45` instead of `46`; the anchors stay `36` | B_GLO/PERF, EN_K1/SCALE, B_HOME/SCALE |
| 13 | Tap OCT+ and turn K1 (PICK) until the cell reads `MEL 3`, then tap OCT- | toast `PATTERNS: MELODY 3  LEN 32`; the page map shows two pages; lane header `MEL 3   32 STEPS  PG 1/2` | the 32-step call and answer in MELODY degrees: MINPEN gives `ch2 ON 36` 39 41 for degrees 0 1 2 on bar 1, bar 2 answers on `46` | B_OCTUP/PERF, EN_K1/PERF, B_OCTDN/PERF |
| 14 | Hold white key F3 for 0.4 s, then turn K1 (PITCH) one detent | NOTE EDIT on step 1; the step is now an edit (`'` in the lane) | step 1 plays its edited pitch from the next pass | KEY_W01_F3/PERF, EN_K1/NOTE_EDIT |
| 15 | Tap HOME, then press EDIT | toast `UNDO` for the last record; the NOTE EDIT change is taken back (an earlier PICK is its own record) | step 1 plays the pattern pitch again | B_EDIT/PERF |
| 16 | Press PLAY (stop), then tap OCT+ and turn K1 one detent counter-clockwise, and wait 0.4 s | stopped; cell `MEL 2`; the lane plays one pass without moving the transport | one audition pass of `MELODY 2` through `ch2` at the current tempo, then silence | B_PLAY/PERF, B_OCTUP/PERF, EN_K1/PERF |
| 17 | Turn ALGO one detent counter-clockwise, then one clockwise | picker `ARP` then `PATT`; the PICK, morphs and seeds of the channel are the same (`MEL 2`, `DENS 5`); edits kept | after PLAY the pattern is the same as before the algorithm was changed | EN_ALGO/PERF |
