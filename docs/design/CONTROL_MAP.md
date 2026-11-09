# Control map v1: every control, every context, one gesture

> P1-014 revision (printed-word pass, scheme B2; supersedes the layer-order layout of the note below): every button now does what its **printed word** says wherever this sequencer can do it. `FX` = DICE ALL; `SCL` = NOTES (scale / pitch), `ENV` = ACCNT (envelope accent), `LFO` = SHAPE (movement over the bar), `ARP` = ARTIC (repeats, slides, gate: the arpeggio-like layer), `SEQ` = HITS (the steps); `EDIT` = UNDO / RESET; `GLO` = the SET page (global feel); `REC` = RECORD (new `REC` context: play the keys, they write); `HOME` hold: HELP while held, MENU on release if nothing was touched. MUTATE is removed (dice + UNDO cover it, owner question 36 stays with the hardware test). White keys stay the steps; hold a step key and press another white key = that scale degree. Contexts are six (`REC` added, 294 rows = 49 x 6). Appendix A regenerated from control_map.tsv. Reasoning and scores: PERFORMANCE_DESIGN.md sections 0, 7, 8.

> P1-014 (performance-first, owner eighth round): the layout now follows ONE principle, see [PERFORMANCE_DESIGN.md](PERFORMANCE_DESIGN.md). Top button row (FX SCL ENV LFO EDIT GLO) = the six dice in layer order; bottom row (HOME SAVE ARP SEQ PLAY REC) = safety and system (HOME escape, SAVE, UNDO, SET, PLAY, MUTATE); the 16 white keys are always the 16 steps (no STEP mode, no live keys); the 11 black keys are performance moves (4 mutes, PAGE, SOLO, FILL, ROLL, SCENE A, SCENE B, RESTART); ninth-round requirements R1-R4 are reserved as interfaces (pattern LENGTH, PAGE, per-step properties, keybed slot) for P1-016. Contexts are now five: `PERF`, `STEP_HELD`, `SET` (replaces `SCL`, `CHAN`, `GLOB`), `RESET` (new, hold UNDO) and `MENU`; `STEP` is gone. Appendix A is regenerated from control_map.tsv (245 rows = 49 controls x 5 contexts). Everything not named here (sections 5, 7 reasoning) is unchanged from P1-013.

> P1-013: the knobs gain a second row (ROW 2: S1..S4, up to 8 morphs per algorithm). OCT- / OCT+ become the row buttons (tap latches ROW 1 / ROW 2, hold and turn is momentary). Design in [FULL_CONTROL.md](FULL_CONTROL.md).

> P2-009: the chord row of the CONTROL tab is removed (owner decision 39). CONTROL tab is 4 rows; 28 menu rows in total (SYSTEM_MAP section 7).

Task P2-001, amended by P1-013 and P1-014. Status: consolidation of the owner-approved P1-012 design (approved 2026-11-10, with P1-010; owner
decisions 31-36 in [OWNER_QUESTIONS.md](../project/OWNER_QUESTIONS.md)), re-organised by the performance-first pass
P1-014 (for owner review). The reasoning, ease scores and rejected alternatives are in
[PERFORMANCE_DESIGN.md](PERFORMANCE_DESIGN.md) (current) and [UX_REVIEW.md](UX_REVIEW.md) (history); the exact
machine-readable table is [control_map.tsv](control_map.tsv) (245 rows = 49 controls x 5 contexts). Appendix A below repeats every TSV row
verbatim, and `python3 tools/check_control_map.py` proves the two agree. If they ever differ, the TSV is the source
and this file is wrong.

Inputs: [VISION.md](../project/VISION.md), [ADR-0004](../project/decisions/ADR-0004-design-lock.md), UX_REVIEW.md,
[CONTROL_INVENTORY.md](CONTROL_INVENTORY.md) (what the firmware does today), [ALGORITHM_SET.md](ALGORITHM_SET.md)
(layers, morphs, dice). Panel photo: `docs/controls.jpg`. Screen pixel layout is P2-003, not here.

## 0. Fixed facts used below

- **4 channels** (owner decision 31). Power-on defaults: CH1 `ACID` MIDI 1 unmuted; CH2 `EUCLID` MIDI 2, CH3 `SNAKE`
  MIDI 3, CH4 `STAGE` MIDI 4, all three muted. Global KEY C, SCALE `MINPEN`, OCTAVE 0, GATE 50 %, DIV 1/16, SWING 50 %,
  BPM 120, preset P01, knob row ROW 1 (UI state, not saved), scenes A and B empty. A channel has only a MIDI channel (1..16 or OFF) and an algorithm.
- **Tap versus hold timing.** A *tap* is a press released **under 0.4 s**. EDIT (UNDO) and GLO (SET) use the menu setting HOLD
  (0.3, 0.4, 0.5 or 0.6 s; default **0.4 s**); HOME, SAVE and the two scene keys use a fixed **0.7 s** hold. A button with a hold role
  acts on **release** when tapped; every other control acts on **press**. The six dice buttons have **no hold role**, so they act on press
  (up to 0.4 s earlier than the approved tap-on-release NOTES dice). HOME's hold-plus-control (HELP) cancels the menu hold.
- **Dice sub-labels.** The five layer dice show a shared main label and the selected algorithm's own layer name as a sub-label (`NOTES +sub`); a layer the algorithm lacks is dimmed, inert, and toasts `NO SHAPE IN EUCLID` for 0.5 s.
- **Next-step commit.** Morphs, dice, algorithm and scale changes are heard from the next step. A preset load or a scene recall is heard at the next bar (768 ticks).
- **Momentary state.** SOLO, FILL and ROLL are held controls: they act on press and the state returns on release. They are never latched. (MUTATE was removed by the P1-014 revision.)

## 1. Modes, and how you always know which one you are in

There is no mode you can be lost in: the sequencer boots straight into the main screen, and every other state is
either **momentary** (a held button or key) or **latched with a banner** (SET and REC; both have a banner and an LED or `BACK` cell). Six panel states. Precedence when two could apply: **MENU > SET > RESET > REC >
STEP_HELD > PERF**. HOME tap leaves every state for PERF (the one escape gesture).

| Context | Kind | Entered by | Left by | How you know (always on screen) |
| --- | --- | --- | --- | --- |
| `PERF` | resting state | power-on default | n/a | Main screen: header, 4 channel cells, 16-step lane, two knob rows, button picture, black-key legend, cue strip |
| `STEP_HELD` | momentary | hold a white key (a tap toggles the pin; a hold edits it) | let go of all white keys | knob band replaced by the step band, two rows like the morph rows: row 1 `NOTE ACC SLIDE RATCH`, row 2 `TIE MUTE PROB OCT`; OCT- / OCT+ cells read `STEP 1` / `STEP 2` |
| `SET` | latched (tap GLO) or momentary (hold GLO) | tap or hold GLO | tap GLO, tap HOME, or release a hold | SET page: row 1 `GATE DIV SWING BPM`, row 2 `SCALE OCTAVE MIDI LENGTH` (the KEY is set by pressing any key) (OCT- / OCT+ pick the row); GLO cell reads `BACK`; banner `SET` |
| `RESET` | momentary | hold UNDO past HOLD, or press any control while UNDO is held | let go of UNDO | cells tagged `RST`, buttons relabelled (`RST ALL`, `RST R1`, `RST R2`, `CLR PIN`...), cue strip `RESET: turn a knob = that morph` |
| `REC` | latched | tap REC | tap REC or HOME | banner `REC`, REC LED lit, the knob band becomes one row `CURSOR ACC SLIDE OCT`, the lane shows the write cursor, black-key legend replaced by `KEYS = NOTES`; cue strip `REC: PLAY KEYS TO WRITE  REC=done` |
| `MENU` | latched | hold HOME 0.7 s (with no control pressed) | tap or hold HOME | Felucca menu; every button but PLAY and HOME reads `close` |

**One held modifier at most.** Every combo is "one held thing plus a turn or a press": hold a white key then turn a knob or
press OCT, or hold EDIT (UNDO) then turn a knob or press a button, or hold HOME then touch any control (HELP), or (PERF only) hold OCT- / OCT+ while turning a knob for the momentary row.
The **context** column of the TSV and of Appendix A uses exactly these six names (`PERF`, `STEP_HELD`, `SET`, `RESET`, `REC`, `MENU`).

## 2. The control table: Control | Tap | Hold | Combo | Screen label

Summary in panel order. Per-context detail is Appendix A; where the summary and Appendix A differ, Appendix A (= the TSV) wins.

