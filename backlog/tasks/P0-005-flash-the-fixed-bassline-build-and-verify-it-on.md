---
id: P0-005
title: Flash the fixed-bassline build and verify it on the TB-3
phase: P0
type: task
status: todo
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
- [ ] Owner approved the flash in the session (recorded in the log)
- [ ] `fm1_install.py --info --port JIANT` reports FM-1_900 after the install
- [ ] Listener SUMMARY: note-ons only on ch2 in T1 mode; every note's pitch class is in {A,C,D,E,G}; at least one velocity 120 (an accent); CC123 count ≥ 1 after STOP
- [ ] Owner heard it on the TB-3 (via tools/fm1_bridge.py or a host box) and signed off in the log

## Verification
```sh
sh tools/check.sh                    # CHECK PASS
.venv/bin/python tools/fm1_install.py build/felucca.fwsc --port "JIANT" --yes   # only after the owner's OK
.venv/bin/python tools/fm1_midi_listen.py --seconds 120 --out docs/project/sessions/$(date +%F)-p0-005.log
```

## Out of scope
- Firmware changes (file a BUG instead).

## Log
