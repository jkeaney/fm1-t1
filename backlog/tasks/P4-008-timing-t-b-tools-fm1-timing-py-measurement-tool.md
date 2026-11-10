---
id: P4-008
title: Timing T-B: tools/fm1_timing.py measurement tool
phase: P4
type: task
status: done
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
- [x] `.venv/bin/python tests/fm1_timing_selftest.py` prints `TIMING SELFTEST PASS`
- [x] `.venv/bin/python tools/fm1_timing.py --help` lists --loopback, --sweep, --port, --minutes

## Verification
```sh
.venv/bin/python tests/fm1_timing_selftest.py
.venv/bin/python tools/fm1_timing.py --help
```

## Out of scope
- Firmware
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log

### 2026-11-10 executor (claude-haiku-5-5)

Built `tools/fm1_timing.py` (callback capture with the driver deltas, every message logged as `time_ns status d1 d2`,
TIMING_SPEC 6.2 report lines and verdicts, `--loopback`, `--sweep`, `--follow`/`--host-port`, `--from-log`, `--list`),
`tests/fm1_timing_selftest.py`, and one `run` line in `tests/run_tests.sh`.

Commands run and decisive output:

```
$ .venv/bin/python tests/fm1_timing_selftest.py
TIMING SELFTEST PASS
$ python3 tests/fm1_timing_selftest.py            (system python3, as run_tests.sh runs it)
TIMING SELFTEST PASS
$ .venv/bin/python tools/fm1_timing.py --help
  --port PORT           FM-1 input port name (part of it); default: JIANT FM / FM-1 / Felucca
  --minutes MINUTES     length of each run, default 5 (TIMING_SPEC 6.2)
  --sweep [SWEEP]       run each tempo in turn (default 60,90,120,180,240), prompting before each
  --loopback            host noise floor on the IAC bus, then exit
$ .venv/bin/python tools/fm1_timing.py --from-log /tmp/fm1_timing_demo.log --bpm 120   (grid at 400 ppm fast)
  T2   note on/off jitter at host, p-p                 0.000 ms  PASS
  T4   constant latency, grid tick to host             2.000 ms  PASS
  T5   internal tempo error                          -400.000 ppm  FAIL
  T6   clock quarters without exactly 24 pulses             0 quarters  PASS
  T12a note-on while the same note is still open            0 count  PASS
TIMING VERDICT FAIL: T5, T8
$ .venv/bin/python tools/fm1_timing.py --list
inputs: ['IAC Driver Bus 1']
$ .venv/bin/python tools/fm1_timing.py --loopback --seconds 2
LOOPBACK host floor at 120 BPM over 96 of 96 pulses: p-p 0.112 ms, RMS 0.013 ms
```

`sh tools/check.sh`: started. Its build stage passed (`RAM .data+.bss 55744 B of 73728`). Its test stage did NOT
finish cleanly: P3-001 ran its own `tools/check.sh` in the same `build/` directory at the same time, and both runs
overwrote `build/check/tests.log`, so neither result can be attributed. My run was stopped. A clean rerun of
`sh tools/check.sh` (with no other check running) was then run on its own once pid 59720 had exited: `sh tools/check.sh` ends `CHECK PASS`. Its output: `RAM .data+.bss 55744 B of 73728`, `== timing tool self-test (P4-008)` then `TIMING SELFTEST PASS` in build/check/tests.log, `109 items, 0 errors` in build/check/backlog.log.

Notes for the reviewer:
- T1 is a device counter (TIMING_SPEC 6.1), not in the host log, so it is N/A and not counted in the exit code.
- T4 is the note offset from its own clock tick, because the FM-1 port alone cannot show the absolute grid tick. An
  offset shared by notes and clock reads as 0. The absolute figure needs the Mac-side reference of G2.
- T9 and T10 (follow) need the host's output on `--host-port`, for example an IAC bus fed by the DAW. T11 and T13
  are not in TIMING_SPEC 6.2 and are not implemented.
- T12a counts a note-on arriving while the same note is still open, as an approximation of "note-on before
  note-off at an equal tick" (the log has no tick numbers).
- `tests/run_tests.sh` is shared with P3-001 (uncommitted edits there). My change is one `run` line after the
  installer CLI step. That line did not appear in the concurrent run's log, so it is verified directly above, not by
  the full runner.

### 2026-11-10 review (sonnet)

Verdict: PASS (gate none).
- Selftest prints TIMING SELFTEST PASS; `--help` lists --loopback, --sweep, --port, --minutes.
- `sh tools/check.sh` ends CHECK PASS; RAM .data+.bss 55744 B of 73728 (17984 B headroom); backlog 109 items, 0 errors.
- Tool covers TIMING_SPEC 6.2: callback capture, full message log incl. F8/FA/FB/FC/F2, --loopback, --sweep, exit 0 only on all PASS.
- Notes (not blocking): T1 N/A, T4 is note-vs-own-clock offset, T11/T13 not implemented (not in 6.2). The working-tree run_tests.sh has uncommitted edits from P3-001 unrelated to this commit.
