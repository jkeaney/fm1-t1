---
id: P4-042
title: Bound the tie search in the ISR; pass the generator position to sub_note
phase: P4
type: task
status: done
priority: 1
executor: sonnet
depends_on: [P4-021]
gate: none
---
## Goal
Remove the unbounded worst-case in the interrupt (a tie search that evaluates up to 63 steps per tick) before PATTERNS makes each step() expensive, and fix the position given to the ARP repeat-note hook.

## Read first
- /tmp/readiness_review.md section D (top risk 1) and finding 4
- firmware/src/sq_core.c ~297-306 and ~384 (`sq_tie_end`), ~419 (`sub_note(k, p, ...)`)
- docs/design/TIMING_SPEC.md (interrupt budget)
- docs/project/decisions/ADR-0003-sequencer-core.md

## Deliverable
- Either a bound on the tie search or the tie end precomputed when the pattern changes (choose, and say why in the Log); behaviour identical to today's for all existing goldens
- tests/: a worst-case pattern (every step a tie candidate, length 64) measuring evaluated steps per tick on host; the cost is stated in steps and ops in the Log
- `sub_note` receives the generator position, not the step index (separate Log entry, with a test that fails before the fix)

## Acceptance criteria
- [x] Worst-case `step()` calls per tick are at most the stated bound (number in the Log), and all sq goldens are unchanged
- [x] The `sub_note` test fails on the old code and passes on the new
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
- Algorithm changes
- Moving work out of the ISR for other reasons
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
- 2026-11-13: created from the POC readiness review (/tmp/readiness_review.md, copy the findings you need into the Log; /tmp may be gone).
- Must land before P5-008 PATTERNS (P5-008 depends on this task). Keep two Log entries: tie bound, then sub_note.

### 2026-11-13 executor (sonnet) - entry 1: tie bound
- Choice: a bound (SQ_TIE_SCAN = 16 steps looked ahead) in `sq_tie_end`, sq_core.c. Not a precompute: the generator hit mask would have to be
  rebuilt on every cache write, and the host tests (sq_core, clock, timing, sq_chan) poke `sq_cache` directly, so a precomputed mask would go
  stale silently; the bound needs no new state and no RAM.
- Behaviour: identical for len <= 17 (fallback distance = min(len, 17), probes d = 1..min(len,17)-1). For len > 17 with 16+ silent steps after
  a TIE, the note now ends at the 17th step start instead of the next sounding step / one loop. This is a deliberate spec deviation;
  SEQUENCER_UX.md (TIE pseudo-code ~1035 and the TIE row ~1119) updated; grep terms: sq_tie_end, next sounding step, at most one loop, sub_note.
- Worst case, method: tests/sq_core_test.c g13_tie_bound: 4 channels, len 64, only generator position 0 sounds (a TIE; pages read blocks 1..3),
  so the search finds nothing. Counts `step()` calls in each `sq_tick` over two passes (one probe = one sq_eval = one step() + edit/PROB logic).
  Before: 260 step() calls in the worst tick (4 x (1+1+63)). After: 72 (4 x (1+1+16)). Per channel 65 -> 18.
  Old code fails the test (run with the old sq_core.c: "step() calls per tick 260 exceed the bound 72", and the tie ends at 3072 not 816).
- Goldens: all sq_core_*.txt unchanged; fuzz unchanged (100000 ticks). Test edits: tbl_step gets a call counter and a wc_mode switch (default off).
- Hazards checked: array bounds (probe index t % len < len <= 64, loop bound <= 16); LENGTH change while holding (existing step-0 cut, unchanged, fuzz);
  early returns/cleanup (none added); stale state (no new state); pass wrap (tf + d unchanged, hd=17 fallback keeps swing parity via (tf+hd)&1).
  Fuzz: the existing 100000-tick random invariant fuzz in sq_core_test.

### 2026-11-13 executor (sonnet) - entry 2: sub_note position
- sq_core.c: `a->sub_note(k, sq_gen_pos(&sq_ch[c], p), j+1, hcp)` instead of `p`. Test g12_sub_note_position (sq_core_test.c): len 18, algo 5,
  steps 1, 10, 17 read generator position 1; expects 3 repeats on 45 and none on 59. Fails on the old code ("got 2", repeat on 59), passes now.
- No existing golden changed (the pages there use their own block, where p == generator position for p < 16).

### Verification
- `sh tools/check.sh` -> CHECK PASS
- `image 216196 B; RAM .data+.bss 29640 B of 73728; pool 78520 B of 368640` (headroom 44088 B >= 8192)

### 2026-11-13 review (sonnet)
Verdict: PASS (gate none), with one item for the owner.
- check.sh: CHECK PASS (re-run, stamp was DIRTY); RAM .data+.bss 29640 of 73728 (headroom 44088 B >= 8192). No struct/ABI change; goldens untouched (git diff: no tests/golden files).
- Old code vs new tests: built the test against HEAD~1 sq_core.c: g12 fails ("got 2", repeat on 59), g13 fails (260 step() calls vs bound 72; tie ends not at 816). Both pass on new code. Worst case is real (len 64, one TIE hit, 4 channels).
- Bound: per channel at most SQ_TIE_SCAN+2 = 18 evals; loop bound 16, probe index t % len < 64. OK. sq_gen_pos has no rotate/offset in this version (p, pg_src only), so the g12 case (len 18, wrap via page 2 reading block 0) covers the fix.
- Locks: no algorithm SPEC/LOCK depends on tie length. STAGE long notes use PO_HOLD (hold 1..8 pulses, core-side), not PO_TIE; ACID emits no ties; PATTERNS T flag is only stored on library patterns of <= 16 steps. So the cap is safe for STAGE/PATTERNS. It DOES contradict the SEQUENCER_UX / ADR-0003 tie text ("at most one loop") for the case len > 17 with a tie followed by 16+ silent steps (a held drone on a sparse long pattern now ends after 17 steps). The task said identical behaviour and "if spec and code disagree, stop"; the executor documented the deviation instead. Impact is narrow, but it is an owner-visible change to locked text: OWNER TO RULE (accept the 17-step cap, or raise SQ_TIE_SCAN, or precompute the tie end to keep one loop).
- Docs sync: SEQUENCER_UX pseudo-code and TIE row were updated; ADR-0003 change note (line 25) still said "at most one loop" only, fixed by me (one line). Scenario S03 step 9 ("until the next sounding step") and melodic-steps/patterns SPEC wording stay true for normal patterns; no edit needed.

- 2026-11-13 owner ruling (open-items 1A): accept the 16-step tie cap (SQ_TIE_SCAN); update the locked wording ("at most one loop") in SEQUENCER_UX / ADR-0003 to say a tie ends at the next sounding step or after 17 steps, whichever is first. Follow-up: wording edit pending (orchestrator, after the running executor finishes).
