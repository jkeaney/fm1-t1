---
id: P3-010
title: Remove the remaining engines and eng_mem; ANALOG only (R6c)
phase: P3
type: task
status: todo
priority: 2
executor: haiku
depends_on: [P3-009]
gate: none
---
## Goal
Leave exactly one engine, ANALOG preset 0, as the fixed preview synth (SYSTEM_MAP section 2, decision 40). Rehearsed on top
of 007 + 011 + 008 + 009; the change exists as a verified patch.

## Read first
- docs/design/rehearsal/P3-010.patch, P3-010.deletes.txt; docs/design/SYSTEM_MAP.md section 6 row R6, sections 8 and 9

## Deliverable
1. `git apply --check docs/design/rehearsal/P3-010.patch`, `git apply`, then `git rm` the files in P3-010.deletes.txt:
   eng_phase.c, eng_lofi.c, eng_trio.c, eng_wheel.c, eng_formant.c, eng_noise.c; tests/noise_test.c, fold155_test.c.
2. What the patch does: engines.c deletes the `eng_mem` pool buffer (51552 B), `eng_mem_claim`, `ENGINE_ORDER`, `eng_ok/_vis/
   _rank/_step`, the `ENG_FM4_GONE` placeholder and the four SLICE/DRUM factory patterns (`PATTERNS[]` is 9 long);
   `ENGINES[] = {&ENG_ANALOG}`, `NENGINES` 1, `NENG_SHOWN` 1, `eng_idx()` still returns 0; `TRK_DEF` is ANALOG presets
   4, 0, 1, 0; voice.c and main.c lose their `eng_mem_claim` calls; params.c `N_ENGNAME` = {"ANALOG"};
   midi_control.c `MIDI_CC_ENG` has one row; ui_menu.c `MENU_CREDITS` keeps only Hugelton, the Fukiai font and Inter Tight
   (the CrispyZebra, KLATTSCH, DaisySP, Dexed/msfa, Rings, VSCO and VOICE credits are for removed code); LICENSING.md
   loses the PHASE (CrispyZebra) and klattsch rows. The page definitions in params.c for the other engines were
   `ENGINES[e]->edit[]` tables inside the deleted files, so nothing else is pruned (the shared TP[] slots stay).
3. Tests (run-time breakages the syntax compile does not show; the patch fixes them): hostsim.c defines a local `eng_ok`
   (`e < NENGINES`) for the loops that used it; regress.c: `chk_budget` uses ANALOG presets 1, 2, 3, 0 and has no VOICE
   cap term, `chk_voice_cap` deleted, `chk_sample_end` was already gone; audio_test.c: the one-shot drum overload case deleted;
   mod_test.c: the TRIO cases and the E-name case deleted; midi_control_test.c: power-on parts all ANALOG, the WHEEL CC case
   deleted; project_test.c: engine 0 in the round trip. sound14_test and sound15_test pass unchanged (ANALOG cases).
   run_tests.sh: noise_test and fold155_test runs and comments. golden.txt keeps 24 lines (ANALOG presets, modes, sends,
   song); cpu_baseline.txt the ANALOG rows and the mixes; target_budget.py/.txt keep `analog_render`, the two ISRs,
   `uac_tap48`, `click_render`, `mod_*`.
4. Golden: run `GOLDEN_UPDATE=1 sh tests/run_tests.sh` once. **Expected: exactly two lines change, `song/4track_mix` and
   `song/4track_swing20`** (the demo song's tracks 2 and 3 were PHASE and LOFI; they are ANALOG now); the rehearsal's regress
   run showed those two and nothing else, 24 renders, 0 health failures. Any other change is a real sound change: stop. No
   BUDGET_UPDATE. The `fm1_alnk0_irq` and `mod_begin` budget lines print a note or stay within tolerance (rehearsal: -17 %,
   157 vs 163): not failures.

## Acceptance criteria
- [ ] The eight files in P3-010.deletes.txt are gone; no `#include` of them remains in engines.c or tests
- [ ] `grep -rnEI 'eng_(phase|lofi|trio|wheel|formant|noise)|ENG_(PHASE|LOFI|TRIO|WHEEL|FORMANT|NOISE|MEM|FM4_GONE)|ENG_MEM|eng_mem|phase_render|lofi_render|trio_render|wheel_render|formant_render|noise_render|eng_vis|eng_rank|eng_step|ENGINE_ORDER|ENGI_DIGITAL' firmware/src tests tools web LICENSING.md` prints nothing (comment hits: reword them, the rehearsal left some in core.h 18/178-180 and tests/regress.c 13)
- [ ] `grep -c ENG_MEM firmware/src/*.c` prints 0 for every file
- [ ] Pool used drops by about 51552 B (rehearsal: 144052 to 92500)
- [ ] Build line `image 169068 B; RAM .data+.bss 23060 B of 73728; pool 92500 B of 368640` (on top of 007 + 011 + 008 + 009)
- [ ] `sh tools/check.sh` ends CHECK PASS; RAM headroom at least 8192 B (rehearsal: 50668); the Log names the two golden lines

## Verification
```sh
git apply --check docs/design/rehearsal/P3-010.patch
sh tools/check.sh
./build.sh | grep 'RAM'
```

## Out of scope
- The dead parameter slots and the shared TP[] table (the v2 params prune, SYSTEM_MAP row 10)
- Any subsystem SYSTEM_MAP marks KEEP or ADAPT beyond the caller edits named here
- Behaviour changes to the T-1 path (P3-001's test must still pass)
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
