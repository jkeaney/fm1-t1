---
id: P3-005
title: Menu rows 37 to 28 and the T-1 always on (R3a)
phase: P3
type: task
status: review
priority: 2
executor: haiku
depends_on: [P3-004]
gate: none
---
## Goal
Remove the menu rows the sequencer does not use and make the T-1 mode permanent (SYSTEM_MAP §3 item 3), as the stepping stone to the new UI.

## Read first
- docs/design/SYSTEM_MAP.md §4 (rows named below), §5 (dependency edges), §6 row R3, §8 (test fate), §9 (bytes)
- firmware/src/felucca.c (the include list: include order is the dependency order)
- tests/run_tests.sh
- docs/design/SYSTEM_MAP.md §7 (exact ui_test edits)
- backlog/feedback/FB-006-*.md (the decisions this task now carries)

## Decisions (FB-006, rehearsed in a scratch copy: build and one full `tests/run_tests.sh` pass, 2026-11-11)
- **Boot rule for `t1_mode`.** The host tests link the REAL `ui_t1.c` and `t1_glue.c` since P3-001 (tests/host_t1_hooks.h; the CLAUDE.md
  line about no-op host stubs is stale, P3-012 fixes it). So "always on" cannot be a plain `on = 1` in `t1_poll`: it would put every host
  UI test (ui_test, motion_test, step15, detail, ...) into the T-1 screen and fail them until P3-006 deletes them. The rule is a
  per-build default in ui_t1.c: `static uint8_t t1u_always` is **1 under `T1_DEVICE`, 0 on the host**; `t1_poll` computes
  `on = t1u_always != 0u`. `t1_mode` itself keeps its static 0 (`seq.c:60`) and becomes 1 on the first main-loop frame (`ui_input` calls
  `t1_poll` every frame), so **tests/t1_link_test.c:41 stays unchanged and passes**. P3-006 drops the `#ifdef` (always 1) when the host
  tests no longer need Felucca's own pages.
- **Retired settings, fixed defaults.** The rows go; the code that read their bits stays until its own task, so the bits must read as the
  default whatever an old settings record saved. `settings_import` (settings_persist.c) clears them: `factory[15][30] &= ~(1|16)`
  (FX LATCH, BPM LOCK = OFF) and `factory[15][24] &= ~(0x04|0x10|0x40|0x80)` (CHORD ENTRY = HOLD, SELECT = BPM, T1 SEQ bit unused).
  `sel_mode()` (ui.c) becomes `return SEL_BPM;`. `PREF_T1` is deleted (ui.c). `PREF_LATCH`, `PREF_BPM_LOCK`, `PREF_CHORD_ADD`,
  `PREF_SEL_*`, `sel_play`, `kb_vel` stay as dead code: P3-006 removes the BPM LOCK / SELECT readers (ui_input.c, ui_draw.c, ui_help.c),
  P3-007 the CHORD ADD and FX LATCH readers.
- **MI_TCH1..4.** The values are the project's `song.mch` (read by `mch_get`, seq.c:82, 1214; saved by project.c). Fixed default: `song.mch`
  stays all 0 = T1..T4 on channels 1..4 (what the rows showed by default); `mch_get/mch_set`, project.c and seq.c are untouched. Only the
  rows go. `MI_PROJECT(row)` becomes `row == MI_TUNE || row == MI_MIDIIN`; delete the `MI_TCH(row)` macro and its two users
  (`menu_n` in menu_items.c, the dim-value test in `menu_row`, ui_menu.c:299-300).
- **"ui_render boots into the T-1 screen" cannot hold on the host and is replaced** by: (1) on the device build `t1u_always` is 1 (grep
  below); (2) a new ui_test check `test_retired_menu_bits` first sets `t1u_always = 1`, runs `frame()`, asserts `t1_mode == 1`, draws once
  (`ui.force = 1; ui_draw()`), then sets 0, `frame()`, asserts `t1_mode == 0` and calls `events_block(CTL)` (this flushes the CC123 sweep
  that `t1_off_req` asks for; without it five CC123 packets leak into the later NAME test and it FAILs). ui_render.c keeps rendering the menu
  tabs; it does not get a T-1 screen scene (that is P4-011).
