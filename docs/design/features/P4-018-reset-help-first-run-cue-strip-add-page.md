<!-- SPDX-License-Identifier: GPL-3.0-only -->
# P4-018: RESET, HELP, first run, cue priority, ADD_PAGE and MOVE_HELD: use case

Process: docs/project/UX_GATE.md. Gate A fills 1-6 before the build; Gate B fills 7-9 after it.
Task: backlog/tasks/P4-018-reset-help-first-run-cue-strip-add-page-and-move-h.md   Spec: SCREEN_SPEC_V2 4.9, 4.10, 4.11, 5; SEQUENCER_UX 2.6, 2.7, 11.1-11.4   Scenario: S01 (all), S08 (all), S15, S16; S03 steps 23-24

**STATUS: BUILT on the host (the real unit is HW-1). RESET and HELP are NEW screens on V2 (no mock scene): the owner approves them (SCREEN_SPEC_V2 4.9, 4.10).**

## 1. Who and when
A producer who has made a mess (a morph, a dice, a step edit) wants one thing back without losing the rest; who does not know what a control does and wants to ask the panel; who turns the device on for the first time and wants the first sound in a few gestures.

## 2. Steps a producer takes
| # | Producer does (control id, context) | Screen shows after | Sound / MIDI after |
| --- | --- | --- | --- |
| 1 | Hold EDIT (B_EDIT, PERF / NOTE_EDIT / CHAN / SET) | from 0.25 s the slot reads `RESET` with the undo icon and a bar; at 0.4 s RESET: white label bands on the tiles that go back, white outlines on the buttons and keys that do, footer `TOUCH WHAT TO RESET / EDIT = NO` | playback continues |
| 2 | Turn a knob / press a dice button / tap a step key / REC / D#4 | that value, layer, step, page back to default; RESET closes; toast `RESET  EDIT=UNDO` (`CLEAR`, `COPIED`) | from the next step |
| 3 | Tap EDIT | the reset is taken back (toast `UNDO`; again `REDO`) | from the next step |
| 4 | Touch something with nothing to reset (GLO, SAVE, ALGO, PRESETS, an M key, a layer the algorithm lacks) | toast `NOTHING TO RESET`, RESET stays | none |
| 5 | Tap EDIT or HOME while RESET is open | RESET closes (HOME also returns to PERF) | none |
| 6 | Hold HOME | from 0.25 s the slot reads `HELP / MENU` with a bar to 0.7 s | none |
| 7 | While HOME is down, touch or turn any control | the HELP box names it (two lines), the slot reads `HELP  LET GO = BACK`; the control does nothing (PLAY still plays) | none (PLAY: start / stop) |
| 8 | Let go of HOME | back on the page as it was; no menu | none |
| 9 | Hold HOME to 0.7 s touching nothing | the MENU opens (Felucca, unchanged) | none |
| 10 | Tap HOME on PERF | toast `HOME  HOLD=HELP`; cancels an add-page prompt, ends the first-run strip | none |
| 11 | First power-on (S01) | PERF with the setup cue `CHN KEY, K2 = MIDI` (CH1, CHN lit); then D#4, K2, HOME, PLAY: the footer line advances through `PRESS PLAY` ... `PG+ TWICE: ADD PAGE 2` with `HOME = SKIP`; the end stores one bit and it never returns | the TB-3 plays from the first PLAY |
| 12 | PG+ on the last page | toast `PAGE 2?  PG+=ADD`; PG+ again adds; HOME or any other control cancels | none |
| 13 | Hold SOL / FIL / ROL | the key inverted, the slot `SOLO  CH1 SOLO` / `FILL  100%  LET GO` / `ROLL  2X  LET GO` while it is down | none yet (P6-003 wires the move) |

## 3. Functions and class
| Function | Class (Performance or Setup) |
| --- | --- |
| Put one value, layer, step or page back (hold EDIT + one control) | Setup (2 gestures) |
| Take it back (EDIT tap) | Performance (1) |
| Ask what a control does (hold HOME + touch) | Setup (2 gestures) |
| First run strip | Setup |
| Add a page (PG+ twice) | Setup (2 gestures) |

