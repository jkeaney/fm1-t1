# Performance-first control design (P1-014)

Task P1-014 (owner challenge, eighth round, 2026-11-10). Status: **for owner review** (gate `human-review`; no approval is
recorded here). Design only: no firmware, no tests. **Revision pass 2026-11-10** after review: honest L scores, printed-word
variant B2, fast pitch entry, the HOME fix, the undefined states filled. The final scheme is **B2**.

> Change note (P1-017, sequencer UX lock; for owner review): the **B2 button layout stays** (decision 55). Superseded by [SEQUENCER_UX.md](SEQUENCER_UX.md): the step band and "hold a step + press another white key = pitch" (replaced by a latched NOTE EDIT page: K1 PITCH, K2 GATE, K3 VELO, K4 RATCH, OCT octave, the layer buttons toggle the step's fields); scenes A / B and tap tempo (removed; C#5 / D#5 are PG- / PG+, D#4 is CHAN); the R1-R4 reserve of 3.3 (now pages, LENGTH, the edit layer and force sensing); GLO hold (now the SCALE page); HOME stays as decided (decisions 57 / 61: tap = home, hold + touch = HELP, release after 0.7 s untouched = MENU; the P1-017 first draft's HOME-tap HELP page was reverted in revision 1); the RESET layer becomes a latched one-shot page (hold EDIT, then touch); the ROW 1 / ROW 2 assignment (3B re-lock). The rubric gains the high bar (performance >= 11, setup >= 10, no sub-score below 2, at most 2 gestures), under which every hold-while-turning gesture (input 1) of this document fails; that is why holds now open pages. **P1-017 revision 1 restates the L rule of 7.1 openly** (SEQUENCER_UX 8.1, owner question 72): a role reached through a hold is L2 when the hold is named permanently on its starting page (the new `h:` sub-labels) and opens a labelled page; otherwise L1 as below.

> Change note (revision): the first version chose scheme B ("Rows": dice in layer order across the top row). Review found
> that B scored L3 for five dice because the cue strip "teaches" them, that REC = MUTATE overrode the clearest printed word,
> that the white keys could no longer play a pitch, and that HOME hold opened the MENU before HELP could be asked. This version
> (1) rescored L honestly (a printed word that contradicts the role is L2, section 7.1), (2) adds **B2**, where every button
> keeps its printed word wherever the sequencer can do what the word says, and picks B2 over A, B and C (section 8),
> (3) adds **REC = record** (play the keys, they write pitches; also the audition path) and **hold a step key + press another
> white key = that scale degree**, (4) fixes HOME (menu on release, HELP while held, section 5.3), (5) defines every state the
> reviewer listed as undefined (section 5.6) and extends the confusing-states list (5.5). **MUTATE is removed**: as a gesture it
> scored input 2 and L1-2 (a hold that quietly undoes itself is not guessable), and dice + EDIT (UNDO) already do "try it, take it back".
> Owner questions 55 to 59 keep their numbers; the revision adds 60 to 64. It supersedes the layout of
> FULL_CONTROL.md section 1 / UX_REVIEW.md section 1 / CONTROL_MAP.md as approved with P1-013, and answers owner questions 36 and
> 49. The edits it forces are applied to [control_map.tsv](control_map.tsv), [CONTROL_MAP.md](CONTROL_MAP.md),
> [FULL_CONTROL.md](FULL_CONTROL.md), [UX_REVIEW.md](UX_REVIEW.md), [ALGORITHM_SET.md](ALGORITHM_SET.md) and
> [ADR-0003](../project/decisions/ADR-0003-sequencer-core.md), each with a change note at its top.

Inputs: VISION.md, ADR-0004 (+ flexibility addendum), OWNER_QUESTIONS.md (all rounds, especially 36, 48-52, the sixth-round
"Shift" and the eighth-round "Perf"), FULL_CONTROL.md, UX_REVIEW.md (rubric 3.1), CONTROL_MAP.md / control_map.tsv,
CONTROL_INVENTORY.md, ALGORITHM_SET.md and section 6 of the four algorithm docs, ADR-0003, SYSTEM_MAP.md,
`firmware/src/panel.c`, and the panel photo `docs/controls.jpg` (looked at, including a 2x crop of the key labels).

**What the photo says (it sets learnability).** The printing is the only thing a first-time user reads before touching
the device, so a button that does what its word says needs no teaching. Printed on the panel:

| Printed | Photo (stock meaning) | What this sequencer can do with that word | Fit |
| --- | --- | --- | --- |
| `SELECT` `ALGORITHM` `PRESETS` `MASTER` | tempo (stock), track, sound, volume | channel, algorithm, preset, volume (all as printed) | strong (SELECT moves from tempo to channel; the screen names it) |
| `KNOB 1..4` | the four columns of the page | the four morphs of the live row | strong |
| `PLAY/STOP` `SAVE` `HOME` | start/stop; presets, undo the last load by holding; home, menu by holding | start/stop; save, hold = undo a load; home, hold = menu | strong (the stock holds are kept) |
| `REC` | "arm the track to record" | **record**: play pitches in, they write | strong |
| `GLO` | "mixer, global, system" | the **global** feel page (gate, division, swing, tempo, scale, MIDI, length) = SET | strong |
| `SCL` | "scale, chord keys; hold: a key sets the root" | the **pitch** layer (dice NOTES: new pitches inside the scale) | fair (the key and scale live on SET) |
| `SEQ` | "sequencer: steps, pattern" | **steps**: dice HITS (where the steps fall) | fair |
| `FX` | "effects and slicer" | an effect on the whole pattern: dice ALL | fair |
| `ENV` | "envelope" | the **accent** layer (accents drive the envelope on an acid line) | fair |
| `LFO` | "LFO, modulation" | the **shape** layer (movement over the bar: drift, octave moves) | fair |
| `ARP` | "arpeggiator; flashes on the beat" | the **articulation** layer (gate, slide, ratchet: repeating figures) | fair |
| `EDIT` | "engine parameters" | edit = **undo / reset** (the approved scheme already used it) | weak |
| `OCT-` `OCT+` | octave down / up (stock: back / do it in dialogs) | the **ROW 1 / ROW 2** switch (owner decision 48); octave is on SET / K4 | **override** |
| Black keys `OP1 OP2 OP3 \| OP4 OP5 \| OP6 PIT GLD \| MONO POLY \| (blank)` | FM operators and voice words | no sequencer meaning, except `OP1..OP4` = "part n" = mute channel n and `MONO` = solo | weak; the legend carries them |

"Fair" fits are L2, not L3: a first-time user cannot *guess* that ENV means accent. They can read it (the screen draws the
button's role on the button's picture, and HOME-held HELP explains it), which is exactly the rubric's L2. The only way to a
true L3 is a printed word that already means the job (PLAY, SAVE, HOME, GLO, REC, SELECT, ALGORITHM, PRESETS, KNOB 1-4).

## 0. The scheme in one paragraph

**One principle: the word on the control says what it does, in sequencer terms; where there is no word, the panel is split by
what the control does to the music.** The button block, in panel order:

| Zone | Principle | Controls |
| --- | --- | --- |
| Top row `FX SCL ENV LFO EDIT GLO` | the effect and the five printed words that name a layer, then undo and global | FX dice ALL, SCL dice NOTES, ENV dice ACCNT, LFO dice SHAPE, EDIT = UNDO (hold: RESET), GLO = SET |
| Bottom row `HOME SAVE ARP SEQ PLAY REC` | home, keep, the two layers named by sequencer words, start, record | HOME (back / menu / help), SAVE (keep, hold = undo a load), ARP dice ARTIC, SEQ dice HITS, PLAY, **REC = record** |
| Four knobs `KNOB 1..4` + `OCT-` `OCT+` | **morph**: two rows of four, the row buttons sit left of the knobs | unchanged from P1-013 |
| Encoders `SELECT ALGORITHM PRESETS` | what is printed | channel, algorithm, preset |
| 16 white keys | **the 16 steps of the viewed page** (no STEP mode); in REC they play pitches | tap = pin, hold = select the step, hold + another white key = its pitch |
| 11 black keys | **performance moves** (in REC they play pitches too) | 4 mutes, PAGE, SOLO, FILL, ROLL, SCENE A, SCENE B, RESTART |

The five layer dice (SCL NOTES, SEQ HITS, ENV ACCNT, ARP ARTIC, LFO SHAPE) and the undo (EDIT) sit exactly where the approved
P1-012 / P1-013 scheme put them. That is not a coincidence: P1-012 chose them from the printed words. The first P1-014 version
(B) moved them into layer order for the sake of one picture and fought the printing; B2 undoes that and keeps everything else
that was good in B (the keys, the reset layer, the cue strip, first run, escape, HELP, PAGE, scenes). Nothing needs a mode
except two latched states with a banner, SET and REC, both left by one tap of HOME.

## 1. Control accounting table

Every physical control, its role in PERF, and the single sentence that explains it. "PERF" is the resting main screen.
All 49 controls (MASTER, 3 encoders, 4 knobs, 14 buttons, 27 keys) have a role; the only intentional blanks are listed
under the table. The per-context detail (six contexts, 294 rows) is control_map.tsv.

