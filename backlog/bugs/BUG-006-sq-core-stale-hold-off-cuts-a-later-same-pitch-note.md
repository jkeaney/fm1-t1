---
id: BUG-006
title: sq_core stale queued OFF of a HOLD note can cut a later same-pitch note short
phase: P4
type: bug
status: done
priority: 2
executor: sonnet
depends_on: [P4-004]
gate: none
severity: low   # latent: no stuck note, no double OFF; one note or repeat is cut early
found_in: 6b4862f (P4-004), read by the reviewer; the fuzz stays green
---
## Goal
Fix: a ratchet repeat that lands on a still-sounding HOLD pitch leaves the HOLD's queued OFF in the event queue, so that OFF later ends a same-pitch note early. Expected: the stale OFF is removed, as `sq_step_start` already does for a primary ON.

## Read first
- backlog/feedback/FB-010-*.md (the observation and suggested fix)
- firmware/src/sq_core.c: the `sq_emit` same-pitch guard, `sq_evq_purge_off`, `sq_step_start`
- tests/sq_core_test.c and tests/golden/sq_core_*.txt

## Deliverable
- The fix in `sq_emit` (call `sq_evq_purge_off(c, note)` in the guard, or tag OFFs with an instance id if that proves cleaner), plus a host test (a directed event-level case, and a fuzz assertion if one can be stated) that fails on the old code and passes on the new.

## Steps to reproduce
1. A HOLD step on pitch P with its OFF queued beyond the next step.
2. Inside that span a ratcheting hit of the same pitch P (via `sub_note`).
3. A same-pitch note is sounding when the HOLD's OFF falls due.

## Expected
The later note sounds for its full length.

## Actual
`sq_emit` (kind 1) matches by pitch and ends the later note early. Invariants still hold (ON == OFF per channel and note).

## Acceptance criteria
- [x] The new test fails on the old code and passes on the new.
- [x] All nine existing sq_core goldens are unchanged unless the log explains why.
- [x] The 100000-tick fuzz still passes under ASan/UBSan on at least 3 seeds.
- [x] `sh tools/check.sh` ends with `CHECK PASS`; the RAM line is pasted into the Log with headroom of at least 8192 B.

## Verification
```sh
sh tools/check.sh
```

## UX gate
n/a, scheduler internals; no control or screen change.

## Out of scope
- Ratchet velocity slope (currently 0) and no ratchet on slide, hold or tie steps: the ADR is silent and the locked prototypes' `sq_ratchet` defines the behaviour. P4-006 and P5-001 confirm the core matches the prototypes.

## Log

### 2026-11-12 executor (sonnet)
Test first: added g10_stale_hold_off + tests/golden/sq_core_10_stale_hold_off.txt (algo 5, HOLD 3 on P=60 with off queued at 142, step 2 on 57 with 1 repeat whose sub_note is 60, gate 100 %). On the unmodified code it FAILED: "no stale HOLD OFF at tick 142 cutting the repeat" (log: ON 60 @0, ON 57 @96, OFF 57 @127, OFF 60 @128, ON 60 @128, OFF 60 @142).
Fix (deviates from the suggested one, deliberately): a purge in the sq_emit guard FIRST attempted, but it also removes the incoming repeat's own queued OFF (sq_sched_pair queues the pair at schedule time, so the stale and own OFF are indistinguishable at emit time; golden showed the repeat ending only by the next pass's ON at 144, a hang risk). An instance id needs a SeqEvent/ChanRT layout change (ADR-0003 locked sizes 8 and 24). So: new `sq_evq_purge_off_after(c, note, on_t)` called in `sq_step_start`'s ratchet loop before each repeat's `sq_sched_pair`: it drops queued OFFs of that channel/pitch due strictly AFTER the repeat's ON (the repeat's ON will end the sounding instance anyway). OFFs due before the ON are kept (a HOLD that ends before the repeat still ends normally). Known residual: an earlier-scheduled pair of the same pitch whose ON is later than this repeat's ON loses its own OFF and is ended by the repeat's OFF instead (shorter, no hang); not reachable from one step's primary + repeats.
After fix: golden 10 = ... OFF 60 @128, ON 60 @128, OFF 60 @143 (repeat plays its full length), ON 60 @144.
Verification: the 9 existing goldens report ok (unchanged, git shows no golden modified); fuzz 100000 ticks under ASan/UBSan, seeds 0x1234567, 7, 99: SQ_CORE PASS each.
`sh tools/check.sh`: CHECK PASS. RAM line: "RAM .data+.bss 23828 B of 73728" (headroom 49900 B).

### 2026-11-12 review (sonnet)
Verdict: PASS (gate none). 
- (a) Scratch copy with HEAD~1 sq_core.c: the new g10 test FAILS (stale HOLD OFF at 142; golden differs at byte 157). On the fix: passes, under ASan/UBSan.
- (b) sq_evq_purge_off_after (sq_core.c:215, called :344) read case by case. HOLD OFF due before the repeat ON: kept (tick > on_t is false). HOLD OFF due after: dropped, and the repeat's ON ends the HOLD via the sq_emit guard, so no stuck note and no double OFF. Primary's own OFF: always < rr[0].off, kept. A previous repeat's OFF: len <= sp-1, so it is < the next ON, kept. PO_TIE and HOLD steps never ratchet (:334), so the purge cannot run there. Different channel: filtered by c. Slide-out never ratchets. Slide-in step whose repeat pitch equals the slid-from note: that OFF (S+D) is dropped only if D > the repeat offset (>= 3); the repeat ON then ends it a little later. No hang, no unpaired event.
- Fuzz 100000 ticks ASan/UBSan: 11 seeds (0x1234567, 7, 99, 1, 2, 3, 0xdeadbeef, 12345, 424242, 777, 31337) all PASS. Extra pressure variant (scratch only, not committed: 2 distinct pitches, 60 % ratchet, 40 % hold, 90 % hits, sub_note algos in play) on 8 more seeds: all PASS.
- (c) Residual (an earlier-scheduled same-pitch pair with ON after this repeat's ON): not reachable. Earlier pairs come from earlier steps (ONs < S) or from this step's primary and lower-numbered repeats (ONs ascending), so no. Also only sub_note (ARP, not in the firmware build: acid has sub_note = 0) makes a repeat differ from the primary, and with sub_note NULL the primary's same-pitch guard already purges. No bug filed.
- (d) git diff HEAD~1 tests/golden: only the new sq_core_10 added; the nine goldens are untouched.
- sh tools/check.sh: CHECK PASS. RAM .data+.bss 23828 B of 73728 (headroom 49900 B, above 8192).
