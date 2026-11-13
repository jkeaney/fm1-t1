---
id: P4-041
title: ACID end-to-end MIDI test: slide, tie, ratchet timing
phase: P4
type: task
status: todo
priority: 2
executor: sonnet
depends_on: [P4-034]
gate: none
---
## Goal
Prove on the host what ACID puts on the MIDI wire: slide overlap, tie and ratchet note-on/off timing, through the real `events_block`, so the TB-3 behaviour is checked before more algorithms are added.

## Read first
- /tmp/readiness_review.md section C (the end-to-end gap)
- tests/sq_link_test.c (counts only today)
- firmware/src/sq_core.c (scheduler: slide, tie, ratchet, hold)
- firmware/src/seq.c `events_block`
- tests/host_sq_hooks.h
- docs/design/algorithms/acid/SPEC.md and golden.txt

## Deliverable
- tests/sq_acid_midi_test.c: drives `events_block` over several bars with fixed seeds and a fixed pattern and checks sample-accurate note-on/off times: a slide overlaps (next note-on before previous note-off), a tie holds across the boundary with no retrigger, a ratchet splits the step evenly, gate length and swing positions
- A checked-in golden text of the MIDI event list for one reference pattern; run_tests.sh step

## Acceptance criteria
- [ ] Test fails if a slide has a gap or a tie retriggers (shown once by temporarily breaking the scheduler locally, not committed)
- [ ] The golden is deterministic across two runs
- [ ] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)

## Verification
```sh
sh tests/run_tests.sh   # after ./build.sh; the new step passes
```

## UX gate
n/a, internal work (tooling, plumbing, docs, process); no producer-visible control, screen or sound change.

## Out of scope
- Firmware changes (if the test finds a defect, file a BUG and stop)
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
- 2026-11-13: created from the POC readiness review (/tmp/readiness_review.md, copy the findings you need into the Log; /tmp may be gone).
- haiku-able, sonnet chosen.
