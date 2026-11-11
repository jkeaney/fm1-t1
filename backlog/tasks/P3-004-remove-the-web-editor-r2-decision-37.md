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
Scope rehearsed in a scratch copy (FB-005): device build, syntax-only compile of every tests/*.c, one full `sh tests/run_tests.sh`. Do exactly this; the list is complete.
- `git rm` firmware/src: editor.c, editor_backup.c, editor_fm6.c, editor_learn.c, editor_menu.c, editor_palette.c, editor_preferences.c
- `git rm` web: editor.html, EDITOR_PROTOCOL.md, test_web.mjs (it tests editor.html and its mock device)
- `git rm` tests: editor_test.c, fuzz_ed.c, backup_test.c, fm6bank_test.c, fuzz_proj.c, fuzz_smp.c (both `#include "editor_test.c"`), descdump.c (only feeds the removed web test), **robust_test.c** (see below)
- KEEP (do not delete): web/fm1backup.js and web/test_backup.mjs (the installer page index_pkg.html inlines fm1backup.js for its "return to official V15" backup flow; make_site.py keeps inlining it), web/fm1ota.js, fm1pkg.js, **web/fukiai.ttf and web/FUKIAI-LICENSE.txt** (tools/gen_aa_icons.py and gen_panel_diagram.py read web/fukiai.ttf at build time; LICENSING.md lists both), ota.c, tools/fm1_install.py
- Edit callers:
  - firmware/src/felucca.c: delete `#include "editor.c"` (line ~109). Comments there naming the editor may stay.
  - firmware/src/main.c: delete the `ed_service();` line in the `#if FELUCCA_OTA` block (keep `ota_service()`), and delete the whole `#if FELUCCA_OTA` / `ed_service();` / `#endif` in the 15 ms input loop (3 lines), leaving `ui_input();` in the loop.
  - firmware/src/usb.c ~798: comment `ota_take() / ed_service()` becomes `ota_take()`.
  - tools/size_fns.py: drop "editor.c", "editor_preferences.c", "editor_backup.c", "editor_menu.c" and "editor_fm6.c" from `SIZE_FILES` and the word "editor" from the comment above it. **Without this ./build.sh crashes** (FileNotFoundError in size_fns.py).
  - tests/persistence_test.c: delete the editor stubs and includes (the `enum { ED_UI_STATE...` line through `#include ".../editor_menu.c"`, ~lines 61-69) and the two opening blocks of `main()` that call `ed_ui_set` and `ed_menu_handle` (from the `{` before `const uint8_t amber[]` through the closing `}` after `settings_hold = HOLD_DEF; settings_leds = LEDS_DIM;`, ~lines 436-500). Keep `autosave_hold()` and everything else. After the cut, `main()` must still compile; remove now-unused locals only if the compiler warns. The settings-deferral cases those blocks covered are lost for now; settings_test.c remains.
  - tests/run_tests.sh: delete the run blocks for backup_test, fm6bank_test, editor_test, robust_test, descdump (compile line, echo line and run line), fuzz_ed, fuzz_proj, fuzz_smp, and `web/test_web.mjs`; change the ASan loop to `for t in project_test fun10_test; do`; keep the `web/test_backup.mjs` run inside the `if command -v node` block (it must not end up as an empty branch). Update the header comments (lines ~39-44, 136-137, 158-165) that describe the removed tests.
  - web/make_site.py: remove the editor copy (`ed` dir, `shutil.copy(HERE / "editor.html"...)`, the fukiai/fm1backup copy loop to `ed`), the `webapp/editor` line in the docstring and in the final print. Keep the inlined `fm1backup.js` in `lib`.
  - web/index_pkg.html: delete the `<p><a href="../editor/" ... data-t="editor">` line (52) and the `editor:` entry in both language tables (67, 98). Do not touch the `stockBackup`/backup strings.
  - web/emu/index.html: delete the `<a href="../editor/">Editor</a>` link (line 102).
  - BUILDING.md only if it names the editor page (grep; today it does not).
- robust_test.c: it is built on `#include "editor_test.c"` (hostsim, stubs, RAM flash, `reset()`), so it cannot be split in place; its other parts test sample slots, projects and user presets, all removed by P3-009/P3-011. Delete the whole file. Note the interim loss of that coverage in the Log.
- Not edited on purpose: tests/ui_test.c:3807 and other comments that say "editor"; firmware comments mentioning the editor (they die with their files in later tasks); tools/sampleio.py, gen_samples.py, gen_fm6_patches.py comments; docs/design/*.
- Expected results (scratch rehearsal at HEAD 339a70d): build line before `image 515492 B; RAM .data+.bss 55248 B of 73728; pool 192808 B of 368640`, after `image 500212 B; RAM .data+.bss 54064 B of 73728; pool 191048 B of 368640` (freed: 15280 B image, 1184 B RAM, 1760 B pool). RAM headroom after: 19664 B. Golden: 97 renders, 0 changed. Target budget: no row change needed (no editor row in target_budget.txt; do not run BUDGET_UPDATE or GOLDEN_UPDATE). If your numbers differ, say so in the Log and explain.
- Log: before/after build line and the bytes freed vs SYSTEM_MAP §9; the lost-coverage note; whether the full run_tests.sh result matches the rehearsal (rehearsal: ALL HOST TESTS PASSED, 0 budget rows over, test_backup.mjs passed, emulator skipped for lack of emcc).

## Acceptance criteria
- [ ] Every file listed under `git rm` is gone (`git status` shows `D`); `grep -n 'editor.c' firmware/src/felucca.c` is empty
- [ ] `grep -rnE 'ed_service|ed_handle|ed_ui_set|ed_menu_handle|ed_backup_handle|ed_fm6_handle|editor_test\.c' firmware tests tools web build.sh` returns nothing except comments in tests/ui_test.c
- [ ] `grep -n 'editor' tools/size_fns.py web/make_site.py web/index_pkg.html web/emu/index.html tests/run_tests.sh` has no file name, link or test run left (comments naming past history are fine)
- [ ] web/fm1backup.js, web/test_backup.mjs, web/fukiai.ttf, web/FUKIAI-LICENSE.txt still exist; `node web/test_backup.mjs` passes
- [ ] `sh tests/run_tests.sh` still runs ota_test and ldr_test and both pass; `python3 web/make_site.py build/felucca.fwsc dev /tmp/site-check` succeeds (if the package identity check allows) and writes no `webapp/editor`
- [ ] Syntax-only compile of every tests/*.c that compiled before still compiles (uac_test and usb_desc_test need run_tests.sh flags; skip them)
- [ ] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [ ] The build line is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## Out of scope
- Any subsystem SYSTEM_MAP marks KEEP or ADAPT beyond the caller edits named here
- The installer's "return to official V15" backup flow (index_pkg.html + fm1backup.js): it asks the device for a SysEx backup that the cleaned firmware no longer answers. Kept as is here; recorded in FB-005 as a follow-up for the owner. Do not change it in this task.
- Behaviour changes to the T-1 path (P3-001's test must still pass)
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log

### 2026-11-11 executor (claude-haiku-5-5)

Stopped before any deletion. The Deliverable does not name files that the removal cannot leave behind. Nothing was edited; the tree is unchanged.

Missing from the Deliverable (found by grep, not guessed):

1. **tests/fm6bank_test.c** includes and calls `ed_fm6_handle`, `ed_fm6_name` and the `ED_FM6B_*` enum, all defined in `firmware/src/editor_fm6.c`, which is in the Delete list. The test is run at `tests/run_tests.sh:316-317`. It is not named under Tests.
2. **tests/fuzz_proj.c** and **tests/fuzz_smp.c** do `#include "editor_test.c"`, and fuzz_smp calls `ed_service()` (line 57). They are run at `run_tests.sh:429-432`. Deleting editor_test.c breaks their build. SYSTEM_MAP §8 marks them "go" under R6/R7, not R2.
3. **web/test_web.mjs** (editor protocol, run at `run_tests.sh:443`) and **web/test_backup.mjs** (run at `:444`) test the deleted editor.html and fm1backup.js. Neither is named.
4. **web/make_site.py** copies `editor.html` into the site (lines 10, 58, 77). **web/index_pkg.html** links to `../editor/` (line 52) and says "use the editor" (line 104). Neither is named. The deliverable's "keep web/ OTA files" does not say whether the editor link and site copy go.
5. **tests/run_tests.sh** also references the editor in the ASan list (`:423`, `:427-428`, fuzz_ed) and in comments (`:39`, `:43`, `:136-137`, `:158-165`). The deliverable names only the test files, not the run_tests.sh edits.
6. **firmware/src/usb.c:798** has a comment `ed_service()`. The acceptance criterion `grep -rn 'ed_service' firmware/src` must be empty, so this comment needs editing. The comment is not a caller edit named in the Deliverable. Comments in felucca.c (lines 14, 35, 89, 98) mention "editor" and are harmless.
7. **tests/persistence_test.c**, **tests/ui_test.c** and others only have their own copies or comments. These are fine but should be checked by the reviewer.
8. **tests/robust_test.c**: the "malformed requests" part is not delimited in the file. The Deliverable says "the malformed-request part of robust_test" but the test file has `ED_SMP_END` / `ED_BACKUP_PUT` requests at lines 122-128 and the test is run as one step. The split must be specified.

Also not yet measured: the before build line (`./build.sh | grep RAM`). Not run because the task is blocked before any change.

Needed decision: name the files for items 1-6 and 8 in the Deliverable (or accept the proposed scope: delete fm6bank_test, fuzz_proj, fuzz_smp, web/test_web.mjs and web/test_backup.mjs; drop the editor link and copy in make_site.py and index_pkg.html; edit the usb.c comment; split robust_test at lines 122-128). Then re-run this task.

Filed as blocked. No bug or feedback file is written here because the spec is incomplete rather than contradicted; the owner or orchestrator can choose the route.
