# Control map v1: every control, every context, one gesture

> P2-009: the chord row of the CONTROL tab is removed (owner decision 39). CONTROL tab is 4 rows; 28 menu rows in total (SYSTEM_MAP section 7).

Task P2-001. Status: consolidation of the owner-approved P1-012 design (approved 2026-11-10, with P1-010; owner
decisions 31-36 in [OWNER_QUESTIONS.md](../project/OWNER_QUESTIONS.md)). **This file does not redesign anything.** The
reasoning, ease scores and rejected alternatives stay in [UX_REVIEW.md](UX_REVIEW.md); the exact machine-readable table
is [control_map.tsv](control_map.tsv) (343 rows = 49 controls x 7 contexts). Appendix A below repeats every TSV row
verbatim, and `python3 tools/check_control_map.py` proves the two agree. If they ever differ, the TSV is the source
and this file is wrong.

Inputs: [VISION.md](../project/VISION.md), [ADR-0004](../project/decisions/ADR-0004-design-lock.md), UX_REVIEW.md,
[CONTROL_INVENTORY.md](CONTROL_INVENTORY.md) (what the firmware does today), [ALGORITHM_SET.md](ALGORITHM_SET.md)
(layers, morphs, dice). Panel photo: `docs/controls.jpg`. Screen pixel layout is P2-003, not here.

## 0. Fixed facts used below

- **4 channels** (owner decision 31). Power-on defaults: CH1 `ACID` MIDI 1 unmuted; CH2 `EUCLID` MIDI 2, CH3 `SNAKE`
  MIDI 3, CH4 `STAGE` MIDI 4, all three muted. Global KEY C, SCALE `MINPEN`, OCTAVE 0, GATE 50 %, DIV 1/16, SWING 50 %,
  BPM 120, preset P01, keyboard OCT 0. A channel has only a MIDI channel (1..16 or OFF) and an algorithm.
- **Tap versus hold timing.** A *tap* is a press released **under 0.4 s**. The layer buttons (SCL, EDIT, GLO) use the
  menu setting HOLD (0.3, 0.4, 0.5 or 0.6 s; default **0.4 s**); HOME, SAVE and REC use a fixed **0.7 s** hold
  (`btn_hold`). A button with a hold role acts on **release** when tapped; every other button acts on **press**. All
  four numbers are the firmware's existing values.
- **Dice sub-labels.** The five dice buttons carry a shared main label and show the selected algorithm's own layer name
  as a sub-label (`NOTES +sub`); a layer the algorithm lacks is dimmed, inert, and toasts `NO SHAPE IN EUCLID` for 0.5 s.
- **Next-step commit.** Morphs, dice, algorithm and scale changes are heard from the next step (owner question 9 path).
  A preset load is heard at the next loop start (decision 34).

## 1. Modes, and how you always know which one you are in

There is no mode you can be lost in: the sequencer boots straight into the main screen, and every other state is
either **momentary** (a held button) or **latched with a banner and a lit LED**. Seven panel states, taken from the
approved P1-012 contexts. Precedence when two could apply: **MENU > SCL/CHAN (held layer) > GLOB > STEP_HELD > STEP >
PERF**. Only one layer button counts at a time: a second layer button pressed while one is held keeps its PERF role.

| Context | Kind | Entered by | Left by | How you know (always on screen) |
| --- | --- | --- | --- | --- |
| `PERF` | resting state | power-on default | n/a | Main screen: header, 4 channel cells, 16-step lane, 4 knob cells, 2 x 6 button picture |
| `STEP` | latched | tap REC (LED lit) | tap REC or tap HOME | `STEP` banner in the header; the lane shows key numbers 1..16 |
| `STEP_HELD` | momentary | hold a white key while in STEP | let go of all white keys | K3 cell reads `NOTE`; OCT- / OCT+ cells read `PIN OCT` |
| `SCL` | momentary | hold SCL past 0.4 s | let go of SCL | SCL layer: K1 `KEY`, K2 `SCALE`, K3 `OCTAVE`; the SCL cell reads `SCALE` |
| `CHAN` | momentary | hold EDIT past 0.4 s | let go of EDIT | CHAN layer: four cells `CH1 MID` .. `CH4 MID` with MIDI channel and algorithm |
| `GLOB` | latched (tap) or momentary (hold) | tap GLO, or hold GLO past 0.4 s | tap GLO or HOME; or release a hold | GLOB page: K1 `GATE`, K2 `DIV`, K3 `SWING`, K4 `BPM`; GLO cell reads `BACK` |
| `MENU` | latched | hold HOME 0.7 s | tap or hold HOME | Felucca menu; every button but PLAY reads `close` |

**One held modifier at most.** Every combo is "one held thing plus a turn or a press": hold a layer button then turn a
knob or press a key, or hold white key(s) then turn K3 or press OCT. No gesture needs two different modifiers held, and
`STEP_HELD` uses only one *kind* of held control (white keys), which all receive the same edit.

The **context** column of the TSV and of Appendix A uses exactly these seven names.

## 2. The control table: Control | Tap | Hold | Combo | Screen label

Summary of the approved map in the order of the panel. Per-context detail (what each control does in each of the seven
states) is Appendix A; where the summary and Appendix A differ, Appendix A (= the TSV) wins. "Combo" means "while a
layer or key is held / another context is active".

