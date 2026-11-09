---
id: P2-008
title: Algorithm reference prototypes with golden outputs
phase: P2
type: task
status: todo
priority: 1
executor: haiku
depends_on: [P1-010, P2-004]
gate: human-review
---
## Goal
Prove each v1 algorithm in a seeded host prototype and check in its golden output, so dev tasks reproduce known-good behaviour (ADR-0004).

## Read first
- docs/project/decisions/ADR-0004-design-lock.md
- docs/project/decisions/ADR-0003-sequencer-core.md
- the P1-010 v1 shortlist and layer/morph model
- docs/t1-algorithms/verification/ (probe pattern to copy)

## Deliverable
- docs/design/algorithms/<name>/SPEC.md: parameters (range, default), procedure, morph and dice behaviour per layer
- docs/design/algorithms/<name>/proto.c: seeded, deterministic, freestanding-compatible
- docs/design/algorithms/<name>/golden.txt: output for fixed seeds/morph/dice inputs
- tests/design/run_protos.sh running all prototypes against golden

## Acceptance criteria
- [ ] One folder per v1 algorithm
- [ ] Same seed gives byte-identical output on two runs
- [ ] Morph at min/mid/max and each dice layer each have golden cases
- [ ] Music-theory checks (scale, range) pass; theory beats the fixture
- [ ] SPEC.md has no 'TBD'
- [ ] Design locked: owner line in the log

## Verification
```sh
sh tests/design/run_protos.sh   # prints PROTOS PASS
```

## Out of scope
- Changing the locked designs, or touching firmware outside docs/design and tests/design.

## Log
