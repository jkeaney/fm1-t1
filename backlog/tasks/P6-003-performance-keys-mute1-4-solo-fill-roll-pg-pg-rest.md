---
id: P6-003
title: Performance keys: MUTE1-4, SOLO, FILL, ROLL, PG-/PG+, RESTRT
phase: P6
type: task
status: todo
priority: 23
executor: sonnet
depends_on: [P6-002, BUG-007]
gate: none
---
## Goal
The black-key performance functions on PERF, with their non-force behaviour (force adds to FILL/ROLL in P6-006).

## Read first
- docs/design/SEQUENCER_UX.md §4.3 (key map PERF), §5 (traces)
- docs/design/key_map.tsv
- firmware/src/ui_sq.c

## Deliverable
- ui_sq.c PERF black keys per key_map: mute toggles (`MUTE n` toast), SOLO, FILL (maximum density fallback), ROLL (2x fallback), page keys, RESTRT

## Acceptance criteria
- [ ] Every PERF black key in key_map.tsv has a test
- [ ] Mute keeps phase (sq_core golden 5 still passes)
- [ ] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [ ] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## UX gate
Producer-facing. Use case: `docs/design/features/P6-003-performance-keys-mute1-4-solo-fill-roll.md` from `_TEMPLATE.md` (docs/project/UX_GATE.md). Gate A (steps, predicted ease score, five-surface sync, live checklist) is filled before the build or cited from the locked design; Gate B is filled by the reviewer.

## Out of scope
- Pages not named here
- ISR changes beyond the API
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
