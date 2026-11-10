---
id: P4-017
title: REC page
phase: P4
type: task
status: todo
priority: 2
executor: haiku
depends_on: [P4-014]
gate: none
---
## Goal
Live step recording: all 27 keys play and write notes, cursor when stopped, nearest step when playing.

## Read first
- docs/design/SEQUENCER_UX.md §2.5, §5 (U54, U67), §6.1 FS4 fallback (velocity 90, or 120 with K2 ACC)
- docs/design/SCREEN_SPEC.md (zones and mock-ups for this page)
- docs/design/page_states.tsv, button_modes.tsv, knob_map.tsv, key_map.tsv, help_text.tsv (rows for this page)
- docs/design/scenarios/ (the scenario steps cited in the criteria)
- firmware/src/ui_sq.c (framework from P4-011)

## Deliverable
- ui_sq.c REC page: `sq_rec_write(ch,key,vel)`, cursor, K2 ACC, K3 SLIDE, octave keys; the fixed-velocity fallback (force comes in P6-006)
- tests/ui_sq_test.c: cases for this page (input sequence -> sq_* API calls and screen strings) and PNG renders in build/ui_sq/

## Acceptance criteria
- [ ] Scenario steps citing REC reproduce
- [ ] Stopped: each key writes at the cursor and advances; playing: writes at the nearest step (test)
- [ ] Every string on the page comes from ui_tables.h (no literal label strings in the page code; reviewer greps)
- [ ] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [ ] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## Out of scope
- Other pages
- ISR code
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
