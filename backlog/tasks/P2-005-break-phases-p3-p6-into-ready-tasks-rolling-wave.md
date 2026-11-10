---
id: P2-005
title: Break phases P3–P6 into ready tasks (rolling-wave planning)
phase: P2
type: task
status: todo
priority: 1
executor: sonnet
depends_on: [P2-001, P2-002, P2-003, P2-004, P2-006, P2-007, P2-008, P2-009, P2-010, P2-011, P2-012, P2-013, P2-014, P2-015]
gate: human-review
---
## Goal
Turn epics E3–E6 into Haiku-executable tasks, using the approved maps, spec and ADR.

### Additions (owner, 2026-11-10)
- Decision 80: the **fx-bus removal is the first dev task in E3**, so 16 preset slots fit.
- Decision 82: **E5 splits into two dev tasks**: (1) ACID, EUCLID, SNAKE and STAGE, (2) ARP, PATTERNS and HARMONY.
- P2-011 review note: the MIDI channel commits on **any** exit from CHAN, including PLAY or another page's button, not only after 0.6 s of stillness. Put this in the CHAN dev task's acceptance criteria.
- P2-015 (keybed expression use cases) feeds the force-sensing dev tasks.

## Read first
- docs/project/PROCESS.md (Definition of Ready, model routing)
- docs/project/decisions/ADR-0004-design-lock.md
- docs/design/*.md, docs/design/scenarios/, docs/design/algorithms/
- docs/project/decisions/ADR-0003-sequencer-core.md
- backlog/tasks/E3-*.md … E6-*.md
- backlog/_templates/task.md

## Deliverable
- Task files P3-nnn … P6-nnn in backlog/tasks/ with full Definition-of-Ready content and depends_on chains; E3–E6 set to status done with a log line listing their tasks; backlog/BACKLOG.md regenerated

## Acceptance criteria
- [ ] `python3 tools/backlog.py check` reports 0 errors
- [ ] Every P3 removal task names exact files to delete, the felucca.c include lines to drop and the test lines to remove, and ends with tools/check.sh
- [ ] Every task with executor haiku names exact files, commands and expected output
- [ ] Every dev task cites, in Read first, a locked spec section plus a golden file or scenario step, and its criteria reproduce those artefacts (ADR-0004)
- [ ] No dev task contains 'TBD', 'choose' or 'decide'; any gap found is filed as a BUG/FB against the design, not guessed
- [ ] The owner approved it (log line)
- [ ] The first dev task in the removal chain links the real T-1/sequencer sources into the host tests (owner decision 42), before any removal
- [ ] Hardware-test gated tasks exist, with a TB-3 checklist (docs/project/HARDWARE_TEST.md) covering steps, notes, slides, accents, morph and dice, for the owner's TB-3 session (owner decision P0-005 note)
- [ ] Every dev task's review step reports RAM headroom from the build line (memory trigger, E9)

## Verification
```sh
python3 tools/backlog.py check   # 0 errors
python3 tools/backlog.py list --phase P3
```

## Out of scope
- Executing any of the new tasks.

## Log
