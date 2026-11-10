---
id: P4-003
title: usb.c 128-entry MIDI out ring, midi_out_free and the reserve rule
phase: P4
type: task
status: todo
priority: 2
executor: haiku
depends_on: [P3-012]
gate: none
---
## Goal
ADR-0003 §8.3: a separate 128-entry out ring so note-offs and clock are never dropped when the host is slow.

## Read first
- docs/project/decisions/ADR-0003-sequencer-core.md §8.3, §8.4
- docs/design/TIMING_SPEC.md §3 M5 (ring sizing and priority)
- firmware/src/usb.c (`midi_out_event`, the 64-entry ring)

## Deliverable
- usb.c: `MQO` 128 out ring, `midi_out_free()` returning free entries; the reserve rule (when fewer than the reserve remain, NOTE_ON is dropped and counted, NOTE_OFF/CC123/clock still queue)
- tests/usb_ring_test.c: fill the ring; assert ONs are dropped first, OFFs and 0xF8 are kept, order is preserved, the drop counter counts; run_tests.sh step

## Acceptance criteria
- [ ] usb_ring_test passes
- [ ] uac_test and usb_desc_test still pass
- [ ] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [ ] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## Out of scope
- Timestamps and TIMER5 release (P4-007)
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
