---
id: P5-009
title: HW-2: all six algorithms and HARMONY on the TB-3
phase: P5
type: task
status: todo
priority: 2
executor: human
depends_on: [P5-006, P5-007, P5-008, P4-021]
gate: hardware-test
---
## Goal
Hear every algorithm and the chord stage on the target synth before the performance layer is built on them.

## Read first
- docs/project/HARDWARE_TEST.md section HW-2

## Deliverable
- Owner's OK to flash is asked first (CLAUDE.md: always ask before flashing); install with `.venv/bin/python tools/fm1_install.py build/felucca.fwsc --port "JIANT" --yes`
- Recovery if it fails to boot: `.venv/bin/python tools/fm1_install.py restore/FM-1-official-V15.fwsc --port "JIANT"`, then power-cycle
- Owner runs HW-2; each algorithm judged by ear: steps, notes, slides, accents; HARMONY 1-4-5-1 with ROLE ROOT and FULL; files FB-nnn per problem

## Acceptance criteria
- [ ] Each of ACID, EUCLID, SNAKE, STAGE, ARP, PATTERNS recorded pass/fail with notes
- [ ] HARMONY default changes nothing audible; 1-4-5-1 is heard
- [ ] Approved: owner line

## Verification
```sh
.venv/bin/python tools/fm1_midi_listen.py --seconds 600 --out /tmp/hw2.log
```

## UX gate
Gate B on real hardware: the owner walks the cited use cases on the FM-1 and TB-3; actual ease scores and live checklists are recorded in each use case (docs/project/UX_GATE.md).

## Out of scope
- Code changes

## Log
