---
id: P3-010
title: Remove the remaining engines and eng_mem; ANALOG only (R6c)
phase: P3
type: task
status: done
priority: 2
executor: haiku
depends_on: [P3-009]
gate: none
---
## Goal
Leave exactly one engine, ANALOG preset 0, as the fixed preview synth (SYSTEM_MAP section 2, decision 40). Rehearsed on top
of 007 + 011 + 008 + 009; the change exists as a verified patch (regenerated on HEAD d03c6c8 after BUG-004).

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
   cap term, `chk_voice_cap` deleted, `chk_sample_end` was already gone; audio_test.c: the one-shot drum overload case is already deleted by P3-008;
   mod_test.c: the TRIO cases and the E-name case deleted (its cost test is already ANALOG-only from P3-008);
   hostsim.c: the pad is poly (`V_POLY`: ANALOG preset 0 is mono), the lead is ANALOG, the mix cost block uses ANALOG parts and the
   engine-switch crossfade run (`xfade_test`) is dropped (nothing to switch between); midi_control.c `MIDI_CC_ENG` has one row; midi_control_test.c: power-on parts all ANALOG, the WHEEL CC case
   deleted; project_test.c: engine 0 in the round trip. sound14_test and sound15_test pass unchanged (ANALOG cases).
   run_tests.sh: noise_test and fold155_test runs and comments. golden.txt keeps 24 lines (ANALOG presets, modes, sends,
   song); cpu_baseline.txt the ANALOG rows and the mixes; target_budget.py/.txt keep `analog_render`, the two ISRs,
   `uac_tap48`, `click_render`, `mod_*`.
