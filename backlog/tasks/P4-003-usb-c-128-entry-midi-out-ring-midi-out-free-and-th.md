---
id: P4-003
title: usb.c 128-entry MIDI out ring, midi_out_free and the reserve rule
phase: P4
type: task
status: done
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
- [x] usb_ring_test passes
- [x] uac_test and usb_desc_test still pass
- [x] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [x] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## UX gate
n/a, internal work (tooling, plumbing, docs, process); no producer-visible control, screen or sound change.

## Out of scope
- Timestamps and TIMER5 release (P4-007)
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log

### 2026-11-12 executor (haiku)

Change (firmware/src/usb.c): the out ring is its own `midi_out_q[MQO]`, `MQO = 128` (the in ring stays `MQ = 64`, +256 B).
`midi_out_free()` returns `MQO - (mo_w - mo_r)`. `midi_out_event` (audio ISR) with a host configured refuses a packet
when the ring is full, or when it is a NOTE_ON (status 0x9n, velocity != 0) and fewer than `MO_RESERVE = 8` slots are
free; each refusal increments `midi_out_drops`. NOTE_OFF, velocity-0 NOTE_ON, CC and clock (0xF8) queue while any slot
is free. With `usb.config = 0` nothing is queued or counted (as before). `ep1_tx` reads `midi_out_q[mo_r % MQO]`.
Test-side: `tests/t1_link_test.c` and `tests/midi_control_test.c` read the out ring with `% MQO` (were `% MQ`).
New: `tests/usb_ring_test.c` (includes usb.c as usb_desc_test does; 14 checks: size, no host, reserve boundary at 8 and
7 free, velocity-0 ON kept, offs/clock kept to full, drop counts, order, a 300-ON flood with 121 accepted, wrap over
1000 rounds). `tests/run_tests.sh`: one new step after the usb_desc loop.

Verification:
```
$ sh tools/check.sh
== build (log: build/check/build.log)
== ram headroom
RAM .data+.bss 23316 B of 73728
== tests (log: build/check/tests.log)
== backlog (log: build/check/backlog.log)
CHECK PASS
$ ./build.sh | grep 'RAM'
  ok    image 168860 B; RAM .data+.bss 23316 B of 73728; pool 92500 B of 368640
$ cc -O1 -Wall -Wno-unused-function -o build/host/usb_ring_test tests/usb_ring_test.c && ./build/host/usb_ring_test
...
1000 rounds of ON / OFF / clock across the wrap (mo_w > 128): in order, no drops ok
usb_ring_test passed (0 failures)
$ (ASan + UBSan build of usb_ring_test: clean, no reports)
tests/build log: usb_ring_test passed (0 failures); uac: all ok (x2 and x2 variants); usb_desc: all ok; ALL HOST TESTS PASSED
$ python3 tools/backlog.py check
134 items, 0 errors
```

RAM headroom: 73728 - 23316 = 50412 B (>= 8192 B, so no FB under the E9 trigger).

Notes for the reviewer:
- `midi_out_drops` counts both reserve drops (NOTE_ON) and full-ring refusals (any packet). The deliverable asks for "the drop counter counts"; a split into ON / full counters was not made.
- The NOTE_ON test is `status 0x9n and velocity != 0`. Velocity-0 NOTE_ONs are never dropped, matching ADR-0003 8.1 (never sent as NOTE_ON).
- Timestamps and TIMER5 release (P4-007) not touched. The ADR/doc edits in the working tree (docs/design, ADR-0003, P2-029/P2-030) belong to another agent and are not in this commit.

### 2026-11-12 review (sonnet)

Verdict: PASS (gate none -> done).
- Re-ran `sh tools/check.sh`: CHECK PASS (usb_ring_test, uac, usb_desc all ok in build/check/tests.log). `./build.sh`: `image 168860 B; RAM .data+.bss 23316 B of 73728; pool 92500 B of 368640`. Headroom 50412 B >= 8192 B.
- Reserve rule (usb.c:211-214): refuses when free == 0, or NOTE_ON with velocity != 0 and free < MO_RESERVE (8). So 8 free still queues an ON, 7 does not; OFF, vel-0 ON, CC123 and 0xF8 queue until the ring is full. midi_out_is_on checks status 0x9n and `pkt >> 24 != 0`, so velocity-0 is a note-off.
- Index math: all producers/consumers of midi_out_q use `% MQO` (usb.c producer, ep1_tx, t1_link_test.c:21, midi_control_test.c:276); grep finds no stray `% MQ` on the out ring. scale_test uses literal slots 0/1 only (fine). Free count `MQO - (mo_w - mo_r)` is unsigned-wrap safe; 128 is a power of two.
- ISR safety: midi_out_event is the single producer (audio ISR; seq.c keyboard_block and t1_glue flush both run there); ep1_tx only advances mo_r, so a stale free count can only under-estimate (safe). midi_out_drops is written only by the producer. Minor, non-blocking: t1_glue.c:88 note-off on stop relies on being called from the ISR (transport_req), as before.
- midi_out_drops semantics: documented at usb.c:177 and in the executor Log (counts reserve drops and full-ring refusals together, none when no host). Not yet exposed anywhere; P4-007 or a later diagnostics task may split it.
- Scope: only usb.c, two test index fixes, usb_ring_test.c, run_tests.sh. Nothing else touched.
