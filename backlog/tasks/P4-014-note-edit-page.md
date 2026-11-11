---
id: P4-014
title: NOTE_EDIT page
phase: P4
type: task
status: todo
priority: 2
executor: haiku
depends_on: [P4-013]
gate: none
---
## Goal
Per-step editing opened by holding a step: every field AUTO or set, as locked.

## Read first
- docs/design/SEQUENCER_UX.md §1.1, §1.2, §2.2, §4.1 (NOTE_EDIT knobs), §5 (NOTE_EDIT traces)
- docs/design/SCREEN_SPEC.md (zones and mock-ups for this page)
- docs/design/page_states.tsv, button_modes.tsv, knob_map.tsv, key_map.tsv, help_text.tsv (rows for this page)
- docs/design/scenarios/ (the scenario steps cited in the criteria)
- firmware/src/ui_sq.c (framework from P4-011)

## Deliverable
- ui_sq.c NOTE_EDIT page: fields from knob_map rows, AUTO/set display, white keys tap ON/OFF and hold = move the editor, EDIT tap = undo, exit rules of §2.7
- tests/ui_sq_test.c: cases for this page (input sequence -> sq_* API calls and screen strings) and PNG renders in build/ui_sq/

## Acceptance criteria
- [ ] Scenario steps citing NOTE_EDIT in docs/design/scenarios reproduce
- [ ] Turning a knob to its AUTO end shows `AUTO` and clears that StepEdit field
- [ ] Every string on the page comes from ui_tables.h (no literal label strings in the page code; reviewer greps)
- [ ] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [ ] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## UX gate
Producer-facing. Use case: `docs/design/features/P4-014-note-edit-page.md` from `_TEMPLATE.md` (docs/project/UX_GATE.md). Gate A (steps, predicted ease score, five-surface sync, live checklist) is filled before the build or cited from the locked design; Gate B is filled by the reviewer.

## Out of scope
- Other pages
- ISR code
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
