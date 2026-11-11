---
id: P3-007
title: Remove song chain, motion, chord and perform (R4)
phase: P3
type: task
status: todo
priority: 2
executor: haiku
depends_on: [P3-006]
gate: none
---
## Goal
Remove the groovebox sequencing features (song chain, motion/automation, chord keys, the FX hold layer "perform") after
cutting their callers (SYSTEM_MAP section 5 edges 5, 9, 16). **This task was rehearsed end to end in a scratch copy
(2026-11-11, orchestrator, FB-008): the whole change exists as a verified patch. Apply it, then verify.** Decision
(FB-008 point 1, option A): the stored FUN10 record keeps its byte layout; the song chain and motion bytes become
opaque reserved arrays (`rsv_song` 84 B, `rsv_auto` 388 B) that loads ignore and saves write as zeros. P3-011 removes
the whole record layout later, so nothing else is kept for them.

## Read first
- backlog/feedback/FB-008-p3-007-r4-project-format-and-test-scope-undecided.md (the decisions)
- docs/design/rehearsal/P3-007.patch and P3-007.deletes.txt (the verified change) and P3-007.acceptance-grep.sh
- docs/design/SYSTEM_MAP.md section 6 row R4, section 8 (test fate), section 9 (bytes)

## Deliverable
1. `git apply --check docs/design/rehearsal/P3-007.patch` must pass on the tree you start from (it was made against the
   tree after P3-006, HEAD 06d0a96 plus the backlog files). If it does not, stop: set `blocked` and file a BUG, do not
   hand-merge.
2. `git apply docs/design/rehearsal/P3-007.patch`, then delete the files in docs/design/rehearsal/P3-007.deletes.txt with
   `git rm`: firmware/src/song_chain.c, motion.c, chord.c, perform.c, tests/chord_test.c, tests/song_probe.h
   (nothing includes song_probe.h; chord_test.c is the only test of chord.c).
