<!-- SPDX-License-Identifier: GPL-3.0-only -->
# <TASK-ID>: <feature name>: use case

Process: docs/project/UX_GATE.md. Gate A fills 1-6 before the build; Gate B fills 7-9 after it.
Task: backlog/tasks/<file>.md   Spec: <doc section>   Scenario: <Sxx step ids>

## 1. Who and when
<One line: the producer's situation and what they want to happen.>

## 2. Steps a producer takes
| # | Producer does (control id, context) | Screen shows after | Sound / MIDI after |
| --- | --- | --- | --- |
| 1 | | | |

## 3. Functions and class
| Function | Class (Performance or Setup) |
| --- | --- |
| | |

## 4. Predicted ease score
| Function | V | I | O | L | /12 | Why these numbers | Pass? |
| --- | --- | --- | --- | --- | --- | --- | --- |
Pass: performance >= 11 and no sub-score < 2; setup >= 9 and no 0.

## 5. Sync check (row or section, added or changed in this task)
| Surface | Row / section | Agrees? |
| --- | --- | --- |
| Buttons and knobs (control_map, button_modes, knob_map, key_map) | | |
| UI / screen (SCREEN_SPEC, page_states, widths) | | |
| UX / help (help_text, UX_REVIEW) | | |
| Workflow (scenario step) | | |

## 6. Live checklist (yes / no + reason / n/a)
- [ ] Reachable from the main screen, no menu
- [ ] Undoable in one press or self-reverting
- [ ] Lands on a musical boundary, no hung or cut notes
- [ ] Screen shows the value within one frame; sound changes by the next step
- [ ] One-handed
- [ ] Does not change another channel (or says so on screen)
- [ ] Survives stop/start, clock-source change, preset load
- [ ] Cost recorded (RAM headroom >= 8 KB)

Design locked: <name> <date>

## 7. Walk-through (after the build)
Method (host test / real FM-1), firmware SHA, and per step: what the screen showed, what MIDI left.

## 8. Actual ease score
| Function | V | I | O | L | /12 | Predicted | Delta | Pass? |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |

## 9. Sync re-check and outcome
- check_scenarios.py / check_help.py / host UI tests: <result>
- Live checklist re-answered from observation: <result>
- Outcome: Pass | Pass with debt (FB-nnn) | Fail (<function, sub-score>)
- Approved: <owner> <date>
