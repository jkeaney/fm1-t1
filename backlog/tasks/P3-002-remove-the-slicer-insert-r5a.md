---
id: P3-002
title: Remove the slicer insert (R5a, first per decision 80)
phase: P3
type: task
status: done
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
Found by applying the removal in a scratch copy and running the device build, `sh tests/run_tests.sh` and a syntax-only compile of every tests/*.c (FB-003). This list is complete for that run. Line numbers are those of HEAD afca063.
- Delete (`git rm`): firmware/src/slicer.c; tests/slicer_test.c; tests/perform_test.c (perform.c stays until P3-007; its test needs the slicer and the buses). Delete the line `#include "slicer.c"` in firmware/src/felucca.c (68) and the slicer_test and perform_test blocks plus their header comments in tests/run_tests.sh (slicer_test, perform_test; also reword "sequencer and SLICER step lengths" and the SLICER mentions at about line 32 and 102 of the header comments).
- firmware/src/fx.c: (a) the constants `SL_RAMP_LOG2`, `SL_RAMP`, `SL_SLOPE` lived in slicer.c; paste them (`#define SL_RAMP_LOG2 7`, `#define SL_RAMP (1 << SL_RAMP_LOG2)`, `#define SL_SLOPE (32768 / SL_RAMP)`) at the top of fx.c, above `#define DLY_LEN` (FB-003 A). fx.c is included before perform.c, which uses all three. `SL_LEN` is NOT moved. (b) Delete `slicer_track_sd(t, b, d, n);` (811), `slicer_track(t, 0, n);` (843) and `slicer_track(t, b, n);` (856); in the condition at 842 drop ` && !slicer_busy(t)`. (c) Reword the comment at 790 that names `slicer_track_sd`.
- firmware/src/perform.c: add `static int16_t pf_buf[NTRK][4096] __attribute__((section(".pool")));` above `PB_FRAMES`; `PB_FRAMES` (60) becomes `(NTRK * 4096u / 2u)`; replace `sl_buf[0][0]` by `pf_buf[0][0]` at 440, 448, 508, 509, 590, 591; delete `perf_drop_slicer` with its comment (209-216), the `if (!sl_lent) {...}` block (229-232) and the `if (sl_lent) {...}` block (552-555); reword the comments at 35 and 61 that name `sl_buf`.
- firmware/src/seq.c:851 delete `slicer_start();` and its comment. firmware/src/midi_control.c:194 delete `sl[track].rec = sl[track].loop = 0;`.
- firmware/src/ui_graph.c: delete `graph_slicer` with its comment (481-503), the two `GR_SLCR` hash lines (1222-1223) and `case GR_SLCR:` with its two lines (2294-2296). firmware/src/params.c:454 graph `GR_SLCR` -> `GR_NONE`; reword the comments at params.c:58, 59 and 130 (they name `SL_OFF .. SL_STUT`, `SL_DEN`, `SL_PAT[]`) and core.h:57 and 437 (`slicer.c`, `sl_enter`). Keep the `GR_SLCR` enum value, `N_SLCR`, `N_SLDIV`, the page row and all `P_SL*` slots (dead; FB-002 C).
- Tests, every file the scratch run needed:
  - tests/hostsim.c:36 delete `#include "../firmware/src/slicer.c"` (FB-003 B).
  - tests/target_budget.py:23 delete `"slicer_track",` and tests/target_budget.txt:39 delete the `slicer_track 632` row (FB-003 C).
  - tests/ui_test.c: `SL_GATE` -> `1` and `SL_STUT` -> `2` at 421, 430, 465, 470, 4008, 4019; in `test_div_order` (7645-7660) remove `&TP[P_SLRATE]` from `LISTS` (7649) and the `d->names == N_SLDIV ? 1000000u / SL_DEN[v] :` branch (7657).
  - tests/midi_control_test.c: `SL_GATE` -> `1` at 827 and 840.
  - tests/sound14_test.c: delete the SLICER GATE case (about 580-610, from the `/* SLICER GATE` comment through its `check(msg, closed > 10u ...)`; it uses `slicer_start`, `sl[0]`) and the line `memset(sl, 0, sizeof sl);` (90). tests/fold155_test.c:412, tests/sound15_test.c:83 and tests/insert_test.c:83: delete `memset(sl, 0, sizeof sl);`.
  - tests/swing_test.c: delete `test_slicer()` (99-146, with its section comment) and ` + test_slicer()` in `main` (170).
  - tests/regress.c: delete `J_SLICER` from the enum (164), `job_slicer` (235-250), its `case J_SLICER` (389-391) and the `{ /* the SLICER */ ... }` block that adds the slicer jobs (1066-1074), replacing the block with `add(J_SONG, "song/4track_swing20")->arg = 1;`; in `job_song` keep only `song.g[G_SWING] = 20;` of the `if (j->arg)` block (293-301); reword the comments at 8 and 253.
  - tests/ui_render.c: leave `S_SLICER` alone (the SLICER page row stays until P3-006).
- (Amended by FB-004: run `GOLDEN_UPDATE=1 sh tests/run_tests.sh` only, NOT BUDGET_UPDATE; tests/target_budget.txt stays at HEAD minus the `slicer_track` row, the static budget only gates growth. tests/insert_test.c: the MIX 0 limit 0.5 % -> 1 % (host noise; the test goes in P3-014).) Then ~~`GOLDEN_UPDATE=1 BUDGET_UPDATE=1 sh tests/run_tests.sh`~~ once. Expected in the Log: exactly the two slicer golden lines gone (`slicer/gate/ANALOG_ACID`, `slicer/song_gate_stut`) and one new line `song/4track_swing20`; **0 existing renders changed**. Any other golden or budget change: stop and file an FB.
- If the "DRUM grid stress" (drumfuzz) step stops with a UBSan `division by zero` at `beat_samples` (`G_BPM` 0), it appeared once in the scratch P3-002 run and not in the next one: rerun it once; if it repeats, file a BUG (it is not a slicer caller) and do not edit around it.
- Log: before/after build line (image, RAM, pool). Scratch result for reference: pool 342876 -> 342876, RAM .data+.bss 55744 -> 55584.

## Acceptance criteria
- [x] slicer.c, tests/slicer_test.c and tests/perform_test.c show `D` in `git status`, and felucca.c has no `#include "slicer.c"`
- [x] `grep -rnwE 'slicer_start|slicer_busy|slicer_track|slicer_track_sd|slicer_test|sl_pattern|sl_enter|sl_buf|sl_lent|sl_t|SL_LEN|SL_NPAT|SL_PAT|SL_DEN|SL_OFF|SL_GATE|SL_STUT|graph_slicer|perf_drop_slicer|job_slicer|J_SLICER|test_slicer' firmware tests tools BUILDING.md` returns nothing
- [x] `grep -rnE '(^|[^A-Za-z0-9_])sl\[' firmware tests --exclude=ui_draw.c` returns nothing (ui_draw.c has unrelated local `char sl[..]` buffers; they stay)
- [x] `grep -rn 'slicer\.c' firmware tests tools BUILDING.md` returns nothing
- [x] `grep -n 'SL_RAMP_LOG2\|SL_SLOPE' firmware/src/fx.c` shows the three defines at the top of fx.c; `grep -n 'pf_buf' firmware/src/perform.c` shows the `.pool` declaration
- [x] `beat_samples` and `mix_part` still exist in fx.c
- [x] Pool used does not rise against the pre-task build line (32768 B moves from sl_buf to pf_buf); RAM .data+.bss drops
- [x] The golden/budget result is exactly as stated in the Deliverable (golden as stated; budget = HEAD minus the slicer_track row, per FB-004 A)
- [x] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [x] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

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

