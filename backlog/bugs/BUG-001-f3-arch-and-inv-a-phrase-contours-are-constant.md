---
id: BUG-001
title: F3 - ARCH and INV A phrase contours are constant
phase: P3
type: bug
status: review
priority: 2
executor: sonnet
depends_on: [P3-001]
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

### 2026-11-11 executor (sonnet)
- Fix: quad now `norm*norm*max/(total*total)` in cases 5 and 6 (torso_t1_algo.c); test added in tests/torso_phase2_test.c.
- Old code (stash): `FAILED: test_harmony_and_voicing_engine (line 263): ARCH has more than one value`.
- New code: `ALL 106 TORSO T-1 PHASE 2 TESTS PASSED`.
- `sh tools/check.sh`: `RAM .data+.bss 23060 B of 73728`, `CHECK PASS`.
- docs/T1_ISSUES.md F3 marked FIXED (summary counts line not updated).
