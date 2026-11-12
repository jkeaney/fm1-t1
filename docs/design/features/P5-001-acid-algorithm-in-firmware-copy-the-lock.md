<!-- SPDX-License-Identifier: GPL-3.0-only -->
# P5-001: ACID algorithm in firmware (copy the locked prototype): use case

Process: docs/project/UX_GATE.md. Gate A fills 1-6 before the build; Gate B fills 7-9 after it.
Task: backlog/tasks/P5-001-acid-algorithm-in-firmware-copy-the-locked-prototy.md   Spec: docs/design/algorithms/acid/SPEC.md (1 to 4, 6)   Scenario: S02 steps 1 to 19 (S01 step 9 for the start state)

Gate A is **cited from the locked design (P2-017)**: the controls, labels and what the producer hears are fixed by the locked prototype (`acid/proto.c`, `acid/golden.txt`) and the scenario S02, and this task changes none of them. It puts the same algorithm into the firmware table `ALGOS[]` (row 0). The core and the UI that make it audible and dial-able are other tasks (P4-012 cut-over, P4-011 and later UI tasks), so this task has no visible effect on the device yet; the producer-facing steps below are what Gate B walks once those land (P5-009 on the TB-3).

## 1. Who and when
A producer with a Roland TB-3 on MIDI channel 2 has channel 1 on ACID, the clock running, and wants a busier, more varied acid line without stopping: morph with the knobs, re-roll one layer with a dice button, take it back with EDIT.

## 2. Steps a producer takes
From S02 (control ids as in `control_map.tsv`; every step is on the main screen, no menu, no page change):

| # | Producer does (control id, context) | Screen shows after | Sound / MIDI after |
| --- | --- | --- | --- |
| 1 | Turn EN_ALGO one detent back, then one forward (EN_ALGO / PERF) | HD `1 EUCL M2` then `1 ACID M2`; K1R `DENS SLIDE JUMP ACCNT` | the line returns exactly (deterministic), from the next step, no note cut |
| 2 | Turn K1 (DENS) four detents down (EN_K1 / PERF) | K1R `DENS 10` | 10 hits in 16 steps from the next step |
| 3 | Turn K3 (JUMP) four detents up (EN_K3 / PERF) | K1R `JUMP 9` | octave jumps appear in the `ch2 ON` notes |
| 4 | Turn K4 (ACCNT) two detents up (EN_K4 / PERF) | K1R `ACCNT 5` | more notes at velocity 100 or more (the TB-3 accent threshold) |
| 5 | Tap OCT+, turn K2 (PEDAL), K4 (DIR), K1 (OFFSET), K3 (PUNCH) (B_OCTUP, EN_K1..K4 / PERF) | ROW 2 live: `PEDAL 3`, `DIR BWD`, `OFFSET 3`, `PUNCH 41` | fewer root repeats; reverse order; rotation by 3; accent velocity 80 + PUNCH |
| 6 | Press SEQ, SCL, ENV, ARP, LFO (HITS, NOTES, ACCNT, ARTIC, SHAPE dice) (B_SEQ, B_SCL, B_ENV, B_ARP, B_LFO / PERF) | toast per layer, sub-labels `HITS NOTES ACCNT GT+SLD OCTAVE` | exactly that layer changes at the next step; the other layers keep their notes |
| 7 | Press FX (dice all), then EDIT twice (B_FX, B_EDIT / PERF) | toasts `ALL`, `UNDO`, `REDO` | a new line; the previous line; the new line again |

## 3. Functions and class
| Function | Class (Performance or Setup) |
| --- | --- |
| Choose ACID (EN_ALGO) | Performance (UX_REVIEW F03 lists it P) |
| Morph ROW 1: DENS SLIDE JUMP ACCNT (K1..K4) | Performance |
| Morph ROW 2: PEDAL OFFSET PUNCH DIR (OCT+ latch or held) | Performance when held, Setup when latched (two gestures) |
| Dice a layer (SEQ SCL ENV ARP LFO), dice all (FX), undo and redo (EDIT) | Performance |

## 4. Predicted ease score
Cited from the locked design: the same functions are scored in S02 / UX_REVIEW and re-scored in the P2-017 use case (sections 4 there); nothing about them changes here.

