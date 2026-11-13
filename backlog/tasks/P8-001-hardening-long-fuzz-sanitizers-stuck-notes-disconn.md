---
id: P8-001
title: Hardening: long fuzz, sanitizers, stuck notes, disconnects; T1_ISSUES closed
phase: P8
type: task
status: todo
priority: 32
executor: sonnet
depends_on: [P7-003, P6-007]
gate: none
---
## Goal
Make v1 robust before release: no stuck notes under any input sequence, clean sanitizer runs, safe behaviour on USB disconnect and clock loss, no test or debug code in the normal image.

## Read first
- docs/project/decisions/ADR-0003-sequencer-core.md §8.4, §14 (fuzz)
- docs/design/TIMING_SPEC.md §7
- docs/T1_ISSUES.md
- tests/run_tests.sh (ASan/UBSan reruns)

## Deliverable
- Fuzz extended to all six algorithms + HARMONY + presets + UI API for 1000000 ticks under sanitizers (tests/sq_fuzz_long.c, opt-in run in check.sh as a separate stage)
- Host tests for USB disconnect mid-note and clock loss while playing (TIMING R-rules): every note gets its off
- docs/T1_ISSUES.md: every entry FIXED, DROPPED with reason, or N/A after R8
- A script tests/check_release_image.sh asserting no FELUCCA_TIMING_TEST / FELUCCA_FORCE_DEBUG symbols in build/felucca.bin

## Acceptance criteria
- [ ] Long fuzz passes with zero sanitizer reports
- [ ] check_release_image.sh passes
- [ ] T1_ISSUES.md has no OPEN entry
- [ ] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [ ] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
sh tests/check_release_image.sh
```

## UX gate
n/a, internal work (tooling, plumbing, docs, process); no producer-visible control, screen or sound change.

## Out of scope
- New features
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
