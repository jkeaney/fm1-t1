---
id: P2-016
title: EUCLID, SNAKE and STAGE prototypes with golden output
phase: P2
type: task
status: todo
priority: 1
executor: sonnet
depends_on: [P2-008]
gate: none
---
## Goal
Add the other three classic v1 algorithms to the P2-008 harness, each as the future firmware file with its AlgoDesc table and golden output.

## Read first
- docs/design/algorithms/common/ and acid/ (P2-008: the pattern to follow)
- docs/project/decisions/ADR-0003-sequencer-core.md §4.1, §4.2, §14
- docs/design/ALGORITHM_SET.md §5.2, §5.2b, §6, §7 (STAGE is the most complex: variable loop 8..64, cumulative lookup, holds)
- docs/t1-algorithms/ (EUCLID source; `t1_euclidean_bjorklund`/`_rotate` are reused unchanged, SP11), docs/snake-algorithms/, docs/ml185-algorithms/ (STAGE)
- docs/T1_ISSUES.md F3 (EUCLID's phrase contour depends on the BUG-001 fix: prototype the fixed behaviour)
- docs/design/HARMONY.md §4.3 (per-algorithm pull and strong steps)

## Deliverable
- docs/design/algorithms/euclid/, snake/, stage/: SPEC.md, proto.c, golden.txt each, same layout as acid/
- tests/design/run_protos.sh runs all four

## Acceptance criteria
- [ ] `sh tests/design/run_protos.sh` prints `PROTOS PASS` with four algorithms
- [ ] Each golden covers morph min/mid/max per knob, each present dice layer, and the absent layers (EUCLID SHAPE, SNAKE ARTIC) as no-ops
- [ ] Each runs H-INERT and the 1-4-5-1 ROLE FULL case through harm.c
- [ ] No 'TBD' in any SPEC.md; cache_bytes <= 64 for each

## Verification
```sh
sh tests/design/run_protos.sh   # PROTOS PASS
python3 tools/backlog.py check
```

## Out of scope
- firmware/
- ARP and PATTERNS (P2-017)

## Log
