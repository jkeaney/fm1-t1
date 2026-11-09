# Performance-first control design (P1-014)

Task P1-014 (owner challenge, eighth round, 2026-11-10). Status: **for owner review** (gate `human-review`; no approval is
recorded here). Design only: no firmware, no tests.

> Change note: this file is new. It supersedes the control layout of FULL_CONTROL.md section 1 / UX_REVIEW.md section 1 /
> CONTROL_MAP.md as approved with P1-013, and answers owner questions 36 and 49. The edits it forces are applied to
> [control_map.tsv](control_map.tsv), [CONTROL_MAP.md](CONTROL_MAP.md), [FULL_CONTROL.md](FULL_CONTROL.md),
> [UX_REVIEW.md](UX_REVIEW.md), [ALGORITHM_SET.md](ALGORITHM_SET.md) and
> [ADR-0003](../project/decisions/ADR-0003-sequencer-core.md), each with a change note at its top.

Inputs: VISION.md, ADR-0004 (+ flexibility addendum), OWNER_QUESTIONS.md (all rounds, especially 36, 48-52, the sixth-round
"Shift" and the eighth-round "Perf"), FULL_CONTROL.md, UX_REVIEW.md (rubric 3.1), CONTROL_MAP.md / control_map.tsv,
CONTROL_INVENTORY.md, ALGORITHM_SET.md and section 6 of the four algorithm docs, ADR-0003, SYSTEM_MAP.md,
`firmware/src/panel.c`, and the panel photo `docs/controls.jpg`.

**What the photo says (it shapes learnability).** Printed on the panel: knobs `MASTER SELECT PRESETS ALGORITHM` and
`KNOB 1..4`; buttons `FX SCL ENV LFO EDIT GLO` over `HOME SAVE ARP SEQ PLAY/STOP REC`; `OCT-` `OCT+`; keys with small
marks on the black keys. Nothing says "dice", "morph", "mute" or "scene". So **the printing cannot teach this device;
the screen must**, and the screen must mirror the panel so that "where" does half the teaching. Three printed words
already match the job (SELECT, ALGORITHM, PRESETS) and four more are close enough once the screen labels them (PLAY,
SAVE, HOME, KNOB 1-4). Every other printed word (FX, SCL, ENV, LFO, EDIT, GLO, ARP, SEQ, REC, OCT) is a synth word that we
override on screen. That is the largest learnability risk in every scheme and is carried to the owner as question 57.

## 0. The scheme in one paragraph

**One principle: the panel is split by what it does to the music.**

| Zone | Principle | Controls |
| --- | --- | --- |
| Top button row `FX SCL ENV LFO EDIT GLO` | **the six dice, in layer order, left to right** (`ALL NOTES HITS ACCNT ARTIC SHAPE`) | press = re-roll that layer; hold UNDO and press = reset that layer |
| Bottom button row `HOME SAVE ARP SEQ PLAY REC` | **safety and system**: HOME (back), SAVE (keep), ARP = UNDO (take back), SEQ = SET (feel, tempo, key, MIDI), PLAY, REC = MUTATE | one escape, one undo, one setup page |
| Four knobs `KNOB 1..4` + `OCT-` `OCT+` | **morph**: two rows of four, the row buttons sit left of the knobs | unchanged from P1-013 |
| Encoders `SELECT ALGORITHM PRESETS` | **what is printed** | channel, algorithm, preset |
| 16 white keys | **the 16 steps of the viewed page, always** (no STEP mode, no live notes) | tap = pin, hold = edit the step (two rows of four properties) |
| 11 black keys | **performance moves** | 4 mutes, PAGE, SOLO, FILL, ROLL, SCENE A, SCENE B, RESTART |

A performer's two hands are split the same way the panel is: the left hand has the dice and the safety row, the right
hand has the knobs and the keys. Nothing needs a mode: the only latched page is SET, drawn with a banner and left by the
one escape (tap HOME).

## 1. Control accounting table

Every physical control, its role in PERF, and the single sentence that explains it. "PERF" is the resting main screen.
All 49 controls (MASTER, 3 encoders, 4 knobs, 14 buttons, 27 keys) have a role; the only intentional blanks are listed
under the table.

| Control (printed) | Principle | PERF role (tap / hold / combo) | On-screen label |
| --- | --- | --- | --- |
| `MASTER` pot | what is printed | volume (analogue; no MIDI effect) | none (a pot) |
| `SELECT` | what is printed | turn: select channel CH1..CH4 | `CH n` cell inverted |
| `ALGORITHM` | what is printed | turn: algorithm of the selected channel | algorithm name |
| `PRESETS` | what is printed | turn: preset slot; a filled slot loads at the next bar | header `Pnn`, `Pnn>` queued |
| `KNOB 1..4` | morph | turn: the live row (ROW 1 DENS FORM PITCH SPICE; ROW 2 START ARTIC DYNAMICS READ); held step key: step band row 1 `NOTE ACC SLIDE RATCH`, row 2 `TIE MUTE PROB OCT`; SET: row of four settings; held UNDO: reset that morph | two rows of cells, live row filled |
| `FX` | dice, position 1 | press: DICE ALL; hold UNDO + press: RESET the algorithm | `ALL` |
| `SCL` | dice, position 2 | press: dice NOTES (acts on press) | `NOTES` + algorithm sub-label |
| `ENV` | dice, position 3 | press: dice HITS | `HITS` + sub |
| `LFO` | dice, position 4 | press: dice ACCNT | `ACCNT` + sub |
| `EDIT` | dice, position 5 | press: dice ARTIC (dim if absent) | `ARTIC` + sub |
| `GLO` | dice, position 6 | press: dice SHAPE (dim if absent) | `SHAPE` + sub |
| `HOME` | safety: back | tap: back to this screen from anywhere; hold 0.7 s: MENU; hold + touch a control: HELP | `HOME`, `BACK` |
| `SAVE` | safety: keep | tap: save to the slot in the header; hold 0.7 s: undo a load | `SAVE Pnn` |
| `ARP` | safety: take back | tap: UNDO / REDO (dice, pin edit, reset); hold: the RESET layer | `UNDO` |
| `SEQ` | system: setup | tap: SET page (latched); hold: while held | `SET` |
| `PLAY` | system: transport | start / stop | `PLAY / STOP` (LED) |
| `REC` | system: momentary | hold: MUTATE (snaps back on release) | `MUTATE` |
| `OCT-` `OCT+` | rows of four | tap: latch ROW 1 / ROW 2; hold and turn: momentary; the same pair picks the step band row while a step key is held, and the SET row | `ROW 1`, `ROW 2`, `STEP 1`, `STEP 2` |
| White keys F3..G5 (16) | steps of the viewed page (R2) | press: pin step n of the page (audition); tap a pinned step: clear; hold: edit it (R3); with PAGE held: end the pattern here (R1); **[R4 slot: per-key expression]** | lane cell n lights; `step n` |
| Black `F#3 G#3 A#3 C#4` | moves | tap: mute / unmute CH1..CH4 | `MUTE1..4` |
| Black `D#4` | moves | tap: **PAGE** swap, next page, wraps (R2); hold: the next white key sets the pattern length (R1) | `PAGE`, header `PG 2/4` |
| Black `F#4` | moves | hold: SOLO the selected channel | `SOLO` |
| Black `G#4` | moves | hold: FILL (maximum density) | `FILL` |
| Black `A#4` | moves | hold: ROLL (every step repeats) | `ROLL` |
| Black `C#5` | moves | tap: recall scene A at the next bar; hold 0.7 s: store scene A | `SCN A` |
| Black `D#5` | moves | tap: recall scene B; hold 0.7 s: store scene B | `SCN B` |
| Black `F#5` | moves | press: restart all channels from step 1 | `RESTRT` |

**Intentional blanks (not dead, not ambiguous):**

- The six dice have **no hold role** (nothing to learn but "press"), so they act on press. This is a gain over the
  approved NOTES dice, which acted on release.
