---
id: P7-004
title: HW-4: power-cycle keeps presets and settings
phase: P7
type: task
status: todo
priority: 2
executor: human
depends_on: [P7-003]
gate: hardware-test
---
## Goal
Prove storage on the device.

## Read first
- docs/project/HARDWARE_TEST.md section HW-4

## Deliverable
- Owner's OK to flash is asked first (CLAUDE.md: always ask before flashing); install with `.venv/bin/python tools/fm1_install.py build/felucca.fwsc --port "JIANT" --yes`
- Recovery if it fails to boot: `.venv/bin/python tools/fm1_install.py restore/FM-1-official-V15.fwsc --port "JIANT"`, then power-cycle
- Owner saves 3 presets, changes globals, power-cycles twice, reloads; tries an empty slot; flashes the same build again over the top and checks presets survive an update

## Acceptance criteria
- [ ] Presets and settings survive power cycles and a reflash
- [ ] Empty slot shows `P09 EMPTY: NOTHING LOADED`
- [ ] Approved: owner line

## Verification
```sh
.venv/bin/python tools/fm1_install.py --info --port "JIANT"
```

## Out of scope
- Code changes

## Log