4. Golden: the patch already holds the accepted hashes (`song/4track_mix` and `song/4track_swing20` changed once more: the demo
   song's tracks 2 and 3 were PHASE and LOFI, they are ANALOG now; 24 renders, 0 health failures). After the patch and the deletes
   run `sh tests/run_tests.sh` once, no GOLDEN_UPDATE: expected ALL HOST TESTS PASSED. Any golden line that differs is a real
   sound change or a host difference: stop and file a BUG. No BUDGET_UPDATE. The `fm1_alnk0_irq` and `mod_begin` budget lines print a note or stay within tolerance (rehearsal: -17 %,
   157 vs 163): not failures.

## Acceptance criteria
- [x] The eight files in P3-010.deletes.txt are gone; no `#include` of them remains in engines.c or tests
- [x] `grep -rnEI 'eng_(phase|lofi|trio|wheel|formant|noise)|ENG_(PHASE|LOFI|TRIO|WHEEL|FORMANT|NOISE|MEM|FM4_GONE)|ENG_MEM|eng_mem|phase_render|lofi_render|trio_render|wheel_render|formant_render|noise_render|eng_vis|eng_rank|eng_step|ENGINE_ORDER|ENGI_DIGITAL' firmware/src tests tools web LICENSING.md` prints nothing (comment hits: reword them, the rehearsal left some in core.h 18/178-180 and tests/regress.c 13)
- [x] `grep -c ENG_MEM firmware/src/*.c` prints 0 for every file
- [x] Pool used drops by about 51552 B (rehearsal: 144052 to 92500)
- [x] Build line `image 169084 B; RAM .data+.bss 23060 B of 73728; pool 92500 B of 368640` (on top of 007 + 011 + 008 + 009)
- [x] `sh tools/check.sh` ends CHECK PASS; RAM headroom at least 8192 B (rehearsal: 50668); the Log names the two golden lines

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
- Patch fallback: if `git apply --check` fails only on comment or context-only hunks (no code line differs), you may apply with `git apply --3way` (or `patch` with fuzz) and must list in the Log which hunks differed. Any failing code hunk is still a stop: file a BUG (as BUG-004, BUG-005).

## Log

### 2026-11-11 executor (claude-haiku-5-5)

**Patch fallback used.** `git apply --check docs/design/rehearsal/P3-010.patch` failed on two deleted comment lines in
`firmware/src/engines.c` (hunk @4 and @145). The patch's `-` lines read `... convert to FM6, fm4_convert.c)` and
`/* SLICE (eng_slice.c): note = C4 + slice */`, but HEAD says `... converted to FM6 until P3-009)` and
`/* SLICE (removed in P3-009): note = C4 + slice */`. Both are removed comments, so no code line differs. I applied a copy
with those two `-` lines set to the HEAD text (`/tmp/P3-010-fixed.patch`, not committed). `--3way` failed (blob missing).
Every other file in the patch matched the old side exactly.

Deletes: `git rm` of the eight files in P3-010.deletes.txt (`D` in `git status`).

Comment rewording (acceptance grep): `firmware/src/core.h` lines 18 and 178-180 (the `ENGINE_ORDER` and `eng_mem` comments)
and `tests/regress.c` line 13 (`eng_formant.c formant_nz`).

Verification:
```
$ git apply --check docs/design/rehearsal/P3-010.patch        -> fails on engines.c comment lines (see above)
$ grep -rnEI '<acceptance pattern>' firmware/src tests tools web LICENSING.md   -> no output (exit 1)
$ grep -c ENG_MEM firmware/src/*.c                            -> 0 for every file
$ sh tools/check.sh                                            -> CHECK PASS (exit 0)
  build/check/build.log: ok    image 169084 B; RAM .data+.bss 23060 B of 73728; pool 92500 B of 368640
  RAM headroom: 73728 - 23060 = 50668 B (criterion: at least 8192)
  pool: 144052 -> 92500 = -51552 B
  tests/tests.log: regress: 24 golden renders (0 changed, 0 gone), 0 health failures, 0 voice / routing checks failed,
                   0 CPU entries over budget
  tests/tests.log: target: fm1_alnk0_irq ... cost 14373 (budget 17319) note: -17 % (not a failure)
  tests/tests.log: ALL HOST TESTS PASSED
$ sh tools/check.sh also: backlog stage passed
```
Golden lines named by the patch's accepted hashes: `song/4track_mix a5f5f88872a0cfa4` (tests/golden.txt line 25) and
`song/4track_swing20 81316d9df95fddf4` (line 26). Their change was accepted in the patch and both render unchanged on this run.

Notes for the reviewer:
- `engine_t.mem` (`firmware/src/core.h` line 180) is now unused. The patch did not remove the field, and the task's scope
  does not include removing struct fields, so I left it.
- `./build.sh | grep RAM` was run through `sh tools/check.sh`, which builds the same image; the RAM line is above.

Acceptance: all six criteria verified above and ticked.

### 2026-11-11 review (sonnet)
Verdict: PASS (gate none).
- `sh tools/check.sh` run once: CHECK PASS. Build line `image 169084 B; RAM .data+.bss 23060 B of 73728; pool 92500 B of 368640`
  (headroom 50668 B, over 8192). Matches expected.
- Acceptance grep prints nothing; `grep -c ENG_MEM firmware/src/*.c` is 0 everywhere; the eight files are deleted.
- Patch fallback: logged. The two differing hunks (engines.c deleted comment lines only) are named; I read the final engines.c
  and it is equivalent to the intended removal: `ENGINES[] = {&ENG_ANALOG}`, `eng_idx()` clamps any stored engine number to 0
  (project.c uses `ENGINES[e]` after that clamp), `PATTERNS[]` is 9 long, `TRK_DEF` ANALOG presets 4,0,1,0, `MIDI_CC_ENG` one row.
- Golden: only `song/4track_mix` (a5f5f88872a0cfa4) and `song/4track_swing20` (81316d9df95fddf4) changed; the other removed lines
  are the deleted engines' presets/modes. 26 lines remain (24 renders + header lines); 0 changed, 0 health failures.
- Budget: target_budget.txt/.py and cpu_baseline.txt show deletions only (FUNCS list shortened); no BUDGET_UPDATE.
- Preview design: ANALOG presets (ACID LEAD, BASS, PAD, STRINGS, PIANO-type) are untouched in eng_analog.c; nothing made impossible.
- `engine_t.mem` left unused deliberately: accepted.
- Non-blocking observation: eng_analog.c presets RAVE `PAT(13)` and SUB BASS `PAT(8)` hold pattern numbers; `.pat` is not read
  anywhere, so PAT(13) (now beyond the 9-entry PATTERNS, ARP is entry 9) is harmless but stale. Tidy in the v2 params prune.
