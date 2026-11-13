---
id: P4-020
title: Rewrite HARDWARE_TEST.md for the new sequencer with TB-3 checklists
phase: P4
type: task
status: review
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
- [x] Every session ID used in backlog hardware tasks (HW-1, HW-T, HW-2, HW-3, HT-FORCE, HW-4, release) has a checklist section
- [x] No mention of `T1 SEQ` or the T-1 screen remains

## Verification
```sh
grep -n 'T1 SEQ' docs/project/HARDWARE_TEST.md   # no output
python3 tools/backlog.py check
```

## UX gate
n/a, internal work (tooling, plumbing, docs, process); no producer-visible control, screen or sound change.

## Out of scope
- Code
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log

### 2026-11-13 executor (Sonnet 5.5)
- Rewrote docs/project/HARDWARE_TEST.md: Ableton/bridge/host-box setup, flash and restore, sessions HW-1 (A to I), HW-T (fm1_timing.py commands), HW-2, HW-3, HT-FORCE, HW-4, Release; "Known gaps at HW-1"; "Things to judge by eye". Smoke checklist removed.
- `grep -n 'T1 SEQ' docs/project/HARDWARE_TEST.md` -> no output; also no `T-1 screen`. `python3 tools/backlog.py check` -> 166 items, 0 errors.
- Docs sync: no spec deviates; nothing else edited (docs only, no build run).
- Could not verify (written from the specs and task Logs, not from the unit): Ableton port/channel settings (assumed, not tried); the ALGO encoder staying on ACID with only ACID filled; whether a NOTE_EDIT K1 detent sounds (knob_map says yes, P4-014 Log says debt); that keys past LENGTH are dimmed; SAVE tap/hold wording in HW-4 (P7-002 not built); `fm1_timing.py --follow --host-port "IAC"` port name; HW-T/HW-2/3/4/Release items are outlines from the task Deliverables and need refinement when those tasks start. fm1_bridge.py could not be run (no mido in system python).
