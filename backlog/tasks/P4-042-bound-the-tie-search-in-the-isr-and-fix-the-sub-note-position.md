---
id: P4-042
title: Bound the tie search in the ISR; pass the generator position to sub_note
phase: P4
type: task
status: todo
priority: 1
executor: sonnet
depends_on: [P4-021]
gate: none
---
## Goal
Remove the unbounded worst-case in the interrupt (a tie search that evaluates up to 63 steps per tick) before PATTERNS makes each step() expensive, and fix the position given to the ARP repeat-note hook.

## Read first
- /tmp/readiness_review.md section D (top risk 1) and finding 4
- firmware/src/sq_core.c ~297-306 and ~384 (`sq_tie_end`), ~419 (`sub_note(k, p, ...)`)
- docs/design/TIMING_SPEC.md (interrupt budget)
- docs/project/decisions/ADR-0003-sequencer-core.md

## Deliverable
- Either a bound on the tie search or the tie end precomputed when the pattern changes (choose, and say why in the Log); behaviour identical to today's for all existing goldens
- tests/: a worst-case pattern (every step a tie candidate, length 64) measuring evaluated steps per tick on host; the cost is stated in steps and ops in the Log
- `sub_note` receives the generator position, not the step index (separate Log entry, with a test that fails before the fix)

## Acceptance criteria
- [ ] Worst-case `step()` calls per tick are at most the stated bound (number in the Log), and all sq goldens are unchanged
- [ ] The `sub_note` test fails on the old code and passes on the new
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
- Algorithm changes
- Moving work out of the ISR for other reasons
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
- 2026-11-13: created from the POC readiness review (/tmp/readiness_review.md, copy the findings you need into the Log; /tmp may be gone).
- Must land before P5-008 PATTERNS (P5-008 depends on this task). Keep two Log entries: tie bound, then sub_note.
