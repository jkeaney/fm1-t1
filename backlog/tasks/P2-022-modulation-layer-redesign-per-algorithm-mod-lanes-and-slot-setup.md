---
id: P2-022
title: Modulation layer redesign: per-algorithm mod lanes and 4-slot control setup
phase: P2
type: task
status: done
priority: 1
executor: sonnet
depends_on: []
gate: human-review
---
## Goal
Replace the RIG/ROW 3 "wire" bolt-on (P2-018..P2-021) with a modulation layer that is part of each algorithm's engine, per owner decision 107. The device parameters the player chooses are sequenced, diced and morphed like pitch and velocity.

## Read first
- docs/project/OWNER_QUESTIONS.md decisions 96-107 (107 is the brief)
- docs/design/hardware/ (HW_SUPPORT_HIGHLEVEL.md, HW_SUPPORT_LOWLEVEL.md section 9, TB3_PROFILE.md), tools/hwprof/tb3.tsv, docs/design/hardware/tb3-midi-guide.csv
- docs/snake* and algorithms/snake.md: how snakes drive pitch and velocity, and how a mod snake maps to another parameter
- docs/design/ALGORITHM_SET.md, SEQUENCER_UX.md, algorithms/*/SPEC.md, scenarios S21-S23
- docs/project/VISION.md, CLAUDE.md product rules

## Deliverable
Design docs only (no firmware): update or replace docs/design/hardware/*, algorithm SPECs and scenarios so that:
1. Settings model: GLOBAL settings and CHANNEL settings; each channel has a device; each device profile lists its controls in the device's real names.
2. Remote vs mod control: from the MIDI facts we hold (CC push, the device's own knobs keep working), state what we can and cannot do (we push values; no read-back; what happens when the user turns the device knob). Say what is verified and what is not.
3. Modulation generation: a mod gen beside pattern gen, note gen and quantise. Per-step modulation and LFO forms. Dice and morph semantics for it.
4. Per-algorithm mod lanes: for EVERY algorithm in the set, which mod lane(s) it has and how they map to a device parameter (snake mod-snake as the model). Table plus one worked example.
5. Setup UX: about 4 slots per channel; the control-setup page (page 3) where K1..K4 each pick the parameter for slots 1..4 by turning to select it by name; at most 2 gestures; screen shows what each knob does. Reconcile with decisions 97-101 (three knob rows, RIG page) and say clearly what is dropped or kept.
6. TB-3 alignment: default slot sets using the 11 verified CCs; scatter type (select) linked to scatter depth. Scatter zone count is not a priority (decision 107, 5B).
7. Budget (RAM/pool/preset) and a list of tasks this changes (P4-xxx etc).
8. Owner question sheet, lettered options with a recommendation.

## Acceptance criteria
- [x] All eight deliverables present
- [x] Every algorithm has mod lane(s) listed
- [x] Design checkers pass: tests/design/check_scenarios.py, check_help.py, tools/check_sequencer_ux.py, tests/design/width_probe.sh, backlog.py check
- [x] Decisions 97-101, 103-107 honoured or explicitly reopened in the owner sheet

## Verification
```sh
python3 tests/design/check_scenarios.py; python3 tests/design/check_help.py; python3 tools/check_sequencer_ux.py; sh tests/design/width_probe.sh; python3 tools/backlog.py check
```

## Out of scope
- Firmware changes. Do not touch P3-002, P3-014 or FB-003 files (another agent owns them). Do not run tools/check.sh.

## Log

### 2026-11-11 executor (sonnet)
Deliverable: new docs/design/MODULATION.md (sections 1-11 cover deliverables 1-8: settings model, remote vs mod control, mod gen + dice/morph, per-algorithm lanes table + worked ACID/TB-3 example, MOD page setup UX + reconciliation of 97-101, TB-3 default slots + scatter pair, budget + task list, owner sheet of 9 questions). SUPERSEDED banners on HW_SUPPORT_HIGHLEVEL/LOWLEVEL, TB3_PROFILE, S21-S23; change notes in ALGORITHM_SET.md, algorithms/snake.md, hardware/README.md; 18 measured strings added to tests/design/widths.tsv. No firmware, no control_map or scenario rows touched.
```
python3 tests/design/check_scenarios.py -> scenarios: 23 files, 490 control-map rows, 0 with no scenario
python3 tests/design/check_help.py -> OK: 392 help rows, 2695 strings measured
python3 tools/check_sequencer_ux.py -> OK
sh tests/design/width_probe.sh -> 286 strings, 0 over
python3 tools/backlog.py check -> 117 items, 0 errors
```
Reviewer: check the ROW 3 = MOD page gestures (K pick, hold OCT+ depth, hold OCT- level, ALGO = device) and the shared SHAPE+MOD dice (owner Q1/Q2). Decisions 97-101 reconciliation is MODULATION.md 5.3; Q5 and Q6 reopen decision 100's start state and VISION rule 3 wording.

### 2026-11-11 review (sonnet)
Verdict: PASS on all checkable criteria; gate is human-review, so left at `review`.
- Re-ran the five verification commands: scenarios 0 unmapped, help OK (392 rows), check_sequencer_ux OK, width_probe 286 strings 0 over, backlog check 0 errors.
- Scope: commit touches only design docs, widths.tsv and the task file; P3-002/P3-014/FB-003 untouched; no firmware.
- All six algorithms (ACID, EUCLID, SNAKE, STAGE, ARP, PATTERNS) have lane A and B rows in MODULATION.md section 4, plus worked ACID example (4.1).
- Decisions 97-101 reconciled row by row in 5.3 (kept/dropped stated); 102-107 honoured; 5B scatter placeholder kept; setup is K turn to pick by name, 2 gestures.
- Remote vs mod (section 2) tags [V-ear]/[V-doc]/[I]/[D] correctly: CC receive of the 11 is verified, knob-vs-CC overwrite and no read-back of TB-3 OUT are [I] until H-MOD1/3.
- Minor nit (not blocking): MODULATION.md section 1 table row 3 describes ENV-X as "lane B" use, but 6.1 puts ENV-X on slot 3 = lane A. Align the wording.
Awaiting owner: review MODULATION.md section 10 questions 1-9 (esp. Q1 shared SHAPE+MOD dice, Q2 DEPTH on hold OCT+, Q6 VISION rule 3 wording) and sign off in this log.

### 2026-11-11 orchestrator
Approved: owner 2026-11-11 (questions 1-9 all A; decision 108).
