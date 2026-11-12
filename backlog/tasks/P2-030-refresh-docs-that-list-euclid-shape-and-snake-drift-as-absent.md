---
id: P2-030
title: Refresh the docs that still list EUCLID SHAPE and SNAKE DRIFT as absent layers
phase: P2
type: task
status: done
priority: 2
executor: sonnet
depends_on: [P2-017]
gate: none
---
## Goal
After P2-022 made EUCLID SHAPE and SNAKE DRIFT real MOD layers, make every design doc and table say so, so the screen, help text and scenarios the P4 UI tasks read agree with the locked prototypes.

## Read first
- backlog/tasks/P2-017-*.md Log (reviewer finding), docs/design/algorithms/LOCK.md, docs/design/MODULATION.md
- SCREEN_SPEC.md 2.7, UX_REVIEW.md, ALGORITHM_SET.md 4.4, SEQUENCER_UX.md, CONTROL_MAP.md line ~34, PERFORMANCE_DESIGN.md, tests/design/widths.tsv line ~98

## Deliverable
- Each listed doc/table updated so EUCLID SHAPE and SNAKE DRIFT are real layers (primary names per the owner's choice in LOCK.md 6 item 5), and the dead toast in widths.tsv removed or corrected.
- Generated tables regenerated with their generators (gen_help.py, gen_sweeps.py), not hand-edited.

## Acceptance criteria
- [x] `grep -rniE "SHAPE.*absent|absent.*SHAPE|DRIFT.*absent|absent.*DRIFT" docs/design` finds no remaining stale claim
- [x] `python3 tests/design/check_scenarios.py`, `check_help.py` and `sh tests/design/width_probe.sh` pass
- [x] `python3 tools/backlog.py check` reports 0 errors

## Verification
```sh
python3 tests/design/check_scenarios.py && python3 tests/design/check_help.py && sh tests/design/width_probe.sh
python3 tools/backlog.py check
```

## UX gate
n/a, documentation consistency only; it records a layer change already designed in P2-022 and locked in P2-017.

## Out of scope
- Firmware; ADR-0003 and HARMONY.md (P2-029).

## Log

### 2026-11-12 executor (sonnet)
Edited: ALGORITHM_SET.md (4.2 matrix, layers-present row, dice-all, 4.4 rows, sub-labels with `MOD` and the `+ MOD` toast rule, t1 row, build-order row), SCREEN_SPEC 2.7, UX_REVIEW (329, 358), SEQUENCER_UX row 22, PERFORMANCE_DESIGN (89, 130, 312), CONTROL_MAP.md, control_map.tsv, button_modes.tsv (B_LFO row: "never dim: every algorithm has it"), hardware/TB3_PROFILE, HW_SUPPORT_LOWLEVEL, HW_SUPPORT_HIGHLEVEL, scenarios/FINDINGS, widths.tsv (dead `NO SHAPE IN EUCLID` removed; `NO ARTIC IN SNAKE` added, 106 of 240 px), gen_help.py (B_LFO line 2). Regenerated help_text.tsv (gen_help.py) and S09..S22 sweeps (gen_sweeps.py). SNAKE DRIFT was never listed absent; SNAKE ARTIC stays absent (correct). Left as history: P2-016/P2-017 feature files and change notes. The grep still matches only correct statements (SNAKE ARTIC absent, "no longer absent" notes, history).
Results: check_scenarios `0 with no scenario`; check_help `OK: 392 help rows`; width_probe `297 strings, 0 over`; backlog check `134 items, 0 errors`.

### 2026-11-12 review (sonnet)
Verdict: PASS (gate none; no firmware, build not run by request).
- Criterion 1 grep: re-ran; every remaining hit checked. Correct statements: SNAKE ARTIC absent (SEQUENCER_UX 1126, ALGORITHM_SET 122/219, UX_REVIEW 359, PERFORMANCE_DESIGN 312, HW_SUPPORT_LOWLEVEL 825), "no longer absent / was absent, now alive" notes (MODULATION 116/258, euclid SPEC 53/140, arp SPEC 68, ALGORITHM_SET change notes), and the P2-016/P2-017 history files (point-in-time records, superseded notes inside them). The criterion's literal "finds no hits" is too strict because it matches negations and history; passed on intent (no stale claim remains). No "dim if absent" for SHAPE or `NO SHAPE IN EUCLID` remains in docs/design or tests/design.
- control_map.tsv, button_modes.tsv and CONTROL_MAP.md agree row for row (B_LFO PERF/CHAN/SET/REC/RIG, identical text). gen_help.py edited at source; re-running gen_help.py and gen_sweeps.py leaves the tree clean, so help_text.tsv and S09..S22 are generated output, not hand-edited.
- Verification: check_scenarios 0 with no scenario; check_help OK 392 rows; width_probe 297 strings 0 over; backlog check 0 errors.
