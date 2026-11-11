---
id: P3-003
title: Remove the dead DIGITAL engine and the FELUCCA_FM4 flag (R1)
phase: P3
type: task
status: review
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
- [x] Every file listed under Delete is gone (`git status` shows `D`) and no `#include` of it remains in felucca.c
- [x] `grep -rn` for each removed file's public symbols (named in the Deliverable) over firmware/src and tests returns nothing
- [x] `grep -rn FELUCCA_FM4 .` outside docs/ returns nothing
- [x] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [x] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

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

### 2026-11-11 executor (claude-haiku-5-5)

**Done.** Deleted `firmware/src/eng_digital.c` and the `FELUCCA_FM4` flag (core.h `#ifndef` default, every `#if FELUCCA_FM4` /
`#if !FELUCCA_FM4` block: dead branches removed, live `#if !` bodies kept unwrapped; ternaries and comments fixed).
Deleted `tests/digital_test.c`, `tests/fm4_test.c`, `tests/fm4_div0_test.c`; removed their steps and `ui_test_fm4` / `ui_render_fm4`
from `tests/run_tests.sh`. `tools/build.py` flag list, `BUILDING.md` flag row, `tests/target_budget.py` (DIGITAL OPTIONAL and
FUNCS entries), and comment-only mentions in web/editor.html and web/EDITOR_PROTOCOL.md updated. `fm4_convert.c` (the conversion
tables, ENG_FM4_GONE, DIGITAL_PRESETS) stays, as SYSTEM_MAP row 7 keeps it until R6.

**Before/after build (`./build.sh`, same tree otherwise):**
```
before: image 515492 B; RAM .data+.bss 55248 B of 73728; pool 192808 B of 368640
after:  image 515492 B; RAM .data+.bss 55248 B of 73728; pool 192808 B of 368640
```
Bytes freed: 0 B flash, 0 B RAM, 0 B pool. This matches SYSTEM_MAP row 56 ("not compiled today ... 0 B freed") and §9 (R1 is a leaf
with no measured size). Headroom 73728 - 55248 = 18480 B >= 8192 B, so no FB needed.

**Verification.**
- `sh tests/run_tests.sh`: exit 0, `ALL HOST TESTS PASSED`. Goldens unchanged: `regress: 97 golden renders (0 changed, 0 gone)`;
  `union_test: 720 engine switch sequences, 0 differ from 1.4`.
- `sh tools/check.sh`, first run: `CHECK FAIL: tests`. The failing check was
  `mod: cost: 4 active slots add at most a few percent (PHYS part, the 4-part mix) FAIL` (+6.7 %). It is a host-counter check
  (`proc_pid_rusage` instructions), load-sensitive. Reran `tests/mod_test.c` alone: the HEAD sources and this tree gave the same
  spread (HEAD +5.6, +5.4, +5.5, +5.4, +5.5 %; this tree +6.1, +6.2 (two FAILs), then +5.4, +5.6, +5.5 %). `mod_test.c` changed only
  a comment. Reran `sh tools/check.sh`: `RAM .data+.bss 55248 B of 73728`, tests all pass, `backlog`: `125 items, 0 errors`,
  final line `CHECK PASS`.
- Grep, run after the edits:
  - `git cat-file -e HEAD:firmware/src/eng_digital.c` -> absent. No `#include` of any removed file remains (grep for `eng_digital` /
    `#include.*digital`: only comments in fm4_convert.c and target_budget.py).
  - Public symbols of eng_digital.c (`ENG_DIGITAL`, `digital_render*`, `digital_env/stage`, `digital_note_on/mod/op_tick/custom`,
    `RATIO_Q8`, `fm_ratio_inc`) over firmware/ tests/ tools/: no match.
  - `grep -rn FELUCCA_FM4 .` outside docs/: the only hits are backlog records (BACKLOG.md index row, P1-002 and P3-002 logs,
    and this file's own Deliverable text). Those are history, so I did not edit them. Every code, test, tool, BUILDING and
    web hit is gone. The reviewer should decide whether backlog/ history counts against this criterion.

**Repo event the reviewer should know about.** This repo is shared with other executors committing on the same branch
(`t1-hardware-bringup`). I staged the deletions with `git rm`. Commit `7fbbe4b` ("Decision 110: P2-023 approved with amendments; P2-024
created"), made by another agent during this run, committed those staged deletions (`firmware/src/eng_digital.c`,
`tests/digital_test.c`, `tests/fm4_test.c`, `tests/fm4_div0_test.c`). The deletions are therefore already in history under that
commit. This task's commit carries the remaining edits. No history was rewritten.

**Criteria.** All five ticked. The first is ticked on the working tree and HEAD (`git status` no longer shows the staged `D`
because `7fbbe4b` already holds it; see above).

**Reviewer look at:** the `#if` strip (`git diff -- firmware tests`): every removed line is a dead `#if FELUCCA_FM4` body or its
`#else`/`#endif`. `ui_graph.c` `graph_fm` (DIGITAL algorithm charts) and `test_fm_charts` are gone with it. `page_visible` in ui.c and
the hash in `ui_graph.c` lose a `FELUCCA_FM4 &&` term that was already 0. Also `web/editor.html` text-only edits (not named in
the Deliverable; the criterion's grep covers them).
