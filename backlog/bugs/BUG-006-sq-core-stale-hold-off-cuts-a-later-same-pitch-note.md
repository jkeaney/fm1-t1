---
id: BUG-006
title: sq_core stale queued OFF of a HOLD note can cut a later same-pitch note short
phase: P4
type: bug
status: todo
priority: 2
executor: sonnet
depends_on: [P4-004]
gate: none
severity: low   # latent: no stuck note, no double OFF; one note or repeat is cut early
found_in: 6b4862f (P4-004), read by the reviewer; the fuzz stays green
---
## Goal
Fix: a ratchet repeat that lands on a still-sounding HOLD pitch leaves the HOLD's queued OFF in the event queue, so that OFF later ends a same-pitch note early. Expected: the stale OFF is removed, as `sq_step_start` already does for a primary ON.

## Read first
- backlog/feedback/FB-010-*.md (the observation and suggested fix)
- firmware/src/sq_core.c: the `sq_emit` same-pitch guard, `sq_evq_purge_off`, `sq_step_start`
- tests/sq_core_test.c and tests/golden/sq_core_*.txt

## Deliverable
- The fix in `sq_emit` (call `sq_evq_purge_off(c, note)` in the guard, or tag OFFs with an instance id if that proves cleaner), plus a host test (a directed event-level case, and a fuzz assertion if one can be stated) that fails on the old code and passes on the new.

## Steps to reproduce
1. A HOLD step on pitch P with its OFF queued beyond the next step.
2. Inside that span a ratcheting hit of the same pitch P (via `sub_note`).
3. A same-pitch note is sounding when the HOLD's OFF falls due.

## Expected
The later note sounds for its full length.

## Actual
`sq_emit` (kind 1) matches by pitch and ends the later note early. Invariants still hold (ON == OFF per channel and note).

## Acceptance criteria
- [ ] The new test fails on the old code and passes on the new.
- [ ] All nine existing sq_core goldens are unchanged unless the log explains why.
- [ ] The 100000-tick fuzz still passes under ASan/UBSan on at least 3 seeds.
- [ ] `sh tools/check.sh` ends with `CHECK PASS`; the RAM line is pasted into the Log with headroom of at least 8192 B.

## Verification
```sh
sh tools/check.sh
```

## UX gate
n/a, scheduler internals; no control or screen change.

## Out of scope
- Ratchet velocity slope (currently 0) and no ratchet on slide, hold or tie steps: the ADR is silent and the locked prototypes' `sq_ratchet` defines the behaviour. P4-006 and P5-001 confirm the core matches the prototypes.

## Log
