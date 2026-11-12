---
id: P4-016
title: SET page and tempo-source visibility (T-E)
phase: P4
type: task
status: todo
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
