---
id: P4-004
title: sq_core.c scheduler with event-level golden tests
phase: P4
type: task
status: done
priority: 1
executor: sonnet
depends_on: [P4-002, P4-003]
gate: none
---
## Goal
The shared scheduler that turns PulseOut into timed MIDI: queue, `sq_emit`, pairing, two-pass dispatch, step start (swing, gate, ratchet, slide, hold), all-off. Tested against a mock algorithm table before any real algorithm exists.

## Read first
- docs/project/decisions/ADR-0003-sequencer-core.md §6, §7, §8, §14 (event-level golden list 1-9 and invariant fuzz)
- docs/design/TIMING_SPEC.md §3 M1, M4, M8, M9 (tick position, ordering, start/stop, intentional offsets)
- docs/sting-algorithms/ 6.3a (the 8-step slide worked example)
- firmware/src/t1_glue.c and torso_t1_pipeline.c (the code being replaced: keep the accumulator idea)

## Deliverable
- firmware/src/sq_core.c: `evq[96]`, `sq_emit`, `sq_sched_pair`, `sq_dispatch`, `sq_step_start`, `sq_tick`, `sq_all_off`, `sq_on_start`, `sq_on_stop`; the only file that calls `midi_out_event`, `trk_*`, `beat_samples`; not yet called from events_block
- tests/sq_core_test.c with a host `sq_emit` log hook and a mock ALGOS table: the nine golden cases of ADR-0003 §14 as checked-in text files tests/golden/sq_core_*.txt, plus the 100000-tick invariant fuzz under `-fsanitize=address,undefined`

## Acceptance criteria
- [x] All nine event goldens match
- [x] Fuzz: #NOTE_ON == #NOTE_OFF per (channel, note) after the final STOP; no double ON; no OFF without ON
- [x] sq_core.c is not yet referenced from events_block (`grep -n sq_block firmware/src/seq.c` shows only the forward declaration/stub)
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
- Channel state API (P4-005)
- Timing release (P4-007)
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log

### 2026-11-12 executor (sonnet)
Commands and decisive output:
```
sh tools/check.sh      -> RAM .data+.bss 23316 B of 73728 ... CHECK PASS
build line             -> image 168876 B; RAM .data+.bss 23316 B of 73728; pool 92500 B of 368640   (headroom 50412 B, >= 8192)
tests log              -> ok sq_core_1..9 goldens; "fuzz: 100000 ticks, 2575 MIDI notes, 3230 preview notes, 257 drops"; SQ_CORE PASS
extra seeds (argv[1])  -> 1 2 3 77 999 31337 424242 5150 all SQ_CORE PASS under ASan+UBSan
grep -n sq_block firmware/src/seq.c -> no output (sq_core.c is not in felucca.c and nothing references it)
```
Golden 1 (sting 6.3a) was checked against the hand-written 12-row table of the spec before being accepted: byte-identical.
The other eight goldens were generated with GOLDEN_UPDATE=1 and reviewed by eye (swing odd steps 15 ticks late, ratchet spacings, hold OFF at 382, etc.).

Decisions for the reviewer:
- sq_core.c is NOT included in felucca.c: it needs the ALGOS table (no sq_algos.c yet). The unity build's RAM line is therefore unchanged.
- sq_block uses an internal `sq_playing` flag, not `song.playing`; P4-012 wires the transport.
- Swing parity uses the free-running step count (pass*len + p), as ADR 7.3 says t is free-running.
- Ratchet velocity slope is 0 (unspecified in the ADR); ratchet is skipped on slide, hold and tie steps.
- sq_emit also does an emit-time same-pitch guard (OFF before ON if the note still sounds), found by the fuzz: ratchet repeats via sub_note can collide with a HOLD note.
- Not done (other tasks): PO_TIE (P4-006), HARMONY (AF_CHORD gets an inert chord), clock out and stamping (P4-007/009), sq_preset_load staging (P4-005; only the ISR half sq_apply_bar_boundary is here).
- tests/run_tests.sh gained one step running sq_core_test (sanitizers when available).

### 2026-11-12 review (sonnet)
Verdict: PASS (gate none).
- `sh tools/check.sh` -> CHECK PASS. `./build.sh` -> `RAM .data+.bss 23316 B of 73728; pool 92500 B of 368640` (headroom 50412 B, >= 8192).
- Goldens re-derived by hand from ADR-0003 14 / 7.3 and TIMING_SPEC M4/M8/M9, not from code output: (1) byte-identical to the 12 rows of docs/design/algorithms/sting.md worked example (0/48/54/107/192/240/246/288/294/299/336/362); (2) swing 66 gives o=15, odd ONs at 63/159/255/351, OFFs follow own start; (3) ratchet spacings 12/8/4, 4/8/12, 9/7/5/3, all >= 3 ticks inside the cell, a 4-trigger case; (4) hold OFF at 0+8*48-2=382 regardless of GATE, next ordinary step capped at S1-1=431; (5) muted ONs silent, a sounding note's OFF still sent, playhead equals unmuted run; (6) OFFs per channel byte then CC123 2,5,8 at the stop tick, queue empty; (7) 3 steps lost whole, no unmatched ON; (8) OFF 38 on old channel 2, next ON on 6; (9) staged preset lands at 768, playhead not jumped (step 1 -> note 61).
- Fuzz re-run under ASan+UBSan with seeds 11, 4242, 987654, 0xdeadbeef, 7: all SQ_CORE PASS (100000 ticks each, 0 violations, nothing sounding after STOP).
- sq_emit fix: OFF-before-ON of a still-sounding same pitch is correct: it drops the slot via sq_snd_drop, is skipped if the original ON never went out (0x10), refuses the new ON rather than hang when the ring is full, so no double OFF and no stuck note. One gap, see FB-010 (stale queued HOLD OFF can cut a later same-pitch note early; not an invariant breach).
- Reserve rule: sq_emit checks midi_out_free() < SQ_RESERVE (= MO_RESERVE) before an ON, OFF and CC123 only need one free slot; usb.c also enforces it. sq_core.c is the only sq_* file calling midi_out_event/trk_*/beat_samples (grep clean). No reference to sq_core or sq_block in seq.c or felucca.c (stronger than the "forward declaration only" in the criterion).
- Executor choices: ratchet velocity slope 0, no ratchet on slide/hold/tie steps: fine for now, ADR silent; listed in FB-010 for a design look. Swing parity from free-running pass*len+p matches ADR "t free-running": fine. Separate sq_playing flag: fine until P4-012 wires song.playing. HARMONY inert and PO_TIE deferred: correctly out of scope.
