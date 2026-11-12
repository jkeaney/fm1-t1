### buttons: panel order (ui_sq.c uisq_btn_ctl / uisq_btn_icon) -> icon drawn -> button_modes NOTE_EDIT tap row (label, action) and the PERF label of the same button
| # | control | icon drawn (tile) | tile fill / icon colour | button_modes NOTE_EDIT label | NOTE_EDIT tap action (first 150 chars) | PERF label | text on the tile? |
|---|---|---|---|---|---|---|---|
| 1 | B_FX | SQI_DICE_ALL 13x13 | DICE / WHITE | ALL | press: DICE THIS STEP: new pitch, accent, slide and gate for the edited step from fresh entropy, written as edits and auditioned; EDIT tap undoes | ALL | no |
| 2 | B_SCL | SQI_DICE_NOTES 13x13 | DICE / WHITE | NOTE | press: dice the edited step's pitch: a new scale note, written as its PITCH edit and auditioned; EDIT tap undoes | NOTES | no |
| 3 | B_ENV | SQI_DICE_ACCNT 13x13 | DICE / WHITE | ACCENT | press: toggle ACCENT on the edited step: velocity to ACC_VEL 120, or back to 80 / AUTO; EDIT tap undoes | ACCNT | no |
| 4 | B_LFO | SQI_DICE_SHAPE 13x13 | DICE / WHITE | PROB | press: toggle PROB on the edited step: 100 % / 50 % (PROB_LOW), drawn deterministically per pass; EDIT tap undoes | SHAPE | no |
| 5 | B_EDIT | SQI_UNDO 13x11 | SURF / TEXT | UNDO | UNDO: the edited step back to how it was when this NOTE EDIT visit began; again = redo | UNDO | no |
| 6 | B_GLO | SQI_SETTINGS 13x11 | SURF / TEXT | SET | open SET (latched): BPM SWING GATE DIV (closes NOTE EDIT) | SET | no |
| 7 | B_HOME | SQI_HOME 13x11 | SURF / TEXT | HOME | release under 0.7 s, nothing touched: back to PERF (leaves NOTE_EDIT) | HOME | no |
| 8 | B_SAVE | SQI_SAVE 13x11 | SURF / TEXT | SAVE | release under 0.7 s: save the whole device state to the slot in the header; while a slot is pending (header Pnn?), save to the PENDING slot instead an | SAVE | no |
| 9 | B_ARP | SQI_DICE_ARTIC 13x13 | DICE / WHITE | SLIDE | press: toggle SLIDE on the edited step: this note glides into the next step (step 1 after the last step; TB-303 rule; no glide if that step is silent) | ARTIC | no |
| 10 | B_SEQ | SQI_DICE_HITS 13x13 | DICE / WHITE | ON/OFF | press: toggle the edited step ON/OFF (same as tapping its key); EDIT tap undoes | HITS | no |
| 11 | B_PLAY | SQI_PLAY 13x11 | SURF / TEXT | PLAY | press: start / stop all channels (stop sends every note-off then CC123) | PLAY | no |
| 12 | B_REC | SQI_REC_RING 13x11 | SURF / TEXT | REC | press: open REC with the write cursor on the edited step | REC | no |
| - | B_OCTDN | (no tile on the screen) | | OCT- | release: the edited step one octave down (OCT -2..+2), auditioned | | no |
| - | B_OCTUP | (no tile on the screen) | | OCT+ | release: the edited step one octave up (OCT -2..+2), auditioned | | no |

