---
id: FB-014
title: P4-012 cut-over would run channels 2-4 on ALGOS[] rows with NULL build/step
phase: P4
type: feedback
status: todo
priority: 2
executor: sonnet
depends_on: []
gate: none
session: 2026-11-12, review of P5-001, code reading only
---
## Goal
Make sure no empty ALGOS[] row can be selected or defaulted once sq_core.c / sq_chan.c enter the build.

## Read first
- firmware/src/sq_algos.c (only row 0 ACID is filled; rows 1-5 are zero)
- firmware/src/sq_chan.c:57-62 (sq_rebuild calls a->build unguarded), :209-212 (sq_set_algo checks only < SQ_NALGO), :218-221 (sq_default_chan gives CH2..CH4 EUCLID, SNAKE, STAGE)
- backlog/tasks/P4-012-*.md (depends_on P4-006, P4-009, P4-011, P5-001 only)

## Deliverable
- Either add P5-002, P5-003, P5-004 to P4-012's depends_on, or guard sq_set_algo / sq_default_chan / sq_apply_load / sq_rebuild against rows with build == NULL (and fall back to ACID).

## Observation
P5-001 leaves rows 1-5 zero. Nothing dereferences them today because sq_core.c and sq_chan.c are not in felucca.c. P4-012 cuts them in without P5-002..4, so CH2..CH4 default to rows whose build is NULL and the first rebuild jumps to address 0 (also, EN_ALGO could select such a row, and a preset load only checks algo < SQ_NALGO).

## Acceptance criteria
- [ ] Converted into a dependency or guard task (ids listed in the log) or dropped with a reason.

## Verification
```sh
python3 tools/backlog.py check
```

## Log
