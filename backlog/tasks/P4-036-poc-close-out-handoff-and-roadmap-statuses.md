---
id: P4-036
title: POC close-out: HANDOFF and ROADMAP statuses
phase: P4
type: task
status: done
priority: 1
executor: sonnet
depends_on: []
gate: none
---
## Goal
Make the project's front page tell the truth after HW-1: the new core is flashed and working on the TB-3, what is open and what comes next, including the LED requirement.

## Read first
- docs/project/HANDOFF.md
- docs/project/ROADMAP.md (phase table)
- /tmp/readiness_review.md (findings A-F; summarise in the HANDOFF if /tmp is gone)
- backlog/tasks/P4-021-*.md Log

## Deliverable
- docs/project/HANDOFF.md: state (new core flashed and working on the TB-3 at efe0dc9; branch pushed to origin/t1-hardware-bringup), known debts, next steps (P4-035..044 order), the LED requirement, open owner items (P4-021 `Approved:` line, RIG in v1, MOD output in v1, hard-blink ruling)
- docs/project/ROADMAP.md: P4 row status updated (HW-1 interim flash done, sign-off pending; POC review done), no other row changed
- Re-run the status again at the end of P4 (add a line in the Log for the orchestrator)

## Acceptance criteria
- [x] HANDOFF.md no longer says 'nothing from the new core has been flashed'
- [x] HANDOFF.md lists the LED requirement and the open owner items by task id
- [x] `python3 tools/backlog.py check` shows 0 errors

## Verification
```sh
grep -n 'Nothing from the new core' docs/project/HANDOFF.md   # no output
python3 tools/backlog.py check
```

## UX gate
n/a, internal work (tooling, plumbing, docs, process); no producer-visible control, screen or sound change.

## Out of scope
- Any task file other than this one
- Writing the owner's `Approved:` line
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
- 2026-11-13: created from the POC readiness review (/tmp/readiness_review.md, copy the findings you need into the Log; /tmp may be gone).
- 2026-11-13 (sonnet): HANDOFF.md and ROADMAP.md updated as part of creating this task (state, debts, the P4-035..044 plan, LED requirement, open owner items). Self-reviewed; status done. Re-do the update when P4-021 is signed off.
