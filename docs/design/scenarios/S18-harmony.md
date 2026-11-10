# S18: HARMONY, from the inert default to a performed progression (P2-012)

HARMONY.md 1 to 8, SEQUENCER_UX 2.4 and U70 to U79. Start: stopped, CH1 ACID MIDI 2 (the TB-3), CH2..CH4 muted, scale MINPEN, key C, octave 0, VOICE 2, MOVE 8,
PEDAL off, progression `1---` (one chord, degree 1, 4 bars), every ROLE AUTO. Notation as S01. MIDI base note is 36 + key + 12 x octave, so a semitone `s` above the key is `36 + s`.
In MINPEN the degrees 1 to 5 are the tones C, Eb, F, G, Bb, so `1451` is the chords C, G, Bb, C and every chord is built from scale tones only (HARMONY.md 3.1).

| Step | User action | Exact screen text | MIDI out | Row ids |
| --- | --- | --- | --- | --- |
| 1 | Press PLAY with the default HARMONY state | PERF as in S02: the lane header shows no chord chip (HARMONY is inert) | `FA`, `F8` stream, then the same `ch2 ON/OFF` bytes, bar after bar, as a build without HARMONY (the golden of P2-008) | B_PLAY/PERF |
| 2 | Hold GLO for 0.4 s | Page HARMONY. Lane header `HARMONY  C MINPEN  OCT 0`; strip `1---` with `C` under the first numeral; legend `F3 G3 A3 B3 C4 D4 E4 = 1..7`; K1R `SCALE MINPEN` `KEY C` `VOICE 2` `MOVE 8`; tag column `OCT-` `OCT+`; status line 1 `PEDAL OFF`, line 2 `CH1 ROLE AUTO`; buttons `PROG` `BARS` `ROLE` `NEW`; CUE `HARM: KEYS F3-E4 = CHORD 1-7` | none; the pattern keeps playing | B_GLO/PERF |
| 3 | Tap key F3 (chord 1) | strip stays `1---` (the first key starts a new progression: one chord, degree 1; the same as the default, so HARMONY is still inert); the chord sounds | audition, ascending 80 ms apart, 120 ms long: `ch2 ON/OFF` 36, 39, 43 (C, Eb, G) | KEY_W01_F3/SCALE |
| 4 | Tap key B3 (chord 4) | strip `1-4-` (two chords share 4 bars: chord 2 starts at bar 3); names `C` and `G`; HARMONY is now active, so the lane header on PERF will show a chord chip | audition `ch2 ON/OFF` 43, 46, 51 (G, Bb, Eb) | KEY_W04_B3/SCALE |
| 5 | Tap key C4 (chord 5) | strip `145-`; names `C G Bb` | audition 46, 51, 53 (Bb, Eb, F) | KEY_W05_C4/SCALE |
| 6 | Tap key F3 (chord 1) | strip `1451`; names `C G Bb C`; the progression is `1-4-5-1` as typed | audition 36, 39, 43 | KEY_W01_F3/SCALE |
| 7 | Tap HOME | PERF; the lane header shows the chord chip `I` (right-aligned, left of `>P1`) and it follows the bar: `I` `iv` `V` `I`; the AUTO channel CH1 now lands strong steps on chord tones | from the next bar CH1 plays chord-tone-pulled notes: bar 1 starts on `ch2 ON 36` (the root of I), bar 2 on 43 (G) | B_HOME/SCALE |
| 8 | Hold GLO for 0.4 s, then turn K3 one detent clockwise | `VOICE 3`; the strip is unchanged; the chords gain their seventh (C Eb G Bb) | from the next step the pull targets include the seventh (Bb = 46 over I) | B_GLO/PERF, EN_K3/SCALE |
| 9 | Press SCL (PROG) | the progression is re-rolled from the sample counter; with its low 16 bits 0x1234 and MINPEN (5 degrees) the result is 4 bars, 2 chords, degrees `1` `4`: strip `1-4-`; toast `PROG` | the new chords from the next bar | B_SCL/SCALE |
| 10 | Press EDIT | toast `UNDO: PROG`; the strip is `1451` again | the old chords from the next bar | B_EDIT/SCALE |
| 11 | Press LFO (BARS) | strip `1451-` (5 bars, 4 chords: chord 4 lasts 2 bars); toast `BARS 5` | heard at the next phrase start | B_LFO/SCALE |
| 12 | Press ARP (ROLE) | status line 2 `CH1 ROLE ROOT`: CH1 plays only chord roots (the pedal if one is set) in the octave of the algorithm's note | from the next step CH1 plays 36 (C), then 43 (G) over bar 2, 46 (Bb) over bar 3 | B_ARP/SCALE |
| 13 | Press ARP three times | `ROLE FULL`, then `ROLE OFF`, then `ROLE AUTO` (cycle OFF AUTO ROOT FULL wraps); with OFF the channel plays its unharmonised notes | OFF: the same bytes as step 1 | B_ARP/SCALE |
| 14 | Tap OCT+ | tag column `PEDAL 1`; status line 1 `PEDAL 1 C +0  + - - +` (degree 1, tone C, octave shift +0; one mark per chord of the loop: `+` the pedal is a chord tone, `-` it is a passing tone, `X` it is a semitone from a chord tone) | AUTO channels are pulled toward C as well as the chord | B_OCTUP/SCALE |
| 15 | Turn K4 one detent counter-clockwise | `MOVE 7` (MOVE only changes FULL channels and the voiced notes the strip would audition) | none | EN_K4/SCALE |
| 16 | Turn SELECT one detent clockwise | status line 2 `CH2 ROLE AUTO` (the role is per channel; CH2 is muted) | none | EN_SELECT/SCALE |
| 17 | Press SEQ (NEW) | toast `NEW: TYPE CHORDS`; nothing changes until a key is typed | none | B_SEQ/SCALE |
| 18 | Hold EDIT 0.4 s, then press SCL | RESET page over HARMONY: the progression goes back to `1---`; toast `RST PROG`; RESET closes | the chord stage is inert again once VOICE is 2 and PEDAL is off | B_EDIT/SCALE, B_SCL/RESET |
| 19 | Tap OCT- | PEDAL walks back to `PEDAL OFF` | none | B_OCTDN/SCALE |
| 20 | Tap HOME | PERF; the chord chip is gone (inert); CH1 is back to its unharmonised notes | the bytes of step 1 | B_HOME/SCALE |
