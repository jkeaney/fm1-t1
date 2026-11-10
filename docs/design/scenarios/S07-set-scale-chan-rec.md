# S07: global settings (SET, SCALE) and recording notes (REC)

SEQUENCER_UX 2.4, 2.5, 3, U41 to U47 and U54 to U55. Start: stopped, CH1 ACID MIDI 2, BPM `INT 120`, scale MINPEN, key C,
octave 0. Notation as S01. Rows with `K1R` on a one-row page use the whole band (SCREEN_SPEC 2.4).

| Step | User action | Exact screen text | MIDI out | Row ids |
| --- | --- | --- | --- | --- |
| 1 | Tap GLO | Page SET. K1R `BPM 120` `SWING 50` `GATE 50` `DIV 1/16`; tag column `-` `-`; status line 1 `TEMPO INT 120` and `OUT MASTER`; status line 2 `host clock takes over`; GLO cell `BACK`; CUE `SET: GLO hold=SCALE  HOME=back` | none | B_GLO/PERF |
| 2 | Turn K1 eight detents clockwise | `BPM 128`; HD `INT 128`; status line 1 `TEMPO INT 128` | none (stopped); `beat_samples()` changes at once | EN_K1/SET |
| 3 | Turn K2 six detents clockwise | `SWING 56` (odd steps late by 12 % of a step) | from the next step on playing | EN_K2/SET |
| 4 | Turn K3 ten detents counter-clockwise | `GATE 40` (global gate, 50 is neutral) | from the next step on playing | EN_K3/SET |
| 5 | Turn K4 one detent clockwise | `DIV 1/8T` | step length changes from the next step | EN_K4/SET |
| 6 | Tap OCT+ | toast `ONE ROW HERE`; nothing changes | none | B_OCTUP/SET, B_OCTDN/SET |
| 7 | Press FX (dice all) while SET is open | toast `ALL`; lane redraws; the SET page stays open | next step: a new line | B_FX/SET, B_SCL/SET, B_ENV/SET, B_LFO/SET, B_ARP/SET, B_SEQ/SET, B_EDIT/SET |
| 8 | Press PLAY, then PLAY | the pattern runs, then stops | `FA`, notes, then `FC`, offs, `B1 7B 00` | B_PLAY/SET |
| 9 | Tap REC | REC opens from SET: see step 15 | none | B_REC/SET |
| 10 | Tap REC | leaves REC, back to PERF; tap GLO | none | B_REC/REC, B_GLO/PERF |
| 11 | Hold GLO for 0.4 s | Page SCALE. Banner `SCALE (all channels)  C MINPEN  OCT 0`; the 27-key picture with the scale tones lit and the root boxed; K1R `SCALE MINPEN` `KEY C` `OCTAVE 0` `-` (dim); tag `OCT-` `OCT+` read `OCTAVE -` `OCTAVE +`; GLO cell `BACK`; LEG and lane replaced by the picture; CUE `SCALE: PRESS A KEY = KEY  HOME=back` | none | B_GLO/PERF |
| 12 | Turn K1 one detent clockwise | `SCALE DORIAN`; scale tones relit | from the next step every pitch is re-snapped | EN_K1/SCALE |
| 13 | Tap white key E4 | `KEY E`; the root is boxed on E | the new root sounds once: `ch2 ON/OFF` of the E root | KEY_W07_E4/SCALE |
| 14 | Tap every other key once (the 15 other white keys and the 11 black keys) | `KEY` shows the pitch class of each key pressed: F, G, A, B, C, D, F, G, A, B, C, D, E, F, G for the white keys F3 to G5 and F#, G#, A#, C#, D#, F#, G#, A#, C#, D#, F# for the black keys | each key sounds its new root once | KEY_W*/SCALE, KEY_B*/SCALE |
| 15 | Turn K2 one detent counter-clockwise, K3 one detent clockwise | `KEY` of the previous pitch class; `OCTAVE +1` | from the next step | EN_K2/SCALE, EN_K3/SCALE |
| 16 | Tap OCT- | `OCTAVE 0` | from the next step | B_OCTDN/SCALE, B_OCTUP/SCALE |
| 17 | Turn K4 | nothing changes; the cell stays dim `-` | none | EN_K4/SCALE |
| 18 | Tap GLO | PERF | none | B_GLO/SCALE |
| 19 | Tap REC | Page REC, REC LED lit; HD transport `R`; banner `CH1 REC writing to page 1` in T_REC; lane with the cursor `^` on step 1; K1R `CURSOR 1` `ACC OFF` `SLIDE OFF` `OCT 0`; status line `KEYS = NOTES (all 27)`; tag `OCT-` `OCT+` read `OCT -` `OCT +`; CUE `REC: PLAY KEYS TO WRITE  REC=done` | none | B_REC/PERF |
| 20 | Press white key C4 and release it | cell 1 fills; the cursor moves to step 2 (`CURSOR 2`); with force sensing the cell reports the last velocity, `v 96` style | `ch2 ON n v` at landing plus about 5 ms, `ch2 OFF n` on release; v = landing force, or 100 without force | KEY_W05_C4/REC |
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

Step 14 shows the SCALE-page rule that all 27 keys set the key, so the mute keys are unreachable there; step 23 shows
the same for REC (a note, not a mute). Both are stated in the locked tables.
