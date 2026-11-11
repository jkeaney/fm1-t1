---
id: FB-006
title: P3-005 removes menu rows and makes T-1 always on, but names too few tests and conflicts on t1_mode at boot
phase: P3
type: feedback
status: done
priority: 2
executor: sonnet
depends_on: [P3-005]
gate: none
session: 2026-11-11, executor (haiku), P3-005 blocked before any code change
---
## Goal
Complete P3-005's scope so the executor can finish without surprises, and settle two rules the task leaves open.

## Read first
- backlog/tasks/P3-005-menu-rows-37-to-28-and-the-t-1-always-on-r3a.md (Log: the executor's list)
- docs/design/SYSTEM_MAP.md sections 3, 7, 8
- backlog/feedback/FB-005-p3-004-web-editor-removal-misses-tests-tools-and-web.md (same gap, found for P3-004)

## Observation
P3-005 names the menu rows and some tests, but a grep (no edits made) shows these files that must change for the
acceptance criteria and for `sh tests/run_tests.sh` to pass, and that the task does not name:

1. **tests/v156_test.c** (built by run_tests.sh, 1.5.6 step): uses `MI_SELECT` throughout (lines 353-419). Compile break.
2. **tests/ui_render.c** (built by run_tests.sh): `MI_TCH1` at 1165 and 1983, `MI_SELECT` at 1516. Compile break.
   Acceptance says ui_render "boots into" the T-1 screen, but host stubs keep `t1_mode` 0 (seq.c:60, host_t1_hooks.h),
   and the T-1 screen is not built on the host, so that criterion cannot be met as written.
3. **tests/t1_link_test.c:41** asserts `t1_mode == 0` "off at power-on". Deliverable says `t1_mode` is set to 1 at boot.
   These cannot both hold. P3-001's test must still pass (task Out of scope), so the rule for boot must be decided first.
4. **tests/ui_test.c** has more removed-row references than the task names: LATCH 1674/1682/1741/7560, TCH1 1677,
   CHADD 4512-4659 (the chord-add test block), BPMLOCK 8051-8059, and the MTAB counts at 1660-1661.
5. **tests/midi_control_test.c:361-371** and **tests/ui155_test.c:132-211** are named, but with more lines than the
   "SELECT lines" and "MI_TCH lines" wording suggests (ui155 SELECT is used through line 211).
6. **Fixed defaults are undefined for three rows.** The rows are read by bits, not by `menu_get`, so "code reading the
   removed rows' values gets the fixed default" needs a decision per row:
   - `MI_SELECT`: `sel_mode()` (ui.c) reads `ui_prefs2` bits 4 and 6. Default BPM (0) assumed.
   - `MI_LATCH`, `MI_BPMLOCK`, `MI_CHADD`: `PREF_LATCH`, `PREF_BPM_LOCK`, `PREF_CHORD_ADD` are read in ui_input.c and ui_draw.c.
     Default OFF / HOLD (bit clear) assumed. Also needs: are stored settings with these bits set forced off at load?
   - `MI_TCH1..4`: `mch_get` feeds seq.c (lines 82, 1214). These are project values (`song.mch`), not settings. What
     is the fixed default: the boot value of `song.mch`, or a constant?

## Deliverable
- Rewrite P3-005's Deliverable to name every file in the list above (v156_test.c, ui_render.c, t1_link_test.c, the
  ui_test.c line ranges) and the chosen default for each removed row (items 6).
- Decide whether `t1_mode` is forced on at boot by (a) `t1_poll` alone (no boot-time value change, test kept at a
  power-on-before-first-poll check), or (b) a static initialiser, which needs the t1_link_test assertion changed.
- Decide v156_test.c: delete its SELECT block (it tests the removed row) or delete the file (R3 territory).
- Then set P3-005 back to `todo`.

## Acceptance criteria
- [ ] P3-005 is `todo` with a Deliverable naming every file that the grep in Observation shows.
- [ ] Each removed row has a stated fixed default, and the t1_mode-at-boot rule is written down.

## Verification
```sh
python3 tools/backlog.py check
grep -rn 'MI_LATCH\|MI_BPMLOCK\|MI_SELECT\|MI_TCH[1-4]\|MI_T1SEQ\|MI_CHADD' firmware/src tests   # after the rewrite: only the named files, then none
```

## Log
- 2026-11-11 executor (haiku): found by grep before any edit. No firmware or test file changed.
- 2026-11-11 planner (sonnet): answered by rehearsal in a scratch copy (applied P3-005, device build, ONE full tests/run_tests.sh = ALL HOST TESTS PASSED, goldens and budget unchanged).
  Decisions: (1) boot rule is neither (a) nor (b) as posed: the host tests link the real ui_t1.c (P3-001), so an unconditional `on = 1` in t1_poll failed 35 checks in motion_test and
  most UI suites; the rule is `t1u_always`, 1 under T1_DEVICE and 0 on the host, t1_link_test unchanged. (2) Defaults: SELECT = BPM (sel_mode constant), LATCH/BPM LOCK/CHORD ADD = OFF/HOLD
  (cleared in settings_import), MI_TCH = song.mch all 0 (channels 1..4), rows only removed. (3) v156_test.c and ui155_test.c lose only their SELECT blocks; ui_render.c redirects its three
  scenes. (4) "ui_render boots into the T-1 screen" replaced by the t1u_always grep plus test_retired_menu_bits. (5) Found beyond FB-006: ui_menu.c:299 `MI_TCH(row)`, settings_test.c:169,
  ui_test.c 6042 (SCREEN WAKE flag byte), 7498 (FX LATCH menu steps), the ui_power_on that the CHORD block performed, and the CC123 leak from t1_off_req. All are in P3-005 now.
  Also patched: P3-006 (category.c stays until P3-011; include sites; dead readers), P3-007 (include sites, caller counts, dead readers), P3-011 (takes category.c). P3-008..P3-012, P3-014: no MI_/menu_items references (grep).