| Function | V | I | O | L | /12 | Why these numbers | Pass? |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Choose the algorithm | 3 | 3 | 3 | 3 | 12 | labelled encoder, one detent, knob labels and header change in the same frame; the line plays at once (default DENS 12 of 16) | yes |
| Morph ROW 1 | 3 | 3 | 3 | 3 | 12 | every ACID morph is a threshold or an index on stored draws (SPEC 1), so turning back restores the exact pattern; golden has min, mid and max for all eight | yes |
| Morph ROW 2 (held) | 3 | 2 | 3 | 2 | 10 | modifier plus knob; the OCT+ role is named on screen; same value as P2-017 (class S when latched, so the setup rule 9 applies; 10 for the held form is the accepted P2-017 reading) | yes (as P2-017) |
| Dice a layer, all, undo | 3 | 3 | 3 | 2 | 11 | each button re-rolls one seed group (SPEC 2); the golden shows the other layers unchanged; L 2 because a sub-label names what is re-rolled, not how it sounds | yes |

## 5. Sync check (row or section, added or changed in this task)
| Surface | Row / section | Agrees? |
| --- | --- | --- |
| Buttons and knobs (control_map, button_modes, knob_map, key_map) | none added or changed: `knob_map.tsv` PERF ACID K1..S4 equals `AlgoDesc.knob_morph = {0,5,2,3,4,1,6,7}` of the firmware copy | yes: the descriptor block is the one in SPEC.md section 5, line for line (a run_tests step diffs it; only a trailing backslash per line, the table row being a macro) |
| UI / screen (SCREEN_SPEC, page_states, widths) | none: labels `DENS PEDAL JUMP ACCNT OFFSET SLIDE PUNCH DIR` and sub-labels already in `widths.tsv` | yes (P2-016 / P2-017 `width_probe.sh`) |
| UX / help (help_text, UX_REVIEW) | none changed | yes |
| Workflow (scenario step) | S02 steps 1 to 19 walk this use case | yes |

## 6. Live checklist (yes / no + reason / n/a)
- [x] Reachable from the main screen, no menu: yes, all on PERF.
- [x] Undoable in one press or self-reverting: yes. A morph reverses by turning back; a dice undoes with EDIT (core, P4-005).
- [x] Lands on a musical boundary, no hung or cut notes: yes by design. The cache is double-buffered and swapped at a step; `step()` is pure (purity tests (a)-(g) pass in `tests/sq_algo_acid_test.c`); CC123 on stop is the core's.
- [x] Screen shows the value within one frame; sound changes by the next step: by design (cache rebuilt in the main-loop frame). To be measured at Gate B on the device.
- [x] One-handed: yes for ROW 1 and the dice; ROW 2 held needs OCT+ and a knob (or the latch).
- [x] Does not change another channel (or says so on screen): yes; cache, seeds and morphs are per channel and per algorithm.
- [x] Survives stop/start, clock-source change, preset load: yes. `step()` keeps no phase (call-order test); seeds and morphs are in the preset.
- [x] Cost recorded (RAM headroom >= 8 KB): the build line is in the task log (the descriptor and the code are const / flash; the cache is the core's 64 B per buffer).

Design locked: cited from the locked design (P2-017), owner lock of all six prototypes 2026-11-12

## 7. Walk-through (after the build)
Pending Gate B (reviewer). This task is firmware-table only: the host test `tests/sq_algo_acid_test.c` reproduces the plain golden matrix (6890 lines) byte for byte, which fixes what is heard. Screen text and MIDI need the core and UI (P4-011, P4-012) and the device (P5-009).

## 8. Actual ease score
| Function | V | I | O | L | /12 | Predicted | Delta | Pass? |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| all | n/a | n/a | n/a | n/a | n/a | 12 / 12 / 10 / 11 | n/a | needs the UI and the device (Gate B at P5-009) |

## 9. Sync re-check and outcome
- check_scenarios.py / check_help.py / host UI tests: pending the reviewer.
- Live checklist re-answered from observation: pending the device.
- Outcome: pending.
- Approved: pending
