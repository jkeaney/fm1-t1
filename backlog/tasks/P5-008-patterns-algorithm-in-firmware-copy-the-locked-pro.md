---
id: P5-008
title: PATTERNS algorithm in firmware (copy the locked prototype)
phase: P5
type: task
status: todo
priority: 19
executor: haiku
depends_on: [P5-005, P4-042]
gate: none
---
## Goal
Land PATTERNS exactly as prototyped (ADR-0003 §14: the prototype is the firmware file).

## Read first
- docs/design/algorithms/patterns/SPEC.md, proto.c, golden.txt
- docs/project/decisions/ADR-0003-sequencer-core.md §4 (vtable, AlgoDesc), §14 (purity and table tests)
- firmware/src/sq_algos.c (the ALGOS[] table; created by P5-001)
- docs/design/algorithms/patterns/ (pattern data table, RESEARCH.md)
- docs/project/OWNER_QUESTIONS.md decision 90 (style names, never song titles)

## Deliverable
- firmware/src/sq_algo_patterns.c: copy of proto.c with `static` on every function, SPDX header, included in felucca.c after sq_prims.c and before sq_algos.c
- `ALGOS[]` row for PATTERNS in sq_algos.c (P5-001 creates sq_algos.c with the table and `SQ_NALGO` entries)
- tests/sq_algo_patterns_test.c: includes the firmware file, prints the golden matrix, diffs against docs/design/algorithms/patterns/golden.txt; runs the generic purity and table tests (ADR-0003 §14); run_tests.sh step

- Orchestrator decision 2026-11-12 (resolves the P5-001 block): the algorithm golden is checked in two parts. This task diffs only the plain matrix part of golden.txt (everything before the `H-INERT` section); the `H-INERT` and `HARM 1-4-5-1` sections need `harm.c` and are checked by P5-005's test, whose acceptance criterion gains them. `static` is not applied to typedefs. Rows of the `ALGOS[]` table for algorithms that do not exist yet are filled by their own tasks (the unity build must still compile: use the real rows only, with `SQ_NALGO` and the table sized per the ADR). If the Use case file `docs/design/features/<ID>-*.md` named in the UX gate section does not exist, create it from `_TEMPLATE.md` using the algorithm's SPEC.md section 1 and the scenario steps that exercise it, with Gate A marked 'cited from the locked design (P2-017)'.

## Acceptance criteria
- [ ] `diff` of the test output against docs/design/algorithms/patterns/golden.txt is empty
- [ ] Purity tests (a)-(e) and table tests pass
- [ ] The pattern data is `static const` (flash, not RAM): RAM growth matches LOCK.md
- [ ] No song title appears in firmware strings (`strings build/felucca.bin` checked against the RESEARCH.md title list by a test script)
- [ ] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [ ] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## UX gate
Producer-facing. Use case: `docs/design/features/P5-008-patterns-algorithm-in-firmware-copy-the.md` from `_TEMPLATE.md` (docs/project/UX_GATE.md). Gate A (steps, predicted ease score, five-surface sync, live checklist) is filled before the build or cited from the locked design; Gate B is filled by the reviewer.

## Out of scope
- Other algorithms
- UI
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
- 2026-11-13: now depends on P4-042 (bounded tie search in the ISR) before PATTERNS adds an expensive step().
- 2026-11-13 planner (note only, no re-ordering): PATTERNS' UP/DN dice re-rolls its seed and `s_mod`; the `s_mod` half is audible only through the MOD engine (P4-047 / P4-048 / P4-049). Copy the prototype's `mod` row as is; check it on the TB-3 in HW-2 (P5-009).
