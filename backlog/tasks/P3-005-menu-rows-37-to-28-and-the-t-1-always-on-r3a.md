---
id: P3-005
title: Menu rows 37 to 28 and the T-1 always on (R3a)
phase: P3
type: task
status: todo
priority: 2
executor: haiku
depends_on: [P3-004]
gate: none
---
## Goal
Remove the menu rows the sequencer does not use and make the T-1 mode permanent (SYSTEM_MAP §3 item 3), as the stepping stone to the new UI.

## Read first
- docs/design/SYSTEM_MAP.md §4 (rows named below), §5 (dependency edges), §6 row R3, §8 (test fate), §9 (bytes)
- firmware/src/felucca.c (the include list: include order is the dependency order)
- tests/run_tests.sh
- docs/design/SYSTEM_MAP.md §7 (exact ui_test edits)

## Deliverable
- Delete: the menu rows MI_LATCH, MI_BPMLOCK, MI_SELECT, MI_TCH1..MI_TCH4, MI_T1SEQ, MI_CHADD from menu_items.c (MENU_FLAGS, MI_TAB tables)
- Edit callers: `t1_mode` set to 1 at boot and `PREF_T1` retired (ui.c / ui_t1.c); any code reading the removed rows' values gets the fixed default
- Tests: tests/ui_test.c: row counts become "8 4 6 4 6", `mtab_rows(MTAB_CONTROL)==4`, `MTAB_MIDI` 4, `MTAB_SYSTEM` 6, navigation walk rewritten (SYSTEM_MAP §7); ui155_test.c SELECT lines, midi_control_test.c MI_TCH lines
- Log: before/after build line (image, RAM, pool) and the bytes freed vs SYSTEM_MAP §9

## Acceptance criteria
- [ ] Every file listed under Delete is gone (`git status` shows `D`) and no `#include` of it remains in felucca.c
- [ ] `grep -rn` for each removed file's public symbols (named in the Deliverable) over firmware/src and tests returns nothing
- [ ] `MI_COUNT` is 28
- [ ] The T-1 screen is the boot page in the host render test (`tests/ui_render.c` boots into it)
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
