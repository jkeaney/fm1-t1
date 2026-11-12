---
id: BUG-009
title: MIDI out ring is not cleared when usb.config drops without a bus reset (stale ONs, dropped OFFs)
phase: P2
type: bug
status: todo
priority: 4
executor: sonnet
depends_on: []
gate: none
severity: low   # latent: needs a host that sends SET_CONFIGURATION 0 then 1 with no bus reset
found_in: dfa7e9b
---
## Goal
Fix: `usb.c` SET_CONFIGURATION with value 0 (case 0x0009) so the out ring is emptied (`mo_r = mo_w`), as the bus-reset path does (usb.c ~1366).

## Read first
- firmware/src/usb.c: midi_out_event_at (refuses events while `!usb.config`), SET_CONFIGURATION handler, bus reset handler.
- docs/design/TIMING_SPEC.md M3 accepted limit (P2-032 review).

## Deliverable
- On `usb.config` going 1 -> 0 by SET_CONFIGURATION 0, clear the ring. Host test if the ring is host-testable, else a code-review note.

## Steps to reproduce
1. Queue NOTE_ONs with future stamps, host sends SET_CONFIGURATION 0 (driver unload), OFFs are refused (`!usb.config`), host sends SET_CONFIGURATION 1.

## Expected
No stale packets reach the new session.

## Actual
The queued ONs are released after reconfigure without their OFFs (stuck notes); also the 89 s signed-compare wedge (TIMING_SPEC M3) applies to a stale head.

## Acceptance criteria
- [ ] Ring empty after SET_CONFIGURATION 0.
- [ ] `tools/check.sh` passes.

## Verification
```sh
sh tools/check.sh
```

## UX gate
n/a

## Log
Filed by the P2-032 reviewer.
