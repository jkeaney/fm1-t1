<!-- SPDX-License-Identifier: GPL-3.0-only -->
# P4-015: CHAN page: MIDI channel, algorithm, length: use case

Process: docs/project/UX_GATE.md. Gate A fills 1-6 before the build; Gate B fills 7-9 after it.
Task: backlog/tasks/P4-015-chan-page-midi-channel-algorithm-length.md   Spec: SCREEN_SPEC_V2 (CHAN band, 4.2, 5, 5.6, 6), SEQUENCER_UX 1.6, 2.3, 4.4   Scenario: S01 steps 2-4, S11

## 1. Who and when
A producer with a TB-3 on MIDI channel 2 wants channel 1 to send on 2, pick an algorithm and set how long the pattern is, without a menu.

## 2. Steps a producer takes
| # | Producer does (control id, context) | Screen shows after | Sound / MIDI after |
| --- | --- | --- | --- |
| 0 | Power on (P4-044) | only CH1 is in use and selected; CH2..CH4 are out of use until added (SELECT past the last, only if their algorithm row exists) | CH1 audible (ACID), the others silent |
| 1 | Tap D#4 (KEY_B05_D#4, PERF) | CHAN band `LENGTH MIDI SOURCE FOLLOW`, message slot `CHN CH1 SETUP`, footer `K2 = MIDI` / `CHN = BACK` | none |
| 2 | Turn K2 (EN_K2) | the MIDI tile and the header `MIDI n` show the new number at once | none yet: the value is pending |
| 3 | Let K2 rest 0.6 s, or leave (HOME, D#4, PLAY, GLO, REC, SAVE, SELECT, any button) | toast `[k] MIDI OFF  n TO CH1` when another channel held n (that channel is OFF); no toast otherwise | notes now go out on the new MIDI channel; the displaced channel sends nothing |
| 4 | Turn ALGO (EN_ALGO) | the strip lists the algorithms with the current one in brackets; slot `ALGO <short> AT STEP` | the new algorithm starts at the next step |
| 5 | Turn K1 (LENGTH) or press OCT- / OCT+ (-PAGE / +PAGE) | LENGTH tile, `PAGE v/n`; a new page is a copy of the page before | the pattern length changes at its next pass |
| 6 | Press EDIT after a MIDI steal | both channels have their old MIDI channels back; toast `UNDO  EDIT=REDO` | as before the steal |
| 7 | Tap HOME or D#4 | back to PERF | none |

## 3. Functions and class
| Function | Class (Performance or Setup) |
| --- | --- |
| MIDI channel (page open + turn) | Setup (2 gestures) |
| Algorithm (ALGO encoder) | Setup (1 gesture on CHAN) |
| LENGTH, +PAGE / -PAGE | Setup (2 gestures) |
| FOLLOW | Setup |
| MIDI steal undo (EDIT) | Performance (1) |

## 4. Predicted ease score
| Function | V | I | O | L | /12 | Why these numbers | Pass? |
| --- | --- | --- | --- | --- | --- | --- | --- |
| MIDI channel | 3 | 3 | 3 | 3 | 12 | the page names itself, the tile and header move at once, the footer says `K2 = MIDI`; the steal is a toast with its undo | yes |
| Algorithm | 2 | 3 | 3 | 3 | 11 | the strip lists the six; the screen keeps the old name until the step lands | yes |
| LENGTH / pages | 3 | 3 | 3 | 3 | 12 | the tile and `PAGE v/n` answer; the buttons are labelled -PAGE / +PAGE | yes |
Pass: performance >= 11 and no sub-score < 2; setup >= 9 and no 0.

## 5. Sync check (row or section, added or changed in this task)
| Surface | Row / section | Agrees? |
| --- | --- | --- |
| Buttons and knobs (control_map, button_modes, knob_map, key_map) | CHAN rows K1, K2, K4, OCT-, OCT+, ALGO, SELECT, dice, EDIT, HOME, D#4; K3 SOURCE and the `ONE PAGE  K1=LESS` (built; P4-040 changes the hint to K1=LEN) / `64 STEPS = MAX` toasts are not built (no MESSAGES row) | yes, with the debt |
| UI / screen (SCREEN_SPEC_V2, page_states, widths) | chan_page and chan_midi_clash scenes: tests/v12_compare.py 0 failed; the footer halves and the page title now come from page_states.tsv and ui_msgs | yes |
| UX / help (help_text, UX_REVIEW) | no new words | yes |
| Workflow (scenario step) | S01 steps 2-4 reproduce in tests/ui_sq_chan_test.c (the S01 toast wording is the v12 `MIDI OFF  n TO CHk`, which supersedes the older `CH2 IS MIDI OFF` line) | yes |

## 6. Live checklist (yes / no + reason / n/a)
- [x] Reachable from the main screen, no menu: D#4 tap
- [x] Undoable in one press or self-reverting: the steal by one EDIT (both channels); LENGTH by turning back; a page add by EDIT
- [x] Lands on a musical boundary, no hung or cut notes: the MIDI channel and LENGTH are read by the scheduler; sq_set_midi ends sounding notes on the byte they started with (decision 47); the algorithm lands at the next step
- [x] Screen shows the value within one frame; sound changes by the next step
- [x] One-handed
- [x] Does not change another channel: only the steal does, and the toast names the channel
- [x] Survives stop/start, clock-source change, preset load: a pending value is dropped if the channel left use; a preset that lands clears the records (the core)
- [x] Cost recorded: RAM .data+.bss 29224 B of 73728 (headroom 44504 B); 6 B of UI state

Design locked: SCREEN_SPEC_V2 (owner lock 2026-11-12); the behaviour follows SEQUENCER_UX 4.4.

## 7. Walk-through (after the build)
Method: host test tests/ui_sq_chan_test.c (it drives the handlers as ui_sq_glue.c does); frames in build/ui_sq/chan_*.png. Real unit: pending (HW-1).
| Step | Screen showed | MIDI |
| --- | --- | --- |
| 1 | CHAN band, title `CHN CH1 SETUP`, footer `K2 = MIDI` / `CHN = BACK` | none |
| 2 | K2 +1: tile 2, header `MIDI 2`, channel still MIDI 1 | none |
| 3 | 0.65 s: committed; CH2 OFF; toast `[2] MIDI OFF  2 TO CH1` (fits 136 px, also `16 TO CH1`) | sq_ch[0].midi_ch = 1 |
| 4 | ALGO +1: brackets move; `ALGO <name> AT STEP`; clamps at both ends | sq_set_algo |
| 5 | K1 clamps 1..64, the viewed page follows a shrink; hidden edits survive a shrink and regrow; OCT- 20 -> 16, OCT+ 20 -> 32 | sq_len_set / sq_page_add |
| 6 | EDIT: both channels restored, again redoes | UndoMidi |
| 7 | HOME or D#4: PERF | none |
Differences from the prediction: none found on the host. K3 SOURCE is drawn and inert.

## 8. Actual ease score
| Function | V | I | O | L | /12 | Predicted | Delta | Pass? |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| MIDI channel | 3 | 3 | 3 | 3 | 12 | 12 | 0 | yes (host; real unit HW-1) |
| Algorithm | 2 | 3 | 3 | 3 | 11 | 11 | 0 | yes (host) |
| LENGTH / pages | 3 | 3 | 3 | 3 | 12 | 12 | 0 | yes (host) |

## 9. Sync re-check and outcome
- `sh tools/check.sh`: CHECK PASS; tests/ui_sq_chan_test.c: 0 failures; v12_compare 44 scenes, 0 failed.
- Debt: K3 SOURCE (offered-block list); toasts `ONE PAGE  K1=LESS` (built; P4-040 changes the hint to K1=LEN) and `64 STEPS = MAX` (need MESSAGES rows in v12.py); GLO / REC / SAVE actions from CHAN (their pages' tasks); PERF ALGO encoder and the CHN hold = RIG (P4-032).
- Outcome: Pass with debt. Reviewer to confirm (Gate B).
