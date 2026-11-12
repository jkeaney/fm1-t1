---
id: P4-002
title: sq_prims.c with golden tests
phase: P4
type: task
status: todo
priority: 2
executor: sonnet
depends_on: [P4-001, BUG-002, P2-029]
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

- Orchestrator decision 2026-11-12 (resolves FB-009, option a): the prototype harness gets a deterministic dump mode. Add `--dump` to `tests/design/prims_test.c` (stdout only, one line per primitive case: `<function> <inputs> -> <outputs>`, covering every function in sq_prims.h with the cases it already checks, fixed seeds, no timestamps). Check `tests/design/run_protos.sh` still passes (the normal run prints nothing new). Generate `tests/sq_prims_golden.txt` from `--dump`; the firmware test `tests/sq_prims_test.c` includes `firmware/src/sq_prims.c` and prints the same dump format, compared byte for byte.

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

## UX gate
n/a, internal work (tooling, plumbing, docs, process); no producer-visible control, screen or sound change.

## Out of scope
- The scheduler
- Algorithms
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log

### 2026-11-12 executor (claude-haiku-5-5)
Status: **blocked** before any firmware change. Filed `backlog/feedback/FB-009-p4-002-prototype-harness-prints-no-primitive-golden-lines.md`.

What is missing: the golden. The deliverable needs `tests/sq_prims_golden.txt` "copied from the prototype run", with the
same lines the prototype harness prints. The prototype prints none:

    cc -std=c11 -Wall -Wextra -Werror -O1 -fno-strict-aliasing -Idocs/design/algorithms/common \
       -o /tmp/p4002/prims tests/design/prims_test.c docs/design/algorithms/common/sq_prims.c
    /tmp/p4002/prims > /tmp/p4002/out.txt; echo "run exit $?"; wc -c < /tmp/p4002/out.txt
    run exit 0
    0
    (stderr) prims: primitive checks OK

stdout is empty; the only output is one stderr line and the exit code. The values are checked against constants in
the source, not printed. A golden built from this would be empty or invented. Per the task's stop rule, no format was invented.

Not run: `sh tools/check.sh` and `./build.sh` (no code changed, so there is no RAM result to report yet).
Needed from the owner: FB-009 decision (print mode, exit-code stand-in, or other). Then P4-002 goes back to `todo`.
