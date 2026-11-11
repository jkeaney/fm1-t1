---
id: P4-019
title: Timing T-A: the timing-test image
phase: P4
type: task
status: todo
priority: 2
executor: haiku
depends_on: [P4-012, P4-007]
gate: none
---
## Goal
A separate firmware image that plays a fixed pattern and prints timing counters, for the timing gate (TIMING_SPEC 6.1). The normal image contains none of it.

## Read first
- docs/design/TIMING_SPEC.md §6.1
- firmware/src/console.c (`con_kv`)
- build.sh (flags, output names)

## Deliverable
- `FELUCCA_TIMING_TEST` flag: fixed pattern (channel 1, note 60, 1/16, gate 50 %, velocity 90 and 127 on step 1 of each bar), SET K1 steps 60/90/120/180/240 BPM, counters on the CDC console as listed
- `TIMING_TEST=1 ./build.sh` writes build/felucca_timing_test.fwsc; the normal build is byte-identical with and without this task's code compiled out (compare image size and a hash of the normal image's code section to before)

## Acceptance criteria
- [ ] `TIMING_TEST=1 ./build.sh` produces build/felucca_timing_test.fwsc
- [ ] `grep -c FELUCCA_TIMING_TEST` code is all under `#if`; normal build size unchanged within 0 B
- [ ] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [ ] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
TIMING_TEST=1 ./build.sh && ls -l build/felucca_timing_test.fwsc
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## UX gate
n/a, internal work (tooling, plumbing, docs, process); no producer-visible control, screen or sound change.

## Out of scope
- The measurement tool (P4-008)
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
