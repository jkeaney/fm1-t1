---
id: P2-001
title: Design the global control map v1 (no menu diving)
phase: P2
type: task
status: todo
priority: 1
executor: haiku
depends_on: [P1-001, P1-010]
gate: human-review
---
## Goal
Assign every button, knob, encoder and key to the vision: channel select, algorithm select, step entry, morph, dice, transport, mute. Every musical function one gesture away, with on-screen labels.

## Read first
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
- [ ] The owner approved it (log line)

## Verification
```sh
for b in FX SCL ENV LFO EDIT GLO HOME SAVE ARP SEQ PLAY REC OCTDN OCTUP; do grep -q "$b" docs/design/CONTROL_MAP.md || echo missing $b; done   # prints nothing
```

## Out of scope
- Screen pixel layout (that's P2-003).

## Log
