---
id: FB-019
title: P4-017 REC keys and step audition write the note queue; nothing in the ISR plays it yet
phase: P4
type: feedback
status: todo
priority: 1
executor: human
depends_on: [P4-017]
gate: none
session: 2026-11-13, host tests only (no device), P4-017 executor
---
## Goal
Triage the missing ISR end of the REC note queue into a task (before HW-1), or drop it with a reason.

## Read first
- docs/project/VISION.md
- backlog/tasks/P4-017-rec-page.md (Log), firmware/src/ui_sq.c (the REC section: `uisq_aud_push`, `ui_sq_aud_next`)
- firmware/src/sq_core.c `sq_live_on` / `sq_live_off` (what the ISR would call), seq.c `events_block`, backlog/tasks/P4-032 (its "stopped step sounds" deliverable needs the same road)

## Deliverable
- A task (sonnet or higher; it touches ISR code) that drains the queue in `events_block`.

## Observation
P4-017 was scoped to the UI ("Out of scope: ISR code"). Its REC page writes every key press as a step, and queues the key's note-on / note-off (and the K1 step audition) in a 16-slot single-producer ring in ui_sq.c: `uisq_aud_q`, written by the main loop, to be read by the ISR through `ui_sq_aud_next(&on, &note, &vel)`. Nothing calls it yet, so on a device REC writes steps silently: the keys do not sound the preview or MIDI out. The glue sets `song.grid = 2`, so keyboard_block stays silent too.
What the consumer must do (it is the whole task): each block in `events_block`, pop events and call `sq_live_on(note, vel)` / `sq_live_off(note)` (sq_core.c; they play on `sq_sel`, which is the selected channel, and remember the channel per note so an off follows its on after SELECT). The UI guarantees every on has an off (it owes the off until the ring takes it, also after REC is left), and never queues an off without an on. A forward declaration of `ui_sq_aud_next` is needed in seq.c (or the ring moves to a file included before seq.c, as `transport_req` is). The same queue serves P4-032's "a step turned on while stopped sounds once".
Also check on the unit: the written pitch is stored as semitones above 36 + KEY with OCTAVE cancelled, so the played note and the later generated note agree (host test: ui_sq_rec_test); the ring is 16 slots, 15 usable.

## Acceptance criteria
- [ ] Converted into a backlog item (id listed in the log) or dropped with a reason.

## Verification
```sh
python3 tools/backlog.py check
```

## UX gate
n/a, triage item.

## Log
- 2026-11-13: filed by the P4-017 executor. Not flashing; the REC page cannot be called done on hardware (HW-1) until this lands.
