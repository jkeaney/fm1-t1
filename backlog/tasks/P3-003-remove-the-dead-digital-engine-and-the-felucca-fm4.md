---
id: P3-003
title: Remove the dead DIGITAL engine and the FELUCCA_FM4 flag (R1)
phase: P3
type: task
status: in_progress
priority: 2
executor: haiku
depends_on: [P3-014]
gate: none
---
## Goal
Delete code that is not even compiled today.

## Read first
- docs/design/SYSTEM_MAP.md §4 (rows named below), §5 (dependency edges), §6 row R1, §8 (test fate), §9 (bytes)
- firmware/src/felucca.c (the include list: include order is the dependency order)
- tests/run_tests.sh

## Deliverable
- Delete: firmware/src/eng_digital.c; the `FELUCCA_FM4` flag and its `#if` blocks
- Edit callers: build.sh/build.py flag handling; BUILDING.md flag table
- Tests: digital_test, fm4_test, fm4_div0_test, `ui_test_fm4` and its render step go from run_tests.sh
- Log: before/after build line (image, RAM, pool) and the bytes freed vs SYSTEM_MAP §9

## Acceptance criteria
- [ ] Every file listed under Delete is gone (`git status` shows `D`) and no `#include` of it remains in felucca.c
- [ ] `grep -rn` for each removed file's public symbols (named in the Deliverable) over firmware/src and tests returns nothing
- [ ] `grep -rn FELUCCA_FM4 .` outside docs/ returns nothing
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
