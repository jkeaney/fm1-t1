---
id: P4-002
title: sq_prims.c with golden tests
phase: P4
type: task
status: todo
priority: 2
executor: haiku
depends_on: [P4-001, BUG-002]
gate: none
---
## Goal
Land the shared primitives (SP1-SP9, SP11 wrappers) exactly as prototyped.

## Read first
- docs/project/decisions/ADR-0003-sequencer-core.md §4, §5, §14
- docs/design/algorithms/common/sq_prims.h, sq_prims.c, and their expected output in tests/design/run_protos.sh
- docs/design/ALGORITHM_SET.md §6

## Deliverable
- firmware/src/sq_prims.h, sq_prims.c: copies with `static` added to every function, SPDX header; felucca.c includes them after sq_types.h
- tests/sq_prims_test.c: includes firmware/src/sq_prims.c, prints the same lines the prototype harness prints for the primitives, compared with a checked-in tests/sq_prims_golden.txt copied from the prototype run; run_tests.sh step

## Acceptance criteria
- [ ] sq_prims_test output equals tests/sq_prims_golden.txt byte for byte
- [ ] sq_prims.c includes only `<stdint.h>`/`<stddef.h>`/`<stdbool.h>` and sq_types.h
- [ ] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [ ] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## Out of scope
- The scheduler
- Algorithms
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
