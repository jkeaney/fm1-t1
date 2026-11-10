---
id: P3-004
title: Remove the web editor (R2, decision 37)
phase: P3
type: task
status: todo
priority: 2
executor: haiku
depends_on: [P3-003]
gate: none
---
## Goal
The SysEx web editor is groovebox-only; the update path (ota.c, loader, tools/fm1_install.py) stays untouched.

## Read first
- docs/design/SYSTEM_MAP.md §4 (rows named below), §5 (dependency edges), §6 row R2, §8 (test fate), §9 (bytes)
- firmware/src/felucca.c (the include list: include order is the dependency order)
- tests/run_tests.sh

## Deliverable
- Delete: firmware/src/editor.c, editor_backup.c, editor_fm6.c, editor_learn.c, editor_menu.c, editor_palette.c, editor_preferences.c; web/editor.html, web/EDITOR_PROTOCOL.md and the backup JS named in SYSTEM_MAP row 50 (keep web/ OTA files and fm1ota.js)
- Edit callers: main.c: the `ed_service` calls; felucca.c include
- Tests: editor_test, fuzz_ed, backup_test, the malformed-request part of robust_test
- Log: before/after build line (image, RAM, pool) and the bytes freed vs SYSTEM_MAP §9

## Acceptance criteria
- [ ] Every file listed under Delete is gone (`git status` shows `D`) and no `#include` of it remains in felucca.c
- [ ] `grep -rn` for each removed file's public symbols (named in the Deliverable) over firmware/src and tests returns nothing
- [ ] `sh tests/run_tests.sh` still runs ota_test and ldr_test and both pass
- [ ] `grep -rn 'ed_service' firmware/src` is empty
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
