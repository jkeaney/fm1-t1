---
id: P0-005
title: Flash the fixed-bassline build and verify it on the TB-3
phase: P0
type: task
status: done
priority: 1
executor: human
depends_on: [P0-003, P0-004]
gate: hardware-test
---
## Goal
Get the scale/range/accent fixes onto the device and confirm the reference sound path: FM-1 → TB-3 playing an in-key A-minor-pentatonic bassline.

## Read first
- docs/project/HARDWARE_TEST.md
- CLAUDE.md (Flashing: always ask the owner; --port "JIANT")

## Deliverable
- build/felucca.fwsc installed on the FM-1; one listener log saved to docs/project/sessions/<date>-p0-005.log; one FB- item per problem found

## Acceptance criteria
- [x] Owner approved the flash in the session (recorded in the log)
- [x] `fm1_install.py --info --port JIANT` reports FM-1_900 after the install
- [x] Listener SUMMARY: note-ons only on ch2 in T1 mode; every note's pitch class is in {A,C,D,E,G}; at least one velocity 120 (an accent); CC123 count ≥ 1 after STOP
- [x] Owner heard it on the TB-3 (via tools/fm1_bridge.py or a host box) and signed off in the log

## Verification
```sh
sh tools/check.sh                    # CHECK PASS
.venv/bin/python tools/fm1_install.py build/felucca.fwsc --port "JIANT" --yes   # only after the owner's OK
.venv/bin/python tools/fm1_midi_listen.py --seconds 120 --out docs/project/sessions/$(date +%F)-p0-005.log
```

## Out of scope
- Firmware changes (file a BUG instead).

## Log

### 2026-11-09 orchestrator
Owner decision: skip the flash; dropped. Hardware testing deferred to the designed build.

### 2026-11-09 orchestrator
Owner revised: will flash in the morning. Restored to todo (human task; do not dispatch; wait for owner).

### 2026-11-11 orchestrator
Session with owner: FM-1 `FM-1_900` already installed (--info reported it), so no reflash. Path: FM-1 (JIANT FM) -> tools/fm1_bridge.py -> TB-3 (receive ch 2, TB-3 sequencer stopped). Logs: docs/project/sessions/2026-11-11-p0-005-run1.log (336 note-ons, ch2 only, pitch classes all in {A,C,D,E,G}, 68 accents at vel 120) and 2026-11-11-p0-005.log (493 note-ons, ch2 only, 146 at vel 120, CC123 sent on STOP). In the second log 26 notes after 137 s are outside the pentatonic set: owner confirms they were changing keys and pitches by hand then (expected, not a bug).
- Owner heard the bassline on the TB-3 and the accent squelching. Approved: owner 2026-11-11.
- H0 (accent threshold by ear) NOT answered: the firmware sends only 80 and 120. Whether the TB-3 accents at exactly 100 is still open (see decision 95).
