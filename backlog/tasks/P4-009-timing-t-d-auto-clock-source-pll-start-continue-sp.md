---
id: P4-009
title: Timing T-D: AUTO clock source, PLL, Start/Continue/SPP
phase: P4
type: task
status: todo
priority: 1
executor: sonnet
depends_on: [P4-007]
gate: none
---
## Goal
Follow an external MIDI clock without jitter or drift and switch sources by the owner's AUTO rules (TIMING_SPEC §4, M11, M12).

## Read first
- docs/design/TIMING_SPEC.md §3 M11, M12, §4 (rules R1-R16), §6.3 (PLL, source rules, start tests)
- docs/design/SCREEN_SPEC.md §6.1 (state fields the screen reads)
- firmware/src/midi_clock.c, usb.c (SPP parse)

## Deliverable
- Clock source state machine (INT/EXT SYNC/EXT LIVE/EXT WAIT per §4.1), PLL with the §10 gains, `midi_in_t[]` stamps, SPP parse, `CLK_TIMEOUT_MS` 500; state fields exported for the UI
- tests/clock_test.c: PLL jitter/burst/ramp/no-loss cases, table-driven R1-R16, Start-then-pulses and SPP+Continue

## Acceptance criteria
- [ ] clock_test passes every case
- [ ] Clock out is silent in every EXT state (asserted)
- [ ] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [ ] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## UX gate
n/a, internal work (tooling, plumbing, docs, process); no producer-visible control, screen or sound change.

## Out of scope
- Screen drawing (P4-016)
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
