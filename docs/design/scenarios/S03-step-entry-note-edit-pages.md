# S03: enter steps on the white keys, edit a note, grow to two pages

SEQUENCER_UX W3 and W4 walked gesture by gesture. Start: S01 state, PLAY stopped, CH1 ACID on MIDI channel 2, DENS 12.
Notation as S01. Keys are named by the printed key and its number n on the lane (F3 = 1 ... G5 = 16).

| Step | User action | Exact screen text | MIDI out | Row ids |
| --- | --- | --- | --- | --- |
| 1 | Turn K1 twelve detents counter-clockwise | K1R `DENS KEYS` (`KEYS ONLY` was 62 px in a 51 px cell; F-04 fixed); lane empty; HD unchanged | none | EN_K1/PERF |
| 2 | Tap white key 1 (F3) | Lane cell 1 filled with a corner tick (an edit); CUE keeps `PLAY: start   K1-K4: morph` | stopped: `ch2 ON n v` for step 1 now, `ch2 OFF n` after its gate | KEY_W01_F3/PERF |
| 3 | Tap white keys 5 (C4), 9 (G4) and 13 (D5) | Lane cells 5, 9 and 13 filled with corner ticks | each tap sounds its step once, as step 2 | KEY_W05_C4/PERF, KEY_W09_G4/PERF, KEY_W13_D5/PERF |
| 4 | Press PLAY | HD `>`; the playhead underline walks the lane; CUE `TAP STEP=ON/OFF  HOLD=EDIT NOTE` | `FA`, `F8` stream; `ch2 ON/OFF` on steps 1, 5, 9, 13 only | B_PLAY/PERF |
| 5 | Hard-tap white key 7 (E4) so that the step turns on | While the key is down the cell shows `ACC`; after release the cell shows `!` | `ch2 ON` at velocity 120 on step 7 from the next pass | KEY_W07_E4/PERF |
| 6 | Tap white key 5 again | Cell 5 empties, its corner tick goes (the edit now equals the generator, so it returns to AUTO) | step 5 stops sounding from its next pass | KEY_W05_C4/PERF |
| 7 | Hold white key 9 for 0.4 s | Page NOTE_EDIT. Header: a channel-colour chip `STEP 9` where the algorithm name was; status line `STEP 9` (and chips for set fields: `PROB 50`, `OCT-2`, `OFF`, `NO SLD`); lane cell 9 framed (and its key); K1R `PITCH GATE VELO RATCH`: each tile shows the value the algorithm gives, big and grey, with the word `AUTO` under it (a value you set is big and white, the tile filled, a marker square at its corner, `GEN <value>` under it); BTN six "this step" buttons (corner-bracket icons with a white tile outline; ENV, LFO, ARP, SEQ lit in the channel colour while on) `ALL` `NOTE` `ACCENT` `PROB` `UNDO` `SET` / `HOME` `SAVE` `SLIDE` `ON/OFF` `PLAY` `REC`; CUE footer `HOLD KEY = MOVE` (left) and `HOME = DONE` (right), on screen in every state; a toast (`[1] DICED  EDIT=UNDO`) takes the message slot only | the step sounds once (audition: not built, debt) | KEY_W09_G4/PERF |
| 8 | Turn K1 two detents clockwise | `PITCH` shows the scale note name big and white, the tile filled (a set value; the AUTO word is gone), `GEN <generated note>` under it. The first detent moves one scale degree up from the generated note (SEQUENCER_UX F-12: the first detent starts from the generated value and moves one step in the turning direction), the second one more; lane bar updates | each detent sends `ch2 ON n` / `ch2 OFF n` for 120 ms (audition: not built, debt); while playing also at each pass | EN_K1/NOTE_EDIT |
| 9 | Turn K2 clockwise until the cell reads `GATE TIE` | cell `GATE TIE`; lane glyph `-` | next pass: no `ch2 OFF` for step 9 until the next sounding step starts | EN_K2/NOTE_EDIT |
| 10 | Turn K3 two detents clockwise | cell `VELO` with a value of 1..127 and `*`; at 100 or more the value reads `ACC` | next pass at that velocity | EN_K3/NOTE_EDIT |
| 11 | Turn K4 two detents clockwise | cell `RATCH 2x*`; lane glyph `x2` | next pass: two `ch2 ON/OFF` pairs inside step 9 | EN_K4/NOTE_EDIT |
| 12 | Press ARP (SLIDE) | Button label reads `SLIDE`; cell 9 shows `~` toward cell 10 drawn dim, because step 10 is silent (no glide) | next pass: the note plays its own gate; no overlap | B_ARP/NOTE_EDIT |
| 13 | Press LFO (PROB) | `PROB 50`; lane glyph `%` | next pass: step 9 plays on a deterministic draw (about half the passes) | B_LFO/NOTE_EDIT |
| 14 | Press ENV (ACCENT) | `ACCENT` cell lit; K3 cell reads `ACC`; lane `!` | next pass at velocity 120 | B_ENV/NOTE_EDIT |
| 15 | Tap OCT+ | `OCT+1` on the tag; lane `^` glyph | audition one octave up; next pass one octave up | B_OCTUP/NOTE_EDIT |
| 16 | Press SCL (NOTE) | `PITCH` shows a new note and `*` | audition of the new pitch | B_SCL/NOTE_EDIT |
| 17 | Press FX (ALL) | PITCH, accent, slide and gate of step 9 all show new set values with `*` | audition | B_FX/NOTE_EDIT |
| 18 | Press EDIT | Toast `UNDO: STEP 9`; every field of step 9 reads as when the page opened (`AUTO`, step ON because step 3 set it) | next pass: the original step 9 | B_EDIT/NOTE_EDIT |
| 19 | Press SEQ (ON/OFF) | cell 9 hollow with a slash (forced off, the generator is silent so the edit returns to AUTO and the cell empties) | next pass: silent | B_SEQ/NOTE_EDIT |
| 20 | Hold white key 13 for 0.4 s | the box moves to cell 13; header chip `STEP 13`, status line `STEP 13`; footer `HOLD KEY = MOVE`, `HOME = DONE` | audition of step 13 | KEY_W13_D5/NOTE_EDIT |
| 21 | Tap white key 2 (G3) | cell 2 fills; the editor stays on step 13 | next pass: step 2 sounds | KEY_W02_G3/NOTE_EDIT |
| 22 | Tap HOME | PERF; CUE `TAP STEP=ON/OFF  HOLD=EDIT NOTE` | none | B_HOME/NOTE_EDIT |
| 23 | Tap black key D#5 (PG+) on page 1 of 1 | CUE `ADD PAGE 2 = COPY OF 1? PG+ = yes`; ghost page cell in the lane header row | none | KEY_B10_D#5/PERF |
| 24 | Tap D#5 (PG+) again | Page 2 appears: HD `[1][2]. .` `L32`; LN `PG 2/2` then the FOLLOW word (adding a page sets FOLLOW off, like PG-/PG+: SEQUENCER_UX 15, F-13) and `hold=EDIT`; toast `PAGE 2 = COPY OF PAGE 1: CHANGE IT`; the lane shows the copy | none audible: page 2 plays the same as page 1 | KEY_B10_D#5/PERF |
| 25 | Tap white key 4 (B3) | cell 4 filled on page 2 (step 20) | next pass of page 2: step 20 sounds | KEY_W04_B3/PERF |
| 26 | Tap black key C#5 (PG-) | the lane shows page 1; toast `VIEW P1 . FOLLOW OFF` | none | KEY_B09_C#5/PERF |
| 27 | Tap black key D#4 (CHN) | CHAN: pages `[1 OWN][2 P1][  ][  ]`; K1R `LENGTH 32` `MIDI 2` `SOURCE OWN` `FOLLOW OFF` | none | KEY_B05_D#4/PERF |
| 28 | Turn K4 one detent clockwise | `FOLLOW ON` | the view follows the playhead again | EN_K4/CHAN |
| 29 | Turn K1 twelve detents counter-clockwise | `LENGTH 20`; page map: page 2 partial; toast `HIDDEN, NOT LOST: turn K1 back` | from the next step the loop ends at step 20 | EN_K1/CHAN |
| 30 | Tap OCT- | LENGTH 20 goes to 16 (the page boundary below); page map: page 2 hollow (hidden, not lost) | the loop ends at step 16 | B_OCTDN/CHAN |
| 31 | Tap OCT+ | LENGTH 16 goes to 32; page 2 comes back as stored (step 20 still on); page map `[1][2]. .` | the loop is 32 steps again | B_OCTUP/CHAN |
| 32 | Turn K3 one detent clockwise while viewing page 2 | `SOURCE OWN`; toast `ACID REPEATS EVERY 16` | none audible: ACID repeats every 16 | EN_K3/CHAN |
| 33 | Turn K2 one detent counter-clockwise, then clockwise | `MIDI 1` then `MIDI 2`; HD `M1` then `M2` | notes after the change go to the new MIDI channel; sounding notes end on the old one | EN_K2/CHAN |
| 34 | Turn K1 twelve detents counter-clockwise (LENGTH 32 to 20), then tap D#4 (BACK) | PERF; page 2 shows steps 17 to 20 | the loop ends at step 20 | KEY_B05_D#4/CHAN |
| 35 | Tap white key 8 (F4) on page 2 | cell 8 (step 24) is dim; toast `PAST THE END: CHAN K1 = LENGTH` | none | KEY_W08_F4/PERF |
| 36 | Tap D#5 (PG+) on page 2 | CUE `SHOW ALL OF PAGE 2 (L32)? PG+ = yes` (a partial page is completed before a page 3 is added) | none | KEY_B10_D#5/PERF |
| 37 | Tap CHN, then hold white key 3 for 0.4 s (NOTE EDIT from CHAN) | NOTE_EDIT on step 3 of the viewed page; header chip `STEP 3`, status line `STEP 3` | audition | KEY_W03_A3/CHAN |

Observed (all fixed by P2-011, see FINDINGS): step 1 `KEYS ONLY` overflowed the knob cell (F-04, now `KEYS`). Step 9 and step 10 start the cell from AUTO and no
text said which value the first detent shows (F-12, SEQUENCER_UX 15). Step 7 NOTE_EDIT drew `(gen ...)` readouts in the
mock-up (SEQUENCER_UX 2.2) with no slot or string in SCREEN_SPEC (F-05, now the status line 1 of SCREEN_SPEC 7.1). Step 14 off direction (ENV toggled
off) was "back to 80 / AUTO" and did not say which (F-10, SEQUENCER_UX 15). The page-map glyphs for partial and hidden pages are now drawn (F-05, SCREEN_SPEC 2.1 and 7.1).
