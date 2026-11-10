---
id: P2-022
title: Modulation layer redesign: per-algorithm mod lanes and 4-slot control setup
phase: P2
type: task
status: todo
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
- [ ] All eight deliverables present
- [ ] Every algorithm has mod lane(s) listed
- [ ] Design checkers pass: tests/design/check_scenarios.py, check_help.py, tools/check_sequencer_ux.py, tests/design/width_probe.sh, backlog.py check
- [ ] Decisions 97-101, 103-107 honoured or explicitly reopened in the owner sheet

## Verification
```sh
python3 tests/design/check_scenarios.py; python3 tests/design/check_help.py; python3 tools/check_sequencer_ux.py; sh tests/design/width_probe.sh; python3 tools/backlog.py check
```

## Out of scope
- Firmware changes. Do not touch P3-002, P3-014 or FB-003 files (another agent owns them). Do not run tools/check.sh.

## Log
