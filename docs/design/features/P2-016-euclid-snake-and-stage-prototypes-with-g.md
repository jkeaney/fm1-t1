<!-- SPDX-License-Identifier: GPL-3.0-only -->
# P2-016: EUCLID, SNAKE and STAGE: use case

Process: docs/project/UX_GATE.md. Gate A fills 1-6 before the build; Gate B fills 7-9 after it.
Task: backlog/tasks/P2-016-euclid-snake-and-stage-prototypes-with-golden-outp.md   Spec: docs/design/algorithms/euclid/SPEC.md, snake/SPEC.md, stage/SPEC.md   Scenario: S04 steps 1-11 (EUCLID, SNAKE), 17-19 (STAGE); S02 for the ACID reference

This task writes the algorithms' behaviour as prototype code and golden output. What a producer will touch is already locked in the control tables; the prototypes make the *sound* behind those controls real and testable.
Nothing here adds or moves a control, label or screen row (the tables already carry the EUCLID, SNAKE and STAGE rows), so this use case records the walk of the existing rows against the prototypes' behaviour.

## 1. Who and when
A producer with a Roland TB-3 on channel 1 has an ACID line running and wants a different feel (evenly spread hits, a drifting polymeter or a stage loop that ratchets and holds) without stopping, then to perform it with the knobs and dice buttons.

## 2. Steps a producer takes
| # | Producer does (control id, context) | Screen shows after | Sound / MIDI after |
| --- | --- | --- | --- |
| 1 | Turn ALGORITHM one detent (EN_ALGO / PERF) from ACID to EUCLID | HD `1 EUCL M2`; K1R `HITS 5` `OFFSET 0` `RATCH 4` `ACCNT 2`; K2R `LEN 16` `SPAN 12` `PUNCH 40` `CURVE AUTO`; dice sub-labels `NOTES` `ROTATE` `ACCNT` `RATCH`, `SHAPE -` dim | from the next step: 5 hits spread over 16 steps (steps 0 3 6 9 12), contour notes in the scale, 2 accents |
| 2 | Turn K1 (HITS, EN_K1 / PERF) up | cell value changes in the same frame | next step: more hits, still evenly spread; turning back restores the exact pattern |
| 3 | Press SEQ (B_SEQ, HITS dice, sub-label ROTATE) | toast `HITS` / `ROTATE`, cue `EDIT takes it back` | next step: the same rhythm starting on another step, notes moved with it |
| 4 | Press LFO (B_LFO, SHAPE, absent in EUCLID) | toast `NO SHAPE IN EUCLID`; no undo entry | none |
| 5 | Turn ALGORITHM one detent to SNAKE; turn K4 (PATH) | K1R `DENS 10` `LEN 7` `RANGE 12` `PATH 7`; ARTIC dim `ARTIC -` (press ARP: toast `NO ARTIC IN SNAKE`) | next step: a different traversal re-orders pitch, rhythm and velocity together |
| 6 | Turn ALGORITHM to STAGE; turn K2 (TOTAL) | K1R `DENS 6` `TOTAL 16` `DIR FWD` `RATCH 2` | next pulse: one stage one pulse longer per detent; held notes and ratchets keep their stage |

Every step is one gesture on the main screen (PERF), no menu, no page change. Setup functions (choose the algorithm) are one gesture; performance functions (morph, dice) are one gesture.

## 3. Functions and class
| Function | Class (Performance or Setup) |
| --- | --- |
| Pick EUCLID, SNAKE or STAGE (UX_REVIEW F03) | Performance (listed P in UX_REVIEW) |
| Morph live: HITS/LEN/SPAN/ACCNT/OFFSET/RATCH/PUNCH/CURVE, DENS/LEN/RANGE/PATH/OFFSET/GLEN/DYN/DIR, DENS/TOTAL/DIR/RATCH/OFFSET/RDIV/DYN/RANGE (F08) | Performance |
| Dice a present layer (F09-F12, F13 for STAGE LONG, SNAKE DRIFT) | Performance |
| Press a dice button whose layer is absent (EUCLID SHAPE, SNAKE ARTIC) | Performance (must be a clean no-op) |

## 4. Predicted ease score
| Function | V | I | O | L | /12 | Why these numbers | Pass? |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Pick algorithm | 3 | 3 | 3 | 3 | 12 | The existing UX_REVIEW F03 score is 3/3/3 (9 on its three-part scale): a labelled encoder, one detent, the name and new knob labels appear in the same frame. The prototypes add the sound that matches the labels: each new algorithm plays within a step (no empty state: DENS/HITS defaults are non-zero) | yes |
| Morph live | 3 | 3 | 3 | 3 | 12 | Every morph is a threshold or index on stored draws (golden file: min/mid/max per knob), so a knob move changes the sound by the next step and turning back restores it exactly. The label of each knob is the morph's own name | yes |
| Dice a layer | 3 | 3 | 3 | 2 | 11 | L is 2 because STAGE's `RT+HLD` and `LONG` sub-labels do not say what they change until heard. Each layer re-rolls one seed only (tables bind every seed to exactly one layer, checked by the table test); the sub-label tells which part of the pattern changes (`ROTATE`, `GATES`, `RESTS`) | yes |
| Absent layer press | 3 | 3 | 3 | 3 | 12 | Dim `SHAPE -` / `ARTIC -` label before the press, toast after, no seed or undo change (driver checks no seed moved) | yes |
Pass: performance >= 11 and no sub-score < 2; setup >= 9 and no 0. The scores are on the four-part /12 scale of UX_GATE; UX_REVIEW's 3-part scores are cited for the controls that already exist.
L is the weakest guess, for STAGE: the hold/ratchet modes are explained only by their sub-labels (`RT+HLD`, `LONG`), so a first-time producer may need to hear it. 11/12 still passes; Gate B should confirm on the device.

