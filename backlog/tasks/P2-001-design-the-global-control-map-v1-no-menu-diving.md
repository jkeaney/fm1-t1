---
id: P2-001
title: Design the global control map v1 (no menu diving)
phase: P2
type: task
status: todo
priority: 1
executor: sonnet
depends_on: [P1-001, P1-010, P1-012]
gate: none
---
## Goal
OWNER-APPROVED INPUT: P1-012 already designed and the owner approved the control scheme (docs/design/UX_REVIEW.md, control_map.tsv, 2026-11-10). This task CONSOLIDATES it into the firmware-facing CONTROL_MAP.md; it must not redesign. Any conflict or gap found goes to docs/project/OWNER_QUESTIONS.md as a question and the task stops at `blocked` with the reason in the log.

Assign every button, knob, encoder and key to the vision: channel select, algorithm select, step entry, morph, dice, transport, mute. Every musical function one gesture away, with on-screen labels.

## Read first
- docs/design/UX_REVIEW.md and docs/design/control_map.tsv (P1-012: the owner-directed hardware/UX rethink; build on it, do not redo it)
- docs/project/OWNER_QUESTIONS.md (owner decisions: scale, GATE/DIV/SWING are global; ACID has slide)
- docs/design/CONTROL_INVENTORY.md
- docs/design/ALGORITHM_SET.md
- docs/project/VISION.md (reference workflow)
- docs/controls.jpg

## Deliverable
- docs/design/CONTROL_MAP.md: (1) modes, if any, and how you always know which one you're in; (2) a table Control | Tap | Hold | Combo | Screen label; (3) keys: steps vs channels vs dice; (4) the reference workflow as a gesture-by-gesture walkthrough; (5) what stays in the MENU (settings only); (6) the conflicts resolved from the T-1 guide

## Acceptance criteria
- [ ] The reference workflow walkthrough reaches 'performing' without any MENU step
- [ ] Every control from CONTROL_INVENTORY has a row (or 'unused, reserved')
- [ ] No gesture needs more than one held modifier
- [ ] CONTROL_MAP.md agrees with control_map.tsv row for row (a script check is logged); no behaviour differs from the approved UX_REVIEW.md
- [ ] Control map is a machine-readable table (docs/design/control_map.tsv or similar) that firmware can load as a const table; a 'Flex points' section lists what is re-mappable by editing a row
- [ ] Every row meets VISION rule 1: reachable without menu diving, with its on-screen label

## Verification
```sh
for b in FX SCL ENV LFO EDIT GLO HOME SAVE ARP SEQ PLAY REC OCTDN OCTUP; do grep -q "$b" docs/design/CONTROL_MAP.md || echo missing $b; done   # prints nothing
```

## Out of scope
- Screen pixel layout (that's P2-003).

## Log
