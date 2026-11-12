---
id: FB-013
title: Preview note can hang when ChanSave.pad[0] bit 7 (PREVIEW) changes while a note sounds
phase: P4
type: feedback
status: todo
priority: 2
executor: human
depends_on: []
gate: none
session: 2026-11-12 review of P4-005, host tests only
---
## Goal
Triage this observation into tasks or bugs, or drop it with a reason.

## Read first
- docs/project/VISION.md
- backlog/tasks/P4-005-channel-state-api-defaults-undo-and-ram-presets-wi.md (Log and review)
- firmware/src/sq_chan.c

## Deliverable
- Linked BUG-/P*- ids, or a drop reason, recorded in the log.

## Observation
sq_core.c sq_emit gates the trk_note_off on SQ_PRV_ON(c) at the time of the OFF (lines 89, 106, 307). If bit 7 of pad[0] is cleared between a note ON and its OFF, the OFF never reaches trk[c], the preview voice hangs, and a later ON hits the host mock 'double ON'. A preset load that changes pad[0], and the future preview toggle (PREVIEW.md, P2-025 cue), both do this. The fuzz avoids it by keeping pad[0] constant. Not a P4-005 defect, but real once a toggle exists. Fix options: send trk_note_off for a sounding preview note when the bit falls (a main-loop request flag the ISR consumes), or always send trk_note_off for notes whose ON went to trk (record the destination in ChanRT.snd). Needs a task before the preview toggle is wired (P4-013 / P6-003 / P4-018) and an sq_core test.

## Acceptance criteria
- [ ] Converted into backlog items (ids listed in the log) or dropped with a reason.

## Verification
```sh
python3 tools/backlog.py check
```

## UX gate
n/a, internal work (tooling, plumbing, docs, process); no producer-visible control, screen or sound change.

## Log
