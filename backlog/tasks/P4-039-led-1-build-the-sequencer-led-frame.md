---
id: P4-039
title: LED-1 build: the sequencer LED frame
phase: P4
type: task
status: todo
priority: 1
executor: sonnet
depends_on: [P4-038, P4-021]
gate: hardware-test
---
## Goal
The LEDs follow the sequencer as designed in P4-038, built in the main loop from the data the screen already computes (no extra step evaluation, no ISR cost).

## Read first
- docs/design/features/P4-038-led-design.md (the locked scheme)
- /tmp/readiness_review.md section E
- firmware/src/ui_input.c:170-247 (`ui_leds`), firmware/src/main.c (call site)
- firmware/src/ui_sq.c (`UisqView`, `uisq_view_fill`), firmware/src/ui_sq_glue.c
- firmware/hal/fm1_input.h (fm1_led*)
- tests/host_sq_hooks.h and an existing ui_sq test for the harness pattern

## Deliverable
- A pure `uisq_leds(const UisqView*, now) -> {lit, mid, glow, breath}` in the ui_sq layer (host-testable) and glue that maps it to `fm1_led*` and the MENU LEDS modes; `ui_leds()` calls it
- Fix the dead variables: the REC LED reads `song.rec` and OCT reads `song.octave`, which nothing writes; SCALE LEDS read the legacy track scale instead of `sq_gdb`
- tests/ui_sq_led_test.c with bitmap checks for: stopped; playing with the playhead on an ON step and on an OFF step; muted channel; removed channel; MIDI OFF; REC armed; EXT WAIT; page 2 of 3; held key. Added to tests/run_tests.sh
- No new `sq_eval` calls per frame (the frame reuses the lanes the view computed); no code in the ISR

## Acceptance criteria
- [ ] All nine bitmap cases above pass
- [ ] REC and OCT LEDs light on the right states in the host test (they could not before)
- [ ] Grep shows no new call to `sq_eval` or `step` from the LED code
- [ ] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [ ] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)
- [ ] Hardware: the owner checks each rule on the unit; approval line in the Log

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## UX gate
Producer-facing. Use case: `docs/design/features/P4-038-led-design.md` (Gate A done there); Gate B is filled after the owner's walk on the unit.

## Out of scope
- Algorithm-aware layers, FILL/ROLL and dice flash (P6-004 = LED-2)
- Scan ISR changes and LED blanking (P6-005)
- Any firmware outside the LED path
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
- 2026-11-13: created from the POC readiness review (/tmp/readiness_review.md, copy the findings you need into the Log; /tmp may be gone).
- P6-004 becomes LED-2; P6-005 now depends on this task.
