---
id: P3-010
title: Remove the remaining engines and eng_mem; ANALOG only (R6c)
phase: P3
type: task
status: todo
priority: 2
executor: haiku
depends_on: [P3-009]
gate: none
---
## Goal
Leave exactly one engine, ANALOG preset 0, as the fixed preview synth (SYSTEM_MAP §2, decision 40).

## Read first
- docs/design/SYSTEM_MAP.md §4 (rows named below), §5 (dependency edges), §6 row R6, §8 (test fate), §9 (bytes)
- firmware/src/felucca.c (the include list: include order is the dependency order)
- tests/run_tests.sh

## Deliverable
- Delete: eng_phase.c, eng_lofi.c, eng_trio.c, eng_wheel.c, eng_formant.c (VOICE), eng_noise.c; the `eng_mem` pool buffer
- Edit callers: engines.c `ENGINES[]` = ANALOG only, `NENGINES` 1, `eng_idx()` still returns 0; params.c pruned to ANALOG pages; the factory sequence patterns (SAVE > PHRASES) in engines.c
- Tests: noise_test, fold155_test, union_test go; sound14/sound15 keep ANALOG cases; `GOLDEN_UPDATE=1` and `BUDGET_UPDATE=1` re-baseline regress to ANALOG, Log lists the change
- Log: before/after build line (image, RAM, pool) and the bytes freed vs SYSTEM_MAP §9

## Acceptance criteria
- [ ] Every file listed under Delete is gone (`git status` shows `D`) and no `#include` of it remains in felucca.c
- [ ] `grep -rn` for each removed file's public symbols (named in the Deliverable) over firmware/src and tests returns nothing
- [ ] `grep -c ENG_MEM firmware/src/*.c` is 0
- [ ] Pool used drops by about 51552 B
- [ ] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [ ] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## Out of scope
- Any subsystem SYSTEM_MAP marks KEEP or ADAPT beyond the caller edits named here
- Behaviour changes to the T-1 path (P3-001's test must still pass)
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
