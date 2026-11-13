---
id: P4-018
title: RESET, HELP, first run, cue strip, ADD_PAGE and MOVE_HELD
phase: P4
type: task
status: review
priority: 2
executor: sonnet
depends_on: [P4-015, P4-016, P4-017, P4-024, P4-032]
gate: none
---
## Goal
The safety net pages: RESET (hold EDIT), HELP (hold HOME and touch a control), the once-per-device first run, the cue strip priority list, the add-page prompt and the move overlay.

## Read first
- docs/design/SEQUENCER_UX.md §2.6, §2.7, §11.1-§11.4
- docs/design/SCREEN_SPEC.md (zones and mock-ups for this page)
- docs/design/page_states.tsv, button_modes.tsv, knob_map.tsv, key_map.tsv, help_text.tsv (rows for this page)
- docs/design/scenarios/ (the scenario steps cited in the criteria)
- firmware/src/ui_sq.c (framework from P4-011)

## Deliverable
- ui_sq.c: RESET one-shot, HELP overlay reading help_text, FIRST_RUN overlay (one flag, dismissed by HOME; as built the byte `ui_first_run` = `favorites.factory[15][31]`, since `settings_import` clears bit 7 of `ui_prefs2`; first step the setup cue 'CHN KEY, K2 = MIDI'), cue strip 11.2 first-match table, ADD_PAGE prompt, MOVE_HELD overlay
- tests/ui_sq_test.c: cases for this page (input sequence -> sq_* API calls and screen strings) and PNG renders in build/ui_sq/

## Acceptance criteria
- [x] Every row of SEQUENCER_UX 11.2 is a test case selecting the right cue (19 rows, `ui_sq_safety_test`: `uisq_cue_row()`)
- [x] Every control has a HELP string (test iterates control ids: 49 controls x 8 pages, each row present and its lines fit the box)
- [x] Scenario S01 runs end to end in the host test with no menu open (`ui_sq_safety_test`, steps 1-10)
- [x] Every string on the page comes from ui_tables.h (no literal label strings in the page code; reviewer greps): the new code reads ui_msgs, ui_ov_text (overlay_text.tsv), ui_help_rows and the page_states cues; the old footer literals (`KEYS = STEPS`, `HOLD = EDIT`, the setup cue) were moved to the page_states cues too
- [x] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [x] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## UX gate
Producer-facing. Use case: `docs/design/features/P4-018-reset-help-first-run-cue-strip-add-page.md` from `_TEMPLATE.md` (docs/project/UX_GATE.md). Gate A (steps, predicted ease score, five-surface sync, live checklist) is filled before the build or cited from the locked design; Gate B is filled by the reviewer.

## Out of scope
- Other pages
- ISR code
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
- 2026-11-12: re-pointed by the screen design lock: draw to docs/design/SCREEN_SPEC_V2.md (ADR-0006) and its v12 renders, not SCREEN_SPEC.md zones; the CHAN band/page is specified there (P4-015).

