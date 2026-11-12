---
id: P4-015
title: CHAN page: MIDI channel, algorithm, length
phase: P4
type: task
status: todo
priority: 2
executor: haiku
depends_on: [P4-013]
gate: none
---
## Goal
Set up a channel in at most two gestures: MIDI channel (unique across channels), algorithm, pattern length and pages.

## Read first
- docs/design/SEQUENCER_UX.md §2.3, §4.4 (unique MIDI channels), §1.3, §1.6, §11.3 (first-run step 'D#4 K2: MIDI CHANNEL')
- docs/design/SCREEN_SPEC.md (zones and mock-ups for this page)
- docs/design/page_states.tsv, button_modes.tsv, knob_map.tsv, key_map.tsv, help_text.tsv (rows for this page)
- docs/design/scenarios/ (the scenario steps cited in the criteria)
- firmware/src/ui_sq.c (framework from P4-011)

## Deliverable
- ui_sq.c CHAN page: K2 MIDI channel with the 0.6 s settle, which also commits on ANY exit from CHAN (PLAY, a page button, HOME), via `sq_set_midi`; the unique rule toast naming the channel set to OFF; ALGO encoder picker over the `ALGOS[]` names (six after P5-008); length and page add
- tests/ui_sq_test.c: cases for this page (input sequence -> sq_* API calls and screen strings) and PNG renders in build/ui_sq/

## Acceptance criteria
- [ ] Committing the MIDI channel happens on 0.6 s stillness AND on every exit path (test covers PLAY, GLO, HOME, D#4 exits)
- [ ] Taking a used MIDI channel sets the other channel OFF and one EDIT undo restores both (decision 77)
- [ ] Scenario S01 steps for CHAN reproduce
- [ ] Every string on the page comes from ui_tables.h (no literal label strings in the page code; reviewer greps)
- [ ] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [ ] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## UX gate
Producer-facing. Use case: `docs/design/features/P4-015-chan-page-midi-channel-algorithm-length.md` from `_TEMPLATE.md` (docs/project/UX_GATE.md). Gate A (steps, predicted ease score, five-surface sync, live checklist) is filled before the build or cited from the locked design; Gate B is filled by the reviewer.

## Out of scope
- Other pages
- ISR code
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
- 2026-11-12: re-pointed by the screen design lock: draw to docs/design/SCREEN_SPEC_V2.md (ADR-0006) and its v12 renders, not SCREEN_SPEC.md zones; the CHAN band/page is specified there (P4-015).
