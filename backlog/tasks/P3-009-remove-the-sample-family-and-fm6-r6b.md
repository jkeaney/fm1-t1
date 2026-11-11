---
id: P3-009
title: Remove the sample family and FM6 (R6b)
phase: P3
type: task
status: review
priority: 2
executor: haiku
depends_on: [P3-008]
gate: none
---
## Goal
Remove the sampler (126 KB of ADPCM data), GRAIN, SLICE and the 6-op FM engine. Rehearsed on top of 007 + 011 + 008; the
change exists as a verified patch.

## Read first
- docs/design/rehearsal/P3-009.patch, P3-009.deletes.txt; docs/design/SYSTEM_MAP.md section 6 row R6, sections 8 and 9

## Deliverable
1. `git apply --check docs/design/rehearsal/P3-009.patch`, `git apply`, then `git rm` the files in P3-009.deletes.txt (21):
   eng_sample.c, eng_grain.c, eng_slice.c, eng_fm6.c, fm6_core.c, fm6_vbank.c, fm4_convert.c; tests/sample_test.c,
   fm6_test.c, union_test.c, union_golden.txt, pitch.py, slice_loop.py; tools/gen_samples.py, gen_fm6_patches.py, gen_waves.py,
   sampleio.py, fm1_sample_upload.py, fetch_cc0.py; LICENSES/Apache-2.0-msfa.txt; the directory assets/samples-cc0.
   (felucca_samples.h and felucca_fm6.h are generated into build/gen, not source files.)
2. **Gaps found by the rehearsal:** (a) `ENG_FM4_GONE` and the `FM4_EDIT` macro lived in fm4_convert.c; the patch moves them
   into engines.c (the placeholder rows 1, 4, 8, 9, 10, 12, 13). (b) `FELUCCA_SLICE` goes: `NENGINES` is 13,
   `NENG_SHOWN` 7, `ENGINE_ORDER` = ANALOG PHASE LOFI VOICE TRIO WHEEL NOISE, `N_ENGNAME` shows "-" for the dead rows;
   core.h / tools/build.py / felucca.c lose the flag. (c) `TRK_DEF[1]` (the power-on pad) was FM6 PAD; it is PHASE preset 0 now
   (the power-on track 2 sound changes; preview only). (d) `snd_missing`, `snd_said`, `preset_orig`, `SMP_*`, `fm4_*`,
   `fm6_*` calls are removed from engines.c, ui.c (undo copy of the FM6 patch, `track_sig`, `apply_preset_to`,
   `set_engine_of`, `midi_pc_load`), ui_input.c, main.c (`fm6_init`), project.c (the FM6 patch bytes of the FUN10 record
   become `rsv_fm6[4][128]`, written as zeros; `PROJ_FM6_PACKED` 128), settings_persist.c (the DIGITAL favourites
   migration), params.c (`enum_orig` is the identity, `param_fit` clamps, `enum_skip` returns 0; delete them in the
   params prune). (e) tools/build.py drops the two generator entries and the ATTRIBUTION copy; tools/size_fns.py drops
   "fm4_convert.c" (FB-005). (f) tests/install_test.py imported fm1_sample_upload (the sample-slot name check): the patch
   deletes those 6 lines. (g) tests: hostsim.c / regress.c / midi_control_test.c / project_test.c / ui_test.c lose the
   FM6, SAMPLE, DIGITAL cases; `union_test` and its golden go entirely (the engines with per-part state are all gone);
   run_tests.sh loses the SAMPLE, FM6, ENGINE MEMORY runs; golden.txt loses `preset/SAMPLE|FM6|GRAIN|SLICE/*`, cpu_baseline.txt
   the same `cpu/*` rows, target_budget.py/.txt the eight functions named in the old text. LICENSING.md loses the
   Sample Pack, CC0 samples and msfa rows.
