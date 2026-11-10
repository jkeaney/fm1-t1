---
id: P3-006
title: Remove the groovebox UI files and page code (R3b)
phase: P3
type: task
status: todo
priority: 2
executor: sonnet
depends_on: [P3-005]
gate: none
---
## Goal
Delete the groovebox pages; the T-1 screen (ui_t1.c) is the only page left besides the menu, until P4-011 replaces it.

## Read first
- docs/design/SYSTEM_MAP.md §4 (rows named below), §5 (dependency edges), §6 row R3, §8 (test fate), §9 (bytes)
- firmware/src/felucca.c (the include list: include order is the dependency order)
- tests/run_tests.sh

## Deliverable
- Delete: firmware/src/ui_tools.c, ui_layer.c, ui_name.c, ui_events.c, ui_slice.c, category.c, ui_fm6op.c; the drum/step/pattern/browse/piano-roll/sample page code inside ui_draw.c, ui_input.c, ui_graph.c; parameter icon cells (`FELUCCA_ICONS`); the dead page-table rows FX, SLICER, INSERT, INSERT 2, REVERB in params.c (their slicer/bus code went in P3-002/P3-014) and the `GR_FX`, `GR_SLCR`, `GR_INS` graph ids with `graph_fx`
- Edit callers: ui.c (page state, browse, `up_used` calls), ui_draw.c `ui_draw_page` priority list (keep uboot, menu, count-in, confirm, T-1), ui_input.c (keep hold timers, menu dispatch, the T-1 early return), ui_help.c only if `draw_layer` was its last user
- Tests: ui_test and ui_render reduced to menu + T-1 screen; ui155_test, detail_test, step15_test, v16_test, v156_test, drumfuzz_test go
- Log: before/after build line (image, RAM, pool) and the bytes freed vs SYSTEM_MAP §9

## Acceptance criteria
- [ ] Every file listed under Delete is gone (`git status` shows `D`) and no `#include` of it remains in felucca.c
- [ ] `grep -rn` for each removed file's public symbols (named in the Deliverable) over firmware/src and tests returns nothing
- [ ] `ui_draw_page` handles exactly: uboot, menu, count-in, confirm, T-1
- [ ] Host render PNGs for the menu tabs and the T-1 screen still generate
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
