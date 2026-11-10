---
id: P6-006
title: Force gestures per KEYBED_USE_CASES
phase: P6
type: task
status: todo
priority: 2
executor: haiku
depends_on: [P6-005]
gate: none
---
## Goal
Turn the locked use cases into gestures: what each does to the sound, its MIDI output, its screen feedback and fallback.

## Read first
- docs/design/KEYBED_USE_CASES.md (P2-015: the gesture table is the spec)
- docs/design/SEQUENCER_UX.md §6.1 as amended by P2-015
- firmware/src/ui_sq.c

## Deliverable
- ui_sq.c / sq_core.c: each use case of KEYBED_USE_CASES with its MIDI output (velocity, CC or repeats) and visible-before-it-counts feedback; each fallback when FORCE is OFF

## Acceptance criteria
- [ ] One host test per use case: input bins -> MIDI out and screen string
- [ ] FORCE OFF: every use case uses its fallback (test)
- [ ] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [ ] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## Out of scope
- Pages not named here
- ISR changes beyond the API
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
