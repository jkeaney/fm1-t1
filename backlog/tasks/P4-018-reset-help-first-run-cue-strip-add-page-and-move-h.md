---
id: P4-018
title: RESET, HELP, first run, cue strip, ADD_PAGE and MOVE_HELD
phase: P4
type: task
status: todo
priority: 2
executor: haiku
depends_on: [P4-015, P4-016, P4-017, P4-024]
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
- ui_sq.c: RESET one-shot, HELP overlay reading help_text, FIRST_RUN overlay (one bit in `ui_prefs2`, dismissed by HOME, first step 'D#4 K2: MIDI CHANNEL'), cue strip 11.2 first-match table, ADD_PAGE prompt, MOVE_HELD overlay
- tests/ui_sq_test.c: cases for this page (input sequence -> sq_* API calls and screen strings) and PNG renders in build/ui_sq/

## Acceptance criteria
- [ ] Every row of SEQUENCER_UX 11.2 is a test case selecting the right cue
- [ ] Every control has a HELP string (test iterates control ids)
- [ ] Scenario S01 runs end to end in the host test with no menu open
- [ ] Every string on the page comes from ui_tables.h (no literal label strings in the page code; reviewer greps)
- [ ] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [ ] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

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
