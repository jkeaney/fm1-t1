---
id: BUG-002
title: F2 - stale T1_SCALE_NAMES array mislabels the scale masks
phase: P3
type: bug
status: todo
priority: 3
executor: sonnet
depends_on: []
gate: none
severity: low
found_in: bea0fd3 (+ working tree, T1_ISSUES review 2026-11-09)
---
## Goal
Fix: remove the stale `T1_SCALE_NAMES` so that `T1_SCALE_MASKS` is the one scale table and nothing mislabels the masks.

## Read first
- docs/T1_ISSUES.md, entries F2 (residue) and F4 (header comment note)
- firmware/src/torso_t1_algo.c (line ~242) and torso_t1_algo.h (line ~75)
- docs/project/decisions/ADR-0003-sequencer-core.md section 13 (`T1_SCALE_MASKS` reused as the one scale table, delete `T1_SCALE_NAMES`) and section 16 prerequisite 3

## Deliverable
- Delete `T1_SCALE_NAMES` (declaration and definition; it is referenced nowhere). Correct the `range ... in octaves (0 to 7)` comment in `torso_t1_algo.h` to semitones. The new names live with the sequencer scale table (ADR-0003 section 5), not here.

## Steps to reproduce
1. Read `T1_SCALE_NAMES`: it lists the old order (Chromatic, Major, Minor, Dorian, Mixolydian, Maj Pent, ...), which no longer matches the masks.

## Expected
No second, disagreeing names table exists.

## Actual
`T1_SCALE_NAMES` is declared and defined but unused, and its order is wrong.

## Acceptance criteria
- [ ] `grep -rn T1_SCALE_NAMES firmware tests` finds nothing.
- [ ] `tools/check.sh` passes.

## Verification
```sh
grep -rn T1_SCALE_NAMES firmware tests
sh tools/check.sh
```

## Log
Created by P2-006 (triage of F2: `T1_SCALE_MASKS` and its file survive, so the residue is tracked). May be done inside ADR-0003 prerequisite 3.
