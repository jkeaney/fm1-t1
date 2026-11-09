---
id: P2-006
title: Triage the T-1 issues (F1–F19) against the new architecture
phase: P2
type: task
status: todo
priority: 2
executor: sonnet
depends_on: [P2-004]
gate: none
---
## Goal
Each open T-1 defect either becomes a BUG item (the code survives into the new design) or is dropped (the code is being replaced).

## Read first
- docs/T1_ISSUES.md
- docs/project/decisions/ADR-0003-sequencer-core.md (what of the T-1 code is reused)
- backlog/_templates/bug.md

## Deliverable
- One backlog/bugs/BUG-nnn-*.md per surviving issue, with its F-number in the title and repro copied from T1_ISSUES; a line in T1_ISSUES.md under each issue: 'Tracked as BUG-nnn' or 'Dropped: <reason>'

## Acceptance criteria
- [ ] Every OPEN or PARTIAL issue in T1_ISSUES.md has a 'Tracked as' or 'Dropped' line
- [ ] `python3 tools/backlog.py check` reports 0 errors

## Verification
```sh
grep -cE 'Tracked as BUG-|Dropped:' docs/T1_ISSUES.md
python3 tools/backlog.py check
```

## Out of scope
- Fixing the bugs.

## Log