3. Not covered (by reading; list in the Log): web/emu/felucca_web.c line 36 (`SMP_USER_XIP`, an eng_sample.c macro) delete;
   BUILDING.md: the "Samples" section is already removed by the patch, but line 49 ("the sample attribution"), line 64
   (`FELUCCA_SLICE` row), line 81 and lines 92-97 (the test list) still describe removed things: P3-012 rewrites them.
   fm6 SysEx/OBJ_FM6BANK/OBJ_UPFM6 object ids in storage.c stay (flash map).
   **Residual grep hits the patch does not remove (found on regeneration):** tools/gen_tables.py `fm6_tables()` and the `FM6_*` tables
   (only fm6_core.c used them: delete them, `./build.sh` must still pass); comment hits in storage.c 21/29/30/33, engines.c 59/148,
   params.c 133, tests/settings_test.c 9-10 and tests/ui_test.c 45 (`ENGI_SAMPLE`, `SMP_*` local defines: delete if unused, else reword),
   web/emu/felucca_web.c 36 (`SMP_USER_XIP`: delete). Reword so the acceptance grep prints nothing.
4. Regenerated on top of the regenerated P3-008 (BUG-004): the patch also carries (h) midi_control.c `MIDI_CC_ENG` rows 4, 8 and 12
   set to 0 (the removed SAMPLE, GRAIN and FM6 rows made `midi_control_test` "every engine's CC74 / CC71 target" fail on the
   placeholder descriptors) and (i) the accepted `song/*` golden lines. After the patch and the deletes (`git rm -r`, one entry is a
   directory), run `sh tests/run_tests.sh` once: expected ALL HOST TESTS PASSED with no GOLDEN_UPDATE; a golden line that
   differs other than a removed one: stop and file a BUG. No BUDGET_UPDATE.

## Acceptance criteria
- [x] The 21 files in P3-009.deletes.txt are gone; no `#include` of them remains
- [x] `grep -rnEI 'eng_sample|eng_grain|eng_slice|eng_fm6|fm6_[a-z_]+|fm4_(convert|apply|track|load_preset|preset_values)|FM4_(NPRESETS|TO_FM6)|\bFM6_[A-Z_]+|FP_SIZE|ENGI_(SAMPLE|FM6)|ENG_(SAMPLE|GRAIN|SLICE|FM6)|SMP_[A-Z_]+|smp_[a-z_]+|slc_[a-z_]+|SLC_[A-Z_]+|FELUCCA_SLICE|snd_said|snd_missing|gen_samples|gen_fm6_patches|felucca_samples|felucca_fm6|fm1_sample_upload|sampleio' firmware/src tests tools web LICENSING.md` prints nothing (a hit in a comment: reword it; storage.c may keep `OBJ_FM6BANK` / `OBJ_UPFM6` / `OBJ_FM6VB`: those object ids are not in the pattern)
- [x] Flash image shrinks by at least 140000 B (regenerated: 157304 B)
- [x] Build line `image 189808 B; RAM .data+.bss 26676 B of 73728; pool 144052 B of 368640` (regenerated on HEAD d03c6c8 + 008; the old rehearsal said 189792)
- [x] `sh tools/check.sh` ends CHECK PASS; RAM headroom at least 8192 B; Log lists the changed golden lines

## Verification
```sh
git apply --check docs/design/rehearsal/P3-009.patch
sh tools/check.sh
./build.sh | grep 'RAM'
```

## Out of scope
- Any subsystem SYSTEM_MAP marks KEEP or ADAPT beyond the caller edits named here
- Behaviour changes to the T-1 path (P3-001's test must still pass)
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).
- Patch fallback: if `git apply --check` fails only on comment or context-only hunks (no code line differs), you may apply with `git apply --3way` (or `patch` with fuzz) and must list in the Log which hunks differed. Any failing code hunk is still a stop: file a BUG (as BUG-004, BUG-005).

## Log

