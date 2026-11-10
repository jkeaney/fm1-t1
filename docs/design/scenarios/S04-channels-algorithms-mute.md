# S04: select channels, switch algorithms, mute, solo, perform two channels

SEQUENCER_UX W2 and W5. Start: end of S02, playing, CH1 ACID on MIDI channel 2, CH2 EUCLID / CH3 SNAKE / CH4 STAGE
muted on MIDI channels 2, 3, 4. Notation as S01.

| Step | User action | Exact screen text | MIDI out | Row ids |
| --- | --- | --- | --- | --- |
| 1 | Turn SELECT one detent clockwise | CHB cell 2 inverted; HD `CH2 EUCL M2`; K1R `HITS 5` `OFFSET 0` `RATCH 4` `ACCNT 2`; K2R `LEN 16` `SPAN 12` `PUNCH 40` `CURVE AUTO`; BTN sub-labels `ROTATE` `RATCH`; the SHAPE button dim with a dash `SHAPE -` | none (CH2 is muted) | EN_SELECT/PERF |
| 2 | Look at HD: CH2 default MIDI channel is 2, the same as CH1 now | HD `CH2 EUCL M2` and CHB cell 1 `MIDI 2`: both lanes target the TB-3 channel and no warning exists (FINDINGS F-14) | none | EN_SELECT/PERF |
| 3 | Tap black key D#4 (CHN), turn K2 three detents clockwise, tap HOME | `MIDI 5` then PERF; HD `CH2 EUCL M5` | none | KEY_B05_D#4/PERF, EN_K2/CHAN, B_HOME/CHAN |
| 4 | Tap black key G#3 (M2) | CHB cell 2 loses `MUTE`; LEG cell M2 lit; activity dot starts | next step: `ch5 ON/OFF` pairs of the EUCLID pattern (5 hits in 16 steps) | KEY_B02_G#3/PERF |
| 5 | Turn K2 (OFFSET) three detents clockwise | K1R `OFFSET 3` | the 5 hits rotate by 3 steps from the next step | EN_K2/PERF |
| 6 | Press SEQ (HITS dice) | toast `HITS` with sub-label `ROTATE`; CUE `EDIT takes it back` | next step: a new rotation | B_SEQ/PERF |
| 7 | Press LFO (SHAPE, absent in EUCLID) | the button is dim `SHAPE -`; toast `NO SHAPE IN EUCLID`; no undo entry | none | B_LFO/PERF |
| 8 | Tap OCT+, turn K1 (LEN, row 2) four detents counter-clockwise, tap OCT- | K2R `LEN 12`, then ROW 1 live again | a 12-step cycle inside the 16-step channel loop from the next step | B_OCTUP/PERF, EN_K1/PERF, B_OCTDN/PERF |
| 9 | Turn ALGORITHM one detent clockwise | HD `CH2 SNAK M5`; K1R `DENS 10` `LEN 7` `RANGE 12` `PATH 7`; K2R `OFFSET 0` `GLEN 16` `DYN 10` `DIR FWD`; ARTIC dim `ARTIC -` | a new SNAKE line from the next step; no note is cut; the EUCLID morphs are kept under EUCLID | EN_ALGO/PERF |
| 10 | Press ARP (ARTIC, absent in SNAKE) | toast `NO ARTIC IN SNAKE` | none | B_ARP/PERF |
| 11 | Turn ALGORITHM one detent counter-clockwise | HD `CH2 EUCL M5`; K1R `HITS 5` `OFFSET 3` `RATCH 4` `ACCNT 2` (the previous EUCLID morphs, seeds and edits are back) | the rotated EUCLID line again | EN_ALGO/PERF |
| 12 | Turn SELECT five detents counter-clockwise | CHB cell 1 inverted (no wrap below CH1) | none | EN_SELECT/PERF |
| 13 | Tap black key F#3 (M1) | CHB cell 1 reads `MUTE`; LEG cell M1 dim | no new `ch2 ON`; pending `ch2 OFF` still goes out | KEY_B01_F#3/PERF |
| 14 | Tap black key F#3 (M1) again | mute cleared; phase kept running | `ch2 ON/OFF` resume on the running step grid | KEY_B01_F#3/PERF |
| 15 | Hold black key F#4 (SOL) for one bar and release | only CH1 sounds while held; CUE `SOLO CH1: let go`; on release the mutes are restored (CH2 stays unmuted) | `ch5` notes stop for the held bar, then resume | KEY_B06_F#4/PERF |
| 16 | Tap black key A#3 (M3) and C#4 (M4) | CHB cells 3 and 4 lose `MUTE` | `ch3` SNAKE and `ch4` STAGE notes start at the next step; the preview synth stays on CH1 | KEY_B03_A#3/PERF, KEY_B04_C#4/PERF |
| 17 | Turn SELECT three detents clockwise | CHB cell 4 inverted; HD `CH4 STAG M4`; K1R `DENS 6` `TOTAL 16` `DIR FWD` `RATCH 2` | none | EN_SELECT/PERF |
| 18 | Turn K2 (TOTAL) four detents clockwise | K1R `TOTAL` value of 20 or more with the heard count `(16 heard)` that overflows the cell (FINDINGS F-04); toast `17-32 NOT HEARD: D#4 K3 = OWN` | from the next step: pulses 17 to 20 are not read by any page | EN_K2/PERF |
| 19 | Tap black key C#4 (M4) | CH4 muted again | `ch4` stops | KEY_B04_C#4/PERF |

The mute keys are the only way to silence a channel while performing; they are one gesture and live on PERF, NOTE_EDIT,
CHAN and SET (KEY_B01..B04 on those pages), not on REC or SCALE where all 27 keys have other jobs.
MIDI status bytes: note on is `0x90` plus the channel number minus one, so `ch2` is `0x91`, `ch5` is `0x94`.
