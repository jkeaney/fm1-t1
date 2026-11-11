---
id: P4-006
title: Edit layer, pages and step resolution (sq_eval)
phase: P4
type: task
status: todo
priority: 2
executor: haiku
depends_on: [P4-005]
gate: none
---
## Goal
Per-step overrides (StepEdit, AUTO vs set), pages of 16 up to 64 steps, the length change rule and page sources, resolved in the ISR by `sq_eval` exactly as specified.

## Read first
- docs/design/SEQUENCER_UX.md §1.1-§1.6 (model, pages, page sources, FOLLOW, length rule), §9.1-§9.3 (structs, resolution, determinism)
- docs/project/decisions/ADR-0003-sequencer-core.md §9 as amended

## Deliverable
- sq_core.c: `sq_eval` per SEQUENCER_UX 9.2; `sq_step_set/clear`, `sq_len_set` (extend duplicates, shrink hides), `sq_page_add`, page source handling, FOLLOW flag
- tests/sq_edit_test.c: one case per row of the SEQUENCER_UX 1.2 table and per worked example in 1.3-1.6, as golden text tests/golden/sq_edit.txt

## Acceptance criteria
- [ ] sq_edit_test passes
- [ ] A forced-OFF step stays silent after HITS dice; a forced-ON step keeps playing as DENS goes down (1.2 rows)
- [ ] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [ ] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## UX gate
n/a, internal work (tooling, plumbing, docs, process); no producer-visible control, screen or sound change.

## Out of scope
- UI pages
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
