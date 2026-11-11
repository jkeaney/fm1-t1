---
id: P3-008
title: Remove the DRUM and PHYS engines (R6a)
phase: P3
type: task
status: done
priority: 2
executor: haiku
depends_on: [P3-011, P2-023]
gate: none
---
## Goal
Remove the drum kit and physical models (sound design, not preview). Runs after P3-011 (order change, see P3-011).
Rehearsed on top of P3-007 + P3-011; the change exists as a verified patch. The patch was regenerated on HEAD d03c6c8 after BUG-004 (two hunks drifted after P3-011) and now also carries the test fixes and the accepted goldens that the first rehearsal left to the executor.

## Read first
- docs/design/rehearsal/P3-008.patch, P3-008.deletes.txt
- docs/design/SYSTEM_MAP.md section 6 row R6, section 8, section 9

## Deliverable
1. `git apply --check docs/design/rehearsal/P3-008.patch` (stop and file a BUG if it fails), `git apply`, then `git rm` the
   files in P3-008.deletes.txt: eng_drum.c, drum_voice.c, eng_phys.c, phys_dsp.c, phys_symp.c; tests/drum_test.c, phys_test.c,
   phys_fixed.c, phys_ref.cpp; LICENSES/MIT-DaisySP.txt, MIT-Rings.txt.
2. What the patch does. **Gap found by the rehearsal: the table is positional.** `ENGINES[]` indices are stored by
   projects/presets and used as ENGI_* constants, so a deleted engine's row becomes `&ENG_FM4_GONE` (the retired-engine
   placeholder from fm4_convert.c; P3-009 moves it into engines.c; P3-010 deletes the table down to ANALOG). `ENGINE_ORDER`
   and `NENG_SHOWN` lose the two engines (`NENG_SHOWN` is `NENGINES - 3`); `eng_ok(e)` is now "the row is not the
   placeholder"; `TRK_DEF[3]` (the power-on track 4) is ANALOG preset 0 (was the DRUM kit).
   **eng_drum.c also owned the step-lane helpers**: `DRUM_LANE_NOTE[]` moves to seq.c next to `grid_key_note` (steps
   keep their `hit`/`acc` bits; `NLANE` stays); `drum_track()` is deleted with its branches (seq.c rec_note drum branch,
   midi_control.c x2, voice.c `drum_reuse` and the bend special case, ui.c `drum_mute`, `step_to_grid`, `ENGI_DRUM` favourite
   and PC paths, settings_persist.c SAMPLE PERC favourites migration); core.h drops `ENGI_PHYS`, `ENGI_DRUM`, `drum_from_phys`,
   `drum_from_perc`, `DRUM_KIT_E`; params.c drops `N_DRUM_KIT`/`DK_PLAYS` from `enum_orig` / `param_fit`.
   midi_control.c `MIDI_CC_ENG` rows 9 and 10 become 0.
3. Tests: hostsim.c `host_drums()` is ANALOG preset 0 (all render fixtures that used the drum part now use ANALOG);
   regress.c deletes `job_drums`, the `drums/*` and `cpu/DRUM/*` jobs; audio_test.c, scale_test.c (the DRUM and kit
   cases), midi_control_test.c (CC74 on DRUM, drum pedal and bend, the DRUM PC block), settings_test.c (the PERC to DRUM
   favourite migration), ui_test.c (`lane_steps`, the DRUM LED case), union_test.c (PHYS, DRUM out of `UE`) and
   speaker_test.c: the kick pattern is generated in the test (`dv_*` is gone; HD_MAX still passes) are edited. run_tests.sh
   loses the PHYS, DRUM runs and comments. tests/golden.txt loses `preset/PHYS/*`, `preset/DRUM/*`, `drums/*` (22 lines),
   tests/cpu_baseline.txt the `cpu/PHYS/*`, `cpu/DRUM/*` rows, tests/union_golden.txt the sequences naming engine 9 or 10
   (396 sequences left), tests/target_budget.py/.txt the 19 functions named in the old text (`phys_render` ... `dv_metal_mix`).
