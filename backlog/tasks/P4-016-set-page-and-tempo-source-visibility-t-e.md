---
id: P4-016
title: SET page and tempo-source visibility (T-E)
phase: P4
type: task
status: review
priority: 2
executor: haiku
depends_on: [P4-013, P4-009, P4-024]
gate: none
---
## Goal
Global settings (BPM, GATE, DIV, SWING, OCTAVE) and the always-visible clock: header H2 text and the blinking colour-coded clock dot on every screen.

## Read first
- docs/design/SEQUENCER_UX.md §2.4 (SET), §2.8 (tempo source and clock out)
- docs/design/SCREEN_SPEC.md (zones and mock-ups for this page)
- docs/design/page_states.tsv, button_modes.tsv, knob_map.tsv, key_map.tsv, help_text.tsv (rows for this page)
- docs/design/scenarios/ (the scenario steps cited in the criteria)
- firmware/src/ui_sq.c (framework from P4-011)

## Deliverable
- ui_sq.c SET page: K1 BPM (inert in EXT), GATE, DIV, SWING per knob_map, OCT-/OCT+ = OCTAVE (moved here by HARMONY, P2-012)
- Header slot H2 strings and the clock dot per SCREEN_SPEC §6 and TIMING_SPEC §5 on every page (drawn by the framework)
- Header string width test with the real `text_w`
- tests/ui_sq_test.c: cases for this page (input sequence -> sq_* API calls and screen strings) and PNG renders in build/ui_sq/

## Acceptance criteria
- [x] Each of the four source states shows its string (SCREEN_SPEC_V2 4.1: `n` / `EXT` over `n` / `EXT` over `WAIT` / `EXT` over `--` in T_REC; the dot of SCREEN_SPEC 6.2 was dropped by the owner 2026-11-13) (host test: tests/ui_sq_set_test.c draws the four states with real text_w widths and the T_REC mark; v12_compare ext_* scenes pixel-exact; the P4-009 model itself is not wired yet: `ui_sq_clock_set` is the hook for P4-031)
- [x] K1 does nothing while the source is EXT (tests/ui_sq_set_test.c for EXT / WAIT / lost, tests/ui_test.c through the main loop)
- [x] Every string on the page comes from ui_tables.h (knob_map labels, ui_msgs rows, the new set_text.tsv words, page_states cue); the only literals left in the SET code are the digit `0` and the empty string; the header's `EXT` / `WAIT` / `--` now come from set_text too. (Other elements' literals, `MIDI`, `OFF`, `ROW`, `PAGE`, `+`, are P4-028's, not SET's.)
- [x] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [x] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## UX gate
Producer-facing. Use case: `docs/design/features/P4-016-set-page-and-tempo-source-visibility-t-e.md` from `_TEMPLATE.md` (docs/project/UX_GATE.md). Gate A (steps, predicted ease score, five-surface sync, live checklist) is filled before the build or cited from the locked design; Gate B is filled by the reviewer.

## Out of scope
- Other pages
- ISR code
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
- 2026-11-12: re-pointed by the screen design lock: draw to docs/design/SCREEN_SPEC_V2.md (ADR-0006) and its v12 renders, not SCREEN_SPEC.md zones; the CHAN band/page is specified there (P4-015).

### 2026-11-13 executor (sonnet): Gate A proposal only, build deferred for owner sign-off
- Per the orchestrator's brief the SET page is not drawn in SCREEN_SPEC_V2, so the renders come first (Gate A). Nothing in firmware/ or the locked v12 scene set changed.
- Added `docs/design/ui_review/proposals/v12/v12_set.py` (imports v12.py, overrides knob row / tiles / status / footer / slot for page SET) and rendered 8 scenes to `docs/design/ui_review/proposals/v12/out_set/` (each `.png`, `_4x.png`, `_grid.png`): `v12_set_set_int`, `_int_stopped`, `_turn_swing`, `_octave`, `_ext_live`, `_ext_k1_turned`, `_ext_wait`, `_ext_lost`.
- `check_messages()` on the SET strings: `[]` (all fit the 136 px slot); the mock asserts the status-line blocks and footers fit.
- Use case `docs/design/features/P4-016-*.md` filled for Gate A (steps, ease scores, sync table, live checklist, hazards, five owner questions). Owner approval NOT marked.
- Pointers added (docs only): SCREEN_SPEC.md 6.5 (zones superseded), SCREEN_SPEC_V2.md section 8. The locked tables (page_states, knob_map, button_modes) and scenarios S06/S07/S12 are deliberately NOT edited until sign-off (they feed generated tests); the use case section 5 lists each edit the build commit makes.
- Hazards checked: bounds (BPM 40..240, SWING 50..75, GATE 10..100, DIV 0..5, OCTAVE -2..+2 clamped, no stack or history on SET); stale state (EXT mirror reads the source per frame, view signature must include clock and octave, K1 touch frame clears on leaving SET, EXT -> INT restores own tempo in the same frame); surface sync (TEMPO/OUT status lines and OCT tag column in SCREEN_SPEC 6.5/7, SEQUENCER_UX 2.4, page_states SET, S06/S07/S12); width (check_messages + asserts).
- Deviation to flag: the task text asks for the blinking clock dot; SCREEN_SPEC_V2 4.1 (locked) has none. Proposal follows V2 (owner question 1). The header strings INT / EXT / WAIT / `--` are already drawn by ui_sq.c (P4-028); this task would add their host test.
- Verification: `sh tools/check.sh` -> `CHECK PASS`; `RAM .data+.bss 29224 B of 73728` (headroom 44504 B, unchanged: no firmware change).
- Acceptance criteria left unticked: the build (SET page code, tests/ui_sq tests, grep of strings) has not been done; it follows owner approval of the renders.

