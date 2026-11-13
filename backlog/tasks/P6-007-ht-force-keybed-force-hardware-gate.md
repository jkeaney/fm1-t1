---
id: P6-007
title: HT-FORCE: keybed force hardware gate
phase: P6
type: task
status: todo
priority: 27
executor: human
depends_on: [P6-006]
gate: hardware-test
---
## Goal
Decide on hardware which force gestures ship (SEQUENCER_UX 6.6 G1-G6) before FORCE defaults to NORMAL.

## Read first
- docs/design/SEQUENCER_UX.md §6.6
- docs/design/KEYBED_USE_CASES.md (per-use-case checklist)
- docs/project/HARDWARE_TEST.md section HT-FORCE

## Deliverable
- Owner's OK to flash is asked first (CLAUDE.md: always ask before flashing); install with `.venv/bin/python tools/fm1_install.py build/felucca.fwsc --port "JIANT" --yes`
- Recovery if it fails to boot: `.venv/bin/python tools/fm1_install.py restore/FM-1-official-V15.fwsc --port "JIANT"`, then power-cycle
- Flash the `FELUCCA_FORCE_DEBUG` test build, run G1-G6 and the use-case checklist by ear on the TB-3, then reflash the normal build

## Acceptance criteria
- [ ] G1-G6 recorded with numbers
- [ ] Decision recorded per gesture: ships with FORCE NORMAL, or fallback only; a follow-up task sets the FORCE default
- [ ] Approved: owner line

## Verification
```sh
.venv/bin/python tools/fm1_install.py --info --port "JIANT"
```

## UX gate
Gate B on real hardware: the owner walks the cited use cases on the FM-1 and TB-3; actual ease scores and live checklists are recorded in each use case (docs/project/UX_GATE.md).

## Out of scope
- Code changes

## Log
