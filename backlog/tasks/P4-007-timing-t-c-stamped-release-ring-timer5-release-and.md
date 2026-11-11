---
id: P4-007
title: Timing T-C: stamped release ring, TIMER5 release and the tempo constant
phase: P4
type: task
status: todo
priority: 1
executor: sonnet
depends_on: [P4-004]
gate: none
---
## Goal
Tight MIDI timing (TIMING_SPEC M1-M7, M9, M10): every outgoing packet carries a due time and is released by the 100 us TIMER5 at that time, not in the audio-interrupt burst.

## Read first
- docs/design/TIMING_SPEC.md §1, §2 (targets), §3 M1-M10, §6.3 (host tests), §7 (fallbacks)
- docs/project/decisions/ADR-0003-sequencer-core.md §19
- firmware/src/usb.c, audio.c, hal timer headers

## Deliverable
- usb.c: `txr[128]` {pkt, due}, `midi_out_event_at`, `midi_tx_release` in the TIMER5 branch, `LEAD` 128 samples; a compile flag selecting the M3 fallback (release between audio interrupts)
- sq_core.c: exact tick position inside the block (M1), stamps (M2), ordering (M4), clock-out pulses at exact positions when master (M7), `SEQ_FS_Q4` tempo constant (M10)
- tests/timing_test.c: the TIMING_SPEC 6.3 rows tick position, stamp, ordering, clock out count, release, tempo constant

## Acceptance criteria
- [ ] timing_test passes every listed row
- [ ] The fallback flag builds and passes the same tests
- [ ] `.ram_text` no-call check in build still passes
- [ ] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [ ] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## UX gate
n/a, internal work (tooling, plumbing, docs, process); no producer-visible control, screen or sound change.

## Out of scope
- Clock follow / PLL (P4-009)
- The test image (P4-019)
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
