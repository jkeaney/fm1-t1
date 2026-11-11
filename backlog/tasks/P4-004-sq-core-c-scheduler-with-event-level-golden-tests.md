---
id: P4-004
title: sq_core.c scheduler with event-level golden tests
phase: P4
type: task
status: todo
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
- [ ] All nine event goldens match
- [ ] Fuzz: #NOTE_ON == #NOTE_OFF per (channel, note) after the final STOP; no double ON; no OFF without ON
- [ ] sq_core.c is not yet referenced from events_block (`grep -n sq_block firmware/src/seq.c` shows only the forward declaration/stub)
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
- Channel state API (P4-005)
- Timing release (P4-007)
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
