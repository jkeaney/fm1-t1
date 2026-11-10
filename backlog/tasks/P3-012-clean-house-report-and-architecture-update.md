---
id: P3-012
title: Clean-house report and architecture update
phase: P3
type: task
status: todo
priority: 2
executor: haiku
depends_on: [P3-011, BUG-001, BUG-002]
gate: none
---
## Goal
Record what P3 freed and leave CLAUDE.md describing the cleaned firmware, so P4 starts from true numbers.

## Read first
- docs/design/SYSTEM_MAP.md §9 (predicted bytes)
- docs/design/inventory/ (the scripts that measured the inventory)
- CLAUDE.md (Architecture section)
- git log of P3-002..P3-011 and P3-014 (their Log build lines)

## Deliverable
- docs/design/CLEAN_HOUSE_REPORT.md: a table of predicted vs measured flash/RAM/pool freed per removal step, the final build line, and the re-run inventory of ADAPT rows
- CLAUDE.md Architecture and include-order text updated to the files that remain (no mention of removed engines, editor, layers, chain)
- docs/project/HANDOFF.md 'Budget facts' updated with the measured headroom

## Acceptance criteria
- [ ] CLEAN_HOUSE_REPORT.md exists with one row per step R1-R7
- [ ] `grep -nE 'ui_layer|song_chain|editor\.c|eng_drum' CLAUDE.md` returns nothing
- [ ] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [ ] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## Out of scope
- Code changes
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
