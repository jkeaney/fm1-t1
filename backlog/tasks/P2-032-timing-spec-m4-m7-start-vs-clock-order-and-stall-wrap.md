---
id: P2-032
title: TIMING_SPEC fixes from the P4-007 review (M4 vs M7 order, 89 s stall, M5 never-dropped wording)
phase: P2
type: task
status: review
priority: 3
executor: sonnet
depends_on: []
gate: none
---
## Goal
Make docs/design/TIMING_SPEC.md say what the code and MIDI require, so the P4-019/P4-022 hardware test checks the right order.

## Read first
- docs/design/TIMING_SPEC.md M4 (line ~218), M5, M7 (line ~240), section 6.3
- firmware/src/sq_core.c (tick 0 emission), firmware/src/usb.c midi_out_event_at / mo_due

## Deliverable
- M4: "clock, then Start/Continue/Stop" is the steady-state order; at tick 0 of a start the order is 0xFA then the first 0xF8 (M7, MIDI-correct: Start must precede the first clock). State the exception in M4 and the 6.3 ordering row.
- M5: "NOTE_OFFs and clock are never dropped" is false at a completely full ring (midi_out_free()==0 drops everything, usb.c midi_out_event_at); reword, or file a bug if the drop of an OFF must be impossible.
- M3/section 7: note that a due stamp is compared with a signed 32-bit difference, so a head packet older than 2^31 ticks (89 s at 24 ticks/us) looks "not due" until the counter wraps; a host stalled for more than 89 s with a non-empty ring wedges the head for up to another 89 s. State the accepted limit or ask for a fix (e.g. clear the ring when usb.config drops, already done on bus reset).

## Acceptance criteria
- [x] M4 and M7 no longer disagree about tick 0
- [x] M5 wording matches the full-ring behaviour
- [x] The 89 s limit is written down
- [ ] `sh tools/check.sh` ends with `CHECK PASS`

## Verification
```sh
grep -n "0xFA" docs/design/TIMING_SPEC.md
sh tools/check.sh
```

## UX gate
n/a, docs only.

## Out of scope
- Any firmware change

## Log

### 2026-11-12 executor (sonnet)
Docs only. Edited TIMING_SPEC.md: M3 (89 s signed-compare limit, accepted; ring cleared on bus reset only, usb.c mo_r = mo_w), M4 (tick-0 exception 0xFA then 0xF8), M5 (full-ring drop wording), M7 (cross-ref), 6.3 ordering row, section 7 rows.
`grep -n "0xFA" docs/design/TIMING_SPEC.md` shows the new M4/M7/6.3 lines. `python3 tools/backlog.py check`: 148 items, 0 errors.
`sh tools/check.sh` NOT run: orchestrator reserved it for P4-023 (docs-only change, no code touched).
