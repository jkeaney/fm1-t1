---
id: P2-009
title: Apply owner decisions 32b and 39 to the control map documents
phase: P2
type: task
status: review
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
- [x] `grep -rni 'chord entry' docs/design/CONTROL_MAP.md docs/design/UX_REVIEW.md docs/design/control_map.tsv` prints nothing
- [x] `python3 tools/check_control_map.py` prints OK
- [x] No other row of control_map.tsv changed (git diff shows only CHORD ENTRY lines and counts)
- [x] `python3 tools/backlog.py check` reports 0 errors

## Verification
```sh
grep -rni 'chord entry' docs/design/CONTROL_MAP.md docs/design/UX_REVIEW.md docs/design/control_map.tsv   # prints nothing
python3 tools/check_control_map.py                                                                         # prints OK
git diff --stat
```

## Out of scope
- Any other design change; firmware.

## Log

### 2026-11-10 executor (claude-haiku-5-5)
Files changed: `docs/design/CONTROL_MAP.md`, `docs/design/UX_REVIEW.md`. `docs/design/control_map.tsv` has no chord row (no `chord` text in it), so it is unchanged. No menu-row count in these two files needed a change: they name no row count, and the CONTROL tab list in each now has 4 items, matching SYSTEM_MAP section 7 (CONTROL 4, total 28).

Each edited file gets a one-line change note under its H1 title. The TSV got no note, because the checker reads it as a header-plus-rows table.

Commands and output:
```
grep -rni 'chord entry' docs/design/CONTROL_MAP.md docs/design/UX_REVIEW.md docs/design/control_map.tsv
(no output, exit 1)
python3 tools/check_control_map.py
rows tsv=343 md=343
OK
git diff --stat
 docs/design/CONTROL_MAP.md | 4 +++-
 docs/design/UX_REVIEW.md   | 4 +++-
 2 files changed, 6 insertions(+), 2 deletions(-)
python3 tools/backlog.py check
33 items, 0 errors
```
The first grep pass matched the change note itself, so the note was reworded to "the chord row of the CONTROL tab" before the final run above.

Acceptance criteria: all four ticked and verified above. Only docs were changed: no build, no firmware, no flash, no push.