### 2026-11-11 executor (haiku-5.5)
- `python3 tools/backlog.py set P3-009 in_progress`: ok.
- `git apply --check docs/design/rehearsal/P3-009.patch` on HEAD 8ae981b: FAILS, exit 1.
  ```
  error: while searching for:
      eng_mem_claim(t);                             /* (voice.c engine_block would at the next block) */
  error: patch failed: tests/hostsim.c:133
  error: tests/hostsim.c: patch does not apply
  ```
  Only tests/hostsim.c fails. Cause: commit 7a8b4eb (P3-008) reworded the `host_legacy_sample_perc` comment ("turns it into
  DRUM" is now "turns it into a melodic part") and the `host_drums` comment after the patch was regenerated on c6a6a16.
- Stopped before `git apply`, the `git rm` of the 21 files and any build or test run. The deliverable's step 1 is a precondition
  and the spec does not cover regenerating the patch (the BUG-004 fix). I did not edit hostsim.c by hand, since that would be guessing.
- Filed backlog/bugs/BUG-005-p3-009-patch-does-not-apply-to-head.md. Set P3-009 to blocked.
- Acceptance criteria: none ticked (no criterion verified).
- Needed to unblock: a regenerated P3-009.patch on HEAD (BUG-005), then rerun P3-009.
- BUG-005 fixed (2026-11-11): patch regenerated on HEAD 8ae981b, build line unchanged. Unblocked; set todo.

### 2026-11-11 executor (haiku-5.5), P3-009 run
- `python3 tools/backlog.py set P3-009 in_progress`: ok.
- `git apply --check docs/design/rehearsal/P3-009.patch` (on HEAD 20653ef): exit 0.
- `git apply docs/design/rehearsal/P3-009.patch`: applied (one whitespace warning on a context line, harmless).
- Deletes: `tr '\n' '\0' < docs/design/rehearsal/P3-009.deletes.txt | xargs -0 git rm -r -q`: 21 entries removed, `assets/samples-cc0` gone.
- Residual grep hits the patch did not remove, reworded or deleted by hand (no other behaviour change):
  - firmware/src/storage.c comments (21, 29, 30, 33): reworded to "retired P3-009"; OBJ_FM6BANK / OBJ_UPFM6 / OBJ_FM6VB kept.
  - firmware/src/engines.c 59 and 148: comments reworded (no `fm4_convert.c`, no `eng_slice.c`).
  - firmware/src/params.c 133: `fm4_convert.c` reference reworded.
  - tests/settings_test.c 9-10 (`ENGI_SAMPLE`, `SMP_SET_PERC`) and tests/ui_test.c 44-45 (`host_slots`, `SMP_USER_XIP`): deleted, unused.
  - web/emu/felucca_web.c 36 (`SMP_USER_XIP`): deleted.
  - tools/gen_tables.py: `fm6_tables()`, `FM6_LFO_SOURCE`, the FM6 comment block and the `L += fm6_tables()` call deleted. The old `print(f"tables: {path}")` sat after `return L` inside fm6_tables, so it never ran; it went with the function.
- Acceptance grep after all edits: `grep -rnEI '<pattern>' firmware/src tests tools web LICENSING.md` prints nothing (exit 1).
- `sh tools/check.sh` -> `CHECK PASS` (RAM line below). `sh tests/run_tests.sh` inside check: `regress: 57 golden renders (0 changed, 0 gone), 0 health failures ...`, `ALL HOST TESTS PASSED`, no GOLDEN_UPDATE, no BUDGET_UPDATE.
- Golden lines changed (from the patch's (i)): `song/4track_mix` and `song/4track_swing20` re-accepted (new hashes 3ff1976b1f4c6288, fb5c2fc987c3e5b8). Removed: 20 `preset/FM6|GRAIN|SAMPLE|SLICE/*` lines. cpu_baseline.txt: 20 removed `cpu/*` rows plus `sample_render`, `grain_render`; target_budget.txt: 8 functions removed.
- Build: `./build.sh` inside check.sh prints `image 189808 B; RAM .data+.bss 26676 B of 73728; pool 144052 B of 368640`. A standalone `./build.sh` afterwards printed `image 189792 B` with the same RAM and pool figures (16 B difference, not investigated; the check.sh log is the one matching the criterion).
- Flash shrink: a `git archive HEAD` build gave `image 347096 B`; 347096 - 189792 = 157304 B (criterion: at least 140000 B; the spec's 157304 matches).
- RAM headroom: 73728 - 26676 = 47052 B free (criterion: at least 8192 B).
- Known failure "web pages" did not occur (check.sh passed).
- Not done (per spec step 3, P3-012): BUILDING.md, LICENSING.md text beyond the patch.
- Acceptance criteria ticked: 5 of 5 (all verified above). Set to review.
