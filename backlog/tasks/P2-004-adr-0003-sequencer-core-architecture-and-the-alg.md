---
id: P2-004
title: ADR-0003: sequencer core architecture and the algorithm interface
phase: P2
type: task
status: todo
priority: 1
executor: opus
depends_on: [P1-010, P2-002]
gate: human-review
---
## Goal
Fix the data model and interfaces that P4–P6 build on: channels, steps, the algorithm plugin interface (layers, seeds, morph parameters), the event router to MIDI and the preview synth, timing, memory budget.

## Read first
- docs/design/ALGORITHM_SET.md
- docs/design/SYSTEM_MAP.md
- docs/t1-algorithms/01-architecture.md, 08-output-and-dispatch.md
- firmware/src/t1_glue.c, torso_t1_pipeline.c
- CLAUDE.md (Architecture)

## Deliverable
- docs/project/decisions/ADR-0003-sequencer-core.md: C structs for Channel and Step (with sizes, and a _Static_assert plan), the algorithm vtable (e.g. `init`, `on_step`, `morph`, `dice(layer)`), the seed model (per channel, per layer), the morph parameter ranges, the event path ISR→MIDI ring→midi_out_event and to the preview synth, RAM and pool budget per channel, and what of the T-1 code is reused, rewritten or dropped
- docs/project/decisions/README.md updated

## Acceptance criteria
- [ ] Every v1 algorithm in ALGORITHM_SET fits the vtable (a short mapping table)
- [ ] The RAM budget sums to at most the headroom freed by SYSTEM_MAP plus the current headroom, minus 4 KB
- [ ] The owner approved it (log line)

## Verification
```sh
test -f docs/project/decisions/ADR-0003-sequencer-core.md
```

## Out of scope
- Writing the implementation.

## Log