4. LICENSING.md: the DRUM voices row, the DaisySP and Rings rows and the "ports" sentence; web/emu/felucca_web.c: the
   `ENGI_PHYS` use (now engine 2). (The `upreset.c` include is already gone: P3-011 removed it, so the patch has no hunk for it.)
5. **Test fixes beyond the rehearsal's first cut (BUG-004 regeneration found them by running the whole suite; all are in the patch):**
   hostsim.c: the clap-into-the-DRUM-lane check is dropped (a melodic part's recording replaces the step; `ok_clap` leaves the
   fail count); audio_test.c: the "held one-shot drum" overload case is deleted; mod_test.c: the cost test uses ANALOG BRASS and
   an all-ANALOG mix (`PHYS` and the mixed engines are gone) and `MOD_COST_MAX` is 7.0 (the cheaper mix makes the same slot cost
   a larger share: 6.0 % measured); tests/golden.txt `song/4track_mix` and `song/4track_swing20` and tests/union_golden.txt
   already hold the accepted new hashes (the demo song's track 4 is ANALOG now; the union sequences were regenerated).
   **After applying the patch and the deletes, run `sh tests/run_tests.sh` once: expected ALL HOST TESTS PASSED with no
   GOLDEN_UPDATE.** A golden line that differs (any `preset/ANALOG/*` or `mode/ANALOG/*` especially) is a real sound change or a
   host difference: stop and file a BUG. **Do not set BUDGET_UPDATE** (it also rewrites tests/target_budget.txt, FB-004 A).
   The target budget step may print a `note` for `fm1_alnk0_irq` (cheaper, about -30 %): expected, not a failure.

## Acceptance criteria
- [x] The files in P3-008.deletes.txt show `D`; no `#include` of them remains in felucca.c / engines.c / tests
- [x] `grep -rnEI 'ENGI_(DRUM|PHYS)|ENG_(DRUM|PHYS)|drum_track|drum_voice|eng_drum|eng_phys|phys_dsp|phys_symp|\bdv_[a-z_]+|\bpx_(modal|memb|string|symp)[a-z_]*|drum_from_(phys|perc)|\bN_DRUM_KIT|DK_(PLAYS|COUNT)' firmware/src tests tools web LICENSING.md BUILDING.md` prints nothing (a hit left in a comment: reword it, one line each, so nothing remains; the regenerated patch leaves such comments in core.h 6/63, params.c 148, seq.c 970 and tests/hostsim.c 137; hits in eng_sample.c:8 and tools/gen_samples.py:23 are files P3-009 deletes: leave them)
- [x] Build line `image 347112 B; RAM .data+.bss 34340 B of 73728; pool 144720 B of 368640` (regenerated on HEAD d03c6c8; the old rehearsal said 347096)
- [x] `sh tools/check.sh` ends CHECK PASS; RAM headroom at least 8192 B; the Log lists the golden lines that changed
- [x] The Log states that no BUDGET_UPDATE was run and what was written by hand into cpu_baseline.txt, if anything

## Verification
```sh
git apply --check docs/design/rehearsal/P3-008.patch
sh tools/check.sh
./build.sh | grep 'RAM'
```

## Out of scope
- Any subsystem SYSTEM_MAP marks KEEP or ADAPT beyond the caller edits named here; the step `hit`/`acc` fields and `NLANE`
- Behaviour changes to the T-1 path (P3-001's test must still pass)
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log

### 2026-11-11 executor (claude-haiku-5-5)
- Read CLAUDE.md, VISION.md, this task, and the Read-first files. Set in_progress.
- Verification step 1 failed: `git apply --check docs/design/rehearsal/P3-008.patch` exits 1 on HEAD 0f080ac:
```
error: patch failed: firmware/src/core.h:18
error: firmware/src/core.h: patch does not apply
error: patch failed: web/emu/felucca_web.c:202
error: web/emu/felucca_web.c: patch does not apply
```
  Other files in the patch pass the check. Cause: the patch was rehearsed on a base before P3-011. HEAD core.h has
  the P3-011 comment on UP_SLOTS, and HEAD felucca_web.c no longer includes upreset.c (P3-011 deleted it).
- Per Deliverable step 1 ("stop and file a BUG") I applied nothing and deleted nothing. Filed backlog/bugs/BUG-004-p3-008-patch-does-not-apply-to-head.md.
- Not run: `git apply`, `git rm`, `sh tools/check.sh`, `./build.sh`, the golden/budget steps. Nothing else was changed.
- Status set to blocked until BUG-004 is fixed.

### 2026-11-11 executor (claude-haiku-5-5)
- BUG-004 is fixed on HEAD (commit c6a6a16; BUG-004 status done). The previous blocked attempt is superseded.
- Verification, decisive lines:
  - `git apply --check docs/design/rehearsal/P3-008.patch`: exit 0.
  - `git apply docs/design/rehearsal/P3-008.patch` applied, then `git rm` of every line of P3-008.deletes.txt (11 files, all show `D`).
  - Comment rewording: core.h 6 and 63, params.c 148, seq.c 970, tests/hostsim.c 137 reworded to drop the removed names (one line each, no code change). Left as allowed: eng_sample.c:8, tools/gen_samples.py:23.
  - Grep from the criteria: only `firmware/src/eng_sample.c:8` and `tools/gen_samples.py:23` match (both allowed, P3-009 deletes them).
  - `grep '#include.*(eng_drum|eng_phys|drum_voice|phys_)'` over firmware, tests, web: no output.
  - `sh tools/check.sh`: `CHECK PASS`. RAM line: `RAM .data+.bss 34340 B of 73728` (headroom 39388 B, above the 8192 B floor).
  - Build line: `ok    image 347112 B; RAM .data+.bss 34340 B of 73728; pool 144720 B of 368640` (matches the criterion exactly).
  - Tests: `regress: 75 golden renders (0 changed, 0 gone), 0 health failures ...`, `union_test: 396 engine switch sequences, 0 differ`, `target: fm1_alnk0_irq ... note: -32 %` (the expected note), `ALL HOST TESTS PASSED`.
- Golden lines that changed (accepted in the patch, no GOLDEN_UPDATE run): `tests/golden.txt` adds `song/4track_mix 0537ab5e4a6a39a0` and `song/4track_swing20 7935db39f87dee54` (replacing the old hashes 351764e3e2056e34 and 5117b1b1fd9c8c40). The demo song's track 4 is ANALOG now. All `preset/ANALOG/*` and `mode/ANALOG/*` lines are unchanged.
- No BUDGET_UPDATE was run. tests/cpu_baseline.txt and tests/target_budget.txt are changed only by the patch (deletions of PHYS, DRUM, dv_, px_ rows). Nothing was hand-written into cpu_baseline.txt.
- Not touched: restore/, .venv/, no flashing, no push.

### 2026-11-11 review (sonnet)
Verdict: PASS (gate none).
- `sh tools/check.sh` run once: CHECK PASS. Build line `image 347112 B; RAM .data+.bss 34340 B of 73728; pool 144720 B of 368640` (headroom 39388 B, above 8192 B).
- Acceptance grep: only firmware/src/eng_sample.c:8 and tools/gen_samples.py:23 hit, both allowed (P3-009 deletes them).
- golden.txt: the only non-PHYS/DRUM/drums lines changed are song/4track_mix and song/4track_swing20; no ANALOG lines changed.
- target_budget.txt and cpu_baseline.txt: deletions only. target_budget.py: name-list deletions only. No BUDGET_UPDATE.
- mod_test.c MOD_COST_MAX 6.0 -> 7.0: justified (all-ANALOG mix is cheaper per sample, so the same slot cost is a larger share). It is recorded in Deliverable item 5 and in the code comment; the executor Log entry does not repeat it (minor).
