---
id: P2-031
title: ADR-0003 doc fixes from the P4-005 review (bad preset rejects, sq_chan_sync, dice entropy)
phase: P2
type: task
status: todo
priority: 2
executor: sonnet
depends_on: [P4-005]
gate: none
---
## Goal
Make ADR-0003 say what the reviewed code does (FB-012), so later tasks copy a correct contract.

## Read first
- backlog/feedback/FB-012-*.md, backlog/tasks/P4-005-*.md Log
- docs/project/decisions/ADR-0003-sequencer-core.md sections 3.1, 6, 10, 12

## Deliverable
- Section 10: an invalid version, CRC or algorithm id REJECTS the load (state untouched, `sq_preset_load` returns -1); the storage layer (P7-002) decides what an empty or corrupt slot shows
- Sections 6 and 10: the ISR only bumps `sq_epoch`; the main loop's `sq_chan_sync()` clears the UndoRecs; name `sq_chan_sync` in the P4-011 / P4-012 hook lists (task files)
- Section 3.1: dice entropy is the free-running sample counter mixed with a dice counter; no `fm1_ms` hook

## Acceptance criteria
- [ ] ADR-0003 sections 3.1, 6, 10 and 12 agree with sq_chan.c
- [ ] P4-011 and P4-012 Deliverables name `sq_chan_sync()` and its per-frame call
- [ ] `python3 tools/backlog.py check` reports 0 errors

## Verification
```sh
python3 tools/backlog.py check
```

## UX gate
n/a, specification text.

## Out of scope
- Firmware changes.

## Log
