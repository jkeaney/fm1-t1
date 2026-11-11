---
id: FB-004
title: P3-002 slicer removal moves unrelated budget rows and fails the INSERT cost check
phase: P3
type: feedback
status: done
priority: 2
executor: human
depends_on: [P3-002]
gate: none
session: 2026-11-11, executor (haiku), P3-002 run on the working tree after the slicer deletion
---
## Goal
Decide what P3-002 does when the host tests show results the task says to stop on.

## Read first
- backlog/tasks/P3-002-remove-the-slicer-insert-r5a.md (Deliverable: "Any other golden or budget change: stop and file an FB")
- tests/insert_test.c (the "cost" check, about line 4368 of the run log)

## Observation
`GOLDEN_UPDATE=1 BUDGET_UPDATE=1 sh tests/run_tests.sh` after the slicer removal:
- Golden: exactly as expected (`slicer/gate/ANALOG_ACID` and `slicer/song_gate_stut` removed, `song/4track_swing20` added). 0 existing renders changed.
- Budget: the expected `slicer_track` row is gone, but about 25 other rows moved, for example `analog_render 585 -> 614`, `fm1_alnk0_irq 37502 -> 17977`, `fm1_timer5_irq 33387 -> 34807`, `grain_render 1731 -> 1723`, `perf_block 814 -> 853`, `rev_hall 1995 -> 1994`. The task says any budget change other than the slicer row must stop the task.
- Test: `insert: cost: MIX 0 next to nothing; every TYPE a fixed cost per track below 10 % of an 8-voice part FAIL`. Removing the slicer from the 8-voice part makes it cheaper, so each INSERT's fixed cost is a larger share. Test threshold, not a code fault. insert_test is deleted in P3-014.
- Host tests end `HOST TESTS FAILED` because of this single check.
- Earlier scratch audit (FB-003) reported a `mod_test` MOD_COST_MAX failure in the combined P3-002 + P3-014 run. This run shows the INSERT check instead; the P3-002-alone outcome was not recorded.
- Build: RAM .data+.bss 55744 -> 55584; pool 342876 -> 342876 (matches the scratch result).

## Questions for the owner
A. Accept the moved budget rows as the new baseline (BUDGET_UPDATE once, with a note in the log), or keep the old numbers and investigate why the ISR rows moved (`fm1_alnk0_irq` halved)?
B. The INSERT cost check: loosen its threshold in P3-002 (an edit outside the deliverable), or move the insert_test cost part into P3-014 and skip it here?
C. Is the mod_test cost failure from FB-003 still expected in P3-002 alone?

## Deliverable
- Owner answers A-C; P3-002 amended to match, or re-scoped.

## Acceptance criteria
- [ ] Owner answers A-C.

## Verification
```sh
python3 tools/backlog.py check
```

## Log
- 2026-11-11 executor (haiku): P3-002 blocked on this finding. Working tree: slicer deleted and every named caller and test edit applied; `tests/target_budget.txt` restored to HEAD minus the `slicer_track` row (the BUDGET_UPDATE rewrite is not kept). `tests/golden.txt` holds the expected golden change.
- 2026-11-11 orchestrator (answers, technical grounds):
  - A: tests/target_budget.py is a static count of loop instructions from build/felucca.dis (not a host timing): exact run to run, no noise. It gates only growth (+10 %); decreases print a `note`. So the moved rows are a real compiler effect of removing the slicer (slicer_track was inlined into the audio ISR: fm1_alnk0_irq 37502 -> 17977, wheel_block 102 -> 86, rev_side_mix 9 -> 8). The BUDGET_UPDATE rewrite also re-baselined rows that only moved within tolerance. Decision: keep HEAD numbers minus the `slicer_track` row, no BUDGET_UPDATE. The budget check passes (3 `note` lines, 0 over). The ISR headroom is wide until a later task re-baselines; not a P3-002 matter.
  - B: the INSERT cost check is host-instruction-count noise, not the 8-voice part getting cheaper: with HEAD budget and no other change it passed once (PHASR MIX 0 delta +0.2) and failed once (+6.4 vs the 0.5 % limit of 6.4). The 10 % TYPE limit has margin (worst 6.2 %). Fix: tests/insert_test.c limit for the MIX 0 delta 0.5 % -> 1 %, with a comment. insert_test is deleted in P3-014.
  - C: not seen in P3-002 alone. mod_test cost ran +4.3 % (limit 5.0) and passed; the FB-003 failure belongs to the combined P3-002 + P3-014 run (P3-014 concern). No change to MOD_COST_MAX.
  - Also: the DRUM grid stress UBSan `division by zero` at fx.c:224 (beat_samples, G_BPM 0) appeared once in one of three runs here and not on the rerun, as the P3-002 task predicted; it is not a slicer caller. Not edited around.
