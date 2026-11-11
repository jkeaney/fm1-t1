<!-- SPDX-License-Identifier: GPL-3.0-only -->
# P2-017: ARP and PATTERNS prototypes with golden output; owner locks all six prototypes: use case

Process: docs/project/UX_GATE.md. Gate A fills 1-6 before the build; Gate B fills 7-9 after it.
Task: backlog/tasks/P2-017-arp-and-patterns-prototypes-with-golden-output-own.md   Spec: docs/design/algorithms/arp/SPEC.md (6.3 to 6.8, 7), docs/design/algorithms/patterns/SPEC.md (4, 5), docs/design/algorithms/LOCK.md   Scenario: S19 (ARP), S20 (PATTERNS), S04 steps 1 and 7 (EUCLID SHAPE), S02 for the ACID reference

This task writes ARP and PATTERNS as prototype code and golden output, adds the mod seed `s_mod` and the mod lanes to the other four prototypes (P2-016 review follow-up), and prepares the owner's lock of all six. The controls are those already locked in the control tables
(P2-013 for ARP, P2-014 for PATTERNS); what the producer *hears* behind them is what the prototypes fix. Gate A therefore re-walks the locked steps against the prototype behaviour and records the one control-visible change (EUCLID's SHAPE button is alive).

## 1. Who and when
A producer with a Roland TB-3 on channel 1 has an acid line running and wants an arpeggio of the chords of the progression, or a ready-made bass pattern of a style, without stopping the clock, and then to morph and dice it live.

## 2. Steps a producer takes
| # | Producer does (control id, context) | Screen shows after | Sound / MIDI after |
| --- | --- | --- | --- |
| 1 | Turn ALGO (EN_ALGO / PERF) four detents to ARP, one at a time | HD `1 ARP M2`, CHB cell `1 ARP`; for 2 s the CUE lists `ACID EUCL SNAK STAG [ARP] PATT`; K1R `DENS 12` `SHAPE LINE` `SPAN 2 OCT` `RATE 1X`; dice sub-labels `VARY RESTS VELO LENGTH PHASE` | next step: the key's triad as a 12-of-16 arpeggio, `36 - 43 48 51 55 36 39 - 48 51 55 36 39 - -` (golden `## ARP DEFAULT`) |
| 2 | Turn K2 (SHAPE) three detents (EN_K2 / PERF) | K1R `SHAPE OCTV`; toast `FIGURE 12: LENGTH 16 CUTS IT` | next step: each tone then its octave `36 - 39 51 43 55 48 60 - 63 55 67 36 48 - -` |
| 3 | Turn K4 (RATE) one detent (EN_K4 / PERF) | K1R `RATE 2X` | each sounding step plays two notes (`36+48`), the second half a step later |
| 4 | Hold OCT+, turn K4 (DIR), release (B_OCTUP, EN_K4 / PERF) | ROW 2 live while held, `DIR UPDN`; toast `FIGURE 10: ...` | the figure goes up then down |
| 5 | Press SCL (NOTES dice), then LFO (SHAPE dice) (B_SCL, B_LFO / PERF) | toasts `NOTES` / `VARY`, then `SHAPE` / `PHASE` (the LFO press also re-rolls `s_mod`) | octave hops; then a new start phase; the CC stream of any active device slot changes, the notes do not |
| 6 | Turn ALGO to PATT; tap OCT+, turn K1 (PICK) to `HSE 3` (EN_ALGO, B_OCTUP, EN_K1 / PERF) | HD `1 PATT M2`; K2R `PICK HSE 3` `SLIDE 4` `DYN 10` `CTONE 2`; toast `PATTERNS: HOUSE 3  LEN 16` | next step: the stored pattern, in the scale (`. 36 36 36 . 36 36 36 . 36 36 36 . 36 36 48`) |
| 7 | Turn K1 (DENS) down, K2 (VARY) up (EN_K1, EN_K2 / PERF) | K1R `DENS 5`, `VARY 4` | decoration thins, anchors stay; one or two decoration notes step to a neighbour tone |
| 8 | Press EDIT (B_EDIT / PERF) | toast `UNDO: ...` | back to the previous dice or pick |

Every step is on the main screen (PERF), no menu, no page change. Setup (choose the algorithm, pick a pattern): one or two gestures; performance (morph, dice, undo): one.

## 3. Functions and class
| Function | Class (Performance or Setup) |
| --- | --- |
| Choose ARP or PATT (EN_ALGO) | Performance (UX_REVIEW F03 lists it P) |
| Morph ARP / PATT: DENS SHAPE SPAN RATE, DENS VARY SPAN ACCNT (ROW 1 knobs) | Performance |
| ROW 2 of ARP / PATT: OFFSET LEGATO DYN DIR; SLIDE DYN CTONE (OCT+ held or latched) | Performance when held, Setup when latched (two gestures) |
| PICK a library pattern (OCT+ tap, K1) | Setup |
| Dice a layer, dice all, undo (B_SCL..B_LFO, B_FX, B_EDIT) | Performance |
| SHAPE press on EUCLID (now alive, `MOD`) | Performance |

## 4. Predicted ease score
| Function | V | I | O | L | /12 | Why these numbers | Pass? |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Choose the algorithm | 3 | 3 | 3 | 3 | 12 | ARP SPEC 7.2 and PATTERNS 5.6: a labelled encoder, one detent, the six-name strip and the new knob labels appear in the same frame; the prototype plays at once (the golden default sounds 12 of 16 steps) | yes |
| Morph ROW 1 (ARP: DENS SHAPE SPAN RATE; PATT: DENS VARY SPAN ACCNT) | 3 | 3 | 3 | 3 | 12 | every morph is a pure function of the stored draws (purity test, call-order independent); turning back restores the output exactly; the golden has min / mid / max for every morph | yes |
| ROW 2 held (OFFSET LEGATO DYN DIR; SLIDE DYN CTONE) | 3 | 2 | 3 | 2 | 10 | modifier plus knob (I2), the OCT+ role is named on screen (L2); the held form is one gesture, scored with the 11-point rule at the ARP SPEC 7.2 value | yes (setup 9; held performance form is 10, noted: below 11) |
| PICK a pattern | 3 | 2 | 3 | 2 | 10 | tap plus turn (PATTERNS 5.6), cell `PICK HSE 3`, header names it; setup rule 9 | yes |
| Dice a layer / dice all / undo | 3 | 3 | 3 | 2 | 11 | each layer re-rolls one seed (SHAPE also `s_mod`), the golden shows the other layers' notes unchanged for every dice case; L is 2 because a sub-label says what is re-rolled, not how it sounds | yes |
| EUCLID SHAPE (`MOD`) | 3 | 3 | 3 | 2 | 11 | the button was dim; it now re-rolls the mod lanes only (notes unchanged: golden `dice SHAPE MOD`). L 2: with no device slot active nothing audible changes, which the toast should say (P4-031 string) | yes |
Pass: performance >= 11 and no sub-score < 2; setup >= 9 and no 0. The ROW 2 held form scores 10, one under the performance rule; ARP SPEC 7.2 (F09) already accepts it because ROW 2 is a dial-in, class S when latched. The owner is asked to confirm that classification in the lock (LOCK.md 6, item 1).

## 5. Sync check (row or section, added or changed in this task)
| Surface | Row / section | Agrees? |
| --- | --- | --- |
| Buttons and knobs (control_map, button_modes, knob_map, key_map) | `knob_map.tsv` PERF ARP K1..S4 and PERF PATTERNS K1..S4 (P2-013 / P2-014), the prototype `knob_morph` tables are the identity for both; EUCLID PERF `B_LFO` becomes alive (sub-label `MOD`) | yes: no control row added or moved; the identity tables equal the ones in knob_map.tsv. The `B_LFO` / EUCLID wording in `button_modes.tsv` is unchanged (it is the shared dice role) |
| UI / screen (SCREEN_SPEC, page_states, widths) | `widths.tsv`: all picker, H4, cell and sub-label strings of ARP / PATT exist (P2-013, P2-014); this task adds `MOD` (EUCLID sub-label, 27 of 42 px) | yes: `width_probe.sh` 297 strings, 0 over. **Stale, not edited here (P2-029 / P4-031):** SCREEN_SPEC 2.7, UX_REVIEW 329 and ALGORITHM_SET 4.4 still list EUCLID SHAPE as absent (`SHAPE -`, toast `NO SHAPE IN EUCLID`) |
| UX / help (help_text, UX_REVIEW) | `help_text.tsv` is generated and unchanged (no label was renamed) | yes: `check_help.py` OK (392 rows) |
| Workflow (scenario step) | S19 (ARP) and S20 (PATTERNS) walk steps 1 to 8; S04 steps 1 and 7 rewritten this task (EUCLID SHAPE is `MOD`, not absent) | yes: `check_scenarios.py` 0 control-map rows with no scenario. S19 / S20 numbers were checked against the golden (SPEC 10.1 twelve values equal; 10.2 OCTV and UPDN rows equal) |

## 6. Live checklist (yes / no + reason / n/a)
- [x] Reachable from the main screen, no menu: yes, all on PERF (EN_ALGO, K1..K4, OCT+ held, dice buttons, EDIT).
- [x] Undoable in one press or self-reverting: yes. A morph reverses by turning back (pure functions); a dice or pick undoes with EDIT (one record per channel; `UndoRec` carries the other `s_mod`, 94 B).
- [x] Lands on a musical boundary, no hung or cut notes: yes. A change lands on the next step; ARP repeats are scheduled inside their step (`sub_note`); a PATT tie (`PO_TIE`) holds into the next sounding step; the stop path (CC123) is the core's.
- [x] Screen shows the value within one frame; sound changes by the next step: yes by design (the cache is rebuilt in the main-loop frame; PATT has no cache to rebuild). To be measured at Gate B on the device.
- [x] One-handed: yes, one knob turn or one button press; ROW 2 held needs OCT+ and a knob (two hands or the latch).
- [x] Does not change another channel (or says so on screen): yes; the cache, seeds, `s_mod` and morphs are per channel and per algorithm.
- [x] Survives stop/start, clock-source change, preset load: yes. `step()` is a pure function of `(cache, g, hc)` with no stored phase (call-order test); seeds, morphs and `s_mod` are in the preset (`SeqPreset` 1556 B + `ModSave`).
- [x] Cost recorded (RAM headroom >= 8 KB): yes, LOCK.md 2: `.bss` +168 B, headroom about 38.6 KB, pool 29808 B for 18 preset copies, flash 4472 B of pattern data.

Design locked: awaiting owner (task gate `human-review`; Sonnet Gate A prediction recorded 2026-11-12)

## 7. Walk-through (after the build)
Prototype task, no firmware: Gate B proper waits for P5 (the device). Executor walk on 2026-11-12 against `sh tests/design/run_protos.sh`: steps 1 to 4 and 7 are covered by the ARP golden (default, the 1728-case grid checked by the independent `arp_ref.py`), steps 5 and 6 by the dice cases and `PATTERNS-COMPACT` (equal to the Python prototype on 126 cases). Screen text and toasts are not rendered by a prototype (not walked).
Difference from prediction: none yet; the THUMB-with-a-drone quirk (LOCK.md 4 item 7) is a finding for the owner, not a score change.

## 8. Actual ease score
| Function | V | I | O | L | /12 | Predicted | Delta | Pass? |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| all | n/a | n/a | n/a | n/a | n/a | 12 / 12 / 10 / 10 / 11 / 11 | n/a | needs firmware and the device (Gate B at P5) |

## 9. Sync re-check and outcome
- check_scenarios.py / check_help.py / check_sequencer_ux.py / width_probe.sh: see the task Log.
- Live checklist re-answered from observation: pending the device.
- Outcome: pending (reviewer Gate B, owner design lock).
- Approved: <owner> <date>
