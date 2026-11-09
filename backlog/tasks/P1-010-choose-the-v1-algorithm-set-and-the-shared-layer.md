---
id: P1-010
title: Choose the v1 algorithm set and the shared layer/morph model
phase: P1
type: task
status: todo
priority: 1
executor: sonnet
depends_on: [P1-011, P1-003, P1-004, P1-005, P1-006, P1-007, P1-008, P1-009]
gate: human-review
---
## Goal
One coherent algorithm menu for channels: which algorithms ship in v1, the layers they share (so the dice buttons mean the same thing everywhere) and a consistent knob morph scheme.

## Read first
- docs/project/OWNER_QUESTIONS.md (owner decisions: scale, GATE/DIV/SWING are global; ACID has slide)
- docs/design/algorithms/*.md (all 7 assessments)
- docs/project/VISION.md

## Deliverable
- docs/design/ALGORITHM_SET.md: the v1 list (3–6 algorithms) with a reason for each; the shared layer vocabulary (e.g. RHYTHM, PITCH, ACCENT, LENGTH) and which algorithm supports which; the knob convention (e.g. K1 always density-like, K4 always variation); an implementation order; the rejected or later list

## Acceptance criteria
- [ ] Every INCLUDE recommendation from the assessments is either in the v1 list or explicitly deferred with a reason
- [ ] The layer vocabulary has at most 5 layers, and every v1 algorithm maps onto it
- [ ] The owner approved it (log line 'Approved: <name> <date>')

## Verification
```sh
test -f docs/design/ALGORITHM_SET.md && grep -c '^|' docs/design/ALGORITHM_SET.md
```

## Out of scope
- Interface code (that's P2-004).

## Log