- White keys are inert in the MENU and during `RESET` (RESET gives them a defined role, clear that step); in `MENU` they do
  nothing so that no pin is set while a setting is changed. The MENU shows `close` on the buttons.
- `SEQ` is inert while a step key or UNDO is held (one held modifier at most). The cue strip says so (`FINISH THE STEP`).
- **Reserved slots (ninth-round requirements R1-R4, interface for P1-016 and P1-015, section 3.3):** `PAGE` (D#4), pattern `LENGTH` (hold PAGE + key; SET row 2 K4), per-step property rows (step band), and the `[R4]` keybed-expression slot on the white keys. Their control and screen contracts are fixed here; their ranges, storage and algorithm interaction are not.
- There are no live-note keys any more (see section 3 for why that is a gain, and the cost).

## 2. Panel buttons aligned to the algorithm controls

The rule, which holds in every algorithm: **the top row is the dice in the order of the layer list and of the screen's
button picture; a button never changes meaning with the algorithm, only its sub-label (and its dimming) does.**

| Position | Button | Layer | ACID sub-label | EUCLID | SNAKE | STAGE | Reset (UNDO held) |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | `FX` | ALL | all 6 seeds | all 4 | all 4 | all 5 | whole algorithm to factory |
| 2 | `SCL` | NOTES | `NOTES` | `NOTES` | `NOTES` | `NOTES` | NOTES seed to factory |
| 3 | `ENV` | HITS | `HITS` | `ROTATE` | `GATES` | `RESTS` | HITS seed to factory |
| 4 | `LFO` | ACCNT | `ACCNT` | `ACCNT` | `VELO` | `VELO` | ACCNT seed to factory |
| 5 | `EDIT` | ARTIC | `GT+SLD` | `RATCH` | dim `-` | `RT+HLD` | ARTIC seed to factory |
| 6 | `GLO` | SHAPE | `OCTAVE` | dim `-` | `DRIFT` | `LONG` | SHAPE seed to factory |

(Sub-labels from ALGORITHM_SET 4.4; the layer-to-seed matrix 4.2 is unchanged.)

The bottom row and the row buttons complete the picture: **`ARP` = UNDO for everything the dice, the pins and the reset
can do** (ADR-0003 `UndoRec`, extended), `SEQ` = the one setup page, `REC` = MUTATE. In every algorithm the same button
does the same job to the algorithm: top row changes the *content* (re-roll), the knobs change the *amount* (morph),
`ARP` takes the last change back, `UNDO` held resets. No algorithm-specific button exists.

**Why this order (evidence).** ALGORITHM_SET 4.1 lists the layers `NOTES HITS ACCNT ARTIC SHAPE`; the approved scheme
scattered them as `SCL SEQ ENV ARP LFO` over two rows, so a performer reaching for "rhythm" had to know that HITS lives on
the *bottom* row. The photo shows the top row as one block of six equal buttons: six dice, one block. The button picture
on screen is drawn as the same 2 x 6 block, so the position of a dice cell on screen is its position on the panel.

## 3. Keyboard: steps and performance moves

### 3.1 White keys: the steps of the viewed page (always)

- **Key n = step n of the viewed page** (R2): step number `(page - 1) x 16 + n`. With a pattern of 16 steps or fewer there is one page and nothing to swap. Keys past the pattern length are drawn dim and are inert (toast `PAST THE END: hold PAGE + key`).
- **Press** key n: if the step is not pinned, it is pinned now and **sounds** (the note the algorithm generated there, or the root if it was silent). If it is pinned, release within 0.4 s without turning anything clears it (tap = toggle).
- **Hold** key n = **select the step** (R3). The step band replaces the knob band and has two rows of four, the same model as the morph rows, with `OCT-` = row 1 and `OCT+` = row 2:

| Step band | K1 | K2 | K3 | K4 |
| --- | --- | --- | --- | --- |
| Row 1 (`STEP 1`, the default on hold) | `NOTE` (one scale degree per detent) | `ACC` (accent) `ON/OFF` | `SLIDE` `ON/OFF` | `RATCH` `1x..4x` |
| Row 2 (`STEP 2`, tap or hold `OCT+`) | `TIE` `ON/OFF` | `MUTE` `ON/OFF` | `PROB` `100 75 50 25` | `OCT` `-2..+2` |

  All held keys receive the same edit. Editing a step that the algorithm generated pins it first (with its generated note), so **any step can be performed on**, not only typed ones. Release of the last key returns to the main screen; the step record stays.
- **Pins ignore OFFSET and DIR** (core overlay keyed to the step position): a pin on key 5 stays on key 5.
- **Clear:** tap a pinned key; hold UNDO and press the key (clears that step's record); hold UNDO and press REC (clears all). UNDO tap restores.
- `K1 = 0` (`KEYS ONLY`) plays only the pins (owner decision 35).
- **Stopped vs playing.** Stopped: a press sounds the step once on the selected channel (audition, MIDI + preview). Playing: same, and the record takes effect at the next pass of that step.
- **Why white keys are never live notes.** The product is a sequencer (VISION rule 2); the approved PERF keys played live notes that were then lost, so a first press had no visible result (F26 L2). Now a press always writes a step the screen shows. **Cost:** a hand brushing a white key sets a pin. Rescue: the pin is outlined on the lane, UNDO tap removes it, tapping it again removes it. Owner question 56.

### 3.2 Black keys: performance moves

Layout follows the physical groups of black keys (3, 2, 3, 2, 1): the first four keys are the channels' mutes, `D#4` is `PAGE`, then the three **hold** moves together `F#4 G#4 A#4` (SOLO FILL ROLL: "press and keep holding"), the two scenes `C#5 D#5`, and the lone last black key `F#5` RESTART (the easiest key to find blind: the last one). PAGE sits at the left end of the keybed next to the first white keys because it changes what those keys mean.

| Key | Label | Gesture | Exact behaviour | State it needs |
| --- | --- | --- | --- | --- |
| F#3 G#3 A#3 C#4 | `MUTE1..4` | tap | latched mute; muted cell dimmed `M`; phase keeps running | existing `ChanSave.muted` |
| D#4 | `PAGE` | tap / hold | tap: view the next page of the selected channel (wraps 1..M); hold: the next white key pressed ends the pattern at that step (`LENGTH`) | UI `view_page` 1 B; length is P1-016's |
| F#4 | `SOLO` | hold | only the selected channel sounds while held; release restores every mute. Phase keeps running | 1 bit `sq_perf` read by the ISR |
| G#4 | `FILL` | hold | the selected channel's K1 morph goes to its maximum (every step / stage plays); release restores the value | 1 B saved morph + flag |
| A#4 | `ROLL` | hold | every step of the selected channel plays its note twice (ratchet x2) while held; release stops | 1 bit `sq_perf`, one `max(rep,2)` in ADR-0003 7.3 step 6 |
| C#5 | `SCN A` | tap / hold 0.7 s | tap: recall scene A at the next bar (header `A>`); hold 0.7 s: store the whole current state into A; toast `A STORED`. Empty: toast `A EMPTY: hold to store` | one preset-sized pool copy each |
| D#5 | `SCN B` | tap / hold 0.7 s | as A | one pool copy |
| F#5 | `RESTRT` | tap | all channels to step 1 at the next step | existing |

**Tap tempo is given up as a key** (it held `D#4`): tempo is on SET K4 (`BPM`) and a live tempo change is rare; PAGE needs the key (R2). Scored: F17 3/2/3/2 = 10 (setup, pass).

**Legend, always on screen.** A 12 px band under the button picture shows the 11 black keys as 11 cells in the physical 3-2-3-2-1 grouping, each with a label of at most 6 characters (P2-003 verifies with `text_w(&AF_S)` that `RESTRT` fits the 21 px cell pitch; if not, the band uses a 2-line cell and the sequence band gives up 8 px). A cell is inverted while its key is held; `M` cells are dimmed for muted channels. The 16 white keys need no legend: **the step lane above is the legend**, because lane cell n is drawn in the same left-to-right order as white key n, and a lit lane cell is a pinned key.

### 3.3 Pattern length, pages, per-step performance and keybed expression (ninth round R1-R4): the interface P1-016 fills

These four are reserved at the control level. The gestures, labels and screen contract are fixed and scored; ranges, storage, and the algorithm side are **not** (P1-016 integrates them; P1-015 researches R4). Nothing below edits ADR-0003 or the algorithm docs.

| Requirement | Control reserved | Gesture | Screen contract | P1-016 / P1-015 fills |
| --- | --- | --- | --- | --- |
| **R1** pattern length per channel (amends VISION rule 3: MIDI channel, algorithm, **length**) | `LENGTH`: SET row 2 K4 (slot reserved); and the fast gesture **hold `PAGE`, press the last step key** (TR-style "last step") | hold D#4 + press white key n of the viewed page: the pattern ends at step `(page-1) x 16 + n` | channel cell shows `n` steps if not 16; lane header `LEN 24`; keys past the end dim | range (assumed 1..64), default (16), relation to the algorithm `LEN` / `TOTAL` morph (K2 FORM), step storage, preset bytes |
| **R2** pages | `PAGE` = black D#4 | tap: next page (wraps). The view never moves by itself; the playhead is never moved by paging | lane header `PG 2/4` (page N of M); a **page map** of M small cells (max 4) with the viewed page outlined and the playhead's page filled; if the playhead is on another page, a marker `> P3` appears at the lane edge; on the viewed page the playhead cell is inverted | max pages, whether paging while stopped follows the cursor, how `OFFSET` and `DIR` wrap across pages |
| **R3** per-step performance | step select = **hold a white key**; step band (3.1) | hold key, turn K1..K4 (row 1), tap `OCT+` for row 2: `NOTE ACC SLIDE RATCH` / `TIE MUTE PROB OCT` | band replaces the knob band (`STEP 1`/`STEP 2` in the OCT cells), the lane cell is outlined, a pin shows its note; a flag shows a small glyph on the lane cell (accent `!`, slide `~`, tie `-`, ratchet `x2`, mute `m`, probability `%`) | the per-step record, which flags exist, how PROB draws deterministically, how an algorithm-generated step becomes a record, preset bytes |
| **R4** keybed expression (MPE-like) | **slot `[R4]` on the 16 white keys** (no gesture promised) | if P1-015 confirms per-key pressure, position or velocity-after-touch: pressing a held step key harder / sliding writes the **armed** step property (the last cell turned in the step band) | the armed cell is outlined in the step band; the value reads live | hardware capability, the axes available, ranges, whether it applies live (playing a step) or edit-only. **Floor:** if it cannot score P >= 11 with no sub-score under 2 it stays out of v1 and the step band remains the only route |

Scores for R1-R3 are in section 7 (F49 PAGE 12; F50 LENGTH 9; F51 / F42 per-step 9; F52 page / length / playhead cue 12). **Per-step performance is scored as step editing (setup class, input 1: hold a step key and turn a knob)**, because a latch that would give input 2 (a latched step cursor) is a mode the scheme avoids (scheme C scores it, section 8). If the owner wants per-step properties to count as a performance function, the control change is one row of control_map.tsv (give the step band a latched cursor) and the cost is a mode.

## 4. Reset gestures and the momentary morph

### 4.1 Reset (owner question 49, now designed)

Reset lives on the UNDO button because UNDO already means "take back": the performer who has lost the sound reaches for the
same button.

- **Tap UNDO** = undo / redo (unchanged, including pins and resets). One record per channel (owner decision 46, extended to carry morphs).
- **Hold UNDO** (0.4 s, or at once if any control is pressed meanwhile; the existing layer behaviour, CONTROL_INVENTORY 3) = the **RESET layer**. Cells tag
  `RST`, buttons relabel, the cue strip reads `RESET: knob = that morph  OCT = row  FX = all`.

| In the RESET layer | Gesture | Resets | To |
| --- | --- | --- | --- |
| **Single morph** | turn that knob (any detent) | that one morph of the live row of the selected channel | the morph's default (FULL_CONTROL tables) |
| **Row** | tap `OCT-` or `OCT+` | the four morphs of ROW 1 / ROW 2 | defaults |
| **Algorithm** | tap `FX` | all 8 morphs **and** all seeds of the selected channel's algorithm | defaults and factory seeds (pins and flags kept) |
| **One layer** | tap a dice button | that layer's seeds | factory seeds |
| **One step / all pins** | press a white key / press `REC` | that pin and its flags / every pin of the channel | no pin |

Every reset is **one UNDO tap away from being taken back** (`UndoRec` stores the other 8 morphs and the touched step record).
Factory seeds are the existing default-seed rule (`AlgoDesc.seed_k[]`, ADR-0003 section 4): no new table.
Scores: section 7 (F43 F44 F45 are setup/recovery functions at 9/12; input is 1 because a hold plus a control is "modifier + control" in the rubric, and no latch can make it 2 without a second hold).
Row-reset by double-tap of OCT was scored (3/2/3/2 = 10) and rejected: two quick OCT taps are a normal way to be sure of the row, so it would reset by accident.

### 4.2 Momentary morph (owner question 36)

Felucca's "LFO held, knob moves snap back" scores input 1 (a hold plus a turn is "modifier + control") and a
performance function needs input >= 2 (rule: no sub-score below 2), so a *hold + turn* gesture does not earn a place as
a performance function. The goal behind it (try a change, then snap back) is met by **one-control moves**:

- **MUTATE** (hold REC): the selected channel's whole algorithm state (8 morphs + 6 seeds) is saved, all layers are
  re-rolled, and **release restores the saved state exactly**. Anything diced or turned while REC is held is also undone on release,
  so a performer who wants a temporary morph holds REC and turns: that works but is **not advertised** (its score is the I=1 score, F36's
  extension; it is a free consequence of "release restores", not a counted function).
- **FILL** (hold `G#4`): the K1 morph goes to maximum and returns. **ROLL** and **SOLO**: the other two momentary moves.
- To **keep** a mutation: release REC (it snaps back), then press `FX` (dice all, kept) or use the dice buttons; there is no "commit" gesture because there is nothing hidden to commit.

Cost: MUTATE needs 24 B of RAM (`AlgoState` 20 B + channel + flags). Resolved: question 36 is closed by MUTATE / FILL / ROLL / SOLO.

## 5. Never lost, no manual

Design rule: **at every moment the screen answers four questions without a manual: where am I, what is playing, what does each control do right now, how do I get back.** Four always-on elements answer them.

### 5.1 The always-on cue strip

A 12 px line at the very bottom of the main screen (AF_S, at most 34 characters, one line, never scrolling). It shows the one most useful instruction for the current state; the first matching row wins. It is drawn in every context except `MENU`. It does not replace the button picture (which labels all 12 buttons) or the knob band (which labels all 8 morphs): the strip says what to do **next** or what the held control does.

| Priority | State | Cue strip text |
| --- | --- | --- |
| 1 | HELP active (HOME held + a control touched) | `<LABEL>: <one line>` (for example `HITS: re-roll where notes fall`) |
| 2 | RESET layer | `RESET: knob=one OCT=row FX=all` |
| 3 | A step key held | `STEP n: K=edit  OCT+: row 2` |
| 4 | PAGE held | `PAGE: press a key = pattern ends` |
| 5 | MUTATE held | `MUTATE: let go to snap back` |
| 6 | SOLO / FILL / ROLL held | `SOLO CH2: let go to restore` (same shape) |
| 7 | SET page, row 2 | `SET: press a key = KEY  HOME=back` |
| 8 | SET page, row 1 | `SET: OCT-/OCT+ = row  HOME=back` |
| 9 | A preset or scene is queued | `LOADS AT NEXT BAR  SAVE hold=undo` |
| 10 | Just after any dice / reset | `UNDO takes it back` (2 s) |
| 11 | First-run step (5.2) | the step's instruction |
| 12 | Stopped, nothing else | `PLAY: start   K1-K4: morph` |
| 13 | Playing, ROW 1 | `FX=dice all  ARP=undo  OCT+=row 2` |
| 14 | Playing, ROW 2 | `ROW 2: OCT- = back to row 1` |

Texts are table data (ADR-0004); the priorities and the 34-character limit are the contract.

### 5.2 First run (shown once, dismissed by any control)

Power-on boots into a working sequencer (owner decision 32b: CH1 ACID, unmuted, MIDI 1, plays on PLAY). A 4-step **guided strip** uses the cue strip itself; it never blocks anything.

| Step | Cue strip | Advances when | The user learns |
| --- | --- | --- | --- |
| 1 | `PRESS PLAY` | PLAY pressed | the device makes sound at once |
| 2 | `TURN K1 TO MORPH IT` | any knob turned | knobs change the pattern |
| 3 | `PRESS FX: NEW PATTERN` | any dice button pressed | the top row re-rolls; ARP takes it back |
| 4 | `TAP WHITE KEYS: ADD STEPS` | any white key pressed | keys are the steps |

Rules: **any other control also counts as dismissing** (the strip clears and the main screen is normal); HOME tap skips it; it is shown until completed or skipped **once per device** (one bit in the persisted settings word `ui_prefs2`, 0 B RAM, set at completion or skip); `MENU > SYSTEM` is not needed to see it again because a settings reset clears the bit. RAM: 1 B progress counter in the UI allowance. It never repeats after the bit is set.

### 5.3 One escape, undo everywhere, and "what does this do"

- **Escape: tap `HOME`.** From SET, RESET, a held step, the first-run strip and the MENU it returns to the main screen. In PERF it shows the help line for 3 s (so pressing it never feels dead). Layers are momentary anyway; the escape exists for the one latched page (SET) and the MENU.
- **Undo is reachable everywhere except the MENU:** `ARP` tap works in PERF, a held step, SET and RESET (it does one thing in every context: take back the last dice, dice-all, pin or step edit, or reset; again = redo). One record per channel (owner decision 46).
- **What does this do (touch-to-learn):** **hold HOME, then press or turn any control**: the cue strip and a toast explain that control (`K3 PITCH: how wide pitches range`), the control itself does nothing, and releasing HOME returns to normal. A press of HOME alone for 0.7 s still opens the MENU (the menu opens only if no control was touched). Scored (F46): V3 I1 O3 L2 = 9, a setup function that passes and **earns its place** for two reasons: it is the only route to a plain-words explanation of the physical controls whose printing contradicts their role (FX, SCL, ENV, LFO, EDIT, GLO, ARP, SEQ, REC, OCT), and the cue strip advertises it on the HOME tap. Cost: a 49-entry one-line help table (about 1.5 KB flash) and no RAM beyond a 1-B "help held" bit.
- **Why not hold-any-control-to-learn:** the dice, mutes and keys act on press, so a "touch" would already have changed the music. Hold HOME is the only form that changes nothing.

### 5.4 Naming and iconography rules

1. A label is at most 6 characters, capitals, no punctuation but `+` `-` `/` and digits (`RST R1`, `SCN A`, `GT+SLD`).
2. **Layer names are nouns of the music** (`NOTES HITS ACCNT ARTIC SHAPE`); **moves are one verb or noun the performer would say** (`MUTATE FILL ROLL SOLO PAGE`); resets are `RST` + a short target; scenes are `SCN A` / `SCN B`.
3. The same word means the same thing everywhere: `ALL` = every layer or every channel it is shown on; `ROW 1` / `ROW 2` appear on the OCT cells everywhere (`SET 1` / `STEP 1` only while those bands are up).
4. Tap = set or latch, hold = momentary, in every context (mutes latch on a tap; MUTATE, SOLO, FILL, ROLL, PAGE-length are held). No control mixes a tap role and a different hold role except UNDO (hold = RESET), SEQ (hold = SET while held), SAVE, HOME and the scene keys, each of which carries a sub-label naming its hold (`UNDO / RESET`).
5. A dimmed label with `-` means "this algorithm has no such layer" and nothing else.
6. Icons: only three glyphs are used beyond text: a filled dot for a lit/held move, the lane-cell flag glyphs of section 3.3, and the LED on PLAY.

### 5.5 Every state a first-time user could be confused in, and the cue that rescues them

| # | Confusing state | Rescuing cue |
| --- | --- | --- |
| 1 | "I pressed PLAY and hear nothing" (muted channel, MIDI OFF, K1 = 0) | the channel cell shows `M` or `OFF`; the strip says `UNMUTE: F#3` / `KEYS ONLY: tap white keys`; the K1 cell reads `KEYS ONLY` |
| 2 | "The knobs do nothing" (ROW 2 is latched and I turned a ROW 1 knob; or the wrong channel is selected) | gutter digit and filled row show which row is live; strip `ROW 2: OCT- = back to row 1`; the selected channel cell is inverted |
| 3 | "A step lit up and I did not mean it" (brushed a white key) | the cell is outlined; the strip says `UNDO takes it back`; ARP |
| 4 | "I changed the sound and cannot get back" | UNDO tap; hold UNDO resets (the strip names both) |
| 5 | "I am in some page" (SET latched) | `SET` banner, SEQ cell reads `BACK`; strip `HOME=back` |
| 6 | "The buttons are labelled FX, SCL, ENV... but do other things" | the button picture on screen shows the real role; hold HOME + a button explains it |
| 7 | "I pressed a preset and nothing happened" (queued to the bar) | header `P03>`, strip `LOADS AT NEXT BAR`; SAVE hold undoes |
| 8 | "A key does nothing" (past the pattern length) | the key is dim; toast `PAST THE END: hold PAGE + key` |
| 9 | "Where is the rest of my pattern?" (more than 16 steps) | lane header `PG 1/4`, page map, `> P3` playhead marker; PAGE |
| 10 | "I held REC and the sound changed" | strip `MUTATE: let go to snap back`; the pattern returns on release |
| 11 | "The dice button does nothing" (layer absent) | dimmed label `SHAPE -` and toast `NO SHAPE IN EUCLID` |
| 12 | "Nothing at all on screen changes" (SCREEN OFF / wake) | any control wakes the screen without acting (existing MENU > MIDI SCREEN WAKE) |
| 13 | "I am in the menu" | the menu has its own title; every button reads `close`; HOME tap leaves |
| 14 | "I held a step key and the knobs changed meaning" | the step band replaces the knob band and the strip says `STEP n: K=edit  OCT+: row 2` |

## 6. Fun to play

### 6.1 The performance moves and their gestures

| Move | Gesture | What it does to the music | Score |
| --- | --- | --- | --- |
| Re-roll a layer | press one of 5 top-row buttons | new notes / hits / accents / articulation / shape, same morphs | 12 |
| Re-roll everything | press FX | a new pattern in one press | 12 |
| Morph | turn K1..K4, tap OCT for row 2 | density, form, pitch, spice; start, articulation, dynamics, read | 12 / 11 |
| Mute / unmute | tap F#3..C#4 | drop a channel in or out, phase keeps running | 12 |
| Solo | hold F#4 | only the selected channel; release brings the mix back | 12 |
| Fill | hold G#4 | maximum density while held (the drum-fill gesture) | 12 |
| Roll | hold A#4 | every step doubles (a ratchet roll) while held | 12 |
| Mutate | hold REC | a temporary mutation of the selected channel, exact snap-back | 11 |
| Scene | tap C#5 / D#5; hold 0.7 s to store | jump between two stored whole-device states on the bar (build / drop) | 11 |
| Restart | tap F#5 | cue everything to step 1 | 12 |
| Page | tap D#4 | look at and edit another page of a long pattern | 12 |
| Perform a step | hold a white key, turn a knob | per-step accent, slide, ratchet, tie, mute, probability | 9 (setup class) |
| Undo | tap ARP | take back the last change | 12 |

**Build and drop** are made of existing moves: a *build* = FILL held on one channel while turning ROW 2 `DYNAMICS` up and tapping PAGE; a *drop* = SOLO a channel (everything else vanishes) or recall a sparse scene A; the *release* = let go of SOLO, or recall the dense scene B on the bar.

### 6.2 A 60-second live performance, gesture by gesture (TB-3 on MIDI channel 2)

Starting at power-on, first-run dismissed. CH1 ACID (MIDI 1) unmuted, CH2 EUCLID (MIDI 2) muted. Only documented gestures. `t` is seconds.

| t | Gesture | Controls | What happens |
| --- | --- | --- | --- |
| 0 | start | press PLAY | ACID bassline plays on CH1; strip `FX=dice all  ARP=undo  OCT+=row 2` |
| 4 | more notes | turn K1 (DENS) up to 14 | denser riff, cell shows `14` |
| 8 | new tune | press SCL (NOTES) twice | two new pitch patterns, same rhythm |
| 12 | anchor the downbeats | tap white keys 1, 5, 9, 13 | four pins outlined and auditioned; K1 untouched |
| 16 | lift one pin | hold key 5, turn K1 `NOTE` +3, tap OCT+, turn K4 `OCT` +1, release | step 5 jumps up an octave plus a third |
| 21 | slide in | tap OCT+ (ROW 2), turn K2 (SLIDE) to 6 | more glides; gutter shows `2` |
| 25 | back to row 1 | tap OCT- | row 1 live |
| 27 | bring in channel 2 | tap G#3 (MUTE2); SELECT to CH2 | EUCLID joins on MIDI 2, cell `2` inverted |
| 32 | rhythm | turn K1 (HITS) to 7, press ENV (HITS) | a new evenly spaced rhythm |
| 36 | lock this groove | hold C#5 for 0.7 s | `A STORED` |
| 39 | mutate | hold REC for two beats, release | the channel mutates, then snaps back exactly |
| 44 | fill into the break | hold G#4 for one bar | CH2 plays every step; released: back |
| 48 | cue the top | tap F#5 | both channels restart at step 1 |
| 51 | wreck it | press FX; turn K3 (PITCH) up | a new, wider pattern |
| 54 | take one change back | tap ARP | the wide pitch pattern's last dice undone |
| 56 | bring the groove back on the bar | tap C#5 | header `A>`; at the next bar scene A returns |
| 59 | stop | press PLAY | everything stops; note-offs sent |

All 17 gestures are in section 2-5 tables; none uses a menu or a manual. The performer's left hand stays on the button block (PLAY, SCL, ENV, FX, OCT, ARP, REC) and the right on the knobs and keys.

## 7. Score (rubric extended with L)

### 7.1 The rubric

V, I, O are UX_REVIEW 3.1 unchanged. **L = learnable without a manual** is new:

| L | Meaning |
| --- | --- |
| 0 | needs the manual |
| 1 | guessable only from other controls (the role is hidden until a hold or a combo is found) |
| 2 | role visible on screen but must be discovered (a label exists; the printed panel word differs or the position gives no hint) |
| 3 | role obvious from the label and position, and the cue strip names it |

Totals are out of 12. **Pass rule:** performance functions (P): **V+I+O+L >= 11 with no sub-score below 2**; setup functions (S): **>= 9 with no 0**. Where the printed panel word contradicts the on-screen role, L cannot exceed 2 unless position and the cue strip teach it (that is why the top-row dice score L3 here and the scattered approved dice score L2). Approved = the P1-013 scheme (control_map.tsv before this task), scored with L added.

### 7.2 Scores, every function, approved versus proposed

| Id | Function | Class | Approved P1-013 V/I/O/L = total | Approved | Proposed V/I/O/L = total (control) | Proposed |
| --- | --- | --- | --- | --- | --- | --- |
| F00 | Reach the sequencer | S | 3/3/3/3 = 12 | pass | 3/3/3/3 = 12 (power-on default) | pass |
| F01 | Select channel | P | 3/3/3/3 = 12 | pass | 3/3/3/3 = 12 (SELECT turn) | pass |
| F02 | Set the selected channel's MIDI channel | S | 3/1/3/1 = 8 | FAIL | 3/1/3/2 = 9 (SET, OCT+, K3 MIDI (approved: hold EDIT, K1..K4)) | pass |
| F03 | Pick an algorithm | P | 3/3/3/3 = 12 | pass | 3/3/3/3 = 12 (ALGORITHM turn) | pass |
| F04 | Enter a step (pin) | S | 3/2/3/2 = 10 | pass | 3/3/3/3 = 12 (tap a white key (approved: REC tap, then key)) | pass |
| F05 | Set a pin's pitch and octave | S | 3/1/3/1 = 8 | FAIL | 3/1/3/2 = 9 (hold a step key + K1 NOTE; OCT+ then K4 OCT) | pass |
| F06 | Clear pins | S | 3/2/3/1 = 9 | pass | 3/1/3/3 = 10 (tap a pinned key; hold UNDO + REC clears all (approved: hold REC 0.7 s)) | pass |
| F07 | Dial a sequence in with the knobs | S | 3/3/3/3 = 12 | pass | 3/3/3/3 = 12 (K1..K4) | pass |
| F08 | Morph live, ROW 1 (K1..K4) | P | 3/3/3/3 = 12 | pass | 3/3/3/3 = 12 (K1..K4 on the latched ROW 1) | pass |
| F09 | Dice NOTES | P | 3/3/3/2 = 11 | pass | 3/3/3/3 = 12 (SCL press (approved: SCL tap, acts on release)) | pass |
| F10 | Dice HITS | P | 3/3/3/2 = 11 | pass | 3/3/3/3 = 12 (ENV press (approved: SEQ)) | pass |
| F11 | Dice ACCNT | P | 3/3/3/2 = 11 | pass | 3/3/3/3 = 12 (LFO press (approved: ENV)) | pass |
| F12 | Dice ARTIC | P | 3/3/3/2 = 11 | pass | 3/3/3/3 = 12 (EDIT press (approved: ARP)) | pass |
| F13 | Dice SHAPE | P | 3/3/3/2 = 11 | pass | 3/3/3/3 = 12 (GLO press (approved: LFO)) | pass |
| F14 | Dice all | P | 3/3/3/2 = 11 | pass | 3/3/3/3 = 12 (FX press) | pass |
| F15 | Undo / redo (dice, pins, resets) | P | 3/3/3/3 = 12 | pass | 3/3/3/3 = 12 (ARP (UNDO) tap (approved: EDIT tap)) | pass |
| F16 | Tune GATE, DIV, SWING | S | 3/2/3/2 = 10 | pass | 3/2/3/2 = 10 (SET (latched), K1..K3 (approved: GLO tap)) | pass |
| F17 | Tempo | S | 3/2/3/2 = 10 | pass | 3/2/3/2 = 10 (SET K4 BPM (approved: GLO + K4 or the F#5 TAP key; the TAP key is given up for PAGE)) | pass |
| F18 | Set global key | S | 3/1/3/2 = 9 | pass | 3/2/3/2 = 10 (SET, OCT+, then any key (approved: hold SCL + key)) | pass |
| F19 | Set global scale | S | 3/1/3/2 = 9 | pass | 3/1/3/2 = 9 (SET, OCT+, K1) | pass |
| F20 | Mute channel | P | 2/3/3/2 = 10 | FAIL | 3/3/3/3 = 12 (black F#3..C#4 (legend always on screen)) | pass |
| F21 | Switch channel | P | 3/3/3/3 = 12 | pass | 3/3/3/3 = 12 (SELECT turn (approved also had four select keys)) | pass |
| F22 | Save | S | 3/3/3/3 = 12 | pass | 3/3/3/3 = 12 (SAVE tap) | pass |
| F23 | Load | S | 3/3/1/3 = 10 | pass | 3/3/1/3 = 10 (PRESETS turn (queued to the bar)) | pass |
| F24 | Undo a load | S | 3/2/3/1 = 9 | pass | 3/2/3/2 = 10 (hold SAVE 0.7 s (cue strip names it after a load)) | pass |
| F25 | Start / stop | P | 3/3/3/3 = 12 | pass | 3/3/3/3 = 12 (PLAY) | pass |
| F26 | Hear a step's note while entering | S | 2/3/3/2 = 10 | pass | 3/3/3/3 = 12 (the key press auditions the pin (approved: live keys in PERF)) | pass |
| F27 | Global output octave | S | 3/1/3/2 = 9 | pass | 3/1/3/2 = 9 (SET, OCT+, K2) | pass |
| F28 | Restart all from step 1 | P | 2/3/3/2 = 10 | FAIL | 3/3/3/3 = 12 (black F#5 RESTRT) | pass |
| F29 | Set-and-forget settings (menu) | S | 3/2/3/2 = 10 | pass | 3/2/3/3 = 11 (hold HOME 0.7 s) | pass |
| F30 | Switch the knob row | P | 3/3/3/2 = 11 | pass | 3/3/3/3 = 12 (tap OCT- / OCT+) | pass |
| F31 | Turn S1 START | P | 3/2/3/2 = 10 | FAIL | 3/2/3/3 = 11 (tap OCT+, turn K1) | pass |
| F32 | Turn S2 ARTIC | P | 3/2/3/2 = 10 | FAIL | 3/2/3/3 = 11 (tap OCT+, turn K2) | pass |
| F33 | Turn S3 DYNAMICS | P | 3/2/3/2 = 10 | FAIL | 3/2/3/3 = 11 (tap OCT+, turn K3) | pass |
| F34 | Turn S4 READ | P | 3/2/3/2 = 10 | FAIL | 3/2/3/3 = 11 (tap OCT+, turn K4) | pass |
| F35 | Keyboard octave (live keys) | S | 3/1/3/2 = 9 | pass | retired (retired: white keys are steps, there are no live keys) | - |
| F36 | MUTATE (hold, snaps back) | P | none: 0/0/0/0 = 0 | FAIL | 3/3/3/2 = 11 (hold REC) | pass |
| F37 | FILL (hold) | P | none: 0/0/0/0 = 0 | FAIL | 3/3/3/3 = 12 (hold black G#4) | pass |
| F38 | ROLL (hold) | P | none: 0/0/0/0 = 0 | FAIL | 3/3/3/3 = 12 (hold black A#4) | pass |
| F39 | SOLO (hold) | P | none: 0/0/0/0 = 0 | FAIL | 3/3/3/3 = 12 (hold black F#4) | pass |
| F40 | Recall scene A / B | P | none: 0/0/0/0 = 0 | FAIL | 3/3/3/2 = 11 (tap black C#5 / D#5 (queued to the bar)) | pass |
| F41 | Store scene A / B | S | none: 0/0/0/0 = 0 | FAIL | 3/2/3/2 = 10 (hold C#5 / D#5 0.7 s) | pass |
| F42 | Per-step ACC, SLIDE, RATCH (R3) | S | none: 0/0/0/0 = 0 | FAIL | 3/1/3/2 = 9 (hold a step key + K2 / K3 / K4 (step band row 1)) | pass |
| F43 | Reset one morph | S | none: 0/0/0/0 = 0 | FAIL | 3/1/3/2 = 9 (hold UNDO, turn the knob) | pass |
| F44 | Reset a row | S | none: 0/0/0/0 = 0 | FAIL | 3/1/3/2 = 9 (hold UNDO, tap OCT- / OCT+) | pass |
| F45 | Reset an algorithm or one layer | S | none: 0/0/0/0 = 0 | FAIL | 3/1/3/2 = 9 (hold UNDO, tap FX / a dice button) | pass |
| F46 | Ask what a control does (HELP) | S | none: 0/0/0/0 = 0 | FAIL | 3/1/3/2 = 9 (hold HOME, press or turn the control) | pass |
| F47 | Escape to the main screen | S | 3/3/3/2 = 11 | pass | 3/3/3/3 = 12 (tap HOME) | pass |
| F48 | First-run guidance | S | none: 0/0/0/0 = 0 | FAIL | 3/3/3/3 = 12 (shown at power-on until done) | pass |
| F49 | Swap the viewed page (R2) | P | none: 0/0/0/0 = 0 | FAIL | 3/3/3/3 = 12 (tap black D#4 PAGE) | pass |
| F50 | Set the channel's pattern length (R1) | S | none: 0/0/0/0 = 0 | FAIL | 3/1/3/2 = 9 (hold PAGE + press the last step key; or SET, OCT+, K4 LENGTH) | pass |
| F51 | Per-step TIE, MUTE, PROB, OCT (R3) | S | none: 0/0/0/0 = 0 | FAIL | 3/1/3/2 = 9 (hold a step key, tap OCT+ (step band row 2), turn K1..K4) | pass |
| F52 | Know the page, the length and the playhead (R2) | S | none: 0/0/0/0 = 0 | FAIL | 3/3/3/3 = 12 (lane header `PG 2/4 LEN 24` + page map) | pass |

Totals over the 53 functions: approved 386 of 636; **the approved scheme fails 24 of 53** under the extended rule (F02 F05 F20 F28 F31 F32 F33 F34 F36 F37 F38 F39 F40 F41 F42 F43 F44 F45 F46 F48 F49 F50 F51 F52; F36-F52 are new functions that the approved scheme did not have, F02 and F05 fail on L1, F20 and F28 on L2 with V2, F31-F34 on L2). The proposed scheme: 570 of 624 over the 52 live functions (F35 retired), **all 52 pass**: every performance function is at least 11/12 with no sub-score below 2, every setup function at least 9/12.

### 7.3 Why the scores moved (evidence)

- **Dice F09-F14: L2 to L3.** Same V/I/O. The six dice are one block in the order of the layer list, the screen draws the same block, and the cue strip names the most recent one; the approved scheme scattered them over both rows and put NOTES on the same button as the SCALE layer.
- **F20 mute and F28 restart: 10 to 12.** V2 to V3 because the black-key legend is now on the main screen, not behind a HOME tap; L2 to L3 because the legend is always there.
- **F31-F34 (ROW 2): 10 to 11.** The approved scheme left L at 2 (printed `OCT+` means octave). The cue strip names `OCT+ = ROW 2` on every playing screen and first-run step 2 teaches the knobs; this is the weakest L3 of the whole scheme and is carried to question 57.
- **F36-F41, F49: new moves** all at 11-12.
- **F04, F26: white keys always steps.** The approved path needed REC first; now a press is a step and is heard.
- **F43-F45 resets and F46 HELP: 9** (input 1 is the price of a hold plus a control; no latch is possible without a second hold), setup class, pass.
- **F17 tempo:** 10 to 10, but now only SET K4 (the TAP key is given up for PAGE).
- **F29 menu: L2 to L3:** the strip advertises `HOME hold = MENU`.

## 8. Three candidate schemes

All three obey VISION rules 1-4, keep the four algorithms, ROW 1 / ROW 2, the encoders as printed, and the ninth-round interface (3.3). They differ in the **organising principle**.

| | A: Approved plus cues | **B: Rows (chosen)** | C: Modes and gestures |
| --- | --- | --- | --- |
| Principle | keep the P1-013 map; bolt on a cue strip, first-run, escape, resets | panel top row = dice, bottom row = safety / system; white keys = steps; black keys = moves | one MODE button re-labels the whole keyboard (STEPS / MOVES / SCENES); dice gain hold = lock and double-tap = reset |
| Dice buttons | `FX SCL SEQ ENV ARP LFO` scattered; SCL tap / hold double role | top row in layer order, press only | same scattering; every dice button has tap, hold (lock the layer) and double-tap (reset) |
| White keys | steps in STEP mode, live notes in PERF | the steps, always | steps in STEPS mode, 16 scene cells in SCENES mode, live notes in MOVES mode |
| Black keys | mutes, selects, ALL ON, RESTART, TAP (full) | 4 mutes, PAGE, SOLO, FILL, ROLL, 2 scenes, RESTART | mutes and a few moves in MOVES mode |
| Reset | none collision-free (EDIT, SAVE, REC holds are taken): double-tap OCT row only | the RESET layer on UNDO hold: one morph, row, algorithm, layer, step, all pins | double-tap per button |
| Modes the user can be in | PERF, STEP, STEP_HELD, SCL, CHAN, GLOB, MENU (7) | PERF, STEP_HELD, SET, RESET, MENU (5, three momentary) | PERF x 3 key modes plus layers (8+) |

### 8.1 Scores (the same 21 functions for each scheme, V/I/O/L)

| Function | Class | A | B (chosen) | C |
| --- | --- | --- | --- | --- |
| Dice a layer | P | 3/3/3/2 = 11 ok | 3/3/3/3 = 12 ok | 3/3/3/2 = 11 ok |
| Dice all | P | 3/3/3/2 = 11 ok | 3/3/3/3 = 12 ok | 3/3/3/2 = 11 ok |
| Undo | P | 3/3/3/3 = 12 ok | 3/3/3/3 = 12 ok | 3/3/3/3 = 12 ok |
| Mute a channel | P | 2/3/3/2 = 10 FAIL | 3/3/3/3 = 12 ok | 3/2/3/2 = 10 FAIL |
| Turn a ROW 2 morph | P | 3/2/3/2 = 10 FAIL | 3/2/3/3 = 11 ok | 3/2/3/2 = 10 FAIL |
| Enter a step | S | 3/2/3/2 = 10 ok | 3/3/3/3 = 12 ok | 3/2/3/2 = 10 ok |
| Per-step ACC / SLIDE / RATCH | S | 3/1/3/1 = 8 FAIL | 3/1/3/2 = 9 ok | 3/2/3/2 = 10 ok |
| MUTATE | P | 2/3/3/2 = 10 FAIL | 3/3/3/2 = 11 ok | 3/2/3/2 = 10 FAIL |
| FILL | P | 2/3/3/2 = 10 FAIL | 3/3/3/3 = 12 ok | 3/2/3/2 = 10 FAIL |
| ROLL | P | 0/0/0/0 = 0 FAIL | 3/3/3/3 = 12 ok | 3/2/3/2 = 10 FAIL |
| SOLO | P | 0/0/0/0 = 0 FAIL | 3/3/3/3 = 12 ok | 3/2/3/2 = 10 FAIL |
| Scene recall | P | 0/0/0/0 = 0 FAIL | 3/3/3/2 = 11 ok | 3/2/3/2 = 10 FAIL |
| Reset one morph | S | 0/0/0/0 = 0 FAIL | 3/1/3/2 = 9 ok | 3/1/3/1 = 8 FAIL |
| Reset a row | S | 3/2/3/1 = 9 ok | 3/1/3/2 = 9 ok | 3/2/3/1 = 9 ok |
| Restart | P | 2/3/3/2 = 10 FAIL | 3/3/3/3 = 12 ok | 3/2/3/2 = 10 FAIL |
| Escape | S | 3/3/3/2 = 11 ok | 3/3/3/3 = 12 ok | 3/3/3/2 = 11 ok |
| Set MIDI channel | S | 3/1/3/1 = 8 FAIL | 3/1/3/2 = 9 ok | 3/1/3/1 = 8 FAIL |
| First-run guidance | S | 3/3/3/3 = 12 ok | 3/3/3/3 = 12 ok | 3/3/3/2 = 11 ok |
| PAGE swap (R2) | P | 2/3/3/2 = 10 FAIL | 3/3/3/3 = 12 ok | 3/2/3/2 = 10 FAIL |
| Pattern length (R1) | S | 3/1/3/1 = 8 FAIL | 3/1/3/2 = 9 ok | 3/1/3/2 = 9 ok |
| Know page and playhead (R2) | S | 3/3/3/3 = 12 ok | 3/3/3/3 = 12 ok | 3/3/3/2 = 11 ok |
| **Total / 252, failures** | | **172, 13 fail** | **234, 0 fail** | **211, 11 fail** |

Rows marked FAIL do not meet the pass rule. A and C score the new moves by what each scheme can place: A has no free control for ROLL, SOLO or scenes (0), and puts MUTATE and FILL on keys whose legend is behind a HOME tap (V2); C needs a mode switch first (input 2) and a hidden hold (lock) on the dice, so L is 2 or 1 almost everywhere.

### 8.2 Choice

**B wins on score and on fun.** Score: B passes all 21 functions; A fails 13 of 21 and C fails 11 (every performance function that needs a key mode first scores input 2 and L2 = 10). Fun (performance moves reachable by ONE control from PERF): A 14 (6 dice, undo, 4 mutes, restart, plus MUTATE and FILL bolted onto the keys of ALL ON and TAP; no room for ROLL, SOLO or scenes), **B 19** (6 dice, undo, 4 mutes, SOLO, FILL, ROLL, MUTATE, 2 scenes, restart, PAGE), C 7 directly (6 dice, undo) and 13 more behind the mode switch (its 16 scene cells are the most fun idea in the three, but every move waits for a mode). B also has the fewest modes (five, three of them momentary), so the user is the least often "somewhere".

### 8.3 Appendix: why A and C lost

- **A (approved plus cues).** The cue strip, first run and escape bolt on well (F47, F48 score 12), but five things cannot be fixed without moving buttons: (1) NOTES dice acts on release because SCL also has a hold role; (2) reset has no free gesture (EDIT, SAVE, REC, SCL, GLO holds are all taken, so single-morph reset scores 0 and row reset is a double-tap OCT that collides with confirming the row); (3) the black keys are full, so FILL, ROLL, SOLO and scenes cannot exist; (4) the legend is behind a HOME tap (F20 V2); (5) PAGE and LENGTH (R1, R2) displace `ALL ON` and a CHAN-layer slot (both score 8-10). Kept as a fallback: if the owner rejects moving the dice, A plus a scene key is the smallest change that passes.
- **C (modes and gestures).** Its best ideas are real: 16 scene cells on the white keys, a layer lock on dice hold. But a mode switch is an input of 2 and learnability L2 for every key function (the keyboard means three different things, and a first-time user presses a key and gets the wrong one), double-tap reset collides with fast re-rolling, and a hidden hold (lock) on every dice button gives L1. It scores a latched step cursor at 10 (per-step performance as a performance function would need that mode). If the owner wants more than two scenes, C's SCENES mode is the route and is a table edit of the black-key rows plus one mode; it costs F40 L3 to L2.

## 9. Change list from the approved P1-013 scheme

| # | Change | Evidence (score before / after, section 7) | RAM / flash (ADR-0003 section 11, SYSTEM_MAP section 9) |
| --- | --- | --- | --- |
| 1 | Dice move to the top row in layer order: `FX` ALL, `SCL` NOTES, `ENV` HITS, `LFO` ACCNT, `EDIT` ARTIC, `GLO` SHAPE; all act on press (no hold role) | F09-F14 L2 to L3 (11 to 12); NOTES no longer waits for release | none (control table) |
| 2 | UNDO moves from `EDIT` tap to `ARP` tap; hold = RESET layer | F15 12 to 12; frees EDIT for a dice | none |
| 3 | `SEQ` = the SET page (replaces the SCL layer, the CHAN layer and the GLOB page): row 1 `GATE DIV SWING BPM`, row 2 `SCALE OCTAVE MIDI LENGTH`, the KEY by pressing any key; contexts 7 to 5 | F16 10 to 10, F18 9 to 10, F02 8 (fail, L1) to 9; fewer places to be | none |
| 4 | `REC` = MUTATE (hold, snaps back); **STEP mode removed**; white keys are always the steps | F04 10 to 12, F36 0 to 11 | MUTATE snapshot 24 B |
| 5 | Live notes on the keys removed (F26 re-defined as the audition of a step, F35 keyboard octave retired) | F26 10 to 12; removes the `KB OCT` setup | none |
| 6 | Black keys: select keys, `ALL ON` and `TAP` removed; `PAGE` (R2), `SOLO`, `FILL`, `ROLL`, `SCN A`, `SCN B` added; mutes and `RESTRT` kept | F20 / F28 10 to 12; F37-F40 0 to 11-12; F17 10 to 10 | `sq_perf` 1 B, fill morph 1 B, scene flags 2 B |
| 7 | The held-step band has two rows of four, row buttons OCT- / OCT+: `NOTE ACC SLIDE RATCH` / `TIE MUTE PROB OCT`; the octave of a pin is now a K4 value on row 2, not a direct OCT press | F05 8 (fail, L1) to 9; F42 F51 (R3) 0 to 9 | step record (R3, reserved, row 12) |
| 8 | RESET layer on UNDO hold: one morph (knob), a row (`OCT-` / `OCT+`), the algorithm (`FX`), a layer (a dice button), one step (white key), all pins (`REC`) | F43-F45 0 to 9 (answers question 49) | `UndoRec` +14 B each (8 morphs, touched step record, index) = +56 B; factory seeds reuse `seed_k[]` (0 B) |
| 9 | MUTATE / FILL / ROLL / SOLO replace the momentary-morph gesture (question 36: a hold + turn scores input 1) | F36-F39 11-12 | included in 4 and 6 |
| 10 | HELP: hold HOME + any control explains it without acting; HOME hold alone still opens the MENU | F46 0 to 9; F29 L2 to L3 | 1.5 KB flash (49 one-line strings), 1 bit RAM |
| 11 | Always-on cue strip (12 px), first-run guidance (4 steps, one persisted bit), one escape (`HOME` tap), the black-key legend always on screen, the page map | F47 11 to 12, F48 0 to 12, F20 V2 to V3 | about 1.2 KB flash for strings; 6 B UI state inside the 3072 B allowance |
| 12 | **R1-R4 reserved** (section 3.3): pattern `LENGTH` (PAGE hold + key; SET row 2 K4), `PAGE`, step selection and per-step properties, the `[R4]` keybed slot | F49 12, F50 9, F51 9, F52 12 | step records upper bound 768 B live + 768 B undo (P1-016 sizes), preset 420 to about 1120 B (upper bound) |
| 13 | Scenes A / B: two whole-device snapshots in the pool; recall through the existing queued-load path (768-tick bar); volatile (lost at power-off) | F40 0 to 11, F41 0 to 10 | 2 x preset-sized pool copy (about 2240 B at the 1120 B upper bound); no new ISR code |
| 14 | Screen budget re-cut: header 20, channels 28, sequence 60, knob band 52 (unchanged), buttons 52, legend 12, cue strip 12 = 236 of 240 px | owner question 51 already accepted a 52 px band; the fallback (drop the dimmed row's value line) stays | pixel layout is P2-003's |

### 9.1 RAM and flash, re-added (ADR-0003 section 11 plus this task)

Facts used: RAM `.data+.bss` after the E3 removals 27232 of 73728 B = **46496 B headroom** (SYSTEM_MAP section 9), 17984 B before the removals; floor 4096 B; E9 trigger 8192 B. Pool free 25764 B today, 217975 B after the removals.

| Item | New B (.bss) | Note |
| --- | ---: | --- |
| ADR-0003 section 11 total new .bss (P1-013 version) | 5329 | unchanged base |
| `UndoRec` carries 8 morphs + a touched step record: 4 x 14 | 56 | change 8 |
| MUTATE snapshot (`AlgoState` 20 B + channel + flags) | 24 | change 4 |
| performance flags, saved FILL morph, scene valid / pending, `view_page` | 8 | change 6 |
| **Reserved for R1-R3 (P1-016 sizes it):** step records 64 steps x 3 B x 4 channels (live) | 768 | upper bound, replaces the 72 B of `pin_note` / `pin_mask` |
| **Reserved:** step-record copy for undo, 64 x 3 B x 4 | 768 | upper bound |
| **Total new .bss** | **6953** | 5329 + 56 + 24 + 8 + 768 + 768 |

| Check | Allowed | Used | Result |
| --- | ---: | ---: | --- |
| .bss after the removals: 46496 - 4096 | 42400 | 6953 | fits, **39543 B headroom** left (> 8 KB trigger) |
| .bss if built before the removals: 17984 - 4096 | 13888 | 6953 | fits, **11031 B headroom** left (> 8 KB trigger, < 4 KB floor never touched) |
| Pool, after the removals | 217975 | about 22400 (16 slots + staging + undo copy + 2 scenes, each about 1120 B) | fits |
| Pool, **today** | 25764 | about 22400 | fits with **3364 B** spare: P2-005 must put the removals (R6) before the preset storage task, or P1-016 must size the step record below 3 B |
| Flash | not constrained | about 3 KB (help 1.5 KB, cue / first-run strings 1.2 KB, band tables) | SYSTEM_MAP section 9 frees well over 140 KB of sample data alone |

ISR cost: SOLO and ROLL are one flag read per step start; FILL, MUTATE and RESET run in the main loop (`sq_rebuild`, a few hundred integer ops, ADR-0003 section 6); scene recall reuses the bar-queued load. No change to the 725 us block budget.

## 10. Flex points (ADR-0004)

### 10.1 Changeable by a table row (no code)

| Table | Edit |
| --- | --- |
| `control_map.tsv` rows | move any dice to another top-row button; swap UNDO and SEQ; give MUTATE to a black key; swap SOLO / FILL / ROLL / PAGE / scenes between black keys; put TAP back by giving up ROLL; change which button the RESET layer hangs on |
| Black-key table (section 3.2) | add a third scene (give up ROLL); make FILL maximise a different morph per algorithm; change the 0.7 s scene hold |
| Cue-strip table (5.1) | every string, the priority order, the 3 s and 2 s timers; first-run steps and wording |
| Step band (3.1) | which property is on which of the eight cells; the PROB levels (100 75 50 25); the RATCH counts |
| SET page (section 3 of CONTROL_MAP) | which setting is on which of the eight cells; moving `LENGTH` to row 1 |
| `AlgoDesc.seed_k[]` (ADR-0003) | the factory seeds that RESET returns to |
| HELP strings | any one-line explanation |
| Timings | HOLD 0.4 s (menu), 0.7 s holds, the 40 ms fast-turn gap |

### 10.2 Not changeable without code or an ADR amendment

| Item | Why |
| --- | --- |
| Five contexts and their precedence; the one-held-modifier rule | the gesture engine (`btn_hold`, `ui_layer.c`) |
| Dice acting on press (no hold role) | a button with a hold role acts on release |
| A latched step cursor (per-step performance as a performance function) | a new mode in the UI state machine; scheme C scores it |
| `UndoRec` content (8 morphs + one step record) | ADR-0003 stored layout |
| Scene storage and volatility | pool layout; persisting scenes with presets needs the E7 slot size and an ADR amendment |
| The R1-R4 data side (length range, step record, MPE axes) | P1-016 / P1-015 |

## 11. Owner questions raised by this design

55 to 59 in [OWNER_QUESTIONS.md](../project/OWNER_QUESTIONS.md) (53-54 belong to P1-015): (55) approve moving the dice to the top row in layer order and `ARP` = UNDO, `SEQ` = SET; (56) white keys always steps, with no live notes, accepting the stray-pin risk; (57) the printed panel words (FX SCL ENV LFO EDIT GLO ARP SEQ REC OCT) contradict the on-screen roles, so L is capped at 2-3: accept screen labels plus HELP, or a removable overlay strip; (58) scenes are volatile (lost at power-off) and there are two of them: persist them with the preset slots (about 15 KB more pool, an E7 decision) and/or more than two, and tap tempo is given up as a key; (59) the R1-R3 gestures and what P1-016 must fix (length range, per-step performance scored as step editing).