| Control (printed) | Principle | PERF role (tap / hold / combo) | On-screen label |
| --- | --- | --- | --- |
| `MASTER` pot | what is printed | volume (analogue; no MIDI effect) | none (a pot) |
| `SELECT` | what is printed | turn: select channel CH1..CH4 | `CH n` cell inverted |
| `ALGORITHM` | what is printed | turn: algorithm of the selected channel | algorithm name |
| `PRESETS` | what is printed | turn: preset slot; a filled slot loads at the next bar | header `Pnn`, `Pnn>` queued |
| `KNOB 1..4` | morph | turn: the live row (ROW 1 DENS FORM PITCH SPICE; ROW 2 START ARTIC DYNAMICS READ); held step key: step band row 1 `NOTE ACC SLIDE RATCH`, row 2 `TIE MUTE PROB OCT`; SET: a row of four settings; REC: `CURSOR ACC SLIDE OCT`; held EDIT: reset that morph | two rows of cells, live row filled |
| `FX` | effect: all layers | press: DICE ALL | `ALL` |
| `SCL` | pitch layer | press: dice NOTES | `NOTES` + algorithm sub-label |
| `ENV` | accent layer | press: dice ACCNT | `ACCNT` + sub |
| `LFO` | shape layer | press: dice SHAPE (dim if absent) | `SHAPE` + sub |
| `EDIT` | take back | tap: UNDO / REDO (dice, pin edit, written note, reset); hold: the RESET layer | `UNDO` |
| `GLO` | global | tap: SET page (latched); hold: while held | `SET` |
| `HOME` | home | tap: back to this screen from anywhere; hold: `touch = HELP, let go = MENU`; release after 0.7 s with nothing touched: MENU | `HOME`, `BACK` |
| `SAVE` | keep | tap: save to the slot in the header; hold 0.7 s: undo a load | `SAVE Pnn` |
| `ARP` | articulation layer | press: dice ARTIC (dim if absent) | `ARTIC` + sub |
| `SEQ` | steps layer | press: dice HITS | `HITS` + sub |
| `PLAY` | transport | start / stop | `PLAY / STOP` (LED) |
| `REC` | record | tap: arm RECORD (latched, LED lit); tap again or HOME: leave | `REC` |
| `OCT-` `OCT+` | rows of four (override) | tap: latch ROW 1 / ROW 2; hold and turn: momentary; the same pair picks the step band row while a step key is held, and the SET row | `ROW 1`, `ROW 2`, `STEP 1`, `STEP 2` |
| White keys F3..G5 (16) | steps of the viewed page (R2) | press: pin step n of the page (audition); tap a pinned step: clear; hold: select it (R3), then knobs edit it and **another white key sets its pitch**; with PAGE held: end the pattern here (R1); **[R4 slot: per-key expression]** | lane cell n lights; `step n` |
| Black `F#3 G#3 A#3 C#4` (printed OP1..OP4) | moves | tap: mute / unmute CH1..CH4 | `MUTE1..4` |
| Black `D#4` (OP5) | moves | tap: **PAGE** swap (R2); hold: the next white key sets the pattern length (R1) | `PAGE`, header `PG 2/4` |
| Black `F#4` (OP6) | moves | hold: SOLO the selected channel | `SOLO` |
| Black `G#4` (PIT) | moves | hold: FILL (maximum density) | `FILL` |
| Black `A#4` (GLD) | moves | hold: ROLL (every step repeats) | `ROLL` |
| Black `C#5` (MONO) | moves | tap: recall scene A at the next bar; hold 0.7 s: store scene A | `SCN A` |
| Black `D#5` (POLY) | moves | tap: recall scene B; hold 0.7 s: store scene B | `SCN B` |
| Black `F#5` | moves | press: restart all channels from step 1 | `RESTRT` |

**Intentional blanks (not dead, not ambiguous):**

