---
id: P5-003
title: SNAKE algorithm in firmware (copy the locked prototype)
phase: P5
type: task
status: todo
priority: 2
executor: haiku
depends_on: [P5-001]
gate: none
---
## Goal
Land SNAKE exactly as prototyped (ADR-0003 §14: the prototype is the firmware file).

## Read first
- docs/design/algorithms/snake/SPEC.md, proto.c, golden.txt
- docs/project/decisions/ADR-0003-sequencer-core.md §4 (vtable, AlgoDesc), §14 (purity and table tests)
- firmware/src/sq_algos.c (the ALGOS[] table; created by P5-001)

## Deliverable
- firmware/src/sq_algo_snake.c: copy of proto.c with `static` on every function, SPDX header, included in felucca.c after sq_prims.c and before sq_algos.c
- `ALGOS[]` row for SNAKE in sq_algos.c (P5-001 creates sq_algos.c with the table and `SQ_NALGO` entries)
- tests/sq_algo_snake_test.c: includes the firmware file, prints the golden matrix, diffs against docs/design/algorithms/snake/golden.txt; runs the generic purity and table tests (ADR-0003 §14); run_tests.sh step

## Acceptance criteria
- [ ] `diff` of the test output against docs/design/algorithms/snake/golden.txt is empty
- [ ] Purity tests (a)-(e) and table tests pass
- [ ] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [ ] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## Out of scope
- Other algorithms
- UI
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
