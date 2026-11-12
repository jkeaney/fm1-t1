<!-- SPDX-License-Identifier: GPL-3.0-only -->
# P4-011: ui_sq.c framework (screen zones, toasts, labels, page dispatch): use case

Process: docs/project/UX_GATE.md. Gate A fills 1-6 before the build; Gate B fills 7-9 after it.
Task: backlog/tasks/P4-011-ui-sq-c-framework-screen-zones-toasts-labels-and-p.md   Spec: docs/design/SCREEN_SPEC.md (2, 2.7, 3, 4, 5)   Scenario: S01 steps 1-9 (first screen), S02 steps 1-19 (perform), S04 (channels)

Gate A is **cited from the locked design**: the zones, labels and the toast are fixed by SCREEN_SPEC and the generated tables; this task draws them and changes none. The framework is not wired into the device screen (P4-012 does the cut-over), so a producer sees it only after that; the steps below are what Gate B walks on the host render and, after the cut-over, on the FM-1.

## 1. Who and when
A producer with a TB-3 on MIDI channel 2 looks at the main screen while the bass line plays and wants to read, without a menu, what the device is doing (transport, tempo, channel, algorithm, MIDI channel, page, length, the playing step) and what each knob and button does right now.

## 2. Steps a producer takes
| # | Producer does (control id, context) | Screen shows after | Sound / MIDI after |
| --- | --- | --- | --- |
| 1 | Power on (PERF) | Z0 `. INT 120 P01 1 ACID M1 [1]. . . L16`; Z1 four cells, CH1 inverted, CH2..4 dim `MUTE n`; Z2 lane of 16 steps; Z3 `ROW 1` live `DENS SLIDE JUMP ACCNT` over dim ROW 2; Z4 `ALL NOTES ACCNT SHAPE UNDO SET` / `HOME SAVE ARTIC HITS PLAY REC` with the hold marks; Z5 legend; Z6 `PLAY: start   K1-K4: morph` | none |
| 2 | Press PLAY (B_PLAY) | H1 `>`, playhead underline moves, Z6 `TAP STEP=ON/OFF  HOLD=EDIT NOTE` | the line plays |
| 3 | Turn K1 (EN_K1) | the K1 value changes in the same frame, in T_ACCENT while hot | the pattern morphs at the next step |
| 4 | Turn ALGO (EN_ALGO) | H4 and the CH1 cell show the new 4-letter name; Z6 lists `ACID EUCL SNAK STAG [ARP] PATT` for 2 s; Z3 labels change | the new algorithm at the next step |
| 5 | Any control that answers with a message (for example SAVE) | Z6 shows the toast on T_TINT in T_ACCENT for 2 s, then the cue returns | none |

## 3. Functions and class
| Function | Class (Performance or Setup) |
| --- | --- |
| Read device state and control roles on the main screen (Z0..Z6) | Performance (always on, no gesture) |
| The toast and the algorithm-pick strip | Performance (feedback of other gestures) |

## 4. Predicted ease score
| Function | V | I | O | L | /12 | Why these numbers | Pass? |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Read state and control roles | 3 | 3 | 3 | 3 | 12 | every control has a label slot on every page (SCREEN_SPEC 3); no gesture is needed to read it; labels come from the tables so they cannot drift from the control map; a label that does not fit is a test failure | yes |
| Toast and algo strip | 3 | 3 | 3 | 2 | 11 | distinct colours (T_ACCENT on T_TINT) tell a message from a hint; 2 s then the cue returns; L is 2 because a first-time producer must learn that the strip is where messages land | yes |
Pass: performance >= 11 and no sub-score < 2.

## 5. Sync check (row or section, added or changed in this task)
| Surface | Row / section | Agrees? |
| --- | --- | --- |
| Buttons and knobs (control_map, button_modes, knob_map, key_map) | read through build/gen/ui_tables.h only; no row changed | yes, with the findings of FB-016 (legend `MUTE1`, tag `ROW 1` vs the spec text) |
| UI / screen (SCREEN_SPEC, page_states, widths) | SCREEN_SPEC 2 (zones), 2.1 (H slots), 2.2, 2.3, 2.4, 2.5, 2.6, 2.7, 4, 5; `tests/design/widths.tsv` (297 rows) measured by tests/ui_sq_test.c | yes; deviations listed in FB-016 (tag wrap, Z6 text 3 px up, cue copy) |
| UX / help (help_text, UX_REVIEW) | untouched (HELP overlay is a stub page) | n/a |
| Workflow (scenario step) | S01 step 1-9, S02 (screen text of the main page) | partly: the clock dot is a placeholder (INT), H2 states are P4-012 |

## 6. Live checklist (yes / no + reason / n/a)
- [x] Reachable from the main screen, no menu: it is the main screen.
- [x] Undoable in one press or self-reverting: n/a, display only; the toast and the algo strip expire by themselves.
- [x] Lands on a musical boundary, no hung or cut notes: n/a, the framework sends no MIDI and writes no sq_* state.
- [x] Screen shows the value within one frame; sound changes by the next step: yes, redraw by signature draws a changed zone in the next frame (test: a mute redraws blits A and C, a morph blit B, a toast blit C).
- [x] One-handed: n/a, display only.
- [x] Does not change another channel (or says so on screen): yes, read-only.
- [x] Survives stop/start, clock-source change, preset load: it reads state each frame after SQ_FRAME_SYNC(), so it follows a load; the clock-source states of H2 are not drawn yet (P4-012).
- [x] Cost recorded: build line `RAM .data+.bss 23828 B of 73728` (headroom 49900 B); ui_sq.c is not in the device build yet, its static state is about 100 B and it adds no pool use (the canvas cv_px is shared).

Design locked: cited from SCREEN_SPEC (P2-003, P2-011) and ADR-0004 locks 2026-11-12 (Gate A by the executor from the locked design; the reviewer confirms)

## 7. Walk-through (after the build)
Gate B: the reviewer fills this (host render in build/ui_sq/*.png, tests/ui_sq_test.c).

## 8. Actual ease score
| Function | V | I | O | L | /12 | Predicted | Delta | Pass? |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |

## 9. Sync re-check and outcome
- check_scenarios.py / check_help.py / host UI tests:
- Live checklist re-answered from observation:
- Outcome:
- Approved:
