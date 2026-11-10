---
id: P4-008
title: Timing T-B: tools/fm1_timing.py measurement tool
phase: P4
type: task
status: todo
priority: 2
executor: haiku
depends_on: [P2-005]
gate: none
---
## Goal
The Mac-side measurement tool of TIMING_SPEC 6.2, so the timing gate has numbers, not impressions. Independent of firmware: can run any time.

## Read first
- docs/design/TIMING_SPEC.md §2 (thresholds T1-T13), §6.2
- tools/fm1_midi_listen.py (port finding only; do not extend it)

## Deliverable
- tools/fm1_timing.py: python-rtmidi callback with driver timestamps; logs every message including 0xF8 0xFA 0xFB 0xFC 0xF2 as `time_ns status d1 d2`; `--loopback` IAC calibration; report lines and verdicts per TIMING_SPEC 6.2; `--sweep` 60/90/120/180/240; exit 0 only if every threshold passes
- tests/fm1_timing_selftest.py: feeds a synthetic log (perfect grid, jittered grid, missing clock pulse) to the analysis functions and checks the verdicts; run_tests.sh step

## Acceptance criteria
- [ ] `.venv/bin/python tests/fm1_timing_selftest.py` prints `TIMING SELFTEST PASS`
- [ ] `.venv/bin/python tools/fm1_timing.py --help` lists --loopback, --sweep, --port, --minutes

## Verification
```sh
.venv/bin/python tests/fm1_timing_selftest.py
.venv/bin/python tools/fm1_timing.py --help
```

## Out of scope
- Firmware
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
