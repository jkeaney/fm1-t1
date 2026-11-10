---
id: P3-009
title: Remove the sample family and FM6 (R6b)
phase: P3
type: task
status: todo
priority: 2
executor: haiku
depends_on: [P3-008]
gate: none
---
## Goal
Remove the sampler (126 KB of ADPCM data) and the 6-op FM engine.

## Read first
- docs/design/SYSTEM_MAP.md §4 (rows named below), §5 (dependency edges), §6 row R6, §8 (test fate), §9 (bytes)
- firmware/src/felucca.c (the include list: include order is the dependency order)
- tests/run_tests.sh

## Deliverable
- Delete: eng_sample.c, eng_grain.c, eng_slice.c, felucca_samples.h, eng_fm6.c, fm6_core.c, fm6_vbank.c, fm4_convert.c, felucca_fm6.h, slice data
- Edit callers: engines.c, params.c, settings_persist.c/category remnants, project.c loaders that name them
- Tests: sample_test, slice_test, fm6_test, fm6bank_test, fuzz_smp; regress lines
- Log: before/after build line (image, RAM, pool) and the bytes freed vs SYSTEM_MAP §9

## Acceptance criteria
- [ ] Every file listed under Delete is gone (`git status` shows `D`) and no `#include` of it remains in felucca.c
- [ ] `grep -rn` for each removed file's public symbols (named in the Deliverable) over firmware/src and tests returns nothing
- [ ] Flash image shrinks by at least 140000 B
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
