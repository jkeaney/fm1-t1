---
id: P2-004
title: ADR-0003: sequencer core architecture and the algorithm interface
phase: P2
type: task
status: review
priority: 1
executor: sonnet
depends_on: [P1-010, P2-002]
gate: human-review
---
## Goal
Fix the data model and interfaces that P4–P6 build on: channels, steps, the algorithm plugin interface (layers, seeds, morph parameters), the event router to MIDI and the preview synth, timing, memory budget.

## Read first
- docs/project/OWNER_QUESTIONS.md (owner decisions: scale, GATE/DIV/SWING are global; ACID has slide)
- docs/design/ALGORITHM_SET.md
- docs/design/SYSTEM_MAP.md
- docs/t1-algorithms/01-architecture.md, 08-output-and-dispatch.md
- firmware/src/t1_glue.c, torso_t1_pipeline.c
- CLAUDE.md (Architecture)

## Deliverable
- docs/project/decisions/ADR-0003-sequencer-core.md: C structs for Channel and Step (with sizes, and a _Static_assert plan), the algorithm vtable (e.g. `init`, `on_step`, `morph`, `dice(layer)`), the seed model (per channel, per layer), the morph parameter ranges, the event path ISR→MIDI ring→midi_out_event and to the preview synth, RAM and pool budget per channel, and what of the T-1 code is reused, rewritten or dropped
- docs/project/decisions/README.md updated

## Acceptance criteria
- [x] Every v1 algorithm in ALGORITHM_SET fits the vtable (a short mapping table)
- [x] The RAM budget sums to at most the headroom freed by SYSTEM_MAP plus the current headroom, minus 4 KB
- [ ] The owner approved it (log line)
- [x] The algorithm interface declares layers, morph targets and dice targets as data tables so remapping a knob/button needs no algorithm code change; 'Flex points' section included
- [x] The architecture states the boot sequence into the sequencer and the power-on defaults (owner decision 32b); no groovebox mode switch exists

## Verification
```sh
test -f docs/project/decisions/ADR-0003-sequencer-core.md
```

## Out of scope
- Writing the implementation.

## Log

### 2026-11-10 executor (sonnet)
- Wrote docs/project/decisions/ADR-0003-sequencer-core.md (structs with sizes and _Static_assert plan, AlgoDesc vtable and data tables, seed model, timing 192 PPQN with swing/ratchet/slide ordering, event path, note-off guarantees, pins/KEYS ONLY, presets, RAM table, boot/defaults, T-1 reuse table, host-test strategy, flex points, 12 numbered dev prerequisites). Updated decisions/README.md (0003 proposed, 0004 added).
- RAM: core 2185 B + UI allowance 3072 = 5257 B .bss vs 42400 B allowed (46496-4096) after E3, and 13888 B today; pool 6408 B. Struct sizes hand-computed (PulseOut 6, ChanSave 86, UndoRec 34, SeqPreset 356, SeqEvent 8, ChanRT 24); not compiled (design only).
- Verification: `test -f docs/project/decisions/ADR-0003-sequencer-core.md` -> exit 0.
- Not ticked: "The owner approved it" (no Approved line; human gate).
- New owner questions 43-47 (swing formula, GATE neutral point, bar-quantised preset load, per-channel undo record, no cut on algo/MIDI change) added to OWNER_QUESTIONS.md.
