---
id: P4-001
title: sq_types.h: constants, structs and static asserts
phase: P4
type: task
status: todo
priority: 2
executor: haiku
depends_on: [P3-012, P2-017, P2-029]
gate: none
---
## Goal
Land the locked data model in firmware, unchanged from the locked prototype header.

## Read first
- docs/project/decisions/ADR-0003-sequencer-core.md §3, §14 (layout of the sources)
- docs/design/algorithms/common/sq_types.h (P2-017 final: the source to copy)
- firmware/src/felucca.c (include order)

## Deliverable
- firmware/src/sq_types.h: a copy of docs/design/algorithms/common/sq_types.h with the GPL SPDX header; no behaviour
- felucca.c includes it after torso_t1_algo.c and before audio.c (ADR-0003 §14)
- tests/sq_types_test.c: compiles the header with `cc -std=c11 -Wall -Werror` and prints `sizeof` of ChanSave, ChanRT, UndoRec, SeqPreset, HarmonySave; run_tests.sh step

## Acceptance criteria
- [ ] `diff <(tail -n +3 firmware/src/sq_types.h) <(tail -n +3 docs/design/algorithms/common/sq_types.h)` shows only the SPDX/comment header lines
- [ ] sq_types_test prints ChanRT 24, HarmonySave 8, SeqPreset 1396 (or the P2-017 LOCK.md values)
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
- Any .c behaviour
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