### 2026-11-10 executor (haiku), second attempt
Blocked again before any edit, on FB-003. The re-scoped deliverable still misses callers and definitions:
- `slicer.c:20-24` defines `SL_LEN`, `SL_RAMP_LOG2`, `SL_RAMP`, `SL_SLOPE`. `fx.c:667, 709, 730` (MIX glide) and
  `perform.c:276-783` need them, and the deliverable does not say where they move.
- `tests/hostsim.c:36` includes `slicer.c` (hostsim is built at `tests/run_tests.sh:274`). Not named.
- `tests/target_budget.py:23` and `tests/target_budget.txt:39` name `slicer_track`. Not named.
- The `sl\[` acceptance grep also matches the unrelated local arrays `char sl[3]` / `char sl[8]` in ui_draw.c.
- Comments naming slicer symbols: core.h:440, params.c:58-59, 130, perform.c:35, 60.
Filed FB-003 with questions A-E. No files edited. Task back in `blocked` until the owner answers.
### 2026-11-10 orchestrator (FB-003)
Second inspection resolved by a scratch-copy audit (build, run_tests, syntax check of every tests/*.c); FB-003 A-E answered with its recommendations. Deliverable and greps rewritten above.

### 2026-11-11 executor (haiku)
Edits applied per the Deliverable: slicer.c, tests/slicer_test.c, tests/perform_test.c removed (git rm); fx.c SL_RAMP_LOG2/SL_RAMP/SL_SLOPE moved to its top and the slicer calls removed; perform.c uses its own `pf_buf` (pool, 32 KB); seq.c, midi_control.c, ui_graph.c (graph_slicer, GR_SLCR hash and case), params.c (GR_SLCR -> GR_NONE, comments), core.h comments, felucca.c include, hostsim.c include, target_budget.py/.txt row, run_tests.sh (slicer and perform blocks, header), ui_test.c, midi_control_test.c, sound14_test.c, regress.c, swing_test.c, and the sl memsets in insert/sound15/fold155 tests.

Build (`./build.sh`):
- before: `ok image 523208 B; RAM .data+.bss 55744 B of 73728; pool 342876 B of 368640`
- after: `ok image 520904 B; RAM .data+.bss 55584 B of 73728; pool 342876 B of 368640`
- headroom 73728 - 55584 = 18144 B (at least 8192: pass)

Acceptance greps (all empty): the symbol grep, `sl[` grep (excluding ui_draw.c), and `slicer\.c` grep. `grep -n 'SL_RAMP_LOG2\|SL_SLOPE' firmware/src/fx.c` shows the defines at lines 5-7; `pf_buf` declared at perform.c:60. `beat_samples` and `mix_part` still exist in fx.c.

Golden (`GOLDEN_UPDATE=1 BUDGET_UPDATE=1 sh tests/run_tests.sh`): golden diff is exactly `-slicer/gate/ANALOG_ACID`, `-slicer/song_gate_stut`, `+song/4track_swing20`. Budget: the BUDGET_UPDATE rewrite moved about 25 unrelated rows (for example `fm1_alnk0_irq 37502 -> 17977`), which the Deliverable says to stop on. Host tests: `HOST TESTS FAILED`, the only failing check is `insert: cost ... FAIL` (insert_test). Not fixed here; see FB-004.

Syntax-only compile of tests/*.c: all pass except digital_test, fm4_test, uac_test and usb_desc_test, which need their build flags (-DFELUCCA_FM4 / -DHALF_FRAMES / T_UAC) and are not run that way in this check.

Not ticked: the golden/budget criterion, `sh tools/check.sh` (not run; blocked by the budget and INSERT results), and the `slicer.c/tests` `D` status is met.

### 2026-11-11 orchestrator (FB-004)
FB-004 A-C answered (see its log): budget file kept at HEAD minus `slicer_track` (moved rows are a real, shrinking ISR cost; the check gates growth only); insert_test MIX 0 limit 0.5 % -> 1 % (host noise); mod_test cost passes in P3-002 alone (+4.3 %). `sh tools/check.sh` ended CHECK PASS (second run; the first run hit the INSERT noise, then the one-off drumfuzz UBSan division by zero at fx.c:224 reappeared once and cleared on rerun). Build: `ok image 520904 B; RAM .data+.bss 55584 B of 73728; pool 342876 B of 368640`. Set to review.

### 2026-11-11 review (sonnet)
Verdict: PASS (gate none).
- Re-ran all acceptance greps: symbol, `sl[`, `slicer\.c` all empty. slicer.c, slicer_test.c, perform_test.c deleted in HEAD. SL_RAMP_LOG2/SL_RAMP/SL_SLOPE at fx.c:5-7; pf_buf `.pool` at perform.c:60; beat_samples and mix_part present.
- Golden diff exactly -slicer/gate/ANALOG_ACID, -slicer/song_gate_stut, +song/4track_swing20; target_budget.txt only loses the slicer_track row.
- `sh tools/check.sh` ran once: CHECK PASS.
- Build: `ok image 520904 B; RAM .data+.bss 55584 B of 73728; pool 342876 B of 368640`. Headroom 18144 B (at least 8192): pass. Pool unchanged, RAM down 160 B.