| Control (printed) | Tap or press | Hold | Combo | Screen label |
| --- | --- | --- | --- | --- |
| `MASTER` (pot) | analogue: preview-synth and USB-audio volume | n/a | n/a | none (no MIDI effect) |
| `EN_SELECT` (SELECT) | turn: select channel CH1..CH4 (clamped, no wrap) | n/a | every context including MENU | `CH n` |
| `EN_ALGO` (ALGORITHM) | turn: algorithm of the selected channel `ACID > EUCLID > SNAKE > STAGE`; lands at the next step; morphs and seeds kept per algorithm (S7) | n/a | in MENU: next/previous tab | algorithm name |
| `EN_PRESET` (PRESETS) | turn: preset slot P01..P16; filled slot loads at the next bar (now if stopped); empty slot only selects the SAVE target | n/a | in MENU: next/previous row | `Pnn`, `Pnn>` while queued, `Pnn EMPTY` |
| `EN_K1` (KNOB 1) | turn: the live row's first function: ROW 1 DENSITY (0 = `KEYS ONLY`), ROW 2 START (`OFFSET`) | n/a | step key held: row 1 `NOTE`, row 2 `TIE`; REC: CURSOR; SET: GATE / SCALE; RESET held: reset this morph; MENU: value | algorithm label (both rows always drawn), `NOTE`, `TIE`, `GATE`, `SCALE`, `RST` |
| `EN_K2` (KNOB 2) | turn: ROW 1 FORM (PEDAL, LEN, LEN, TOTAL), ROW 2 ARTIC (SLIDE, RATCH, GLEN, RDIV) | n/a | step key held: `ACC` / `MUTE`; REC: ACC; SET: DIV / OCTAVE; RESET: reset this morph | algorithm label, `ACC`, `MUTE`, `DIV`, `OCTAVE`, `RST` |
| `EN_K3` (KNOB 3) | turn: ROW 1 PITCH (JUMP, SPAN, RANGE, RANGE), ROW 2 DYNAMICS (PUNCH, PUNCH, DYN, DYN) | n/a | step key held: `SLIDE` / `PROB`; REC: SLIDE; SET: SWING / MIDI (the selected channel's MIDI channel); RESET: reset this morph | algorithm label, `SLIDE`, `PROB`, `SWING`, `MIDI`, `RST` |
| `EN_K4` (KNOB 4) | turn: ROW 1 SPICE (ACCNT, ACCNT, PATH, RATCH), ROW 2 READ (DIR, CURVE, DIR, DIR) | n/a | step key held: `RATCH` / `OCT`; REC: OCT; SET: BPM / LENGTH (pattern length of the selected channel, reserved for P1-016); RESET: reset this morph | algorithm label, `RATCH`, `OCT`, `BPM`, `LENGTH`, `RST` |
| `B_FX` (FX) | press: DICE ALL layers present | n/a | RESET held: reset the whole algorithm (all 8 morphs and all seeds to factory) | `ALL`, `RST ALL` |
| `B_SCL` (SCL) | press: dice NOTES (the scale / pitch layer) | n/a | RESET held: reset the NOTES seed | `NOTES +sub`, `RST NOT` |
| `B_ENV` (ENV) | press: dice ACCNT (envelope accent) | n/a | RESET held: reset the ACCNT seed | `ACCNT +sub`, `RST ACC` |
| `B_LFO` (LFO) | press: dice SHAPE (movement over the bar; inert if none) | n/a | RESET held: reset the SHAPE seed | `SHAPE +sub`, `RST SHP` |
| `B_EDIT` (EDIT) | tap: UNDO / REDO the last dice, dice-all, pin edit, written note, pin clear or reset | past 0.4 s: RESET layer | in RESET: held keeps the layer open | `UNDO`, `RESET` |
| `B_GLO` (GLO) | tap: open the SET page (latched); tap again closes | past 0.4 s: SET page while held | in SET: `BACK`; in REC: inert | `SET`, `BACK` |
| `B_HOME` (HOME) | tap: back to PERF from anywhere (leaves REC, SET, RESET, a held step); in MENU: close | hold: shows `touch = HELP, let go = MENU`; release after 0.7 s with nothing touched: open MENU | touch any control while held: HELP for that control (no time limit; nothing else happens, release does nothing) | `HOME`, `BACK` |
| `B_SAVE` (SAVE) | tap: save the whole device state to the preset slot in the header | 0.7 s: undo the last preset load (again: redo) | n/a | `SAVE Pnn` |
| `B_ARP` (ARP) | press: dice ARTIC (gate, slide, ratchet; inert if none) | n/a | RESET held: reset the ARTIC seed | `ARTIC +sub`, `RST ART` |
| `B_SEQ` (SEQ) | press: dice HITS (where the steps fall) | n/a | RESET held: reset the HITS seed | `HITS +sub`, `RST HIT` |
| `B_PLAY` (PLAY/STOP) | press: start / stop all channels | n/a | every context including MENU | `PLAY / STOP` |
| `B_REC` (REC) | tap: arm RECORD (latched, LED lit, banner `REC`): the 27 keys play pitches and write them; tap again or HOME: disarm | n/a | RESET held: clear all pins of the selected channel | `REC`, `CLR PIN` |
| `B_OCTDN` (OCT-) | PERF: tap latches ROW 1 (a set, not a toggle) | n/a | hold and turn a knob = ROW 1 live while held; step key held: step band row 1; SET: SET row 1; RESET: reset ROW 1; both held 5 s = UPDATE MODE; both at power-on = CALIBRATION | `ROW 1`, `STEP 1`, `SET 1`, `RST R1` |
| `B_OCTUP` (OCT+) | PERF: tap latches ROW 2 | n/a | hold and turn a knob = ROW 2 live while held; step key held: step band row 2; SET: SET row 2; RESET: reset ROW 2; hidden gestures as OCT- | `ROW 2`, `STEP 2`, `SET 2`, `RST R2` |
| White keys F3 G3 A3 B3 C4 D4 E4 F4 G4 A4 B4 C5 D5 E5 F5 G5 (`KEY_W01_F3` .. `KEY_W16_G5`) | press: step n of the viewed page: pin it (auditioned); tap a pinned step again: clear it; with PAGE held: the pattern ends here | hold = STEP_HELD (knobs and OCT edit the held step; pressing ANOTHER white key sets its pitch = that scale degree); REC: plays pitches and writes them; [R4 slot: per-key pressure or position] | SET row 2: set the global KEY; RESET held: clear that step; MENU: inert | `step n`, `KEY`, `CLR n` |
| Black key F#3 G#3 A#3 C#4 (`KEY_B01..B04`, printed OP1..OP4) | press: mute / unmute CH1..CH4 (latched) | n/a | SET row 2: set KEY; REC: plays pitch | `MUTE1`..`MUTE4`, `KEY` |
| Black key D#4 (`KEY_B05`) | press: PAGE: view the next page of the selected channel (wraps) | hold: the next white key pressed sets the pattern length | SET row 2: set KEY | `PAGE`, `KEY` |
| Black key F#4 (`KEY_B06`) | hold: SOLO the selected channel (release restores the mutes) | acts on press | SET row 2: set KEY | `SOLO`, `KEY` |
| Black key G#4 (`KEY_B07`) | hold: FILL (the selected channel plays at maximum density) | acts on press | SET row 2: set KEY | `FILL`, `KEY` |
| Black key A#4 (`KEY_B08`) | hold: ROLL (every step of the selected channel repeats twice) | acts on press | SET row 2: set KEY | `ROLL`, `KEY` |
| Black key C#5 (`KEY_B09`) | tap: recall scene A at the next bar | 0.7 s: store the whole current state into scene A | SET row 2: set KEY | `SCN A`, `KEY` |
| Black key D#5 (`KEY_B10`) | tap: recall scene B at the next bar | 0.7 s: store the whole current state into scene B | SET row 2: set KEY | `SCN B`, `KEY` |
| Black key F#5 (`KEY_B11`) | press: restart all channels from step 1 at the next step | n/a | SET row 2: set KEY | `RESTRT`, `KEY` |

No control is "unused, reserved": all 49 controls (MASTER, 3 encoders, 4 knobs, 14 buttons, 16 white keys and 11 black
keys) have an assigned role in PERF. The only unused combinations are those in the TSV marked `-`: GLO while a step key or EDIT is held or while recording, OCT- / OCT+ and PRESETS while recording,
all 27 keys in MENU (deliberately inert so nothing is pinned or muted by accident), and OCT- / OCT+ inside MENU, which keep Felucca's menu rule (previous / next value).

## 3. Keys: steps versus moves

- **16 white keys (F3..G5, place 0..15) = the 16 steps, always (outside REC).** Press key n: step n is pinned (sounds at
  once; key n is step n of the **viewed page**: with a pattern longer than 16, `PAGE` (D#4) swaps pages; hold PAGE and press a key to end the pattern there; a new pin is the note generated there, or the root when silent). A touch under 30 ms is ignored (brush guard). Tap a pinned key to clear it. **Hold a key** = select that step: K1 `NOTE`, K2 `ACC`, K3 `SLIDE`, K4 `RATCH` (row 1) and, after tapping OCT+, `TIE MUTE PROB OCT` (row 2); **press another white key while holding = that scale degree** (key 1 = the root, key n = degree n-1; sounds at once; replaces the pitch; UNDO restores). K1 at 0 (`KEYS ONLY`) plays only pins.
- **REC = play a tune in.** Tap REC: all 27 keys play real pitches F3..G5 on the selected channel (preview and MIDI) and write each as a pin at the cursor (stopped: the cursor, which advances; playing: the step nearest the playhead). K1 CURSOR, K2 ACC, K3 SLIDE, K4 OCT. EDIT tap takes the last note back. Tap REC or HOME to leave. This is the fast path (17 gestures for 16 pitched steps) and the audition path (every key sounds).
- **11 black keys = performance moves** (outside REC). `F#3 G#3 A#3 C#4` (printed OP1..OP4) MUTE1..4; `D#4` PAGE; `F#4` SOLO; `G#4` FILL; `A#4` ROLL; `C#5` SCENE A; `D#5` SCENE B; `F#5` RESTART. (D#4 PAGE shows `PG 2/4`.) The legend of these labels is always on the screen
  (cue strip band). They behave the same in PERF and RESET, are inert in STEP_HELD (except that a held step key keeps its edit), inert in MENU, and set KEY in SET row 2.
- **Dice are never on keys.** They are buttons. The keys stay free of dice so a performer's hands stay split: left on the buttons, right on the keys.
- **Channel select is not on keys** (the SELECT encoder alone; four detents reach any channel). The four select keys and `ALL ON` of the approved map were given up for SOLO, FILL, ROLL and the scenes.

## 4. Reference workflow, gesture by gesture (TB-3 on MIDI channel 2)

Starting from power-on. No step opens the MENU. Result: **performing within 5 gestures**, every one on the main screen except the SET page.

| # | Gesture | Controls | What the screen shows | Reaches |
| --- | --- | --- | --- | --- |
| 1 | Select channel 1 | turn SELECT to `1` | header cell `1` inverted; knob cells show channel 1's algorithm | W1 |
| 2 | Set MIDI channel 2 | tap GLO (SET page), tap OCT+ (row 2), turn K3 until `MIDI` reads `2`; tap HOME | SET page with both rows; header cell `1 ACID 2` after the escape | W2 |
| 3 | Choose an algorithm | turn ALGORITHM: `ACID > EUCLID > SNAKE > STAGE` (lands at the next step; its own morphs and seeds are restored) | algorithm name in large type | W3 |
| 4a | Dial a sequence in | press PLAY; turn K1 (density), K2 (form), K3 (pitch width), K4 (spice) | each knob cell highlights for 1 s and shows its value | W5, W6 |
| 4b | or enter steps | turn K1 to `KEYS ONLY`; tap white keys to pin steps; tap REC and play the pitches (or hold a step key and press another white key) | pins outlined on the lane; the pin sounds | W4 |
| 5 | Perform | morph with K1-K4, tap OCT+ for ROW 2; dice with FX (all) SCL (NOTES) SEQ (HITS) ENV (ACCNT) ARP (ARTIC) LFO (SHAPE); EDIT undoes; mute with F#3..C#4; hold G#4 to fill | every button shows its role; muted channel cell dimmed with `M` | W6, W7, W10 |
| 6 | Optional global changes | tap GLO: row 1 GATE, DIV, SWING, BPM; row 2 SCALE, OCTAVE, MIDI, LENGTH (any key sets the KEY); SAVE to keep it | header `C MINPEN` on every screen | W8, W9, W11, W12 |

Gestures 1-3 plus PLAY make sound on a TB-3; the only latched page is SET. Save and load: tap SAVE
(`SAVED P01`); a filled slot loads at the next bar and the header shows `P03>` until then (decision 34).

## 5. What stays in the MENU (hold HOME 0.7 s): settings only

ALGORITHM = tab, PRESETS = row, K1-K4 = value, HOME tap closes (Felucca menu navigation, unchanged).

- **DISPLAY:** COLOR, STYLE, LARGE, ANIM, LEDS, SCREEN OFF, SCOPE, HOME view.
- **CONTROL:** HOLD (the 0.3-0.6 s tap/hold threshold), ACCEL, SCALE LEDS, STEP PREVIEW.
- **AUDIO:** SPEAKER EQ, USB LEVEL, CLICK, CLICK LEVEL, COUNT-IN, TUNE.
- **MIDI:** MIDI IN routing, AUTO SELECT, SCREEN WAKE, LEARN CLEAR.
- **SYSTEM:** USB SERIAL,
  RESTORE LAST, HELP, CALIBRATION, INFO, ABOUT.

Moved out because they are musical functions now on the panel: `MI_TCH1..4` (SET page row 2, K4 MIDI of the selected channel), `MI_SELECT` (SELECT is fixed
to channel select), `MI_BPMLOCK` (tempo is on SET K4), `MI_T1SEQ` (the sequencer is always on, decision 32b), `MI_LATCH` (no FX layer). Row counts change in `menu_items.c`, so the dev task updates `tests/ui_test.c`.

## 6. Conflicts from the T-1 guide, resolved

`TORSO_T1_GUIDE.md` section 2 disagrees with the code and with itself (CONTROL_INVENTORY 1.4). The v1 map replaces all of
it; this table says what each guide claim becomes.

| Guide claim / T-1 behaviour | Reality today | Resolution in this map |
| --- | --- | --- |
| REC arms real-time recording | REC is unreferenced in T1 mode | REC = RECORD (tap): the keys play and write pitches; pins are entered by pressing white keys (no STEP mode); clear all pins = hold EDIT (UNDO) + REC |
| Keys 1-16 hold = step P-LOCK | keys select track/cycle; `edit_step` never set | White key toggles a pin; hold it + K1 NOTE / K2 ACC / K3 SLIDE / K4 RATCH (and row 2 TIE MUTE PROB OCT) edits the pin and its flags, or press another white key for its pitch; P-LOCK is dropped |
| Page navigation via SEQ / OCT- / OCT+ | SEQ next page, OCT- previous, OCT+ is SHIFT | No pages; one SHIFT row: OCT- / OCT+ select ROW 1 / ROW 2 of the knobs (labelled on screen, FULL_CONTROL.md); GLO is the SET page; pin octave stays on OCT in a held step; there is no keyboard octave (no live keys) |
| 6 tracks x 6 cycles | 6 tracks, track 1 on the synth | 4 channels (decision 31); every channel sends MIDI; no cycles |
| Black keys 1-6 mute, white keys 1-6 select track, 7-12 cycle | as the guide | Black keys F#3..C#4 mute CH1-4 and the rest are moves; SELECT selects; white keys are the steps |
| SAVE or LFO held = TEMP snapshot; OCT+ commits | ui_t1.c:56-68 | SAVE tap saves the preset, SAVE hold 0.7 s undoes a load; TEMP is dropped |
| GLO held = relative delta on all tracks | ui_t1.c:69-72 | GLO opens the SET page: GATE, DIV, SWING, BPM (row 1) are global by nature (decision 4); no per-track delta |
| Page 4 K4 shift = "LoopSync", MOD/CC page, RATCHET page, HARMONY page | chord voices etc. | Not in v1; ratchet is the ARTIC layer and scale / key live on SET row 2 |
| LFO-held MOMENTARY (Felucca) | knob moves snap back on release | Replaced by one-control moves FILL, ROLL, SOLO and by dice + EDIT (UNDO); a hold + turn would score input 1, below the performance bar, and MUTATE was removed (PERFORMANCE_DESIGN.md section 4) |

## 7. Flex points (ADR-0004 addendum)

What a producer can re-map by **editing one row**, with no algorithm or screen code change.

| Table | Row / field | What a row edit changes |
| --- | --- | --- |
| `control_map.tsv` | action, label for a (control, context) | Any button, encoder, knob or key role in any context, for example swap two dice, move UNDO, move mutes or moves to other black keys |
| Context table (section 1) | hold thresholds, enter/leave gesture | Make SET hold-only, move UNDO to another button, change the scene hold time, change the 0.4 s default via the HOLD menu setting |
| Knob table (UX_REVIEW 5.1, FULL_CONTROL.md section 3) | slot, label, range, default, step, fast step | Remap K1..K4 and S1..S4 per algorithm, rename `PEDAL`, change SNAKE RANGE default, STAGE TOTAL coarse step |
| Dice table (UX_REVIEW 5.2) | button -> layer, sub-label, seeds | Bind SLIDE alone to its own gesture, rename a sub-label, mark a layer absent |
| Global table (UX_REVIEW 6) | scale list order, defaults | Default scale, key, octave; scale order on K2 |
| Black-key rows | key -> MUTEn / PAGE / SOLO / FILL / ROLL / SCN A / SCN B / RESTRT | Re-place any move; 6 channels would need two more mute keys (give up TAP and ROLL) |
| Defaults (section 0) | channel algorithms, MIDI channels, mutes | Power-on state |

Not changeable without code: two knob rows (eight slots); the six contexts and their precedence; tap-on-release for buttons with a hold role
(`btn_hold`, `ui_layer.c`); the five shared dice layers and their meanings; scale snap rule and output order; the queued
preset load at the loop start; the main-screen bands.


The per-control reset, scene and REC state is in ADR-0003 (section 11) and PERFORMANCE_DESIGN.md section 9.

## Open owner questions from this consolidation

None from this consolidation. Questions from the performance-first pass are 55 to 59 and 60 to 64 (revision) in OWNER_QUESTIONS.md.

## Appendix A. Every (control, context) row, verbatim from control_map.tsv

Columns: control, context, action, screen label. Checked by `python3 tools/check_control_map.py`.

| Control | Context | Action | Label |
| --- | --- | --- | --- |
<!-- BEGIN-APPENDIX-A -->
| `B_FX` | `PERF` | press (acts on press, no hold role): DICE ALL layers present in the channel's algorithm (order NOTES HITS ACCNT ARTIC SHAPE) | `ALL` |
| `B_FX` | `STEP_HELD` | press (acts on press, no hold role): DICE ALL layers present in the channel's algorithm (order NOTES HITS ACCNT ARTIC SHAPE) | `ALL` |
| `B_FX` | `SET` | press (acts on press, no hold role): DICE ALL layers present in the channel's algorithm (order NOTES HITS ACCNT ARTIC SHAPE) | `ALL` |
| `B_FX` | `RESET` | press: reset the selected channel's current algorithm to its factory state: all 8 morphs to their defaults AND every seed to its factory seed (pins and flags kept); UNDO tap restores | `RST ALL` |
| `B_FX` | `REC` | press (acts on press, no hold role): DICE ALL layers present in the channel's algorithm (order NOTES HITS ACCNT ARTIC SHAPE) | `ALL` |
| `B_FX` | `MENU` | press: closes the menu (no other action) | `close` |
| `B_SCL` | `PERF` | press (acts on press, no hold role): dice NOTES | `NOTES +sub` |
| `B_SCL` | `STEP_HELD` | press (acts on press, no hold role): dice NOTES | `NOTES +sub` |
| `B_SCL` | `SET` | press (acts on press, no hold role): dice NOTES | `NOTES +sub` |
| `B_SCL` | `RESET` | press: reset the NOTES layer's seed to its factory seed | `RST NOT` |
| `B_SCL` | `REC` | press (acts on press, no hold role): dice NOTES | `NOTES +sub` |
| `B_SCL` | `MENU` | press: closes the menu (no other action) | `close` |
| `B_ENV` | `PERF` | press (acts on press, no hold role): dice ACCNT | `ACCNT +sub` |
| `B_ENV` | `STEP_HELD` | press (acts on press, no hold role): dice ACCNT | `ACCNT +sub` |
| `B_ENV` | `SET` | press (acts on press, no hold role): dice ACCNT | `ACCNT +sub` |
| `B_ENV` | `RESET` | press: reset the ACCNT layer's seed to its factory seed | `RST ACC` |
| `B_ENV` | `REC` | press (acts on press, no hold role): dice ACCNT | `ACCNT +sub` |
| `B_ENV` | `MENU` | press: closes the menu (no other action) | `close` |
| `B_LFO` | `PERF` | press (acts on press, no hold role): dice SHAPE (dimmed and inert if the algorithm has no SHAPE layer) | `SHAPE +sub` |
| `B_LFO` | `STEP_HELD` | press (acts on press, no hold role): dice SHAPE (dimmed and inert if the algorithm has no SHAPE layer) | `SHAPE +sub` |
| `B_LFO` | `SET` | press (acts on press, no hold role): dice SHAPE (dimmed and inert if the algorithm has no SHAPE layer) | `SHAPE +sub` |
| `B_LFO` | `RESET` | press: reset the SHAPE layer's seed to its factory seed (inert if the algorithm has none) | `RST SHP` |
| `B_LFO` | `REC` | press (acts on press, no hold role): dice SHAPE (dimmed and inert if the algorithm has no SHAPE layer) | `SHAPE +sub` |
| `B_LFO` | `MENU` | press: closes the menu (no other action) | `close` |
| `B_EDIT` | `PERF` | tap (release before HOLD): UNDO / REDO the last dice, dice-all, pin edit, written note, pin-clear or reset (swaps current and previous; one record per channel; after a preset load there is nothing to undo, toast `SAVE hold undoes a load`); hold past HOLD, or any control pressed while it is held: open the RESET layer (no undo) | `UNDO` |
| `B_EDIT` | `STEP_HELD` | tap: UNDO / REDO as in PERF; hold: nothing (the RESET layer opens from PERF only) | `UNDO` |
| `B_EDIT` | `SET` | tap: UNDO / REDO as in PERF; hold: nothing (the RESET layer opens from PERF only) | `UNDO` |
| `B_EDIT` | `RESET` | held: keeps the RESET layer open; release closes it and does not undo | `RESET` |
| `B_EDIT` | `REC` | tap: UNDO / REDO the last written note (the cursor steps back to it); hold: nothing (leave REC to reach RESET) | `UNDO` |
| `B_EDIT` | `MENU` | press: closes the menu (no other action) | `close` |
| `B_GLO` | `PERF` | tap: open the SET page (latched, row 1 first); hold past HOLD: SET page while held | `SET` |
| `B_GLO` | `STEP_HELD` | inert while a step key is held (finish the step edit first) | `-` |
| `B_GLO` | `SET` | tap: close the SET page (same as HOME tap); hold: keeps it open while held | `BACK` |
| `B_GLO` | `RESET` | inert while UNDO is held | `-` |
| `B_GLO` | `REC` | inert while recording (toast `REC: tap REC to finish`) | `-` |
| `B_GLO` | `MENU` | press: closes the menu (no other action) | `close` |
| `B_HOME` | `PERF` | tap (release before 0.7 s): back to PERF from anywhere (in PERF the cue strip shows `HOME=back  hold=MENU/HELP` for 3 s); hold: the cue strip shows `touch = HELP, let go = MENU` at once; touch or turn any control while it is held = HELP for that control (no time limit; the control does nothing; releasing HOME then does nothing); release after 0.7 s with no control touched = open the MENU | `HOME` |
| `B_HOME` | `STEP_HELD` | tap (release before 0.7 s): back to PERF from anywhere (in PERF the cue strip shows `HOME=back  hold=MENU/HELP` for 3 s); hold: the cue strip shows `touch = HELP, let go = MENU` at once; touch or turn any control while it is held = HELP for that control (no time limit; the control does nothing; releasing HOME then does nothing); release after 0.7 s with no control touched = open the MENU | `HOME` |
| `B_HOME` | `SET` | tap (release before 0.7 s): back to PERF from anywhere (in PERF the cue strip shows `HOME=back  hold=MENU/HELP` for 3 s); hold: the cue strip shows `touch = HELP, let go = MENU` at once; touch or turn any control while it is held = HELP for that control (no time limit; the control does nothing; releasing HOME then does nothing); release after 0.7 s with no control touched = open the MENU | `HOME` |
| `B_HOME` | `RESET` | tap: close the RESET layer and return to PERF; hold + control: HELP (same rule as PERF) | `HOME` |
| `B_HOME` | `REC` | tap (release before 0.7 s): leave REC (back to PERF) (in PERF the cue strip shows `HOME=back  hold=MENU/HELP` for 3 s); hold: the cue strip shows `touch = HELP, let go = MENU` at once; touch or turn any control while it is held = HELP for that control (no time limit; the control does nothing; releasing HOME then does nothing); release after 0.7 s with no control touched = open the MENU | `HOME` |
| `B_HOME` | `MENU` | tap or hold 0.7 s: close the menu | `BACK` |
| `B_SAVE` | `PERF` | tap: save the whole device state into the preset slot shown in the header; hold 0.7 s: undo the last preset load (again: redo) | `SAVE Pnn` |
| `B_SAVE` | `STEP_HELD` | tap: save the whole device state into the preset slot shown in the header; hold 0.7 s: undo the last preset load (again: redo) | `SAVE Pnn` |
| `B_SAVE` | `SET` | tap: save the whole device state into the preset slot shown in the header; hold 0.7 s: undo the last preset load (again: redo) | `SAVE Pnn` |
| `B_SAVE` | `RESET` | tap: save the whole device state into the preset slot shown in the header; hold 0.7 s: undo the last preset load (again: redo) | `SAVE Pnn` |
| `B_SAVE` | `REC` | tap: save the whole device state into the preset slot shown in the header; hold 0.7 s: undo the last preset load (again: redo) | `SAVE Pnn` |
| `B_SAVE` | `MENU` | press: closes the menu (no other action) | `close` |
| `B_ARP` | `PERF` | press (acts on press, no hold role): dice ARTIC (dimmed and inert if the algorithm has no ARTIC layer) | `ARTIC +sub` |
| `B_ARP` | `STEP_HELD` | press (acts on press, no hold role): dice ARTIC (dimmed and inert if the algorithm has no ARTIC layer) | `ARTIC +sub` |
| `B_ARP` | `SET` | press (acts on press, no hold role): dice ARTIC (dimmed and inert if the algorithm has no ARTIC layer) | `ARTIC +sub` |
| `B_ARP` | `RESET` | press: reset the ARTIC layer's seed to its factory seed (inert if the algorithm has none) | `RST ART` |
| `B_ARP` | `REC` | press (acts on press, no hold role): dice ARTIC (dimmed and inert if the algorithm has no ARTIC layer) | `ARTIC +sub` |
| `B_ARP` | `MENU` | press: closes the menu (no other action) | `close` |
| `B_SEQ` | `PERF` | press (acts on press, no hold role): dice HITS | `HITS +sub` |
| `B_SEQ` | `STEP_HELD` | press (acts on press, no hold role): dice HITS | `HITS +sub` |
| `B_SEQ` | `SET` | press (acts on press, no hold role): dice HITS | `HITS +sub` |
| `B_SEQ` | `RESET` | press: reset the HITS layer's seed to its factory seed | `RST HIT` |
| `B_SEQ` | `REC` | press (acts on press, no hold role): dice HITS | `HITS +sub` |
| `B_SEQ` | `MENU` | press: closes the menu (no other action) | `close` |
| `B_PLAY` | `PERF` | press: start / stop all channels | `PLAY / STOP` |
| `B_PLAY` | `STEP_HELD` | press: start / stop all channels | `PLAY / STOP` |
| `B_PLAY` | `SET` | press: start / stop all channels | `PLAY / STOP` |
| `B_PLAY` | `RESET` | press: start / stop all channels | `PLAY / STOP` |
| `B_PLAY` | `REC` | press: start / stop all channels (RECORD stays armed; playing, a note lands on the step nearest the playhead) | `PLAY / STOP` |
| `B_PLAY` | `MENU` | press: start / stop all channels | `PLAY / STOP` |
| `B_REC` | `PERF` | tap (release before HOLD): arm RECORD (latched, LED lit, banner `REC`): the 27 keys now PLAY pitches F3..G5 on the selected channel and WRITE each note into the step at the cursor (stopped) or the step nearest the playhead (playing); tap again or HOME: disarm | `REC` |
| `B_REC` | `STEP_HELD` | tap: arm RECORD (the held step stays selected until released; the key then plays pitches) | `REC` |
| `B_REC` | `SET` | tap: close SET and arm RECORD | `REC` |
| `B_REC` | `RESET` | press: CLEAR PINS: remove every pin and per-step flag of the selected channel (UNDO tap restores) | `CLR PIN` |
| `B_REC` | `REC` | tap: disarm RECORD (back to PERF; every written note stays and is one UNDO away) | `REC` |
| `B_REC` | `MENU` | press: closes the menu (no other action) | `close` |
| `B_OCTDN` | `PERF` | tap (release before HOLD, no knob turned): latch ROW 1 of the knob band (a set, not a toggle); hold and turn a knob: ROW 1 is live while held, then the latched row returns; both OCT held 5 s = UPDATE MODE; both at power-on = CALIBRATION | `ROW 1` |
| `B_OCTDN` | `STEP_HELD` | tap: step band row 1 (K1..K4 edit that row of the held step); hold and turn: row 1 while held | `STEP 1` |
| `B_OCTDN` | `SET` | tap: SET row 1 (hold: SET row 1 while held); the morph latch is untouched | `SET 1` |
| `B_OCTDN` | `RESET` | press: reset ROW 1 (its four morphs) of the selected channel to the defaults; UNDO tap restores | `RST R1` |
| `B_OCTDN` | `REC` | inert while recording (the REC band is one row; toast `REC: tap REC to finish`) | `-` |
| `B_OCTDN` | `MENU` | release: previous value / back (Felucca menu rule) | `-` |
| `B_OCTUP` | `PERF` | tap (release before HOLD, no knob turned): latch ROW 2 of the knob band (a set, not a toggle); hold and turn a knob: ROW 2 is live while held, then the latched row returns; both OCT held 5 s = UPDATE MODE; both at power-on = CALIBRATION | `ROW 2` |
| `B_OCTUP` | `STEP_HELD` | tap: step band row 2 (K1..K4 edit that row of the held step); hold and turn: row 2 while held | `STEP 2` |
| `B_OCTUP` | `SET` | tap: SET row 2 (hold: SET row 2 while held); the morph latch is untouched | `SET 2` |
| `B_OCTUP` | `RESET` | press: reset ROW 2 (its four morphs) of the selected channel to the defaults; UNDO tap restores | `RST R2` |
| `B_OCTUP` | `REC` | inert while recording (the REC band is one row; toast `REC: tap REC to finish`) | `-` |
| `B_OCTUP` | `MENU` | release: next value / do it (opens CALIBRATION, INFO, ABOUT rows) | `-` |
| `EN_SELECT` | `PERF` | turn: select channel CH1..CH4 (clamped, no wrap) | `CH n` |
| `EN_SELECT` | `STEP_HELD` | turn: select channel CH1..CH4 (clamped, no wrap) | `CH n` |
| `EN_SELECT` | `SET` | turn: select channel CH1..CH4 (clamped, no wrap) | `CH n` |
| `EN_SELECT` | `RESET` | turn: select channel CH1..CH4 (clamped, no wrap) | `CH n` |
| `EN_SELECT` | `REC` | turn: select channel CH1..CH4 (the cursor goes to step 1) | `CH n` |
| `EN_SELECT` | `MENU` | turn: select channel CH1..CH4 (clamped, no wrap) | `CH n` |
| `EN_ALGO` | `PERF` | turn: algorithm of the selected channel ACID > EUCLID > SNAKE > STAGE (clamped); lands at the next step; morphs and seeds kept per channel (S7) | `ALGO name` |
| `EN_ALGO` | `STEP_HELD` | turn: algorithm of the selected channel ACID > EUCLID > SNAKE > STAGE (clamped); lands at the next step; morphs and seeds kept per channel (S7) | `ALGO name` |
| `EN_ALGO` | `SET` | turn: algorithm of the selected channel ACID > EUCLID > SNAKE > STAGE (clamped); lands at the next step; morphs and seeds kept per channel (S7) | `ALGO name` |
| `EN_ALGO` | `RESET` | turn: algorithm of the selected channel ACID > EUCLID > SNAKE > STAGE (clamped); lands at the next step; morphs and seeds kept per channel (S7) | `ALGO name` |
| `EN_ALGO` | `REC` | turn: algorithm of the selected channel ACID > EUCLID > SNAKE > STAGE (clamped); lands at the next step; morphs and seeds kept per channel (S7) | `ALGO name` |
| `EN_ALGO` | `MENU` | turn: next / previous menu tab (wraps) | `tab` |
| `EN_PRESET` | `PERF` | turn: preset slot P01..P16; a filled slot loads at the next bar (immediately when stopped); an empty slot only selects the target for SAVE | `Pnn` |
| `EN_PRESET` | `STEP_HELD` | turn: preset slot P01..P16; a filled slot loads at the next bar (immediately when stopped); an empty slot only selects the target for SAVE | `Pnn` |
| `EN_PRESET` | `SET` | turn: preset slot P01..P16; a filled slot loads at the next bar (immediately when stopped); an empty slot only selects the target for SAVE | `Pnn` |
| `EN_PRESET` | `RESET` | turn: preset slot P01..P16; a filled slot loads at the next bar (immediately when stopped); an empty slot only selects the target for SAVE | `Pnn` |
| `EN_PRESET` | `REC` | inert while recording (no preset loads under a take; toast `REC: tap REC to finish`) | `-` |
| `EN_PRESET` | `MENU` | turn: next / previous menu row (wraps) | `row` |
| `MASTER` | `PERF` | analogue pot: preview-synth and USB-audio volume (song.master_q12); no MIDI effect | `-` |
| `MASTER` | `STEP_HELD` | analogue pot: preview-synth and USB-audio volume (song.master_q12); no MIDI effect | `-` |
| `MASTER` | `SET` | analogue pot: preview-synth and USB-audio volume (song.master_q12); no MIDI effect | `-` |
| `MASTER` | `RESET` | analogue pot: preview-synth and USB-audio volume (song.master_q12); no MIDI effect | `-` |
| `MASTER` | `REC` | analogue pot: preview-synth and USB-audio volume (song.master_q12); no MIDI effect | `-` |
| `MASTER` | `MENU` | analogue pot: preview-synth and USB-audio volume (song.master_q12); no MIDI effect | `-` |
| `EN_K1` | `PERF` | turn: morph of the LIVE ROW. ROW 1 (latched by tapping OCT-, the power-on row) = K1 DENSITY: ACID DENS 0..16 / EUCLID HITS 0..16 / SNAKE DENS 0..16 / STAGE DENS 0..8; 0 = KEYS ONLY (only pins play). ROW 2 (latched by tapping OCT+; or momentary while OCT+ is held, or OCT- for ROW 1) = S1 START: OFFSET (ACID 0..15 / EUCLID 0..15 / SNAKE 0..15 / STAGE 0..63). Details FULL_CONTROL.md section 3 | `algo label` |
| `EN_K1` | `STEP_HELD` | turn: step band row 1 (OCT- or the default): the held step's pin NOTE, one scale degree per detent (auditioned); row 2 (tap OCT+): TIE flag (the previous step's note is held through it), turn right = ON, left = OFF | `NOTE / TIE` |
| `EN_K1` | `SET` | turn: SET row 1 = GATE 10..100 % (50 % neutral); SET row 2 = global SCALE (the KEY is set by pressing any key while row 2 shows) | `GATE / SCALE` |
| `EN_K1` | `RESET` | turn: reset that ONE morph of the live row (DENSITY slot of the latched ROW 1 or ROW 2) to its default; the cell reads RST; further turns of the same knob do nothing until UNDO is released and held again | `RST` |
| `EN_K1` | `REC` | turn: CURSOR: move the write cursor one step per detent (stopped: the step plays so you can hear where you are; wraps at the pattern length) | `CURSOR` |
| `EN_K1` | `MENU` | turn: change the selected row's value (K1..K4 equivalent) | `value` |
| `EN_K2` | `PERF` | turn: morph of the LIVE ROW. ROW 1 (latched by tapping OCT-, the power-on row) = K2 FORM: ACID PEDAL 0..12 / EUCLID LEN 2..16 / SNAKE LEN 1..16 / STAGE TOTAL 8..64 (step 4 when turned fast, 1 when slow). ROW 2 (latched by tapping OCT+; or momentary while OCT+ is held, or OCT- for ROW 1) = S2 ARTIC: ACID SLIDE 0..8 / EUCLID RATCH 0..8 / SNAKE GLEN 1..16 / STAGE RDIV 1..4. Details FULL_CONTROL.md section 3 | `algo label` |
| `EN_K2` | `STEP_HELD` | turn: row 1: ACC accent flag of the held step, right = ON, left = OFF; row 2: MUTE (the step does not sound even if the algorithm hits it), right = ON, left = OFF | `ACC / MUTE` |
| `EN_K2` | `SET` | turn: SET row 1 = DIV 1/4..1/32; SET row 2 = global OCTAVE -3..+3 | `DIV / OCTAVE` |
| `EN_K2` | `RESET` | turn: reset that ONE morph of the live row (FORM slot of the latched ROW 1 or ROW 2) to its default; the cell reads RST; further turns of the same knob do nothing until UNDO is released and held again | `RST` |
| `EN_K2` | `REC` | turn: ACC for the notes written next: ON / OFF (accent flag) | `ACC` |
| `EN_K2` | `MENU` | turn: change the selected row's value (K1..K4 equivalent) | `value` |
| `EN_K3` | `PERF` | turn: morph of the LIVE ROW. ROW 1 (latched by tapping OCT-, the power-on row) = K3 PITCH: ACID JUMP 0..20 / EUCLID SPAN 0..24 / SNAKE RANGE 0..48 / STAGE RANGE 0..36 (step 4 when turned fast on RANGE, 1 when slow). ROW 2 (latched by tapping OCT+; or momentary while OCT+ is held, or OCT- for ROW 1) = S3 DYNAMICS: ACID PUNCH 0..47 / EUCLID PUNCH 0..47 / SNAKE DYN 0..10 / STAGE DYN 0..10. Details FULL_CONTROL.md section 3 | `algo label` |
| `EN_K3` | `STEP_HELD` | turn: row 1: SLIDE flag (glide into that step), right = ON, left = OFF; row 2: PROB probability of the step sounding: 100, 75, 50, 25 % (a seeded draw: the same pass plays the same way; P1-016 fixes the draw) | `SLIDE / PROB` |
| `EN_K3` | `SET` | turn: SET row 1 = SWING 50..75 %; SET row 2 = MIDI channel of the SELECTED channel 1..16 or OFF | `SWING / MIDI` |
| `EN_K3` | `RESET` | turn: reset that ONE morph of the live row (PITCH slot of the latched ROW 1 or ROW 2) to its default; the cell reads RST; further turns of the same knob do nothing until UNDO is released and held again | `RST` |
| `EN_K3` | `REC` | turn: SLIDE for the notes written next: ON / OFF (slide into the next step) | `SLIDE` |
| `EN_K3` | `MENU` | turn: change the selected row's value (K1..K4 equivalent) | `value` |
| `EN_K4` | `PERF` | turn: morph of the LIVE ROW. ROW 1 (latched by tapping OCT-, the power-on row) = K4 SPICE: ACID ACCNT 0..8 / EUCLID ACCNT 0..8 / SNAKE PATH 1..14 / STAGE RATCH 0..8. ROW 2 (latched by tapping OCT+; or momentary while OCT+ is held, or OCT- for ROW 1) = S4 READ: ACID DIR 0..2 / EUCLID CURVE 0..6 / SNAKE DIR 0..2 / STAGE DIR 0..2. Details FULL_CONTROL.md section 3 | `algo label` |
| `EN_K4` | `STEP_HELD` | turn: row 1: RATCH repeats in the step: 1x, 2x, 3x, 4x; row 2: OCT pin octave -2..+2 (in steps of 12 semitones) | `RATCH / OCT` |
| `EN_K4` | `SET` | turn: SET row 1 = BPM 40..240 (EXT when slaved); SET row 2 = pattern length of the SELECTED channel in steps (R1; range, default and the relation to the algorithm LEN morph are filled by P1-016; the slot is reserved here) | `BPM / LENGTH` |
| `EN_K4` | `RESET` | turn: reset that ONE morph of the live row (SPICE slot of the latched ROW 1 or ROW 2) to its default; the cell reads RST; further turns of the same knob do nothing until UNDO is released and held again | `RST` |
| `EN_K4` | `REC` | turn: OCT: octave of the keys -2..+2 (added to the global OCTAVE; applies to the next key) | `OCT` |
| `EN_K4` | `MENU` | turn: change the selected row's value (K1..K4 equivalent) | `value` |
| `KEY_W01_F3` | `PERF` | press (a touch under 30 ms is ignored): step (viewed page - 1) x 16 + 1 (past the pattern length the key is dim and inert). An unpinned step gets a pin now (the note generated there, or the root when silent; auditioned). A pinned step is cleared on release if it was held under 0.4 s and no knob or OCT was used; otherwise the held step goes to STEP_HELD. While PAGE (D#4) is held: the pattern of the selected channel ends at this step (LENGTH). [R4 slot: per-key pressure / position on a held step key writes the armed step property, reserved, P1-015] | `step 1` |
| `KEY_W01_F3` | `STEP_HELD` | press while another white key is held: set the HELD step's pitch to the ROOT (degree 0; sounds); pressed first, it is the held step and the knobs edit it | `pitch / step 1` |
| `KEY_W01_F3` | `SET` | press: SET row 2 (the cue strip says KEYS SET THE KEY): set the global KEY to the pitch class of F3; SET row 1: step 1 as PERF | `KEY / step` |
| `KEY_W01_F3` | `RESET` | press: clear the pin and flags of step 1 (UNDO tap restores) | `CLR 1` |
| `KEY_W01_F3` | `REC` | press: PLAY the pitch F3 (+ the REC OCT K4 and the global OCTAVE) on the selected channel (MIDI + preview), WRITE it as a pin at the cursor with the ACC and SLIDE flags, then the cursor advances; release: note off. A press under 30 ms is ignored | `note` |
| `KEY_W01_F3` | `MENU` | inert while the menu is open (no accidental pins) | `-` |
| `KEY_W02_G3` | `PERF` | press (a touch under 30 ms is ignored): step (viewed page - 1) x 16 + 2 (past the pattern length the key is dim and inert). An unpinned step gets a pin now (the note generated there, or the root when silent; auditioned). A pinned step is cleared on release if it was held under 0.4 s and no knob or OCT was used; otherwise the held step goes to STEP_HELD. While PAGE (D#4) is held: the pattern of the selected channel ends at this step (LENGTH). [R4 slot: per-key pressure / position on a held step key writes the armed step property, reserved, P1-015] | `step 2` |
| `KEY_W02_G3` | `STEP_HELD` | press while another white key is held: set the HELD step's pitch to scale degree 1 (key 1 = the root, one scale degree per key; the note sounds; OCT+ then K4 OCT shifts the octave); a second press replaces it; UNDO tap restores | `pitch` |
| `KEY_W02_G3` | `SET` | press: SET row 2 (the cue strip says KEYS SET THE KEY): set the global KEY to the pitch class of G3; SET row 1: step 2 as PERF | `KEY / step` |
| `KEY_W02_G3` | `RESET` | press: clear the pin and flags of step 2 (UNDO tap restores) | `CLR 2` |
| `KEY_W02_G3` | `REC` | press: PLAY the pitch G3 (+ the REC OCT K4 and the global OCTAVE) on the selected channel (MIDI + preview), WRITE it as a pin at the cursor with the ACC and SLIDE flags, then the cursor advances; release: note off. A press under 30 ms is ignored | `note` |
| `KEY_W02_G3` | `MENU` | inert while the menu is open (no accidental pins) | `-` |
| `KEY_W03_A3` | `PERF` | press (a touch under 30 ms is ignored): step (viewed page - 1) x 16 + 3 (past the pattern length the key is dim and inert). An unpinned step gets a pin now (the note generated there, or the root when silent; auditioned). A pinned step is cleared on release if it was held under 0.4 s and no knob or OCT was used; otherwise the held step goes to STEP_HELD. While PAGE (D#4) is held: the pattern of the selected channel ends at this step (LENGTH). [R4 slot: per-key pressure / position on a held step key writes the armed step property, reserved, P1-015] | `step 3` |
| `KEY_W03_A3` | `STEP_HELD` | press while another white key is held: set the HELD step's pitch to scale degree 2 (key 1 = the root, one scale degree per key; the note sounds; OCT+ then K4 OCT shifts the octave); a second press replaces it; UNDO tap restores | `pitch` |
| `KEY_W03_A3` | `SET` | press: SET row 2 (the cue strip says KEYS SET THE KEY): set the global KEY to the pitch class of A3; SET row 1: step 3 as PERF | `KEY / step` |
| `KEY_W03_A3` | `RESET` | press: clear the pin and flags of step 3 (UNDO tap restores) | `CLR 3` |
| `KEY_W03_A3` | `REC` | press: PLAY the pitch A3 (+ the REC OCT K4 and the global OCTAVE) on the selected channel (MIDI + preview), WRITE it as a pin at the cursor with the ACC and SLIDE flags, then the cursor advances; release: note off. A press under 30 ms is ignored | `note` |
| `KEY_W03_A3` | `MENU` | inert while the menu is open (no accidental pins) | `-` |
| `KEY_W04_B3` | `PERF` | press (a touch under 30 ms is ignored): step (viewed page - 1) x 16 + 4 (past the pattern length the key is dim and inert). An unpinned step gets a pin now (the note generated there, or the root when silent; auditioned). A pinned step is cleared on release if it was held under 0.4 s and no knob or OCT was used; otherwise the held step goes to STEP_HELD. While PAGE (D#4) is held: the pattern of the selected channel ends at this step (LENGTH). [R4 slot: per-key pressure / position on a held step key writes the armed step property, reserved, P1-015] | `step 4` |
| `KEY_W04_B3` | `STEP_HELD` | press while another white key is held: set the HELD step's pitch to scale degree 3 (key 1 = the root, one scale degree per key; the note sounds; OCT+ then K4 OCT shifts the octave); a second press replaces it; UNDO tap restores | `pitch` |
| `KEY_W04_B3` | `SET` | press: SET row 2 (the cue strip says KEYS SET THE KEY): set the global KEY to the pitch class of B3; SET row 1: step 4 as PERF | `KEY / step` |
| `KEY_W04_B3` | `RESET` | press: clear the pin and flags of step 4 (UNDO tap restores) | `CLR 4` |
| `KEY_W04_B3` | `REC` | press: PLAY the pitch B3 (+ the REC OCT K4 and the global OCTAVE) on the selected channel (MIDI + preview), WRITE it as a pin at the cursor with the ACC and SLIDE flags, then the cursor advances; release: note off. A press under 30 ms is ignored | `note` |
| `KEY_W04_B3` | `MENU` | inert while the menu is open (no accidental pins) | `-` |
| `KEY_W05_C4` | `PERF` | press (a touch under 30 ms is ignored): step (viewed page - 1) x 16 + 5 (past the pattern length the key is dim and inert). An unpinned step gets a pin now (the note generated there, or the root when silent; auditioned). A pinned step is cleared on release if it was held under 0.4 s and no knob or OCT was used; otherwise the held step goes to STEP_HELD. While PAGE (D#4) is held: the pattern of the selected channel ends at this step (LENGTH). [R4 slot: per-key pressure / position on a held step key writes the armed step property, reserved, P1-015] | `step 5` |
| `KEY_W05_C4` | `STEP_HELD` | press while another white key is held: set the HELD step's pitch to scale degree 4 (key 1 = the root, one scale degree per key; the note sounds; OCT+ then K4 OCT shifts the octave); a second press replaces it; UNDO tap restores | `pitch` |
| `KEY_W05_C4` | `SET` | press: SET row 2 (the cue strip says KEYS SET THE KEY): set the global KEY to the pitch class of C4; SET row 1: step 5 as PERF | `KEY / step` |
| `KEY_W05_C4` | `RESET` | press: clear the pin and flags of step 5 (UNDO tap restores) | `CLR 5` |
| `KEY_W05_C4` | `REC` | press: PLAY the pitch C4 (+ the REC OCT K4 and the global OCTAVE) on the selected channel (MIDI + preview), WRITE it as a pin at the cursor with the ACC and SLIDE flags, then the cursor advances; release: note off. A press under 30 ms is ignored | `note` |
| `KEY_W05_C4` | `MENU` | inert while the menu is open (no accidental pins) | `-` |
| `KEY_W06_D4` | `PERF` | press (a touch under 30 ms is ignored): step (viewed page - 1) x 16 + 6 (past the pattern length the key is dim and inert). An unpinned step gets a pin now (the note generated there, or the root when silent; auditioned). A pinned step is cleared on release if it was held under 0.4 s and no knob or OCT was used; otherwise the held step goes to STEP_HELD. While PAGE (D#4) is held: the pattern of the selected channel ends at this step (LENGTH). [R4 slot: per-key pressure / position on a held step key writes the armed step property, reserved, P1-015] | `step 6` |
| `KEY_W06_D4` | `STEP_HELD` | press while another white key is held: set the HELD step's pitch to scale degree 5 (key 1 = the root, one scale degree per key; the note sounds; OCT+ then K4 OCT shifts the octave); a second press replaces it; UNDO tap restores | `pitch` |
| `KEY_W06_D4` | `SET` | press: SET row 2 (the cue strip says KEYS SET THE KEY): set the global KEY to the pitch class of D4; SET row 1: step 6 as PERF | `KEY / step` |
| `KEY_W06_D4` | `RESET` | press: clear the pin and flags of step 6 (UNDO tap restores) | `CLR 6` |
| `KEY_W06_D4` | `REC` | press: PLAY the pitch D4 (+ the REC OCT K4 and the global OCTAVE) on the selected channel (MIDI + preview), WRITE it as a pin at the cursor with the ACC and SLIDE flags, then the cursor advances; release: note off. A press under 30 ms is ignored | `note` |
| `KEY_W06_D4` | `MENU` | inert while the menu is open (no accidental pins) | `-` |
| `KEY_W07_E4` | `PERF` | press (a touch under 30 ms is ignored): step (viewed page - 1) x 16 + 7 (past the pattern length the key is dim and inert). An unpinned step gets a pin now (the note generated there, or the root when silent; auditioned). A pinned step is cleared on release if it was held under 0.4 s and no knob or OCT was used; otherwise the held step goes to STEP_HELD. While PAGE (D#4) is held: the pattern of the selected channel ends at this step (LENGTH). [R4 slot: per-key pressure / position on a held step key writes the armed step property, reserved, P1-015] | `step 7` |
| `KEY_W07_E4` | `STEP_HELD` | press while another white key is held: set the HELD step's pitch to scale degree 6 (key 1 = the root, one scale degree per key; the note sounds; OCT+ then K4 OCT shifts the octave); a second press replaces it; UNDO tap restores | `pitch` |
| `KEY_W07_E4` | `SET` | press: SET row 2 (the cue strip says KEYS SET THE KEY): set the global KEY to the pitch class of E4; SET row 1: step 7 as PERF | `KEY / step` |
| `KEY_W07_E4` | `RESET` | press: clear the pin and flags of step 7 (UNDO tap restores) | `CLR 7` |
| `KEY_W07_E4` | `REC` | press: PLAY the pitch E4 (+ the REC OCT K4 and the global OCTAVE) on the selected channel (MIDI + preview), WRITE it as a pin at the cursor with the ACC and SLIDE flags, then the cursor advances; release: note off. A press under 30 ms is ignored | `note` |
| `KEY_W07_E4` | `MENU` | inert while the menu is open (no accidental pins) | `-` |
| `KEY_W08_F4` | `PERF` | press (a touch under 30 ms is ignored): step (viewed page - 1) x 16 + 8 (past the pattern length the key is dim and inert). An unpinned step gets a pin now (the note generated there, or the root when silent; auditioned). A pinned step is cleared on release if it was held under 0.4 s and no knob or OCT was used; otherwise the held step goes to STEP_HELD. While PAGE (D#4) is held: the pattern of the selected channel ends at this step (LENGTH). [R4 slot: per-key pressure / position on a held step key writes the armed step property, reserved, P1-015] | `step 8` |
| `KEY_W08_F4` | `STEP_HELD` | press while another white key is held: set the HELD step's pitch to scale degree 7 (key 1 = the root, one scale degree per key; the note sounds; OCT+ then K4 OCT shifts the octave); a second press replaces it; UNDO tap restores | `pitch` |
| `KEY_W08_F4` | `SET` | press: SET row 2 (the cue strip says KEYS SET THE KEY): set the global KEY to the pitch class of F4; SET row 1: step 8 as PERF | `KEY / step` |
| `KEY_W08_F4` | `RESET` | press: clear the pin and flags of step 8 (UNDO tap restores) | `CLR 8` |
| `KEY_W08_F4` | `REC` | press: PLAY the pitch F4 (+ the REC OCT K4 and the global OCTAVE) on the selected channel (MIDI + preview), WRITE it as a pin at the cursor with the ACC and SLIDE flags, then the cursor advances; release: note off. A press under 30 ms is ignored | `note` |
| `KEY_W08_F4` | `MENU` | inert while the menu is open (no accidental pins) | `-` |
| `KEY_W09_G4` | `PERF` | press (a touch under 30 ms is ignored): step (viewed page - 1) x 16 + 9 (past the pattern length the key is dim and inert). An unpinned step gets a pin now (the note generated there, or the root when silent; auditioned). A pinned step is cleared on release if it was held under 0.4 s and no knob or OCT was used; otherwise the held step goes to STEP_HELD. While PAGE (D#4) is held: the pattern of the selected channel ends at this step (LENGTH). [R4 slot: per-key pressure / position on a held step key writes the armed step property, reserved, P1-015] | `step 9` |
| `KEY_W09_G4` | `STEP_HELD` | press while another white key is held: set the HELD step's pitch to scale degree 8 (key 1 = the root, one scale degree per key; the note sounds; OCT+ then K4 OCT shifts the octave); a second press replaces it; UNDO tap restores | `pitch` |
| `KEY_W09_G4` | `SET` | press: SET row 2 (the cue strip says KEYS SET THE KEY): set the global KEY to the pitch class of G4; SET row 1: step 9 as PERF | `KEY / step` |
| `KEY_W09_G4` | `RESET` | press: clear the pin and flags of step 9 (UNDO tap restores) | `CLR 9` |
| `KEY_W09_G4` | `REC` | press: PLAY the pitch G4 (+ the REC OCT K4 and the global OCTAVE) on the selected channel (MIDI + preview), WRITE it as a pin at the cursor with the ACC and SLIDE flags, then the cursor advances; release: note off. A press under 30 ms is ignored | `note` |
| `KEY_W09_G4` | `MENU` | inert while the menu is open (no accidental pins) | `-` |
| `KEY_W10_A4` | `PERF` | press (a touch under 30 ms is ignored): step (viewed page - 1) x 16 + 10 (past the pattern length the key is dim and inert). An unpinned step gets a pin now (the note generated there, or the root when silent; auditioned). A pinned step is cleared on release if it was held under 0.4 s and no knob or OCT was used; otherwise the held step goes to STEP_HELD. While PAGE (D#4) is held: the pattern of the selected channel ends at this step (LENGTH). [R4 slot: per-key pressure / position on a held step key writes the armed step property, reserved, P1-015] | `step 10` |
| `KEY_W10_A4` | `STEP_HELD` | press while another white key is held: set the HELD step's pitch to scale degree 9 (key 1 = the root, one scale degree per key; the note sounds; OCT+ then K4 OCT shifts the octave); a second press replaces it; UNDO tap restores | `pitch` |
| `KEY_W10_A4` | `SET` | press: SET row 2 (the cue strip says KEYS SET THE KEY): set the global KEY to the pitch class of A4; SET row 1: step 10 as PERF | `KEY / step` |
| `KEY_W10_A4` | `RESET` | press: clear the pin and flags of step 10 (UNDO tap restores) | `CLR 10` |
| `KEY_W10_A4` | `REC` | press: PLAY the pitch A4 (+ the REC OCT K4 and the global OCTAVE) on the selected channel (MIDI + preview), WRITE it as a pin at the cursor with the ACC and SLIDE flags, then the cursor advances; release: note off. A press under 30 ms is ignored | `note` |
| `KEY_W10_A4` | `MENU` | inert while the menu is open (no accidental pins) | `-` |
| `KEY_W11_B4` | `PERF` | press (a touch under 30 ms is ignored): step (viewed page - 1) x 16 + 11 (past the pattern length the key is dim and inert). An unpinned step gets a pin now (the note generated there, or the root when silent; auditioned). A pinned step is cleared on release if it was held under 0.4 s and no knob or OCT was used; otherwise the held step goes to STEP_HELD. While PAGE (D#4) is held: the pattern of the selected channel ends at this step (LENGTH). [R4 slot: per-key pressure / position on a held step key writes the armed step property, reserved, P1-015] | `step 11` |
| `KEY_W11_B4` | `STEP_HELD` | press while another white key is held: set the HELD step's pitch to scale degree 10 (key 1 = the root, one scale degree per key; the note sounds; OCT+ then K4 OCT shifts the octave); a second press replaces it; UNDO tap restores | `pitch` |
| `KEY_W11_B4` | `SET` | press: SET row 2 (the cue strip says KEYS SET THE KEY): set the global KEY to the pitch class of B4; SET row 1: step 11 as PERF | `KEY / step` |
| `KEY_W11_B4` | `RESET` | press: clear the pin and flags of step 11 (UNDO tap restores) | `CLR 11` |
| `KEY_W11_B4` | `REC` | press: PLAY the pitch B4 (+ the REC OCT K4 and the global OCTAVE) on the selected channel (MIDI + preview), WRITE it as a pin at the cursor with the ACC and SLIDE flags, then the cursor advances; release: note off. A press under 30 ms is ignored | `note` |
| `KEY_W11_B4` | `MENU` | inert while the menu is open (no accidental pins) | `-` |
| `KEY_W12_C5` | `PERF` | press (a touch under 30 ms is ignored): step (viewed page - 1) x 16 + 12 (past the pattern length the key is dim and inert). An unpinned step gets a pin now (the note generated there, or the root when silent; auditioned). A pinned step is cleared on release if it was held under 0.4 s and no knob or OCT was used; otherwise the held step goes to STEP_HELD. While PAGE (D#4) is held: the pattern of the selected channel ends at this step (LENGTH). [R4 slot: per-key pressure / position on a held step key writes the armed step property, reserved, P1-015] | `step 12` |
| `KEY_W12_C5` | `STEP_HELD` | press while another white key is held: set the HELD step's pitch to scale degree 11 (key 1 = the root, one scale degree per key; the note sounds; OCT+ then K4 OCT shifts the octave); a second press replaces it; UNDO tap restores | `pitch` |
| `KEY_W12_C5` | `SET` | press: SET row 2 (the cue strip says KEYS SET THE KEY): set the global KEY to the pitch class of C5; SET row 1: step 12 as PERF | `KEY / step` |
| `KEY_W12_C5` | `RESET` | press: clear the pin and flags of step 12 (UNDO tap restores) | `CLR 12` |
| `KEY_W12_C5` | `REC` | press: PLAY the pitch C5 (+ the REC OCT K4 and the global OCTAVE) on the selected channel (MIDI + preview), WRITE it as a pin at the cursor with the ACC and SLIDE flags, then the cursor advances; release: note off. A press under 30 ms is ignored | `note` |
| `KEY_W12_C5` | `MENU` | inert while the menu is open (no accidental pins) | `-` |
| `KEY_W13_D5` | `PERF` | press (a touch under 30 ms is ignored): step (viewed page - 1) x 16 + 13 (past the pattern length the key is dim and inert). An unpinned step gets a pin now (the note generated there, or the root when silent; auditioned). A pinned step is cleared on release if it was held under 0.4 s and no knob or OCT was used; otherwise the held step goes to STEP_HELD. While PAGE (D#4) is held: the pattern of the selected channel ends at this step (LENGTH). [R4 slot: per-key pressure / position on a held step key writes the armed step property, reserved, P1-015] | `step 13` |
| `KEY_W13_D5` | `STEP_HELD` | press while another white key is held: set the HELD step's pitch to scale degree 12 (key 1 = the root, one scale degree per key; the note sounds; OCT+ then K4 OCT shifts the octave); a second press replaces it; UNDO tap restores | `pitch` |
| `KEY_W13_D5` | `SET` | press: SET row 2 (the cue strip says KEYS SET THE KEY): set the global KEY to the pitch class of D5; SET row 1: step 13 as PERF | `KEY / step` |
| `KEY_W13_D5` | `RESET` | press: clear the pin and flags of step 13 (UNDO tap restores) | `CLR 13` |
| `KEY_W13_D5` | `REC` | press: PLAY the pitch D5 (+ the REC OCT K4 and the global OCTAVE) on the selected channel (MIDI + preview), WRITE it as a pin at the cursor with the ACC and SLIDE flags, then the cursor advances; release: note off. A press under 30 ms is ignored | `note` |
| `KEY_W13_D5` | `MENU` | inert while the menu is open (no accidental pins) | `-` |
| `KEY_W14_E5` | `PERF` | press (a touch under 30 ms is ignored): step (viewed page - 1) x 16 + 14 (past the pattern length the key is dim and inert). An unpinned step gets a pin now (the note generated there, or the root when silent; auditioned). A pinned step is cleared on release if it was held under 0.4 s and no knob or OCT was used; otherwise the held step goes to STEP_HELD. While PAGE (D#4) is held: the pattern of the selected channel ends at this step (LENGTH). [R4 slot: per-key pressure / position on a held step key writes the armed step property, reserved, P1-015] | `step 14` |
| `KEY_W14_E5` | `STEP_HELD` | press while another white key is held: set the HELD step's pitch to scale degree 13 (key 1 = the root, one scale degree per key; the note sounds; OCT+ then K4 OCT shifts the octave); a second press replaces it; UNDO tap restores | `pitch` |
| `KEY_W14_E5` | `SET` | press: SET row 2 (the cue strip says KEYS SET THE KEY): set the global KEY to the pitch class of E5; SET row 1: step 14 as PERF | `KEY / step` |
| `KEY_W14_E5` | `RESET` | press: clear the pin and flags of step 14 (UNDO tap restores) | `CLR 14` |
| `KEY_W14_E5` | `REC` | press: PLAY the pitch E5 (+ the REC OCT K4 and the global OCTAVE) on the selected channel (MIDI + preview), WRITE it as a pin at the cursor with the ACC and SLIDE flags, then the cursor advances; release: note off. A press under 30 ms is ignored | `note` |
| `KEY_W14_E5` | `MENU` | inert while the menu is open (no accidental pins) | `-` |
| `KEY_W15_F5` | `PERF` | press (a touch under 30 ms is ignored): step (viewed page - 1) x 16 + 15 (past the pattern length the key is dim and inert). An unpinned step gets a pin now (the note generated there, or the root when silent; auditioned). A pinned step is cleared on release if it was held under 0.4 s and no knob or OCT was used; otherwise the held step goes to STEP_HELD. While PAGE (D#4) is held: the pattern of the selected channel ends at this step (LENGTH). [R4 slot: per-key pressure / position on a held step key writes the armed step property, reserved, P1-015] | `step 15` |
| `KEY_W15_F5` | `STEP_HELD` | press while another white key is held: set the HELD step's pitch to scale degree 14 (key 1 = the root, one scale degree per key; the note sounds; OCT+ then K4 OCT shifts the octave); a second press replaces it; UNDO tap restores | `pitch` |
| `KEY_W15_F5` | `SET` | press: SET row 2 (the cue strip says KEYS SET THE KEY): set the global KEY to the pitch class of F5; SET row 1: step 15 as PERF | `KEY / step` |
| `KEY_W15_F5` | `RESET` | press: clear the pin and flags of step 15 (UNDO tap restores) | `CLR 15` |
| `KEY_W15_F5` | `REC` | press: PLAY the pitch F5 (+ the REC OCT K4 and the global OCTAVE) on the selected channel (MIDI + preview), WRITE it as a pin at the cursor with the ACC and SLIDE flags, then the cursor advances; release: note off. A press under 30 ms is ignored | `note` |
| `KEY_W15_F5` | `MENU` | inert while the menu is open (no accidental pins) | `-` |
| `KEY_W16_G5` | `PERF` | press (a touch under 30 ms is ignored): step (viewed page - 1) x 16 + 16 (past the pattern length the key is dim and inert). An unpinned step gets a pin now (the note generated there, or the root when silent; auditioned). A pinned step is cleared on release if it was held under 0.4 s and no knob or OCT was used; otherwise the held step goes to STEP_HELD. While PAGE (D#4) is held: the pattern of the selected channel ends at this step (LENGTH). [R4 slot: per-key pressure / position on a held step key writes the armed step property, reserved, P1-015] | `step 16` |
| `KEY_W16_G5` | `STEP_HELD` | press while another white key is held: set the HELD step's pitch to scale degree 15 (key 1 = the root, one scale degree per key; the note sounds; OCT+ then K4 OCT shifts the octave); a second press replaces it; UNDO tap restores | `pitch` |
| `KEY_W16_G5` | `SET` | press: SET row 2 (the cue strip says KEYS SET THE KEY): set the global KEY to the pitch class of G5; SET row 1: step 16 as PERF | `KEY / step` |
| `KEY_W16_G5` | `RESET` | press: clear the pin and flags of step 16 (UNDO tap restores) | `CLR 16` |
| `KEY_W16_G5` | `REC` | press: PLAY the pitch G5 (+ the REC OCT K4 and the global OCTAVE) on the selected channel (MIDI + preview), WRITE it as a pin at the cursor with the ACC and SLIDE flags, then the cursor advances; release: note off. A press under 30 ms is ignored | `note` |
| `KEY_W16_G5` | `MENU` | inert while the menu is open (no accidental pins) | `-` |
| `KEY_B01_F#3` | `PERF` | press: mute / unmute CH1 (latched) | `MUTE1` |
| `KEY_B01_F#3` | `STEP_HELD` | press: mute / unmute CH1 (latched) | `MUTE1` |
| `KEY_B01_F#3` | `SET` | SET row 2 (the cue strip says KEYS SET THE KEY): set the global KEY to the pitch class of F#3; SET row 1: mute / unmute CH1 (latched) | `KEY / MUTE1` |
| `KEY_B01_F#3` | `RESET` | press: mute / unmute CH1 (latched) | `MUTE1` |
| `KEY_B01_F#3` | `REC` | press: PLAY the pitch F#3 (+ the REC OCT K4 and the global OCTAVE) on the selected channel (MIDI + preview), WRITE it as a pin at the cursor with the ACC and SLIDE flags, then the cursor advances; release: note off. A press under 30 ms is ignored | `note` |
| `KEY_B01_F#3` | `MENU` | inert while the menu is open (no accidental moves) | `-` |
| `KEY_B02_G#3` | `PERF` | press: mute / unmute CH2 (latched) | `MUTE2` |
| `KEY_B02_G#3` | `STEP_HELD` | press: mute / unmute CH2 (latched) | `MUTE2` |
| `KEY_B02_G#3` | `SET` | SET row 2 (the cue strip says KEYS SET THE KEY): set the global KEY to the pitch class of G#3; SET row 1: mute / unmute CH2 (latched) | `KEY / MUTE2` |
| `KEY_B02_G#3` | `RESET` | press: mute / unmute CH2 (latched) | `MUTE2` |
| `KEY_B02_G#3` | `REC` | press: PLAY the pitch G#3 (+ the REC OCT K4 and the global OCTAVE) on the selected channel (MIDI + preview), WRITE it as a pin at the cursor with the ACC and SLIDE flags, then the cursor advances; release: note off. A press under 30 ms is ignored | `note` |
| `KEY_B02_G#3` | `MENU` | inert while the menu is open (no accidental moves) | `-` |
| `KEY_B03_A#3` | `PERF` | press: mute / unmute CH3 (latched) | `MUTE3` |
| `KEY_B03_A#3` | `STEP_HELD` | press: mute / unmute CH3 (latched) | `MUTE3` |
| `KEY_B03_A#3` | `SET` | SET row 2 (the cue strip says KEYS SET THE KEY): set the global KEY to the pitch class of A#3; SET row 1: mute / unmute CH3 (latched) | `KEY / MUTE3` |
| `KEY_B03_A#3` | `RESET` | press: mute / unmute CH3 (latched) | `MUTE3` |
| `KEY_B03_A#3` | `REC` | press: PLAY the pitch A#3 (+ the REC OCT K4 and the global OCTAVE) on the selected channel (MIDI + preview), WRITE it as a pin at the cursor with the ACC and SLIDE flags, then the cursor advances; release: note off. A press under 30 ms is ignored | `note` |
| `KEY_B03_A#3` | `MENU` | inert while the menu is open (no accidental moves) | `-` |
| `KEY_B04_C#4` | `PERF` | press: mute / unmute CH4 (latched) | `MUTE4` |
| `KEY_B04_C#4` | `STEP_HELD` | press: mute / unmute CH4 (latched) | `MUTE4` |
| `KEY_B04_C#4` | `SET` | SET row 2 (the cue strip says KEYS SET THE KEY): set the global KEY to the pitch class of C#4; SET row 1: mute / unmute CH4 (latched) | `KEY / MUTE4` |
| `KEY_B04_C#4` | `RESET` | press: mute / unmute CH4 (latched) | `MUTE4` |
| `KEY_B04_C#4` | `REC` | press: PLAY the pitch C#4 (+ the REC OCT K4 and the global OCTAVE) on the selected channel (MIDI + preview), WRITE it as a pin at the cursor with the ACC and SLIDE flags, then the cursor advances; release: note off. A press under 30 ms is ignored | `note` |
| `KEY_B04_C#4` | `MENU` | inert while the menu is open (no accidental moves) | `-` |
| `KEY_B05_D#4` | `PERF` | press: PAGE: tap: view the next page of the selected channel (wraps; the viewed page is UI state, the playhead is never moved); hold: the next white key pressed sets the pattern length to end at that step | `PAGE` |
| `KEY_B05_D#4` | `STEP_HELD` | inert while a step key is held (finish the step first; toast `FINISH THE STEP`) | `-` |
| `KEY_B05_D#4` | `SET` | SET row 2 (the cue strip says KEYS SET THE KEY): set the global KEY to the pitch class of D#4; SET row 1: PAGE: tap: view the next page of the selected channel (wraps; the viewed page is UI state, the playhead is never moved); hold: the next white key pressed sets the pattern length to end at that step | `KEY / PAGE` |
| `KEY_B05_D#4` | `RESET` | press: PAGE: tap: view the next page of the selected channel (wraps; the viewed page is UI state, the playhead is never moved); hold: the next white key pressed sets the pattern length to end at that step | `PAGE` |
| `KEY_B05_D#4` | `REC` | press: PLAY the pitch D#4 (+ the REC OCT K4 and the global OCTAVE) on the selected channel (MIDI + preview), WRITE it as a pin at the cursor with the ACC and SLIDE flags, then the cursor advances; release: note off. A press under 30 ms is ignored | `note` |
| `KEY_B05_D#4` | `MENU` | inert while the menu is open (no accidental moves) | `-` |
| `KEY_B06_F#4` | `PERF` | press: SOLO: hold: only the selected channel sounds, release restores every mute | `SOLO` |
| `KEY_B06_F#4` | `STEP_HELD` | press: SOLO: hold: only the selected channel sounds, release restores every mute | `SOLO` |
| `KEY_B06_F#4` | `SET` | SET row 2 (the cue strip says KEYS SET THE KEY): set the global KEY to the pitch class of F#4; SET row 1: SOLO: hold: only the selected channel sounds, release restores every mute | `KEY / SOLO` |
| `KEY_B06_F#4` | `RESET` | press: SOLO: hold: only the selected channel sounds, release restores every mute | `SOLO` |
| `KEY_B06_F#4` | `REC` | press: PLAY the pitch F#4 (+ the REC OCT K4 and the global OCTAVE) on the selected channel (MIDI + preview), WRITE it as a pin at the cursor with the ACC and SLIDE flags, then the cursor advances; release: note off. A press under 30 ms is ignored | `note` |
| `KEY_B06_F#4` | `MENU` | inert while the menu is open (no accidental moves) | `-` |
| `KEY_B07_G#4` | `PERF` | press: FILL: hold: the selected channel's K1 morph goes to its maximum (every step / stage plays), release restores it | `FILL` |
| `KEY_B07_G#4` | `STEP_HELD` | press: FILL: hold: the selected channel's K1 morph goes to its maximum (every step / stage plays), release restores it | `FILL` |
| `KEY_B07_G#4` | `SET` | SET row 2 (the cue strip says KEYS SET THE KEY): set the global KEY to the pitch class of G#4; SET row 1: FILL: hold: the selected channel's K1 morph goes to its maximum (every step / stage plays), release restores it | `KEY / FILL` |
| `KEY_B07_G#4` | `RESET` | press: FILL: hold: the selected channel's K1 morph goes to its maximum (every step / stage plays), release restores it | `FILL` |
| `KEY_B07_G#4` | `REC` | press: PLAY the pitch G#4 (+ the REC OCT K4 and the global OCTAVE) on the selected channel (MIDI + preview), WRITE it as a pin at the cursor with the ACC and SLIDE flags, then the cursor advances; release: note off. A press under 30 ms is ignored | `note` |
| `KEY_B07_G#4` | `MENU` | inert while the menu is open (no accidental moves) | `-` |
| `KEY_B08_A#4` | `PERF` | press: ROLL: hold: every step of the selected channel repeats twice (1/32 at DIV 1/16), release stops it | `ROLL` |
| `KEY_B08_A#4` | `STEP_HELD` | press: ROLL: hold: every step of the selected channel repeats twice (1/32 at DIV 1/16), release stops it | `ROLL` |
| `KEY_B08_A#4` | `SET` | SET row 2 (the cue strip says KEYS SET THE KEY): set the global KEY to the pitch class of A#4; SET row 1: ROLL: hold: every step of the selected channel repeats twice (1/32 at DIV 1/16), release stops it | `KEY / ROLL` |
| `KEY_B08_A#4` | `RESET` | press: ROLL: hold: every step of the selected channel repeats twice (1/32 at DIV 1/16), release stops it | `ROLL` |
| `KEY_B08_A#4` | `REC` | press: PLAY the pitch A#4 (+ the REC OCT K4 and the global OCTAVE) on the selected channel (MIDI + preview), WRITE it as a pin at the cursor with the ACC and SLIDE flags, then the cursor advances; release: note off. A press under 30 ms is ignored | `note` |
| `KEY_B08_A#4` | `MENU` | inert while the menu is open (no accidental moves) | `-` |
| `KEY_B09_C#5` | `PERF` | press: SCENE A: tap: recall scene A at the next bar (toast `A EMPTY: hold to store` if unset); hold 0.7 s: store the whole current state into scene A | `SCN A` |
| `KEY_B09_C#5` | `STEP_HELD` | press: SCENE A: tap: recall scene A at the next bar (toast `A EMPTY: hold to store` if unset); hold 0.7 s: store the whole current state into scene A | `SCN A` |
| `KEY_B09_C#5` | `SET` | SET row 2 (the cue strip says KEYS SET THE KEY): set the global KEY to the pitch class of C#5; SET row 1: SCENE A: tap: recall scene A at the next bar (toast `A EMPTY: hold to store` if unset); hold 0.7 s: store the whole current state into scene A | `KEY / SCN A` |
| `KEY_B09_C#5` | `RESET` | press: SCENE A: tap: recall scene A at the next bar (toast `A EMPTY: hold to store` if unset); hold 0.7 s: store the whole current state into scene A | `SCN A` |
| `KEY_B09_C#5` | `REC` | press: PLAY the pitch C#5 (+ the REC OCT K4 and the global OCTAVE) on the selected channel (MIDI + preview), WRITE it as a pin at the cursor with the ACC and SLIDE flags, then the cursor advances; release: note off. A press under 30 ms is ignored | `note` |
| `KEY_B09_C#5` | `MENU` | inert while the menu is open (no accidental moves) | `-` |
| `KEY_B10_D#5` | `PERF` | press: SCENE B: tap: recall scene B at the next bar (toast `B EMPTY: hold to store` if unset); hold 0.7 s: store the whole current state into scene B | `SCN B` |
| `KEY_B10_D#5` | `STEP_HELD` | press: SCENE B: tap: recall scene B at the next bar (toast `B EMPTY: hold to store` if unset); hold 0.7 s: store the whole current state into scene B | `SCN B` |
| `KEY_B10_D#5` | `SET` | SET row 2 (the cue strip says KEYS SET THE KEY): set the global KEY to the pitch class of D#5; SET row 1: SCENE B: tap: recall scene B at the next bar (toast `B EMPTY: hold to store` if unset); hold 0.7 s: store the whole current state into scene B | `KEY / SCN B` |
| `KEY_B10_D#5` | `RESET` | press: SCENE B: tap: recall scene B at the next bar (toast `B EMPTY: hold to store` if unset); hold 0.7 s: store the whole current state into scene B | `SCN B` |
| `KEY_B10_D#5` | `REC` | press: PLAY the pitch D#5 (+ the REC OCT K4 and the global OCTAVE) on the selected channel (MIDI + preview), WRITE it as a pin at the cursor with the ACC and SLIDE flags, then the cursor advances; release: note off. A press under 30 ms is ignored | `note` |
| `KEY_B10_D#5` | `MENU` | inert while the menu is open (no accidental moves) | `-` |
| `KEY_B11_F#5` | `PERF` | press: restart all channels from step 1 at the next step | `RESTRT` |
| `KEY_B11_F#5` | `STEP_HELD` | press: restart all channels from step 1 at the next step | `RESTRT` |
| `KEY_B11_F#5` | `SET` | SET row 2 (the cue strip says KEYS SET THE KEY): set the global KEY to the pitch class of F#5; SET row 1: restart all channels from step 1 at the next step | `KEY / RESTRT` |
| `KEY_B11_F#5` | `RESET` | press: restart all channels from step 1 at the next step | `RESTRT` |
| `KEY_B11_F#5` | `REC` | press: PLAY the pitch F#5 (+ the REC OCT K4 and the global OCTAVE) on the selected channel (MIDI + preview), WRITE it as a pin at the cursor with the ACC and SLIDE flags, then the cursor advances; release: note off. A press under 30 ms is ignored | `note` |
| `KEY_B11_F#5` | `MENU` | inert while the menu is open (no accidental moves) | `-` |
<!-- END-APPENDIX-A -->
