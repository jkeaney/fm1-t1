---
id: P2-005
title: Break phases P3–P6 into ready tasks (rolling-wave planning)
phase: P2
type: task
status: todo
priority: 1
executor: opus
depends_on: [P2-001, P2-002, P2-003, P2-004, P2-006]
gate: human-review
---
## Goal
Turn epics E3–E6 into Haiku-executable tasks, using the approved maps, spec and ADR.

## Read first
- docs/project/PROCESS.md (Definition of Ready, model routing)
- docs/design/*.md
- docs/project/decisions/ADR-0003-sequencer-core.md
- backlog/tasks/E3-*.md … E6-*.md
- backlog/_templates/task.md

## Deliverable
- Task files P3-nnn … P6-nnn in backlog/tasks/ with full Definition-of-Ready content and depends_on chains; E3–E6 set to status done with a log line listing their tasks; backlog/BACKLOG.md regenerated

## Acceptance criteria
- [ ] `python3 tools/backlog.py check` reports 0 errors
- [ ] Every P3 removal task names exact files to delete, the felucca.c include lines to drop and the test lines to remove, and ends with tools/check.sh
- [ ] Every task with executor haiku names exact files, commands and expected output
- [ ] The owner approved it (log line)

## Verification
```sh
python3 tools/backlog.py check   # 0 errors
python3 tools/backlog.py list --phase P3
```

## Out of scope
- Executing any of the new tasks.

## Log
