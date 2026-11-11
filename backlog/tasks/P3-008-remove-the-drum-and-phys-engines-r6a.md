---
id: P3-008
title: Remove the DRUM and PHYS engines (R6a)
phase: P3
type: task
status: todo
priority: 2
executor: haiku
depends_on: [P3-011, P2-023]
gate: none
---
## Goal
Remove the drum kit and physical models (sound design, not preview). Runs after P3-011 (order change, see P3-011).
Rehearsed on top of P3-007 + P3-011; the change exists as a verified patch.

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
   `ENGI_PHYS` use (now engine 2) and the upreset include.
5. **Not covered by the patch (the rehearsal did not run the full suite after this step):** `GOLDEN_UPDATE=1 sh tests/run_tests.sh`
   once. Expected: `union_test` reports every remaining sequence changed (the sequences share one random generator, so
   removing two engines moves all hashes): accept; `regress` may report `song/*` and the mix rows changed (the demo song's
   track 4 is ANALOG now). Review `git diff tests/golden.txt tests/union_golden.txt`: only `song/*`, `sends/*` and the union
   lines may change; any `preset/ANALOG/*` or `mode/ANALOG/*` change means a real sound change: stop and file a BUG.
   `tests/cpu_baseline.txt`: if regress prints `CPU OVER BUDGET` for `cpu/mix/idle_drums` or `cpu/mix/3parts_full_drums`,
   write the printed instruction count into those two rows by hand. **Do not set BUDGET_UPDATE** (it also rewrites
   tests/target_budget.txt, FB-004 A). The target budget step prints a `note` for `fm1_alnk0_irq` (cheaper, about -30 %):
   expected, not a failure.

## Acceptance criteria
- [ ] The files in P3-008.deletes.txt show `D`; no `#include` of them remains in felucca.c / engines.c / tests
- [ ] `grep -rnEI 'ENGI_(DRUM|PHYS)|ENG_(DRUM|PHYS)|drum_track|drum_voice|eng_drum|eng_phys|phys_dsp|phys_symp|\bdv_[a-z_]+|\bpx_(modal|memb|string|symp)[a-z_]*|drum_from_(phys|perc)|\bN_DRUM_KIT|DK_(PLAYS|COUNT)' firmware/src tests tools web LICENSING.md BUILDING.md` prints nothing (a hit left in a comment: reword it, one line each, so nothing remains; the rehearsal left such comments in core.h 6/63, params.c 148, seq.c 970)
- [ ] Build line `image 347096 B; RAM .data+.bss 34340 B of 73728; pool 144720 B of 368640` (rehearsal, on top of 007 + 011)
- [ ] `sh tools/check.sh` ends CHECK PASS; RAM headroom at least 8192 B; the Log lists the golden lines that changed
- [ ] The Log states that no BUDGET_UPDATE was run and what was written by hand into cpu_baseline.txt, if anything

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
