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
Found by applying the removal on top of P3-002 in a scratch copy: device build, `sh tests/run_tests.sh` and a syntax-only compile of every tests/*.c (FB-003). Line numbers are those of HEAD afca063 (they move after P3-002).
- firmware/src/fx.c, delete: `DLY_LEN`, `CHO_LEN`, `dly_buf`, `cho_buf`, `REV_COMB`, `REV_AP`, `rev_comb`, `rev_u` with `rev_ap`, the `fx` state struct, the SPRING (`SP_*`) and HALL (`HL_*`, `hl`, `rev_side`, the two `_Static_assert`s) blocks and their comments, `track_dist` (everything between `#define SL_SLOPE` and the "master: peak limiter" comment); `delay_samples`, `rev_room`, `rev_spring`, `sh_tz`, `exp2_neg`, `hall_gains`, `hall_ap`, `hall_end`, `hall_end_lp`, `HL_RUNS`, `HL_LFO`, `rev_hall`, `rev_clear` (everything from `delay_samples` up to `part_buf`); `rev_switch`, `fx_buses`, `rev_side_mix`, and the whole INSERT block (`IT_*`, `IL_LEN`, `INS_HOLD`, `ins_t`, `ins[]`, `ins_mixq`, `ins_lpk`, `ins_shape`, `ins_bits_sh`, `ins_drive`, `ins_swept`, `ins_rest`, `track_insert`) (everything from `rev_switch` up to the "one block of the whole mix" comment).
- fx.c, keep: `SL_RAMP*`/`SL_SLOPE` (P3-002), the master/limiter/USB level/SPEAKER EQ block, `beat_samples`, `div_samples`, `DIV_DOT` (tests/ui_test.c uses it), `part_buf`, `spread_buf` users, `scope_*`, `perf_master`, `mix_spread`, `mix_part`, `mix_block`, and the `send_d[]`/`send_r[]` arrays (perform.c's `perf_throw` and `perf_pre` still take them until P3-007). Edits inside the kept functions: the declaration becomes `static int32_t send_d[CTL], send_r[CTL], mix_l[CTL], mix_r[CTL];` (`send_c` and `wet` go); in `mix_spread` and `mix_part` delete the `c`/`dl`/`r`/`xmax` send gain code, the three `send_*[i] += mulq15(...)` lines each, the `track_dist`/`track_insert` calls and the `t->tail`/`ins_run`/`ins_rest` tail logic (a part with no voice now just runs `mod_end` and returns: `if (!track_render(t, b, n)) { if (mod.on) mod_end(t); return; }`); in `mix_block` and `perf_master` and `scope_take_mix` drop `+ wet[i]`, in `mix_block` zero only `send_d`, `send_r`, `mix_l`, `mix_r`, and delete the `fx_buses(...)` call and the `hl.side`/`rev_side_mix` call. Reword the header comment and the comments naming DIST/INSERT/SLICER (fx.c 3-4, 125, 139, 168, and the "one block of the whole mix" comment).
- firmware/src/core.h: delete `dist_hp, dist_lp1, dist_lp2`, `tail`, `ins_run` from `track_t` (381-383) and change `uint8_t rsv_sz[14]` to `rsv_sz[28]` so `sizeof(track_t)` is unchanged (the existing `_Static_assert` must pass). Keep `preset_t.fx[4]`.
- firmware/src/ui_graph.c: delete `graph_insert` with its comment (about 448-480) and `case GR_INS:` (2269-2271). firmware/src/params.c: graph `GR_INS` -> `GR_NONE` for the INSERT and INSERT 2 page rows (455-456); reword the comments at params.c:15 (`delay_samples`), 62 (`INS_HOLD`), 159 (`track_insert`), 183 (`IT_*`), firmware/src/motion.c:44 (`track_insert`), firmware/src/perform.c:12 (`fx_buses`). Keep `ins_desc`, `N_ITYPE`, `N_RTYPE`, all `P_DIST..P_REV`, `P_ITYPE..P_IMIX`, `G_RTYPE..G_RDAMP` slots, `MD_DIST..MD_REV` in mod.c and midi_control.c CC 91/93/94 (dead slots, FB-002 C).
- Tests: `git rm` tests/insert_test.c and tests/reverb_test.c and delete their run_tests.sh blocks and header comments (the INSERT and REVERB blocks; check nothing else needs build/fx_demo). Delete the three lines `memset(rev_ap, 0, sizeof rev_ap);`, `memset(&fx, 0, sizeof fx);` (and, if still present, the `dly_buf`/`cho_buf`/`rev_comb` memsets) in tests/fold155_test.c (396-398), tests/mod_test.c (83-85), tests/sound14_test.c (74-76) and tests/sound15_test.c (67-69). tests/mod_test.c:24 `MOD_COST_MAX 5.0` -> `6.0` with a comment (the 4-part cost check measures the matrix's extra cost as a percentage of the mix; the mix got cheaper because the buses are gone, the matrix's own cost did not change). tests/ui_render.c: delete `S_INSERT_CRUSH, S_INSERT_FOLD, S_INSERT_PHASER, S_INSERT_OFF, S_INSERT_MIX` from the enum (764), their names `"insert_crush"...` (794), `set_ins_ui` (964-972) and the five `case S_INSERT_*` lines under the comment at 1151. tests/ui_test.c:7658: reword the comment naming `delay_samples`. tests/target_budget.py: delete `"rev_room", "rev_spring", "rev_hall", "rev_side_mix"` and `"track_insert", "ins_drive", "ins_swept"` from `FUNCS` (31-32; keep `"perf_throw"` and close the list with `]`) and tests/target_budget.txt the rows `rev_room`, `rev_spring`, `rev_hall`, `rev_side_mix`, `track_insert`, `ins_drive`, `ins_swept`.
- Then `GOLDEN_UPDATE=1 BUDGET_UPDATE=1 sh tests/run_tests.sh` once. **The preview sound changes**: the presets carry CHO/DLY/REV sends, so every render that had them changes (95 of 97 golden lines in the scratch run, including 12 `preset/ANALOG/*` and the 4 `mode/ANALOG`). This is the intended effect of owner decision 80, not a regression: the Log lists the count of changed golden lines by group (ANALOG / other) and says so. `union_test` hashes follow `GOLDEN_UPDATE`.
- Log: before/after build line (image, RAM, pool). Scratch result for reference: pool 342876 -> 192808 (150068 B freed), RAM .data+.bss 55584 -> 55248, image 520904 -> 515492.

## Acceptance criteria
- [ ] tests/insert_test.c and tests/reverb_test.c show `D` in `git status`
- [ ] `grep -rnwE 'dly_buf|cho_buf|rev_comb|rev_u|rev_ap|track_dist|track_insert|fx_buses|rev_clear|rev_side_mix|rev_side|rev_switch|rev_room|rev_spring|rev_hall|delay_samples|ins_bits_sh|ins_mixq|ins_shape|ins_lpk|ins_drive|ins_swept|ins_rest|ins_t|graph_insert|dist_hp|dist_lp1|dist_lp2|ins_run|hall_gains|hall_ap|hall_end|hall_end_lp|IT_OFF|IT_SOFT|IT_HARD|IT_FOLD|IT_FUZZ|IT_CRUSH|IT_PHASER|IT_FLANGER|IT_CHORUS|IT_N|DLY_LEN|CHO_LEN|REV_COMB|REV_AP|INS_HOLD|IL_LEN|send_c|set_ins_ui|S_INSERT_CRUSH|S_INSERT_FOLD|S_INSERT_PHASER|S_INSERT_OFF|S_INSERT_MIX|insert_test|reverb_test' firmware tests tools BUILDING.md` returns nothing
- [ ] `grep -rnE '(^|[^A-Za-z0-9_])(hl|fx)\.(dly|cho|comb|sp_|rtype|side|sd|lp|le)' firmware/src tests` returns nothing; `grep -n 'wet\[' firmware/src/fx.c` returns nothing (the word `tail` is NOT grepped: unrelated uses abound)
- [ ] `beat_samples`, `div_samples`, `DIV_DOT`, `mix_part`, `mix_block`, `spk_bass`, `master_out` still exist in fx.c
- [ ] Pool used drops by at least 140000 B against the pre-task build line (owner constraint: the hardware-support design needs pool for the +2016 B preset store; do not reduce this floor)
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
