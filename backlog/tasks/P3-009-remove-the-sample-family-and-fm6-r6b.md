---
id: P3-009
title: Remove the sample family and FM6 (R6b)
phase: P3
type: task
status: todo
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
4. As in P3-008 step 5: run `GOLDEN_UPDATE=1 sh tests/run_tests.sh` once, review the diff (only `song/*`, `sends/*` may
   change), no BUDGET_UPDATE; hand-edit cpu_baseline rows only if regress prints OVER BUDGET for a `cpu/mix/*` row.

## Acceptance criteria
- [ ] The 21 files in P3-009.deletes.txt are gone; no `#include` of them remains
- [ ] `grep -rnEI 'eng_sample|eng_grain|eng_slice|eng_fm6|fm6_[a-z_]+|fm4_(convert|apply|track|load_preset|preset_values)|FM4_(NPRESETS|TO_FM6)|\bFM6_[A-Z_]+|FP_SIZE|ENGI_(SAMPLE|FM6)|ENG_(SAMPLE|GRAIN|SLICE|FM6)|SMP_[A-Z_]+|smp_[a-z_]+|slc_[a-z_]+|SLC_[A-Z_]+|FELUCCA_SLICE|snd_said|snd_missing|gen_samples|gen_fm6_patches|felucca_samples|felucca_fm6|fm1_sample_upload|sampleio' firmware/src tests tools web LICENSING.md` prints nothing (a hit in a comment: reword it; storage.c may keep `OBJ_FM6BANK` / `OBJ_UPFM6` / `OBJ_FM6VB`: those object ids are not in the pattern)
- [ ] Flash image shrinks by at least 140000 B (rehearsal: 157304 B)
- [ ] Build line `image 189792 B; RAM .data+.bss 26676 B of 73728; pool 144052 B of 368640` (on top of 007 + 011 + 008)
- [ ] `sh tools/check.sh` ends CHECK PASS; RAM headroom at least 8192 B; Log lists the changed golden lines

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

## Log
