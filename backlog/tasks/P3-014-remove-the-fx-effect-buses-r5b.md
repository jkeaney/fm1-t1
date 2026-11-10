---
id: P3-014
title: Remove the fx effect buses (R5b, second per decision 80)
phase: P3
type: task
status: todo
priority: 2
executor: haiku
depends_on: [P3-002]
gate: none
---
## Goal
Free about 145 KB of pool (dly_buf 131072 + rev_comb 9874 + cho_buf 4096 + HALL/INSERT state) so the 16 preset slots, HARMONY, ARP and PATTERNS fit (owner decision 80). Split out of P3-002 by FB-002. perform.c stays until P3-007: fx.c keeps the two send scratch arrays it feeds.

## Read first
- backlog/feedback/FB-002-p3-002-slicer-and-fx-removal-has-hidden-callers.md
- docs/design/SYSTEM_MAP.md §4 row 14, §6 row R5, §9
- firmware/src/fx.c lines 1-230 (buffers), 480-780 (buses, INSERT, HALL), 783-945 (mix_part, mix_block)
- tests/run_tests.sh

## Deliverable
- firmware/src/fx.c delete: the buffers `dly_buf`, `cho_buf`, `rev_comb`, `rev_u`, the HALL state `hl` and `hall_*`, `ins[]`; the functions `track_dist`, `track_insert`, `fx_buses`, `rev_clear`, `rev_side_mix`, `delay_samples`, `ins_bits_sh`, `ins_mixq`, `ins_shape` (and any helper whose only callers were these: check each with grep before deleting); the `wet[]` and `send_c[]` arrays and their uses in `mix_part`/`mix_block` (the `fx_buses(...)`, `hl.side` and `wet[i]` terms); the DIST/INSERT/tail logic at fx.c:842 and the `t->tail`/`ins_run` assignments.
- Keep in fx.c: `mix_part`, `mix_block`, `beat_samples`, `div_samples`, master level, SPEAKER EQ (`fx_lowcut`, `spk_bass`, `master_out`), USB level, and the `send_d[]` / `send_r[]` scratch arrays (perform.c's `perf_throw` and `perf_pre` still take them until P3-007; they no longer feed any bus). Keep `perf_master`, `perf_begin`, `perf_block` calls as they are.
- firmware/src/core.h: remove `dist_hp, dist_lp1, dist_lp2`, `tail`, `ins_run` from `track_t` and grow `rsv_sz` by the bytes removed so `sizeof(track_t)` is unchanged (the existing `_Static_assert` must still pass). Keep `preset_t.fx[4]` (used by project.c:792 and ui.c:1377).
- firmware/src/ui_graph.c: delete `graph_insert` (about 450-480) and `case GR_INS:`; firmware/src/params.c: set the graph of the INSERT and INSERT 2 page rows (lines 455-456) to `GR_NONE`. Keep `ins_desc`, `N_ITYPE`, `N_RTYPE`, the `P_DIST..P_REV`, `P_ITYPE..P_IMIX` and `G_RTYPE/G_RSIZE/G_RDAMP` slots, `mod.c` `MD_DIST..MD_REV`, and midi_control.c's CC 91/93/94 table: they are dead slots now (FB-002 C); pages go in P3-006, slots in the v2 params prune.
- Comment-only mentions of `delay_samples` (params.c:15, tests/ui_test.c:7658) are reworded to "the dotted divisions".
- Tests: delete tests/insert_test.c, tests/reverb_test.c and their run_tests.sh blocks (insert block about 370-373, reverb about 390-392, header comments 104-128; check nothing else needs `build/fx_demo`). Delete the `memset(dly_buf ...)`, `memset(cho_buf ...)`, `memset(rev_comb ...)` lines in tests/fold155_test.c:396-398, mod_test.c:83-85, sound14_test.c:74-76, sound15_test.c:67-69. In tests/ui_render.c delete the INSERT page cases (about line 1151 onward, the CRUSH/FOLD/PHASR/INSERT 2 renders). `speaker_test`, `audio_test`, `mod_test` must pass with no assertion edits: if one fails, stop and file an FB. Then `GOLDEN_UPDATE=1 sh tests/run_tests.sh` and `BUDGET_UPDATE=1 sh tests/run_tests.sh` once; the Log lists the golden lines that changed (only cases with CHO/DLY/REV/DIST/INSERT sends non-zero may change; presets send 0 by default so none should).
- Log: before/after build line (image, RAM, pool) and bytes freed vs SYSTEM_MAP §9.

## Acceptance criteria
- [ ] tests/insert_test.c and tests/reverb_test.c show `D` in `git status`
- [ ] `grep -rnw -e dly_buf -e cho_buf -e rev_comb -e rev_u -e track_dist -e track_insert -e fx_buses -e rev_clear -e rev_side_mix -e delay_samples -e ins_bits_sh -e ins_mixq -e ins_shape -e graph_insert -e dist_hp -e ins_run firmware/src tests` returns nothing; `grep -rn 'hl\.' firmware/src/fx.c` returns nothing
- [ ] `beat_samples`, `mix_part`, `mix_block`, `spk_bass`, `master_out` still exist in fx.c
- [ ] Pool used drops by at least 140000 B against the pre-task build line
- [ ] `sizeof(track_t)` unchanged (the `_Static_assert` passes)
- [ ] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [ ] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## Out of scope
- perform.c (P3-007), the dead parameter slots and page rows named above, the preview synth's sound (sends are 0 by default, so it must not change)
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
