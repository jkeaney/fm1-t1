---
id: FB-010
title: sq_core stale queued OFF of a HOLD note can cut a later same-pitch note short
phase: P4
type: feedback
status: done
priority: 3
executor: human
depends_on: []
gate: none
session: 2026-11-12 review of P4-004, host test only
---
## Goal
Triage this observation into tasks or bugs, or drop it with a reason.

## Read first
- docs/project/VISION.md
- firmware/src/sq_core.c (sq_emit same-pitch guard, sq_evq_purge_off, sq_step_start)

## Deliverable
- Linked BUG-/P*- ids, or a drop reason, recorded in the log.

## Observation
Found by reading, not by a failing test (the fuzz stays green on 8 seeds). sq_step_start purges queued OFFs of a pitch when the primary ON ends a still-sounding same pitch (sq_evq_purge_off). The emit-time guard in sq_emit (ratchet repeat via sub_note landing on a still-sounding HOLD pitch) does not purge. The HOLD's queued OFF then stays in the queue; if a same-pitch note is sounding when it falls due, sq_emit(kind 1) matches by pitch and ends that note early. No stuck note and no double OFF (the invariants hold); the effect is an early cut of one repeat or note. Needs a HOLD step followed inside its span by a ratcheting hit of the same pitch, which STAGE may never produce. Suggested fix if it matters: call sq_evq_purge_off(c, note) in the sq_emit guard, or tag OFFs with an instance id. Also open for the owner/design: ratchet velocity slope is 0 and ratchet is skipped on slide/hold/tie steps (ADR-0003 silent).

## Acceptance criteria
- [ ] Converted into backlog items (ids listed in the log) or dropped with a reason.

## Verification
```sh
python3 tools/backlog.py check
```

## Log

- 2026-11-12 orchestrator triage: filed BUG-006 (stale HOLD OFF, low severity, priority 2). The ratchet slope / skip rules are not a separate item: the locked prototypes define them and P4-006/P5-001 check the core against them.
