<!-- SPDX-License-Identifier: GPL-3.0-only -->
# P4-014: NOTE_EDIT page: use case

Process: docs/project/UX_GATE.md. Gate A fills 1-6 before the build; Gate B fills 7-9 after it.
Task: backlog/tasks/P4-014-note-edit-page.md   Spec: SEQUENCER_UX 1.1, 1.2, 2.2, 2.7, 4.1, F-10, F-12; SCREEN_SPEC_V2 (no NOTE_EDIT picture: proposed below)   Scenario: S03 steps 7-22, S10

## 1. Who and when
The bass line plays; the producer wants one note changed (a pitch, a longer or accented note, a ratchet, a slide) without touching the generator: hold the step's key, turn knobs, press HOME.

## 2. Steps a producer takes
| # | Producer does (control id, context) | Screen shows after | Sound / MIDI after |
| --- | --- | --- | --- |
| 1 | Hold white key 9 for 0.4 s (KEY_W09_G4, PERF) | NOTE_EDIT: the four tiles `PITCH GATE VELO RATCH` all `AUTO`, each with `GEN <value>` under it; status line `STEP 9`; the step framed in the music row and on its key; footer `KNOBS EDIT   HOME = DONE` | none (the audition is an ISR job, see Gate B debt) |
| 2 | Turn K1 two detents up (EN_K1) | the tile fills in the channel colour and reads a note name (first detent: the generated pitch's next scale degree, F-12); AUTO is gone | from the next pass the step plays that pitch |
| 3 | Turn K2 to the top | the tile reads `TIE`; turn it back down to the bottom: `AUTO` and the fill goes (the StepEdit field is cleared) | next pass: no note-off until the next step |
| 4 | Turn K3 / K4 (EN_K3, EN_K4) | `VELO` shows the number, `ACC` at 100 or more; `RATCH` shows `2X`; each goes back to `AUTO` at the bottom | next pass |
| 5 | Press ARP (SLIDE), LFO (PROB), ENV (ACCENT), OCT+ (EDIT-less one-press each) | status line chips `SLIDE`, `PROB 50`, `OCT+1`; `ACC` on VELO | next pass |
| 6 | Press SCL (NOTE) or FX (ALL) | new pitch (and for ALL accent, slide, gate), toast `NOTES` / `ALL  EDIT=UNDO` | next pass |
| 7 | Press EDIT (B_EDIT) | the step is as it was when the page opened; toast `UNDO  EDIT=REDO`; again = `REDO` | next pass |
| 8 | Hold another white key 0.4 s | the frame moves to that step, `STEP 13`; a new undo snapshot | none |
| 9 | Tap HOME | back to PERF; the session is closed | none |

## 3. Functions and class
| Function | Class (Performance or Setup) |
| --- | --- |
| Open on a step, move the editor | Performance (one hold) |
| Set / clear PITCH GATE VELO RATCH | Performance (one turn) |
| SLIDE, PROB, ACCENT, OCT, ON/OFF, dice this step | Performance (one press) |
| EDIT undo / redo, HOME | Performance |

## 4. Predicted ease score
| Function | V | I | O | L | /12 | Why these numbers | Pass? |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Open on a step | 3 | 3 | 3 | 2 | 11 | the framed step and `STEP n` answer at once; the PERF cue `HOLD=EDIT NOTE` names it; one hold | yes |
| Knob set / clear | 3 | 3 | 3 | 3 | 12 | the knob's tile is the value; AUTO is a word and an unfilled tile; the generated value sits under it | yes |
| Layer buttons | 2 | 3 | 2 | 2 | 9 | the icons are the PERF dice icons: the picture does not say "this step"; the chips and tiles answer after the press | **no: 9 < 11** |
| EDIT undo | 2 | 3 | 3 | 3 | 11 | the printed EDIT icon is an undo arrow; the toast names REDO | yes |
| HOME | 3 | 3 | 2 | 3 | 11 | printed word HOME; the footer says `HOME = DONE` | yes |
Pass: performance >= 11 and no sub-score < 2; setup >= 9 and no 0.
The layer buttons predict 9: they stay as designed (SEQUENCER_UX 3: "a layer button acts on its layer") and the page names the effect through the chips; a NOTE_EDIT-specific button picture is a design change for the owner (proposal 2 below).

## 5. Sync check (row or section, added or changed in this task)
| Surface | Row / section | Agrees? |
| --- | --- | --- |
| Buttons and knobs (control_map, button_modes, knob_map, key_map) | the NOTE_EDIT rows are unchanged; code follows them except the listed debt (audition, hard-tap ACCENT, EDIT hold = RESET, GLO / REC / CHN switching) | yes, with the debt |
| UI / screen (SCREEN_SPEC, page_states, widths) | page_states NOTE_EDIT row unchanged; the picture is new (proposal below); SCREEN_SPEC_V2 section 8 still lists NOTE_EDIT as not drawn there | **owner to sign the picture** |
| UX / help (help_text, UX_REVIEW) | help_text rows exist; new words live in docs/design/note_edit_text.tsv (generated into ui_ne_text) | yes |
| Workflow (scenario step) | S03 steps 7-22 reproduce in tests/ui_sq_note_test.c except the audition column and step 5-of-PERF hard tap; S03 step 8 text ("first detent = the generated note") follows SEQUENCER_UX F-12 (first detent already moves one degree) | yes, F-12 wins |

### Proposed NOTE_EDIT picture (v12 idiom; renders: build/ui_sq/note_open.png, note_set.png, note_dice.png, note_final.png)
- Header, message slot, music row, buttons, keybed: as PERF.
- Knob tiles: label from knob_map (`PITCH GATE VELO RATCH`). AUTO: plain tile, `AUTO` in AF_M MID. Set: the whole value area filled in the channel's dim colour (the meter fill at 100 %) with a level edge, the value in white AF_L (AF_M if wider than 54 px). Under every value, AF_S: `GEN <what the algorithm gives>` (SEQUENCER_UX 2.2's `(gen F2)`).
- Status line: `STEP n` (AF_S + AF_M); chips in the channel colour for the step's other set fields: `ON/OFF`, `SLIDE` / `NO SLIDE`, `PROB 50`, `OCT+1`; the set-mark legend `= SET BY YOU` when the chips leave room (an absent chip = AUTO).
- Music row and keybed: the edited step is framed in white (the REC cursor's frame, and a white frame on its key).
- Footer: `KNOBS EDIT` left, `HOME = DONE` right. No new font, token or icon.
Open for the owner: (1) the set fill is the channel's dim colour like a full meter (a shape, not colour alone); (2) a dedicated button picture for "this step" (the dice icons are reused).

## 6. Live checklist (yes / no + reason / n/a)
- [x] Reachable from the main screen, no menu: one 0.4 s hold on a step key
- [x] Undoable in one press or self-reverting: EDIT undoes the whole visit's writes (one record), again = redo
- [ ] Lands on a musical boundary, no hung or cut notes: yes for the edits (they are StepEdit fields, read by the scheduler at the next pass; TIE and SLIDE are the core's); the audition on a knob turn is NOT built (needs an ISR request flag), so a changed pitch is heard on the next pass only. Debt.
- [x] Screen shows the value within one frame; sound changes by the next step
- [x] One-handed: hold a key, turn a knob with the other hand; every button is a single press
- [x] Does not change another channel: edits are the edited channel's; the page closes when the selection moves
- [x] Survives stop/start, clock-source change, preset load: edits are in ChanSave; a landed preset ends the session (the core) and the next write opens a new one; the page needs the channel selected and in use or it closes
- [x] Cost recorded: RAM .data+.bss 29208 B of 73728 (headroom 44520 B); no new static buffer beyond ~90 B of view text; UisqView grows by 99 B on the stack of ui_sq_draw

Design locked: pending owner sign-off of the picture (James Keaney)

## 7. Walk-through (after the build)
Method: host test tests/ui_sq_note_test.c (it drives ui_sq_keys / ui_sq_button / ui_sq_knob / ui_sq_select_turn / ui_sq_home exactly as ui_sq_glue.c does); frames in build/ui_sq/note_*.png. Firmware: working tree on bb1c058.
| Step | Screen showed | MIDI |
| --- | --- | --- |
| 1 | NOTE band, `STEP 9`, nothing set, `GEN ...` under each AUTO, column 9 framed (pixel-checked) | none |
| 2 | K1: ED.note = 0x80 | degree of the generated pitch | the StepEdit field only |
| 3 | K2 top = TIE (gr 11), bottom = AUTO (gr 0) | StepEdit only |
| 4 | K3 reads ACC at 100+, 127 max, AUTO at the bottom (vel 0); K4 2X..4X, AUTO | StepEdit only |
| 5 | chips `SLIDE`, `PROB 50`, `OCT-2`; ENV sets 120 and back to AUTO (or 80 when the generator accents: F-10) | StepEdit only |
| 6 | SCL sets a pitch, FX pitch + velocity + slide + gate; toasts `NOTES` / `ALL  EDIT=UNDO` | StepEdit only |
| 7 | EDIT restores the visit's start state; again redoes; third undoes | StepEdit only |
| 8 | the editor moved to step 13 and its own snapshot; EDIT with nothing written there is inert | none |
| 9 | HOME tap: PERF, session closed; a PERF tap on the same step then undoes alone | none |
Differences from the prediction: none on the screen; MIDI is not sent by the page at all (no audition).

## 8. Actual ease score
| Function | V | I | O | L | /12 | Predicted | Delta | Pass? |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Open on a step | 3 | 3 | 3 | 2 | 11 | 11 | 0 | yes (host render; real unit pending HW-1) |
| Knob set / clear | 3 | 3 | 3 | 3 | 12 | 12 | 0 | yes |
| Layer buttons | 2 | 3 | 2 | 2 | 9 | 9 | 0 | no (see 4): owner decision |
| EDIT undo | 2 | 3 | 3 | 3 | 11 | 11 | 0 | yes |
| HOME | 3 | 3 | 2 | 3 | 11 | 11 | 0 | yes |

## 9. Sync re-check and outcome
- `sh tools/check.sh`: CHECK PASS; tests/ui_sq_note_test.c: 0 failures. `python3 tests/design/check_scenarios.py`: 0 control-map rows with no scenario; `python3 tests/design/check_help.py`: OK (392 help rows).
- Debt (to file as FB by the reviewer if accepted): audition of a changed pitch (ISR flag); hard-tap ACCENT (key force; NOTE_EDIT ENV and K3 are the fallback and exist); EDIT hold = RESET over NOTE_EDIT (P4-018); GLO / REC / CHN switching out of NOTE_EDIT (SET / REC / CHAN pages: P4-016, P4-017, P4-015).
- Outcome: Pass with debt, layer-button picture and the NOTE_EDIT picture awaiting the owner.
- Approved: pending
