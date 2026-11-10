---
id: P8-004
title: Release flash and acceptance session
phase: P8
type: task
status: todo
priority: 1
executor: human
depends_on: [P8-003, P7-004, P6-008, P4-022]
gate: hardware-test
---
## Goal
The finish line: the release image flashed on the owner's FM-1 and accepted on the TB-3.

## Read first
- docs/project/HARDWARE_TEST.md 'release acceptance'
- release/v1/INSTALL.md

## Deliverable
- Owner's OK to flash is asked first (CLAUDE.md: always ask before flashing); install with `.venv/bin/python tools/fm1_install.py build/felucca.fwsc --port "JIANT" --yes`
- Recovery if it fails to boot: `.venv/bin/python tools/fm1_install.py restore/FM-1-official-V15.fwsc --port "JIANT"`, then power-cycle
- Owner flashes release/v1/felucca.fwsc and runs the release acceptance checklist; then decides on push and tag

## Acceptance criteria
- [ ] Every release acceptance item passes
- [ ] Target workflow (VISION rule 4) performed on the TB-3 without the menu
- [ ] Approved: owner line with firmware SHA; v1 is done

## Verification
```sh
.venv/bin/python tools/fm1_install.py release/v1/felucca.fwsc --port "JIANT" --yes
.venv/bin/python tools/fm1_install.py --info --port "JIANT"
```

## Out of scope
- Code changes

## Log
