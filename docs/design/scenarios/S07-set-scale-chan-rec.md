# S07: global settings (SET, HARMONY page, table id SCALE) and recording notes (REC)

SEQUENCER_UX 2.4, 2.5, 3, U41 to U47 and U54 to U55. Start: stopped, CH1 ACID MIDI 2, BPM `INT 120`, scale MINPEN, key C,
octave 0. Notation as S01. Rows with `K1R` on a one-row page use the whole band (SCREEN_SPEC 2.4).

| Step | User action | Exact screen text | MIDI out | Row ids |
| --- | --- | --- | --- | --- |
| 1 | Tap GLO | Page SET. K1R `BPM 120` `SWING 50` `GATE 50` `DIV 1/16`; tag column `OCT-` `OCT+` (OCTAVE 0); status line 1 `TEMPO INT 120` and `OUT MASTER`; status line 2 `host clock takes over`; GLO cell `BACK`; CUE `SET: GLO hold=HARM  HOME=back` | none | B_GLO/PERF |
| 2 | Turn K1 eight detents clockwise | `BPM 128`; HD `INT 128`; status line 1 `TEMPO INT 128` | none (stopped); `beat_samples()` changes at once | EN_K1/SET |
| 3 | Turn K2 six detents clockwise | `SWING 56` (odd steps late by 12 % of a step) | from the next step on playing | EN_K2/SET |
| 4 | Turn K3 ten detents counter-clockwise | `GATE 40` (global gate, 50 is neutral) | from the next step on playing | EN_K3/SET |
| 5 | Turn K4 one detent clockwise | `DIV 1/8T` | step length changes from the next step | EN_K4/SET |
| 6 | Tap OCT+, then OCT- | tag column reads `OCT +1`, then `OCT 0`; the global OCTAVE moves up one and back (it moved here from the SCALE page) | from the next step the notes are an octave higher, then as before | B_OCTUP/SET, B_OCTDN/SET |
| 7 | Press FX (dice all) while SET is open | toast `ALL`; lane redraws; the SET page stays open | next step: a new line | B_FX/SET, B_SCL/SET, B_ENV/SET, B_LFO/SET, B_ARP/SET, B_SEQ/SET, B_EDIT/SET |
| 8 | Press PLAY, then PLAY | the pattern runs, then stops | `FA`, notes, then `FC`, offs, `B1 7B 00` | B_PLAY/SET |
| 9 | Tap REC | REC opens from SET: see step 15 | none | B_REC/SET |
| 10 | Tap REC | leaves REC, back to PERF; tap GLO | none | B_REC/REC, B_GLO/PERF |
| 11 | Hold GLO for 0.4 s | Page HARMONY (table id SCALE). Lane header `HARMONY  C MINPEN  OCT 0`; the progression strip `1---` with `C` under the numeral and the legend `F3 G3 A3 B3 C4 D4 E4 = 1..7`; K1R `SCALE MINPEN` `KEY C` `VOICE 2` `MOVE 8`; tag `OCT-` `OCT+` over `DRONE OFF`; status line 1 `DRONE OFF`, status line 2 `CH1 ROLE AUTO`; buttons `PROG` `BARS` `ROLE` `NEW`; GLO cell `BACK`; CUE `HARM: KEYS F3-E4 = CHORD 1-7` | none | B_GLO/SET |
| 12 | Turn K1 one detent clockwise | `SCALE DORIAN`; scale tones relit | from the next step every pitch is re-snapped | EN_K1/SCALE |
| 13 | Turn K2 four detents clockwise | `KEY E`; the strip's chord name under the numeral reads `E`; the new root sounds once | the new root sounds once: `ch2 ON/OFF` of the E root | EN_K2/SCALE |
| 14 | Tap every white key F3 to G5 and every black key once | only the seven keys F3 to E4 act (steps 19 and 20 of S18); the other 20 keys do nothing and the cell labels read `-` | none from those 20 keys | KEY_W08_F4/SCALE, KEY_W09_G4/SCALE, KEY_W10_A4/SCALE, KEY_W11_B4/SCALE, KEY_W12_C5/SCALE, KEY_W13_D5/SCALE, KEY_W14_E5/SCALE, KEY_W15_F5/SCALE, KEY_W16_G5/SCALE, KEY_B*/SCALE |
| 15 | Turn K2 one detent counter-clockwise, K3 one detent clockwise | `KEY` of the previous pitch class; `VOICE 3` (the chord grows a seventh) | from the next step the AUTO channel is pulled toward the I7 chord tones | EN_K2/SCALE, EN_K3/SCALE |
| 16 | Tap OCT- | `DRONE` stays `OFF` (already at the bottom); toast `DRONE OFF` | none | B_OCTDN/SCALE, B_OCTUP/SCALE |
| 17 | Turn K4 one detent counter-clockwise | `MOVE 7` (with one chord nothing changes in the sound) | none | EN_K4/SCALE |
| 18 | Tap GLO | PERF | none | B_GLO/SCALE |
| 19 | Tap REC | Page REC, REC LED lit; HD transport `R`; banner `CH1 REC writing to page 1` in T_REC; lane with the cursor `^` on step 1; K1R `CURSOR 1` `ACC OFF` `SLIDE OFF` `OCT 0`; status line `KEYS = NOTES (all 27)`; tag `OCT-` `OCT+` read `OCT -` `OCT +`; CUE `REC: PLAY KEYS TO WRITE  REC=done` | none | B_REC/PERF |
| 20 | Press white key C4 and release it | cell 1 fills; the cursor moves to step 2 (`CURSOR 2`); with force sensing the cell reports the last level, `v MED 90` style | `ch2 ON n v` at landing plus about 5 ms, `ch2 OFF n` on release; v = landing level 64 / 90 / 120, or 90 without force | KEY_W05_C4/REC |
| 21 | Turn K2 and K3 one detent clockwise each, press white key E4 | `ACC ON`, `SLIDE ON`; cell 2 shows `!` and `~` | `ch2 ON n 120` (ACC forces 120) | EN_K2/REC, EN_K3/REC, KEY_W07_E4/REC |
| 22 | Turn K1 two detents counter-clockwise | `CURSOR 1` (wraps within 1..LENGTH); stopped: each cursor step sounds | `ch2 ON/OFF` of the step under the cursor | EN_K1/REC |
| 23 | Turn K4 one detent clockwise, press black key F#3, then tap OCT- | `OCT 1`, then `OCT 0`; the key plays and writes (the F#3 key is a note here, not MUTE1) | `ch2 ON/OFF` of F#3 plus the REC octave | EN_K4/REC, KEY_B01_F#3/REC, B_OCTDN/REC, B_OCTUP/REC |
| 24 | Press EDIT | toast `UNDO`; the last written note is removed and the cursor steps back | none | B_EDIT/REC |
| 25 | Hold EDIT 0.4 s, tap GLO, turn PRESETS | toasts `LEAVE REC FIRST` (EDIT hold), `REC: tap REC to finish` (GLO tap and PRESETS turn); nothing opens | none | B_EDIT/REC, B_GLO/REC, EN_PRESET/REC |
| 26 | Turn SELECT one detent clockwise, then back | cursor goes to step 1 of the other channel and back | none | EN_SELECT/REC |
| 27 | Turn ALGORITHM one detent clockwise and back | HD algorithm name changes and returns; REC stays open | the new algorithm's generated steps at the next step | EN_ALGO/REC |
| 28 | Press SEQ (HITS dice) while recording | toast `HITS`; performing while recording is allowed | next step | B_SEQ/REC, B_FX/REC, B_SCL/REC, B_ENV/REC, B_LFO/REC, B_ARP/REC |
| 29 | Tap SAVE | toast `SAVED P01` | none | B_SAVE/REC |
| 30 | Tap PLAY | the sequence plays while REC stays open; HD `>` and `R` shown by the REC banner | `FA`, `F8` stream, notes | B_PLAY/REC |
| 31 | Tap HOME | leaves REC, back to PERF | none | B_HOME/REC |
| 32 | Hold white key 9 for 0.4 s, then tap REC | NOTE_EDIT, then REC with the cursor on step 9; banner `CH1 REC writing to page 1` | audition of step 9 | KEY_W09_G4/PERF, B_REC/NOTE_EDIT |
| 33 | Tap REC | back to PERF | none | B_REC/REC |

Step 14 shows that on the HARMONY page only keys F3 to E4 act (the key is K2), so the mute keys are inert there; step 23 shows
the REC rule (a note, not a mute). Both are stated in the locked tables. HARMONY itself is walked in S18.
