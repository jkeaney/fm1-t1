---
id: FB-005
title: P3-004 web editor removal misses tests, tools and web files it breaks
phase: P3
type: feedback
status: done
priority: 2
executor: sonnet
depends_on: [P3-004]
gate: none
session: 2026-11-11, planner (sonnet), after the Haiku executor blocked P3-004
---
## Goal
Complete P3-004's scope so the executor finishes without surprises, and audit P3-005..P3-012 for the same gap.

## Read first
- backlog/tasks/P3-004-remove-the-web-editor-r2-decision-37.md (Log: the Haiku's list)
- backlog/feedback/FB-003-p3-002-slicer-defines-and-hostsim-unnamed.md

## Observation
P3-004 named the editor files but not what depends on them. See the Log for the verified list.

## Deliverable
- P3-004 rewritten; gaps in P3-006..P3-011 patched; recorded below.

## Acceptance criteria
- [ ] P3-004 is `todo` with a rehearsed scope; other tasks patched where a gap was found.

## Verification
```sh
python3 tools/backlog.py check
```

## Log
- 2026-11-11 planner (sonnet): answered. Rehearsal in a scratch copy (rsync to /tmp, real tree untouched): applied the planned removal, `./build.sh`, syntax-only compile of every tests/*.c, one full `sh tests/run_tests.sh`.
  - The Haiku's 8 items verified by grep. Extra gaps it did not find:
    1. **tools/size_fns.py `SIZE_FILES`** lists editor.c, editor_preferences.c, editor_backup.c, editor_menu.c, editor_fm6.c: ./build.sh crashes after the deletion (FileNotFoundError) until they are removed.
    2. **tests/persistence_test.c** `#include`s editor_preferences.c and editor_menu.c and its main() opens with two blocks that call `ed_ui_set` / `ed_menu_handle`: it must be cut (keeps autosave and flash tests).
    3. **tests/robust_test.c** cannot be "split at 122-128": it is built on `#include "editor_test.c"` (hostsim, stubs, RAM flash, reset()). Its other parts cover sample slots, projects, user presets (removed in P3-009/P3-011). Decision: delete the whole file; interim coverage loss noted in P3-004.
    4. **tests/descdump.c** and its run_tests.sh step only feed web/test_web.mjs: delete.
    5. **web/fm1backup.js is not orphaned**: index_pkg.html (installer) inlines it for the "return to official V15" backup flow, and make_site.py inlines it. Keep it and web/test_backup.mjs.
    6. **web/fukiai.ttf and web/FUKIAI-LICENSE.txt must stay**: tools/gen_aa_icons.py and gen_panel_diagram.py read web/fukiai.ttf at build time; LICENSING.md lists both. A literal "delete web editor files" would break the build.
    7. web/emu/index.html links `../editor/`; main.c has a second `ed_service` inside its own `#if FELUCCA_OTA` (3 lines go).
  - Scope decided (details in P3-004): delete the 7 editor sources, editor.html, EDITOR_PROTOCOL.md, test_web.mjs, and tests editor_test, fuzz_ed, backup_test, fm6bank_test, fuzz_proj, fuzz_smp, descdump, robust_test; edit felucca.c, main.c, usb.c comment, size_fns.py, persistence_test.c, run_tests.sh, make_site.py, index_pkg.html, emu/index.html.
  - Rehearsal result: build before `image 515492 B; RAM .data+.bss 55248 B of 73728; pool 192808 B of 368640`; after `image 500212 B; RAM .data+.bss 54064 B of 73728; pool 191048 B of 368640` (freed 15280 / 1184 / 1760 B; headroom 19664 B). Golden 97 renders, 0 changed; target budget 0 over (see P3-004 for the full-run outcome). No GOLDEN_UPDATE or BUDGET_UPDATE needed.
  - Gaps patched in other tasks (static audit, greps only, no builds):
    - P3-006: size_fns.py (ui_layer, ui_name, ui_slice, ui_events), web/emu/felucca_web.c include of ui_layer.c, settings_test.c / ui_render.* / ui_test.c references.
    - P3-007: size_fns.py (song_chain, motion, chord), **tests/hostsim.c includes song_chain.c** (every host test), target_budget FUNCS/rows for the perf_* functions (a missing function makes the budget step FAIL), run_tests.sh runs.
    - P3-008: target_budget FUNCS/rows for phys/drum/dv_* functions, run_tests.sh runs, LICENSING.md rows.
    - P3-009: size_fns.py (fm4_convert), target_budget rows (sample/grain/slice/fm6), tools/build.py generator entries for felucca_fm6.h and felucca_samples.h, run_tests.sh blocks reading felucca_samples.h, web/emu include; fm6bank_test/fuzz_smp noted as already gone.
    - P3-010: target_budget rows for the other engines, run_tests.sh runs, regress/fold155 references.
    - P3-011: size_fns.py (upreset, up_fm6, slice_store); correction: persistence_test.c includes upreset.c so it does not pass "unchanged"; web/emu include; backup_test/fuzz_proj already gone.
    - P3-012: added the open item below to its report.
    - P3-005: no gap found.
  - **Open item for the owner (not blocking):** after P3-004 the installer page still offers a SysEx backup before returning to official V15, but the cleaned firmware no longer answers backup requests. Recommendation: drop that flow (and fm1backup.js, test_backup.mjs) in a later task, keeping the plain restore; until then it fails safely with "does not support complete backups". P3-012 reports it.