| Control (printed) | Tap or press | Hold | Combo | Screen label |
| --- | --- | --- | --- | --- |
| `MASTER` (pot) | analogue: preview-synth and USB-audio volume | n/a | n/a | none (no MIDI effect) |
| `EN_SELECT` (SELECT) | turn: select channel CH1..CH4 (clamped, no wrap) | n/a | works in every context including MENU | `CH n` |
| `EN_ALGO` (ALGORITHM) | turn: algorithm of the selected channel `ACID > EUCLID > SNAKE > STAGE`; lands at the next step; morphs and seeds kept per algorithm (S7) | n/a | in MENU: next/previous tab | algorithm name |
| `EN_PRESET` (PRESETS) | turn: preset slot P01..P16; filled slot loads at next loop start (now if stopped); empty slot only selects the SAVE target | n/a | in MENU: next/previous row | `Pnn`, `Pnn>` while queued, `Pnn EMPTY` |
| `EN_K1` (KNOB 1) | turn: morph DENSITY (0 = `KEYS ONLY`) | n/a | SCL held: KEY; CHAN held: CH1 MIDI; GLOB: GATE; MENU: value | algorithm label, `KEY`, `CH1 MID`, `GATE` |
| `EN_K2` (KNOB 2) | turn: morph FORM (ACID PEDAL, EUCLID LEN, SNAKE LEN, STAGE TOTAL) | n/a | SCL held: SCALE; CHAN: CH2 MIDI; GLOB: DIV; MENU: value | algorithm label, `SCALE`, `CH2 MID`, `DIV` |
| `EN_K3` (KNOB 3) | turn: morph PITCH (JUMP, SPAN, RANGE, RANGE) | n/a | key held in STEP: pin NOTE by scale degree; SCL held: OCTAVE; CHAN: CH3 MIDI; GLOB: SWING; MENU: value | algorithm label, `NOTE`, `OCTAVE`, `CH3 MID`, `SWING` |
| `EN_K4` (KNOB 4) | turn: morph SPICE (ACCNT, ACCNT, PATH, RATCH) | n/a | SCL held: unchanged morph; CHAN: CH4 MIDI; GLOB: BPM (EXT when slaved); MENU: value | algorithm label, `CH4 MID`, `BPM` |
| `B_FX` (FX) | press: DICE ALL layers present | same as press | any context except MENU (closes) | `ALL` |
| `B_SCL` (SCL) | tap: dice NOTES | past 0.4 s: SCL layer (global KEY, SCALE, OCTAVE; any key sets KEY) | in SCL: held keeps the layer open | `NOTES +sub`, `SCALE` |
| `B_ENV` (ENV) | press: dice ACCNT | n/a | n/a | `ACCNT +sub` |
| `B_LFO` (LFO) | press: dice SHAPE (inert if the algorithm has none) | n/a | n/a | `SHAPE +sub` |
| `B_EDIT` (EDIT) | tap: UNDO / REDO the last dice, dice-all or pin-clear | past 0.4 s: CHAN layer (MIDI channel of each channel) | in CHAN: held keeps the layer open | `UNDO`, `CHAN` |
| `B_GLO` (GLO) | tap: open GLOB page (latched); tap again closes | past 0.4 s: GLOB page while held | in GLOB: `BACK` | `GLOBAL`, `BACK` |
| `B_HOME` (HOME) | PERF: toggle KEYS legend; elsewhere: return to PERF; in MENU: close | 0.7 s: open MENU | n/a | `KEYS / hold MENU`, `HOME`, `BACK` |
| `B_SAVE` (SAVE) | tap: save the whole device state to the preset slot in the header | 0.7 s: undo the last preset load (again: redo) | n/a | `SAVE Pnn` |
| `B_ARP` (ARP) | press: dice ARTIC (inert if none) | n/a | n/a | `ARTIC +sub` |
| `B_SEQ` (SEQ) | press: dice HITS | n/a | n/a | `HITS +sub` |
| `B_PLAY` (PLAY/STOP) | press: start / stop all channels | n/a | works in every context including MENU | `PLAY / STOP` |
| `B_REC` (REC) | tap: enter STEP; in STEP: leave STEP | 0.7 s: clear every pin of the selected channel (EDIT tap undoes) | n/a | `STEP`, `STEP ON` |
| `B_OCTDN` (OCT-) | press: keyboard octave -1 (range -3..+3) | n/a | STEP_HELD: held step pins -12 semitones; both OCT = octave 0; both held 5 s = UPDATE MODE; both at power-on = CALIBRATION | `OCT n`, `PIN OCT` |
| `B_OCTUP` (OCT+) | press: keyboard octave +1 | n/a | STEP_HELD: held step pins +12 semitones; hidden gestures as OCT- | `OCT n`, `PIN OCT` |
| White keys F3 G3 A3 B3 C4 D4 E4 F4 G4 A4 B4 C5 D5 E5 F5 G5 (`KEY_W01_F3` .. `KEY_W16_G5`) | PERF, CHAN, GLOB: play the note live on the selected channel (snapped to the global scale); STEP: toggle the pin on step n | STEP: hold = STEP_HELD (K3 and OCT edit every held step) | SCL held: set global KEY to the key's pitch class | none while playing; `step n` in STEP; `KEY` in SCL |
| Black key F#3 G#3 A#3 C#4 (`KEY_B01..B04`) | press: mute / unmute CH1..CH4 (latched) | n/a | SCL held: set KEY (F# G# A# C#) | `MUTE1`..`MUTE4`, `KEY` |
| Black key D#4 F#4 G#4 A#4 (`KEY_B05..B08`) | press: select CH1..CH4 | n/a | SCL held: set KEY (D# F# G# A#) | `CH1`..`CH4`, `KEY` |
| Black key C#5 (`KEY_B09`) | press: unmute all channels | n/a | SCL held: set KEY C# | `ALL ON`, `KEY` |
| Black key D#5 (`KEY_B10`) | press: restart all channels from step 1 at the next step | n/a | SCL held: set KEY D# | `RESTRT`, `KEY` |
| Black key F#5 (`KEY_B11`) | press: tap tempo (internal clock only) | n/a | SCL held: set KEY F# | `TAP`, `KEY` |

No control is "unused, reserved": all 49 controls (MASTER, 3 encoders, 4 knobs, 14 buttons, 16 white keys and 11 black
keys) have an assigned role. The only unused combinations are those in the TSV marked `-`, namely OCT- / OCT+ inside
MENU, which keep Felucca's menu rule (previous / next value, and `do it` on CALIBRATION, INFO, ABOUT rows).

## 3. Keys: steps versus channels versus dice

- **16 white keys (F3..G5, place 0..15).** PERF: live notes on the selected channel, snapped to the global scale, keyboard
  OCT applies. STEP: step 1..16 pins (below). SCL held: set KEY. A key is **never** a dice or channel control.
