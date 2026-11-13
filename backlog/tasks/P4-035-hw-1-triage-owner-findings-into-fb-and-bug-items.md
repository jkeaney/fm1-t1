---
id: P4-035
title: HW-1 triage: owner findings into FB and BUG items
phase: P4
type: task
status: todo
priority: 1
executor: sonnet
depends_on: [P4-021]
gate: none
---
## Goal
Turn every problem the owner reports during HW-1 (first flash on the TB-3) into a FB or BUG item with a phase, a priority and an owner task, so none is lost and the queue is honest before P5.

## Read first
- backlog/tasks/P4-021-*.md (Log: the owner's HW-1 notes and the checklist results)
- docs/project/HARDWARE_TEST.md section HW-1
- docs/project/PROCESS.md (Bugs and feedback loop)
- backlog/_templates/feedback.md and bug.md
- docs/project/HANDOFF.md

## Deliverable
- One `backlog/feedback/FB-nnn-*.md` per observation not yet filed, or a `backlog/bugs/BUG-nnn-*.md` for a defect with a reproduction (next free numbers: FB-020, BUG-012 at the time of writing)
- Each item names its build SHA, the HW-1 checklist line it came from, a severity, and an owner task (existing or new); a High bug in a shipped feature is priority 0
- A triage table in this task's Log: item -> FB/BUG id -> task or dropped with a reason
- Existing P4/P5/P6 task files edited only to add the new item to Read first, nothing else

## Acceptance criteria
- [ ] Every problem line in the P4-021 Log has an FB or BUG id in the triage table, or is dropped with a stated reason
- [ ] No FB left at `status: todo` without a triage decision in the table
- [ ] `python3 tools/backlog.py check` shows 0 errors

## Verification
```sh
python3 tools/backlog.py check      # 0 errors
python3 tools/backlog.py list | grep -E 'FB-|BUG-'
```

## UX gate
n/a, internal work (tooling, plumbing, docs, process); no producer-visible control, screen or sound change.

## Out of scope
- Fixing anything; asking the owner new questions (list them in the Log for the orchestrator)
- Writing the owner's `Approved:` line for P4-021
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
- 2026-11-13: created from the POC readiness review (/tmp/readiness_review.md, copy the findings you need into the Log; /tmp may be gone).
- Review: self (reviewer = executor re-reads the table against the P4-021 Log).
