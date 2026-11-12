<!-- SPDX-License-Identifier: GPL-3.0-only -->
# P4-014: NOTE_EDIT page: use case

Process: docs/project/UX_GATE.md. Gate A fills 1-6 before the build; Gate B fills 7-9 after it.
Task: backlog/tasks/P4-014-note-edit-page.md   Spec: SEQUENCER_UX 1.1, 1.2, 2.2, 2.7, 4.1, F-10, F-12; SCREEN_SPEC_V2 (no NOTE_EDIT picture: proposed below)   Scenario: S03 steps 7-22, S10

## 1. Who and when
The bass line plays; the producer wants one note changed (a pitch, a longer or accented note, a ratchet, a slide) without touching the generator: hold the step's key, turn knobs, press HOME.

## 2. Steps a producer takes
| # | Producer does (control id, context) | Screen shows after | Sound / MIDI after |
| --- | --- | --- | --- |
| 1 | Hold white key 9 for 0.4 s (KEY_W09_G4, PERF) | NOTE_EDIT: status line `STEP 9` (AF_M) and `HOLD KEY = MOVE`; footer `KNOBS = STEP 9`, `HOME = DONE`; four tiles `PITCH GATE VELO RATCH`, each with the value the step will play BIG in grey (`C2`, `40%`, `43`, `1x`) and the word `AUTO` under it; the step framed in the music row and on its key; the six layer buttons are corner-bracket "this step" icons | none (the audition is an ISR job, debt) |
| 2 | Turn K1 two detents up (EN_K1) | the tile fills in the channel colour, the note is big and white with a small white marker square at the tile's corner, `GEN C2` (the algorithm's note) under it (first detent: the generated pitch's next scale degree, F-12) | from the next pass the step plays that pitch |
| 3 | Turn K2 to the top | `TIE`; turn it down to the bottom: the fill goes, the grey generated gate and `AUTO` return (the StepEdit field is cleared); a number is followed by a small `%` | next pass: no note-off until the next step |
| 4 | Turn K3 / K4 (EN_K3, EN_K4) | `VELO` number, `ACC` at 100 or more; `RATCH` `2` with a small `x`; each back to `AUTO` at the bottom | next pass |
| 5 | Press ARP (SLIDE), LFO (PROB), ENV (ACCENT), SEQ (ON/OFF) | the pressed toggle's button turns channel colour with a white bottom edge while it is on; chips `PROB 50`, `OCT+1`, `NO SLD`, `OFF` for fields with no lit button | next pass |
| 6 | Press OCT+ / OCT- | chip `OCT+1` | next pass |
| 7 | Press SCL (NOTE) or FX (ALL) | new pitch (and for ALL accent, slide, gate); toast in the footer `NOTES` / `ALL  EDIT=UNDO` (the header keeps the algorithm and MIDI channel) | next pass |
| 8 | Press EDIT (B_EDIT) | the step is as it was when the page opened; toast `UNDO  EDIT=REDO`; again = `REDO` | next pass |
| 9 | Hold another white key 0.4 s | the frame moves to that step, `STEP 13`; a new undo snapshot | none |
| 10 | Tap HOME | back to PERF; the session is closed | none |

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
| Layer buttons | 3 | 3 | 3 | 2 | 11 | rev 2: corner brackets say "this step", the toggle's tile shows its state; the picture was the die in rev 1 (9) | yes |
| EDIT undo | 2 | 3 | 3 | 3 | 11 | the printed EDIT icon is an undo arrow; the toast names REDO | yes |
| HOME | 3 | 3 | 2 | 3 | 11 | printed word HOME; the footer says `HOME = DONE` | yes |
Pass: performance >= 11 and no sub-score < 2; setup >= 9 and no 0.
The layer buttons were 9 in revision 1 (the die lied about four of the six); revision 2's "this step" icons and lit toggles predict 11. Judge them on the sheet.

## 5. Sync check (row or section, added or changed in this task)
| Surface | Row / section | Agrees? |
| --- | --- | --- |
| Buttons and knobs (control_map, button_modes, knob_map, key_map) | the NOTE_EDIT rows are unchanged; code follows them except the listed debt (audition, hard-tap ACCENT, EDIT hold = RESET, GLO / REC / CHN switching) | yes, with the debt |
| UI / screen (SCREEN_SPEC, page_states, widths) | page_states NOTE_EDIT row unchanged; the picture is new (proposal below); SCREEN_SPEC_V2 section 8 still lists NOTE_EDIT as not drawn there | **owner to sign the picture** |
| UX / help (help_text, UX_REVIEW) | help_text rows exist; new words live in docs/design/note_edit_text.tsv (generated into ui_ne_text) | yes |
| Workflow (scenario step) | S03 steps 7-22 reproduce in tests/ui_sq_note_test.c except the audition column and step 5-of-PERF hard tap; S03 step 8 text ("first detent = the generated note") follows SEQUENCER_UX F-12 (first detent already moves one degree) | yes, F-12 wins |

### Proposed NOTE_EDIT picture, revision 2 (after the owner's critique: ASSESSMENT_A / EVIDENCE_B in docs/design/ui_review/critique_note_edit)
Renders to look at: `build/ui_sq/note_open.png`, `note_set.png`, `note_dice.png`, `note_final.png`, `note_toast.png`, and the button sheet `note_buttons_1x.png` / `note_buttons_4x.png` (top: PERF's dice; middle: the six step buttons, nothing set; bottom: toggles set).
- Layer buttons: the die ("re-roll the pattern") is replaced by open corner brackets ("this one step", the white step frame), 13 x 13, 2 px strokes, in `icons_v12.py` (`step_all/notes/accnt/slide/prob/onoff`, generated into the firmware table, pixel-identity tested). FX, SCL, ENV reuse the dice glyphs inside; SLIDE, PROB and ON/OFF are new. ENV, LFO, ARP, SEQ are toggles and show their state: grey tile = off, channel-colour tile with black glyph and a white bottom edge = on. PERF's dice are untouched.
- Knob tiles: the BIG text is the value the step will play (AF_L; AF_M for the 24 note names wider than 54 px, e.g. `C#-1`). AUTO: grey, plain tile, the small word `AUTO` under it. Set: white, tile filled in the channel's dim colour, a white marker square, `GEN <value>` under it. GATE `%` and RATCH `x` are small (AF_S) after the digits because the big font has neither.
- Mode named: `STEP n` in AF_M on the status line (white), `HOLD KEY = MOVE` (AF_S) and, when it fits, the legend square `= SET`; footer `KNOBS = STEP n` and `HOME = DONE` (the `X = Y` grammar of PERF's footer).
- Defects from EVIDENCE B: the edited key's frame is drawn below the black keys (white with a black ring inside, it was hidden by them); the music-row frame has a black ring inside so the amber bar never touches white (2.29:1); the chip row starts after a two-digit step number, so it does not move at step 10; the toast is drawn in the footer so the header's algorithm and MIDI channel stay; `2x` is lower case, GATE has `%`. Not changed (locked PERF grammar, reported): black key / SURF / DICE tile contrasts below 3:1 are the v12 palette; the neighbour bar touching the frame's outer white ring (a 1 px adjacency when the neighbours are on).
- Open for the owner: the bracket icons at 1x (the sheet), the footer toast, the short `NO SLD` chip.

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
| Layer buttons | 3 | 3 | 3 | 2 | 11 | 11 | 0 | yes on the host sheet; the real unit and the owner decide (HW-1) |
| EDIT undo | 2 | 3 | 3 | 3 | 11 | 11 | 0 | yes |
| HOME | 3 | 3 | 2 | 3 | 11 | 11 | 0 | yes |

## 9. Sync re-check and outcome
- `sh tools/check.sh`: CHECK PASS; tests/ui_sq_note_test.c: 0 failures. `python3 tests/design/check_scenarios.py`: 0 control-map rows with no scenario; `python3 tests/design/check_help.py`: OK (392 help rows).
- Debt (to file as FB by the reviewer if accepted): audition of a changed pitch (ISR flag); hard-tap ACCENT (key force; NOTE_EDIT ENV and K3 are the fallback and exist); EDIT hold = RESET over NOTE_EDIT (P4-018); GLO / REC / CHN switching out of NOTE_EDIT (SET / REC / CHAN pages: P4-016, P4-017, P4-015).
- Outcome: Pass with debt (revision 2); the picture and the button icons await the owner's re-look.
- Approved: pending
