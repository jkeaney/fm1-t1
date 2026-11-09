---
id: P2-009
title: Apply owner decisions 32b and 39 to the control map documents
phase: P2
type: task
status: todo
priority: 1
executor: haiku
depends_on: [P2-001, P2-002]
gate: none
---
## Goal
Remove the CHORD ENTRY menu row (owner decision 39) from the control map documents so they agree with SYSTEM_MAP (CONTROL tab has 4 rows, MENU rows total 28), without changing any other behaviour.

## Read first
- docs/project/OWNER_QUESTIONS.md (decision 39, 32b)
- docs/design/SYSTEM_MAP.md (the menu row table: 37 -> 28 rows)
- docs/design/CONTROL_MAP.md, docs/design/UX_REVIEW.md, docs/design/control_map.tsv
- tools/check_control_map.py

## Deliverable
- CHORD ENTRY removed (every mention) from docs/design/CONTROL_MAP.md, docs/design/UX_REVIEW.md and docs/design/control_map.tsv, keeping MD and TSV in agreement; any menu-row count in these files changed to match SYSTEM_MAP (CONTROL tab 4 rows, 28 total); a one-line change note at the top of each edited file.

## Acceptance criteria
- [ ] `grep -rni 'chord entry' docs/design/CONTROL_MAP.md docs/design/UX_REVIEW.md docs/design/control_map.tsv` prints nothing
- [ ] `python3 tools/check_control_map.py` prints OK
- [ ] No other row of control_map.tsv changed (git diff shows only CHORD ENTRY lines and counts)
- [ ] `python3 tools/backlog.py check` reports 0 errors

## Verification
```sh
grep -rni 'chord entry' docs/design/CONTROL_MAP.md docs/design/UX_REVIEW.md docs/design/control_map.tsv   # prints nothing
python3 tools/check_control_map.py                                                                         # prints OK
git diff --stat
```

## Out of scope
- Any other design change; firmware.

## Log