## 5. Sync check (row or section, added or changed in this task)
| Surface | Row / section | Agrees? |
| --- | --- | --- |
| Buttons and knobs (control_map, button_modes, knob_map, key_map) | `knob_map.tsv` rows PERF/EUCLID K1-S4, PERF/SNAKE K1-S4, PERF/STAGE K1-S4; `control_map.tsv` EN_ALGO, B_FX/B_SCL/B_SEQ/B_ENV/B_ARP/B_LFO PERF. Prototype `knob_morph` tables: EUCLID `{0,4,5,3,1,2,6,7}`, SNAKE identity, STAGE `{0,1,7,3,4,5,6,2}` | yes: no row changed; the `knob_morph` tables were derived from `knob_map.tsv` (K2 OFFSET, K3 RATCH, S1 LEN, S2 SPAN for EUCLID; K3 DIR, S4 RANGE for STAGE) |
| UI / screen (SCREEN_SPEC, page_states, widths) | `SCREEN_SPEC.md` (toasts `NO SHAPE IN EUCLID`, `NO ARTIC IN SNAKE`), K1R/K2R labels (<= 6 chars; `OFFSET` is the longest), dice sub-labels `ROTATE` (6), `RT+HLD` (6) | yes: the table test in `proto_main.c` enforces <= 6 characters and `-` for an absent layer; no widths added |
| UX / help (help_text, UX_REVIEW) | `help_text.tsv` unchanged; UX_REVIEW F03, F08-F13 | yes: `check_help.py` OK (392 rows) |
| Workflow (scenario step) | S04 steps 1-11 (EUCLID, SNAKE), 17-19 (STAGE) | yes: `check_scenarios.py` shows 0 control-map rows with no scenario |

Deviations that touch a surface and are for the reviewer / P2-029: ADR-0003 section 20 `hi(m)` is too low for all three (EUCLID `SPAN + 3`, SNAKE and STAGE `RANGE + 1`; see each SPEC section 7); ALGORITHM_SET 5.2 and FULL_CONTROL 3.2/3.4 still show the pre-re-lock knob order.

## 6. Live checklist (yes / no + reason / n/a)
- [x] Reachable from the main screen, no menu: yes, all on PERF (EN_ALGO, K1-K4 / S1-S4, dice buttons).
- [x] Undoable in one press or self-reverting: yes. A morph reverses by turning back (pure threshold/index on stored draws, golden-checked min/mid/max); a dice undoes with EDIT tap (A3 4.1).
- [x] Lands on a musical boundary, no hung or cut notes: yes. Changes land at the next step (STAGE: next pulse); a STAGE `I-` hold in progress finishes (`PO_HOLD` carries its length in pulses; the core ends it one control block before the next note-on).
- [x] Screen shows the value within one frame; sound changes by the next step: yes by design (the cache is rebuilt in the main-loop frame; build cost is under ~1,500 integer ops, see each assessment 6.3a); to be measured at Gate B.
- [x] One-handed: yes, each is a single knob turn or button press.
- [x] Does not change another channel (or says so on screen): yes; the cache and seeds are per channel and per algorithm (A3 3.1, S7).
- [x] Survives stop/start, clock-source change, preset load: yes. `step()` is a pure function of `(cache, g)` with no stored phase (purity test: call order does not matter), so stop/start and a clock change cannot desynchronise it; seeds and morphs are in the preset.
- [x] Cost recorded (RAM headroom >= 8 KB): caches EUCLID 30 B, SNAKE 55 B, STAGE 43 B (each <= 64, ACID 54 B); each is within the 64 B cache limit (`SQ_CACHE_MAX`, enforced by the table test), and the memory budget is ADR-0003 section 11. No firmware change in this task, so RAM headroom is unchanged.

Design locked: Sonnet reviewer 2026-11-12 (Gate A; predictions consistent with the locked tables; the SHAPE-absent premise is superseded by P2-022, see section 9)

## 7. Walk-through (after the build)
Prototype task, no firmware: Gate B proper waits for P5. Reviewer walked the steps against the prototypes on 2026-11-12 (`sh tests/design/run_protos.sh`: PROTOS PASS, four algorithms).
Steps 1-3, 5, 6: golden.txt carries min/mid/max for every knob and two diced seeds per present layer; E(5,16) = steps 0 3 6 9 12 (euclid_ref.py, 51 cases); turning a morph back restores the output (purity test). Step 4 (absent layer): `dice SHAPE absent (no change)` in euclid golden and ARTIC in snake golden, the driver checks no seed moved. Screen text and toasts are not rendered by a prototype (not walked).
Difference from prediction: step 4 and the "SHAPE -" label are superseded by P2-022 (EUCLID SHAPE is now MOD, a second seed). The prototypes still model the pre-P2-022 layer set.

## 8. Actual ease score
| Function | V | I | O | L | /12 | Predicted | Delta | Pass? |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Pick / morph / dice / absent press | n/a | n/a | n/a | n/a | n/a | 12 / 12 / 11 / 12 | n/a | Sound side supports the predictions; V/I/O/L need firmware and device (Gate B at P5) |

## 9. Sync re-check and outcome
- check_scenarios.py / check_help.py: not re-run by the reviewer (no control table changed); backlog check 133 items, 0 errors.
- Live checklist: purity, cache <= 64 B and next-step landing are supported by the tests; screen latency and RAM remain for Gate B.
- Outcome: Pass for the prototype task, with debt: the absent-layer premise (EUCLID SHAPE, SNAKE ARTIC) needs the P2-022 MOD seed added (follow-up, see task Log).
- Approved: n/a (gate none)
