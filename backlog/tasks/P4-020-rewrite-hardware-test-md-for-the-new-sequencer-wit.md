---
id: P4-020
title: Rewrite HARDWARE_TEST.md for the new sequencer with TB-3 checklists
phase: P4
type: task
status: todo
priority: 2
executor: haiku
depends_on: [P4-018]
gate: none
---
## Goal
The owner's hardware sessions need a checklist that matches the new UI (the current one names the T-1 menu).

## Read first
- docs/project/HARDWARE_TEST.md (current)
- docs/design/SEQUENCER_UX.md §7 (workflows), §11.4 (first minute)
- docs/design/scenarios/S01 (target workflow)
- docs/design/TIMING_SPEC.md §6.4
- docs/design/SEQUENCER_UX.md §6.6 (HT-FORCE)

## Deliverable
- HARDWARE_TEST.md: setup (Ableton route and host box), tools, and one checklist per session: HW-1 core (boot, first run, CH1 to MIDI 2, ACID, steps, PLAY/STOP, CC123, preview), HW-T (pointer to TIMING_SPEC G1-G7 with the commands), HW-2 algorithms (each of six + HARMONY 1-4-5-1; steps, notes, slides, accents), HW-3 performance (morph each knob, each dice layer, ALL, undo, mute/solo/fill/roll, no menu), HT-FORCE (pointer to 6.6), HW-4 storage (save, power-cycle, load, empty slot toast), release acceptance
- Each item is a checkbox with the expected sound or screen text

## Acceptance criteria
- [ ] Every session ID used in backlog hardware tasks (HW-1, HW-T, HW-2, HW-3, HT-FORCE, HW-4, release) has a checklist section
- [ ] No mention of `T1 SEQ` or the T-1 screen remains

## Verification
```sh
grep -n 'T1 SEQ' docs/project/HARDWARE_TEST.md   # no output
python3 tools/backlog.py check
```

## Out of scope
- Code
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