3. What the patch changes, by file (line ranges are HEAD's). Read each hunk; do not add anything else:
   - firmware/src/felucca.c 72-78: drop `#include "song_chain.c"`. seq.c 176-182 and 222-234: drop `#include "chord.c"`
     and `"motion.c"`; fx.c 120-126: drop `#include "perform.c"`; tests/hostsim.c 37-43: drop the song_chain include.
   - core.h 59-65: the P_CHRD / P_VOIC comment says retired (the parameter slots STAY); 293-324: delete `MOTION_MAX`,
     `MOTION_LOCK`, `MOTION_ID`, `motion_event_t`, `motion_store_t`, `CHAIN_ROWS`, `CHAIN_SILENT`, `chain_row_t`,
     `chain_config_t`, `chain_row_v9_t`, `chain_v9_t` and their two `_Static_assert`s.
   - seq.c: 11-18 comment; 36-42 `kb_chn[27]` moves here (it was in chord.c) and `midi_local_held` gets a forward
     declaration; 176-182 / 222-234 as above plus `isr_guard()` / `isr_unguard()` (the interrupt mask that
     `motion_guard()` was; used by ui.c and project.c) and `seq_steps()` (returns `t->step`; song_chain.c had it);
     228 `rec_on` and 364-370 `click_wanted` lose `!chain.running`; 657-749 delete `perf_key`, `kb_fx`, `perf_key_press`,
     `perf_key_remap` and rewrite `key_on` / `key_off` for one note per key (no chord); 767-822 `keyboard_block` loses the
     layer/FX branch (`kb_layer`, `kb_mask`, `kb_lock`, `perf_mask`, `perf_remap`); 829-854 `seq_start` loses
     `motion_begin`, `chain_start`, `perf_start`; 879-886 `seq_stop` loses `chain_stop`, `motion_end`; 1064-1088 delete
     `chain_motion()` and its use in `seq_play`; 1147-1153 and 1185-1191 drop `chain.running` / `chain.carry`;
     1319-1325 drops `!chain.armed`; 1346-1352 drops `memset(mchord...)`; 1407-1413 drops `chain_tick`.
   - fx.c 5-11, 120-143, 151-201, 215-257: no `perf_*` calls, no `pf.mute` / `pf.thr`, `perf_master` deleted, the
     `send_d` / `send_r` scratch arrays deleted (P3-014 kept them only for perform), `mix_block` is the plain master path.
   - main.c 99-105: no `chain_defaults`. midi_clock.c 22-29: no `motion_begin` on Continue. midi_control.c 51-129: chords
     gone (`midi_note_held`, `midi_local_held` use `kb_chn`/`kb_note`, `midi_release`, `midi_play` one note); 176-182 and
     206-213: no `motion_capture` (a CC just sets the parameter); 233-239 `mchord_forget` call gone. params.c 38-44 comment.
   - project.c: 7-114 header comment and defines (the FUN10 layout is kept; `PROJ_CHAIN_BYTES` 84, `PROJ_MOTION_BYTES`
     388 and `PROJ_CHAIN_V9` 36 replace the types), 124-148 `project_t` holds `rsv_song` / `rsv_auto`, `project_v6_t`
     holds `uint8_t chain[36]`; 226-232 `proj_ok` drops `chain_valid`/`motion_valid`; 318-383 `proj_fm4` / `proj_perc`
     lose their motion filtering; 531-557 `proj_import_old` loses chain conversion; 619-704 `proj_pack` writes the
     reserved bytes, `proj_motion_ids` / `proj_motion_v9` deleted, `proj_unpack` skips the bytes; 854-928 `project_capture`
     uses `isr_guard`, `trk[i].p[j]` for `motion_base_value`; 1063-1073 `project_restore_runtime`; 1186-1240 delete
     `chain_prepare`; 1312-1331 `autosave_sig` drops the motion/chain hash.
   - settings_persist.c 14-20: comment only (the `fx_keys` bytes [15][14..23] are KEPT, tests/settings_test.c reads them).
   - ui.c: 8-14 drop `chain_prepare` decl; 46-84 delete `PREF_LATCH`, `fx_latch`, `PREF_CHORD_ADD` defines; 150-156
     drop `ui.song_row`; 357-370 delete `chain_busy()`; 438-476 and 485-531 delete the motion undo (`UNDO_MOT`,
     `undo.motion_backup`, `motion_sig`, `load_motion_hold`, `load_mo`, `load_from`, `motion_restore/_snapshot_track/
     _sound_loaded/_rebase/_reset`, `undo_mot_tag`); 555-606 and 643-712 `fm4_apply` uses `isr_guard`; `track_defaults_steps`
     loses `motion_reset`. ui_draw.c 281-324, 334-340, 383-390: no `chain.running` (header SONG icon use, song row, slot
     letter for a running song); 401-426 delete `card_mot_*`. ui_input.c 57-63, 135-141: `chain_busy` gone. upreset.c
     407-413 and 574-580: `TSEL->p[i]` for `motion_base_value`, no `chain_busy`.
   - tools/size_fns.py 20-26: remove "song_chain.c", "chord.c", "motion.c" from `SIZE_FILES` (a deleted file listed there
     crashes ./build.sh, FB-005). tests/target_budget.py 25-33 and tests/target_budget.txt 44-54: delete the eight `perf_*`
     functions / rows. tests/run_tests.sh 32-40 and 186-200: the CHORD comment and run.
   - Tests (every one of these used a removed symbol): audio_test.c 41-47 (`chain`) and 201-207 (`kb_layer`);
     click_test.c 51-57; drum_test.c 20-26, 1258-1264, 1315-1344 (a motion event on KIT); fold155_test.c 393-402,
     sound14_test.c 71-80 and 564-633 (`pf`, `perf_held`; the whole `test_spread_fx` goes: it tested the mute key and the
     mono sends), 506-512 (`motion_param`), sound15_test.c 64-73, 193-199, 384-391 (same); midi_control_test.c 130-141 and
     531-549 (motion on external clock; CC74 now just sets CUT); persistence_test.c 70-77, 228-239, 440-449, 589-598,
     634-642 (`chain`, `motion`, "pending SONG" cases); project_test.c (the chain and motion cases; 12 hunks);
     old_pack.h 3-74 (writes zeros for the 36 B chain and 260 B motion blocks); scale_test.c 362-384 (the CHRD case);
     ui_test.c 173-180, 217-248 (power-on state of removed variables), 2024-2056 and 2079-2085 (the HS_SONG header case).
4. Behaviour that follows (owner-visible, nothing to confirm): each key plays one note; MIDI IN notes play single notes;
   the FX-layer keys and the black-key track mutes are gone; the header shows no SONG icon; P_CHRD / P_VOIC are dead
   parameter slots (renumbering them is the v2 params prune, not this task).
5. P3-011 will delete: `PROJ_CHAIN_BYTES`, `PROJ_MOTION_BYTES`, `PROJ_CHAIN_V9`, `rsv_song`, `rsv_auto` and the whole
   FUN10 layout (record it in P3-011's Log if you touch any of them here).

## Acceptance criteria
- [ ] `git apply --check` passed before and the six files in P3-007.deletes.txt show `D` in `git status`; no `#include` of
  song_chain.c / chord.c / motion.c / perform.c remains
- [ ] `bash docs/design/rehearsal/P3-007.acceptance-grep.sh` prints nothing (run from the repo root; it greps firmware/src,
  tests, tools, web, BUILDING.md, LICENSING.md; tests/design is excluded: those are Python prototypes)
- [ ] The build line after the change is `image 377856 B; RAM .data+.bss 34372 B of 73728; pool 154624 B of 368640`
  (before: `image 392280 B; RAM 51472; pool 187780`); a deviation over 2 KB in any number means the patch was altered:
  explain it in the Log
- [ ] Pool used drops by at least 32000 B (measured 33156 B: pf_buf)
- [ ] `sh tests/run_tests.sh` ends `ALL HOST TESTS PASSED` with **no change to tests/golden.txt** (the rehearsal run
  changed none) and tests/t1_link_test.c still passing. The "web pages" step fails only if the repo path has spaces
  (CLAUDE.md known failure); the T-1 phase 3/4 timing benchmarks may fail on a busy host
- [ ] `sh tools/check.sh` ends `CHECK PASS`; RAM headroom 73728 - N is at least 8192 B
- [ ] The Log holds the build line and, for the budget step, the one `note` line it prints (below)

## Verification
```sh
git apply --check docs/design/rehearsal/P3-007.patch
bash docs/design/rehearsal/P3-007.acceptance-grep.sh      # prints nothing
sh tools/check.sh                                           # last line: CHECK PASS
./build.sh | grep 'RAM'                                     # paste into the Log
```

## Budget and golden rules (explicit)
- tests/golden.txt: unchanged. **Do not run GOLDEN_UPDATE.** If a golden line differs, a hunk was misapplied: stop.
- tests/target_budget.txt: only the eight `perf_*` rows are deleted (the patch does it). The step prints
  `target: fm1_alnk0_irq ... note: -15 % (BUDGET_UPDATE=1 to keep it)`: a note about a cheaper audio ISR (perform's
  inlined calls are gone), not a failure. **BUDGET_UPDATE is NOT allowed**: it would rewrite every row, and FB-004 A
  forbids touching rows that did not move. Nothing else moves by more than the 10 % tolerance.
- tests/cpu_baseline.txt: unchanged.

## Out of scope
- Any subsystem SYSTEM_MAP marks KEEP or ADAPT beyond the caller edits above; the dead parameter slots (P_CHRD, P_VOIC,
  P_LN0..P_LN7, ...) and the `CF_*` confirm-dialog constants and `ICON_X_SONG` assets; `SL_RAMP` defines in fx.c
- Behaviour changes to the T-1 path (P3-001's test must still pass)
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log

### 2026-11-11 executor (claude-haiku-5-5)
Status: **blocked**, no kept code change. Filed `backlog/feedback/FB-008-p3-007-r4-project-format-and-test-scope-undecided.md`.

- Baseline before any change: `sh tools/check.sh` ends `CHECK PASS`; `RAM .data+.bss 51472 B of 73728`;
  `image 392280 B; RAM .data+.bss 51472 B of 73728; pool 187780 B of 368640`.
- Blocking reason: `project_t` (project.c:127-128) stores `chain_config_t` and `motion_store_t`, which are written into the
  FUN10 record and read from FUN6..FUN9. The acceptance grep (`motion_`, `chain_` prefixes gone) needs those types gone too,
  so the format must be decided first: keep the byte layout with opaque reserved bytes (recommended), change the layout, or
  keep the types (fails the grep). This is not in the Deliverable.
- Also missing from the Deliverable: the tests that use `pf` / `perf_*` / `kb_layer` (sound14, sound15, fold155, ui_test,
  audio_test, drum_test, scale_test, persistence_test, midi_control_test, song_probe.h, old_pack.h), and the ui.c motion
  undo/load paths, midi_clock.c, upreset.c and ui_draw.c edits. See FB-008.
- Grep before any change (counts of lines naming the removed symbols, outside the four files): project.c 64, seq.c 30,
  ui.c 26, midi_control.c 11, fx.c 7, tests/project_test.c 44, tests/chord_test.c 49, tests/ui_test.c 15.
- Partial edits to fx.c, felucca.c, tests/hostsim.c, tools/size_fns.py, tests/target_budget.py/.txt and tests/run_tests.sh were
  reverted with `git checkout`, so the only changes are this log, the status and FB-008.

### 2026-11-11 orchestrator
FB-008 answered; the task was rehearsed in a scratch copy and rewritten around the verified patch. Rehearsal result: device build `image 377856 B; RAM .data+.bss 34372 B of 73728; pool 154624 B of 368640`; one full `tests/run_tests.sh`: ALL HOST TESTS PASSED, golden and cpu_baseline unchanged, target budget only the `fm1_alnk0_irq -15 %` note. Status set to todo.