- The five layer dice and FX have **no hold role** (nothing to learn but "press"), so they act on press.
- In **MENU** all 27 keys are inert (the black keys too: B inconsistently let them mute while a setting was changed). The MENU shows `close` on the buttons.
- `GLO` is inert while a step key or EDIT is held or while recording (one held modifier at most); the cue strip says so (`FINISH THE STEP`, `REC: tap REC to finish`).
- **In REC** the knob band is one row (`CURSOR ACC SLIDE OCT`), OCT-/OCT+, GLO and PRESETS are inert and say why in a toast; the 27 keys are notes.
- **Reserved slots (ninth-round requirements R1-R4, interface for P1-016 and P1-015, section 3.3):** `PAGE` (D#4), pattern `LENGTH` (hold PAGE + key; SET row 2 K4), per-step property rows (step band), and the `[R4]` keybed-expression slot on the white keys. Their control and screen contracts are fixed here; their ranges, storage and algorithm interaction are not.
- There are no live-note keys in PERF (see section 3.1 for why, and REC for where they went).

## 2. Panel buttons aligned to the algorithm controls

The rule, which holds in every algorithm: **a layer button never changes meaning with the algorithm, only its sub-label (and its
dimming) does; the layer order of the screen's button picture is the panel's own order, so a dice cell sits where its button is.**

| Button | Layer | ACID sub-label | EUCLID | SNAKE | STAGE | Reset (EDIT held) |
| --- | --- | --- | --- | --- | --- | --- |
| `FX` | ALL | all 6 seeds | all 4 | all 4 | all 5 | whole algorithm to factory |
| `SCL` | NOTES | `NOTES` | `NOTES` | `NOTES` | `NOTES` | NOTES seed to factory |
| `SEQ` | HITS | `HITS` | `ROTATE` | `GATES` | `RESTS` | HITS seed to factory |
| `ENV` | ACCNT | `ACCNT` | `ACCNT` | `VELO` | `VELO` | ACCNT seed to factory |
| `ARP` | ARTIC | `GT+SLD` | `RATCH` | dim `-` | `RT+HLD` | ARTIC seed to factory |
| `LFO` | SHAPE | `OCTAVE` | dim `-` | `DRIFT` | `LONG` | SHAPE seed to factory |

(Sub-labels from ALGORITHM_SET 4.4; the layer-to-seed matrix 4.2 is unchanged.) The cue strip names the dice only as they are used and HELP explains any of them, which is why their L is 2 and not 3 (section 7).

`EDIT` = UNDO for everything the dice, the pins, the notes written in REC and the reset can do (ADR-0003 `UndoRec`, extended), `GLO` = the one setup page, `REC` = record. In every algorithm the same button does the same job: the dice change the *content* (re-roll), the knobs change the *amount* (morph), `EDIT` takes the last change back, `EDIT` held resets. No algorithm-specific button exists.

**Why this order (evidence).** The first P1-014 version put the layers in list order (`NOTES HITS ACCNT ARTIC SHAPE`) across the top row. Scored honestly that gained nothing: the position still contradicted every printed word (SCL / ENV / LFO / EDIT / GLO), so no dice earned L3, and it cost the printed fits GLO = global, SEQ = steps, ARP = repeats. B2 keeps the P1-012 positions, which map the printing onto the layers (scale -> notes, sequencer -> steps, envelope -> accent, arpeggiator -> articulation, LFO -> shape), and uses the freed GLO and REC for what they say.

## 3. Keyboard: steps and performance moves

### 3.1 White keys: the steps of the viewed page (outside REC)

- **Key n = step n of the viewed page** (R2): step number `(page - 1) x 16 + n`. With a pattern of 16 steps or fewer there is one page and nothing to swap. Keys past the pattern length are drawn dim and are inert (toast `PAST THE END: hold PAGE + key`).
- **Press** key n: if the step is not pinned, it is pinned now and **sounds** (the note the algorithm generated there, or the root if it was silent). A touch **under 30 ms is ignored** (brush guard, table value). If it is pinned, release within 0.4 s without turning a knob, pressing OCT or pressing a second white key clears it (tap = toggle).
- **Hold** key n = **select the step** (R3). The step band replaces the knob band and has two rows of four, the same model as the morph rows, with `OCT-` = row 1 and `OCT+` = row 2:

| Step band | K1 | K2 | K3 | K4 |
| --- | --- | --- | --- | --- |
| Row 1 (`STEP 1`, the default on hold) | `NOTE` (one scale degree per detent; **each detent sounds**) | `ACC` (accent) `ON/OFF` | `SLIDE` `ON/OFF` | `RATCH` `1x..4x` |
| Row 2 (`STEP 2`, tap or hold `OCT+`) | `TIE` `ON/OFF` | `MUTE` `ON/OFF` | `PROB` `100 75 50 25` | `OCT` `-2..+2` |

- **Fast pitch entry, one step: hold the step key, press another white key = that scale degree.** Key 1 is the root, key n is scale degree n-1 above it (so 16 keys span the scale's first 15 degrees; a pentatonic scale reaches three octaves). The note **sounds** (the audition), replaces the step's pitch, and the screen shows `STEP 5 = G2`. Press a different key to try another pitch; release the held key to keep the last one; UNDO tap restores the old one. The held key is the step; a later press while it is held is a pitch, never a second step. **Multi-select is dropped** (the first version let all held keys receive one edit; one step at a time is what the pitch gesture needs, and a selection of several steps is a setup rarity). Octave: OCT+ then K4 `OCT`, or the REC K4 below.
- **Fast pitch entry, a whole tune: REC** (3.1b). Gesture count for 16 pitched steps: REC path 17 (tap REC, play 16 keys) plus 1 to leave; hold-and-press path 48 (16 x hold, press, release); B's hold + K1 path was about 80 (16 x hold, 2-4 detents, release). Scored in section 7 (F05, F53, F55).
- **Pins ignore OFFSET and DIR** (core overlay keyed to the step position): a pin on key 5 stays on key 5.
- **Clear:** tap a pinned key; hold EDIT and press the key (clears that step's record); hold EDIT and press REC (clears all). EDIT tap restores.
- `K1 = 0` (`KEYS ONLY`) plays only the pins (owner decision 35).
- **Stopped vs playing.** Stopped: a press sounds the step once on the selected channel (audition, MIDI + preview). Playing: same, and the record takes effect at the next pass of that step.
- **Why white keys are not live notes in PERF.** The product is a sequencer (VISION rule 2) and a first press must have a visible result (F26 L2 in the approved scheme: a live note that was then lost). Cost: a hand brushing a white key sets a pin. Rescue: the pin is outlined on the lane, EDIT tap removes it, tapping it again removes it, the 30 ms guard drops the lightest touches, and a `KEYS LOCK` menu row (a table row, owner question 56) is available if hardware testing shows brushing is a problem. **The workflow cost the first version carried (no way to play or hear a pitch) is gone: REC plays them.**

### 3.1b REC: play the keys, they write (new context `REC`)

- **Enter / leave:** tap `REC` (LED lit, banner `REC`, cue strip `REC: PLAY KEYS TO WRITE  REC=done`); tap REC or HOME to leave. A latched state with a banner, like SET; it has the same escape.
- **The 27 keys play real pitches F3..G5** (white and black, chromatic: a note outside the scale is allowed here, which the scale-degree gestures cannot do), plus the global OCTAVE and the REC `OCT` (K4, -2..+2), on the selected channel's MIDI channel and the preview synth. Key up = note off. This is the **audition path**: every key sounds before and as it is kept.
- **Writing.** Each key press writes a pin: **stopped**, at the cursor step (shown on the lane, K1 `CURSOR` moves it, wraps at the pattern length; it advances after each write, and the step sounds as the cursor moves); **playing**, at the step nearest the playhead (so a bassline is played over the loop and lands on the grid; the cursor follows the playhead). A pin carries the `ACC` and `SLIDE` flags of K2 / K3 at that moment (`ON/OFF`, set before playing the next key). A press under 30 ms is ignored.
- **UNDO:** EDIT tap takes the last written note back (the cursor steps back to it). The undo record holds one change (owner decision 46), so a second tap redoes it; earlier notes are fixed with the step band. **Rests:** turn K1 past a step and it keeps whatever it had; to silence one, leave REC and use `MUTE` on step band row 2.
- **Inert in REC:** `OCT-` `OCT+` (one row), `GLO`, `PRESETS`, hold EDIT (leave REC first); each says why in a toast. The black keys are notes (moves are unavailable while recording; one tap of REC brings them back).
- Printed-word check: REC = "arm the track to record" on the photo; this is the same word with the same meaning, so it earns L3.

### 3.2 Black keys: performance moves (outside REC)

Layout follows the physical groups of black keys (3, 2, 3, 2, 1) and the printing `OP1 OP2 OP3 | OP4 OP5 | OP6 PIT GLD | MONO POLY | blank`: the first four keys are the channels' mutes (OP1..OP4 read as "part 1..4"), `D#4` (OP5) is `PAGE`, then the three **hold** moves together `F#4 G#4 A#4` (SOLO FILL ROLL: "press and keep holding"), the two scenes `C#5 D#5`, and the lone last black key `F#5` RESTART (the easiest key to find blind: the last one). PAGE sits at the left end of the keybed next to the first white keys because it changes what those keys mean. **Printing used:** OP1..OP4 only. `MONO` would read as SOLO, but swapping SOLO onto `C#5` splits the scene pair (A and B belong side by side); `PIT` and `GLD` (pitch, glide) are FM words with no sequencer role of their own. These keys therefore score L2 (visible, must be discovered).

| Key | Label | Gesture | Exact behaviour | State it needs |
| --- | --- | --- | --- | --- |
| F#3 G#3 A#3 C#4 | `MUTE1..4` | tap | latched mute; muted cell dimmed `M`; phase keeps running | existing `ChanSave.muted` |
| D#4 | `PAGE` | tap / hold | tap: view the next page of the selected channel (wraps 1..M); hold: the next white key pressed ends the pattern at that step (`LENGTH`); inert while a step key is held (toast `FINISH THE STEP`) | UI `view_page` 1 B; length is P1-016's |
| F#4 | `SOLO` | hold | only the selected channel sounds while held; release restores every mute. Phase keeps running. A mute tapped while SOLO is held toggles the latch silently and is heard on release | 1 bit `sq_perf` read by the ISR |
| G#4 | `FILL` | hold | the selected channel's K1 morph goes to its maximum (every step / stage plays); release restores the value. **K1 turned while FILL is held edits the saved value** (the cell shows `FILL` and the maximum; the new value returns on release) | 1 B saved morph + flag |
| A#4 | `ROLL` | hold | every step of the selected channel plays its note twice (ratchet x2) while held; release stops. K1 turned while ROLL is held is an ordinary morph | 1 bit `sq_perf`, one `max(rep,2)` in ADR-0003 7.3 step 6 |
| C#5 | `SCN A` | tap / hold 0.7 s | tap: recall scene A at the next bar (header `A>`); hold 0.7 s: store the whole current state into A; toast `A STORED`. Empty: toast `A EMPTY: hold to store` | one preset-sized pool copy each |
| D#5 | `SCN B` | tap / hold 0.7 s | as A | one pool copy |
| F#5 | `RESTRT` | tap | all channels to step 1 at the next step | existing |

**Tap tempo is given up as a key** (it held `D#4`): tempo is on SET K4 (`BPM`) and a live tempo change is rare; PAGE needs the key (R2). Scored: F17 3/2/3/3 = 11 (setup, pass).

**Legend, always on screen.** A 12 px band under the button picture shows the 11 black keys as 11 cells in the physical 3-2-3-2-1 grouping, each with a label of at most 6 characters (P2-003 verifies with `text_w(&AF_S)` that `RESTRT` fits the 21 px cell pitch; if not, the band uses a 2-line cell and the sequence band gives up 8 px). A cell is inverted while its key is held; `M` cells are dimmed for muted channels. In REC the band reads `KEYS = NOTES`. The 16 white keys need no legend: **the step lane above is the legend**, because lane cell n is drawn in the same left-to-right order as white key n, and a lit lane cell is a pinned key.

### 3.3 Pattern length, pages, per-step performance and keybed expression (ninth round R1-R4): the interface P1-016 fills

These four are reserved at the control level. The gestures, labels and screen contract are fixed and scored; ranges, storage, and the algorithm side are **not** (P1-016 integrates them; P1-015 researches R4). Nothing below edits ADR-0003 or the algorithm docs.

| Requirement | Control reserved | Gesture | Screen contract | P1-016 / P1-015 fills |
| --- | --- | --- | --- | --- |
| **R1** pattern length per channel (amends VISION rule 3: MIDI channel, algorithm, **length**) | `LENGTH`: SET row 2 K4 (slot reserved); and the fast gesture **hold `PAGE`, press the last step key** (TR-style "last step") | hold D#4 + press white key n of the viewed page: the pattern ends at step `(page-1) x 16 + n` | channel cell shows `n` steps if not 16; lane header `LEN 24`; keys past the end dim | range (assumed 1..64), default (16), relation to the algorithm `LEN` / `TOTAL` morph (K2 FORM), step storage, preset bytes |
| **R2** pages | `PAGE` = black D#4 | tap: next page (wraps). The view never moves by itself; the playhead is never moved by paging | lane header `PG 2/4` (page N of M); a **page map** of M small cells (max 4) with the viewed page outlined and the playhead's page filled; if the playhead is on another page, a marker `> P3` appears at the lane edge; on the viewed page the playhead cell is inverted | max pages, whether paging while stopped follows the cursor, how `OFFSET` and `DIR` wrap across pages |
| **R3** per-step performance | step select = **hold a white key**; step band (3.1) | hold key, turn K1..K4 (row 1), tap `OCT+` for row 2: `NOTE ACC SLIDE RATCH` / `TIE MUTE PROB OCT` | band replaces the knob band (`STEP 1`/`STEP 2` in the OCT cells), the lane cell is outlined, a pin shows its note; a flag shows a small glyph on the lane cell (accent `!`, slide `~`, tie `-`, ratchet `x2`, mute `m`, probability `%`) | the per-step record, which flags exist, how PROB draws deterministically, how an algorithm-generated step becomes a record, preset bytes |
| **R4** keybed expression (MPE-like) | **slot `[R4]` on the 16 white keys** (no gesture promised) | if P1-015 confirms per-key pressure, position or velocity-after-touch: pressing a held step key harder / sliding writes the **armed** step property (the last cell turned in the step band) | the armed cell is outlined in the step band; the value reads live | hardware capability, the axes available, ranges, whether it applies live (playing a step) or edit-only. **Floor:** if it cannot score P >= 11 with no sub-score under 2 it stays out of v1 and the step band remains the only route |

Scores for R1-R3 are in section 7 (F49 PAGE 11; F50 LENGTH 9; F51 / F42 per-step 9; F52 page / length / playhead cue 12). **Per-step performance is scored as step editing (setup class, input 1: hold a step key and turn a knob)**, because a latch that would give input 2 (a latched step cursor) is a mode the scheme avoids (scheme C scores it, section 8). If the owner wants per-step properties to count as a performance function, the control change is one row of control_map.tsv (give the step band a latched cursor) and the cost is a mode.


## 4. Reset gestures and the momentary morph

### 4.1 Reset (owner question 49, now designed)

Reset lives on the UNDO button (`EDIT`) because UNDO already means "take back": the performer who has lost the sound reaches for the
same button.

- **Tap EDIT** = undo / redo (dice, pin edits, written notes, resets). One record per channel (owner decision 46, extended to carry morphs).
- **Hold EDIT** (0.4 s, or at once if any control is pressed meanwhile; the existing layer behaviour, CONTROL_INVENTORY 3) = the **RESET layer**. Cells tag
  `RST`, buttons relabel, the cue strip reads `RESET: knob=one OCT=row FX=all`. HELP can be asked in it (HOME held, then a control).

| In the RESET layer | Gesture | Resets | To |
| --- | --- | --- | --- |
| **Single morph** | turn that knob (any detent) | that one morph of the live row of the selected channel | the morph's default (FULL_CONTROL tables) |
| **Row** | tap `OCT-` or `OCT+` | the four morphs of ROW 1 / ROW 2 | defaults |
| **Algorithm** | tap `FX` | all 8 morphs **and** all seeds of the selected channel's algorithm | defaults and factory seeds (pins and flags kept) |
| **One layer** | tap a dice button (`SCL SEQ ENV ARP LFO`) | that layer's seeds | factory seeds |
| **One step / all pins** | press a white key / press `REC` | that pin and its flags / every pin of the channel | no pin |

Every reset is **one UNDO tap away from being taken back** (`UndoRec` stores the other 8 morphs and the touched step record).
Factory seeds are the existing default-seed rule (`AlgoDesc.seed_k[]`, ADR-0003 section 4): no new table.
Scores: section 7 (F43 F44 F45 are setup/recovery functions at 9/12; input is 1 because a hold plus a control is "modifier + control" in the rubric, and no latch can make it 2 without a second hold).
Row-reset by double-tap of OCT was scored (3/2/3/2 = 10) and rejected: two quick OCT taps are a normal way to be sure of the row, so it would reset by accident.

### 4.2 Momentary morph (owner question 36) and MUTATE

Felucca's "LFO held, knob moves snap back" scores input 1 (a hold plus a turn is "modifier + control") and a
performance function needs input >= 2, so a *hold + turn* gesture does not earn a place as a performance function. The goal behind it (try a change, then snap back) is met by:

- **FILL** (hold `G#4`): the K1 morph goes to maximum and returns. **ROLL** and **SOLO**: the other two momentary moves.
- **Dice, then EDIT (UNDO) tap:** try a re-roll, tap EDIT to take it back (and tap again to bring it back). Two single presses, both score 11 or better.
- **MUTATE is removed** (the first version put it on REC, hold). Scored honestly it is I2 (a hold with a timer) and L1-2: its defining behaviour, the snap-back on release, cannot be guessed, and putting it on REC overrode the clearest printed word. Its 24 B of RAM are returned. Question 36 stays closed by FILL / ROLL / SOLO and the dice + UNDO pair; a hold-to-audition mutation can return after the first hardware test if the owner misses it (it needs a free control: today there is none, see owner question 62).

## 5. Never lost, no manual

Design rule: **at every moment the screen answers four questions without a manual: where am I, what is playing, what does each control do right now, how do I get back.** Four always-on elements answer them.

### 5.1 The always-on cue strip

A 12 px line at the very bottom of the main screen (AF_S, at most 34 characters, one line, never scrolling). It shows the one most useful instruction for the current state; the first matching row wins. It is drawn in every context except `MENU`. It does not replace the button picture (which labels all 12 buttons) or the knob band (which labels all 8 morphs): the strip says what to do **next** or what the held control does.

| Priority | State | Cue strip text |
| --- | --- | --- |
| 1 | HOME held, a control touched (HELP) | `<LABEL>: <one line>` (for example `HITS: re-roll where notes fall`) |
| 2 | HOME held, nothing touched yet | `HOME: touch=HELP  let go=MENU` |
| 3 | RESET layer | `RESET: knob=one OCT=row FX=all` |
| 4 | REC armed | `REC: PLAY KEYS TO WRITE  REC=done` |
| 5 | A step key held | `STEP n: KEY=PITCH  K=EDIT` |
| 6 | PAGE held | `PAGE: press a key = pattern ends` |
| 7 | SOLO / FILL / ROLL held | `SOLO CH2: let go to restore` (same shape) |
| 8 | SET page, row 2 | `SET: press a key = KEY  HOME=back` |
| 9 | SET page, row 1 | `SET: OCT-/OCT+ = row  HOME=back` |
| 10 | A preset or scene is queued | `LOADS AT NEXT BAR  SAVE hold=undo` |
| 11 | Just after any dice / reset | `EDIT takes it back` (2 s) |
| 12 | First-run step (5.2) | the step's instruction |
| 13 | Stopped, nothing else | `PLAY: start   K1-K4: morph` |
| 14 | Playing, ROW 1 | `FX=dice all  EDIT=undo  OCT+=row 2` |
| 15 | Playing, ROW 2 | `ROW 2: OCT- = back to row 1` |

Texts are table data (ADR-0004); the priorities and the 34-character limit are the contract. The OCT+ = row 2 hint also shows while stopped for the first two minutes after power-on (row 13 alternates with it every 3 s).

### 5.2 First run (shown once, dismissed by any control)

Power-on boots into a working sequencer (owner decision 32b: CH1 ACID, unmuted, MIDI 1, plays on PLAY). A 5-step **guided strip** uses the cue strip itself; it never blocks anything.

| Step | Cue strip | Advances when | The user learns |
| --- | --- | --- | --- |
| 1 | `PRESS PLAY` | PLAY pressed | the device makes sound at once |
| 2 | `TURN K1 TO MORPH IT` | any knob turned | knobs change the pattern |
| 3 | `PRESS FX: NEW PATTERN` | any dice button pressed | the buttons re-roll; EDIT takes it back |
| 4 | `OCT+ = MORE KNOBS (ROW 2)` | OCT+ pressed | the second knob row (the one L2 control that must be taught) |
| 5 | `WHITE KEYS=STEPS  REC=PLAY NOTES` | any white key or REC pressed | keys are the steps; REC plays notes in |

Rules: **any other control also counts as dismissing** (the strip clears and the main screen is normal); HOME tap skips it; it is shown until completed or skipped **once per device** (one bit in the persisted settings word `ui_prefs2`, 0 B RAM, set at completion or skip); a settings reset clears the bit. RAM: 1 B progress counter in the UI allowance. It never repeats after the bit is set.

### 5.3 One escape, undo everywhere, "what does this do", and the HOME rule

- **Escape: tap `HOME`.** From SET, REC, RESET, a held step, the first-run strip and the MENU it returns to the main screen. In PERF it shows the help line for 3 s (so pressing it never feels dead). Layers are momentary anyway; the escape exists for the two latched states (SET, REC) and the MENU.
- **Undo is reachable everywhere except the MENU:** `EDIT` tap works in PERF, a held step, SET, REC (takes back the last written note) and RESET. It does one thing in every context: take back the last dice, dice-all, pin, written note, step edit or reset; again = redo. After a preset load there is nothing for it to undo (toast `NO UNDO: SAVE hold = undo load`).
- **HOME: one rule, three outcomes (resolves the HELP-versus-MENU conflict).** HOME acts when it is **released**, never on a timer, and the screen says at once what is possible:
  1. **Tap** (released under 0.7 s): back to the main screen from anywhere.
  2. **Hold and touch or turn any control, at any time while HOME is down** (no 0.7 s window): HELP for that control. The control does nothing; the strip and a toast explain it; further touches while HOME stays down explain those too; releasing HOME then does nothing at all.
  3. **Hold 0.7 s or longer with nothing touched, then release:** open the MENU.
  The strip shows `HOME: touch=HELP  let go=MENU` the moment HOME is pressed, so a slow user is told both options before either can happen. The old rule (MENU opens at 0.7 s while the finger is still down) is dropped; the price is that the menu opens on release (about 0.1 s later than stock). Scored: F29 3/2/3/3 = 11 (the stock photo prints `HOME hold: Menu`), F46 3/1/3/2 = 9.
- **What does this do (touch-to-learn):** hold HOME, then press or turn any control: the cue strip and a toast explain it (`K3 PITCH: how wide pitches range`). Scored F46 = 9 (setup, pass, earns its place: it is the only plain-words route for the controls whose printing differs from their role, and the HOME strip advertises it). Cost: a 49-entry one-line help table (about 1.5 KB flash) and a 1-B "help held" bit.
- **Why not hold-any-control-to-learn:** the dice, mutes and keys act on press, so a "touch" would already have changed the music. Hold HOME is the only form that changes nothing.

### 5.4 Naming and iconography rules

1. A label is at most 6 characters, capitals, no punctuation but `+` `-` `/` and digits (`RST R1`, `SCN A`, `GT+SLD`).
2. **Layer names are nouns of the music** (`NOTES HITS ACCNT ARTIC SHAPE`); **moves are one verb or noun the performer would say** (`FILL ROLL SOLO PAGE REC`); resets are `RST` + a short target; scenes are `SCN A` / `SCN B`.
3. The same word means the same thing everywhere: `ALL` = every layer or every channel it is shown on; `ROW 1` / `ROW 2` appear on the OCT cells everywhere (`SET 1` / `STEP 1` only while those bands are up).
4. Tap = set or latch, hold = momentary, in every context (mutes and REC latch on a tap; SOLO, FILL, ROLL, PAGE-length are held). No control mixes a tap role and a different hold role except EDIT (hold = RESET), GLO (hold = SET while held), SAVE, HOME and the scene keys, each of which carries a sub-label naming its hold (`UNDO / RESET`).
5. A dimmed label with `-` means "this algorithm has no such layer" and nothing else.
6. Icons: only three glyphs are used beyond text: a filled dot for a lit/held move, the lane-cell flag glyphs of section 3.3, and the LED on PLAY and REC.
7. **The printed word is kept whenever it can be** (section 0 table); a label never contradicts a printed word without a row in the override table (OCT-/OCT+ only) and a cue that names the override.

### 5.5 Every state a first-time user could be confused in, and the cue that rescues them

| # | Confusing state | Rescuing cue |
| --- | --- | --- |
| 1 | "I pressed PLAY and hear nothing" (muted channel, MIDI OFF, K1 = 0) | the channel cell shows `M` or `OFF`; the strip says `UNMUTE: F#3` / `KEYS ONLY: tap white keys`; the K1 cell reads `KEYS ONLY` |
| 2 | "The knobs do nothing" (ROW 2 is latched and I turned a ROW 1 knob; or the wrong channel is selected) | gutter digit and filled row show which row is live; the OCT+ LED is lit on ROW 2; strip `ROW 2: OCT- = back to row 1`; the selected channel cell is inverted |
| 3 | "A step lit up and I did not mean it" (brushed a white key, or a touch beyond row 3 of this table) | the cell is outlined; the strip says `EDIT takes it back`; touches under 30 ms are ignored; `KEYS LOCK` is a menu row if needed (question 56) |
| 4 | "I changed the sound and cannot get back" | EDIT tap; hold EDIT resets (the strip names both) |
| 5 | "I am in some page" (SET latched) | `SET` banner, GLO cell reads `BACK`; strip `HOME=back` |
| 6 | "The buttons are labelled FX, SCL, ENV... but do other things" | most do what the word says (section 0); the button picture on screen shows the real role; hold HOME + a button explains it |
| 7 | "I pressed a preset and nothing happened" (queued to the bar) | header `P03>`, strip `LOADS AT NEXT BAR`; SAVE hold undoes |
| 8 | "A key does nothing" (past the pattern length) | the key is dim; toast `PAST THE END: hold PAGE + key` |
| 9 | "Where is the rest of my pattern?" (more than 16 steps) | lane header `PG 1/4`, page map, `> P3` playhead marker; PAGE |
| 10 | "I pressed REC and the keys play notes instead of setting steps" | banner `REC`, LED lit, strip `REC: PLAY KEYS TO WRITE  REC=done`; tap REC or HOME |
| 11 | "The dice button does nothing" (layer absent) | dimmed label `SHAPE -` and toast `NO SHAPE IN EUCLID` |
| 12 | "Nothing at all on screen changes" (SCREEN OFF / wake) | any control wakes the screen without acting (existing MENU > MIDI SCREEN WAKE) |
| 13 | "I am in the menu" | the menu has its own title; every button reads `close`; all 27 keys are inert; HOME tap leaves |
| 14 | "I held a step key and the knobs changed meaning" | the step band replaces the knob band and the strip says `STEP n: KEY=PITCH  K=EDIT` |
| 15 | "The device is stopped and I pressed a key" | stopped: the key sounds the step once (audition) and the lane shows the pin; strip row 13 `PLAY: start   K1-K4: morph` |
| 16 | "I muted a channel and also soloed" | SOLO wins while it is held (only the selected channel sounds); on release the mutes return exactly; a mute tapped during SOLO toggles silently and shows its `M` cell at once |
| 17 | "I pressed EDIT right after loading a preset and nothing came back" | toast `NO UNDO: SAVE hold = undo load` (a load clears the undo record; the load itself is undone by SAVE hold) |
| 18 | "I held HOME and the menu opened" | the strip said `touch=HELP  let go=MENU` from the first moment; HOME tap leaves the menu |
| 19 | "I pressed the wrong REC pitch" | EDIT tap takes the last written note back; K1 moves the cursor |
| 20 | "Two buttons held, nothing happens" | one modifier at a time: the second control is inert and the strip says `FINISH THE STEP` / `REC: tap REC to finish` / `LET GO FIRST` |

### 5.6 States the first version left undefined, now defined

| State | Behaviour |
| --- | --- |
| EDIT tapped while REC is armed | undoes the last written note (cursor steps back); a hold of EDIT is inert (leave REC first) |
| FILL held and K1 turned | K1 edits the saved value; the cell shows `FILL`; the new value returns on release (ROLL held and K1 turned: an ordinary morph) |
| PAGE held while a step key is held, or a step key pressed while PAGE is held | the first held thing wins: a step key held first = PAGE is inert (toast `FINISH THE STEP`); PAGE held first = the white key sets the length and is not a step edit |
| The MENU is open and a black key is pressed | inert (all 27 keys are inert in MENU); the black-key rows of MENU in the TSV read `inert` |
| SOLO held and a mute tapped | the mute latch toggles silently; it is heard when SOLO is released |
| EDIT tapped after a preset load | nothing to undo (toast); SAVE hold is the only undo of a load |
| A queued preset or scene fires while REC is armed | the load lands at the bar; REC disarms with the toast `REC OFF: LOADED` (a take is never rewritten under the player's hands) |
| PLAY pressed while REC is armed | the transport starts or stops; REC stays armed; playing, a note lands on the step nearest the playhead |
| SELECT turned while REC is armed | the channel changes; the cursor goes to step 1; notes now go to the new channel's MIDI channel |
| Power-on | REC, SET, RESET and HELP are never restored; the device boots in PERF (REC is UI state, not saved) |
| HOME held while the RESET layer is down | HELP wins: touching a control explains it (the layer does nothing); releasing HOME returns to RESET until EDIT is released |
| SAVE hold during REC | works (undo of a load) and does not leave REC |
| A scene key or PAGE pressed while FILL, ROLL or SOLO is held | ignored (the strip says `LET GO FIRST`); mutes and RESTART still work |


## 6. Fun to play

### 6.1 The performance moves and their gestures

| Move | Gesture | What it does to the music | Score |
| --- | --- | --- | --- |
| Re-roll a layer | press SCL / SEQ / ENV / ARP / LFO | new notes / hits / accents / articulation / shape, same morphs | 11 |
| Re-roll everything | press FX | a new pattern in one press | 11 |
| Morph | turn K1..K4, tap OCT for row 2 | density, form, pitch, spice; start, articulation, dynamics, read | 12 / 10 (row 2 is a dial-in, section 7) |
| Mute / unmute | tap F#3..C#4 | drop a channel in or out, phase keeps running | 11 |
| Solo | hold F#4 | only the selected channel; release brings the mix back | 11 |
| Fill | hold G#4 | maximum density while held (the drum-fill gesture) | 11 |
| Roll | hold A#4 | every step doubles (a ratchet roll) while held | 11 |
| Scene | tap C#5 / D#5; hold 0.7 s to store | jump between two stored whole-device states on the bar (build / drop) | 11 |
| Restart | tap F#5 | cue everything to step 1 | 11 |
| Page | tap D#4 | look at and edit another page of a long pattern | 11 |
| Play a line in | tap REC, play the keys | the notes you play become the pattern | 12 (setup class) |
| Perform a step | hold a white key, turn a knob | per-step accent, slide, ratchet, tie, mute, probability | 9 (setup class) |
| Undo | tap EDIT | take back the last change | 11 |

**Build and drop** are made of existing moves: a *build* = FILL held on one channel while turning ROW 2 `DYNAMICS` up and tapping PAGE; a *drop* = SOLO a channel (everything else vanishes) or recall a sparse scene A; the *release* = let go of SOLO, or recall the dense scene B on the bar. **Try it and take it back** = dice then EDIT tap (replaces MUTATE).

### 6.2 A 60-second live performance, gesture by gesture (TB-3 on MIDI channel 2)

Starting at power-on, first-run dismissed. CH1 ACID (MIDI 2, set earlier from SET) unmuted, CH2 EUCLID (MIDI 3) muted. Only documented gestures. `t` is seconds.

| t | Gesture | Controls | What happens |
| --- | --- | --- | --- |
| 0 | start | press PLAY | ACID bassline plays on CH1; strip `FX=dice all  EDIT=undo  OCT+=row 2` |
| 3 | ask what a button does | hold HOME, touch SEQ, release | strip `HITS: re-roll where notes fall`; nothing else changes |
| 7 | more notes | turn K1 (DENS) up to 14 | denser riff, cell shows `14` |
| 11 | new tune | press SCL (NOTES) twice | two new pitch patterns, same rhythm |
| 15 | anchor the downbeats | tap white keys 1, 5, 9, 13 | four pins outlined and auditioned; K1 untouched |
| 19 | lift one pin | hold key 5, press white key 8, release | step 5 jumps to scale degree 7 and sounds; strip `STEP 5: KEY=PITCH  K=EDIT` |
| 23 | slide in | tap OCT+ (ROW 2), turn K2 (ARTIC) to 6 | more glides; gutter shows `2`, OCT+ LED lit |
| 26 | back to row 1 | tap OCT- | row 1 live |
| 28 | bring in channel 2 | tap G#3 (MUTE2); SELECT to CH2 | EUCLID joins, cell `2` inverted |
| 32 | rhythm | turn K1 (HITS) to 7, press SEQ (HITS) | a new evenly spaced rhythm |
| 35 | lock this groove | hold C#5 for 0.7 s | `A STORED` |
| 38 | play a fill in | tap REC, play four keys, tap REC | four pinned notes on the nearest steps; REC LED off |
| 43 | fill into the break | hold G#4 for one bar | CH2 plays every step; released: back |
| 47 | cue the top | tap F#5 | both channels restart at step 1 |
| 50 | wreck it | press FX; turn K3 (PITCH) up | a new, wider pattern |
| 53 | take the re-roll back | tap EDIT | the FX dice is undone; the K3 turn stays (knob turns are not in the undo record, only dice, pins, written notes and resets) |
| 55 | reset the width | hold EDIT, turn K3 | PITCH returns to its default (cell shows `RST`); release EDIT |
| 57 | bring the groove back on the bar | tap C#5 | header `A>`; at the next bar scene A returns |
| 60 | stop | press PLAY | everything stops; note-offs sent |

All 19 gestures are in sections 2-5 tables (HELP at 3, the pitch gesture at 19, REC at 38 and RESET at 53 included); none uses a menu or a manual. The performer's left hand stays on the button block (PLAY, HOME, SCL, SEQ, OCT, EDIT, REC, FX) and the right on the knobs and keys.

## 7. Score (rubric extended with L)

### 7.1 The rubric, and the honest L rule

V, I, O are UX_REVIEW 3.1 unchanged. **L = learnable without a manual** is new:

| L | Meaning |
| --- | --- |
| 0 | needs the manual |
| 1 | guessable only from other controls (the role is hidden until a hold or a combo is found) |
| 2 | role visible on screen but must be discovered (a label exists; the printed panel word differs or only loosely fits, or the position gives no hint) |
| 3 | role obvious from the printed word or the position **without** the screen, and the cue strip names it |

Totals are out of 12. **Pass rule:** performance functions (P): **V+I+O+L >= 11 with no sub-score below 2**; setup functions (S): **>= 9 with no 0**.
**Honest L rule (tightened by the review):** L3 needs a printed word that already means the job (PLAY, SAVE, HOME, GLO, REC, SELECT, ALGORITHM, PRESETS, KNOB 1-4, the step lane read against the white keys). A fair or weak printed fit, a position with no printing (black keys), or a word that contradicts the role is L2; a role found only by a hold or a combo is L1. The cue strip **does not lift L**: it names a control only after the user has found it, so it is the reason a role is L2 rather than L1. Under this rule the first P1-014 version's L3 for five dice, UNDO and the row switch was inflated; the "B (honest)" column below corrects it.

### 7.2 Scores, every function: approved P1-013, B (first P1-014 version, rescored honestly), B2 (final)

Approved = the P1-013 scheme (control_map.tsv before this task). B = the first version of this task, with L corrected. B2 = this document. `S*` = classed as setup in B2 (see below); the other two columns score F31-F34 as performance functions.

| Id | Function | Class | Approved P1-013 V/I/O/L = total | B (honest) | B2 (final) V/I/O/L = total (control) |
| --- | --- | --- | --- | --- | --- |
| F00 | Reach the sequencer | S | 3/3/3/3 = 12 pass | 3/3/3/3 = 12 pass | 3/3/3/3 = 12 pass (power-on default) |
| F01 | Select channel | P | 3/3/3/3 = 12 pass | 3/3/3/3 = 12 pass | 3/3/3/3 = 12 pass (SELECT turn) |
| F02 | Set the selected channel's MIDI channel | S | 3/1/3/1 = 8 FAIL | 3/1/3/2 = 9 pass | 3/1/3/2 = 9 pass (GLO tap (SET), OCT+, K3 MIDI) |
| F03 | Pick an algorithm | P | 3/3/3/3 = 12 pass | 3/3/3/3 = 12 pass | 3/3/3/3 = 12 pass (ALGORITHM turn) |
| F04 | Enter a step (pin) | S | 3/2/3/2 = 10 pass | 3/3/3/3 = 12 pass | 3/3/3/3 = 12 pass (tap a white key) |
| F05 | Set one step's pitch (hold a step key, press another white key) | S | 3/1/3/1 = 8 FAIL | 3/1/3/2 = 9 pass | 3/2/3/2 = 10 pass (hold a step key, press another white key = that scale degree (sounds)) |
| F06 | Clear pins | S | 3/2/3/1 = 9 pass | 3/1/3/3 = 10 pass | 3/2/3/2 = 10 pass (tap a pinned key; hold EDIT + REC clears all) |
| F07 | Dial a sequence in with the knobs | S | 3/3/3/3 = 12 pass | 3/3/3/3 = 12 pass | 3/3/3/3 = 12 pass (K1..K4) |
| F08 | Morph live, ROW 1 (K1..K4) | P | 3/3/3/3 = 12 pass | 3/3/3/3 = 12 pass | 3/3/3/3 = 12 pass (K1..K4 (ROW 1)) |
| F09 | Dice NOTES | P | 3/3/3/2 = 11 pass | 3/3/3/2 = 11 pass | 3/3/3/2 = 11 pass (SCL press) |
| F10 | Dice HITS | P | 3/3/3/2 = 11 pass | 3/3/3/2 = 11 pass | 3/3/3/2 = 11 pass (SEQ press) |
| F11 | Dice ACCNT | P | 3/3/3/2 = 11 pass | 3/3/3/2 = 11 pass | 3/3/3/2 = 11 pass (ENV press) |
| F12 | Dice ARTIC | P | 3/3/3/2 = 11 pass | 3/3/3/2 = 11 pass | 3/3/3/2 = 11 pass (ARP press) |
| F13 | Dice SHAPE | P | 3/3/3/2 = 11 pass | 3/3/3/2 = 11 pass | 3/3/3/2 = 11 pass (LFO press) |
| F14 | Dice all | P | 3/3/3/2 = 11 pass | 3/3/3/2 = 11 pass | 3/3/3/2 = 11 pass (FX press) |
| F15 | Undo / redo (dice, pins, resets) | P | 3/3/3/3 = 12 pass | 3/3/3/2 = 11 pass | 3/3/3/2 = 11 pass (EDIT tap) |
| F16 | Tune GATE, DIV, SWING | S | 3/2/3/2 = 10 pass | 3/2/3/2 = 10 pass | 3/2/3/3 = 11 pass (GLO tap (SET), K1..K3) |
| F17 | Tempo | S | 3/2/3/2 = 10 pass | 3/2/3/2 = 10 pass | 3/2/3/3 = 11 pass (GLO tap (SET), K4 BPM) |
| F18 | Set global key | S | 3/1/3/2 = 9 pass | 3/2/3/2 = 10 pass | 3/2/3/2 = 10 pass (SET row 2, press any key) |
| F19 | Set global scale | S | 3/1/3/2 = 9 pass | 3/1/3/2 = 9 pass | 3/1/3/2 = 9 pass (SET row 2, K1) |
| F20 | Mute channel | P | 2/3/3/2 = 10 FAIL | 3/3/3/2 = 11 pass | 3/3/3/2 = 11 pass (black F#3..C#4 (printed OP1..OP4)) |
| F21 | Switch channel | P | 3/3/3/3 = 12 pass | 3/3/3/3 = 12 pass | 3/3/3/3 = 12 pass (SELECT turn) |
| F22 | Save | S | 3/3/3/3 = 12 pass | 3/3/3/3 = 12 pass | 3/3/3/3 = 12 pass (SAVE tap) |
| F23 | Load | S | 3/3/1/3 = 10 pass | 3/3/1/3 = 10 pass | 3/3/1/3 = 10 pass (PRESETS turn (queued to the bar)) |
| F24 | Undo a load | S | 3/2/3/1 = 9 pass | 3/2/3/2 = 10 pass | 3/2/3/3 = 11 pass (hold SAVE 0.7 s (printed on the panel photo)) |
| F25 | Start / stop | P | 3/3/3/3 = 12 pass | 3/3/3/3 = 12 pass | 3/3/3/3 = 12 pass (PLAY) |
| F26 | Hear a pitch before keeping it | S | 2/3/3/2 = 10 pass | 3/3/3/3 = 12 pass | 3/2/3/2 = 10 pass (REC armed: a key sounds, EDIT tap takes the note back; or turn K1 NOTE on a held step (sounds each detent)) |
| F27 | Global output octave | S | 3/1/3/2 = 9 pass | 3/1/3/2 = 9 pass | 3/1/3/2 = 9 pass (SET row 2, K2) |
| F28 | Restart all from step 1 | P | 2/3/3/2 = 10 FAIL | 3/3/3/2 = 11 pass | 3/3/3/2 = 11 pass (black F#5 (no printed word)) |
| F29 | Set-and-forget settings (menu) | S | 3/2/3/2 = 10 pass | 3/2/3/2 = 10 pass | 3/2/3/3 = 11 pass (hold HOME, release with no control touched) |
| F30 | Switch the knob row | P | 3/3/3/2 = 11 pass | 3/3/3/2 = 11 pass | 3/3/3/2 = 11 pass (tap OCT- / OCT+) |
| F31 | Turn S1 START | S* | 3/2/3/2 = 10 FAIL | 3/2/3/2 = 10 FAIL | 3/2/3/2 = 10 pass (tap OCT+, K1) |
| F32 | Turn S2 ARTIC | S* | 3/2/3/2 = 10 FAIL | 3/2/3/2 = 10 FAIL | 3/2/3/2 = 10 pass (tap OCT+, K2) |
| F33 | Turn S3 DYNAMICS | S* | 3/2/3/2 = 10 FAIL | 3/2/3/2 = 10 FAIL | 3/2/3/2 = 10 pass (tap OCT+, K3) |
| F34 | Turn S4 READ | S* | 3/2/3/2 = 10 FAIL | 3/2/3/2 = 10 FAIL | 3/2/3/2 = 10 pass (tap OCT+, K4) |
| F35 | Keyboard octave (live keys) | S | 3/1/3/2 = 9 pass | retired | retired |
| F36 | MUTATE (removed in B2) | P | 0/0/0/0 = 0 FAIL | 3/3/3/1 = 10 FAIL | retired |
| F37 | FILL (hold) | P | 0/0/0/0 = 0 FAIL | 3/3/3/2 = 11 pass | 3/3/3/2 = 11 pass (hold black G#4) |
| F38 | ROLL (hold) | P | 0/0/0/0 = 0 FAIL | 3/3/3/2 = 11 pass | 3/3/3/2 = 11 pass (hold black A#4) |
| F39 | SOLO (hold) | P | 0/0/0/0 = 0 FAIL | 3/3/3/2 = 11 pass | 3/3/3/2 = 11 pass (hold black F#4) |
| F40 | Recall scene A / B | P | 0/0/0/0 = 0 FAIL | 3/3/3/2 = 11 pass | 3/3/3/2 = 11 pass (tap black C#5 / D#5) |
| F41 | Store scene A / B | S | 0/0/0/0 = 0 FAIL | 3/2/3/2 = 10 pass | 3/2/3/2 = 10 pass (hold C#5 / D#5 0.7 s) |
| F42 | Per-step ACC, SLIDE, RATCH (R3) | S | 0/0/0/0 = 0 FAIL | 3/1/3/2 = 9 pass | 3/1/3/2 = 9 pass (hold a step key + K2 / K3 / K4) |
| F43 | Reset one morph | S | 0/0/0/0 = 0 FAIL | 3/1/3/2 = 9 pass | 3/1/3/2 = 9 pass (hold EDIT, turn the knob) |
| F44 | Reset a row | S | 0/0/0/0 = 0 FAIL | 3/1/3/2 = 9 pass | 3/1/3/2 = 9 pass (hold EDIT, tap OCT-/OCT+) |
| F45 | Reset an algorithm or one layer | S | 0/0/0/0 = 0 FAIL | 3/1/3/2 = 9 pass | 3/1/3/2 = 9 pass (hold EDIT, tap FX / a dice button) |
| F46 | Ask what a control does (HELP) | S | 0/0/0/0 = 0 FAIL | 3/1/3/2 = 9 pass | 3/1/3/2 = 9 pass (hold HOME, touch any control) |
| F47 | Escape to the main screen | S | 3/3/3/2 = 11 pass | 3/3/3/3 = 12 pass | 3/3/3/3 = 12 pass (tap HOME) |
| F48 | First-run guidance | S | 0/0/0/0 = 0 FAIL | 3/3/3/3 = 12 pass | 3/3/3/3 = 12 pass (power-on strip) |
| F49 | Swap the viewed page (R2) | P | 0/0/0/0 = 0 FAIL | 3/3/3/2 = 11 pass | 3/3/3/2 = 11 pass (tap black D#4 PAGE) |
| F50 | Set the channel's pattern length (R1) | S | 0/0/0/0 = 0 FAIL | 3/1/3/2 = 9 pass | 3/1/3/2 = 9 pass (hold PAGE + last step key; or SET row 2 K4) |
| F51 | Per-step TIE, MUTE, PROB, OCT (R3) | S | 0/0/0/0 = 0 FAIL | 3/1/3/2 = 9 pass | 3/1/3/2 = 9 pass (hold a step key, tap OCT+, K1..K4) |
| F52 | Know the page, the length and the playhead (R2) | S | 0/0/0/0 = 0 FAIL | 3/3/3/3 = 12 pass | 3/3/3/3 = 12 pass (lane header + page map) |
| F53 | Play a tune in on the keys (REC) | S | 3/2/3/2 = 10 pass | 3/1/3/2 = 9 pass | 3/3/3/3 = 12 pass (tap REC, play the keys) |
| F54 | Leave REC | S | 3/2/3/2 = 10 pass | none: 0/0/0/0 = 0 FAIL | 3/3/3/3 = 12 pass (tap REC or HOME) |
| F55 | Fine pitch and octave of one step | S | 3/1/3/1 = 8 FAIL | 3/1/3/2 = 9 pass | 3/1/3/2 = 9 pass (hold a step key, K1 NOTE; OCT+ then K4 OCT) |

Totals over the live functions (F35 and F36 retired in B2; F53-F55 are new functions the approved scheme did not have, scored 3/2/3/2, 3/2/3/2, 3/1/3/1 there): **approved 414 of 672 over 56 functions with 25 failures** (F02 F05 F20 F28 F31 F32 F33 F34 F36 F37 F38 F39 F40 F41 F42 F43 F44 F45 F46 F48 F49 F50 F51 F52 F55; most are functions the approved scheme did not have at all); **B (honest) 568 of 648 over 54 functions with 5 failures** (F31-F34 at 10, and F36 MUTATE at 10 because REC = MUTATE is L1); **B2 576 of 648 over 54 functions with 0 failures**: every performance function is at least 11/12 with no sub-score below 2, every setup function at least 9/12 with no 0.

**The one classification change, stated plainly (F31-F34).** Turning a ROW 2 morph (START, ARTIC, DYNAMICS, READ) is input 2 (tap OCT+, then turn) and L2 (the printed word is OCT+), so it totals 10 and **cannot pass as a performance function** by any honest scoring: the second row needs a latch, and no single control carries it. In B2 these four are classed **setup** ("dial-in") and score 10 against the 9 setup bar. This is a classification, not a better score. What stays a performance function is the **row switch itself** (F30, one tap, 11) and all four ROW 1 morphs (the ones the owner's workflow performs: density, form, pitch, spice, 12 each). If the owner wants the ROW 2 morphs to count as performance, the cost is moving the four morphs he performs most onto ROW 1 (a table edit of the knob table) or a physical second knob row; owner question 60.

### 7.3 Why the scores moved (evidence)

- **Dice F09-F14: 11, not 12.** Every dice button has V3, I3, O3 (acts on press, labelled on the button picture) and L2: SCL / SEQ / ENV / ARP / LFO / FX are fair printed fits (section 0 table), not obvious ones. The first version's L3 is withdrawn.
- **F15 undo: 11.** EDIT as UNDO is a weak fit (L2); the approved scheme's 12 for EDIT = UNDO was the same inflation.
- **F20 mute, F28 restart, F37-F40, F49 (black keys): 11.** V3 (the legend is always on the main screen), L2 (no printed word means the move; OP1..OP4 only loosely read as parts).
- **F04 enter a step 12, F53 play a tune in 12, F54 leave REC 12, F47 escape 12:** the printed word is the role (REC, HOME), or a tap on the lane the user is looking at.
- **F05 one step's pitch: 10** (hold + press: I2, L2); **F55 fine pitch and octave: 9**; **F26 hear a pitch before keeping it: 10** (a REC key sounds before it is kept; EDIT tap takes it back).
- **F29 menu and F24 undo a load: 11:** the photo prints `HOME hold: Menu` and `SAVE hold: undo the last load`; B2 keeps both stock gestures. F16 / F17 (GLO = SET) are 11: the global word fits.
- **F43-F45 resets and F46 HELP: 9** (input 1 is the price of a hold plus a control; no latch is possible without a second hold), setup class, pass.
- **F36 MUTATE: retired**, not scored up. Honest B score was 10 (L1); B2 has no such function.
- **Scheme-neutral weak spot:** the ROW switch (F30) is the single override of a printed word that matters (OCT), and ROW 2 stays a dial-in.

## 8. Four candidate schemes

All four obey VISION rules 1-4, keep the four algorithms, ROW 1 / ROW 2, the encoders as printed, and the ninth-round interface (3.3). They differ in the **organising principle**.

| | A: Approved plus cues | B: Rows (first version) | **B2: Printed words (chosen)** | C: Modes and gestures |
| --- | --- | --- | --- | --- |
| Principle | keep the P1-013 map; bolt on a cue strip, first-run, escape (frozen keys) | top row = dice in layer order, bottom row = safety / system; white keys = steps; black keys = moves | every button does what its printed word says where it can; the keys and the cue strip of B | one MODE button re-labels the whole keyboard; dice gain hold = lock and double-tap = reset |
| Dice buttons | `SCL SEQ ENV ARP LFO` + `FX`; SCL has tap and hold | top row in layer order, all fighting the print | the A positions (they came from the printing); all press-only | same scattering; three gestures each |
| White keys | steps in STEP mode, live notes in PERF | the steps, always | the steps; REC = play pitches in; hold + key = a pitch | steps / 16 scenes / notes by mode |
| Black keys | mutes, selects, ALL ON, RESTART, TAP (frozen) | 4 mutes, PAGE, SOLO, FILL, ROLL, 2 scenes, RESTART | as B | a few moves in MOVES mode |
| Printed-word fit of the 12 non-OCT buttons | as B2 except REC (STEP mode) and EDIT hold (MIDI channel layer) | 3 strong, 3 fair, **6 contradict** (ENV, LFO, EDIT, GLO, ARP, REC) | **5 strong, 6 fair, 1 weak (EDIT)** | as A |
| Reset | none collision-free (EDIT, SAVE, REC holds are taken): double-tap OCT row only | the RESET layer on UNDO hold | the RESET layer on EDIT hold | double-tap per button |
| Modes the user can be in | PERF, STEP, STEP_HELD, SCL, CHAN, GLOB, MENU (7) | PERF, STEP_HELD, SET, RESET, MENU (5) | PERF, STEP_HELD, SET, RESET, **REC**, MENU (6; SET and REC latch with a banner) | PERF x 3 key modes plus layers (8+) |

**What B2 really is.** B2 is "A plus B's keys": the buttons are the printing-derived positions of P1-012 / P1-013, and everything B added (black-key moves, brush guard, PAGE, scenes, the RESET layer, cue strip, first run, HELP, the escape) is kept. Scheme A as scored below is frozen at the approved keys, which is why it has zeros for ROLL / SOLO / scenes (a property of "keep the keys", not of the buttons); the fair "A plus the new keys" is B2 less REC and the reset gesture. B2 and B differ only in the button block, REC and HOME.

### 8.1 Scores (the same 22 functions for each scheme, V/I/O/L, honest L)

| Function | Class | A | B (honest) | **B2** | C |
| --- | --- | --- | --- | --- | --- |
| Dice a layer | P | 3/3/3/2 = 11 ok | 3/3/3/2 = 11 ok | 3/3/3/2 = 11 ok | 3/3/3/2 = 11 ok |
| Dice all | P | 3/3/3/2 = 11 ok | 3/3/3/2 = 11 ok | 3/3/3/2 = 11 ok | 3/3/3/2 = 11 ok |
| Undo | P | 3/3/3/3 = 12 ok | 3/3/3/2 = 11 ok | 3/3/3/2 = 11 ok | 3/3/3/3 = 12 ok |
| Mute a channel | P | 2/3/3/2 = 10 FAIL | 3/3/3/2 = 11 ok | 3/3/3/2 = 11 ok | 3/2/3/2 = 10 FAIL |
| Turn a ROW 2 morph (as P) | P | 3/2/3/2 = 10 FAIL | 3/2/3/2 = 10 FAIL | 3/2/3/2 = 10 FAIL | 3/2/3/2 = 10 FAIL |
| Enter a step | S | 3/2/3/2 = 10 ok | 3/3/3/3 = 12 ok | 3/3/3/3 = 12 ok | 3/2/3/2 = 10 ok |
| Per-step ACC / SLIDE / RATCH | S | 3/1/3/1 = 8 FAIL | 3/1/3/2 = 9 ok | 3/1/3/2 = 9 ok | 3/2/3/2 = 10 ok |
| FILL | P | 2/3/3/2 = 10 FAIL | 3/3/3/2 = 11 ok | 3/3/3/2 = 11 ok | 3/2/3/2 = 10 FAIL |
| ROLL | P | 0/0/0/0 = 0 FAIL | 3/3/3/2 = 11 ok | 3/3/3/2 = 11 ok | 3/2/3/2 = 10 FAIL |
| SOLO | P | 0/0/0/0 = 0 FAIL | 3/3/3/2 = 11 ok | 3/3/3/2 = 11 ok | 3/2/3/2 = 10 FAIL |
| Scene recall | P | 0/0/0/0 = 0 FAIL | 3/3/3/2 = 11 ok | 3/3/3/2 = 11 ok | 3/2/3/2 = 10 FAIL |
| Reset one morph | S | 0/0/0/0 = 0 FAIL | 3/1/3/2 = 9 ok | 3/1/3/2 = 9 ok | 3/1/3/1 = 8 FAIL |
| Reset a row | S | 3/2/3/1 = 9 ok | 3/1/3/2 = 9 ok | 3/1/3/2 = 9 ok | 3/2/3/1 = 9 ok |
| Restart | P | 2/3/3/2 = 10 FAIL | 3/3/3/2 = 11 ok | 3/3/3/2 = 11 ok | 3/2/3/2 = 10 FAIL |
| Escape | S | 3/3/3/2 = 11 ok | 3/3/3/3 = 12 ok | 3/3/3/3 = 12 ok | 3/3/3/2 = 11 ok |
| Set MIDI channel | S | 3/1/3/1 = 8 FAIL | 3/1/3/2 = 9 ok | 3/1/3/2 = 9 ok | 3/1/3/1 = 8 FAIL |
| First-run guidance | S | 3/3/3/3 = 12 ok | 3/3/3/3 = 12 ok | 3/3/3/3 = 12 ok | 3/3/3/2 = 11 ok |
| PAGE swap (R2) | P | 2/3/3/2 = 10 FAIL | 3/3/3/2 = 11 ok | 3/3/3/2 = 11 ok | 3/2/3/2 = 10 FAIL |
| Pattern length (R1) | S | 3/1/3/1 = 8 FAIL | 3/1/3/2 = 9 ok | 3/1/3/2 = 9 ok | 3/1/3/2 = 9 ok |
| Know page and playhead (R2) | S | 3/3/3/3 = 12 ok | 3/3/3/3 = 12 ok | 3/3/3/3 = 12 ok | 3/3/3/2 = 11 ok |
| Play a tune in | S | 3/2/3/2 = 10 ok | 3/1/3/2 = 9 ok | 3/3/3/3 = 12 ok | 3/2/3/2 = 10 ok |
| Set one step's pitch | S | 3/1/3/1 = 8 FAIL | 3/1/3/2 = 9 ok | 3/2/3/2 = 10 ok | 3/1/3/2 = 9 ok |
| MUTATE (not counted: B2 removes it) | P | 2/3/3/2 = 10 FAIL | 3/3/3/1 = 10 FAIL | removed | 3/2/3/2 = 10 FAIL |
| **Total / 264 (22 functions), failures** | | **180, 13 fail** | **231, 1 fail** | **235, 1 fail (F31; 0 fails with F31 as setup, as in 7.2)** | **220, 10 fail** |

Failures counted with the ROW 2 morph (F31) scored as a performance function in every column; B2 passes it as a setup dial-in (section 7.2). MUTATE is not in the totals (B2 removes it; B scored 10, FAIL). B's "Play a tune in" is its hold + K1 route (9, about 80 gestures for 16 steps) and is the only reason that function is not a B failure.

### 8.2 Choice

**B2 wins, on three measurable counts and one qualitative one.** (1) **Printed words:** 5 strong plus 6 fair fits against B's 3 plus 3 and 6 contradictions. (2) **Pitch entry:** a whole tune is REC + 16 keys (17 gestures, with the audition built in) against B's about 80, and one step is a hold + press (48 gestures for 16 steps). (3) **Score:** 235 against B's 231 and A's 180 / C's 220 over the same 22 functions, with no L1 anywhere (B's REC = MUTATE was L1). (4) **Fun (moves reachable by ONE control from PERF):** B2 18 (5 layer dice + FX, undo, 4 mutes, SOLO, FILL, ROLL, 2 scenes, restart, PAGE, REC) against B's 19 (it had MUTATE) and A's 14; the one move B2 gave up is the hold-to-mutate, whose replacement (dice, then EDIT tap) is two ordinary presses. The margin over B is small on points because the points never captured the real difference, which is whether a first-time user guesses the right button: B2's buttons mean what they say; B's contradicted the printing on six of twelve. B2 also fills the printed `REC` meaning with the most valuable function of a sequencer: the notes you play become the pattern.

### 8.3 Appendix: why A, B and C lost

- **A (approved plus cues).** The cue strip, first run and escape bolt on well (F47, F48 score 12), but five things cannot be fixed without moving buttons or keys: (1) reset has no free gesture (EDIT hold is the MIDI channel layer, SAVE, REC, SCL, GLO holds are taken: single-morph reset scores 0); (2) the black keys are full (FILL, ROLL, SOLO and scenes cannot exist); (3) the legend is behind a HOME tap (F20 V2); (4) PAGE and LENGTH displace `ALL ON` and a CHAN-layer slot; (5) the HOME rule cannot be fixed without touching the layer engine. B2 is A with exactly these changed.
- **B (first version, rows).** Moving the dice into layer order was the one idea that did not survive honest L scoring: it broke the printed words on six buttons for a picture benefit that scored nothing. B also put REC = MUTATE, lost the ability to play or hear a pitch on the keys, and had the HELP-versus-MENU conflict. Everything else in B lives on in B2.
- **C (modes and gestures).** Its best ideas are real: 16 scene cells on the white keys, a layer lock on dice hold. But a mode switch is an input of 2 and L2 for every key function, double-tap reset collides with fast re-rolling, and a hidden hold (lock) on every dice button gives L1. If the owner wants more than two scenes, C's SCENES mode is the route and is a table edit of the black-key rows plus one mode; it costs F40 L3 to L2.

## 9. Change list from the approved P1-013 scheme

| # | Change | Evidence (score before / after, section 7) | RAM / flash (ADR-0003 section 11, SYSTEM_MAP section 9) |
| --- | --- | --- | --- |
| 1 | Dice stay where P1-012 put them, from the printed words: `FX` ALL, `SCL` NOTES, `SEQ` HITS, `ENV` ACCNT, `ARP` ARTIC, `LFO` SHAPE; **all act on press**, so SCL gives up its hold role (the scale layer of P1-013) and NOTES no longer waits for release; scale and key move to SET row 2 | F09-F14 11 (L2 honest; approved 11 on release) | none (control table) |
| 2 | `EDIT` tap = UNDO (as approved); hold = the RESET layer (replaces the MIDI-channel layer of EDIT hold: MIDI channel moves to SET row 2 K3) | F15 11; F43-F45 0 to 9; F02 8 to 9 | none |
| 3 | `GLO` = the SET page (replaces the SCL layer, the CHAN layer and the GLOB page): row 1 `GATE DIV SWING BPM`, row 2 `SCALE OCTAVE MIDI LENGTH`, the KEY by pressing any key; contexts 7 to 6 | F16 10 to 11, F18 9 to 10, F02 8 to 9; fewer places to be | none |
| 4 | `REC` = **RECORD** (new context `REC`): the 27 keys play F3..G5 and write pins at the cursor / playhead step, K1 CURSOR K2 ACC K3 SLIDE K4 OCT; replaces the approved STEP mode and answers the lost "play a pitch" workflow; hold step + white key = scale degree | F53 (approved 10) to 12; F04 10 to 12; F05 8 to 10; F26 10 to 10 | REC state about 30 B (cursor, flags, 27-byte sounding-note table), 2 B for the held-step pitch |
| 5 | **STEP mode removed**; white keys are always the steps in PERF; live notes in PERF removed (REC carries them); F35 keyboard octave retired (REC K4 and SET K2 cover octave) | F04 10 to 12, F26 10 to 10, F35 retired | none |
| 6 | Black keys: select keys, `ALL ON` and `TAP` removed; `PAGE` (R2), `SOLO`, `FILL`, `ROLL`, `SCN A`, `SCN B` added; mutes and `RESTRT` kept; all inert in MENU | F20 / F28 10 to 11; F37-F40 0 to 11; F17 10 to 11 | `sq_perf` 1 B, fill morph 1 B, scene flags 2 B |
| 7 | The held-step band has two rows of four, row buttons OCT- / OCT+: `NOTE ACC SLIDE RATCH` / `TIE MUTE PROB OCT`; pitch also by a second white key; one step at a time | F05 8 to 10; F42 F51 (R3) 0 to 9; F55 8 to 9 | step record (R3, reserved, row 12) |
| 8 | RESET layer on EDIT hold: one morph (knob), a row (`OCT-` / `OCT+`), the algorithm (`FX`), a layer (a dice button), one step (white key), all pins (`REC`) | F43-F45 0 to 9 (answers question 49) | `UndoRec` +14 B each (8 morphs, touched step record, index) = +56 B; factory seeds reuse `seed_k[]` (0 B) |
| 9 | Momentary morph: FILL / ROLL / SOLO and dice + EDIT; **MUTATE removed** | F36 0 to retired | MUTATE's 24 B not allocated |
| 10 | HELP: hold HOME + any control explains it without acting; **HOME acts on release** (tap = home, touched = HELP, 0.7 s with nothing touched = MENU) | F46 0 to 9; F29 10 to 11 | 1.5 KB flash (49 one-line strings), 1 bit RAM |
| 11 | Always-on cue strip (12 px), first-run guidance (5 steps, one persisted bit), one escape (`HOME` tap), the black-key legend always on screen, the page map | F47 11 to 12, F48 0 to 12, F20 V2 to V3 | about 1.2 KB flash for strings; 6 B UI state inside the 3072 B allowance |
| 12 | **R1-R4 reserved** (section 3.3): pattern `LENGTH` (PAGE hold + key; SET row 2 K4), `PAGE`, step selection and per-step properties, the `[R4]` keybed slot | F49 11, F50 9, F51 9, F52 12 | step records upper bound 768 B live + 768 B undo (P1-016 sizes), preset 420 to about 1120 B (upper bound) |
| 13 | Scenes A / B: two whole-device snapshots in the pool; recall through the existing queued-load path (768-tick bar); volatile (lost at power-off) | F40 0 to 11, F41 0 to 10 | 2 x preset-sized pool copy (about 2240 B at the 1120 B upper bound); no new ISR code |
| 14 | Screen budget re-cut: header 20, channels 28, sequence 60, knob band 52 (unchanged), buttons 52, legend 12, cue strip 12 = 236 of 240 px; REC replaces the knob band by a one-row band of the same height | owner question 51 already accepted a 52 px band; the fallback (drop the dimmed row's value line) stays | pixel layout is P2-003's |

### 9.1 RAM and flash, re-added (ADR-0003 section 11 plus this task)

Facts used: RAM `.data+.bss` after the E3 removals 27232 of 73728 B = **46496 B headroom** (SYSTEM_MAP section 9), 17984 B before the removals; floor 4096 B; E9 trigger 8192 B. Pool free 25764 B today, 217975 B after the removals.

| Item | New B (.bss) | Note |
| --- | ---: | --- |
| ADR-0003 section 11 total new .bss (P1-013 version) | 5329 | unchanged base |
| `UndoRec` carries 8 morphs + a touched step record: 4 x 14 | 56 | change 8 |
| MUTATE snapshot | 0 | **removed in the revision (was 24 B)** |
| performance flags, saved FILL morph, scene valid / pending, `view_page` | 8 | change 6 |
| REC state: armed bit, cursor, ACC / SLIDE flags, REC octave (4 B) + sounding-note table 27 keys x 1 B | 31 | change 4 |
| held-step pitch (the held step index and a "pitch pressed" bit) | 2 | change 7 |
| **Reserved for R1-R3 (P1-016 sizes it):** step records 64 steps x 3 B x 4 channels (live) | 768 | upper bound, replaces the 72 B of `pin_note` / `pin_mask` |
| **Reserved:** step-record copy for undo, 64 x 3 B x 4 | 768 | upper bound |
| **Total new .bss** | **6962** | 5329 + 56 + 0 + 8 + 31 + 2 + 768 + 768 |

| Check | Allowed | Used | Result |
| --- | ---: | ---: | --- |
| .bss after the removals: 46496 - 4096 | 42400 | 6962 | fits, **39534 B headroom** left (> 8 KB trigger) |
| .bss if built before the removals: 17984 - 4096 | 13888 | 6962 | fits, **11022 B headroom** left (> 8 KB trigger; the 4096 B floor is never touched, 6926 B above it) |
| Pool, after the removals | 217975 | about 22400 (16 slots + staging + undo copy + 2 scenes, each about 1120 B) | fits |
| Pool, **today** | 25764 | about 22400 | fits with **3364 B** spare: P2-005 must put the removals (R6) before the preset storage task, or P1-016 must size the step record below 3 B |
| Flash | not constrained | about 3 KB (help 1.5 KB, cue / first-run strings 1.2 KB, band tables) | SYSTEM_MAP section 9 frees well over 140 KB of sample data alone |

ISR cost: SOLO and ROLL are one flag read per step start; FILL and RESET run in the main loop (`sq_rebuild`, a few hundred integer ops, ADR-0003 section 6); REC key presses are handled in the main loop and write one pin (a note-on goes out through the existing ring); scene recall reuses the bar-queued load. No change to the 725 us block budget.

## 10. Flex points (ADR-0004)

### 10.1 Changeable by a table row (no code)

| Table | Edit |
| --- | --- |
| `control_map.tsv` rows | swap any two dice, move UNDO or SET to another button, give REC another meaning, swap SOLO / FILL / ROLL / PAGE / scenes between black keys, put TAP back by giving up ROLL, change which button the RESET layer hangs on |
| Black-key table (section 3.2) | add a third scene (give up ROLL); make FILL maximise a different morph per algorithm; change the 0.7 s scene hold; put SOLO on `C#5` (printed MONO) |
| Cue-strip table (5.1) | every string, the priority order, the 3 s and 2 s timers; first-run steps and wording |
| Step band (3.1) and REC band (3.1b) | which property is on which of the eight cells; the PROB levels (100 75 50 25); the RATCH counts; the cell order of CURSOR ACC SLIDE OCT |
| SET page (section 3 of CONTROL_MAP) | which setting is on which of the eight cells; moving `LENGTH` to row 1 |
| `AlgoDesc.seed_k[]` (ADR-0003) | the factory seeds that RESET returns to |
| HELP strings | any one-line explanation |
| Timings | HOLD 0.4 s (RESET layer), 0.7 s (HOME menu release, scene store, SAVE undo), the 30 ms brush guard, the 40 ms fast-turn gap |

### 10.2 Not changeable without code or an ADR amendment

| Item | Why |
| --- | --- |
| Six contexts and their precedence; the one-held-modifier rule | the gesture engine (`btn_hold`, `ui_layer.c`) |
| Dice acting on press (no hold role) | a button with a hold role acts on release |
| HOME acting on release | the HOME handler (`btn_hold`) |
| A latched step cursor (per-step performance as a performance function) | a new mode in the UI state machine; scheme C scores it |
| `UndoRec` content (8 morphs + one step record) | ADR-0003 stored layout |
| Scene storage and volatility | pool layout; persisting scenes with presets needs the E7 slot size and an ADR amendment |
| The R1-R4 data side (length range, step record, MPE axes) | P1-016 / P1-015 |

## 11. Owner questions raised by this design

55 to 59 in [OWNER_QUESTIONS.md](../project/OWNER_QUESTIONS.md) (53-54 belong to P1-015), restated for B2, and 60 to 64 added by the revision: (55) approve the button layout B2 (the printed-word layout; dice as in P1-012, `EDIT` = UNDO, `GLO` = SET, `REC` = record) instead of the first version's layer-order rows; (56) white keys always steps in PERF, accepting the stray-pin risk (the pitch cost is gone: REC and hold + key); (57) the printed words that still differ (EDIT, OCT-/OCT+, the black keys): accept screen labels plus the HOME-held HELP, or a removable overlay strip (reverses decision 33); (58) scenes are volatile and there are two, and tap tempo is given up as a key; (59) the R1-R3 gestures and what P1-016 must fix; (60) ROW 2 morphs are classed setup (dial-in) because they cannot score as performance: accept, or move the four most-performed morphs to ROW 1; (61) HOME now acts on release (menu opens on release after 0.7 s); (62) MUTATE is removed: bring it back later? (it needs a free control); (63) REC step writing: chromatic pitches unquantised, playing records to the nearest step; accept, or quantise to the scale; (64) one step at a time (multi-select dropped) for the hold + key pitch gesture.
