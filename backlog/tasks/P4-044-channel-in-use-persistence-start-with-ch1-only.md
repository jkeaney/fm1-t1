---
id: P4-044
title: Channel in-use persistence: start with CH1 only
phase: P4
type: task
status: todo
priority: 2
executor: sonnet
depends_on: []
gate: none
---
## Goal
Power-on shows only the channels that exist: CH1 in use, the others inert until their algorithm rows exist, and the 'channel in use' state is stored with the preset so P7 can restore it.

## Read first
- /tmp/readiness_review.md finding 7
- firmware/src/ui_sq.c ~115 (in-use state is UI-only)
- firmware/src/sq_chan.c ~535-539 (inert empty rows) and the SeqPreset code ~586-666
- docs/project/decisions/ADR-0003-sequencer-core.md section 22 (SeqPreset layout)
- backlog/feedback/FB-014-cutover-null-algo-rows.md

## Deliverable
- 'Channel in use' becomes core state (a bit per channel in a struct that is already saved or has room, with the size assertion still true); first boot starts with CH1 only
- Adding a channel is possible only for channels with a filled algorithm row; the behaviour is a table read (`ALGOS[]`), not a name check (ADR-0007)
- Tests: power-on state, add and remove channel, a save/load round trip of the bit

## Acceptance criteria
- [ ] Power-on host test shows 1 channel in use
- [ ] The in-use bit survives the SeqPreset round trip in the host test
- [ ] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [ ] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## UX gate
Producer-facing but small: update docs/design/features/P4-015-chan-page-*.md Gate A rows for the new power-on state; Gate B on the unit with the next flash.

## Out of scope
- Flash storage (P7)
- New algorithm rows
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
- 2026-11-13: created from the POC readiness review (/tmp/readiness_review.md, copy the findings you need into the Log; /tmp may be gone).
