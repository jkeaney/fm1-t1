---
id: P6-004
title: LED-2: algorithm-aware layers, FILL/ROLL and dice flash
phase: P6
type: task
status: todo
priority: 2
executor: haiku
depends_on: [P4-039, P6-003]
gate: none
---
## Goal
LED-2: add what needs the P5/P6 features to the LED frame built in P4-039: dice buttons glow by the algorithm's real layers, a lit flash on each dice, and the SOL/FIL/ROL/RST states. The basic frame (steps, playhead, PLAY, REC, M keys) is P4-039.

## Read first
- docs/design/TIMING_SPEC.md §5.4
- docs/design/SEQUENCER_UX.md (LED rows per page)
- docs/design/features/P4-038-led-design.md and the P4-039 LED frame (`uisq_leds`)
- firmware/src/ui_sq.c

## Deliverable
- `uisq_leds` extended: dice buttons glow when the active algorithm has that layer (from `ALGOS[].layer[].n`), a 100 ms lit flash on a dice, FILL/ROLL/SOLO lit while active, RST glow; host bitmap tests for each

## Acceptance criteria
- [ ] Host bitmap tests for dice glow per algorithm (a layer present and absent), dice flash, and FILL/ROLL/SOLO match expected frames
- [ ] LED update cost stays within the main loop frame (no ISR use)
- [ ] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [ ] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## UX gate
Producer-facing. Use case: extend `docs/design/features/P4-038-led-design.md` (rows for the P5/P6 features) (docs/project/UX_GATE.md). Gate A (steps, predicted ease score, five-surface sync, live checklist) is filled before the build or cited from the locked design; Gate B is filled by the reviewer.

## Out of scope
- The basic LED frame (P4-039); pages not named here
- ISR changes beyond the API
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
- 2026-11-13: re-scoped as LED-2 (algorithm-aware layers, FILL/ROLL, dice flash); the base LED frame is P4-039, now a dependency.