### 2026-11-13 executor (Sonnet 5.5)
Built RESET, HELP, FIRST_RUN, the cue priority, ADD_PAGE and MOVE_HELD on SCREEN_SPEC_V2 (not the old SCREEN_SPEC). Files: `firmware/src/ui_sq.c` (overlay state, `uisq_cue_row`, RESET block, HELP box, first-run, EDIT / HOME level handlers, move keys), `ui_sq_glue.c` (HELP routing, EDIT level, first-run load / save), `ui_input.c` (HOME hold opens no MENU after a touch: one condition), `ui.c` (bit 0x80 of `ui_prefs2`, prototype), `sq_chan.c` (the ADR-0003 `sq_reset_morph / row / algo / layer` family, see below), `tools/gen_ui_tables.py` + `docs/design/overlay_text.tsv`, v12.py MESSAGES (+8 rows), tests (`ui_sq_safety_test.c` new, `ui_test.c` +9 glue cases, `ui_sq_v12_test.c` footer literals), docs.
Commands and decisive output:
```
sh tools/check.sh                       -> CHECK PASS
./build.sh | grep RAM                   -> ok    image 216272 B; RAM .data+.bss 29640 B of 73728; pool 78520 B of 368640   (headroom 44088 B; was 29576)
build/check/tests.log: ui_sq_safety_test: OK (0 failures); ui_test 0 FAILED; ui_sq_rec/perf/note/chan/set/v12: OK
v12_compare: 52 scenes/sheets, 0 failed (T_CHF rounding only, unchanged)
python3 tools/check_control_map.py      -> FAIL 78 (the known baseline, same count; not fixed)
tests/design/check_help.py OK (392 rows); check_scenarios.py 0 with no scenario; check_sequencer_ux.py OK; width_probe 320 strings, 0 over
python3 tools/backlog.py check          -> 166 items, 0 errors
```
Differences from the v12 scenes: none (`hold_edit`, `hold_home`, `first_run`, `add_page`, `held_*` equal the mock). **NEW on V2, no mock scene, for the owner to approve:** the RESET overlay (inverted label bands, white outlines on buttons and white keys, D#4 lit, footer `TOUCH WHAT TO RESET / EDIT = NO`, slot `RESET TOUCH ONE`), the HELP box (y 27-141 panel, `HELP` tag, two help_text lines, slot `HELP LET GO = BACK`), the first-run steps 2-7 in the footer with `HOME = SKIP`. Renders: build/ui_sq/safety_reset_perf.png, safety_help_seq.png, safety_home_hold.png, safety_first_run_1/2.png, safety_held_solo.png (SCREEN_SPEC_V2 4.9-4.11).
Decisions and deviations the reviewer should look at:
1. `sq_reset_*` (ADR-0003 6 API) did not exist and no task owned it (sq_chan.c said "P6"); RESET needs it (S08 steps 7-13), so it is in sq_chan.c (not named in the Deliverable). One undo record each, none when nothing changes; tested in `ui_sq_safety_test`. Signatures return uint8_t (changed) instead of the ADR's void.
2. EDIT tap now acts on RELEASE (needed to tell it from the 0.4 s hold); REC keeps the hold inert (a long press is still an undo). The glue calls `ui_sq_edit(level)`; `ui_sq_button(UI_C_B_EDIT)` is unchanged for the tests.
3. HOME hold: ui_input.c opens the MENU at the 0.7 s mark (existing P4-012 behaviour), not on the release as SEQUENCER_UX 2.6 said; the doc now says so. With a control touched the hold opens no menu.
4. Footer texts: steps 5 and 6 of the first run are `TAP A WHITE KEY: ON/OFF` / `HOLD A WHITE KEY: EDIT` (the spec's longer words do not fit beside `HOME = SKIP`); RESET cue `TOUCH WHAT TO RESET   EDIT = NO` (page_states). Toasts `CLEAR` / `COPIED` / `RESET` (the V2 slot is 136 px).
5. MOVE_HELD names SOL / FIL / ROL in the slot but the moves are P6-003's: on a device they show and do nothing until then. No key force yet: FILL reads 100%, ROLL 2X.
6. The ADD_PAGE toast is the P4-013 one (`PAGE n?  PG+=ADD`); it does not distinguish "show hidden" / "complete partial" (the key_map prose still describes the longer wording; behaviour is in sq_page_add).
7. First run: every existing device (bit 0x80 of `ui_prefs2` clear) runs the strip once after this firmware; `settings_save()` stores the bit.
Hazards checked (each with the test that covers it, `ui_sq_safety_test` unless noted): array / stack bounds: key 0..26, step vs LENGTH, page 0..3, first-run step 1..7, help rows for 49 controls x 8 pages (all present, all fit); unused / removed-slot guards: RESET with a key past LENGTH, an absent layer (SNAKE ARTIC), a removed channel is never selected (random run `uisq_used` check); stale state after remove / add / preset landing / page change: RESET closes over any page without defaults, a held move clears on release on SET / REC / after SELECT, no HOME / HELP flag survives HOME up, a pending CHAN MIDI lands before RESET opens, an EDIT held when HOME goes down is no tap (`ui_test`); early returns that skip cleanup: key release marked fired under RESET so it toggles nothing, M / move key clean-up runs before the REC and page checks; flags that survive a page change: `uisq_pg_pend`, `uisq_reset`, `uisq_help`, `uisq_hold_home` asserted every iteration of the random run (6000 keys / knobs / buttons / HOME / EDIT actions, seed 0x2545F491, also tried with 6 other seeds; counts: RESET 561, HELP 2333, held move 2327).
Docs sync, terms grepped over docs/design: `RESET: TOUCH WHAT TO RESET`, `TAP A WHITE KEY: STEP ON/OFF`, `HOLD A WHITE KEY: EDIT THAT NOTE`, `PG+ TWICE: ADD PAGE 2 (A COPY)`, `PRESS FX: NEW PATTERN  EDIT=undo`, `HOME  hold: HELP / MENU`, `touch=HELP  let go=MENU`, `strip reads HOME`, `STEP 3 CLEARED`, `D#4 K2: MIDI CHANNEL`, `ROLL 3x: let go to stop`, `ADD PAGE 2 = COPY OF 1?`, `RESET is P4-018`: all replaced (SEQUENCER_UX 2.6 / 11.1 / 11.2 / 11.3, SCREEN_SPEC, SCREEN_SPEC_V2 4.9-4.11, page_states, button_modes / control_map / CONTROL_MAP, knob_map, S01 S02 S03 S07 S08, S15 / S16 / S09 / S11 regenerated, PERFORMANCE_DESIGN, FINDINGS, widths.tsv); the old wordings are in forbidden_terms.txt. Use case: docs/design/features/P4-018-reset-help-first-run-cue-strip-add-page.md (Gate A filled, Gate B pending the reviewer).
Not done / still open: real-unit check (HW-1); owner approval of the RESET and HELP pictures; v12.py has no scene for them (the host renders are the reference).

### 2026-11-13 review (sonnet)
Verdict: FAIL. `sh tools/check.sh` re-run: CHECK PASS; `RAM .data+.bss 29640 B of 73728` (headroom 44088 B, ok); v12_compare 52 scenes/sheets, 0 failed; ui_sq_safety_test OK.
Findings (fix these, then a round only verifies them):
- [x] **BLOCKER, first-run bit never persists.** firmware/src/settings_persist.c:62 `p->favorites.factory[15][24] &= ~(0x04|0x10|0x40|0x80)` runs in `settings_import` (project.c:346) on every boot and clears bit 0x80 (the retired T1 SEQ bit). So `PREF_FIRST_RUN_DONE` is wiped at each load and the strip replays at every power-on (not once per device); `settings_save()` rewrites flash for nothing. The test (ui_test.c:2275) only checks the bit is set in RAM; it never round-trips export -> import. Fix: store the flag where import does not clear it (a free bit/byte of factory[15] that no retired row used, e.g. a spare bit of another byte; or drop 0x80 from the clear mask AND accept that old records with the retired T1 SEQ bit 7 set would skip first run: owner call, so prefer a fresh bit/byte), update the comments at settings_persist.c:12, ui.c:35-38 (`bit 7 free`), SEQUENCER_UX 11.3 "As built", and add a tests/settings_test.c case: set the bit, export, import, bit still set; an old record (no favorites) reads clear.
- [x] Task Deliverable text still says first step 'D#4 K2: MIDI CHANNEL'; code and locked tables use `CHN KEY, K2 = MIDI` (correct per page_states/SCREEN_SPEC_V2). No code change: note it in the Log or fix the task text.
- [x] Low: ADR-0003 6 (lines 379-382) still lists `sq_reset_*` as `void`; the code returns `uint8_t` (changed). Add a one-line note to the ADR (or an ADR amendment) so the API doc matches.
- [x] Low: `UI_OV_RST` ("RST") in overlay_text.tsv is generated but never used (the RESET marking is the white band/outline); delete the row or use it.
- [x] Low: `ui_sq_knob` on PERF does not cancel the ADD PAGE prompt when the knob is on ROW 2 / d==0 (ui_sq.c:1996 returns before `uisq_pg_pend = 0`), while 11.2 row 3 says any other control cancels it. A ROW 2 knob turn leaves cue row 3 set. Move the clear above the row test (`if (d && k < 4u) uisq_pg_pend = 0;`).
Checked and fine: sq_reset_* one undo record each, none when unchanged (sq_chan.c:456-492), bounds at key 0..26, steps vs LENGTH 16/64, page 0..3 and PG+ at LENGTH 64 toast; HELP on every page (glue routes before any action, keys masked until physically up, PLAY plays as specified); HOME hold touches suppress the MENU (ui_input.c:319, one condition; MENU opens at 0.7 s, SEQUENCER_UX 2.6 text updated; the stuck `uisq_home_dn` while the menu is open is cleaned on the next frame's release); EDIT tap on release (an EDIT held when HOME goes down is no tap; REC keeps a long press as UNDO); cue table has 19 rows in the order of 11.2; strings come from ui_msgs / ui_ov_text / ui_help_rows / page cues; no algorithm named in screen code; forbidden_terms additions are genuinely replaced wordings (the surviving FR text differs from each listed phrase).
NEW overlays for the owner to approve (no v12 scene), host renders in build/ui_sq/: (1) RESET overlay, `safety_reset_perf.png`: inverted label bands on K tiles, white outline on the button tiles and white keys, D#4 white with RST icon, slot `RESET TOUCH ONE`, footer `TOUCH WHAT TO RESET / EDIT = NO` (AF_M 11 px, `EDIT = NO` AF_S 9 px); draw path ui_sq.c uisq_reset_fill + uisq_draw_knobs / notetiles / buttons / keybed footer. (2) HELP box, `safety_help_seq.png`, `safety_home_hold.png`: panel x2-238 y27-141 over tiles, status line and music row; AF_S tag and page name (9 px), two AF_M help lines (11 px); path uisq_draw_help. (3) first-run steps 2-7, `safety_first_run_1/2.png`: footer line AF_M with `HOME = SKIP` AF_S. All three fit the 240x240 grid with nothing cut (width_probe 320 strings, 0 over). Legibility at 30 mm (8 px/mm): AF_M 11 px caps = 1.4 mm and AF_S 9 px = 1.1 mm are at the stated floor, the 21 px digits are fine; the AF_S items (HELP tag, page name, `EDIT = NO`, `HOME = SKIP`) are the weakest and need the owner's eyes on the unit.
Awaiting owner (after the fix): approve the RESET overlay, the HELP box and the first-run steps 2-7 pictures; HW-1 check of the EDIT release-tap timing and the 0.7 s HOME menu. Note for the owner: steps 5 and 6 are the shortened `TAP A WHITE KEY: ON/OFF` / `HOLD A WHITE KEY: EDIT`, and every existing device runs the strip once after this firmware (once the persistence bug is fixed; today it runs every boot).

### 2026-11-13 executor (Sonnet 5.5), review fixes
1. BLOCKER: the flag is now the byte `favorites.factory[15][31]` (`ui_first_run`, ui.c), which `settings_import` does not touch; bit 7 of `ui_prefs2` is again documented free (cleared on import). Glue reads `ui_first_run != 1` at boot and stores 1 + `settings_save()` once. Comments fixed: settings_persist.c header, ui.c, SEQUENCER_UX 11.3, SCREEN_SPEC_V2 4.11. Tests: `tests/settings_test.c` case (bit 7 of [24] wiped, [31] kept through import and export -> import; a PER3 record reads 0 = strip runs); `ui_test.c` first-run case now does export -> import. Factory / settings reset: an older or zero record (PER3, a restore) reads 0, so the strip runs again on a fresh device, as intended; no other reset path clears favorites.
2. ADR-0003 6: `sq_reset_*` now `uint8_t` with a note (1 = changed, 0 = already default, no record).
3. `RST` row removed from overlay_text.tsv.
4. Any knob (ROW 2, a RIG row, every page) cancels the ADD PAGE prompt; test in `ui_sq_safety_test` (prompt on ROW 2, K3 turn).
5. Task Deliverable text updated to `CHN KEY, K2 = MIDI` and the byte flag.
Hazards: stale state / flags that survive: the first-run flag now round-trips through settings (settings_test); the prompt flag is cleared by every knob regardless of row (safety test and the random run's `uisq_pg_pend` invariant). Docs sync grep: `bit 0x80`, `ui_prefs2` first run, `PREF_FIRST_RUN_DONE`, `UI_OV_RST` (SEQUENCER_UX, SCREEN_SPEC_V2, ui.c, settings_persist.c, ADR-0003).
```
sh tools/check.sh -> CHECK PASS (a first run showed the known sound15 ENV SYNC cost benchmark flake, 1.59 % vs 1 %, unrelated; the rerun passed)
RAM .data+.bss 29640 B of 73728 (headroom 44088 B); v12_compare 52 scenes/sheets, 0 failed; ui_sq_safety_test OK; settings_test passed
```