- **v156_test.c and ui155_test.c are not deleted** (P3-006 does that): only their SELECT blocks go. Lost coverage, accepted: MENU > SELECT
  (#84 MODW/VEL, #188 PAGE), the BPM LOCK menu test and its saved-with-settings test, the CHORD ENTRY ADD test (the feature is unreachable
  now; P3-007 deletes chord entry), the FX LATCH menu row test and settings round trip, T1..T4 CHANNEL menu names (the `song.mch` routing
  tests stay).

## Deliverable
Firmware (all line numbers are the current tree):
- **menu_items.c**: remove from the `enum` `MI_LATCH, MI_BPMLOCK, MI_CHADD, MI_SELECT, MI_TCH1..MI_TCH4, MI_T1SEQ` (`MI_COUNT` 37 -> 28);
  the same entries from `MI_NAME[]`, `MI_TAB[]` (and its comments), `MENU_FLAGS` (LATCH, BPMLOCK, CHADD, T1SEQ), `SELECT_N[]` and its
  comment, `menu_n` (SELECT, TCH), `menu_get` (SELECT, TCH), `menu_vname` (SELECT and the whole TCH case), `menu_put` (SELECT, TCH incl.
  `midi_route = MIDI_ROUTE_AGAIN`); the header comment (lines 1-30) names the rows: reword. `MI_PROJECT` and `MI_TCH` as above.
- **ui_menu.c** 299-300: delete the `MI_TCH(row)` dim test.
- **ui.c**: delete `PREF_T1` (lines 84-85); `sel_mode()` returns `SEL_BPM`.
- **ui_t1.c**: `t1u_always` and `t1_poll` as above; reword the header comment (lines 1-3); neither it nor the comments you add may contain the text `PREF_T1` (the grep below must be empty).
- **settings_persist.c**: the two `&=` lines in `settings_import`, with a comment naming P3-005.
Tests (rehearsed; every file the grep finds):
- **tests/ui_test.c**:
  - 1657-1662 the tabs check: title "8 4 6 4 6", `mtab_rows(MTAB_CONTROL)==4` with `MI_TAB[MI_PREVIEW]==MTAB_CONTROL`, `MTAB_MIDI` 4, `MTAB_SYSTEM` 6, drop `MI_TAB[MI_SELECT]`, `MI_TAB[MI_TCH4]`, `MI_TAB[MI_T1SEQ]`, add `MI_COUNT == 28u`.
  - 1674, 1677, 1682 walk: `MI_LATCH` -> `MI_SCLLED` (CONTROL is HOLD, KNOB ACCEL, SCALE LEDS, STEP PREVIEW), `MI_TCH1` -> `MI_MFOLLOW` (MIDI is MIDI IN, AUTO SELECT, SCREEN WAKE, LEARN CLEAR), comment at 1736 "from FX LATCH" -> "from SCALE LEDS"; the rest of the walk is unchanged.
  - 1741 the ON/OFF switch list: drop `MI_LATCH, MI_BPMLOCK`.
  - 4512-4519 (`test_step_preview_chord_entry`): drop every `MI_CHADD` term; `ui_prefs2 == 7u` becomes `3u`; retitle without CHORD ENTRY.
  - 4598-4659: delete the "CHORD ENTRY ADD" block (from `/* CHORD ENTRY ADD */` through `menu_put(MI_CHADD, 0);`) but **keep one `ui_power_on();` in its place** (the removed block reset state for the tests after it).
  - 7498-7517 (inside the FX hold-layer test, after `fx_latch = 1; ...`): delete from `ui.menu_sel = MI_LATCH;` through the "FX LATCH saved with the settings" block; keep `press(B_HOME);` and re-title the check "the menu turns the latched effects off".
  - 6042-6043 (SCREEN WAKE): the "every other flag set" byte `0x1F` becomes `0x0B` and the assert `(ui_prefs2 & 0x10u)` becomes `0x08u`.
  - 8041-8135 `test_bpm_lock` (BPM LOCK menu, locked behaviour, saved round trip) and its call (~9808): replace with `test_retired_menu_bits` (the boot-rule check above, then: factory[15][30] and [15][24] set to 0xFF import as `0xEE` and `0x2B`, `sel_mode() == SEL_BPM`).
