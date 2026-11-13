---
id: P4-040
title: ACID finish: draw generated slides, gate/hold/tie, and a visible dice for GT+SLD
phase: P4
type: task
status: todo
priority: 1
executor: sonnet
depends_on: [P4-021]
gate: human-review
---
## Goal
The music row shows what ACID really plays (generated slides, gate, hold, tie) and the GT+SLD dice visibly changes the screen, so the screen always shows what the device is doing (product rule 1).

## Read first
- /tmp/readiness_review.md finding 1 and section C
- firmware/src/ui_sq.c ~375-390 (`UisqStep`, slide reads the edit only) and ~816-831 (`uisq_view_fill`, the diced diff)
- docs/design/SCREEN_SPEC_V2.md (music row, marks)
- docs/design/ui_review/proposals/v12/ and tests/v12_compare.py
- docs/design/algorithms/acid/SPEC.md (GATE lanes, SLIDE, ARTIC)

## Deliverable
- `UisqStep` gains gate/hold/tie and the generated slide; the music row draws them; the 'just diced' diff includes GT+SLD so re-rolling ARTIC changes what is drawn
- New mark designs shown as renders in docs/design/ui_review/ for the owner to approve before the code is merged as final
- v12 pixel compare still identical for all existing scenes (52 scenes, 0 failed) and new scenes added for the new marks
- Host test: re-rolling seeds 3 and 5 (the case that showed diff = 0) now shows a change

## Acceptance criteria
- [ ] `python3 tests/v12_compare.py` reports the existing scenes unchanged and the new scenes passing
- [ ] The ARTIC dice experiment from the review shows a non-empty diff
- [ ] Owner approval of each new mark (the screen is locked, nothing new ships unapproved)
- [ ] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [ ] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
python3 tests/v12_compare.py   # all scenes pass
```

## UX gate
Producer-facing. Update the use cases that show the music row (docs/design/features/P4-013-perf-page-*.md) with the new marks; Gate A cited from the locked screen design plus the owner's mark approval; Gate B in the HW pass.

## Out of scope
- Other algorithms' layers (P5)
- LED behaviour (P4-039)
- Changing existing marks
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
- 2026-11-13: created from the POC readiness review (/tmp/readiness_review.md, copy the findings you need into the Log; /tmp may be gone).
- Gate: human-review, because SCREEN_SPEC_V2 is locked and any new mark needs the owner's approval.

- 2026-11-13 owner ruling (open-items 3B): change the shortened hints to the clearer wording: PAST THE END hint `LEN` -> `PG+=ADD`, ONE PAGE hint `K1=LESS` -> `K1=LEN` (check they fit the 136 px slot; MESSAGES rows in v12.py + regenerate tables + all docs/scenarios/help text that quote them, incl. forbidden_terms for the old wording). Do this in this task.
