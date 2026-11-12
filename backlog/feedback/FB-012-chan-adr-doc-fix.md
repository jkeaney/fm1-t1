---
id: FB-012
title: ADR-0003 section 10 vs section 12 and the code: bad preset, who clears UndoRec, dice entropy (doc-fix)
phase: P4
type: feedback
status: todo
priority: 2
executor: human
depends_on: []
gate: none
session: 2026-11-12 review of P4-005, host tests only
---
## Goal
Triage this observation into tasks or bugs, or drop it with a reason.

## Read first
- docs/project/VISION.md
- backlog/tasks/P4-005-channel-state-api-defaults-undo-and-ram-presets-wi.md (Log and review)
- firmware/src/sq_chan.c

## Deliverable
- Linked BUG-/P*- ids, or a drop reason, recorded in the log.

## Observation
Review ruling for P4-005. (1) Section 10 says an invalid version or CRC loads the power-on defaults; section 12 step 6 and the code reject (sq_preset_load returns -1, state untouched; the caller can load sq_defaults() through the same path). Reject wins: loading defaults over the live pattern because a slot is corrupt destroys the player's work; the storage layer (P7-002) decides what an empty or corrupt slot shows. Amend section 10 to say that. (2) Section 10 says the ISR clears every UndoRec at the bar; UndoRec is main-loop memory, so the ISR only bumps sq_epoch and the main loop's sq_chan_sync() clears the records: amend section 10/6 and name sq_chan_sync in the P4-011 / P4-012 hooks. (3) Section 3.1 gives dice entropy as the sample counter XOR fm1_ms<<3; the code uses the free-running counter (which counts while stopped) mixed with a dice counter. Still the only non-deterministic moment, and the result is stored; amend 3.1 to match, no fm1_ms hook needed. (4) Executor note: the nit sq_undo_new in sq_chan.c is unused (dead code that -Wunused-function will flag in the unity build): delete at cut-over.

## Acceptance criteria
- [ ] Converted into backlog items (ids listed in the log) or dropped with a reason.

## Verification
```sh
python3 tools/backlog.py check
```

## UX gate
n/a, internal work (tooling, plumbing, docs, process); no producer-visible control, screen or sound change.

## Log
