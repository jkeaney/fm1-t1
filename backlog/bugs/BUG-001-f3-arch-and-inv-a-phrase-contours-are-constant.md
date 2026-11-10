---
id: BUG-001
title: F3 - ARCH and INV A phrase contours are constant
phase: P3
type: bug
status: todo
priority: 2
executor: sonnet
depends_on: []
gate: none
severity: medium
found_in: bea0fd3 (+ working tree, T1_ISSUES review 2026-11-09)
---
## Goal
Fix: `t1_harmony_eval_phrase` ARCH and INV A contours so that each has a real shape (ARCH peaks near the middle, INV A dips near the middle) instead of a constant.

## Read first
- docs/T1_ISSUES.md, entry F3
- firmware/src/torso_t1_algo.c `t1_harmony_eval_phrase` (cases 5 and 6)
- docs/project/decisions/ADR-0003-sequencer-core.md section 13 (this function is REUSED after F3 is fixed, for the EUCLID contour, SP10) and section 16 prerequisite 4

## Deliverable
- The fix, plus a host test that failed before it. This is ADR-0003 section 16 prerequisite 4 (EUCLID depends on it).

## Steps to reproduce
1. Evaluate the phrase contour for ARCH and INV A over 16 steps with `range = 1` (`docs/t1-algorithms/verification/probe.c`, section 4).

## Expected
Each contour has more than one distinct value for `range >= 2` and `steps >= 4`; ARCH peaks near the middle; INV A dips near the middle.

## Actual
`quad = (norm * norm) / (total_steps * total_steps + 1)` is integer division and is always 0, so ARCH is a constant `+max` (`1 1 1 ...`) and INV A a constant 0.

## Acceptance criteria
- [ ] The new test fails on the old code and passes on the new.
- [ ] `tools/check.sh` passes.

## Verification
```sh
sh tools/check.sh
```

## Log
Created by P2-006 (triage of F3: the function survives into the new design, so the defect is tracked).
