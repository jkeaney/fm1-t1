---
id: P3-013
title: HW-0: flash the cleaned build and confirm the T-1 still plays the TB-3
phase: P3
type: task
status: todo
priority: 1
executor: human
depends_on: [P3-012]
gate: hardware-test
---
## Goal
Catch any boot or audio break from the removals before new core work starts (a build that crashes at boot needs mask-ROM recovery).

## Read first
- docs/project/HARDWARE_TEST.md (smoke checklist)
- CLAUDE.md (Flashing, restoring and testing)

## Deliverable
- Owner's OK to flash is asked first (CLAUDE.md: always ask before flashing); install with `.venv/bin/python tools/fm1_install.py build/felucca.fwsc --port "JIANT" --yes`
- Recovery if it fails to boot: `.venv/bin/python tools/fm1_install.py restore/FM-1-official-V15.fwsc --port "JIANT"`, then power-cycle
- Owner runs the smoke checklist of HARDWARE_TEST.md: boot, identity, PLAY, notes on TB-3 channel 2, STOP sends CC123, knobs, mute
- Owner listens to the preview synth (ANALOG) on the speaker

## Acceptance criteria
- [ ] Boots to the T-1 screen without a reboot or crash
- [ ] `fm1_install.py --info --port "JIANT"` reports `FM-1_9xx`
- [ ] TB-3 plays on channel 2; STOP silences it
- [ ] Approved: owner line with date and firmware SHA in the Log

## Verification
```sh
.venv/bin/python tools/fm1_install.py --info --port "JIANT"
.venv/bin/python tools/fm1_midi_listen.py --seconds 120 --out /tmp/hw0.log
```

## Out of scope
- Any code change; problems become FB-nnn files

## Log
