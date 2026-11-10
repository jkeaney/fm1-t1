---
id: P3-002
title: Remove the slicer insert (R5a, first per decision 80)
phase: P3
type: task
status: todo
priority: 2
executor: haiku
depends_on: [P3-001]
gate: none
---
## Goal
Delete slicer.c and every caller of it (FB-002 A-D), keeping the build green. The fx effect buses follow in P3-014, still ahead of every other removal (owner decision 80). perform.c stays until P3-007, so it gets its own loop buffer here.

## Read first
- backlog/feedback/FB-002-p3-002-slicer-and-fx-removal-has-hidden-callers.md (the decisions this task implements)
- docs/design/SYSTEM_MAP.md §4 rows 13-15, §5 edge 7, §6 row R5
- firmware/src/felucca.c (the include list: include order is the dependency order)
- tests/run_tests.sh

## Deliverable
Do the edits in this order; run `sh tools/check.sh` at the end only.
- Delete: firmware/src/slicer.c; tests/slicer_test.c; tests/perform_test.c (perform.c is removed in P3-007; its tests need the slicer and the buses). Remove the `#include "slicer.c"` line (felucca.c:68) and the run_tests.sh blocks for slicer_test and perform_test (plus their header comments).
- firmware/src/perform.c: it borrows slicer.c's `sl_buf` as its loop buffer. Add `static int16_t pf_buf[NTRK][4096] __attribute__((section(".pool")));` (same size as `sl_buf`, 32768 B; the pool is not zeroed, as before), replace the four `&sl_buf[0][0]` uses (lines 440, 448, 508-509, 590-591) by `&pf_buf[0][0]`, and change `PB_FRAMES` (line 60) to `(NTRK * 4096u / 2u)`. Delete `perf_drop_slicer()` (209-216), the `sl_lent` guard at 229-231 and at 552-554 (keep the code each guard wraps only if it is not about the slicer; if unsure stop), and the comment lines naming sl_buf. pf_buf goes away with perform.c in P3-007.
- firmware/src/fx.c: delete the call sites `slicer_track_sd` (811), `slicer_busy`/`slicer_track` (842-843, 856) and keep the rest of the condition at 842 unchanged apart from dropping `&& !slicer_busy(t)`. The `#include "perform.c"` at fx.c:229 stays.
- firmware/src/seq.c:851: delete `slicer_start();` and its comment.
- firmware/src/midi_control.c:194: delete `sl[track].rec = sl[track].loop = 0;` (and its comment).
- firmware/src/ui_graph.c: delete `graph_slicer` (about 483-540), the `GR_SLCR` hash lines (1222-1223) and `case GR_SLCR:` (2294-2296). In firmware/src/params.c:454 change the SLICER page's graph from `GR_SLCR` to `GR_NONE`. Keep the `GR_SLCR` enum value, the page row, `N_SLCR`, `N_SLDIV` and every `P_SLCR..P_SLDEPTH` parameter slot: they are dead slots now (FB-002 C); the page row goes in P3-006, the slots stay until the v2 params prune.
- Tests, exact edits: replace `SL_GATE` by `1` and `SL_STUT` by `2` in tests/ui_test.c (421, 430, 465, 470, 4008, 4019) and tests/midi_control_test.c (827, 840); tests/sound14_test.c: delete the SLICER GATE case (about 580-610, `slicer_start`, `sl[0]`); tests/swing_test.c: delete `test_slicer()` (99-146) and its call at 170; tests/ui_render.c:1113: delete the `S_SLICER` case and its enum/table entry; tests/regress.c: delete `job_slicer` (232-250), its job-table entries, and the slicer block `if (j->arg) {...}` at 293-300 (delete the job entry whose only difference was that arg). Then `GOLDEN_UPDATE=1 sh tests/run_tests.sh` and `BUDGET_UPDATE=1 sh tests/run_tests.sh` once and list in the Log which golden lines were removed or changed (only slicer cases may go; any other change: stop).
- Not touched (dead slots, FB-002 C): core.h P_SL* enum, params.c descriptors, ui.c:1269, project.c, upreset_test, project_test, descdump.c, regress P_SLCR constants beyond those above.
- Log: before/after build line (image, RAM, pool).

## Acceptance criteria
- [ ] slicer.c, tests/slicer_test.c and tests/perform_test.c show `D` in `git status`, and felucca.c has no `#include "slicer.c"`
- [ ] `grep -rnw -e slicer_start -e slicer_busy -e slicer_track -e slicer_track_sd -e sl_pattern -e sl_enter -e sl_buf -e sl_lent -e SL_LEN -e SL_PAT -e SL_OFF -e SL_GATE -e SL_STUT -e graph_slicer firmware/src tests` returns nothing; `grep -rn 'sl\[' firmware/src tests` returns nothing; `grep -rn GR_SLCR firmware/src` shows only the enum declaration and the page-table comment, if any
- [ ] `beat_samples` and `mix_part` still exist in fx.c; `grep -n 'pf_buf' firmware/src/perform.c` shows the declaration with `section(".pool")`
- [ ] Pool used does not rise against the pre-task build line (32768 B moves from sl_buf to pf_buf); RAM .data+.bss drops (slicer state was 164 B)
- [ ] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [ ] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## Out of scope
- The fx effect buses (P3-014), perform.c itself (P3-007), the P_SL* parameter slots and the SLICER page row (kept as dead slots; P3-006 removes the page row)
- Behaviour changes to the T-1 path (P3-001's test must still pass)
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
### 2026-11-10 executor (haiku)
Blocked before any edit (FB-002). Hidden callers found: fx.c:229 includes perform.c, which uses sl_buf, sl[] and sl_lent; seq.c:851 slicer_start; ui_graph.c GR_SLCR page; midi_control.c:194 sl[track]; many tests.
### 2026-11-10 orchestrator
FB-002 resolved: scope rewritten above (slicer only, perform.c given its own buffer, dead P_SL* slots kept); the fx buses moved to new task P3-014, which P3-003 now depends on.
