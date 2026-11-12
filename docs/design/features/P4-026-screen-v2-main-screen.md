<!-- SPDX-License-Identifier: GPL-3.0-only -->
# P4-026..P4-030: screen v2 (main screen, CHAN band, messages): use case

Process: docs/project/UX_GATE.md. Gate A filled at design lock; Gate B after the build (P4-021 HW-1 on the real unit).
Tasks: backlog/tasks/P4-026..P4-030   Spec: docs/design/SCREEN_SPEC_V2.md   ADRs: 0006, 0007
Renders: docs/design/ui_review/proposals/v12/out/ (scene names below)

## 1. Who and when
Dana, one TB-3 on MIDI 2, unit on the desk at 30-40 cm, hands on the knobs: wants a sequence playing in seconds and to
perform it without reading a manual.

## 2. Steps a producer takes
| # | Producer does (control id, context) | Screen shows after | Sound / MIDI after |
| --- | --- | --- | --- |
| 1 | power on, first run | CH1 chip, `+`, footer cue `CHN KEY, K2 = MIDI  CH1`, CHN key lit (first_run) | nothing |
| 2 | KEY_B05 tap (CHN) | message slot `CHN CH1 SETUP`; band LENGTH MIDI SOURCE FOLLOW; footer `K2 = MIDI / CHN = BACK` (chan_page) | nothing |
| 3 | EN_K2 turn to 2 | MIDI tile `2`; header `MIDI 2`; cue clears | CH1 now sends on MIDI 2 |
| 4 | EN_ALGO turn | `ALGO ACID AT STEP`; header and tiles change when it lands (turn_algo) | the new algorithm from the next step |
| 5 | EN_K1..K4 turn | the tile frames itself, white ticks = old level; bars reshape (turning_k2) | the morph from the next step |
| 6 | B_SCL tap (dice NOTES) | SCL tile inverted; `[1] NOTES  EDIT=UNDO`; changed bars hatched (dice) | new notes from the next step |
| 7 | B_PLAY tap | play triangle; playhead column + pointer; playing key bar (playing) | MIDI out to the TB-3 |
| 8 | KEY_B01 tap (M1) | chip struck, bars hollow, X on the key (sel_muted) | CH1 silent |
| 9 | EN_SEL turn | `SELECT CH2 EUCLID`; chips, tiles, lane follow; past the last: `+` becomes a new channel (turn_select, channel_added) | none |
| 10 | B_EDIT tap | `[1] UNDO  EDIT=REDO` (undo) | the previous layer from the next step |
| 11 | EN_PRE turn | `PRESETS P02 AT BAR`; inverted `P02?` (turn_presets) | P02 at the next bar |
| 12 | KEY_B04 long press 1.5 s | `REMOVE CH4` hint with its bar, then `[4] GONE  EDIT=UNDO` (hold_mute_remove, removed) | CH4 stops |

## 3. Functions and class
| Function | Class (Performance or Setup) |
| --- | --- |
| set MIDI channel, add/remove channel | Setup |
| algorithm, morph, dice, mute, switch channel, undo, preset, play | Performance |

## 4. Predicted ease score
| Function | V | I | O | L | /12 | Why these numbers | Pass? |
| --- | --- | --- | --- | --- | --- | --- | --- |
| set MIDI channel | 3 | 2 | 3 | 3 | 11 | standing cue + lit key; 2 gestures | yes |
| add channel | 2 | 2 | 3 | 3 | 10 | `+` chip; named while turning | yes |
| remove channel | 2 | 2 | 3 | 3 | 10 | hint while held; undoable | yes |
| morph (K1-K4) | 3 | 3 | 3 | 3 | 12 | tile = meter, frames itself | yes |
| dice | 3 | 2 | 3 | 3 | 11 | die icon + toast + hatch; undo hint | yes |
| mute | 3 | 3 | 3 | 3 | 12 | chip, bars, key all change shape | yes |
| switch channel / algorithm / preset | 3 | 2 | 3 | 3 | 11 | named while touched; pending shown | yes |
| undo | 3 | 3 | 3 | 3 | 12 | EDIT icon; toast names undo/redo | yes |
Pass: performance >= 11 and no sub-score < 2; setup >= 9 and no 0.

## 5. Sync check (row or section, added or changed in this task)
| Surface | Row / section | Agrees? |
| --- | --- | --- |
| Buttons and knobs (control_map, button_modes, knob_map, key_map) | SELECT adds a channel; M1-M4 hold: PREVIEW on release < 1.5 s, REMOVE at 1.5 s | changes in P4-030 |
| UI / screen (SCREEN_SPEC, page_states, widths) | SCREEN_SPEC_V2 replaces SCREEN_SPEC for PERF; page_states PERF/CHAN/REC/FIRST_RUN screen + cue columns; widths for MESSAGES | changes in P4-030 |
| UX / help (help_text, UX_REVIEW) | help_text rows for the message slot strings | changes in P4-030 |
| Workflow (scenario step) | S01 (first run) and the target workflow | re-walk in P4-030 |

## 6. Live checklist (yes / no + reason / n/a)
- [x] Reachable from the main screen, no menu
- [x] Undoable in one press or self-reverting (EDIT; remove is undoable)
- [x] Lands on a musical boundary (algorithm at the step, preset at the bar)
- [x] Screen shows the value within one frame; sound changes by the next step
- [x] One-handed (except holds, which are one finger)
- [x] Does not change another channel (or says so on screen: MIDI clash toast names it)
- [ ] Survives stop/start, clock-source change, preset load (verify in the build)
- [ ] Cost recorded (RAM headroom >= 8 KB) (P4-026)

Design locked: James Keaney 2026-11-12

## 7. Walk-through (after the build)
## 8. Actual ease score
## 9. Sync re-check and outcome
