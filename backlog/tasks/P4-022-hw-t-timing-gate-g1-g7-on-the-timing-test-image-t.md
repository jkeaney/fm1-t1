---
id: P4-022
title: HW-T: timing gate G1-G7 on the timing-test image (T-F)
phase: P4
type: task
status: todo
priority: 2
executor: human
depends_on: [P4-019, P4-008, P4-021]
gate: hardware-test
---
## Goal
Measure the timing on hardware and decide the M3 release mode, against the TIMING_SPEC thresholds.

## Read first
- docs/design/TIMING_SPEC.md §2, §6.4
- docs/project/HARDWARE_TEST.md section HW-T

## Deliverable
- Owner OK to flash build/felucca_timing_test.fwsc; afterwards reflash the normal build
- Owner runs G1-G7 with tools/fm1_timing.py and Ableton; the tool output is pasted in the Log

## Acceptance criteria
- [ ] G1-G7 each recorded pass/fail with numbers
- [ ] Decision recorded: nested TIMER5 release kept, or the fallback flag set (then a follow-up task flips the default)
- [ ] Approved: owner line

## Verification
```sh
.venv/bin/python tools/fm1_timing.py --loopback
.venv/bin/python tools/fm1_timing.py --sweep --minutes 5
```

## UX gate
Gate B on real hardware: the owner walks the cited use cases on the FM-1 and TB-3; actual ease scores and live checklists are recorded in each use case (docs/project/UX_GATE.md).

## Out of scope
- Code changes

## Log
