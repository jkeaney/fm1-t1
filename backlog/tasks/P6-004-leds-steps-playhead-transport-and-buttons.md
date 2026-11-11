---
id: P6-004
title: LEDs: steps, playhead, transport and buttons
phase: P6
type: task
status: todo
priority: 2
executor: haiku
depends_on: [P6-003]
gate: none
---
## Goal
The LEDs mirror the screen: ON steps, playhead, PLAY/REC states, clock behaviour.

## Read first
- docs/design/TIMING_SPEC.md §5.4
- docs/design/SEQUENCER_UX.md (LED rows per page)
- firmware/src/ui.c `ui_leds()`
- firmware/src/ui_sq.c

## Deliverable
- `ui_leds()` rewritten for ui_sq pages per the spec rows; the PLAY green mid level and flash; step LEDs follow the viewed page

## Acceptance criteria
- [ ] A host test of the LED frame for PERF stopped, playing, REC and EXT WAIT matches expected bitmaps
- [ ] LED update cost stays within the main loop frame (no ISR use)
- [ ] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [ ] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## UX gate
Producer-facing. Use case: `docs/design/features/P6-004-leds-steps-playhead-transport-and-button.md` from `_TEMPLATE.md` (docs/project/UX_GATE.md). Gate A (steps, predicted ease score, five-surface sync, live checklist) is filled before the build or cited from the locked design; Gate B is filled by the reviewer.

## Out of scope
- Pages not named here
- ISR changes beyond the API
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
