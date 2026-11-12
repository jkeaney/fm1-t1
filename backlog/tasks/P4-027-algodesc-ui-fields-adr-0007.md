---
id: P4-027
title: AlgoDesc / MorphDesc UI fields (ADR-0007) and the build-time width check
phase: P4
type: task
status: todo
priority: 1
executor: sonnet
depends_on: [P5-001]
gate: none
---
## Goal
Let every algorithm describe its UI (short name, lane type, value names, unit, zero word, value-text hook) so the screen
has no per-algorithm code and new algorithms reach every screen element (owner requirement, ADR-0007).

## Read first
- docs/project/decisions/ADR-0007-algorithm-ui-descriptor.md; ADR-0003 section 4; docs/design/algorithms/LOCK.md section 5
- firmware/src/sq_types.h (AlgoDesc, MorphDesc), sq_algo_acid.c (ALGO_ACID_ROW), sq_algos.c
- docs/design/ui_review/proposals/v12/algos_v12.py (the mock registry, `check()`, `value_text`, `value_font`, `is_list`)
- docs/design/knob_map.tsv PERF rows; tools/gen_ui_tables.py

## Deliverable
- sq_types.h: the fields of ADR-0007 with asserted sizes; ACID's row filled from knob_map.tsv.
- tools/gen_ui_tables.py: the `check()` rules over every algorithm row and every SCREEN_SPEC_V2 MESSAGES string; fails the build on a misfit.
- tests: a host test that registers a dummy algorithm and renders its knob tiles through the screen code unchanged.

## Acceptance criteria
- [ ] struct size asserts updated and documented in the Log (ADR-0003 amendment)
- [ ] `python3 tools/gen_ui_tables.py --check` passes and fails on a deliberately too-wide label (tested)
- [ ] the dummy-algorithm host test passes with no screen-code change
- [ ] `sh tools/check.sh` ends CHECK PASS

## Verification
```sh
sh tools/check.sh
```

## UX gate
docs/design/features/P4-026-screen-v2-main-screen.md (Gate A filled, Design locked 2026-11-12).

## Out of scope
- EUCLID..PATTERNS rows (their P5 tasks fill the new fields); screen drawing.

## Log
