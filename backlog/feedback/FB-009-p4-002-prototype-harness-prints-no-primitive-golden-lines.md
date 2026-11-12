---
id: FB-009
title: P4-002 golden cannot be copied from the prototype run: prims_test.c prints no primitive lines
phase: P4
type: feedback
status: done
priority: 2
executor: human
depends_on: []
gate: none
session: 2026-11-12, HEAD 2602c08, executor run of P4-002 (no firmware change made)
---
## Goal
Decide how the primitive golden of P4-002 is produced, then unblock P4-002.

## Read first
- backlog/tasks/P4-002-sq-prims-c-with-golden-tests.md
- tests/design/prims_test.c
- tests/design/run_protos.sh (step 3, "primitives")
- docs/design/ALGORITHM_SET.md section 6

## Deliverable
- A recorded decision on the golden format, then P4-002 set back to `todo` (or re-scoped).

## Observation
P4-002 says `tests/sq_prims_test.c` "prints the same lines the prototype harness prints for the primitives",
compared byte for byte with `tests/sq_prims_golden.txt`, "copied from the prototype run".

The prototype does not print such lines. Built and run as `run_protos.sh` does (`cc -std=c11 -Wall -Wextra -Werror -O1
-fno-strict-aliasing -Idocs/design/algorithms/common ... tests/design/prims_test.c docs/design/algorithms/common/sq_prims.c`):

- stdout is 0 bytes;
- stderr is the single line `prims: primitive checks OK` (exit 0), or `prims: N check(s) failed` on failure;
- every primitive value is checked with `CHECK()` against an in-source constant. No value is printed.

So there is no output to copy into a golden. A golden of an empty stdout would pass without testing anything, and
writing one would mean inventing a line format, which the task forbids.

## Acceptance criteria
- [ ] The owner (or the design owner of P2-008) chooses one: (a) the golden is the stdout of a new print-mode of
      `prims_test.c` (a named format, one line per primitive vector, with the values to print written down); (b) the
      pass/fail exit of `prims_test.c` stands in for the golden, and P4-002's first criterion is reworded to say so;
      (c) something else.
- [ ] P4-002's first acceptance criterion is reworded to match the choice, and P4-002 is set back to `todo`.

## UX gate
n/a, test-process decision only; no producer-visible control, screen or sound change.

## Verification
```sh
python3 tools/backlog.py check
```

## Log
- 2026-11-12 executor (claude-haiku-5-5): P4-002 stopped before any firmware change. Copying
  `docs/design/algorithms/common/sq_prims.{h,c}` was not done because the golden step cannot be satisfied as written.
  Blocked; see P4-002 Log.

- 2026-11-12 orchestrator triage: not an owner decision (test-format spec gap). Chose option (a): add `--dump` to the prototype harness; P4-002 re-routed to a Sonnet executor with that scope.
