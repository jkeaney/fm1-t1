---
id: P4-021
title: HW-1: first flash of the new core, target workflow up to PLAY on the TB-3
phase: P4
type: task
status: todo
priority: 1
executor: human
depends_on: [P4-020]
gate: hardware-test
---
## Goal
First time the new sequencer runs on the device: prove the reference workflow up to performing steps works on the TB-3.

## Read first
- docs/project/HARDWARE_TEST.md section HW-1
- docs/design/scenarios/S01

## Deliverable
- Owner's OK to flash is asked first (CLAUDE.md: always ask before flashing); install with `.venv/bin/python tools/fm1_install.py build/felucca.fwsc --port "JIANT" --yes`
- Recovery if it fails to boot: `.venv/bin/python tools/fm1_install.py restore/FM-1-official-V15.fwsc --port "JIANT"`, then power-cycle
- Owner runs HARDWARE_TEST HW-1 and files FB-nnn for every problem

## Acceptance criteria
- [ ] Boots into PERF with the first-run prompt
- [ ] CH1 set to MIDI 2 in two gestures; ACID plays on the TB-3; step taps change what the TB-3 plays
- [ ] STOP leaves no stuck note
- [ ] Approved: owner line with firmware SHA

## Verification
```sh
.venv/bin/python tools/fm1_install.py --info --port "JIANT"
.venv/bin/python tools/fm1_midi_listen.py --seconds 300 --out /tmp/hw1.log
```

## UX gate
Gate B on real hardware: the owner walks the cited use cases on the FM-1 and TB-3; actual ease scores and live checklists are recorded in each use case (docs/project/UX_GATE.md).

## Out of scope
- Code changes

## Log
