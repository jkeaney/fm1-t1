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
- [ ] Each of the four source states shows the exact SCREEN_SPEC 6.2 string and dot colour (host test drives the P4-009 state)
- [ ] K1 does nothing while the source is EXT (test)
- [ ] Every string on the page comes from ui_tables.h (no literal label strings in the page code; reviewer greps)
- [ ] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [ ] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

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
