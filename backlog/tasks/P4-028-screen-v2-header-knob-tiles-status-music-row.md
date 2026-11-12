---
id: P4-028
title: Screen v2 elements: header, knob tiles, status line, music row
phase: P4
type: task
status: todo
priority: 1
executor: sonnet
depends_on: [P4-026, P4-027]
gate: none
---
## Goal
Draw the live state exactly as SCREEN_SPEC_V2 sections 4.1-4.4, reading only the channel state and the algorithm descriptor.

## Read first
- docs/design/SCREEN_SPEC_V2.md sections 4.1-4.4, 6, 7; proposals/v12/v12.py (header, knobs, status, music, rec_cursor)
- firmware/src/ui_sq.c, sq_types.h (ChanSave, AlgoDesc after P4-027)

## Deliverable
- header (transport, tempo source INT/EXT/WAIT/lost, chips incl. muted/OFF/selected/`+`, short name, MIDI), knob tiles
  (meter, value lists with dots, one font per knob, turning frame, REC and CHAN bands, ROW 3 from the device profile),
  status line (preset/pending, ROW boxes, PAGE / FOLLOW off), music row (selected bars, fixed strip rows, playhead,
  past LENGTH, REC cursor).

## Acceptance criteria
- [ ] host renders of the scenes in SCREEN_SPEC_V2 section 7 that do not need P4-029 match the v12 mock renders pixel for pixel (or every difference is listed and approved)
- [ ] no per-algorithm branch in the screen code (grep in the review)
- [ ] `sh tools/check.sh` ends CHECK PASS

## Verification
```sh
sh tools/check.sh
```

## UX gate
docs/design/features/P4-026-screen-v2-main-screen.md (Gate A filled, Design locked 2026-11-12).

## Out of scope
- Buttons, keybed, message slot (P4-029); table rows (P4-030).

## Log