### knobs: knob_map NOTE_EDIT rows vs the four tiles
- K1 `PITCH`: one scale degree per detent from the current note; every detent sounds the note (MIDI + preview); the first detent turns AUTO into a set pitch | range: AUTO, then 0..96 semitones above 36+key (stored unsnapped, played snapped)
- K2 `GATE`: note length of the edited step, x global GATE; top value TIE (holds into the next step) | range: AUTO, 10..100 % in 10 % steps, TIE
- K3 `VELO`: velocity of the edited step; value reads ACC at >= ACC_THR 100 (TB-3 accent) | range: AUTO, 1..127, step 1 fast 8
- K4 `RATCH`: repeats inside the step (SP5 spacing) | range: AUTO, 1x 2x 3x 4x
- SELECT `CH n`: turn: select channel CH1..CH4; closes NOTE EDIT (the edited step belongs to the old channel) | range: CH1..CH4
- ALGO `ALGO`: turn: algorithm of the selected channel ACID EUCLID SNAKE STAGE ARP PATTERNS; lands at the next step; morphs and seeds kept per algorithm (S7); edits kept | range: 6 algorithms
- PRESET `Pnn`: turn: choose a preset slot P01..P16; it becomes PENDING (header Pnn?, strip Pnn: SAVE = save here); stopped: it loads 1.5 s after the last detent (SQ_LOAD_SETTLE); playing: on the first 768-tick bar t | range: P01..P16

### keys: key_map NOTE_EDIT rows (white W01 and every black key) vs what the keybed draws
- KEY_W01_F3 tap: label `STEP` | release under 0.4 s: step 1 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: di
- KEY_W01_F3 hold: label `EDIT` | held 0.4 s: move NOTE EDIT to step 1 (no new page)
- KEY_B01_F#3 tap: label `M1` | tap: mute / unmute CH1 (latched; phase keeps running)
- KEY_B01_F#3 hold: label `PRV1` | held 0.4 s: hint PREVIEW CH1; released before 1.5 s = PREVIEW on/off of CH1 (on release; toast); with ALGO turned: its preview sound ACID LEAD BASS PAD STRINGS PIANO (tur
- KEY_B02_G#3 tap: label `M2` | tap: mute / unmute CH2
- KEY_B02_G#3 hold: label `PRV2` | held 0.4 s: hint PREVIEW CH2; released before 1.5 s = PREVIEW on/off of CH2 (on release; toast); with ALGO turned: its preview sound ACID LEAD BASS PAD STRINGS PIANO (tur
- KEY_B03_A#3 tap: label `M3` | tap: mute / unmute CH3
- KEY_B03_A#3 hold: label `PRV3` | held 0.4 s: hint PREVIEW CH3; released before 1.5 s = PREVIEW on/off of CH3 (on release; toast); with ALGO turned: its preview sound ACID LEAD BASS PAD STRINGS PIANO (tur
- KEY_B04_C#4 tap: label `M4` | tap: mute / unmute CH4
- KEY_B04_C#4 hold: label `PRV4` | held 0.4 s: hint PREVIEW CH4; released before 1.5 s = PREVIEW on/off of CH4 (on release; toast); with ALGO turned: its preview sound ACID LEAD BASS PAD STRINGS PIANO (tur
- KEY_B05_D#4 tap: label `CHN` | switch to the CHAN page (closes NOTE EDIT)
- KEY_B06_F#4 hold: label `SOL` | hold: only the selected channel sounds; release restores the mutes
- KEY_B07_G#4 hold: label `FIL` | hold: the selected channel's K1 density goes up while held (force: light = halfway, firm = maximum); release restores
- KEY_B08_A#4 hold: label `ROL` | hold: every step of the selected channel repeats while held (force: 2x light, 3x medium, 4x firm)
- KEY_B09_C#5 tap: label `PG-` | tap: view the previous page of the selected channel (no wrap; FOLLOW off)
- KEY_B10_D#5 tap: label `PG+` | tap: view the next page (FOLLOW off); on the last page: a prompt that names what PG+ again will do: ADD PAGE n = COPY OF n-1 (edits and source; LENGTH to 16 x pages, max 
- KEY_B11_F#5 tap: label `RST` | tap: all channels to step 1 at the next step

