> P1-017 (sequencer UX lock; for owner review): the contexts are now the nine **pages** of [SEQUENCER_UX.md](SEQUENCER_UX.md): `PERF`, `NOTE_EDIT`, `CHAN`, `SET`, `SCALE`, `REC`, `RESET`, `HELP`, `MENU` (441 rows = 49 controls x 9). `STEP_HELD` is gone (a held step opens the latched NOTE_EDIT page). Black keys: `D#4` = CHAN page, `C#5` / `D#5` = PG- / PG+ (scenes and tap tempo removed). `GLO` tap = SET (BPM SWING GATE DIV), hold = SCALE (SCALE KEY OCTAVE, any key = KEY). `HOME` tap = home (on PERF it only cancels a prompt), hold + touch a control = HELP, release after 0.7 s untouched = MENU (owner decisions 57 / 61; revision 1 reverted the first draft's HOME-tap HELP). CHAN `SOURCE` = the page a page reads (`P1`.. / `OWN`, revision 1). PRESETS turn = pending slot `Pnn?`, SAVE tap while pending = save there. `EDIT` hold = RESET (latched, one shot). Section 2 and Appendix A are regenerated; the per-mode, per-knob and per-key detail is in button_modes.tsv, knob_map.tsv and key_map.tsv. Sections 0-1 and 3-7 below are P1-014 history where they differ.

# Control map v1: every control, every context, one gesture

> P1-014 revision (printed-word pass, scheme B2; supersedes the layer-order layout of the note below): every button now does what its **printed word** says wherever this sequencer can do it. `FX` = DICE ALL; `SCL` = NOTES (scale / pitch), `ENV` = ACCNT (envelope accent), `LFO` = SHAPE (movement over the bar), `ARP` = ARTIC (repeats, slides, gate: the arpeggio-like layer), `SEQ` = HITS (the steps); `EDIT` = UNDO / RESET; `GLO` = the SET page (global feel); `REC` = RECORD (new `REC` context: play the keys, they write); `HOME` hold: HELP while held, MENU on release if nothing was touched. MUTATE is removed (dice + UNDO cover it, owner question 36 stays with the hardware test). White keys stay the steps; hold a step key and press another white key = that scale degree. Contexts are six (`REC` added, 294 rows = 49 x 6). Appendix A regenerated from control_map.tsv. Reasoning and scores: PERFORMANCE_DESIGN.md sections 0, 7, 8.

> P1-014 (performance-first, owner eighth round): the layout now follows ONE principle, see [PERFORMANCE_DESIGN.md](PERFORMANCE_DESIGN.md). Top button row (FX SCL ENV LFO EDIT GLO) = the six dice in layer order; bottom row (HOME SAVE ARP SEQ PLAY REC) = safety and system (HOME escape, SAVE, UNDO, SET, PLAY, MUTATE); the 16 white keys are always the 16 steps (no STEP mode, no live keys); the 11 black keys are performance moves (4 mutes, PAGE, SOLO, FILL, ROLL, SCENE A, SCENE B, RESTART); ninth-round requirements R1-R4 are reserved as interfaces (pattern LENGTH, PAGE, per-step properties, keybed slot) for P1-016. Contexts are now five: `PERF`, `STEP_HELD`, `SET` (replaces `SCL`, `CHAN`, `GLOB`), `RESET` (new, hold UNDO) and `MENU`; `STEP` is gone. Appendix A is regenerated from control_map.tsv (245 rows = 49 controls x 5 contexts). Everything not named here (sections 5, 7 reasoning) is unchanged from P1-013.

> P1-013: the knobs gain a second row (ROW 2: S1..S4, up to 8 morphs per algorithm). OCT- / OCT+ become the row buttons (tap latches ROW 1 / ROW 2, hold and turn is momentary). Design in [FULL_CONTROL.md](FULL_CONTROL.md).

> P2-009: the chord row of the CONTROL tab is removed (owner decision 39). CONTROL tab is 4 rows; 28 menu rows in total (SYSTEM_MAP section 7).

Task P2-001, amended by P1-013 and P1-014. Status: consolidation of the owner-approved P1-012 design (approved 2026-11-10, with P1-010; owner
decisions 31-36 in [OWNER_QUESTIONS.md](../project/OWNER_QUESTIONS.md)), re-organised by the performance-first pass
P1-014 (for owner review). The reasoning, ease scores and rejected alternatives are in
[PERFORMANCE_DESIGN.md](PERFORMANCE_DESIGN.md) (current) and [UX_REVIEW.md](UX_REVIEW.md) (history); the exact
machine-readable table is [control_map.tsv](control_map.tsv) (441 rows = 49 controls x 9 pages, P1-017). Appendix A below repeats every TSV row
verbatim, and `python3 tools/check_control_map.py` proves the two agree. If they ever differ, the TSV is the source
and this file is wrong.

Inputs: [VISION.md](../project/VISION.md), [ADR-0004](../project/decisions/ADR-0004-design-lock.md), UX_REVIEW.md,
[CONTROL_INVENTORY.md](CONTROL_INVENTORY.md) (what the firmware does today), [ALGORITHM_SET.md](ALGORITHM_SET.md)
(layers, morphs, dice). Panel photo: `docs/controls.jpg`. Screen pixel layout is P2-003, not here.

## 0. Fixed facts used below (P1-014 history: superseded by SEQUENCER_UX.md where they differ)

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

## 1. Modes, and how you always know which one you are in (P1-014 history: superseded by SEQUENCER_UX.md where they differ)

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

Summary in panel order (P1-017). Per-page detail is Appendix A; where the summary and Appendix A differ, Appendix A (= the TSV) wins.

| Control (printed) | Tap or press (PERF) | Hold (PERF) | Other pages | Screen label |
| --- | --- | --- | --- | --- |
| `MASTER` (pot) | analogue volume | n/a | every page | none |
| `EN_SELECT` (SELECT) | select channel CH1..CH4 | n/a | NOTE_EDIT: also closes it; REC: cursor to step 1 | `CH n` |
| `EN_ALGO` (ALGORITHM) | algorithm of the selected channel (edits kept) | n/a | MENU: tab | algorithm name |
| `EN_PRESET` (PRESETS) | preset slot, pending (`Pnn?`); playing: lands on the first bar starting >= 1.5 s after the last detent; stopped: after 1.5 s; SAVE tap while pending saves there | n/a | REC, RESET: inert; MENU: row | `Pnn`, `Pnn?`, `Pnn>` |
| `EN_K1` (KNOB 1) | live row column 1 (ACID DENS/OFFSET, EUCLID HITS/LEN, SNAKE DENS/OFFSET, STAGE DENS/OFFSET) | n/a | NOTE_EDIT `PITCH`; CHAN `LENGTH`; SET `BPM`; SCALE `SCALE`; REC `CURSOR`; RESET `RST` | morph label |
| `EN_K2` (KNOB 2) | column 2 (ACID SLIDE/PEDAL, EUCLID OFFSET/SPAN, SNAKE LEN/GLEN, STAGE TOTAL/RDIV) | n/a | NOTE_EDIT `GATE`; CHAN `MIDI`; SET `SWING`; SCALE `KEY`; REC `ACC` | morph label |
| `EN_K3` (KNOB 3) | column 3 (ACID JUMP/PUNCH, EUCLID RATCH/PUNCH, SNAKE RANGE/DYN, STAGE DIR/DYN) | n/a | NOTE_EDIT `VELO`; CHAN `SOURCE`; SET `GATE`; SCALE `OCTAVE`; REC `SLIDE` | morph label |
| `EN_K4` (KNOB 4) | column 4 (ACID ACCNT/DIR, EUCLID ACCNT/CURVE, SNAKE PATH/DIR, STAGE RATCH/RANGE) | n/a | NOTE_EDIT `RATCH`; CHAN `FOLLOW`; SET `DIV`; SCALE `-`; REC `OCT` | morph label |
| `B_FX` (FX) | dice ALL | none | NOTE_EDIT: dice this step; RESET: algorithm / step to default | `ALL` |
| `B_SCL` (SCL) | dice NOTES | none | NOTE_EDIT: dice this step's pitch | `NOTES`, `NOTE` |
| `B_ENV` (ENV) | dice ACCNT | none | NOTE_EDIT: accent on/off | `ACCNT`, `ACCENT` |
| `B_LFO` (LFO) | dice SHAPE | none | NOTE_EDIT: chance 100/50 % | `SHAPE`, `PROB` |
| `B_EDIT` (EDIT) | UNDO / REDO | RESET page (one shot) | REC: undo the last note | `UNDO`, `RESET` |
| `B_GLO` (GLO) | SET page | SCALE page | SET / SCALE: BACK | `SET`, `SCALE` |
| `B_HOME` (HOME) | back to PERF (on PERF: cancel a prompt) | touch a control = HELP; release after 0.7 s untouched = MENU | HELP exists only while held | `HOME`, `HLP/MN` |
| `B_SAVE` (SAVE) | save to the header slot, or to the pending slot | 0.7 s: undo the last load (state and slot; next step) | | `SAVE`, `UNDO L` |
| `B_ARP` (ARP) | dice ARTIC | none | NOTE_EDIT: slide on/off | `ARTIC`, `SLIDE` |
| `B_SEQ` (SEQ) | dice HITS | none | NOTE_EDIT: step on/off | `HITS`, `ON/OFF` |
| `B_PLAY` (PLAY/STOP) | start / stop | none | every page | `PLAY` |
| `B_REC` (REC) | open REC | none | REC: DONE; RESET: clear the viewed page | `REC` |
| `B_OCTDN` (OCT-) | latch ROW 1 | peek ROW 1 while turning | NOTE_EDIT / SCALE / REC: octave -1; CHAN: to the page boundary below | `ROW 1` |
| `B_OCTUP` (OCT+) | latch ROW 2 | peek ROW 2 while turning | NOTE_EDIT / SCALE / REC: octave +1; CHAN: to the page boundary above | `ROW 2` |
| White keys F3 G3 A3 B3 C4 D4 E4 F4 G4 A4 B4 C5 D5 E5 F5 G5 (`KEY_W01_F3` .. `KEY_W16_G5`) | step n of the viewed page ON/OFF; force F3..C5: hard tap = accent | 0.4 s: NOTE_EDIT on that step | SCALE: KEY; REC: notes; RESET: clear the step; MENU: inert | `STEP` |
| Black key F#3 G#3 A#3 C#4 (`KEY_B01..B04`) | MUTE CH1..CH4 | n/a | SCALE: KEY; REC: notes | `MUTE1`..`MUTE4` |
| Black key D#4 (`KEY_B05`) | CHAN page | n/a | CHAN: BACK; RESET: re-copy the page before | `CHAN` |
| Black key F#4 (`KEY_B06`) | SOLO while held | | | `SOLO` |
| Black key G#4 (`KEY_B07`) | FILL while held (pressure = depth) | | | `FILL` |
| Black key A#4 (`KEY_B08`) | ROLL while held (pressure = 2x..4x) | | | `ROLL` |
| Black key C#5 (`KEY_B09`) | PG-: previous page | | | `PG-` |
| Black key D#5 (`KEY_B10`) | PG+: next page; last page: a prompt (add a copy / show a hidden page / complete a partial page), PG+ again confirms | | | `PG+` |
| Black key F#5 (`KEY_B11`) | RESTRT: all channels to step 1 | | | `RESTRT` |

## 3. Keys: steps versus moves (P1-014 history: superseded by SEQUENCER_UX.md where they differ)

- **16 white keys (F3..G5, place 0..15) = the 16 steps, always (outside REC).** Press key n: step n is pinned (sounds at
  once; key n is step n of the **viewed page**: with a pattern longer than 16, `PAGE` (D#4) swaps pages; hold PAGE and press a key to end the pattern there; a new pin is the note generated there, or the root when silent). A touch under 30 ms is ignored (brush guard). Tap a pinned key to clear it. **Hold a key** = select that step: K1 `NOTE`, K2 `ACC`, K3 `SLIDE`, K4 `RATCH` (row 1) and, after tapping OCT+, `TIE MUTE PROB OCT` (row 2); **press another white key while holding = that scale degree** (key 1 = the root, key n = degree n-1; sounds at once; replaces the pitch; UNDO restores). K1 at 0 (`KEYS ONLY`) plays only pins.
- **REC = play a tune in.** Tap REC: all 27 keys play real pitches F3..G5 on the selected channel (preview and MIDI) and write each as a pin at the cursor (stopped: the cursor, which advances; playing: the step nearest the playhead). K1 CURSOR, K2 ACC, K3 SLIDE, K4 OCT. EDIT tap takes the last note back. Tap REC or HOME to leave. This is the fast path (17 gestures for 16 pitched steps) and the audition path (every key sounds).
- **11 black keys = performance moves** (outside REC). `F#3 G#3 A#3 C#4` (printed OP1..OP4) MUTE1..4; `D#4` PAGE; `F#4` SOLO; `G#4` FILL; `A#4` ROLL; `C#5` SCENE A; `D#5` SCENE B; `F#5` RESTART. (D#4 PAGE shows `PG 2/4`.) The legend of these labels is always on the screen
  (cue strip band). They behave the same in PERF and RESET, are inert in STEP_HELD (except that a held step key keeps its edit), inert in MENU, and set KEY in SET row 2.
- **Dice are never on keys.** They are buttons. The keys stay free of dice so a performer's hands stay split: left on the buttons, right on the keys.
- **Channel select is not on keys** (the SELECT encoder alone; four detents reach any channel). The four select keys and `ALL ON` of the approved map were given up for SOLO, FILL, ROLL and the scenes.

## 4. Reference workflow, gesture by gesture (TB-3 on MIDI channel 2) (P1-014 history: superseded by SEQUENCER_UX.md where they differ)

Starting from power-on. No step opens the MENU. Result: **performing within 5 gestures**, every one on the main screen except the SET page.

| # | Gesture | Controls | What the screen shows | Reaches |
| --- | --- | --- | --- | --- |
| 1 | Select channel 1 | turn SELECT to `1` | header cell `1` inverted; knob cells show channel 1's algorithm | W1 |
| 2 | Set MIDI channel 2 | tap GLO (SET page), tap OCT+ (row 2), turn K3 until `MIDI` reads `2`; tap HOME | SET page with both rows; header cell `1 ACID 2` after the escape | W2 |
| 3 | Choose an algorithm | turn ALGORITHM: `ACID > EUCLID > SNAKE > STAGE` (lands at the next step; its own morphs and seeds are restored) | algorithm name in large type | W3 |
| 4a | Dial a sequence in | press PLAY; turn K1 (density), K2 (form), K3 (pitch width), K4 (spice) | each knob cell highlights for 1 s and shows its value | W5, W6 |
| 4b | or enter steps | turn K1 to `KEYS ONLY`; tap white keys to pin steps; tap REC and play the pitches (or hold a step key and press another white key) | pins outlined on the lane; the pin sounds | W4 |
| 5 | Perform | morph with K1-K4, tap OCT+ for ROW 2; dice with FX (all) SCL (NOTES) SEQ (HITS) ENV (ACCNT) ARP (ARTIC) LFO (SHAPE); EDIT undoes; mute with F#3..C#4; hold G#4 to fill | every button shows its role; muted channel cell dimmed with `M` | W6, W7, W10 |
| 6 | Optional global changes | tap GLO: row 1 GATE, DIV, SWING, BPM; row 2 SCALE, OCTAVE, MIDI, LENGTH (superseded by HARMONY.md 5, P2-012: hold GLO = HARMONY, KEY is K2, OCTAVE is OCT-/OCT+ on SET); SAVE to keep it | header `C MINPEN` on every screen | W8, W9, W11, W12 |

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
| `B_FX` | `PERF` | tap: press: DICE ALL layers present in the algorithm (generated layer only; every step edit is kept) // hold: no hold role: acted once on press | `ALL` |
| `B_FX` | `NOTE_EDIT` | tap: press: DICE THIS STEP: new pitch, accent, slide and gate for the edited step from fresh entropy, written as edits and auditioned; EDIT tap undoes // hold: no hold role: acted once on press | `ALL` |
| `B_FX` | `CHAN` | tap: press: DICE ALL layers present in the algorithm (generated layer only; every step edit is kept) // hold: no hold role: acted once on press | `ALL` |
| `B_FX` | `SET` | tap: press: DICE ALL layers present in the algorithm (generated layer only; every step edit is kept) // hold: no hold role: acted once on press | `ALL` |
| `B_FX` | `SCALE` | tap: press: DICE ALL layers present in the algorithm (generated layer only; every step edit is kept) // hold: no hold role: acted once on press | `ALL` |
| `B_FX` | `REC` | tap: press: DICE ALL layers present in the algorithm (generated layer only; every step edit is kept) (performing while recording is allowed) // hold: no hold role: acted once on press | `ALL` |
| `B_FX` | `RESET` | tap: press: from PERF/CHAN/SET/SCALE: the whole algorithm to factory (8 morphs + every seed; edits kept); from NOTE_EDIT: every edit of the edited step to AUTO; RESET closes; EDIT tap undoes // hold: no hold role: acted once on press | `RST AL` |
| `B_FX` | `HELP` | tap: explain this control on the page beneath (help box + cue strip); nothing else happens // hold: as tap (explain while HOME stays down); nothing else happens | `?` |
| `B_FX` | `MENU` | tap: press: closes the menu (no other action) // hold: as tap (the menu has no hold roles; HOME hold 0.7 s also closes) | `close` |
| `B_SCL` | `PERF` | tap: press: dice the NOTES layer (new pitches in the scale; edited pitches kept); algorithm sub-label under it // hold: no hold role: acted once on press | `NOTES` |
| `B_SCL` | `NOTE_EDIT` | tap: press: dice the edited step's pitch: a new scale note, written as its PITCH edit and auditioned; EDIT tap undoes // hold: no hold role: acted once on press | `NOTE` |
| `B_SCL` | `CHAN` | tap: press: dice the NOTES layer (new pitches in the scale; edited pitches kept); algorithm sub-label under it // hold: no hold role: acted once on press | `NOTES` |
| `B_SCL` | `SET` | tap: press: dice the NOTES layer (new pitches in the scale; edited pitches kept); algorithm sub-label under it // hold: no hold role: acted once on press | `NOTES` |
| `B_SCL` | `SCALE` | tap: press: dice the PROGRESSION: new chord degrees, bars and chord count within the musical limits (HARMONY 6); KEY, SCALE, VOICE, MOVE, DRONE and the channel dice layers are kept; EDIT tap undoes // hold: no hold role: acted once on press | `PROG` |
| `B_SCL` | `REC` | tap: press: dice the NOTES layer (new pitches in the scale; edited pitches kept) (performing while recording is allowed) // hold: no hold role: acted once on press | `NOTES` |
| `B_SCL` | `RESET` | tap: press: from PERF/CHAN/SET: that layer's seeds to factory; from HARMONY: the progression back to 1--- (4 bars, one chord, degree 1); from NOTE_EDIT: that property of the edited step to AUTO (NOTE); RESET closes; EDIT tap undoes // hold: no hold role: acted once on press | `RST NT` |
| `B_SCL` | `HELP` | tap: explain this control on the page beneath (help box + cue strip); nothing else happens // hold: as tap (explain while HOME stays down); nothing else happens | `?` |
| `B_SCL` | `MENU` | tap: press: closes the menu (no other action) // hold: as tap (the menu has no hold roles; HOME hold 0.7 s also closes) | `close` |
| `B_ENV` | `PERF` | tap: press: dice the ACCNT layer (which generated steps are loud; VELO edits kept); algorithm sub-label under it // hold: no hold role: acted once on press | `ACCNT` |
| `B_ENV` | `NOTE_EDIT` | tap: press: toggle ACCENT on the edited step: velocity to ACC_VEL 120, or back to 80 / AUTO; EDIT tap undoes // hold: no hold role: acted once on press | `ACCENT` |
| `B_ENV` | `CHAN` | tap: press: dice the ACCNT layer (which generated steps are loud; VELO edits kept); algorithm sub-label under it // hold: no hold role: acted once on press | `ACCNT` |
| `B_ENV` | `SET` | tap: press: dice the ACCNT layer (which generated steps are loud; VELO edits kept); algorithm sub-label under it // hold: no hold role: acted once on press | `ACCNT` |
| `B_ENV` | `SCALE` | tap: press: dice the ACCNT layer (which generated steps are loud; VELO edits kept); algorithm sub-label under it // hold: no hold role: acted once on press | `ACCNT` |
| `B_ENV` | `REC` | tap: press: dice the ACCNT layer (which generated steps are loud; VELO edits kept) (performing while recording is allowed) // hold: no hold role: acted once on press | `ACCNT` |
| `B_ENV` | `RESET` | tap: press: from PERF/CHAN/SET/SCALE: that layer's seeds to factory; from NOTE_EDIT: that property of the edited step to AUTO (ACCENT); RESET closes; EDIT tap undoes // hold: no hold role: acted once on press | `RST AC` |
| `B_ENV` | `HELP` | tap: explain this control on the page beneath (help box + cue strip); nothing else happens // hold: as tap (explain while HOME stays down); nothing else happens | `?` |
| `B_ENV` | `MENU` | tap: press: closes the menu (no other action) // hold: as tap (the menu has no hold roles; HOME hold 0.7 s also closes) | `close` |
| `B_LFO` | `PERF` | tap: press: dice the SHAPE layer (register jumps, slot lengths, drift); dim if absent; algorithm sub-label under it // hold: no hold role: acted once on press | `SHAPE` |
| `B_LFO` | `NOTE_EDIT` | tap: press: toggle PROB on the edited step: 100 % / 50 % (PROB_LOW), drawn deterministically per pass; EDIT tap undoes // hold: no hold role: acted once on press | `PROB` |
| `B_LFO` | `CHAN` | tap: press: dice the SHAPE layer (register jumps, slot lengths, drift); dim if absent; algorithm sub-label under it // hold: no hold role: acted once on press | `SHAPE` |
| `B_LFO` | `SET` | tap: press: dice the SHAPE layer (register jumps, slot lengths, drift); dim if absent; algorithm sub-label under it // hold: no hold role: acted once on press | `SHAPE` |
| `B_LFO` | `SCALE` | tap: press: BARS, the phrase length: 1..8 bars, one step per press, wrapping after 8 to the chord count (never fewer bars than chords); heard at the next phrase start // hold: no hold role: acted once on press | `BARS` |
| `B_LFO` | `REC` | tap: press: dice the SHAPE layer (register jumps, slot lengths, drift); dim if absent (performing while recording is allowed) // hold: no hold role: acted once on press | `SHAPE` |
| `B_LFO` | `RESET` | tap: press: from PERF/CHAN/SET/SCALE: that layer's seeds to factory; from NOTE_EDIT: that property of the edited step to AUTO (PROB); RESET closes; EDIT tap undoes // hold: no hold role: acted once on press | `RST SH` |
| `B_LFO` | `HELP` | tap: explain this control on the page beneath (help box + cue strip); nothing else happens // hold: as tap (explain while HOME stays down); nothing else happens | `?` |
| `B_LFO` | `MENU` | tap: press: closes the menu (no other action) // hold: as tap (the menu has no hold roles; HOME hold 0.7 s also closes) | `close` |
| `B_EDIT` | `PERF` | tap: UNDO / REDO the last dice, dice-all, step edit, page add, page clear or reset of the selected channel, or the last HARMONY change when that came later (PROG dice, typed progression, BARS, VOICE, MOVE, DRONE) // hold: 0.4 s: open RESET (latched, one shot): the next control touched goes back to its default | `UNDO` |
| `B_EDIT` | `NOTE_EDIT` | tap: UNDO: the edited step back to how it was when this NOTE EDIT visit began; again = redo // hold: 0.4 s: open RESET over NOTE_EDIT (next touch puts that property of the step back to AUTO) | `UNDO` |
| `B_EDIT` | `CHAN` | tap: UNDO / REDO the last dice, dice-all, step edit, page add, page clear or reset of the selected channel, or the last HARMONY change when that came later (PROG dice, typed progression, BARS, VOICE, MOVE, DRONE) // hold: 0.4 s: open RESET (latched, one shot): the next control touched goes back to its default | `UNDO` |
| `B_EDIT` | `SET` | tap: UNDO / REDO the last dice, dice-all, step edit, page add, page clear or reset of the selected channel, or the last HARMONY change when that came later (PROG dice, typed progression, BARS, VOICE, MOVE, DRONE) // hold: 0.4 s: open RESET (latched, one shot): the next control touched goes back to its default | `UNDO` |
| `B_EDIT` | `SCALE` | tap: UNDO / REDO the last dice, dice-all, step edit, page add, page clear or reset of the selected channel, or the last HARMONY change when that came later (PROG dice, typed progression, BARS, VOICE, MOVE, DRONE) // hold: 0.4 s: open RESET (latched, one shot): the next control touched goes back to its default | `UNDO` |
| `B_EDIT` | `REC` | tap: UNDO the last written note (the cursor steps back); again = redo // hold: inert in REC (toast LEAVE REC FIRST) | `UNDO` |
| `B_EDIT` | `RESET` | tap: cancel RESET (nothing reset), back to the page beneath // hold: inert (already in RESET) | `CANCEL` |
| `B_EDIT` | `HELP` | tap: explain this control on the page beneath (help box + cue strip); nothing else happens // hold: as tap (explain while HOME stays down); nothing else happens | `?` |
| `B_EDIT` | `MENU` | tap: press: closes the menu (no other action) // hold: as tap (the menu has no hold roles; HOME hold 0.7 s also closes) | `close` |
| `B_GLO` | `PERF` | tap: open SET (latched): BPM SWING GATE DIV // hold: 0.4 s: open HARMONY (latched): SCALE KEY VOICE MOVE, the progression strip, keys F3-E4 = chords 1-7 | `SET` |
| `B_GLO` | `NOTE_EDIT` | tap: open SET (latched): BPM SWING GATE DIV (closes NOTE EDIT) // hold: 0.4 s: open HARMONY (latched): SCALE KEY VOICE MOVE, the progression strip, keys F3-E4 = chords 1-7 | `SET` |
| `B_GLO` | `CHAN` | tap: open SET (latched): BPM SWING GATE DIV // hold: 0.4 s: open HARMONY (latched): SCALE KEY VOICE MOVE, the progression strip, keys F3-E4 = chords 1-7 | `SET` |
| `B_GLO` | `SET` | tap: close SET, back to PERF // hold: 0.4 s: switch to HARMONY | `BACK` |
| `B_GLO` | `SCALE` | tap: close HARMONY, back to PERF // hold: inert (already on HARMONY) | `BACK` |
| `B_GLO` | `REC` | tap: inert in REC (toast REC: tap REC to finish) // hold: inert in REC | `-` |
| `B_GLO` | `RESET` | tap: inert (nothing to reset; toast NOTHING TO RESET) // hold: inert | `-` |
| `B_GLO` | `HELP` | tap: explain this control on the page beneath (help box + cue strip); nothing else happens // hold: as tap (explain while HOME stays down); nothing else happens | `?` |
| `B_GLO` | `MENU` | tap: press: closes the menu (no other action) // hold: as tap (the menu has no hold roles; HOME hold 0.7 s also closes) | `close` |
| `B_HOME` | `PERF` | tap: release under 0.7 s, nothing touched: already home, so it only cancels a pending prompt (ADD PAGE, a pending preset slot) or skips the first-run strip; toast HOME  hold: HELP / MENU; it never opens a page // hold: hold (acts on release; owner decisions 57 / 61): from 0.25 s the strip reads HOME: touch=HELP  let go=MENU; touch or turn any control while HOME is down = HELP for that control (it does not act); release after 0.7 s with nothing touched = MENU | `HOME` |
| `B_HOME` | `NOTE_EDIT` | tap: release under 0.7 s, nothing touched: back to PERF (leaves NOTE_EDIT) // hold: hold (acts on release; owner decisions 57 / 61): from 0.25 s the strip reads HOME: touch=HELP  let go=MENU; touch or turn any control while HOME is down = HELP for that control (it does not act); release after 0.7 s with nothing touched = MENU | `HOME` |
| `B_HOME` | `CHAN` | tap: release under 0.7 s, nothing touched: back to PERF (leaves CHAN) // hold: hold (acts on release; owner decisions 57 / 61): from 0.25 s the strip reads HOME: touch=HELP  let go=MENU; touch or turn any control while HOME is down = HELP for that control (it does not act); release after 0.7 s with nothing touched = MENU | `HOME` |
| `B_HOME` | `SET` | tap: release under 0.7 s, nothing touched: back to PERF (leaves SET) // hold: hold (acts on release; owner decisions 57 / 61): from 0.25 s the strip reads HOME: touch=HELP  let go=MENU; touch or turn any control while HOME is down = HELP for that control (it does not act); release after 0.7 s with nothing touched = MENU | `HOME` |
| `B_HOME` | `SCALE` | tap: release under 0.7 s, nothing touched: back to PERF (leaves SCALE) // hold: hold (acts on release; owner decisions 57 / 61): from 0.25 s the strip reads HOME: touch=HELP  let go=MENU; touch or turn any control while HOME is down = HELP for that control (it does not act); release after 0.7 s with nothing touched = MENU | `HOME` |
| `B_HOME` | `REC` | tap: release under 0.7 s, nothing touched: back to PERF (leaves REC) // hold: hold (acts on release; owner decisions 57 / 61): from 0.25 s the strip reads HOME: touch=HELP  let go=MENU; touch or turn any control while HOME is down = HELP for that control (it does not act); release after 0.7 s with nothing touched = MENU | `HOME` |
| `B_HOME` | `RESET` | tap: cancel RESET and go to PERF // hold: hold (acts on release; owner decisions 57 / 61): from 0.25 s the strip reads HOME: touch=HELP  let go=MENU; touch or turn any control while HOME is down = HELP for that control (it does not act); release after 0.7 s with nothing touched = MENU | `HOME` |
| `B_HOME` | `HELP` | tap: release HOME (a control was touched): HELP closes, back to the page HOME was held on; no MENU // hold: keep HOME down: touch or turn more controls to explain them | `BACK` |
| `B_HOME` | `MENU` | tap: tap or hold 0.7 s: close the menu // hold: as tap (the menu has no hold roles; HOME hold 0.7 s also closes) | `BACK` |
| `B_SAVE` | `PERF` | tap: release under 0.7 s: save the whole device state to the slot in the header; while a slot is pending (header Pnn?), save to the PENDING slot instead and cancel its load (the header becomes Pnn) // hold: 0.7 s: undo the last preset load (the state AND the header slot go back), applied at the next step; again = redo; while a slot is pending: cancel it instead | `SAVE` |
| `B_SAVE` | `NOTE_EDIT` | tap: release under 0.7 s: save the whole device state to the slot in the header; while a slot is pending (header Pnn?), save to the PENDING slot instead and cancel its load (the header becomes Pnn) // hold: 0.7 s: undo the last preset load (the state AND the header slot go back), applied at the next step; again = redo; while a slot is pending: cancel it instead | `SAVE` |
| `B_SAVE` | `CHAN` | tap: release under 0.7 s: save the whole device state to the slot in the header; while a slot is pending (header Pnn?), save to the PENDING slot instead and cancel its load (the header becomes Pnn) // hold: 0.7 s: undo the last preset load (the state AND the header slot go back), applied at the next step; again = redo; while a slot is pending: cancel it instead | `SAVE` |
| `B_SAVE` | `SET` | tap: release under 0.7 s: save the whole device state to the slot in the header; while a slot is pending (header Pnn?), save to the PENDING slot instead and cancel its load (the header becomes Pnn) // hold: 0.7 s: undo the last preset load (the state AND the header slot go back), applied at the next step; again = redo; while a slot is pending: cancel it instead | `SAVE` |
| `B_SAVE` | `SCALE` | tap: release under 0.7 s: save the whole device state to the slot in the header; while a slot is pending (header Pnn?), save to the PENDING slot instead and cancel its load (the header becomes Pnn) // hold: 0.7 s: undo the last preset load (the state AND the header slot go back), applied at the next step; again = redo; while a slot is pending: cancel it instead | `SAVE` |
| `B_SAVE` | `REC` | tap: release under 0.7 s: save the whole device state to the slot in the header; while a slot is pending (header Pnn?), save to the PENDING slot instead and cancel its load (the header becomes Pnn) // hold: 0.7 s: undo the last preset load (the state AND the header slot go back), applied at the next step; again = redo; while a slot is pending: cancel it instead | `SAVE` |
| `B_SAVE` | `RESET` | tap: inert (toast NOTHING TO RESET) // hold: inert | `-` |
| `B_SAVE` | `HELP` | tap: explain this control on the page beneath (help box + cue strip); nothing else happens // hold: as tap (explain while HOME stays down); nothing else happens | `?` |
| `B_SAVE` | `MENU` | tap: press: closes the menu (no other action) // hold: as tap (the menu has no hold roles; HOME hold 0.7 s also closes) | `close` |
| `B_ARP` | `PERF` | tap: press: dice the ARTIC layer (gate, slide, ratchet as the algorithm has them; edits kept); dim if absent; algorithm sub-label under it // hold: no hold role: acted once on press | `ARTIC` |
| `B_ARP` | `NOTE_EDIT` | tap: press: toggle SLIDE on the edited step: this note glides into the next step (step 1 after the last step; TB-303 rule; no glide if that step is silent); EDIT tap undoes // hold: no hold role: acted once on press | `SLIDE` |
| `B_ARP` | `CHAN` | tap: press: dice the ARTIC layer (gate, slide, ratchet as the algorithm has them; edits kept); dim if absent; algorithm sub-label under it // hold: no hold role: acted once on press | `ARTIC` |
| `B_ARP` | `SET` | tap: press: dice the ARTIC layer (gate, slide, ratchet as the algorithm has them; edits kept); dim if absent; algorithm sub-label under it // hold: no hold role: acted once on press | `ARTIC` |
| `B_ARP` | `SCALE` | tap: press: ROLE of the selected channel, wrapping OFF, AUTO, ROOT, FULL (AUTO follows the chords by the quantise rule, ROOT plays the chord root, FULL plays the voiced chord notes, OFF is never harmonised) // hold: no hold role: acted once on press | `ROLE` |
| `B_ARP` | `REC` | tap: press: dice the ARTIC layer (gate, slide, ratchet as the algorithm has them; edits kept); dim if absent (performing while recording is allowed) // hold: no hold role: acted once on press | `ARTIC` |
| `B_ARP` | `RESET` | tap: press: from PERF/CHAN/SET/SCALE: that layer's seeds to factory; from NOTE_EDIT: that property of the edited step to AUTO (SLIDE); RESET closes; EDIT tap undoes // hold: no hold role: acted once on press | `RST AR` |
| `B_ARP` | `HELP` | tap: explain this control on the page beneath (help box + cue strip); nothing else happens // hold: as tap (explain while HOME stays down); nothing else happens | `?` |
| `B_ARP` | `MENU` | tap: press: closes the menu (no other action) // hold: as tap (the menu has no hold roles; HOME hold 0.7 s also closes) | `close` |
| `B_SEQ` | `PERF` | tap: press: dice the HITS layer (where the generated steps fall; ON/OFF edits kept); algorithm sub-label under it // hold: no hold role: acted once on press | `HITS` |
| `B_SEQ` | `NOTE_EDIT` | tap: press: toggle the edited step ON/OFF (same as tapping its key); EDIT tap undoes // hold: no hold role: acted once on press | `ON/OFF` |
| `B_SEQ` | `CHAN` | tap: press: dice the HITS layer (where the generated steps fall; ON/OFF edits kept); algorithm sub-label under it // hold: no hold role: acted once on press | `HITS` |
| `B_SEQ` | `SET` | tap: press: dice the HITS layer (where the generated steps fall; ON/OFF edits kept); algorithm sub-label under it // hold: no hold role: acted once on press | `HITS` |
| `B_SEQ` | `SCALE` | tap: press: NEW progression: the next degree key starts again at chord 1 (nothing changes until a key is typed) // hold: no hold role: acted once on press | `NEW` |
| `B_SEQ` | `REC` | tap: press: dice the HITS layer (where the generated steps fall; ON/OFF edits kept) (performing while recording is allowed) // hold: no hold role: acted once on press | `HITS` |
| `B_SEQ` | `RESET` | tap: press: from PERF/CHAN/SET/SCALE: that layer's seeds to factory; from NOTE_EDIT: that property of the edited step to AUTO (ON/OFF); RESET closes; EDIT tap undoes // hold: no hold role: acted once on press | `RST HT` |
| `B_SEQ` | `HELP` | tap: explain this control on the page beneath (help box + cue strip); nothing else happens // hold: as tap (explain while HOME stays down); nothing else happens | `?` |
| `B_SEQ` | `MENU` | tap: press: closes the menu (no other action) // hold: as tap (the menu has no hold roles; HOME hold 0.7 s also closes) | `close` |
| `B_PLAY` | `PERF` | tap: press: start / stop all channels (stop sends every note-off then CC123) // hold: no hold role: acted once on press | `PLAY` |
| `B_PLAY` | `NOTE_EDIT` | tap: press: start / stop all channels (stop sends every note-off then CC123) // hold: no hold role: acted once on press | `PLAY` |
| `B_PLAY` | `CHAN` | tap: press: start / stop all channels (stop sends every note-off then CC123) // hold: no hold role: acted once on press | `PLAY` |
| `B_PLAY` | `SET` | tap: press: start / stop all channels (stop sends every note-off then CC123) // hold: no hold role: acted once on press | `PLAY` |
| `B_PLAY` | `SCALE` | tap: press: start / stop all channels (stop sends every note-off then CC123) // hold: no hold role: acted once on press | `PLAY` |
| `B_PLAY` | `REC` | tap: press: start / stop all channels (stop sends every note-off then CC123) // hold: no hold role: acted once on press | `PLAY` |
| `B_PLAY` | `RESET` | tap: press: start / stop all channels (stop sends every note-off then CC123) // hold: no hold role: acted once on press | `PLAY` |
| `B_PLAY` | `HELP` | tap: press: start / stop (still works in HELP) and explain it // hold: no hold role: acted once on press | `PLAY` |
| `B_PLAY` | `MENU` | tap: press: start / stop all channels // hold: as tap (the menu has no hold roles; HOME hold 0.7 s also closes) | `PLAY` |
| `B_REC` | `PERF` | tap: press: open REC (latched, LED lit): the 27 keys play and write notes // hold: no hold role: acted once on press | `REC` |
| `B_REC` | `NOTE_EDIT` | tap: press: open REC with the write cursor on the edited step // hold: no hold role: acted once on press | `REC` |
| `B_REC` | `CHAN` | tap: press: open REC (latched, LED lit): the 27 keys play and write notes // hold: no hold role: acted once on press | `REC` |
| `B_REC` | `SET` | tap: press: open REC (latched, LED lit): the 27 keys play and write notes // hold: no hold role: acted once on press | `REC` |
| `B_REC` | `SCALE` | tap: press: open REC (latched, LED lit): the 27 keys play and write notes // hold: no hold role: acted once on press | `REC` |
| `B_REC` | `REC` | tap: press: leave REC, back to PERF // hold: no hold role: acted once on press | `DONE` |
| `B_REC` | `RESET` | tap: press: clear every edit on the viewed page of the selected channel (it becomes pure generator); RESET closes; EDIT tap undoes // hold: no hold role: acted once on press | `RST PG` |
| `B_REC` | `HELP` | tap: explain this control on the page beneath (help box + cue strip); nothing else happens // hold: as tap (explain while HOME stays down); nothing else happens | `?` |
| `B_REC` | `MENU` | tap: press: closes the menu (no other action) // hold: as tap (the menu has no hold roles; HOME hold 0.7 s also closes) | `close` |
| `B_OCTDN` | `PERF` | tap: release: latch ROW 1 of the knob band // hold: hold and turn a knob: ROW 1 live while held (momentary peek; the tap is the main route; sub-label h:peek; unscored) | `ROW 1` |
| `B_OCTDN` | `NOTE_EDIT` | tap: release: the edited step one octave down (OCT -2..+2), auditioned // hold: as tap (no hold role on this page) | `OCT-` |
| `B_OCTDN` | `CHAN` | tap: release: LENGTH to the page boundary below (20 -> 16, 48 -> 32; at 16 or less inert, toast ONE PAGE: K1 = SHORTER); the hidden steps keep their edits // hold: as tap (no hold role on this page) | `-PAGE` |
| `B_OCTDN` | `SET` | tap: release: global OCTAVE -1 (-2..+2); the tag column reads OCT -2..+2 (moved here from the SCALE page, which uses OCT- / OCT+ for DRONE) // hold: as tap (no hold role on this page) | `OCT-` |
| `B_OCTDN` | `SCALE` | tap: release: DRONE walks one candidate down (0 = OFF); the status line names the degree and marks clashes // hold: as tap (no hold role on this page) | `OCT-` |
| `B_OCTDN` | `REC` | tap: release: the REC keyboard octave -1 (same value as K4 OCT) // hold: as tap (no hold role on this page) | `OCT-` |
| `B_OCTDN` | `RESET` | tap: from PERF: reset the four morphs of ROW 1; from NOTE_EDIT: the step octave to AUTO; from HARMONY: DRONE OFF; elsewhere inert; RESET closes; EDIT tap undoes // hold: as tap | `RST R1` |
| `B_OCTDN` | `HELP` | tap: explain this control on the page beneath (help box + cue strip); nothing else happens // hold: as tap (explain while HOME stays down); nothing else happens | `?` |
| `B_OCTDN` | `MENU` | tap: release: previous value / back (Felucca menu rule) // hold: as tap (the menu has no hold roles; HOME hold 0.7 s also closes) | `-` |
| `B_OCTUP` | `PERF` | tap: release: latch ROW 2 of the knob band // hold: hold and turn a knob: ROW 2 live while held (momentary peek; the tap is the main route; sub-label h:peek; unscored) | `ROW 2` |
| `B_OCTUP` | `NOTE_EDIT` | tap: release: the edited step one octave up (OCT -2..+2), auditioned // hold: as tap (no hold role on this page) | `OCT+` |
| `B_OCTUP` | `CHAN` | tap: release: LENGTH to the page boundary above (20 -> 32, 32 -> 48, max 64): a partial page is completed, a new page is a copy of the page before it (edits and source), a hidden page comes back as stored // hold: as tap (no hold role on this page) | `+PAGE` |
| `B_OCTUP` | `SET` | tap: release: global OCTAVE +1 (-2..+2); the tag column reads OCT -2..+2 (moved here from the SCALE page, which uses OCT- / OCT+ for DRONE) // hold: as tap (no hold role on this page) | `OCT+` |
| `B_OCTUP` | `SCALE` | tap: release: DRONE walks one candidate up (14 = top); the status line names the degree and marks clashes // hold: as tap (no hold role on this page) | `OCT+` |
| `B_OCTUP` | `REC` | tap: release: the REC keyboard octave +1 (same value as K4 OCT) // hold: as tap (no hold role on this page) | `OCT+` |
| `B_OCTUP` | `RESET` | tap: from PERF: reset the four morphs of ROW 2; from NOTE_EDIT: the step octave to AUTO; from HARMONY: DRONE OFF; elsewhere inert; RESET closes; EDIT tap undoes // hold: as tap | `RST R2` |
| `B_OCTUP` | `HELP` | tap: explain this control on the page beneath (help box + cue strip); nothing else happens // hold: as tap (explain while HOME stays down); nothing else happens | `?` |
| `B_OCTUP` | `MENU` | tap: release: next value / do it (opens CALIBRATION, INFO, ABOUT rows) // hold: as tap (the menu has no hold roles; HOME hold 0.7 s also closes) | `-` |
| `EN_SELECT` | `PERF` | turn: select channel CH1..CH4 (clamped, no wrap) | `CH n` |
| `EN_SELECT` | `NOTE_EDIT` | turn: select channel CH1..CH4; closes NOTE EDIT (the edited step belongs to the old channel) | `CH n` |
| `EN_SELECT` | `CHAN` | turn: select channel CH1..CH4 (clamped, no wrap) | `CH n` |
| `EN_SELECT` | `SET` | turn: select channel CH1..CH4 (clamped, no wrap) | `CH n` |
| `EN_SELECT` | `SCALE` | turn: select channel CH1..CH4 (clamped, no wrap) | `CH n` |
| `EN_SELECT` | `REC` | turn: select channel; the cursor goes to step 1 | `CH n` |
| `EN_SELECT` | `RESET` | turn: select channel CH1..CH4 (clamped, no wrap) | `CH n` |
| `EN_SELECT` | `HELP` | turn: explain SELECT | `?` |
| `EN_SELECT` | `MENU` | turn: select channel CH1..CH4 | `CH n` |
| `EN_ALGO` | `PERF` | turn: algorithm of the selected channel ACID EUCLID SNAKE STAGE ARP PATTERNS; lands at the next step; morphs and seeds kept per algorithm (S7); edits kept | `ALGO` |
| `EN_ALGO` | `NOTE_EDIT` | turn: algorithm of the selected channel ACID EUCLID SNAKE STAGE ARP PATTERNS; lands at the next step; morphs and seeds kept per algorithm (S7); edits kept | `ALGO` |
| `EN_ALGO` | `CHAN` | turn: algorithm of the selected channel ACID EUCLID SNAKE STAGE ARP PATTERNS; lands at the next step; morphs and seeds kept per algorithm (S7); edits kept | `ALGO` |
| `EN_ALGO` | `SET` | turn: algorithm of the selected channel ACID EUCLID SNAKE STAGE ARP PATTERNS; lands at the next step; morphs and seeds kept per algorithm (S7); edits kept | `ALGO` |
| `EN_ALGO` | `SCALE` | turn: algorithm of the selected channel ACID EUCLID SNAKE STAGE ARP PATTERNS; lands at the next step; morphs and seeds kept per algorithm (S7); edits kept | `ALGO` |
| `EN_ALGO` | `REC` | turn: algorithm of the selected channel ACID EUCLID SNAKE STAGE ARP PATTERNS; lands at the next step; morphs and seeds kept per algorithm (S7); edits kept | `ALGO` |
| `EN_ALGO` | `RESET` | turn: inert (toast NOTHING TO RESET) | `-` |
| `EN_ALGO` | `HELP` | turn: explain ALGORITHM | `?` |
| `EN_ALGO` | `MENU` | turn: next / previous menu tab | `tab` |
| `EN_PRESET` | `PERF` | turn: choose a preset slot P01..P16; it becomes PENDING (header Pnn?, strip Pnn: SAVE = save here); stopped: it loads 1.5 s after the last detent (SQ_LOAD_SETTLE); playing: on the first 768-tick bar that starts at least SQ_LOAD_SETTLE 1.5 s after the last detent (header Pnn? then Pnn>), so a SAVE tap always has at least 1.5 s; SAVE tap while pending saves there instead (no load); turning back to the current slot cancels | `Pnn` |
| `EN_PRESET` | `NOTE_EDIT` | turn: choose a preset slot P01..P16; it becomes PENDING (header Pnn?, strip Pnn: SAVE = save here); stopped: it loads 1.5 s after the last detent (SQ_LOAD_SETTLE); playing: on the first 768-tick bar that starts at least SQ_LOAD_SETTLE 1.5 s after the last detent (header Pnn? then Pnn>), so a SAVE tap always has at least 1.5 s; SAVE tap while pending saves there instead (no load); turning back to the current slot cancels | `Pnn` |
| `EN_PRESET` | `CHAN` | turn: choose a preset slot P01..P16; it becomes PENDING (header Pnn?, strip Pnn: SAVE = save here); stopped: it loads 1.5 s after the last detent (SQ_LOAD_SETTLE); playing: on the first 768-tick bar that starts at least SQ_LOAD_SETTLE 1.5 s after the last detent (header Pnn? then Pnn>), so a SAVE tap always has at least 1.5 s; SAVE tap while pending saves there instead (no load); turning back to the current slot cancels | `Pnn` |
| `EN_PRESET` | `SET` | turn: choose a preset slot P01..P16; it becomes PENDING (header Pnn?, strip Pnn: SAVE = save here); stopped: it loads 1.5 s after the last detent (SQ_LOAD_SETTLE); playing: on the first 768-tick bar that starts at least SQ_LOAD_SETTLE 1.5 s after the last detent (header Pnn? then Pnn>), so a SAVE tap always has at least 1.5 s; SAVE tap while pending saves there instead (no load); turning back to the current slot cancels | `Pnn` |
| `EN_PRESET` | `SCALE` | turn: choose a preset slot P01..P16; it becomes PENDING (header Pnn?, strip Pnn: SAVE = save here); stopped: it loads 1.5 s after the last detent (SQ_LOAD_SETTLE); playing: on the first 768-tick bar that starts at least SQ_LOAD_SETTLE 1.5 s after the last detent (header Pnn? then Pnn>), so a SAVE tap always has at least 1.5 s; SAVE tap while pending saves there instead (no load); turning back to the current slot cancels | `Pnn` |
| `EN_PRESET` | `REC` | turn: inert in REC (toast REC: tap REC to finish) | `-` |
| `EN_PRESET` | `RESET` | turn: inert (toast NOTHING TO RESET) | `-` |
| `EN_PRESET` | `HELP` | turn: explain PRESETS | `?` |
| `EN_PRESET` | `MENU` | turn: next / previous menu row | `row` |
| `EN_K1` | `PERF` | turn: the live row (ROW 1 or ROW 2) morph in this column of the selected channel's algorithm (rows above): ACID DENS/OFFSET, EUCLID HITS/LEN, SNAKE DENS/OFFSET, STAGE DENS/OFFSET | `morph` |
| `EN_K1` | `NOTE_EDIT` | turn: one scale degree per detent from the current note; every detent sounds the note (MIDI + preview); the first detent turns AUTO into a set pitch | `PITCH` |
| `EN_K1` | `CHAN` | turn: pattern length of the selected channel; crossing into a new page fills it (copy of the page before, or the stored page); shrinking hides, never deletes | `LENGTH` |
| `EN_K1` | `SET` | turn: tempo (internal clock; ignored while an external MIDI clock runs, value shows EXT) | `BPM` |
| `EN_K1` | `SCALE` | turn: global scale (every algorithm and every edited pitch snaps to it) | `SCALE` |
| `EN_K1` | `REC` | turn: move the write cursor one step per detent (stopped: the step sounds) | `CURSOR` |
| `EN_K1` | `RESET` | turn (any detent): that value of the page beneath back to its default (PERF: that morph of the live row; NOTE_EDIT: that property of the step to AUTO; CHAN: LENGTH 16 / MIDI default / SOURCE P1 (reads page 1) / FOLLOW ON; SET / SCALE: power-on default); RESET closes; EDIT tap undoes | `RST` |
| `EN_K1` | `HELP` | turn (HOME held): explain this knob on the page beneath (PERF: both rows); nothing changes | `?` |
| `EN_K1` | `MENU` | turn: change the selected row's value (Felucca menu rule) | `value` |
| `EN_K2` | `PERF` | turn: the live row (ROW 1 or ROW 2) morph in this column of the selected channel's algorithm (rows above): ACID SLIDE/PEDAL, EUCLID OFFSET/SPAN, SNAKE LEN/GLEN, STAGE TOTAL/RDIV | `morph` |
| `EN_K2` | `NOTE_EDIT` | turn: note length of the edited step, x global GATE; top value TIE (holds into the next step) | `GATE` |
| `EN_K2` | `CHAN` | turn: MIDI output channel of the selected channel (unique across the 4 channels: taking one another channel uses sets that channel to OFF; toast names it; one undo record) | `MIDI` |
| `EN_K2` | `SET` | turn: odd steps late by 2*(swing-50) % of a step (decision 43); parity by pattern position | `SWING` |
| `EN_K2` | `SCALE` | turn: global key: the scale and every chord move with it (also sounds the new root) | `KEY` |
| `EN_K2` | `REC` | turn: written notes get ACCENT (velocity 120) when ON; with force sensing the landing force picks the level (SOFT 64 / MED 90 / HARD 120) and ACC forces 120 | `ACC` |
| `EN_K2` | `RESET` | turn (any detent): that value of the page beneath back to its default (PERF: that morph of the live row; NOTE_EDIT: that property of the step to AUTO; CHAN: LENGTH 16 / MIDI default / SOURCE P1 (reads page 1) / FOLLOW ON; SET / SCALE: power-on default); RESET closes; EDIT tap undoes | `RST` |
| `EN_K2` | `HELP` | turn (HOME held): explain this knob on the page beneath (PERF: both rows); nothing changes | `?` |
| `EN_K2` | `MENU` | turn: change the selected row's value (Felucca menu rule) | `value` |
| `EN_K3` | `PERF` | turn: the live row (ROW 1 or ROW 2) morph in this column of the selected channel's algorithm (rows above): ACID JUMP/PUNCH, EUCLID RATCH/PUNCH, SNAKE RANGE/DYN, STAGE DIR/DYN | `morph` |
| `EN_K3` | `NOTE_EDIT` | turn: velocity of the edited step; value reads ACC at >= ACC_THR 100 (TB-3 accent) | `VELO` |
| `EN_K3` | `CHAN` | turn: generator source of the VIEWED page k: offers only the blocks the pages before it play now, each labelled Pm by the first page m that plays it (so Pm = the notes page m plays), then OWN = its own positions (the algorithm continues past it); a block no earlier page plays any more stays as the current value shown by its positions (17-32, dim) until turned away; page 1 is always OWN (cell dim); a new page inherits the source of the page before, so a copy is exact | `SOURCE` |
| `EN_K3` | `SET` | turn: global gate, x the step gate (50 % neutral = x1.0, decision 44) | `GATE` |
| `EN_K3` | `SCALE` | turn: voicing of every chord: 0 root only, 1 root and fifth, 2 plain triad (neutral), 3 seventh, 4 ninth, 5 open ninth, 6 wide; changes the chord tones every AUTO channel is pulled to and what FULL channels play | `VOICE` |
| `EN_K3` | `REC` | turn: written notes get SLIDE when ON | `SLIDE` |
| `EN_K3` | `RESET` | turn (any detent): that value of the page beneath back to its default (PERF: that morph of the live row; NOTE_EDIT: that property of the step to AUTO; CHAN: LENGTH 16 / MIDI default / SOURCE P1 (reads page 1) / FOLLOW ON; SET / SCALE: power-on default); RESET closes; EDIT tap undoes | `RST` |
| `EN_K3` | `HELP` | turn (HOME held): explain this knob on the page beneath (PERF: both rows); nothing changes | `?` |
| `EN_K3` | `MENU` | turn: change the selected row's value (Felucca menu rule) | `value` |
| `EN_K4` | `PERF` | turn: the live row (ROW 1 or ROW 2) morph in this column of the selected channel's algorithm (rows above): ACID ACCNT/DIR, EUCLID ACCNT/CURVE, SNAKE PATH/DIR, STAGE RATCH/RANGE | `morph` |
| `EN_K4` | `NOTE_EDIT` | turn: repeats inside the step (SP5 spacing) | `RATCH` |
| `EN_K4` | `CHAN` | turn: the lane view follows the playhead page while playing | `FOLLOW` |
| `EN_K4` | `SET` | turn: step length | `DIV` |
| `EN_K4` | `SCALE` | turn: voice-leading distance between chords: 8 = every chord in root position (neutral), 0 = common tones kept and the smallest step per voice; changes FULL channels only | `MOVE` |
| `EN_K4` | `REC` | turn: REC keyboard octave (also OCT- / OCT+) | `OCT` |
| `EN_K4` | `RESET` | turn (any detent): that value of the page beneath back to its default (PERF: that morph of the live row; NOTE_EDIT: that property of the step to AUTO; CHAN: LENGTH 16 / MIDI default / SOURCE P1 (reads page 1) / FOLLOW ON; SET / SCALE: power-on default); RESET closes; EDIT tap undoes | `RST` |
| `EN_K4` | `HELP` | turn (HOME held): explain this knob on the page beneath (PERF: both rows); nothing changes | `?` |
| `EN_K4` | `MENU` | turn: change the selected row's value (Felucca menu rule) | `value` |
| `MASTER` | `PERF` | analogue pot: preview-synth and USB-audio volume; no MIDI effect (with force sensing the scan ISR samples it, master.c reads the cached value) | `-` |
| `MASTER` | `NOTE_EDIT` | analogue pot: preview-synth and USB-audio volume; no MIDI effect (with force sensing the scan ISR samples it, master.c reads the cached value) | `-` |
| `MASTER` | `CHAN` | analogue pot: preview-synth and USB-audio volume; no MIDI effect (with force sensing the scan ISR samples it, master.c reads the cached value) | `-` |
| `MASTER` | `SET` | analogue pot: preview-synth and USB-audio volume; no MIDI effect (with force sensing the scan ISR samples it, master.c reads the cached value) | `-` |
| `MASTER` | `SCALE` | analogue pot: preview-synth and USB-audio volume; no MIDI effect (with force sensing the scan ISR samples it, master.c reads the cached value) | `-` |
| `MASTER` | `REC` | analogue pot: preview-synth and USB-audio volume; no MIDI effect (with force sensing the scan ISR samples it, master.c reads the cached value) | `-` |
| `MASTER` | `RESET` | analogue pot: preview-synth and USB-audio volume; no MIDI effect (with force sensing the scan ISR samples it, master.c reads the cached value) | `-` |
| `MASTER` | `HELP` | analogue pot: preview-synth and USB-audio volume; no MIDI effect (with force sensing the scan ISR samples it, master.c reads the cached value) | `-` |
| `MASTER` | `MENU` | analogue pot: preview-synth and USB-audio volume; no MIDI effect (with force sensing the scan ISR samples it, master.c reads the cached value) | `-` |
| `KEY_W01_F3` | `PERF` | tap: release under 0.4 s: step 1 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: open NOTE EDIT on step 1 of the viewed page | `STEP` |
| `KEY_W01_F3` | `NOTE_EDIT` | tap: release under 0.4 s: step 1 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: move NOTE EDIT to step 1 (no new page) | `STEP` |
| `KEY_W01_F3` | `CHAN` | tap: release under 0.4 s: step 1 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: open NOTE EDIT on step 1 of the viewed page | `STEP` |
| `KEY_W01_F3` | `SET` | tap: release under 0.4 s: step 1 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: open NOTE EDIT on step 1 of the viewed page | `STEP` |
| `KEY_W01_F3` | `SCALE` | tap: type chord 1: writes degree 1 at the cursor of the progression (the first key after opening or after NEW starts at chord 1; up to 8 chords; BARS grows to hold them) and sounds the chord on the selected channel's MIDI channel; EDIT tap undoes | `DEG` |
| `KEY_W01_F3` | `REC` | tap: play F3 (+ REC OCT and global OCTAVE) on the selected channel and write it at the cursor / nearest step; release = note off; force: landing force picks the written level SOFT 64 / MED 90 / HARD 120 (HARD = accent); fallback: velocity 90, or 120 with K2 ACC ON | `NOTE` |
| `KEY_W01_F3` | `RESET` | tap: clear every edit of step 1 of the viewed page (back to the generator); RESET closes; EDIT tap undoes | `CLR` |
| `KEY_W01_F3` | `HELP` | tap/hold: explain this key on the page beneath (HOME held); nothing happens | `?` |
| `KEY_W01_F3` | `MENU` | tap/hold: inert while the menu is open | `-` |
| `KEY_W02_G3` | `PERF` | tap: release under 0.4 s: step 2 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: open NOTE EDIT on step 2 of the viewed page | `STEP` |
| `KEY_W02_G3` | `NOTE_EDIT` | tap: release under 0.4 s: step 2 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: move NOTE EDIT to step 2 (no new page) | `STEP` |
| `KEY_W02_G3` | `CHAN` | tap: release under 0.4 s: step 2 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: open NOTE EDIT on step 2 of the viewed page | `STEP` |
| `KEY_W02_G3` | `SET` | tap: release under 0.4 s: step 2 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: open NOTE EDIT on step 2 of the viewed page | `STEP` |
| `KEY_W02_G3` | `SCALE` | tap: type chord 2: writes degree 2 at the cursor of the progression (the first key after opening or after NEW starts at chord 1; up to 8 chords; BARS grows to hold them) and sounds the chord on the selected channel's MIDI channel; EDIT tap undoes | `DEG` |
| `KEY_W02_G3` | `REC` | tap: play G3 (+ REC OCT and global OCTAVE) on the selected channel and write it at the cursor / nearest step; release = note off; force: landing force picks the written level SOFT 64 / MED 90 / HARD 120 (HARD = accent); fallback: velocity 90, or 120 with K2 ACC ON | `NOTE` |
| `KEY_W02_G3` | `RESET` | tap: clear every edit of step 2 of the viewed page (back to the generator); RESET closes; EDIT tap undoes | `CLR` |
| `KEY_W02_G3` | `HELP` | tap/hold: explain this key on the page beneath (HOME held); nothing happens | `?` |
| `KEY_W02_G3` | `MENU` | tap/hold: inert while the menu is open | `-` |
| `KEY_W03_A3` | `PERF` | tap: release under 0.4 s: step 3 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: open NOTE EDIT on step 3 of the viewed page | `STEP` |
| `KEY_W03_A3` | `NOTE_EDIT` | tap: release under 0.4 s: step 3 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: move NOTE EDIT to step 3 (no new page) | `STEP` |
| `KEY_W03_A3` | `CHAN` | tap: release under 0.4 s: step 3 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: open NOTE EDIT on step 3 of the viewed page | `STEP` |
| `KEY_W03_A3` | `SET` | tap: release under 0.4 s: step 3 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: open NOTE EDIT on step 3 of the viewed page | `STEP` |
| `KEY_W03_A3` | `SCALE` | tap: type chord 3: writes degree 3 at the cursor of the progression (the first key after opening or after NEW starts at chord 1; up to 8 chords; BARS grows to hold them) and sounds the chord on the selected channel's MIDI channel; EDIT tap undoes | `DEG` |
| `KEY_W03_A3` | `REC` | tap: play A3 (+ REC OCT and global OCTAVE) on the selected channel and write it at the cursor / nearest step; release = note off; force: landing force picks the written level SOFT 64 / MED 90 / HARD 120 (HARD = accent); fallback: velocity 90, or 120 with K2 ACC ON | `NOTE` |
| `KEY_W03_A3` | `RESET` | tap: clear every edit of step 3 of the viewed page (back to the generator); RESET closes; EDIT tap undoes | `CLR` |
| `KEY_W03_A3` | `HELP` | tap/hold: explain this key on the page beneath (HOME held); nothing happens | `?` |
| `KEY_W03_A3` | `MENU` | tap/hold: inert while the menu is open | `-` |
| `KEY_W04_B3` | `PERF` | tap: release under 0.4 s: step 4 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: open NOTE EDIT on step 4 of the viewed page | `STEP` |
| `KEY_W04_B3` | `NOTE_EDIT` | tap: release under 0.4 s: step 4 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: move NOTE EDIT to step 4 (no new page) | `STEP` |
| `KEY_W04_B3` | `CHAN` | tap: release under 0.4 s: step 4 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: open NOTE EDIT on step 4 of the viewed page | `STEP` |
| `KEY_W04_B3` | `SET` | tap: release under 0.4 s: step 4 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: open NOTE EDIT on step 4 of the viewed page | `STEP` |
| `KEY_W04_B3` | `SCALE` | tap: type chord 4: writes degree 4 at the cursor of the progression (the first key after opening or after NEW starts at chord 1; up to 8 chords; BARS grows to hold them) and sounds the chord on the selected channel's MIDI channel; EDIT tap undoes | `DEG` |
| `KEY_W04_B3` | `REC` | tap: play B3 (+ REC OCT and global OCTAVE) on the selected channel and write it at the cursor / nearest step; release = note off; force: landing force picks the written level SOFT 64 / MED 90 / HARD 120 (HARD = accent); fallback: velocity 90, or 120 with K2 ACC ON | `NOTE` |
| `KEY_W04_B3` | `RESET` | tap: clear every edit of step 4 of the viewed page (back to the generator); RESET closes; EDIT tap undoes | `CLR` |
| `KEY_W04_B3` | `HELP` | tap/hold: explain this key on the page beneath (HOME held); nothing happens | `?` |
| `KEY_W04_B3` | `MENU` | tap/hold: inert while the menu is open | `-` |
| `KEY_W05_C4` | `PERF` | tap: release under 0.4 s: step 5 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: open NOTE EDIT on step 5 of the viewed page | `STEP` |
| `KEY_W05_C4` | `NOTE_EDIT` | tap: release under 0.4 s: step 5 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: move NOTE EDIT to step 5 (no new page) | `STEP` |
| `KEY_W05_C4` | `CHAN` | tap: release under 0.4 s: step 5 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: open NOTE EDIT on step 5 of the viewed page | `STEP` |
| `KEY_W05_C4` | `SET` | tap: release under 0.4 s: step 5 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: open NOTE EDIT on step 5 of the viewed page | `STEP` |
| `KEY_W05_C4` | `SCALE` | tap: type chord 5: writes degree 5 at the cursor of the progression (the first key after opening or after NEW starts at chord 1; up to 8 chords; BARS grows to hold them) and sounds the chord on the selected channel's MIDI channel; EDIT tap undoes | `DEG` |
| `KEY_W05_C4` | `REC` | tap: play C4 (+ REC OCT and global OCTAVE) on the selected channel and write it at the cursor / nearest step; release = note off; force: landing force picks the written level SOFT 64 / MED 90 / HARD 120 (HARD = accent); fallback: velocity 90, or 120 with K2 ACC ON | `NOTE` |
| `KEY_W05_C4` | `RESET` | tap: clear every edit of step 5 of the viewed page (back to the generator); RESET closes; EDIT tap undoes | `CLR` |
| `KEY_W05_C4` | `HELP` | tap/hold: explain this key on the page beneath (HOME held); nothing happens | `?` |
| `KEY_W05_C4` | `MENU` | tap/hold: inert while the menu is open | `-` |
| `KEY_W06_D4` | `PERF` | tap: release under 0.4 s: step 6 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: open NOTE EDIT on step 6 of the viewed page | `STEP` |
| `KEY_W06_D4` | `NOTE_EDIT` | tap: release under 0.4 s: step 6 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: move NOTE EDIT to step 6 (no new page) | `STEP` |
| `KEY_W06_D4` | `CHAN` | tap: release under 0.4 s: step 6 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: open NOTE EDIT on step 6 of the viewed page | `STEP` |
| `KEY_W06_D4` | `SET` | tap: release under 0.4 s: step 6 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: open NOTE EDIT on step 6 of the viewed page | `STEP` |
| `KEY_W06_D4` | `SCALE` | tap: type chord 6: writes degree 6 at the cursor of the progression (the first key after opening or after NEW starts at chord 1; up to 8 chords; BARS grows to hold them) and sounds the chord on the selected channel's MIDI channel; EDIT tap undoes | `DEG` |
| `KEY_W06_D4` | `REC` | tap: play D4 (+ REC OCT and global OCTAVE) on the selected channel and write it at the cursor / nearest step; release = note off; force: landing force picks the written level SOFT 64 / MED 90 / HARD 120 (HARD = accent); fallback: velocity 90, or 120 with K2 ACC ON | `NOTE` |
| `KEY_W06_D4` | `RESET` | tap: clear every edit of step 6 of the viewed page (back to the generator); RESET closes; EDIT tap undoes | `CLR` |
| `KEY_W06_D4` | `HELP` | tap/hold: explain this key on the page beneath (HOME held); nothing happens | `?` |
| `KEY_W06_D4` | `MENU` | tap/hold: inert while the menu is open | `-` |
| `KEY_W07_E4` | `PERF` | tap: release under 0.4 s: step 7 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: open NOTE EDIT on step 7 of the viewed page | `STEP` |
| `KEY_W07_E4` | `NOTE_EDIT` | tap: release under 0.4 s: step 7 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: move NOTE EDIT to step 7 (no new page) | `STEP` |
| `KEY_W07_E4` | `CHAN` | tap: release under 0.4 s: step 7 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: open NOTE EDIT on step 7 of the viewed page | `STEP` |
| `KEY_W07_E4` | `SET` | tap: release under 0.4 s: step 7 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: open NOTE EDIT on step 7 of the viewed page | `STEP` |
| `KEY_W07_E4` | `SCALE` | tap: type chord 7: writes degree 7 at the cursor of the progression (the first key after opening or after NEW starts at chord 1; up to 8 chords; BARS grows to hold them) and sounds the chord on the selected channel's MIDI channel; EDIT tap undoes | `DEG` |
| `KEY_W07_E4` | `REC` | tap: play E4 (+ REC OCT and global OCTAVE) on the selected channel and write it at the cursor / nearest step; release = note off; force: landing force picks the written level SOFT 64 / MED 90 / HARD 120 (HARD = accent); fallback: velocity 90, or 120 with K2 ACC ON | `NOTE` |
| `KEY_W07_E4` | `RESET` | tap: clear every edit of step 7 of the viewed page (back to the generator); RESET closes; EDIT tap undoes | `CLR` |
| `KEY_W07_E4` | `HELP` | tap/hold: explain this key on the page beneath (HOME held); nothing happens | `?` |
| `KEY_W07_E4` | `MENU` | tap/hold: inert while the menu is open | `-` |
| `KEY_W08_F4` | `PERF` | tap: release under 0.4 s: step 8 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: open NOTE EDIT on step 8 of the viewed page | `STEP` |
| `KEY_W08_F4` | `NOTE_EDIT` | tap: release under 0.4 s: step 8 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: move NOTE EDIT to step 8 (no new page) | `STEP` |
| `KEY_W08_F4` | `CHAN` | tap: release under 0.4 s: step 8 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: open NOTE EDIT on step 8 of the viewed page | `STEP` |
| `KEY_W08_F4` | `SET` | tap: release under 0.4 s: step 8 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: open NOTE EDIT on step 8 of the viewed page | `STEP` |
| `KEY_W08_F4` | `SCALE` | tap: not used on HARMONY (the KEY is K2) | `-` |
| `KEY_W08_F4` | `REC` | tap: play F4 (+ REC OCT and global OCTAVE) on the selected channel and write it at the cursor / nearest step; release = note off; force: landing force picks the written level SOFT 64 / MED 90 / HARD 120 (HARD = accent); fallback: velocity 90, or 120 with K2 ACC ON | `NOTE` |
| `KEY_W08_F4` | `RESET` | tap: clear every edit of step 8 of the viewed page (back to the generator); RESET closes; EDIT tap undoes | `CLR` |
| `KEY_W08_F4` | `HELP` | tap/hold: explain this key on the page beneath (HOME held); nothing happens | `?` |
| `KEY_W08_F4` | `MENU` | tap/hold: inert while the menu is open | `-` |
| `KEY_W09_G4` | `PERF` | tap: release under 0.4 s: step 9 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: open NOTE EDIT on step 9 of the viewed page | `STEP` |
| `KEY_W09_G4` | `NOTE_EDIT` | tap: release under 0.4 s: step 9 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: move NOTE EDIT to step 9 (no new page) | `STEP` |
| `KEY_W09_G4` | `CHAN` | tap: release under 0.4 s: step 9 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: open NOTE EDIT on step 9 of the viewed page | `STEP` |
| `KEY_W09_G4` | `SET` | tap: release under 0.4 s: step 9 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: open NOTE EDIT on step 9 of the viewed page | `STEP` |
| `KEY_W09_G4` | `SCALE` | tap: not used on HARMONY (the KEY is K2) | `-` |
| `KEY_W09_G4` | `REC` | tap: play G4 (+ REC OCT and global OCTAVE) on the selected channel and write it at the cursor / nearest step; release = note off; force: landing force picks the written level SOFT 64 / MED 90 / HARD 120 (HARD = accent); fallback: velocity 90, or 120 with K2 ACC ON | `NOTE` |
| `KEY_W09_G4` | `RESET` | tap: clear every edit of step 9 of the viewed page (back to the generator); RESET closes; EDIT tap undoes | `CLR` |
| `KEY_W09_G4` | `HELP` | tap/hold: explain this key on the page beneath (HOME held); nothing happens | `?` |
| `KEY_W09_G4` | `MENU` | tap/hold: inert while the menu is open | `-` |
| `KEY_W10_A4` | `PERF` | tap: release under 0.4 s: step 10 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: open NOTE EDIT on step 10 of the viewed page | `STEP` |
| `KEY_W10_A4` | `NOTE_EDIT` | tap: release under 0.4 s: step 10 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: move NOTE EDIT to step 10 (no new page) | `STEP` |
| `KEY_W10_A4` | `CHAN` | tap: release under 0.4 s: step 10 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: open NOTE EDIT on step 10 of the viewed page | `STEP` |
| `KEY_W10_A4` | `SET` | tap: release under 0.4 s: step 10 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: open NOTE EDIT on step 10 of the viewed page | `STEP` |
| `KEY_W10_A4` | `SCALE` | tap: not used on HARMONY (the KEY is K2) | `-` |
| `KEY_W10_A4` | `REC` | tap: play A4 (+ REC OCT and global OCTAVE) on the selected channel and write it at the cursor / nearest step; release = note off; force: landing force picks the written level SOFT 64 / MED 90 / HARD 120 (HARD = accent); fallback: velocity 90, or 120 with K2 ACC ON | `NOTE` |
| `KEY_W10_A4` | `RESET` | tap: clear every edit of step 10 of the viewed page (back to the generator); RESET closes; EDIT tap undoes | `CLR` |
| `KEY_W10_A4` | `HELP` | tap/hold: explain this key on the page beneath (HOME held); nothing happens | `?` |
| `KEY_W10_A4` | `MENU` | tap/hold: inert while the menu is open | `-` |
| `KEY_W11_B4` | `PERF` | tap: release under 0.4 s: step 11 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: open NOTE EDIT on step 11 of the viewed page | `STEP` |
| `KEY_W11_B4` | `NOTE_EDIT` | tap: release under 0.4 s: step 11 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: move NOTE EDIT to step 11 (no new page) | `STEP` |
| `KEY_W11_B4` | `CHAN` | tap: release under 0.4 s: step 11 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: open NOTE EDIT on step 11 of the viewed page | `STEP` |
| `KEY_W11_B4` | `SET` | tap: release under 0.4 s: step 11 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: open NOTE EDIT on step 11 of the viewed page | `STEP` |
| `KEY_W11_B4` | `SCALE` | tap: not used on HARMONY (the KEY is K2) | `-` |
| `KEY_W11_B4` | `REC` | tap: play B4 (+ REC OCT and global OCTAVE) on the selected channel and write it at the cursor / nearest step; release = note off; force: landing force picks the written level SOFT 64 / MED 90 / HARD 120 (HARD = accent); fallback: velocity 90, or 120 with K2 ACC ON | `NOTE` |
| `KEY_W11_B4` | `RESET` | tap: clear every edit of step 11 of the viewed page (back to the generator); RESET closes; EDIT tap undoes | `CLR` |
| `KEY_W11_B4` | `HELP` | tap/hold: explain this key on the page beneath (HOME held); nothing happens | `?` |
| `KEY_W11_B4` | `MENU` | tap/hold: inert while the menu is open | `-` |
| `KEY_W12_C5` | `PERF` | tap: release under 0.4 s: step 12 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: open NOTE EDIT on step 12 of the viewed page | `STEP` |
| `KEY_W12_C5` | `NOTE_EDIT` | tap: release under 0.4 s: step 12 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: move NOTE EDIT to step 12 (no new page) | `STEP` |
| `KEY_W12_C5` | `CHAN` | tap: release under 0.4 s: step 12 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: open NOTE EDIT on step 12 of the viewed page | `STEP` |
| `KEY_W12_C5` | `SET` | tap: release under 0.4 s: step 12 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END; force: hard tap turning a step ON = ACCENT (velocity ACC_VEL); the lane cell shows ACC while the key is down; fallback: NOTE EDIT ENV (ACCENT) or K3 VELO // hold: held 0.4 s: open NOTE EDIT on step 12 of the viewed page | `STEP` |
| `KEY_W12_C5` | `SCALE` | tap: not used on HARMONY (the KEY is K2) | `-` |
| `KEY_W12_C5` | `REC` | tap: play C5 (+ REC OCT and global OCTAVE) on the selected channel and write it at the cursor / nearest step; release = note off; force: landing force picks the written level SOFT 64 / MED 90 / HARD 120 (HARD = accent); fallback: velocity 90, or 120 with K2 ACC ON | `NOTE` |
| `KEY_W12_C5` | `RESET` | tap: clear every edit of step 12 of the viewed page (back to the generator); RESET closes; EDIT tap undoes | `CLR` |
| `KEY_W12_C5` | `HELP` | tap/hold: explain this key on the page beneath (HOME held); nothing happens | `?` |
| `KEY_W12_C5` | `MENU` | tap/hold: inert while the menu is open | `-` |
| `KEY_W13_D5` | `PERF` | tap: release under 0.4 s: step 13 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END // hold: held 0.4 s: open NOTE EDIT on step 13 of the viewed page | `STEP` |
| `KEY_W13_D5` | `NOTE_EDIT` | tap: release under 0.4 s: step 13 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END // hold: held 0.4 s: move NOTE EDIT to step 13 (no new page) | `STEP` |
| `KEY_W13_D5` | `CHAN` | tap: release under 0.4 s: step 13 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END // hold: held 0.4 s: open NOTE EDIT on step 13 of the viewed page | `STEP` |
| `KEY_W13_D5` | `SET` | tap: release under 0.4 s: step 13 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END // hold: held 0.4 s: open NOTE EDIT on step 13 of the viewed page | `STEP` |
| `KEY_W13_D5` | `SCALE` | tap: not used on HARMONY (the KEY is K2) | `-` |
| `KEY_W13_D5` | `REC` | tap: play D5 (+ REC OCT and global OCTAVE) on the selected channel and write it at the cursor / nearest step; release = note off; force: no force on this key; fallback: velocity 90, or 120 with K2 ACC ON | `NOTE` |
| `KEY_W13_D5` | `RESET` | tap: clear every edit of step 13 of the viewed page (back to the generator); RESET closes; EDIT tap undoes | `CLR` |
| `KEY_W13_D5` | `HELP` | tap/hold: explain this key on the page beneath (HOME held); nothing happens | `?` |
| `KEY_W13_D5` | `MENU` | tap/hold: inert while the menu is open | `-` |
| `KEY_W14_E5` | `PERF` | tap: release under 0.4 s: step 14 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END // hold: held 0.4 s: open NOTE EDIT on step 14 of the viewed page | `STEP` |
| `KEY_W14_E5` | `NOTE_EDIT` | tap: release under 0.4 s: step 14 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END // hold: held 0.4 s: move NOTE EDIT to step 14 (no new page) | `STEP` |
| `KEY_W14_E5` | `CHAN` | tap: release under 0.4 s: step 14 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END // hold: held 0.4 s: open NOTE EDIT on step 14 of the viewed page | `STEP` |
| `KEY_W14_E5` | `SET` | tap: release under 0.4 s: step 14 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END // hold: held 0.4 s: open NOTE EDIT on step 14 of the viewed page | `STEP` |
| `KEY_W14_E5` | `SCALE` | tap: not used on HARMONY (the KEY is K2) | `-` |
| `KEY_W14_E5` | `REC` | tap: play E5 (+ REC OCT and global OCTAVE) on the selected channel and write it at the cursor / nearest step; release = note off; force: no force on this key; fallback: velocity 90, or 120 with K2 ACC ON | `NOTE` |
| `KEY_W14_E5` | `RESET` | tap: clear every edit of step 14 of the viewed page (back to the generator); RESET closes; EDIT tap undoes | `CLR` |
| `KEY_W14_E5` | `HELP` | tap/hold: explain this key on the page beneath (HOME held); nothing happens | `?` |
| `KEY_W14_E5` | `MENU` | tap/hold: inert while the menu is open | `-` |
| `KEY_W15_F5` | `PERF` | tap: release under 0.4 s: step 15 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END // hold: held 0.4 s: open NOTE EDIT on step 15 of the viewed page | `STEP` |
| `KEY_W15_F5` | `NOTE_EDIT` | tap: release under 0.4 s: step 15 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END // hold: held 0.4 s: move NOTE EDIT to step 15 (no new page) | `STEP` |
| `KEY_W15_F5` | `CHAN` | tap: release under 0.4 s: step 15 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END // hold: held 0.4 s: open NOTE EDIT on step 15 of the viewed page | `STEP` |
| `KEY_W15_F5` | `SET` | tap: release under 0.4 s: step 15 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END // hold: held 0.4 s: open NOTE EDIT on step 15 of the viewed page | `STEP` |
| `KEY_W15_F5` | `SCALE` | tap: not used on HARMONY (the KEY is K2) | `-` |
| `KEY_W15_F5` | `REC` | tap: play F5 (+ REC OCT and global OCTAVE) on the selected channel and write it at the cursor / nearest step; release = note off; force: no force on this key; fallback: velocity 90, or 120 with K2 ACC ON | `NOTE` |
| `KEY_W15_F5` | `RESET` | tap: clear every edit of step 15 of the viewed page (back to the generator); RESET closes; EDIT tap undoes | `CLR` |
| `KEY_W15_F5` | `HELP` | tap/hold: explain this key on the page beneath (HOME held); nothing happens | `?` |
| `KEY_W15_F5` | `MENU` | tap/hold: inert while the menu is open | `-` |
| `KEY_W16_G5` | `PERF` | tap: release under 0.4 s: step 16 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END // hold: held 0.4 s: open NOTE EDIT on step 16 of the viewed page | `STEP` |
| `KEY_W16_G5` | `NOTE_EDIT` | tap: release under 0.4 s: step 16 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END // hold: held 0.4 s: move NOTE EDIT to step 16 (no new page) | `STEP` |
| `KEY_W16_G5` | `CHAN` | tap: release under 0.4 s: step 16 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END // hold: held 0.4 s: open NOTE EDIT on step 16 of the viewed page | `STEP` |
| `KEY_W16_G5` | `SET` | tap: release under 0.4 s: step 16 of the viewed page ON/OFF (a second tap that matches the generator returns it to AUTO); stopped: a step turned on sounds once; past LENGTH: dim, toast PAST THE END // hold: held 0.4 s: open NOTE EDIT on step 16 of the viewed page | `STEP` |
| `KEY_W16_G5` | `SCALE` | tap: not used on HARMONY (the KEY is K2) | `-` |
| `KEY_W16_G5` | `REC` | tap: play G5 (+ REC OCT and global OCTAVE) on the selected channel and write it at the cursor / nearest step; release = note off; force: no force on this key; fallback: velocity 90, or 120 with K2 ACC ON | `NOTE` |
| `KEY_W16_G5` | `RESET` | tap: clear every edit of step 16 of the viewed page (back to the generator); RESET closes; EDIT tap undoes | `CLR` |
| `KEY_W16_G5` | `HELP` | tap/hold: explain this key on the page beneath (HOME held); nothing happens | `?` |
| `KEY_W16_G5` | `MENU` | tap/hold: inert while the menu is open | `-` |
| `KEY_B01_F#3` | `PERF` | tap: tap: mute / unmute CH1 (latched; phase keeps running) | `MUTE1` |
| `KEY_B01_F#3` | `NOTE_EDIT` | tap: tap: mute / unmute CH1 (latched; phase keeps running) | `MUTE1` |
| `KEY_B01_F#3` | `CHAN` | tap: tap: mute / unmute CH1 (latched; phase keeps running) | `MUTE1` |
| `KEY_B01_F#3` | `SET` | tap: tap: mute / unmute CH1 (latched; phase keeps running) | `MUTE1` |
| `KEY_B01_F#3` | `SCALE` | tap: not used on HARMONY (the KEY is K2) | `-` |
| `KEY_B01_F#3` | `REC` | tap: play F#3 (+ REC OCT and global OCTAVE) on the selected channel and write it at the cursor / nearest step; release = note off; force: landing force picks the written level SOFT 64 / MED 90 / HARD 120 (HARD = accent); fallback: velocity 90, or 120 with K2 ACC ON | `NOTE` |
| `KEY_B01_F#3` | `RESET` | tap: inert in RESET (toast NOTHING TO RESET) | `-` |
| `KEY_B01_F#3` | `HELP` | tap/hold: explain this key on the page beneath (HOME held); nothing happens | `?` |
| `KEY_B01_F#3` | `MENU` | tap/hold: inert while the menu is open | `-` |
| `KEY_B02_G#3` | `PERF` | tap: tap: mute / unmute CH2 | `MUTE2` |
| `KEY_B02_G#3` | `NOTE_EDIT` | tap: tap: mute / unmute CH2 | `MUTE2` |
| `KEY_B02_G#3` | `CHAN` | tap: tap: mute / unmute CH2 | `MUTE2` |
| `KEY_B02_G#3` | `SET` | tap: tap: mute / unmute CH2 | `MUTE2` |
| `KEY_B02_G#3` | `SCALE` | tap: not used on HARMONY (the KEY is K2) | `-` |
| `KEY_B02_G#3` | `REC` | tap: play G#3 (+ REC OCT and global OCTAVE) on the selected channel and write it at the cursor / nearest step; release = note off; force: landing force picks the written level SOFT 64 / MED 90 / HARD 120 (HARD = accent); fallback: velocity 90, or 120 with K2 ACC ON | `NOTE` |
| `KEY_B02_G#3` | `RESET` | tap: inert in RESET (toast NOTHING TO RESET) | `-` |
| `KEY_B02_G#3` | `HELP` | tap/hold: explain this key on the page beneath (HOME held); nothing happens | `?` |
| `KEY_B02_G#3` | `MENU` | tap/hold: inert while the menu is open | `-` |
| `KEY_B03_A#3` | `PERF` | tap: tap: mute / unmute CH3 | `MUTE3` |
| `KEY_B03_A#3` | `NOTE_EDIT` | tap: tap: mute / unmute CH3 | `MUTE3` |
| `KEY_B03_A#3` | `CHAN` | tap: tap: mute / unmute CH3 | `MUTE3` |
| `KEY_B03_A#3` | `SET` | tap: tap: mute / unmute CH3 | `MUTE3` |
| `KEY_B03_A#3` | `SCALE` | tap: not used on HARMONY (the KEY is K2) | `-` |
| `KEY_B03_A#3` | `REC` | tap: play A#3 (+ REC OCT and global OCTAVE) on the selected channel and write it at the cursor / nearest step; release = note off; force: landing force picks the written level SOFT 64 / MED 90 / HARD 120 (HARD = accent); fallback: velocity 90, or 120 with K2 ACC ON | `NOTE` |
| `KEY_B03_A#3` | `RESET` | tap: inert in RESET (toast NOTHING TO RESET) | `-` |
| `KEY_B03_A#3` | `HELP` | tap/hold: explain this key on the page beneath (HOME held); nothing happens | `?` |
| `KEY_B03_A#3` | `MENU` | tap/hold: inert while the menu is open | `-` |
| `KEY_B04_C#4` | `PERF` | tap: tap: mute / unmute CH4 | `MUTE4` |
| `KEY_B04_C#4` | `NOTE_EDIT` | tap: tap: mute / unmute CH4 | `MUTE4` |
| `KEY_B04_C#4` | `CHAN` | tap: tap: mute / unmute CH4 | `MUTE4` |
| `KEY_B04_C#4` | `SET` | tap: tap: mute / unmute CH4 | `MUTE4` |
| `KEY_B04_C#4` | `SCALE` | tap: not used on HARMONY (the KEY is K2) | `-` |
| `KEY_B04_C#4` | `REC` | tap: play C#4 (+ REC OCT and global OCTAVE) on the selected channel and write it at the cursor / nearest step; release = note off; force: landing force picks the written level SOFT 64 / MED 90 / HARD 120 (HARD = accent); fallback: velocity 90, or 120 with K2 ACC ON | `NOTE` |
| `KEY_B04_C#4` | `RESET` | tap: inert in RESET (toast NOTHING TO RESET) | `-` |
| `KEY_B04_C#4` | `HELP` | tap/hold: explain this key on the page beneath (HOME held); nothing happens | `?` |
| `KEY_B04_C#4` | `MENU` | tap/hold: inert while the menu is open | `-` |
| `KEY_B05_D#4` | `PERF` | tap: tap: open the CHAN page of the selected channel (LENGTH MIDI SOURCE FOLLOW) | `CHAN` |
| `KEY_B05_D#4` | `NOTE_EDIT` | tap: switch to the CHAN page (closes NOTE EDIT) | `CHAN` |
| `KEY_B05_D#4` | `CHAN` | tap: close CHAN, back to PERF | `BACK` |
| `KEY_B05_D#4` | `SET` | tap: switch to the CHAN page | `CHAN` |
| `KEY_B05_D#4` | `SCALE` | tap: not used on HARMONY (the KEY is K2) | `-` |
| `KEY_B05_D#4` | `REC` | tap: play D#4 (+ REC OCT and global OCTAVE) on the selected channel and write it at the cursor / nearest step; release = note off; force: landing force picks the written level SOFT 64 / MED 90 / HARD 120 (HARD = accent); fallback: velocity 90, or 120 with K2 ACC ON | `NOTE` |
| `KEY_B05_D#4` | `RESET` | tap: re-copy the page before onto the viewed page (edits and source; page 1: clear its edits); RESET closes; EDIT tap undoes | `COPY P` |
| `KEY_B05_D#4` | `HELP` | tap/hold: explain this key on the page beneath (HOME held); nothing happens | `?` |
| `KEY_B05_D#4` | `MENU` | tap/hold: inert while the menu is open | `-` |
| `KEY_B06_F#4` | `PERF` | hold: hold: only the selected channel sounds; release restores the mutes | `SOLO` |
| `KEY_B06_F#4` | `NOTE_EDIT` | hold: hold: only the selected channel sounds; release restores the mutes | `SOLO` |
| `KEY_B06_F#4` | `CHAN` | hold: hold: only the selected channel sounds; release restores the mutes | `SOLO` |
| `KEY_B06_F#4` | `SET` | hold: hold: only the selected channel sounds; release restores the mutes | `SOLO` |
| `KEY_B06_F#4` | `SCALE` | tap: not used on HARMONY (the KEY is K2) | `-` |
| `KEY_B06_F#4` | `REC` | tap: play F#4 (+ REC OCT and global OCTAVE) on the selected channel and write it at the cursor / nearest step; release = note off; force: landing force picks the written level SOFT 64 / MED 90 / HARD 120 (HARD = accent); fallback: velocity 90, or 120 with K2 ACC ON | `NOTE` |
| `KEY_B06_F#4` | `RESET` | tap: inert in RESET (toast NOTHING TO RESET) | `-` |
| `KEY_B06_F#4` | `HELP` | tap/hold: explain this key on the page beneath (HOME held); nothing happens | `?` |
| `KEY_B06_F#4` | `MENU` | tap/hold: inert while the menu is open | `-` |
| `KEY_B07_G#4` | `PERF` | hold: hold: the selected channel's K1 density goes up while held (force: light = halfway, firm = maximum); release restores; force: aftertouch: depth light..firm = halfway..maximum density; fallback: no force (or FORCE OFF): maximum density | `FILL` |
| `KEY_B07_G#4` | `NOTE_EDIT` | hold: hold: the selected channel's K1 density goes up while held (force: light = halfway, firm = maximum); release restores; force: aftertouch: depth light..firm = halfway..maximum density; fallback: no force (or FORCE OFF): maximum density | `FILL` |
| `KEY_B07_G#4` | `CHAN` | hold: hold: the selected channel's K1 density goes up while held (force: light = halfway, firm = maximum); release restores; force: aftertouch: depth light..firm = halfway..maximum density; fallback: no force (or FORCE OFF): maximum density | `FILL` |
| `KEY_B07_G#4` | `SET` | hold: hold: the selected channel's K1 density goes up while held (force: light = halfway, firm = maximum); release restores; force: aftertouch: depth light..firm = halfway..maximum density; fallback: no force (or FORCE OFF): maximum density | `FILL` |
| `KEY_B07_G#4` | `SCALE` | tap: not used on HARMONY (the KEY is K2) | `-` |
| `KEY_B07_G#4` | `REC` | tap: play G#4 (+ REC OCT and global OCTAVE) on the selected channel and write it at the cursor / nearest step; release = note off; force: landing force picks the written level SOFT 64 / MED 90 / HARD 120 (HARD = accent); fallback: velocity 90, or 120 with K2 ACC ON | `NOTE` |
| `KEY_B07_G#4` | `RESET` | tap: inert in RESET (toast NOTHING TO RESET) | `-` |
| `KEY_B07_G#4` | `HELP` | tap/hold: explain this key on the page beneath (HOME held); nothing happens | `?` |
| `KEY_B07_G#4` | `MENU` | tap/hold: inert while the menu is open | `-` |
| `KEY_B08_A#4` | `PERF` | hold: hold: every step of the selected channel repeats while held (force: 2x light, 3x medium, 4x firm); force: aftertouch: 2x / 3x / 4x by pressure bins; fallback: no force: 2x | `ROLL` |
| `KEY_B08_A#4` | `NOTE_EDIT` | hold: hold: every step of the selected channel repeats while held (force: 2x light, 3x medium, 4x firm); force: aftertouch: 2x / 3x / 4x by pressure bins; fallback: no force: 2x | `ROLL` |
| `KEY_B08_A#4` | `CHAN` | hold: hold: every step of the selected channel repeats while held (force: 2x light, 3x medium, 4x firm); force: aftertouch: 2x / 3x / 4x by pressure bins; fallback: no force: 2x | `ROLL` |
| `KEY_B08_A#4` | `SET` | hold: hold: every step of the selected channel repeats while held (force: 2x light, 3x medium, 4x firm); force: aftertouch: 2x / 3x / 4x by pressure bins; fallback: no force: 2x | `ROLL` |
| `KEY_B08_A#4` | `SCALE` | tap: not used on HARMONY (the KEY is K2) | `-` |
| `KEY_B08_A#4` | `REC` | tap: play A#4 (+ REC OCT and global OCTAVE) on the selected channel and write it at the cursor / nearest step; release = note off; force: landing force picks the written level SOFT 64 / MED 90 / HARD 120 (HARD = accent); fallback: velocity 90, or 120 with K2 ACC ON | `NOTE` |
| `KEY_B08_A#4` | `RESET` | tap: inert in RESET (toast NOTHING TO RESET) | `-` |
| `KEY_B08_A#4` | `HELP` | tap/hold: explain this key on the page beneath (HOME held); nothing happens | `?` |
| `KEY_B08_A#4` | `MENU` | tap/hold: inert while the menu is open | `-` |
| `KEY_B09_C#5` | `PERF` | tap: tap: view the previous page of the selected channel (no wrap; FOLLOW off) | `PG-` |
| `KEY_B09_C#5` | `NOTE_EDIT` | tap: tap: view the previous page of the selected channel (no wrap; FOLLOW off) | `PG-` |
| `KEY_B09_C#5` | `CHAN` | tap: tap: view the previous page of the selected channel (no wrap; FOLLOW off) | `PG-` |
| `KEY_B09_C#5` | `SET` | tap: tap: view the previous page of the selected channel (no wrap; FOLLOW off) | `PG-` |
| `KEY_B09_C#5` | `SCALE` | tap: not used on HARMONY (the KEY is K2) | `-` |
| `KEY_B09_C#5` | `REC` | tap: play C#5 (+ REC OCT and global OCTAVE) on the selected channel and write it at the cursor / nearest step; release = note off; force: no force on this key; fallback: velocity 90, or 120 with K2 ACC ON | `NOTE` |
| `KEY_B09_C#5` | `RESET` | tap: inert in RESET (toast NOTHING TO RESET) | `-` |
| `KEY_B09_C#5` | `HELP` | tap/hold: explain this key on the page beneath (HOME held); nothing happens | `?` |
| `KEY_B09_C#5` | `MENU` | tap/hold: inert while the menu is open | `-` |
| `KEY_B10_D#5` | `PERF` | tap: tap: view the next page (FOLLOW off); on the last page: a prompt that names what PG+ again will do: ADD PAGE n = COPY OF n-1 (edits and source; LENGTH to 16 x pages, max 64), SHOW PAGE n (HIDDEN) (a page hidden by a shrink comes back as stored), or SHOW ALL OF PAGE n (a partial last page, e.g. LENGTH 20, is completed to 32 first); on page 4 at LENGTH 64 no prompt: toast 64 STEPS = MAX, nothing changes | `PG+` |
| `KEY_B10_D#5` | `NOTE_EDIT` | tap: tap: view the next page (FOLLOW off); on the last page: a prompt that names what PG+ again will do: ADD PAGE n = COPY OF n-1 (edits and source; LENGTH to 16 x pages, max 64), SHOW PAGE n (HIDDEN) (a page hidden by a shrink comes back as stored), or SHOW ALL OF PAGE n (a partial last page, e.g. LENGTH 20, is completed to 32 first); on page 4 at LENGTH 64 no prompt: toast 64 STEPS = MAX, nothing changes | `PG+` |
| `KEY_B10_D#5` | `CHAN` | tap: tap: view the next page (FOLLOW off); on the last page: a prompt that names what PG+ again will do: ADD PAGE n = COPY OF n-1 (edits and source; LENGTH to 16 x pages, max 64), SHOW PAGE n (HIDDEN) (a page hidden by a shrink comes back as stored), or SHOW ALL OF PAGE n (a partial last page, e.g. LENGTH 20, is completed to 32 first); on page 4 at LENGTH 64 no prompt: toast 64 STEPS = MAX, nothing changes | `PG+` |
| `KEY_B10_D#5` | `SET` | tap: tap: view the next page (FOLLOW off); on the last page: a prompt that names what PG+ again will do: ADD PAGE n = COPY OF n-1 (edits and source; LENGTH to 16 x pages, max 64), SHOW PAGE n (HIDDEN) (a page hidden by a shrink comes back as stored), or SHOW ALL OF PAGE n (a partial last page, e.g. LENGTH 20, is completed to 32 first); on page 4 at LENGTH 64 no prompt: toast 64 STEPS = MAX, nothing changes | `PG+` |
| `KEY_B10_D#5` | `SCALE` | tap: not used on HARMONY (the KEY is K2) | `-` |
| `KEY_B10_D#5` | `REC` | tap: play D#5 (+ REC OCT and global OCTAVE) on the selected channel and write it at the cursor / nearest step; release = note off; force: no force on this key; fallback: velocity 90, or 120 with K2 ACC ON | `NOTE` |
| `KEY_B10_D#5` | `RESET` | tap: inert in RESET (toast NOTHING TO RESET) | `-` |
| `KEY_B10_D#5` | `HELP` | tap/hold: explain this key on the page beneath (HOME held); nothing happens | `?` |
| `KEY_B10_D#5` | `MENU` | tap/hold: inert while the menu is open | `-` |
| `KEY_B11_F#5` | `PERF` | tap: tap: all channels to step 1 at the next step | `RESTRT` |
| `KEY_B11_F#5` | `NOTE_EDIT` | tap: tap: all channels to step 1 at the next step | `RESTRT` |
| `KEY_B11_F#5` | `CHAN` | tap: tap: all channels to step 1 at the next step | `RESTRT` |
| `KEY_B11_F#5` | `SET` | tap: tap: all channels to step 1 at the next step | `RESTRT` |
| `KEY_B11_F#5` | `SCALE` | tap: not used on HARMONY (the KEY is K2) | `-` |
| `KEY_B11_F#5` | `REC` | tap: play F#5 (+ REC OCT and global OCTAVE) on the selected channel and write it at the cursor / nearest step; release = note off; force: no force on this key; fallback: velocity 90, or 120 with K2 ACC ON | `NOTE` |
| `KEY_B11_F#5` | `RESET` | tap: inert in RESET (toast NOTHING TO RESET) | `-` |
| `KEY_B11_F#5` | `HELP` | tap/hold: explain this key on the page beneath (HOME held); nothing happens | `?` |
| `KEY_B11_F#5` | `MENU` | tap/hold: inert while the menu is open | `-` |
<!-- END-APPENDIX-A -->
