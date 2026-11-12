---
id: P4-001
title: sq_types.h: constants, structs and static asserts
phase: P4
type: task
status: done
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
- [x] `diff <(tail -n +3 firmware/src/sq_types.h) <(tail -n +3 docs/design/algorithms/common/sq_types.h)` shows only the SPDX/comment header lines
- [x] sq_types_test prints ChanSave 384, ChanRT 24, UndoRec 94, SeqPreset 1556, HarmonySave 8 (LOCK.md section 5; 1396 was the superseded figure)
- [x] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [x] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

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

### 2026-11-12 executor (claude-haiku-5-5)

Pre-checks: header matches ADR-0003 section 22 (SeqPreset 1556 B, ChanSave 384, UndoRec 94, PO_TIE 0x80, ModIdx, AlgoDesc.mod).
The task's "SeqPreset 1396" is the superseded figure; the criterion allows the LOCK.md value, 1556 (LOCK.md section 5). No
identifier of the header collides with firmware/src or tests (typedefs and SQ_/PO_/HR_/AF_/SE_/HARM_ macros; the font macros
are AF_S/AF_M/AF_L, no clash with AF_PAIRING/AF_CHORD). The header has no functions, so no `static` was needed.

Changes: `firmware/src/sq_types.h` (byte-identical copy of docs/design/algorithms/common/sq_types.h, which already carries the
GPL SPDX line); `firmware/src/felucca.c` (one include line after `torso_t1_algo.c`, before `torso_t1_pipeline.c`, which is
before `audio.c`, per ADR-0003 section 14); `tests/sq_types_test.c` (new; prints sizes, exits non-zero on a size change);
`tests/run_tests.sh` (one step: `cc -std=c11 -Wall -Werror`, then run).

Commands and output:
```
$ cmp docs/design/algorithms/common/sq_types.h firmware/src/sq_types.h && echo COPY-IDENTICAL
COPY-IDENTICAL
$ diff <(tail -n +3 firmware/src/sq_types.h) <(tail -n +3 docs/design/algorithms/common/sq_types.h) && echo TAIL-DIFF-EMPTY
TAIL-DIFF-EMPTY
$ cc -std=c11 -Wall -Werror -Ifirmware/src -o /tmp/sq_types_test tests/sq_types_test.c && /tmp/sq_types_test
ChanSave 384
ChanRT 24
UndoRec 94
SeqPreset 1556
HarmonySave 8
SQ_TYPES PASS
$ sh tools/check.sh
== build (log: build/check/build.log)
== ram headroom
RAM .data+.bss 23316 B of 73728
== tests (log: build/check/tests.log)
== backlog (log: build/check/backlog.log)
CHECK PASS
$ grep 'RAM .data' build/check/build.log     # the ./build.sh run inside check.sh (same output as ./build.sh | grep RAM)
  ok    image 168876 B; RAM .data+.bss 23316 B of 73728; pool 92500 B of 368640
$ grep -c 'STEP FAILED' build/check/tests.log
0
$ tail -n 1 build/check/tests.log
ALL HOST TESTS PASSED
```
RAM headroom = 73728 - 23316 = 50412 B, above 8192 B; no FB needed. The known "web pages" failure did not occur.
Criteria 1 to 4 verified and ticked; `sq_types_test` prints SeqPreset 1556 (the LOCK.md value, see above).
Not flashed; no device touched.

### 2026-11-12 review (sonnet)
Verdict: PASS (gate none). Verified: `cmp` firmware/src/sq_types.h vs docs/design/algorithms/common/sq_types.h is identical. Sizes (re-run) ChanSave 384, ChanRT 24, UndoRec 94, SeqPreset 1556, HarmonySave 8 match ADR-0003 section 22 and docs/design/algorithms/LOCK.md section 5 (LOCK.md is at docs/design/algorithms/, not common/). The include (felucca.c:77) sits after torso_t1_algo.c, before torso_t1_pipeline.c and audio.c; it is types-only so no order hazard. run_tests.sh step present; `sh tools/check.sh` ended CHECK PASS (host test links). Scope: only the listed files changed.
`./build.sh`: `image 168860 B; RAM .data+.bss 23316 B of 73728; pool 92500 B of 368640` (headroom 50412 B, above 8192).
Correction: acceptance criterion 2 said SeqPreset 1396, a superseded figure (ADR-0003 sections 11.2/20 -> 1556 in section 22); the criterion now lists the five locked sizes. No code change.