### note_edit_text.tsv: each id, the string, and whether it appears on any of the 4 renders + 2 worst cases (as a whole string or as a part of a built string)
- AUTO       `AUTO` -> note_dice, note_final, note_open, note_set, worst_w2
- ACC        `ACC` -> worst_w1, worst_w2
- TIE        `TIE` -> worst_w2
- X          `X` -> note_dice, note_final, note_open, note_set, worst_w1, worst_w2
- GEN        `GEN` -> note_dice, note_final, note_open, note_set, worst_w1, worst_w2
- STEP       `STEP` -> note_dice, note_final, note_open, note_set, worst_w1, worst_w2
- ON         `ON` -> NOT on any of the six frames
- OFF        `OFF` -> worst_w1
- SLIDE      `SLIDE` -> note_dice, note_set, worst_w1
- NO_SLIDE   `NO SLIDE` -> note_dice, worst_w1
- PROB       `PROB` -> note_dice, note_set, worst_w1
- OCT        `OCT` -> note_dice, note_set, worst_w1
- PLUS       `+` -> worst_w1
- MINUS      `-` -> note_dice, note_set, worst_w1, worst_w2
- SET_MARK   `= SET BY YOU` -> note_final, note_open, worst_w2
- KNOBS_EDIT `KNOBS EDIT` -> note_dice, note_final, note_open, note_set, worst_w1, worst_w2
- HOME_DONE  `HOME = DONE` -> note_dice, note_final, note_open, note_set, worst_w1, worst_w2
- NOTE_C     `C` -> note_dice, note_final, note_open, note_set
- NOTE_CS    `C#` -> worst_w1
- NOTE_D     `D` -> NOT on any of the six frames
- NOTE_DS    `D#` -> worst_w2
- NOTE_E     `E` -> NOT on any of the six frames
- NOTE_F     `F` -> note_dice
- NOTE_FS    `F#` -> NOT on any of the six frames
- NOTE_G     `G` -> NOT on any of the six frames
- NOTE_GS    `G#` -> worst_w1
- NOTE_A     `A` -> NOT on any of the six frames
- NOTE_AS    `A#` -> worst_w2
- NOTE_B     `B` -> NOT on any of the six frames

### strings on the NOTE_EDIT frames that are NOT in ui_ne_text, knob_map, or a persistent PERF element (built or literal in ui_sq.c)
- `PROB 50`: the suffix ` 50` is a literal in ui_sq.c (uisq_cat(..., " 50")); tsv PROB holds `PROB` only
- `GEN <v>`: the joining space is a literal in uisq_ne_gen_line
- `S` `F` `R`: black-key letters, literals in uisq_draw_keybed (the same on PERF)
- digits (STEP n, 10..100, 1..127, n X, OCT sign digit, note octave): built by uisq_catu; `120` tempo, `1`..`4` chips, `ACID` (algorithm short name), `MIDI` + channel: persistent header

### page_states.tsv NOTE_EDIT row (the older contract) vs the render
{'state': 'NOTE_EDIT', 'kind': 'page', 'enter': 'hold a white step key 0.4 s (PERF, CHAN, SET, NOTE_EDIT)', 'leave': 'HOME tap; SELECT turn; REC tap (to REC at this step); GLO / CHAN key switch page', 'screen': 'header + lane (edited step outlined) + NOTE EDIT band: 4 knob cells PITCH GATE VELO RATCH with AUTO/set state + 6 button cells (ALL NOTE ON/OFF ACCENT SLIDE PROB) + OCT cells OCT-/OCT+ + legend + cue', 'knob_band': 'K1 PITCH, K2 GATE, K3 VELO, K4 RATCH of the edited step', 'keys': 'white: tap on/off, hold = edit that step; black = moves as PERF', 'cue_strip': 'STEP n: KNOBS EDIT  HOME=done', 'latched': 'yes'}
{'state': 'CHAN', 'kind': 'page', 'enter': 'tap black D#4 (CHAN) from PERF, NOTE_EDIT, SET (on SCALE D#4 is inert)', 'leave': 'D#4 tap again; HOME tap; GLO / REC switch page', 'screen': 'SCREEN_SPEC_V2: header + message slot (page title CHN CHn SETUP while open) + CHAN band tiles LENGTH MIDI SOURCE FOLLOW (MIDI is a 17-name list 1..16, OFF) + status line + music row of the viewed page + button icons + keybed footer; the setup cue clears when the channel gets a MIDI channel', 'knob_band': 'K1 LENGTH, K2 MIDI, K3 SOURCE, K4 FOLLOW', 'keys': 'white = steps (as PERF); black as PERF except CHN (D#4) = BACK (arrow glyph)', 'cue_strip': 'K2 = MIDI   CHN = BACK', 'latched': 'yes'}
