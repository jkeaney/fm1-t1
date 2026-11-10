---
id: P5-006
title: HARMONY page (hold GLO) and the main-screen chord indicator
phase: P5
type: task
status: todo
priority: 2
executor: haiku
depends_on: [P5-005, P4-016]
gate: none
---
## Goal
Make HARMONY performable in one gesture: the hold-GLO page with K2 KEY, K3 VOICE, K4 MOVE, OCT-/OCT+ DRONE walk, F3-E4 chord keys, SCL = PROG dice, ROLE per channel, and the chord indicator on PERF.

## Read first
- docs/design/HARMONY.md §5 (placement, controls, typing, indicator, strings, L scores), §6
- docs/design/SCREEN_SPEC.md §7a
- docs/design/key_map.tsv, knob_map.tsv, button_modes.tsv, help_text.tsv rows for SCALE
- docs/design/scenarios/S18

## Deliverable
- ui_sq.c HARMONY page per HARMONY.md §5.2-§5.5 including the undo toasts; chord indicator on PERF (§5.4)
- tests/ui_sq_test.c: S18 end to end; PNG renders

## Acceptance criteria
- [ ] Scenario S18 reproduces in the host test
- [ ] Typing 1-4-5-1 takes the gesture count stated in HARMONY.md §5.3 (test counts inputs)
- [ ] All strings from ui_tables.h
- [ ] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [ ] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## Out of scope
- Engine changes
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
