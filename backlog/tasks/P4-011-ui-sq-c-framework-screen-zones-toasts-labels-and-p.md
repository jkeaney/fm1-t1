---
id: P4-011
title: ui_sq.c framework: screen zones, toasts, labels and page dispatch
phase: P4
type: task
status: todo
priority: 1
executor: sonnet
depends_on: [P4-010, P4-005, P2-031]
gate: none
---
## Goal
The new main UI skeleton every page task fills in: zones Z0-Z6 at the SCREEN_SPEC pixels, the toast, the always-on control labels from ui_tables.h, redraw by state signature, and page dispatch from the page-state table.

## Read first
- docs/design/SCREEN_SPEC.md §1-§5, §7 (zones, labels, modifier relabel, text limits, mock-ups)
- docs/design/SEQUENCER_UX.md §2 (page inventory), §2.7 (transitions, single escape), §3 (button modes)
- firmware/src/ui_t1.c (the pattern being replaced), ui_draw.c (`draw_head`, cv_* canvas), ui_input.c (hold timers)

## Deliverable
- firmware/src/ui_sq.c (+ ui_sq.h if needed): zone renderers (header with slot H2 placeholder, channel cells, lane, band, buttons, legend, cue), toast queue (2.7), label drawing from ui_tables.h, page enum and dispatch with stub pages, `ui_sq_input()`/`ui_sq_draw()` entry points; not yet wired into ui_draw/ui_input (P4-012 does it)
- tests/ui_sq_test.c: host render of each zone to PNG via the existing ui_render path; asserts every label fits its zone using the real `text_w`

## Acceptance criteria
- [ ] ui_sq_test passes and writes build/ui_sq/*.png
- [ ] No label exceeds its zone width (test asserts)
- [ ] ui_sq.c reads only the sq_* API and exported state, never ISR-private fields (reviewer checks)
- [ ] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [ ] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## UX gate
Producer-facing. Use case: `docs/design/features/P4-011-ui-sq-c-framework-screen-zones-toasts-la.md` from `_TEMPLATE.md` (docs/project/UX_GATE.md). Gate A (steps, predicted ease score, five-surface sync, live checklist) is filled before the build or cited from the locked design; Gate B is filled by the reviewer.

## Out of scope
- Page behaviour (P4-013..P4-018)
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
