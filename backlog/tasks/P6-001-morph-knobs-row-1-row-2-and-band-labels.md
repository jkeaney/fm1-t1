---
id: P6-001
title: Morph knobs: ROW 1 / ROW 2 and band labels
phase: P6
type: task
status: todo
priority: 21
executor: sonnet
depends_on: [P4-018, P5-008, P4-048]
gate: none
---
## Goal
K1-K4 morph the selected channel's algorithm; OCT-/OCT+ switch ROW 1/ROW 2; the band shows each knob's current role and value.

## Read first
- docs/design/SEQUENCER_UX.md §4.1, §4.2
- docs/design/ALGORITHM_SET.md §5 (knob convention, step sizes)
- docs/design/SCREEN_SPEC.md §2.4 (band), §3, §4
- firmware/src/ui_sq.c

## Deliverable
- ui_sq.c PERF knob handling: `sq_morph_turn` with slow/fast step (40 ms threshold), row switch, band labels and values from `ALGOS[].morph` for all six algorithms
- tests: each algorithm's 8 morphs labelled and reachable; row indicator

## Acceptance criteria
- [ ] For each of the six algorithms the band shows the AlgoDesc labels for both rows (test)
- [ ] A fast turn uses `MorphDesc.fast` (test)
- [ ] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [ ] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## UX gate
Producer-facing. Use case: `docs/design/features/P6-001-morph-knobs-row-1-row-2-and-band-labels.md` from `_TEMPLATE.md` (docs/project/UX_GATE.md). Gate A (steps, predicted ease score, five-surface sync, live checklist) is filled before the build or cited from the locked design; Gate B is filled by the reviewer.

## Out of scope
- Pages not named here
- ISR changes beyond the API
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
