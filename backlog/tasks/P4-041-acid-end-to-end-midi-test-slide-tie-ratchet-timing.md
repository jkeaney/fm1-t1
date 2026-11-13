---
id: P4-041
title: ACID end-to-end MIDI test: slide, tie, ratchet timing
phase: P4
type: task
status: done
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
- [x] Test fails if a slide has a gap or a tie retriggers (shown once by temporarily breaking the scheduler locally, not committed)
- [x] The golden is deterministic across two runs
- [x] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)

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

### 2026-11-13 executor (sonnet)
tests/sq_acid_midi_test.c (new) drives the real events_block with sq_half_begin called per 128-sample half (as the audio ISR does), reads each packet's due stamp from the captured usb.c ring (544 TIMER4 ticks = 1 sample), and checks, in ticks (192 per quarter, DIV 1/16 = 48): plain note + GATE 50 % (on 0, off 24); slide overlap (step 2 on at 96, step 1 off at 102 = L/8 later: no gap); TIE (off at the next step start, 192); same-pitch slide (one on at 240, no on at 288, one off at 312); ratchet 3x (on 336, 360, 376, each off before the next); GATE 80 % = 38 ticks, GATE 100 % = 47; bar two = bar one + 768; every stamp a whole sample; no hung note; swing 60 % (odd steps +10). The pattern is written as StepEdit on ACID's CH1 so it does not depend on what ACID generates. Two runs are compared in the test and with tests/golden/sq_acid_midi.txt (46 events, new run_tests.sh step).
Deviations: (1) ACID never sets PO_HOLD or PO_REP, so HOLD is not testable on ACID output (still golden 4 of sq_core_test); ratchet comes from the RATCH edit. (2) The task text says the ratchet splits "evenly": the scheduler uses SP5 spacing with the default pace, 24/16/8 ticks (weights 3:2:1), so the test asserts that. Reviewer: decide whether even spacing was intended (that would be a core change, out of scope here).
Mutation check (copy of the tree in /tmp, not committed): D = 0 in sq_step_start -> FAIL "slide overlap: step 1 off at 102 after step 2 on (on 96.00, off 96.00)"; same-pitch tie disabled -> FAIL "slide+same pitch: no retrigger; the one off at 312 (got 288.00, on=0)".
```
sh tools/check.sh  -> CHECK PASS (new step "ACID on the MIDI wire (P4-041)" passes)
```

### 2026-11-13 review (sonnet)
Verdict: PASS (gate none). `sh tools/check.sh` -> CHECK PASS (the new step "ACID on the MIDI wire (P4-041)" passes); `RAM .data+.bss 29640 B of 73728` (headroom 44088 B, >= 8192 B). Only tests/ files and the task changed; no firmware edit.
- The test includes ui_test.c and drives the real `events_block` -> sq_block -> sq_core, reading each packet's due stamp from the usb.c ring; it runs the scenario twice, compares the lists, and run_tests.sh also compares two process runs and the golden (deterministic, rc non-zero on any failure).
- Mutations redone in a /tmp copy: `D = 0` in sq_core.c:346 fails "slide overlap: step 1 off at 102 after step 2 on (on 96.00, off 96.00)"; `tie = 0` at sq_core.c:383 fails "slide+same pitch: no retrigger; the one off at 312 (got 288.00, on=0)". Both claims true; base run rc 0.
- Ratchet spacing 24/16/8 is NOT a defect against the spec: ADR-0003 addendum row 7 (owner decision 115) stores one pace bit, "accelerates or decelerates", no "even"; sq_core.c:395-396 passes pace -1 or +1 only, never 0 (so `rat_offsets` pace 0 is unreachable); SEQUENCER_UX 1116 and acid/SPEC 135 say "SP5 spacing". The task text "splits the step evenly" was loose wording. No BUG filed. Owner question for later (not a blocker): a producer's RATCH edit on ACID always accelerates (3:2:1); if an even ratchet is wanted for the TB-3, that is a core design change (a third pace state).
- Minor, non-blocking: the `near()` tolerance is +-1.2 ticks (~138 samples), but the golden prints 0.01-tick values and is compared byte for byte, and a separate check requires every stamp to be a whole sample, so sample accuracy is covered.
- sound15 ENV SYNC benchmark: run 3 times alone, all ok (and ok inside check.sh); not added to the known-failures list.