- **11 black keys (F#3..F#5).** Channel and transport: `F#3 G#3 A#3 C#4` mute CH1-4 (latched); `D#4 F#4 G#4 A#4` select
  CH1-4; `C#5` unmute all; `D#5` restart from step 1; `F#5` tap tempo. They behave the same in PERF, STEP,
  STEP_HELD, CHAN, GLOB and MENU, and set KEY while SCL is held.
- **Dice are never on keys.** They are the buttons FX (all), SCL tap (NOTES), SEQ (HITS), ENV (ACCNT), ARP (ARTIC),
  LFO (SHAPE), with EDIT tap as undo. Dice rows of the table are inert on the keyboard so a performer's hands stay on the
  button block.
- **Step entry (W4).** Tap REC, optionally turn K1 fully down (`KEYS ONLY`: only pins play), tap white key n to pin step n
  (sounds at once; a new pin is the note generated there, or the root when silent). To change a pin: hold its key, turn
  K3 (`NOTE`, one scale degree per detent) and/or press OCT- / OCT+ (`PIN OCT`, +-12 semitones). Tap a pinned key to
  clear it. Tap REC to leave; hold REC 0.7 s clears all pins (EDIT tap brings them back). A pin entered with K3 stays in
  scale; the unsnapped `pin_note` is kept so changing the scale back restores it.

## 4. Reference workflow, gesture by gesture (TB-3 on MIDI channel 2)

Starting from power-on in the default `SEQ` mode. No step opens the MENU; the only held gesture is EDIT held for the
channel setup. Result: **performing within 6 gestures**, every one on the main screen except the held CHAN layer.

| # | Gesture | Controls | What the screen shows | Reaches |
| --- | --- | --- | --- | --- |
| 1 | Select channel 1 | turn SELECT to `1`, or press D#4 | header cell `1` inverted; knob cells show channel 1's algorithm | W1 |
| 2 | Set MIDI channel 2 | hold EDIT (CHAN layer opens after 0.4 s); turn K1 until `CH1 MID` reads `2`; let go of EDIT | CHAN layer with four `CHn MID` cells; header cell `1 ACID 2` after release | W2 |
| 3 | Choose an algorithm | turn ALGORITHM: `ACID > EUCLID > SNAKE > STAGE` (lands at the next step; its own morphs and seeds are restored) | algorithm name in large type | W3 |
| 4a | Dial a sequence in | press PLAY; turn K1 (density), K2 (form), K3 (pitch width), K4 (spice) | each knob cell highlights for 1 s and shows its value | W5, W6 |
| 4b | or enter steps | tap REC; turn K1 to `KEYS ONLY`; tap white keys; hold a key + K3 / OCT to set pitch; tap REC | `STEP` banner; pins outlined on the lane | W4 |
| 5 | Perform | morph with K1-K4; dice with SCL tap (NOTES), SEQ (HITS), ENV (ACCNT), ARP (ARTIC), LFO (SHAPE), FX (all); EDIT tap undoes; mute with F#3..C#4, select with D#4..A#4 or SELECT | every dice button shows its role and sub-label; muted channel cell dimmed with `M` | W6, W7, W10 |
| 6 | Optional global changes | hold SCL (any key = KEY, K1 KEY, K2 SCALE, K3 OCTAVE); tap GLO (K1 GATE, K2 DIV, K3 SWING, K4 BPM); SAVE to keep it | header `C MINPEN` on every screen | W8, W9, W11, W12 |

Gestures 1-3 plus PLAY make sound on a TB-3 (4 gestures); the only hold is gesture 2. Save and load: tap SAVE
(`SAVED P01`); to save elsewhere turn PRESETS to an empty slot first; a filled slot loads at the next loop start and the
header shows `P03>` until then (decision 34).

## 5. What stays in the MENU (hold HOME 0.7 s): settings only

ALGORITHM = tab, PRESETS = row, K1-K4 = value, HOME tap closes (Felucca menu navigation, unchanged).

- **DISPLAY:** COLOR, STYLE, LARGE, ANIM, LEDS, SCREEN OFF, SCOPE, HOME view.
- **CONTROL:** HOLD (the 0.3-0.6 s tap/hold threshold), ACCEL, SCALE LEDS, STEP PREVIEW.
- **AUDIO:** SPEAKER EQ, USB LEVEL, CLICK, CLICK LEVEL, COUNT-IN, TUNE.
- **MIDI:** MIDI IN routing, AUTO SELECT, SCREEN WAKE, LEARN CLEAR.
- **SYSTEM:** USB SERIAL,
  RESTORE LAST, HELP, CALIBRATION, INFO, ABOUT.

Moved out because they are musical functions now on the panel: `MI_TCH1..4` (CHAN layer), `MI_SELECT` (SELECT is fixed
to channel select), `MI_BPMLOCK` (tempo is on GLO / tap tempo), `MI_T1SEQ` (the sequencer is always on, decision 32b), `MI_LATCH` (no FX layer). Row counts change in `menu_items.c`, so the dev task updates `tests/ui_test.c`.

## 6. Conflicts from the T-1 guide, resolved

`TORSO_T1_GUIDE.md` section 2 disagrees with the code and with itself (CONTROL_INVENTORY 1.4). The v1 map replaces all of
it; this table says what each guide claim becomes.

| Guide claim / T-1 behaviour | Reality today | Resolution in this map |
| --- | --- | --- |
| REC arms real-time recording | REC is unreferenced in T1 mode | REC tap = STEP entry (pins on white keys); hold 0.7 s = clear pins |
| Keys 1-16 hold = step P-LOCK | keys select track/cycle; `edit_step` never set | White key in STEP toggles a pin; hold it + K3 / OCT edits the pin; P-LOCK is dropped |
| Page navigation via SEQ / OCT- / OCT+ | SEQ next page, OCT- previous, OCT+ is SHIFT | No pages and no SHIFT: K1-K4 always morph the selected channel; SEQ is the HITS dice; OCT keys are keyboard octave / pin octave |
| 6 tracks x 6 cycles | 6 tracks, track 1 on the synth | 4 channels (decision 31); every channel sends MIDI; no cycles |
| Black keys 1-6 mute, white keys 1-6 select track, 7-12 cycle | as the guide | Black keys F#3..C#4 mute CH1-4, D#4..A#4 select CH1-4; white keys play or enter steps |
| SAVE or LFO held = TEMP snapshot; OCT+ commits | ui_t1.c:56-68 | SAVE tap saves the preset, SAVE hold 0.7 s undoes a load; LFO is the SHAPE dice; TEMP is dropped |
| GLO held = relative delta on all tracks | ui_t1.c:69-72 | GLO opens the GLOB page: GATE, DIV, SWING, BPM are global by nature (decision 4); no per-track delta |
| Page 4 K4 shift = "LoopSync", MOD/CC page, RATCHET page, HARMONY page | chord voices etc. | Not in v1; ratchet is the ARTIC layer and scale / key live in the SCL layer |
| LFO-held MOMENTARY (Felucca) | knob moves snap back on release | Dropped (LFO = SHAPE dice); revisit after the first hardware test (decision 36) |

## 7. Flex points (ADR-0004 addendum)

What a producer can re-map by **editing one row**, with no algorithm or screen code change.

| Table | Row / field | What a row edit changes |
| --- | --- | --- |
| `control_map.tsv` | action, label for a (control, context) | Any button, encoder, knob or key role in any context, for example swap FX and EDIT, move UNDO to OCT-, move mutes to other black keys |
| Context table (section 1) | hold thresholds, enter/leave gesture | Make GLOB hold-only, change the REC hold time, change the 0.4 s default via the HOLD menu setting |
| Knob table (UX_REVIEW 5.1) | slot, label, range, default, step | Remap K1..K4 per algorithm, rename `PEDAL`, change SNAKE RANGE default, STAGE TOTAL coarse step |
| Dice table (UX_REVIEW 5.2) | button -> layer, sub-label, seeds | Bind SLIDE alone to its own gesture, rename a sub-label, mark a layer absent |
| Global table (UX_REVIEW 6) | scale list order, defaults | Default scale, key, octave; scale order on K2 |
| Black-key rows | key -> MUTEn / CHn / ALL ON / RESTRT / TAP | Re-place channel keys if the channel count changes (6 channels would need a second CHAN row and moved select keys) |
| Defaults (section 0) | channel algorithms, MIDI channels, mutes | Power-on state |

Not changeable without code: the seven contexts and their precedence; tap-on-release for buttons with a hold role
(`btn_hold`, `ui_layer.c`); the five shared dice layers and their meanings; scale snap rule and output order; the queued
preset load at the loop start; the main-screen bands.

## Open owner questions from this consolidation

None. No conflict or gap was found between UX_REVIEW.md, control_map.tsv and the owner decisions.

## Appendix A. Every (control, context) row, verbatim from control_map.tsv

Columns: control, context, action, screen label. Checked by `python3 tools/check_control_map.py`.

| Control | Context | Action | Label |
| --- | --- | --- | --- |
<!-- BEGIN-APPENDIX-A -->
| `B_FX` | `PERF` | press: DICE ALL layers present in the channel's algorithm (order NOTES HITS ACCNT ARTIC SHAPE); hold = same | `ALL` |
| `B_FX` | `STEP` | press: DICE ALL layers present in the channel's algorithm (order NOTES HITS ACCNT ARTIC SHAPE); hold = same | `ALL` |
| `B_FX` | `STEP_HELD` | press: DICE ALL layers present in the channel's algorithm (order NOTES HITS ACCNT ARTIC SHAPE); hold = same | `ALL` |
| `B_FX` | `SCL` | press: DICE ALL layers present in the channel's algorithm (order NOTES HITS ACCNT ARTIC SHAPE); hold = same | `ALL` |
| `B_FX` | `CHAN` | press: DICE ALL layers present in the channel's algorithm (order NOTES HITS ACCNT ARTIC SHAPE); hold = same | `ALL` |
| `B_FX` | `GLOB` | press: DICE ALL layers present in the channel's algorithm (order NOTES HITS ACCNT ARTIC SHAPE); hold = same | `ALL` |
| `B_FX` | `MENU` | press: closes the menu (no other action) | `close` |
| `B_SCL` | `PERF` | tap (release before HOLD): dice NOTES; hold past HOLD: open SCL layer (no dice) | `NOTES +sub` |
| `B_SCL` | `STEP` | tap (release before HOLD): dice NOTES; hold past HOLD: open SCL layer (no dice) | `NOTES +sub` |
| `B_SCL` | `STEP_HELD` | tap (release before HOLD): dice NOTES; hold past HOLD: open SCL layer (no dice) | `NOTES +sub` |
| `B_SCL` | `SCL` | held: keeps the SCL layer open; release closes it | `SCALE` |
| `B_SCL` | `CHAN` | tap (release before HOLD): dice NOTES; hold past HOLD: open SCL layer (no dice) | `NOTES +sub` |
| `B_SCL` | `GLOB` | tap (release before HOLD): dice NOTES; hold past HOLD: open SCL layer (no dice) | `NOTES +sub` |
| `B_SCL` | `MENU` | press: closes the menu (no other action) | `close` |
| `B_ENV` | `PERF` | press: dice ACCNT | `ACCNT +sub` |
| `B_ENV` | `STEP` | press: dice ACCNT | `ACCNT +sub` |
| `B_ENV` | `STEP_HELD` | press: dice ACCNT | `ACCNT +sub` |
| `B_ENV` | `SCL` | press: dice ACCNT | `ACCNT +sub` |
| `B_ENV` | `CHAN` | press: dice ACCNT | `ACCNT +sub` |
| `B_ENV` | `GLOB` | press: dice ACCNT | `ACCNT +sub` |
| `B_ENV` | `MENU` | press: closes the menu (no other action) | `close` |
| `B_LFO` | `PERF` | press: dice SHAPE (dimmed and inert if the algorithm has no SHAPE layer) | `SHAPE +sub` |
| `B_LFO` | `STEP` | press: dice SHAPE (dimmed and inert if the algorithm has no SHAPE layer) | `SHAPE +sub` |
| `B_LFO` | `STEP_HELD` | press: dice SHAPE (dimmed and inert if the algorithm has no SHAPE layer) | `SHAPE +sub` |
| `B_LFO` | `SCL` | press: dice SHAPE (dimmed and inert if the algorithm has no SHAPE layer) | `SHAPE +sub` |
| `B_LFO` | `CHAN` | press: dice SHAPE (dimmed and inert if the algorithm has no SHAPE layer) | `SHAPE +sub` |
| `B_LFO` | `GLOB` | press: dice SHAPE (dimmed and inert if the algorithm has no SHAPE layer) | `SHAPE +sub` |
| `B_LFO` | `MENU` | press: closes the menu (no other action) | `close` |
| `B_EDIT` | `PERF` | tap (release before HOLD): UNDO/REDO the last dice, dice-all or pin-clear (swaps current and previous seeds); hold past HOLD: open CHAN layer | `UNDO` |
| `B_EDIT` | `STEP` | tap (release before HOLD): UNDO/REDO the last dice, dice-all or pin-clear (swaps current and previous seeds); hold past HOLD: open CHAN layer | `UNDO` |
| `B_EDIT` | `STEP_HELD` | tap (release before HOLD): UNDO/REDO the last dice, dice-all or pin-clear (swaps current and previous seeds); hold past HOLD: open CHAN layer | `UNDO` |
| `B_EDIT` | `SCL` | tap (release before HOLD): UNDO/REDO the last dice, dice-all or pin-clear (swaps current and previous seeds); hold past HOLD: open CHAN layer | `UNDO` |
| `B_EDIT` | `CHAN` | held: keeps the CHAN layer open; release closes it | `CHAN` |
| `B_EDIT` | `GLOB` | tap (release before HOLD): UNDO/REDO the last dice, dice-all or pin-clear (swaps current and previous seeds); hold past HOLD: open CHAN layer | `UNDO` |
| `B_EDIT` | `MENU` | press: closes the menu (no other action) | `close` |
| `B_GLO` | `PERF` | tap: open GLOB page (latched); hold past HOLD: GLOB page while held | `GLOBAL` |
| `B_GLO` | `STEP` | tap: open GLOB page (latched); hold past HOLD: GLOB page while held | `GLOBAL` |
| `B_GLO` | `STEP_HELD` | tap: open GLOB page (latched); hold past HOLD: GLOB page while held | `GLOBAL` |
| `B_GLO` | `SCL` | tap: open GLOB page (latched); hold past HOLD: GLOB page while held | `GLOBAL` |
| `B_GLO` | `CHAN` | tap: open GLOB page (latched); hold past HOLD: GLOB page while held | `GLOBAL` |
| `B_GLO` | `GLOB` | tap: close GLOB page (back to PERF); if opened by hold, release closes it | `BACK` |
| `B_GLO` | `MENU` | press: closes the menu (no other action) | `close` |
| `B_HOME` | `PERF` | tap: toggle the KEYS legend overlay on the main screen; hold 0.7 s: open MENU | `KEYS / hold MENU` |
| `B_HOME` | `STEP` | tap: leave STEP (same as REC tap) and return to PERF; hold 0.7 s: open MENU | `HOME` |
| `B_HOME` | `STEP_HELD` | tap: leave STEP and return to PERF; hold 0.7 s: open MENU | `HOME` |
| `B_HOME` | `SCL` | tap: return to PERF (layer closes when SCL is let go); hold 0.7 s: open MENU | `HOME` |
| `B_HOME` | `CHAN` | tap: return to PERF (layer closes when EDIT is let go); hold 0.7 s: open MENU | `HOME` |
| `B_HOME` | `GLOB` | tap: close GLOB page, return to PERF; hold 0.7 s: open MENU | `HOME` |
| `B_HOME` | `MENU` | tap or hold 0.7 s: close the menu | `BACK` |
| `B_SAVE` | `PERF` | tap: save the whole device state into the preset slot shown in the header; hold 0.7 s: undo the last preset load (again: redo) | `SAVE Pnn` |
| `B_SAVE` | `STEP` | tap: save the whole device state into the preset slot shown in the header; hold 0.7 s: undo the last preset load (again: redo) | `SAVE Pnn` |
| `B_SAVE` | `STEP_HELD` | tap: save the whole device state into the preset slot shown in the header; hold 0.7 s: undo the last preset load (again: redo) | `SAVE Pnn` |
| `B_SAVE` | `SCL` | tap: save the whole device state into the preset slot shown in the header; hold 0.7 s: undo the last preset load (again: redo) | `SAVE Pnn` |
| `B_SAVE` | `CHAN` | tap: save the whole device state into the preset slot shown in the header; hold 0.7 s: undo the last preset load (again: redo) | `SAVE Pnn` |
| `B_SAVE` | `GLOB` | tap: save the whole device state into the preset slot shown in the header; hold 0.7 s: undo the last preset load (again: redo) | `SAVE Pnn` |
| `B_SAVE` | `MENU` | press: closes the menu (no other action) | `close` |
| `B_ARP` | `PERF` | press: dice ARTIC (dimmed and inert if the algorithm has no ARTIC layer) | `ARTIC +sub` |
| `B_ARP` | `STEP` | press: dice ARTIC (dimmed and inert if the algorithm has no ARTIC layer) | `ARTIC +sub` |
| `B_ARP` | `STEP_HELD` | press: dice ARTIC (dimmed and inert if the algorithm has no ARTIC layer) | `ARTIC +sub` |
| `B_ARP` | `SCL` | press: dice ARTIC (dimmed and inert if the algorithm has no ARTIC layer) | `ARTIC +sub` |
| `B_ARP` | `CHAN` | press: dice ARTIC (dimmed and inert if the algorithm has no ARTIC layer) | `ARTIC +sub` |
| `B_ARP` | `GLOB` | press: dice ARTIC (dimmed and inert if the algorithm has no ARTIC layer) | `ARTIC +sub` |
| `B_ARP` | `MENU` | press: closes the menu (no other action) | `close` |
| `B_SEQ` | `PERF` | press: dice HITS | `HITS +sub` |
| `B_SEQ` | `STEP` | press: dice HITS | `HITS +sub` |
| `B_SEQ` | `STEP_HELD` | press: dice HITS | `HITS +sub` |
| `B_SEQ` | `SCL` | press: dice HITS | `HITS +sub` |
| `B_SEQ` | `CHAN` | press: dice HITS | `HITS +sub` |
| `B_SEQ` | `GLOB` | press: dice HITS | `HITS +sub` |
| `B_SEQ` | `MENU` | press: closes the menu (no other action) | `close` |
| `B_PLAY` | `PERF` | press: start / stop all channels | `PLAY / STOP` |
| `B_PLAY` | `STEP` | press: start / stop all channels | `PLAY / STOP` |
| `B_PLAY` | `STEP_HELD` | press: start / stop all channels | `PLAY / STOP` |
| `B_PLAY` | `SCL` | press: start / stop all channels | `PLAY / STOP` |
| `B_PLAY` | `CHAN` | press: start / stop all channels | `PLAY / STOP` |
| `B_PLAY` | `GLOB` | press: start / stop all channels | `PLAY / STOP` |
| `B_PLAY` | `MENU` | press: start / stop all channels | `PLAY / STOP` |
| `B_REC` | `PERF` | tap (release before 0.7 s): enter STEP; hold 0.7 s: clear every pin of the selected channel (undoable with EDIT tap) | `STEP` |
| `B_REC` | `STEP` | tap: leave STEP (back to PERF); hold 0.7 s: clear every pin of the selected channel | `STEP ON` |
| `B_REC` | `STEP_HELD` | tap: leave STEP (the held step key's pin stays); hold 0.7 s: clear every pin of the selected channel | `STEP ON` |
| `B_REC` | `SCL` | tap (release before 0.7 s): enter STEP; hold 0.7 s: clear every pin of the selected channel (undoable with EDIT tap) | `STEP` |
| `B_REC` | `CHAN` | tap (release before 0.7 s): enter STEP; hold 0.7 s: clear every pin of the selected channel (undoable with EDIT tap) | `STEP` |
| `B_REC` | `GLOB` | tap (release before 0.7 s): enter STEP; hold 0.7 s: clear every pin of the selected channel (undoable with EDIT tap) | `STEP` |
| `B_REC` | `MENU` | press: closes the menu (no other action) | `close` |
| `B_OCTDN` | `PERF` | press: keyboard octave -1 (range -3..+3; both OCT = 0; both held 5 s = UPDATE MODE) | `OCT n` |
| `B_OCTDN` | `STEP` | press: keyboard octave -1 (range -3..+3; both OCT = 0; both held 5 s = UPDATE MODE) | `OCT n` |
| `B_OCTDN` | `STEP_HELD` | press: the held step(s)' pin note -12 semitones (clamped 0..96), auditioned | `PIN OCT` |
| `B_OCTDN` | `SCL` | press: keyboard octave -1 (range -3..+3; both OCT = 0; both held 5 s = UPDATE MODE) | `OCT n` |
| `B_OCTDN` | `CHAN` | press: keyboard octave -1 (range -3..+3; both OCT = 0; both held 5 s = UPDATE MODE) | `OCT n` |
| `B_OCTDN` | `GLOB` | press: keyboard octave -1 (range -3..+3; both OCT = 0; both held 5 s = UPDATE MODE) | `OCT n` |
| `B_OCTDN` | `MENU` | release: previous value / back (Felucca menu rule) | `-` |
| `B_OCTUP` | `PERF` | press: keyboard octave +1 (range -3..+3; both OCT = 0; both held 5 s = UPDATE MODE) | `OCT n` |
| `B_OCTUP` | `STEP` | press: keyboard octave +1 (range -3..+3; both OCT = 0; both held 5 s = UPDATE MODE) | `OCT n` |
| `B_OCTUP` | `STEP_HELD` | press: the held step(s)' pin note +12 semitones (clamped 0..96), auditioned | `PIN OCT` |
| `B_OCTUP` | `SCL` | press: keyboard octave +1 (range -3..+3; both OCT = 0; both held 5 s = UPDATE MODE) | `OCT n` |
| `B_OCTUP` | `CHAN` | press: keyboard octave +1 (range -3..+3; both OCT = 0; both held 5 s = UPDATE MODE) | `OCT n` |
| `B_OCTUP` | `GLOB` | press: keyboard octave +1 (range -3..+3; both OCT = 0; both held 5 s = UPDATE MODE) | `OCT n` |
| `B_OCTUP` | `MENU` | release: next value / do it (opens CALIBRATION, INFO, ABOUT rows) | `-` |
| `EN_SELECT` | `PERF` | turn: select channel CH1..CH4 (clamped, no wrap) | `CH n` |
| `EN_SELECT` | `STEP` | turn: select channel CH1..CH4 (clamped, no wrap) | `CH n` |
| `EN_SELECT` | `STEP_HELD` | turn: select channel CH1..CH4 (clamped, no wrap) | `CH n` |
| `EN_SELECT` | `SCL` | turn: select channel CH1..CH4 (clamped, no wrap) | `CH n` |
| `EN_SELECT` | `CHAN` | turn: select channel CH1..CH4 (clamped, no wrap) | `CH n` |
| `EN_SELECT` | `GLOB` | turn: select channel CH1..CH4 (clamped, no wrap) | `CH n` |
| `EN_SELECT` | `MENU` | turn: select channel CH1..CH4 (clamped, no wrap) | `CH n` |
| `EN_ALGO` | `PERF` | turn: algorithm of the selected channel ACID > EUCLID > SNAKE > STAGE (clamped); switch lands at the next step; each algorithm's morphs and seeds are kept per channel (S7) | `ALGO name` |
| `EN_ALGO` | `STEP` | turn: algorithm of the selected channel ACID > EUCLID > SNAKE > STAGE (clamped); switch lands at the next step; each algorithm's morphs and seeds are kept per channel (S7) | `ALGO name` |
| `EN_ALGO` | `STEP_HELD` | turn: algorithm of the selected channel ACID > EUCLID > SNAKE > STAGE (clamped); switch lands at the next step; each algorithm's morphs and seeds are kept per channel (S7) | `ALGO name` |
| `EN_ALGO` | `SCL` | turn: algorithm of the selected channel ACID > EUCLID > SNAKE > STAGE (clamped); switch lands at the next step; each algorithm's morphs and seeds are kept per channel (S7) | `ALGO name` |
| `EN_ALGO` | `CHAN` | turn: algorithm of the selected channel ACID > EUCLID > SNAKE > STAGE (clamped); switch lands at the next step; each algorithm's morphs and seeds are kept per channel (S7) | `ALGO name` |
| `EN_ALGO` | `GLOB` | turn: algorithm of the selected channel ACID > EUCLID > SNAKE > STAGE (clamped); switch lands at the next step; each algorithm's morphs and seeds are kept per channel (S7) | `ALGO name` |
| `EN_ALGO` | `MENU` | turn: next / previous menu tab (wraps) | `tab` |
| `EN_PRESET` | `PERF` | turn: preset slot P01..P16; a filled slot loads at the next loop start (immediately when stopped); an empty slot only selects the target for SAVE | `Pnn` |
| `EN_PRESET` | `STEP` | turn: preset slot P01..P16; a filled slot loads at the next loop start (immediately when stopped); an empty slot only selects the target for SAVE | `Pnn` |
| `EN_PRESET` | `STEP_HELD` | turn: preset slot P01..P16; a filled slot loads at the next loop start (immediately when stopped); an empty slot only selects the target for SAVE | `Pnn` |
| `EN_PRESET` | `SCL` | turn: preset slot P01..P16; a filled slot loads at the next loop start (immediately when stopped); an empty slot only selects the target for SAVE | `Pnn` |
| `EN_PRESET` | `CHAN` | turn: preset slot P01..P16; a filled slot loads at the next loop start (immediately when stopped); an empty slot only selects the target for SAVE | `Pnn` |
| `EN_PRESET` | `GLOB` | turn: preset slot P01..P16; a filled slot loads at the next loop start (immediately when stopped); an empty slot only selects the target for SAVE | `Pnn` |
| `EN_PRESET` | `MENU` | turn: next / previous menu row (wraps) | `row` |
| `MASTER` | `PERF` | analogue pot: preview-synth and USB-audio volume (song.master_q12); no MIDI effect | `-` |
| `MASTER` | `STEP` | analogue pot: preview-synth and USB-audio volume (song.master_q12); no MIDI effect | `-` |
| `MASTER` | `STEP_HELD` | analogue pot: preview-synth and USB-audio volume (song.master_q12); no MIDI effect | `-` |
| `MASTER` | `SCL` | analogue pot: preview-synth and USB-audio volume (song.master_q12); no MIDI effect | `-` |
| `MASTER` | `CHAN` | analogue pot: preview-synth and USB-audio volume (song.master_q12); no MIDI effect | `-` |
| `MASTER` | `GLOB` | analogue pot: preview-synth and USB-audio volume (song.master_q12); no MIDI effect | `-` |
| `MASTER` | `MENU` | analogue pot: preview-synth and USB-audio volume (song.master_q12); no MIDI effect | `-` |
| `EN_K1` | `PERF` | turn: morph K1 DENSITY slot of the selected channel: ACID DENS 0..16 / EUCLID HITS 0..16 / SNAKE DENS 0..16 / STAGE DENS 0..8; 0 = KEYS ONLY (only pins play) (one detent = one step; STAGE TOTAL steps 4, 1 when turned slowly) | `algo label` |
| `EN_K1` | `STEP` | turn: morph K1 DENSITY slot of the selected channel: ACID DENS 0..16 / EUCLID HITS 0..16 / SNAKE DENS 0..16 / STAGE DENS 0..8; 0 = KEYS ONLY (only pins play) (one detent = one step; STAGE TOTAL steps 4, 1 when turned slowly) | `algo label` |
| `EN_K1` | `STEP_HELD` | turn: morph K1 DENSITY slot of the selected channel: ACID DENS 0..16 / EUCLID HITS 0..16 / SNAKE DENS 0..16 / STAGE DENS 0..8; 0 = KEYS ONLY (only pins play) (one detent = one step; STAGE TOTAL steps 4, 1 when turned slowly) | `algo label` |
| `EN_K1` | `SCL` | turn: KEY (global root) C..B | `KEY` |
| `EN_K1` | `CHAN` | turn: MIDI OUT channel of CH1: 1..16, OFF | `CH1 MID` |
| `EN_K1` | `GLOB` | turn: GATE 10..100 % (global gate length) | `GATE` |
| `EN_K1` | `MENU` | turn: change the selected row's value (K1..K4 equivalent) | `value` |
| `EN_K2` | `PERF` | turn: morph K2 FORM slot of the selected channel: ACID PEDAL 0..12 / EUCLID LEN 2..16 / SNAKE LEN 1..16 / STAGE TOTAL 8..64 (one detent = one step; STAGE TOTAL steps 4, 1 when turned slowly) | `algo label` |
| `EN_K2` | `STEP` | turn: morph K2 FORM slot of the selected channel: ACID PEDAL 0..12 / EUCLID LEN 2..16 / SNAKE LEN 1..16 / STAGE TOTAL 8..64 (one detent = one step; STAGE TOTAL steps 4, 1 when turned slowly) | `algo label` |
| `EN_K2` | `STEP_HELD` | turn: morph K2 FORM slot of the selected channel: ACID PEDAL 0..12 / EUCLID LEN 2..16 / SNAKE LEN 1..16 / STAGE TOTAL 8..64 (one detent = one step; STAGE TOTAL steps 4, 1 when turned slowly) | `algo label` |
| `EN_K2` | `SCL` | turn: SCALE (global) CHROM..DIM, 16 entries | `SCALE` |
| `EN_K2` | `CHAN` | turn: MIDI OUT channel of CH2: 1..16, OFF | `CH2 MID` |
| `EN_K2` | `GLOB` | turn: DIV 1/32, 1/16T, 1/16, 1/8T, 1/8, 1/4 (global step rate) | `DIV` |
| `EN_K2` | `MENU` | turn: change the selected row's value (K1..K4 equivalent) | `value` |
| `EN_K3` | `PERF` | turn: morph K3 PITCH slot of the selected channel: ACID JUMP 0..20 / EUCLID SPAN 0..24 / SNAKE RANGE 0..48 / STAGE RANGE 0..36 (one detent = one step; STAGE TOTAL steps 4, 1 when turned slowly) | `algo label` |
| `EN_K3` | `STEP` | turn: morph K3 PITCH slot of the selected channel: ACID JUMP 0..20 / EUCLID SPAN 0..24 / SNAKE RANGE 0..48 / STAGE RANGE 0..36 (one detent = one step; STAGE TOTAL steps 4, 1 when turned slowly) | `algo label` |
| `EN_K3` | `STEP_HELD` | turn: NOTE of the held step(s)' pin, one scale degree per detent (snapped, 0..96), auditioned | `NOTE` |
| `EN_K3` | `SCL` | turn: OCTAVE (global output octave) -2..+2 | `OCTAVE` |
| `EN_K3` | `CHAN` | turn: MIDI OUT channel of CH3: 1..16, OFF | `CH3 MID` |
| `EN_K3` | `GLOB` | turn: SWING 50..75 % (global) | `SWING` |
| `EN_K3` | `MENU` | turn: change the selected row's value (K1..K4 equivalent) | `value` |
| `EN_K4` | `PERF` | turn: morph K4 SPICE slot of the selected channel: ACID ACCNT 0..8 / EUCLID ACCNT 0..8 / SNAKE PATH 1..14 / STAGE RATCH 0..8 (one detent = one step; STAGE TOTAL steps 4, 1 when turned slowly) | `algo label` |
| `EN_K4` | `STEP` | turn: morph K4 SPICE slot of the selected channel: ACID ACCNT 0..8 / EUCLID ACCNT 0..8 / SNAKE PATH 1..14 / STAGE RATCH 0..8 (one detent = one step; STAGE TOTAL steps 4, 1 when turned slowly) | `algo label` |
| `EN_K4` | `STEP_HELD` | turn: morph K4 SPICE slot of the selected channel: ACID ACCNT 0..8 / EUCLID ACCNT 0..8 / SNAKE PATH 1..14 / STAGE RATCH 0..8 (one detent = one step; STAGE TOTAL steps 4, 1 when turned slowly) | `algo label` |
| `EN_K4` | `SCL` | turn: morph K4 SPICE slot of the selected channel: ACID ACCNT 0..8 / EUCLID ACCNT 0..8 / SNAKE PATH 1..14 / STAGE RATCH 0..8 (one detent = one step; STAGE TOTAL steps 4, 1 when turned slowly) | `algo label` |
| `EN_K4` | `CHAN` | turn: MIDI OUT channel of CH4: 1..16, OFF | `CH4 MID` |
| `EN_K4` | `GLOB` | turn: BPM 40..240 (internal clock; shows EXT when slaved) | `BPM` |
| `EN_K4` | `MENU` | turn: change the selected row's value (K1..K4 equivalent) | `value` |
| `KEY_W01_F3` | `PERF` | press: play F3 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W01_F3` | `STEP` | tap: toggle the pin on step 1 (new pin = the note generated there, or the root when silent; auditioned); a pinned step is cleared on release unless K3 or OCT moved while held; hold = STEP_HELD | `step 1` |
| `KEY_W01_F3` | `STEP_HELD` | press: also hold step 1 (K3 and OCT edit every held step); release: as STEP | `step 1` |
| `KEY_W01_F3` | `SCL` | press: set the global KEY to F (pitch class) | `KEY` |
| `KEY_W01_F3` | `CHAN` | press: play F3 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W01_F3` | `GLOB` | press: play F3 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W01_F3` | `MENU` | press: play F3 live (as PERF; the menu stays open) | `-` |
| `KEY_W02_G3` | `PERF` | press: play G3 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W02_G3` | `STEP` | tap: toggle the pin on step 2 (new pin = the note generated there, or the root when silent; auditioned); a pinned step is cleared on release unless K3 or OCT moved while held; hold = STEP_HELD | `step 2` |
| `KEY_W02_G3` | `STEP_HELD` | press: also hold step 2 (K3 and OCT edit every held step); release: as STEP | `step 2` |
| `KEY_W02_G3` | `SCL` | press: set the global KEY to G (pitch class) | `KEY` |
| `KEY_W02_G3` | `CHAN` | press: play G3 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W02_G3` | `GLOB` | press: play G3 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W02_G3` | `MENU` | press: play G3 live (as PERF; the menu stays open) | `-` |
| `KEY_W03_A3` | `PERF` | press: play A3 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W03_A3` | `STEP` | tap: toggle the pin on step 3 (new pin = the note generated there, or the root when silent; auditioned); a pinned step is cleared on release unless K3 or OCT moved while held; hold = STEP_HELD | `step 3` |
| `KEY_W03_A3` | `STEP_HELD` | press: also hold step 3 (K3 and OCT edit every held step); release: as STEP | `step 3` |
| `KEY_W03_A3` | `SCL` | press: set the global KEY to A (pitch class) | `KEY` |
| `KEY_W03_A3` | `CHAN` | press: play A3 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W03_A3` | `GLOB` | press: play A3 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W03_A3` | `MENU` | press: play A3 live (as PERF; the menu stays open) | `-` |
| `KEY_W04_B3` | `PERF` | press: play B3 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W04_B3` | `STEP` | tap: toggle the pin on step 4 (new pin = the note generated there, or the root when silent; auditioned); a pinned step is cleared on release unless K3 or OCT moved while held; hold = STEP_HELD | `step 4` |
| `KEY_W04_B3` | `STEP_HELD` | press: also hold step 4 (K3 and OCT edit every held step); release: as STEP | `step 4` |
| `KEY_W04_B3` | `SCL` | press: set the global KEY to B (pitch class) | `KEY` |
| `KEY_W04_B3` | `CHAN` | press: play B3 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W04_B3` | `GLOB` | press: play B3 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W04_B3` | `MENU` | press: play B3 live (as PERF; the menu stays open) | `-` |
| `KEY_W05_C4` | `PERF` | press: play C4 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W05_C4` | `STEP` | tap: toggle the pin on step 5 (new pin = the note generated there, or the root when silent; auditioned); a pinned step is cleared on release unless K3 or OCT moved while held; hold = STEP_HELD | `step 5` |
| `KEY_W05_C4` | `STEP_HELD` | press: also hold step 5 (K3 and OCT edit every held step); release: as STEP | `step 5` |
| `KEY_W05_C4` | `SCL` | press: set the global KEY to C (pitch class) | `KEY` |
| `KEY_W05_C4` | `CHAN` | press: play C4 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W05_C4` | `GLOB` | press: play C4 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W05_C4` | `MENU` | press: play C4 live (as PERF; the menu stays open) | `-` |
| `KEY_W06_D4` | `PERF` | press: play D4 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W06_D4` | `STEP` | tap: toggle the pin on step 6 (new pin = the note generated there, or the root when silent; auditioned); a pinned step is cleared on release unless K3 or OCT moved while held; hold = STEP_HELD | `step 6` |
| `KEY_W06_D4` | `STEP_HELD` | press: also hold step 6 (K3 and OCT edit every held step); release: as STEP | `step 6` |
| `KEY_W06_D4` | `SCL` | press: set the global KEY to D (pitch class) | `KEY` |
| `KEY_W06_D4` | `CHAN` | press: play D4 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W06_D4` | `GLOB` | press: play D4 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W06_D4` | `MENU` | press: play D4 live (as PERF; the menu stays open) | `-` |
| `KEY_W07_E4` | `PERF` | press: play E4 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W07_E4` | `STEP` | tap: toggle the pin on step 7 (new pin = the note generated there, or the root when silent; auditioned); a pinned step is cleared on release unless K3 or OCT moved while held; hold = STEP_HELD | `step 7` |
| `KEY_W07_E4` | `STEP_HELD` | press: also hold step 7 (K3 and OCT edit every held step); release: as STEP | `step 7` |
| `KEY_W07_E4` | `SCL` | press: set the global KEY to E (pitch class) | `KEY` |
| `KEY_W07_E4` | `CHAN` | press: play E4 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W07_E4` | `GLOB` | press: play E4 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W07_E4` | `MENU` | press: play E4 live (as PERF; the menu stays open) | `-` |
| `KEY_W08_F4` | `PERF` | press: play F4 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W08_F4` | `STEP` | tap: toggle the pin on step 8 (new pin = the note generated there, or the root when silent; auditioned); a pinned step is cleared on release unless K3 or OCT moved while held; hold = STEP_HELD | `step 8` |
| `KEY_W08_F4` | `STEP_HELD` | press: also hold step 8 (K3 and OCT edit every held step); release: as STEP | `step 8` |
| `KEY_W08_F4` | `SCL` | press: set the global KEY to F (pitch class) | `KEY` |
| `KEY_W08_F4` | `CHAN` | press: play F4 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W08_F4` | `GLOB` | press: play F4 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W08_F4` | `MENU` | press: play F4 live (as PERF; the menu stays open) | `-` |
| `KEY_W09_G4` | `PERF` | press: play G4 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W09_G4` | `STEP` | tap: toggle the pin on step 9 (new pin = the note generated there, or the root when silent; auditioned); a pinned step is cleared on release unless K3 or OCT moved while held; hold = STEP_HELD | `step 9` |
| `KEY_W09_G4` | `STEP_HELD` | press: also hold step 9 (K3 and OCT edit every held step); release: as STEP | `step 9` |
| `KEY_W09_G4` | `SCL` | press: set the global KEY to G (pitch class) | `KEY` |
| `KEY_W09_G4` | `CHAN` | press: play G4 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W09_G4` | `GLOB` | press: play G4 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W09_G4` | `MENU` | press: play G4 live (as PERF; the menu stays open) | `-` |
| `KEY_W10_A4` | `PERF` | press: play A4 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W10_A4` | `STEP` | tap: toggle the pin on step 10 (new pin = the note generated there, or the root when silent; auditioned); a pinned step is cleared on release unless K3 or OCT moved while held; hold = STEP_HELD | `step 10` |
| `KEY_W10_A4` | `STEP_HELD` | press: also hold step 10 (K3 and OCT edit every held step); release: as STEP | `step 10` |
| `KEY_W10_A4` | `SCL` | press: set the global KEY to A (pitch class) | `KEY` |
| `KEY_W10_A4` | `CHAN` | press: play A4 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W10_A4` | `GLOB` | press: play A4 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W10_A4` | `MENU` | press: play A4 live (as PERF; the menu stays open) | `-` |
| `KEY_W11_B4` | `PERF` | press: play B4 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W11_B4` | `STEP` | tap: toggle the pin on step 11 (new pin = the note generated there, or the root when silent; auditioned); a pinned step is cleared on release unless K3 or OCT moved while held; hold = STEP_HELD | `step 11` |
| `KEY_W11_B4` | `STEP_HELD` | press: also hold step 11 (K3 and OCT edit every held step); release: as STEP | `step 11` |
| `KEY_W11_B4` | `SCL` | press: set the global KEY to B (pitch class) | `KEY` |
| `KEY_W11_B4` | `CHAN` | press: play B4 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W11_B4` | `GLOB` | press: play B4 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W11_B4` | `MENU` | press: play B4 live (as PERF; the menu stays open) | `-` |
| `KEY_W12_C5` | `PERF` | press: play C5 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W12_C5` | `STEP` | tap: toggle the pin on step 12 (new pin = the note generated there, or the root when silent; auditioned); a pinned step is cleared on release unless K3 or OCT moved while held; hold = STEP_HELD | `step 12` |
| `KEY_W12_C5` | `STEP_HELD` | press: also hold step 12 (K3 and OCT edit every held step); release: as STEP | `step 12` |
| `KEY_W12_C5` | `SCL` | press: set the global KEY to C (pitch class) | `KEY` |
| `KEY_W12_C5` | `CHAN` | press: play C5 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W12_C5` | `GLOB` | press: play C5 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W12_C5` | `MENU` | press: play C5 live (as PERF; the menu stays open) | `-` |
| `KEY_W13_D5` | `PERF` | press: play D5 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W13_D5` | `STEP` | tap: toggle the pin on step 13 (new pin = the note generated there, or the root when silent; auditioned); a pinned step is cleared on release unless K3 or OCT moved while held; hold = STEP_HELD | `step 13` |
| `KEY_W13_D5` | `STEP_HELD` | press: also hold step 13 (K3 and OCT edit every held step); release: as STEP | `step 13` |
| `KEY_W13_D5` | `SCL` | press: set the global KEY to D (pitch class) | `KEY` |
| `KEY_W13_D5` | `CHAN` | press: play D5 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W13_D5` | `GLOB` | press: play D5 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W13_D5` | `MENU` | press: play D5 live (as PERF; the menu stays open) | `-` |
| `KEY_W14_E5` | `PERF` | press: play E5 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W14_E5` | `STEP` | tap: toggle the pin on step 14 (new pin = the note generated there, or the root when silent; auditioned); a pinned step is cleared on release unless K3 or OCT moved while held; hold = STEP_HELD | `step 14` |
| `KEY_W14_E5` | `STEP_HELD` | press: also hold step 14 (K3 and OCT edit every held step); release: as STEP | `step 14` |
| `KEY_W14_E5` | `SCL` | press: set the global KEY to E (pitch class) | `KEY` |
| `KEY_W14_E5` | `CHAN` | press: play E5 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W14_E5` | `GLOB` | press: play E5 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W14_E5` | `MENU` | press: play E5 live (as PERF; the menu stays open) | `-` |
| `KEY_W15_F5` | `PERF` | press: play F5 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W15_F5` | `STEP` | tap: toggle the pin on step 15 (new pin = the note generated there, or the root when silent; auditioned); a pinned step is cleared on release unless K3 or OCT moved while held; hold = STEP_HELD | `step 15` |
| `KEY_W15_F5` | `STEP_HELD` | press: also hold step 15 (K3 and OCT edit every held step); release: as STEP | `step 15` |
| `KEY_W15_F5` | `SCL` | press: set the global KEY to F (pitch class) | `KEY` |
| `KEY_W15_F5` | `CHAN` | press: play F5 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W15_F5` | `GLOB` | press: play F5 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W15_F5` | `MENU` | press: play F5 live (as PERF; the menu stays open) | `-` |
| `KEY_W16_G5` | `PERF` | press: play G5 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W16_G5` | `STEP` | tap: toggle the pin on step 16 (new pin = the note generated there, or the root when silent; auditioned); a pinned step is cleared on release unless K3 or OCT moved while held; hold = STEP_HELD | `step 16` |
| `KEY_W16_G5` | `STEP_HELD` | press: also hold step 16 (K3 and OCT edit every held step); release: as STEP | `step 16` |
| `KEY_W16_G5` | `SCL` | press: set the global KEY to G (pitch class) | `KEY` |
| `KEY_W16_G5` | `CHAN` | press: play G5 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W16_G5` | `GLOB` | press: play G5 live on the selected channel (MIDI out + preview synth), snapped to the global scale, keyboard OCT applies | `-` |
| `KEY_W16_G5` | `MENU` | press: play G5 live (as PERF; the menu stays open) | `-` |
| `KEY_B01_F#3` | `PERF` | press: mute / unmute CH1 (latched) | `MUTE1` |
| `KEY_B01_F#3` | `STEP` | press: mute / unmute CH1 (latched) | `MUTE1` |
| `KEY_B01_F#3` | `STEP_HELD` | press: mute / unmute CH1 (latched) | `MUTE1` |
| `KEY_B01_F#3` | `SCL` | press: set the global KEY to F# (pitch class) | `KEY` |
| `KEY_B01_F#3` | `CHAN` | press: mute / unmute CH1 (latched) | `MUTE1` |
| `KEY_B01_F#3` | `GLOB` | press: mute / unmute CH1 (latched) | `MUTE1` |
| `KEY_B01_F#3` | `MENU` | press: mute / unmute CH1 (latched) | `MUTE1` |
| `KEY_B02_G#3` | `PERF` | press: mute / unmute CH2 (latched) | `MUTE2` |
| `KEY_B02_G#3` | `STEP` | press: mute / unmute CH2 (latched) | `MUTE2` |
| `KEY_B02_G#3` | `STEP_HELD` | press: mute / unmute CH2 (latched) | `MUTE2` |
| `KEY_B02_G#3` | `SCL` | press: set the global KEY to G# (pitch class) | `KEY` |
| `KEY_B02_G#3` | `CHAN` | press: mute / unmute CH2 (latched) | `MUTE2` |
| `KEY_B02_G#3` | `GLOB` | press: mute / unmute CH2 (latched) | `MUTE2` |
| `KEY_B02_G#3` | `MENU` | press: mute / unmute CH2 (latched) | `MUTE2` |
| `KEY_B03_A#3` | `PERF` | press: mute / unmute CH3 (latched) | `MUTE3` |
| `KEY_B03_A#3` | `STEP` | press: mute / unmute CH3 (latched) | `MUTE3` |
| `KEY_B03_A#3` | `STEP_HELD` | press: mute / unmute CH3 (latched) | `MUTE3` |
| `KEY_B03_A#3` | `SCL` | press: set the global KEY to A# (pitch class) | `KEY` |
| `KEY_B03_A#3` | `CHAN` | press: mute / unmute CH3 (latched) | `MUTE3` |
| `KEY_B03_A#3` | `GLOB` | press: mute / unmute CH3 (latched) | `MUTE3` |
| `KEY_B03_A#3` | `MENU` | press: mute / unmute CH3 (latched) | `MUTE3` |
| `KEY_B04_C#4` | `PERF` | press: mute / unmute CH4 (latched) | `MUTE4` |
| `KEY_B04_C#4` | `STEP` | press: mute / unmute CH4 (latched) | `MUTE4` |
| `KEY_B04_C#4` | `STEP_HELD` | press: mute / unmute CH4 (latched) | `MUTE4` |
| `KEY_B04_C#4` | `SCL` | press: set the global KEY to C# (pitch class) | `KEY` |
| `KEY_B04_C#4` | `CHAN` | press: mute / unmute CH4 (latched) | `MUTE4` |
| `KEY_B04_C#4` | `GLOB` | press: mute / unmute CH4 (latched) | `MUTE4` |
| `KEY_B04_C#4` | `MENU` | press: mute / unmute CH4 (latched) | `MUTE4` |
| `KEY_B05_D#4` | `PERF` | press: select CH1 | `CH1` |
| `KEY_B05_D#4` | `STEP` | press: select CH1 | `CH1` |
| `KEY_B05_D#4` | `STEP_HELD` | press: select CH1 | `CH1` |
| `KEY_B05_D#4` | `SCL` | press: set the global KEY to D# (pitch class) | `KEY` |
| `KEY_B05_D#4` | `CHAN` | press: select CH1 | `CH1` |
| `KEY_B05_D#4` | `GLOB` | press: select CH1 | `CH1` |
| `KEY_B05_D#4` | `MENU` | press: select CH1 | `CH1` |
| `KEY_B06_F#4` | `PERF` | press: select CH2 | `CH2` |
| `KEY_B06_F#4` | `STEP` | press: select CH2 | `CH2` |
| `KEY_B06_F#4` | `STEP_HELD` | press: select CH2 | `CH2` |
| `KEY_B06_F#4` | `SCL` | press: set the global KEY to F# (pitch class) | `KEY` |
| `KEY_B06_F#4` | `CHAN` | press: select CH2 | `CH2` |
| `KEY_B06_F#4` | `GLOB` | press: select CH2 | `CH2` |
| `KEY_B06_F#4` | `MENU` | press: select CH2 | `CH2` |
| `KEY_B07_G#4` | `PERF` | press: select CH3 | `CH3` |
| `KEY_B07_G#4` | `STEP` | press: select CH3 | `CH3` |
| `KEY_B07_G#4` | `STEP_HELD` | press: select CH3 | `CH3` |
| `KEY_B07_G#4` | `SCL` | press: set the global KEY to G# (pitch class) | `KEY` |
| `KEY_B07_G#4` | `CHAN` | press: select CH3 | `CH3` |
| `KEY_B07_G#4` | `GLOB` | press: select CH3 | `CH3` |
| `KEY_B07_G#4` | `MENU` | press: select CH3 | `CH3` |
| `KEY_B08_A#4` | `PERF` | press: select CH4 | `CH4` |
| `KEY_B08_A#4` | `STEP` | press: select CH4 | `CH4` |
| `KEY_B08_A#4` | `STEP_HELD` | press: select CH4 | `CH4` |
| `KEY_B08_A#4` | `SCL` | press: set the global KEY to A# (pitch class) | `KEY` |
| `KEY_B08_A#4` | `CHAN` | press: select CH4 | `CH4` |
| `KEY_B08_A#4` | `GLOB` | press: select CH4 | `CH4` |
| `KEY_B08_A#4` | `MENU` | press: select CH4 | `CH4` |
| `KEY_B09_C#5` | `PERF` | press: unmute all channels | `ALL ON` |
| `KEY_B09_C#5` | `STEP` | press: unmute all channels | `ALL ON` |
| `KEY_B09_C#5` | `STEP_HELD` | press: unmute all channels | `ALL ON` |
| `KEY_B09_C#5` | `SCL` | press: set the global KEY to C# (pitch class) | `KEY` |
| `KEY_B09_C#5` | `CHAN` | press: unmute all channels | `ALL ON` |
| `KEY_B09_C#5` | `GLOB` | press: unmute all channels | `ALL ON` |
| `KEY_B09_C#5` | `MENU` | press: unmute all channels | `ALL ON` |
| `KEY_B10_D#5` | `PERF` | press: restart all channels from step 1 at the next step | `RESTRT` |
| `KEY_B10_D#5` | `STEP` | press: restart all channels from step 1 at the next step | `RESTRT` |
| `KEY_B10_D#5` | `STEP_HELD` | press: restart all channels from step 1 at the next step | `RESTRT` |
| `KEY_B10_D#5` | `SCL` | press: set the global KEY to D# (pitch class) | `KEY` |
| `KEY_B10_D#5` | `CHAN` | press: restart all channels from step 1 at the next step | `RESTRT` |
| `KEY_B10_D#5` | `GLOB` | press: restart all channels from step 1 at the next step | `RESTRT` |
| `KEY_B10_D#5` | `MENU` | press: restart all channels from step 1 at the next step | `RESTRT` |
| `KEY_B11_F#5` | `PERF` | press: tap tempo (internal clock only; ignored on EXT) | `TAP` |
| `KEY_B11_F#5` | `STEP` | press: tap tempo (internal clock only; ignored on EXT) | `TAP` |
| `KEY_B11_F#5` | `STEP_HELD` | press: tap tempo (internal clock only; ignored on EXT) | `TAP` |
| `KEY_B11_F#5` | `SCL` | press: set the global KEY to F# (pitch class) | `KEY` |
| `KEY_B11_F#5` | `CHAN` | press: tap tempo (internal clock only; ignored on EXT) | `TAP` |
| `KEY_B11_F#5` | `GLOB` | press: tap tempo (internal clock only; ignored on EXT) | `TAP` |
| `KEY_B11_F#5` | `MENU` | press: tap tempo (internal clock only; ignored on EXT) | `TAP` |
<!-- END-APPENDIX-A -->
