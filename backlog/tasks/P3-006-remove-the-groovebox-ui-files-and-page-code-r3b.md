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
- Delete: firmware/src/ui_tools.c, ui_layer.c, ui_name.c, ui_events.c, ui_slice.c, ui_fm6op.c. **category.c stays** (rehearsal 2026-11-11: upreset.c:79 includes it for `UP_CAT_AT`/`up_cat_of` and tests/upreset_test.c uses `CAT_N`; P3-011 deletes it with upreset.c). Only ui.c's own `#include "category.c"` (line 41) and its PRESETS > LIST category filter (ui.c ~1541-1547, `CAT_N`, `CAT_NAME`, `PRESET_CAT`) go here; the drum/step/pattern/browse/piano-roll/sample page code inside ui_draw.c, ui_input.c, ui_graph.c; parameter icon cells (`FELUCCA_ICONS`); the dead page-table rows FX, SLICER, INSERT, INSERT 2, REVERB in params.c (their slicer/bus code went in P3-002/P3-014) and the `GR_FX`, `GR_SLCR`, `GR_INS` graph ids with `graph_fx`
- Include sites to cut (rehearsal: deleting the files makes these the first build errors): ui_input.c:4 (`ui_name.c`), felucca.c:96 (`ui_layer.c`, which itself includes `ui_tools.c` at its line 52), ui_graph.c:15 (`ui_fm6op.c`), ui.c:41 (`category.c`), ui.c:1747-1748 (`ui_slice.c`, `ui_events.c`); tests/ui_test.c:158 and web/emu/felucca_web.c:170 include `ui_layer.c`. After those, the first compile errors are `CAT_N` in ui.c (4) and 15 in ui_graph.c; expect many more behind them as the page code is cut.
- Dead readers left by P3-005 that this task removes: `sel_mode()`/`SEL_*`/`sel_play`/`kb_vel` (ui.c, ui_input.c:114 and 1768-1775, ui_help.c:78), the `PREF_BPM_LOCK` branches (ui_input.c:1776 onward, ui_draw.c:198, 336, 397) with the tests that set it (tests/ui_render.c 1020-1022; ui_test.c 8463, 8475, 8584, 8604), and the `#ifdef T1_DEVICE` around `t1u_always` in ui_t1.c (make it always 1; the host tests then run the T-1 screen).
- Edit callers: ui.c (page state, browse, `up_used` calls), ui_draw.c `ui_draw_page` priority list (keep uboot, menu, count-in, confirm, T-1), ui_input.c (keep hold timers, menu dispatch, the T-1 early return), ui_help.c only if `draw_layer` was its last user
- Tests: ui_test and ui_render reduced to menu + T-1 screen; ui155_test, detail_test, step15_test, v16_test, v156_test, drumfuzz_test go (P3-005 left v156_test and ui155_test with only their non-SELECT blocks); tests/motion_test.c names ui_events, so P3-007's motion_test deletion must come with it or this task edits it
- tools/size_fns.py: remove "ui_layer.c", "ui_name.c", "ui_slice.c", "ui_events.c" from `SIZE_FILES`. **A deleted file still listed there crashes ./build.sh** (FileNotFoundError); found in FB-005.
- web/emu/felucca_web.c: delete `#include ".../ui_layer.c"` (line ~170); tests/settings_test.c, tests/ui_render.c, tests/ui_test.c include or name ui_layer/ui_name/ui_events/ui_slice/ui_fm6op/ui_tools: edit them too (grep `ui_layer\|ui_name\|ui_events\|ui_slice\|ui_fm6op\|ui_tools` in tests/ and tools/gen_aa_icons.py comments); tests/ui_render.py names ui_name. The emulator is not built here (no Emscripten): check by reading.
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
