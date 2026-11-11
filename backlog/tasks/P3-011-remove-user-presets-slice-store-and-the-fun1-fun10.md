---
id: P3-011
title: Remove user presets, slice_store and the FUN1-FUN10 project formats (R7)
phase: P3
type: task
status: todo
priority: 2
executor: haiku
depends_on: [P3-010]
gate: none
---
## Goal
Remove the groovebox storage formats; project.c keeps only what the preset store (P7-002) will build on.

## Read first
- docs/design/SYSTEM_MAP.md §4 (rows named below), §5 (dependency edges), §6 row R7, §8 (test fate), §9 (bytes)
- firmware/src/felucca.c (the include list: include order is the dependency order)
- tests/run_tests.sh

## Deliverable
- Delete: upreset.c, up_fm6.c, slice_store.c; FUN1..FUN10 loaders/savers, pattern/song/motion sections in project.c
- Edit callers: ui.c `up_used`, storage object ids they used (leave the storage A/B core untouched)
- Tests: upreset_test, fuzz_proj, the remaining backup_test and project_test parts; persistence_test and settings_test still pass unchanged
- tools/size_fns.py: remove "upreset.c", "up_fm6.c", "slice_store.c" from `SIZE_FILES`. **A deleted file still listed there crashes ./build.sh** (FileNotFoundError); found in FB-005.
- Correction to the Tests line: tests/persistence_test.c `#include`s upreset.c and uses `up_bank_t`/`up_*` in main(), so it does NOT pass unchanged: drop the include and the up_* cases, keep the rest. tests/slice_test.c includes slice_store.c (already deleted in P3-009 if slice_test went; check). web/emu/felucca_web.c includes upreset.c: delete. tests/run_tests.sh: upreset_test and slice_test runs. tests/ui_test.c names upreset. (backup_test and fuzz_proj were already deleted in P3-004: do not look for them.)
- Log: before/after build line (image, RAM, pool) and the bytes freed vs SYSTEM_MAP §9

## Acceptance criteria
- [ ] Every file listed under Delete is gone (`git status` shows `D`) and no `#include` of it remains in felucca.c
- [ ] `grep -rn` for each removed file's public symbols (named in the Deliverable) over firmware/src and tests returns nothing
- [ ] settings_test and persistence_test pass with no edits to their assertions
- [ ] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [ ] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## Out of scope
- The dead parameter slots P_DIST..P_REV, P_SLCR..P_SLDEPTH, P_ITYPE..P_IMIX, G_RTYPE/G_RSIZE/G_RDAMP and mod.c MD_DIST..MD_REV (FB-002 C): renumbering the param table is the v2 params prune (SYSTEM_MAP row 10), not this task
- Any subsystem SYSTEM_MAP marks KEEP or ADAPT beyond the caller edits named here
- Behaviour changes to the T-1 path (P3-001's test must still pass)
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
