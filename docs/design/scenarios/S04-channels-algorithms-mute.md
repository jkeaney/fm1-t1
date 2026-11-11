# S04: select channels, switch algorithms, mute, solo, perform two channels

SEQUENCER_UX W2 and W5. Start: end of S02, playing, CH1 ACID on MIDI channel 2, CH2 EUCLID / CH3 SNAKE / CH4 STAGE
muted on MIDI channels OFF, 3, 4 (CH2's default 2 was taken by CH1 in S01: MIDI channels are unique, SEQUENCER_UX 4.4). Notation as S01.

| Step | User action | Exact screen text | MIDI out | Row ids |
| --- | --- | --- | --- | --- |
| 1 | Turn SELECT one detent clockwise | CHB cell 2 inverted; HD `2 EUCL MOFF`; CHB cell 2 line 2 `MUTE -`; K1R `HITS 5` `OFFSET 0` `RATCH 4` `ACCNT 2`; K2R `LEN 16` `SPAN 12` `PUNCH 40` `CURVE AUTO`; BTN sub-labels `ROTATE` `RATCH` `MOD` (SHAPE is alive in EUCLID: it re-rolls `s_mod`, P2-022 and P2-017) | none (CH2 is muted) | EN_SELECT/PERF |
| 2 | Look at HD: CH2's default MIDI channel 2 is the one CH1 now uses | HD `2 EUCL MOFF` and CHB cell 1 `MIDI 2`: the two lanes cannot both target the TB-3 channel; CH2 gave it up when CH1 took it (owner ruling, FINDINGS F-14 decided) | none | EN_SELECT/PERF |
| 3 | Tap black key D#4 (CHN), turn K2 until the cell reads `MIDI 5`, tap HOME | K2 cell `OFF` first, then `5` (dim until 0.6 s of stillness, then committed; no other channel uses 5, so no toast), then PERF; HD `2 EUCL M5` | none | KEY_B05_D#4/PERF, EN_K2/CHAN, B_HOME/CHAN |
| 4 | Tap black key G#3 (M2) | CHB cell 2 loses `MUTE`; LEG cell M2 lit; activity dot starts | next step: `ch5 ON/OFF` pairs of the EUCLID pattern (5 hits in 16 steps) | KEY_B02_G#3/PERF |
| 5 | Turn K2 (OFFSET) three detents clockwise | K1R `OFFSET 3` | the 5 hits rotate by 3 steps from the next step | EN_K2/PERF |
| 6 | Press SEQ (HITS dice) | toast `HITS` with sub-label `ROTATE`; CUE `EDIT takes it back` | next step: a new rotation | B_SEQ/PERF |
| 7 | Press LFO (SHAPE, sub-label `MOD` in EUCLID) | toast `SHAPE` with sub-label `MOD`; CUE `EDIT takes it back`; one undo entry (`s_mod`) | none unless a device slot is active: then the CC stream of the slots changes from the next step, the notes do not | B_LFO/PERF |
| 8 | Tap OCT+, turn K1 (LEN, row 2) four detents counter-clockwise, tap OCT- | K2R `LEN 12`, then ROW 1 live again | a 12-step cycle inside the 16-step channel loop from the next step | B_OCTUP/PERF, EN_K1/PERF, B_OCTDN/PERF |
| 9 | Turn ALGORITHM one detent clockwise | HD `2 SNAK M5`; K1R `DENS 10` `LEN 7` `RANGE 12` `PATH 7`; K2R `OFFSET 0` `GLEN 16` `DYN 10` `DIR FWD`; ARTIC dim `ARTIC -` | a new SNAKE line from the next step; no note is cut; the EUCLID morphs are kept under EUCLID | EN_ALGO/PERF |
| 10 | Press ARP (ARTIC, absent in SNAKE) | toast `NO ARTIC IN SNAKE` | none | B_ARP/PERF |
| 11 | Turn ALGORITHM one detent counter-clockwise | HD `2 EUCL M5`; K1R `HITS 5` `OFFSET 3` `RATCH 4` `ACCNT 2` (the previous EUCLID morphs, seeds and edits are back) | the rotated EUCLID line again | EN_ALGO/PERF |
| 12 | Turn SELECT five detents counter-clockwise | CHB cell 1 inverted (no wrap below CH1) | none | EN_SELECT/PERF |
| 13 | Tap black key F#3 (M1) | CHB cell 1 reads `MUTE 2`; LEG cell M1 dim | no new `ch2 ON`; pending `ch2 OFF` still goes out | KEY_B01_F#3/PERF |
| 14 | Tap black key F#3 (M1) again | mute cleared; phase kept running | `ch2 ON/OFF` resume on the running step grid (a mute closes only the MIDI gate: if CH1 PREVIEW is ON it kept sounding on the speaker while muted, a CUE) | KEY_B01_F#3/PERF |
| 15 | Hold black key F#4 (SOL) for one bar and release | only CH1 sounds while held; CUE `SOLO CH1: let go`; on release the mutes are restored (CH2 stays unmuted) | `ch5` notes stop for the held bar, then resume | KEY_B06_F#4/PERF |
| 16 | Tap black key A#3 (M3) and C#4 (M4) | CHB cells 3 and 4 lose `MUTE` | `ch3` SNAKE and `ch4` STAGE notes start at the next step; no preview part starts (PREVIEW is OFF on CH3 and CH4) | KEY_B03_A#3/PERF, KEY_B04_C#4/PERF |
| 17 | Turn SELECT three detents clockwise | CHB cell 4 inverted; HD `4 STAG M4`; K1R `DENS 6` `TOTAL 16` `DIR FWD` `RATCH 2` | none | EN_SELECT/PERF |
| 18 | Turn K2 (TOTAL) four detents clockwise | K1R `TOTAL` value `20 (16)`: the bracket is the heard count (`20 (16 heard)` was 71 px against 51; F-04 fixed); toast `17-32 NOT HEARD: D#4 K3 = OWN` | from the next step: pulses 17 to 20 are not read by any page | EN_K2/PERF |
| 19 | Tap black key C#4 (M4) | CH4 muted again | `ch4` stops | KEY_B04_C#4/PERF |

The mute keys are the only way to silence a channel while performing; they are one gesture and live on PERF, NOTE_EDIT,
CHAN and SET (KEY_B01..B04 on those pages), not on REC or SCALE where all 27 keys have other jobs.
MIDI status bytes: note on is `0x90` plus the channel number minus one, so `ch2` is `0x91`, `ch5` is `0x94`.
