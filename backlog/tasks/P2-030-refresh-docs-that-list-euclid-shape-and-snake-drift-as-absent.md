---
id: P2-030
title: Refresh the docs that still list EUCLID SHAPE and SNAKE DRIFT as absent layers
phase: P2
type: task
status: todo
priority: 2
executor: sonnet
depends_on: [P2-017]
gate: none
---
## Goal
After P2-022 made EUCLID SHAPE and SNAKE DRIFT real MOD layers, make every design doc and table say so, so the screen, help text and scenarios the P4 UI tasks read agree with the locked prototypes.

## Read first
- backlog/tasks/P2-017-*.md Log (reviewer finding), docs/design/algorithms/LOCK.md, docs/design/MODULATION.md
- SCREEN_SPEC.md 2.7, UX_REVIEW.md, ALGORITHM_SET.md 4.4, SEQUENCER_UX.md, CONTROL_MAP.md line ~34, PERFORMANCE_DESIGN.md, tests/design/widths.tsv line ~98

## Deliverable
- Each listed doc/table updated so EUCLID SHAPE and SNAKE DRIFT are real layers (primary names per the owner's choice in LOCK.md 6 item 5), and the dead toast in widths.tsv removed or corrected.
- Generated tables regenerated with their generators (gen_help.py, gen_sweeps.py), not hand-edited.

## Acceptance criteria
- [ ] `grep -rniE "SHAPE.*absent|absent.*SHAPE|DRIFT.*absent|absent.*DRIFT" docs/design` finds no remaining stale claim
- [ ] `python3 tests/design/check_scenarios.py`, `check_help.py` and `sh tests/design/width_probe.sh` pass
- [ ] `python3 tools/backlog.py check` reports 0 errors

## Verification
```sh
python3 tests/design/check_scenarios.py && python3 tests/design/check_help.py && sh tests/design/width_probe.sh
python3 tools/backlog.py check
```

## UX gate
n/a, documentation consistency only; it records a layer change already designed in P2-022 and locked in P2-017.

## Out of scope
- Firmware; ADR-0003 and HARMONY.md (P2-029).

## Log