- **tests/settings_test.c** ~166-170: the RESTORE LAST flag byte `128u | 64u | 1u` (three places) becomes `128u | 64u | 4u` (bit 0 is now cleared on import).
- **tests/v156_test.c**: delete `select_play()` (line 342 comment banner through its closing brace) and `+ select_play()` in `main`. The rest of the file stays.
- **tests/ui155_test.c**: delete `title_is()` and `select_page()` (the "SELECT = PAGE" section, 122-215) and `+ select_page()` in `main`; fix the header comment mention.
- **tests/midi_control_test.c** 361-371: `tch()` becomes `mch_set(k, ch); midi_route = MIDI_ROUTE_AGAIN; events_block(CTL);`; in `route_track_test` delete the `menu_n(MI_TCH1)...menu_get(MI_TCH2)` assertion (368-371), add `MI_PROJECT(MI_MIDIIN) && !MI_PROJECT(MI_LCLEAR)` to the line above, and retitle the check "TRACK: channels 1..4 by default (song.mch all 0)".
- **tests/ui_render.c**: 1165 `MI_TCH1 : MI_TCH2` -> `MI_MIDIIN : MI_MFOLLOW`; 1330 delete `ui_prefs |= PREF_LATCH`; 1516 `S_MENU_SELECT` -> `ui.menu = 1; ui.menu_sel = MI_PREVIEW;` (the scene ids stay, so the scene tables are untouched); 1983 `MI_TCH1 + (k / 8u) % 4u` -> `MI_MFOLLOW`. Lines 1020-1022 and ui_test.c 8463-8604 set `PREF_BPM_LOCK` directly to draw the header lock; they stay (the reader is P3-006's).
- tests/run_tests.sh: only the header comment lines for 1.5.5 / 1.5.6 mention SELECT; reword (optional).
- No change needed (grep-verified): tools/size_fns.py, tests/target_budget.py/.txt, tests/golden.txt, web/emu, tests/ui_test_*.sh. Goldens and budget are **unchanged** (do not run GOLDEN_UPDATE / BUDGET_UPDATE).
- Docs: CLAUDE.md line ~161 (`PREF_T1`, MENU > SYSTEM > T1 SEQ) is stale: P3-012 rewrites it.
- Log: before/after build line and the bytes freed.

## Acceptance criteria
- [x] `MI_COUNT` is 28; the tab rows are 8 4 6 4 6 (asserted in ui_test)
- [x] `grep -rnE 'MI_LATCH|MI_BPMLOCK|MI_SELECT|MI_TCH|MI_T1SEQ|MI_CHADD|PREF_T1' firmware/src tests tools web` returns nothing
- [x] `grep -n 't1u_always' firmware/src/ui_t1.c` shows the value 1 under `#ifdef T1_DEVICE` and 0 otherwise; `PREF_T1` is gone from ui.c and ui_t1.c
- [x] `tests/t1_link_test.c` passes unchanged (its line 41 `t1_mode == 0` still holds)
- [x] `test_retired_menu_bits` passes (boot rule + retired bits import as defaults)
- [x] `sh tools/check.sh` ends with `CHECK PASS` (the known failures in CLAUDE.md, and the intermittent `mod: cost: 4 active slots` host-counter check, are not regressions; rerun once if only those fail)
- [ ] Expected build line (rehearsal): `image 499168 B; RAM .data+.bss 54048 B of 73728; pool 191048 B of 368640` (before: 500212 / 54064 / 191048). Paste the real line into the Log; RAM headroom (73728 - N) must be at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
grep -rnE 'MI_LATCH|MI_BPMLOCK|MI_SELECT|MI_TCH|MI_T1SEQ|MI_CHADD|PREF_T1' firmware/src tests tools web   # nothing
```

## Out of scope
- Any subsystem SYSTEM_MAP marks KEEP or ADAPT beyond the caller edits named here
- Deleting the dead readers (`PREF_BPM_LOCK`, `PREF_CHORD_ADD`, `PREF_LATCH`/`fx_latch`, `sel_play`, `kb_vel`, `SEL_*`): P3-006 and P3-007
- Deleting v156_test.c / ui155_test.c, or giving ui_render a T-1 screen scene (P3-006, P4-011)
- Behaviour changes to the T-1 path (P3-001's test must still pass)
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
### 2026-11-11 executor (haiku)
Blocked before any code change. A grep over firmware/src and tests (run from the repo root) shows the Deliverable
does not name every file that must change. Details and the decisions needed are in backlog/feedback/FB-006:

- `tests/v156_test.c` uses `MI_SELECT` (lines 353-419). It is built by `tests/run_tests.sh` and fails to compile.
- `tests/ui_render.c` uses `MI_TCH1` (1165, 1983) and `MI_SELECT` (1516). Also fails to compile.
- `tests/t1_link_test.c:41` asserts `t1_mode == 0` at power-on. This conflicts with "`t1_mode` set to 1 at boot".
- `tests/ui_test.c` has more removed-row lines than named: LATCH 1674/1682/1741/7560, TCH1 1677, CHADD 4512-4659,
  BPMLOCK 8051-8059, and the MTAB counts 1660-1661.
- No fixed default is stated for MI_SELECT, MI_LATCH, MI_BPMLOCK, MI_CHADD (bits read in ui.c, ui_input.c, ui_draw.c)
  or MI_TCH1..4 (`song.mch`, read in seq.c).
- The acceptance criterion "ui_render boots into the T-1 screen" cannot hold on the host: the T-1 screen is not built
  there and the host stub keeps `t1_mode` at 0.

Verification: not run (no change made). `python3 tools/backlog.py check` is run after this status change.

### 2026-11-11 planner (sonnet)
FB-006 answered. Deliverable, decisions and acceptance rewritten after applying the plan in a scratch copy: device build `image 499168 B; RAM .data+.bss 54048 B of 73728; pool 191048 B of 368640` (before 500212 / 54064 / 191048), one full tests/run_tests.sh = ALL HOST TESTS PASSED, goldens and budget unchanged. Status todo.

### 2026-11-11 executor (haiku)
Done. The FB-006 decisions are applied as written. SYSTEM_MAP.md was not re-read in full this run: the Deliverable,
Decisions and Tests lists were followed exactly, and the grep / check results below are the proof.

Edits: firmware/src/menu_items.c (enum, MI_NAME, MI_TAB, MENU_FLAGS, menu_n / menu_get / menu_vname / menu_put, MI_PROJECT,
header comment), firmware/src/ui_menu.c (MI_TCH dim test removed), firmware/src/ui.c (PREF_T1 removed, sel_mode -> SEL_BPM),
firmware/src/ui_t1.c (t1u_always 1 under T1_DEVICE, 0 otherwise; t1_poll uses it), firmware/src/settings_persist.c
(the retired bits cleared on import), tests/ui_test.c, tests/ui155_test.c, tests/v156_test.c, tests/midi_control_test.c,
tests/ui_render.c, tests/settings_test.c. tests/t1_link_test.c unchanged.

Verification:
```
$ AC79_SDK=~/fw-AC79_AIoT_SDK sh tools/check.sh      -> CHECK PASS   (log: build/check/tests.log)
== ram headroom
RAM .data+.bss 54048 B of 73728
$ ./build.sh | grep RAM
  ok    image 499168 B; RAM .data+.bss 54048 B of 73728; pool 191048 B of 368640
$ grep -rnE 'MI_LATCH|MI_BPMLOCK|MI_SELECT|MI_TCH|MI_T1SEQ|MI_CHADD|PREF_T1' firmware/src tests tools web   -> no output (exit 1)
ui: MENU tabs: DISPLAY CONTROL AUDIO MIDI SYSTEM (8 4 6 4 6 rows), 1..8 each in a run, fit the page; tab gaps constant (S, LARGE) ok
ui: P3-005: the T-1 mode is on from the first frame (t1u_always), and off again when it is 0 ok
ui: P3-005: retired bits import as their defaults (FX LATCH, BPM LOCK OFF; SELECT BPM; CHORD ENTRY HOLD) ok
ui: TRACK: channels 1..4 by default (song.mch all 0) ok
ui: all passed
t1_link: all checks ok
ALL HOST TESTS PASSED
```
Build: before 500212 / 54064 / 191048; after 499168 / 54048 / 191048 (1044 B of image and 16 B of .bss freed). Headroom 19680 B,
above the 8192 B trigger. Goldens and budget unchanged (GOLDEN_UPDATE / BUDGET_UPDATE not run).
Note for the reviewer: the build prints `torso_t1_pipeline.c:262 unused variable 'q_note'`, a warning in a file this task does not touch.
Not done here (per the Out of scope): the dead readers (PREF_BPM_LOCK, PREF_CHORD_ADD, PREF_LATCH, sel_play, kb_vel, SEL_*), v156/ui155 files, the CLAUDE.md line (P3-012).
