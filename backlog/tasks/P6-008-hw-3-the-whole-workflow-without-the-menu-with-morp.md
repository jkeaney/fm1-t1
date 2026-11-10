---
id: P6-008
title: HW-3: the whole workflow without the menu, with morph, dice and mute on the TB-3
phase: P6
type: task
status: todo
priority: 2
executor: human
depends_on: [P6-004, P5-009]
gate: hardware-test
---
## Goal
VISION rule 1 on hardware: the target workflow, start to performance, without opening the menu.

## Read first
- docs/project/HARDWARE_TEST.md section HW-3
- docs/project/VISION.md rule 4

## Deliverable
- Owner's OK to flash is asked first (CLAUDE.md: always ask before flashing); install with `.venv/bin/python tools/fm1_install.py build/felucca.fwsc --port "JIANT" --yes`
- Recovery if it fails to boot: `.venv/bin/python tools/fm1_install.py restore/FM-1-official-V15.fwsc --port "JIANT"`, then power-cycle
- Owner runs HW-3 with a stopwatch: from power-on to a performed sequence; files FB-nnn per confusion or problem

## Acceptance criteria
- [ ] The workflow completes without opening the menu
- [ ] Every knob shows its role on screen at every step
- [ ] Morph, each dice layer, ALL, undo, mute/solo/fill/roll judged by ear
- [ ] Approved: owner line with the time taken

## Verification
```sh
.venv/bin/python tools/fm1_midi_listen.py --seconds 900 --out /tmp/hw3.log
```

## Out of scope
- Code changes

## Log
