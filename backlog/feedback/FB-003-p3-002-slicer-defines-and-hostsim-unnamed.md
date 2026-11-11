---
id: FB-003
title: P3-002 deliverable misses the SL_ defines fx.c needs, the hostsim include, and the budget table
phase: P3
type: feedback
status: done
priority: 2
executor: human
depends_on: [P3-002]
gate: none
session: 2026-11-10, HEAD afca063, source inspection by executor (haiku); no edits made, no build run
---
## Goal
Decide how P3-002 removes slicer.c without breaking the build. A second inspection after FB-002 found callers and
definitions that the re-scoped deliverable does not name, so the task cannot be done as written.

## Read first
- backlog/tasks/P3-002-remove-the-slicer-insert-r5a.md
- backlog/feedback/FB-002-p3-002-slicer-and-fx-removal-has-hidden-callers.md
- firmware/src/slicer.c:20-62 (the defines and the `sl_t`/`sl[]` state)

## Observation
1. **The mix-glide and perform ramp constants live in slicer.c.** `slicer.c:20-24` defines `SL_LEN`, `SL_RAMP_LOG2`,
   `SL_RAMP` and `SL_SLOPE`. `fx.c:667, 709, 730` (`-SL_SLOPE, SL_SLOPE` in the MIX glide) use `SL_SLOPE`, and
   `fx.c:605` names it in a comment. Deleting slicer.c leaves fx.c with undefined symbols. The P3-002 deliverable
   does not say where these move. The acceptance grep does not list them, so the grep cannot catch this.
2. **`tests/hostsim.c:36` includes `../firmware/src/slicer.c`.** hostsim is built by `tests/run_tests.sh:274`
   (`DSP render`, `TRACKS` lines 281-283). The deliverable does not name hostsim.c.
3. **`perform.c` also uses `SL_RAMP`, `SL_RAMP_LOG2` and `SL_SLOPE`** (perform.c:276, 377-783). Its `PB_FRAMES`
   (line 60) uses `SL_LEN`. The deliverable replaces `PB_FRAMES` by a literal, but not the ramp macros.
4. **`tests/target_budget.py:23` and `tests/target_budget.txt:39`** name `slicer_track` in the cost table
   (`run_tests.sh:487`). The deliverable does not name these files. Without an edit, the budget check either
   fails or silently skips the row.
5. **Acceptance grep false positive.** `grep -rn 'sl\[' firmware/src tests` also matches `firmware/src/ui_draw.c:332-379`
   and `1165, 1199` (`char sl[3]`, `char sl[8]`), which are unrelated local buffers in the status-bar and song
   code. The criterion cannot pass as written unless those names change, which is outside the task.
6. **Comments that the acceptance grep matches.** `core.h:440` (`slicer.c sl_enter`), `params.c:58, 130`
   (`SL_OFF .. SL_STUT`, `SL_PAT[]`), `params.c:59` (`SL_DEN`), `perform.c:35, 60` (`sl_buf`). Only some of these are
   named in the deliverable (perform.c's `sl_buf` lines are named, core.h and params.c comments are not).
7. **`ui_help.c:24` ("SLICER" help row) and `icons.c:161`** are UI strings with no symbol. They are not a blocker, but
   they are outside the deliverable.

## Questions for the owner
A. Where do `SL_LEN`, `SL_RAMP_LOG2`, `SL_RAMP` and `SL_SLOPE` go? Options: (a) move them to fx.c above the MIX glide
   (fx.c is included before perform.c, so perform.c can use them); (b) move them to core.h beside the `P_SL*` enum.
   Recommendation: (a), since fx.c owns the mix glide and P3-014 keeps it.
B. Add `tests/hostsim.c` to P3-002: delete its `#include "../firmware/src/slicer.c"` and check that hostsim still
   compiles without a slicer. Recommendation: yes.
C. Add `tests/target_budget.py` and `tests/target_budget.txt` (the `slicer_track` row) to P3-002. Recommendation: yes;
   the row goes, and BUDGET_UPDATE is run once.
D. The acceptance grep for `sl\[`: exempt `ui_draw.c` local `sl[...]` arrays, or rename them in P3-002? Recommendation:
   exempt them (the criterion is about the slicer state `sl[]`, not local names).
E. Comments naming slicer symbols (core.h:440, params.c:58-59, 130; perform.c:35, 60): edit them in P3-002 so the
   grep is clean, or leave them and narrow the grep? Recommendation: edit them (one-line comment changes).

## Deliverable
- Owner answers A-E recorded in the log, with P3-002 amended to match (or re-scoped).

## Acceptance criteria
- [ ] Owner answers A-E, and the P3-002 deliverable is amended to name every file and symbol above, or the task is
      re-scoped.

## Verification
```sh
python3 tools/backlog.py check
```

## Log
- 2026-11-10 executor (haiku): P3-002 set blocked on this finding. No source or test files edited. The task's
  deliverable order was not started (step 3 of the task), so the working tree is clean.
- 2026-11-10 orchestrator (sonnet): audit done in scratch copies of HEAD (git worktree + tarball in /tmp, not the working tree): applied P3-002, then P3-002+P3-014; ran ./build.sh, `sh tests/run_tests.sh`, and `cc -fsyntax-only` over every tests/*.c. Results: both device builds pass (P3-002: pool 342876 unchanged, RAM 55584; +P3-014: pool 192808, RAM 55248). Extra breakages found beyond FB-003: sound14_test/fold155_test/sound15_test/insert_test `memset(sl, ...)`; ui_test `test_div_order` (`SL_DEN`, `P_SLRATE` list); fold155/mod/sound14/sound15 `rev_ap`/`fx` memsets; ui_render `S_INSERT_*`; mod_test `MOD_COST_MAX` 5.0 fails at 5.4 % because the mix got cheaper; 95/97 goldens change with the buses gone (the preview sound changes, per decision 80; SYSTEM_MAP row 14 corrected); one UBSan `division by zero` in beat_samples in drumfuzz appeared once in the P3-002 run (not in the P3-014 run): P3-002 says rerun once, file a BUG if it repeats.
  - A. Yes (a): `SL_RAMP_LOG2`, `SL_RAMP`, `SL_SLOPE` move to the top of fx.c; `SL_LEN` becomes the literal 4096 in perform.c's `pf_buf`.
  - B. Yes: tests/hostsim.c:36 include deleted.
  - C. Yes: target_budget.py:23 and target_budget.txt:39 (`slicer_track`) go; P3-014 also removes the 7 bus/insert rows; BUDGET_UPDATE once.
  - D. Exempt `ui_draw.c` (grep uses `--exclude=ui_draw.c`, word-boundary pattern).
  - E. Edit the comments (list in P3-002); acceptance greps are word-boundary and cover comments.
  - P3-002 and P3-014 deliverables and greps rewritten from the audit list; P3-002 set to todo. No locked decision is changed (decision 80's sound change is its stated intent; no new owner question).