### 2026-11-13 executor (sonnet): owner approval and build
- Approved: James Keaney 2026-11-13 (answered "Good enough, build it" to the SET renders; five defaults: no clock dot; OUT and OCT in the status line; meters in the selected channel's colour; EXT SYNC shown like WAIT; WAIT/LOST footers in AF_S). Relayed by the coordinator; recorded here and in the use case (Design locked).
- Built: the SET page in `firmware/src/ui_sq.c` (band UISQ_BAND_SET, tile rows from knob_map + set_text, status line, footer, slot title, K1-K4, OCT-/OCT+, GLO / HOME / CHN / held step / SELECT, `ui_sq_clock_set` hook, keys live on SET), `ui_sq_glue.c` (`uisq_bpm_req` handed to `song.g[G_BPM]` by `sq_frame`), new `docs/design/set_text.tsv` + `tools/gen_ui_tables.py` (`ui_st_text`), v12.py scenes `set_*` and goldens `out/v12_set_*.png`, `tests/v12_export.py`, `tests/ui_sq_v12_test.c` (SET band), new `tests/ui_sq_set_test.c`, `tests/ui_test.c` (4 rows through the real main loop), `tests/run_tests.sh`.
- Tables and docs edited (use case section 5): knob_map, control_map, button_modes, page_states, CONTROL_MAP.md, SCREEN_SPEC_V2 4.7 / 5 / 7 / 8, SCREEN_SPEC.md 6.5, SEQUENCER_UX 2.4 / 2.8, S06, S07; S09 / S10 / S12 regenerated by gen_sweeps.py.
- Verification: `sh tools/check.sh` -> `CHECK PASS`. Build line: `image 194416 B; RAM .data+.bss 29352 B of 73728; pool 78520 B of 368640` (headroom 44376 B >= 8192). Host tests: ui_sq_set_test OK (0 failures); v12_compare `52 scenes/sheets, 0 failed`; ui_test 4 new rows ok; check_scenarios / check_help / check_sequencer_ux pass.
- Hazards checked: (1) bounds: BPM 40..240 clamp both at the request and on screen (a stored 9 or 400 is drawn 40 / 240, tested), SWING 50..75, GATE 10..100, DIV 0..5, OCTAVE -2..+2, every one clamped without wrap; SET keeps no stack or history (4 tile rows, 6 names, one request word). (2) stale state: the K1 mirror reads `sq_bpm` per frame, EXT -> INT restores the own tempo in the tile in the same frame (tested), the tile values come from `sq_gdb[sq_gidx]` each frame (a refused `sq_global_set` while a load is pending changes nothing and is not echoed), a pending-preset pill stays on SET, the view signature covers clock, port and octave (they are in UisqView before `msg`), a preset landing drops a pending BPM request (`sq_frame`). (3) surface sync: tables, specs, scenarios and use case edited and the checkers run (use case section 5). (4) test harness: `tests/v12_compare.py` rendered the mock into a relative path under the mock's cwd and read a stale `build/ui_sq/v12_ref` copy; it now uses an absolute path, so every scene (not only SET) is compared with a fresh render (all 44 older scenes still pass).
- Debt for the reviewer: GLO acts on press (hold = HARMONY is P5-006); ALGO encoder works on CHAN only (P4-032); the external-clock source is not wired (P4-031 calls `ui_sq_clock_set`); `midi_clock.c` still overwrites `song.g[G_BPM]` under a host clock, so the own tempo is not kept on the device until P4-031 / P2-010 lands; real unit: HW-1.