## 4. Predicted ease score
| Function | V | I | O | L | /12 | Why these numbers | Pass? |
| --- | --- | --- | --- | --- | --- | --- | --- |
| RESET one control | 3 | 3 | 3 | 2 | 11 | the hold hint names it, the marked controls say what is on offer, one touch does it, EDIT takes it back; the hold is printed on no button (the hint teaches it) | yes |
| Take it back | 3 | 3 | 3 | 3 | 12 | the toast says `EDIT=UNDO` | yes |
| HELP | 3 | 3 | 3 | 2 | 11 | the hint names it from 0.25 s; the box names the control; the hold + touch is learnt once | yes |
| First run | 3 | 3 | 3 | 3 | 12 | each step is the next thing to press, on the screen | yes |
| Add a page | 3 | 3 | 3 | 3 | 12 | the toast says what PG+ will do | yes |
Pass: performance >= 11 and no sub-score < 2; setup >= 9 and no 0.

## 5. Sync check (row or section, added or changed in this task)
| Surface | Row / section | Agrees? |
| --- | --- | --- |
| Buttons and knobs | button_modes / control_map / CONTROL_MAP: HOME tap toast, HOME hold hint wording, REC EDIT hold; `tools/check_control_map.py` stays at its 78 earlier failures | yes |
| UI / screen | SCREEN_SPEC_V2 4.9-4.11, 8, 7; page_states RESET HELP FIRST_RUN ADD_PAGE MOVE_HELD rows (screen, cue); overlay_text.tsv; v12.py MESSAGES (8 rows); widths.tsv; SCREEN_SPEC.md banner | yes |
| UX / help | help_text (392 rows, every control x page) read by the HELP box: `check_help.py` OK; SEQUENCER_UX 2.6, 11.2, 11.3 as built | yes |
| Workflow | S01, S02, S03, S08 steps rewritten; S15 / S16 regenerated; `check_scenarios.py` passes | yes |

## 6. Live checklist (yes / no + reason / n/a)
- [x] Reachable from the main screen, no menu: hold EDIT, hold HOME
- [x] Undoable in one press or self-reverting: a reset is one EDIT tap away (LENGTH, FOLLOW and the SET values are not undoable: their toast has no hint); HELP changes nothing
- [x] Lands on a musical boundary, no hung or cut notes: resets rebuild the caches and play from the next step; nothing is cut
- [x] Screen shows the value within one frame; sound changes by the next step
- [ ] One-handed: no. Hold EDIT (or HOME) with one finger and touch or turn the control with the other hand: a two-finger gesture by design (SEQUENCER_UX 2.6, owner decisions 57 / 61), two gestures in the budget
- [x] Does not change another channel (or says so on screen): every reset acts on the selected channel; CHAN MIDI default may take a MIDI channel from another (its own toast says so)
- [x] Survives stop/start, clock-source change, preset load: RESET and HELP do not depend on the transport; a preset landing clears the undo records (a stale EDIT does nothing)
- [x] Cost recorded (RAM headroom >= 8 KB): see the task Log

Hazards checked in the design and the tests (bounds, stale state, surface sync):
1. Bounds: every key index 0..26, step 0..63 inside LENGTH, page 0..3, the help row lookup over 49 controls x 8 pages (all present), the first-run step 1..7, the message strings (`NOTHING TO RESET` etc.) width-checked by `gen_ui_tables.py`.
2. Stale state: RESET closes when the page beneath cannot be reset (never over REC), when EDIT or HOME cancels, after one reset; HELP clears when HOME is up and masks keys pressed under it until they are physically up; no HOME flag survives its release; a held move is cleared on release on any page (SET, REC, SELECT moved); the EDIT hold ends on release; a pending CHAN MIDI channel lands before RESET opens; the add-page prompt is cancelled by every other control, RESET opening and HOME.
3. Surface sync: the tables and docs named in section 5; `docs/design/forbidden_terms.txt` carries the old wordings.

Design locked: SEQUENCER_UX 2.6, 2.7, 11 (P1-017, approved) for the behaviour; the RESET and HELP pictures are new on V2 and await owner approval.

## 7. Walk-through (after the build; Gate B)
Method: host tests (`tests/ui_sq_safety_test.c`, `tests/ui_test.c` through the real ui_input.c and ui_sq_glue.c). Frames in build/ui_sq/safety_*.png. Real unit: pending (HW-1). Executor-prepared; the reviewer confirms.

## 8. Actual ease score
Pending the reviewer (Gate B).

## 9. Sync re-check and outcome
- `sh tools/check.sh`: CHECK PASS; `check_scenarios.py`, `check_help.py`, `check_sequencer_ux.py`: pass.
- Outcome: pending (reviewer).
