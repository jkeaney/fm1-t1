---
id: P2-016
title: EUCLID, SNAKE and STAGE prototypes with golden output
phase: P2
type: task
status: review
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
- [x] `sh tests/design/run_protos.sh` prints `PROTOS PASS` with four algorithms
- [x] Each golden covers morph min/mid/max per knob, each present dice layer, and the absent layers (EUCLID SHAPE, SNAKE ARTIC) as no-ops
- [x] Each runs H-INERT and the 1-4-5-1 ROLE FULL case through harm.c
- [x] No 'TBD' in any SPEC.md; cache_bytes <= 64 for each

## Verification
```sh
sh tests/design/run_protos.sh   # PROTOS PASS
python3 tools/backlog.py check
```

## UX gate
Producer-facing. Use case: `docs/design/features/P2-016-euclid-snake-and-stage-prototypes-with-g.md` from `_TEMPLATE.md` (docs/project/UX_GATE.md). Gate A (steps, predicted ease score, five-surface sync, live checklist) is filled before the build or cited from the locked design; Gate B is filled by the reviewer.

## Out of scope
- firmware/
- ARP and PATTERNS (P2-017)

## Log

### 2026-11-12 executor (sonnet)
Files: docs/design/algorithms/{euclid,snake,stage}/{SPEC.md,proto.c,golden.txt}; tests/design/euclid_ref.py (independent rhythm check); tests/design/proto_main.c (absent-layer no-op check, PO_HOLD counts as a strong step); tests/design/run_protos.sh (requires >= 4 algorithms); docs/design/features/P2-016-... (Gate A, sections 1-6 filled, "Design locked" pending the reviewer).
```
$ sh tests/design/run_protos.sh
H-GOLDEN: 3566 table lines equal harmony_proto.py
ACID: ... acid_ref: 53 cases, 0 mismatches
EUCLID: matrix, purity, table and HARMONY checks OK
euclid_ref: 51 cases, 0 mismatches
euclid: 58 golden sections, golden.txt equal, two runs byte-identical
SNAKE: ... snake: 58 golden sections ...   STAGE: ... stage: 58 golden sections ...
algorithms: 4
PROTOS PASS
$ python3 tools/backlog.py check   -> 133 items, 0 errors
$ grep -c TBD docs/design/algorithms/{euclid,snake,stage}/SPEC.md   -> 0 each
```
cache_bytes: EUCLID 30, SNAKE 55, STAGE 43 (all <= 64, asserted by _Static_assert and the table test). ACID golden.txt is unchanged (git shows no diff).
Checks of the prototypes against the assessments: SNAKE's snake.md informative check (hit pattern and velocities of 16 steps) and STAGE's ml185.md one (sounding steps, velocities, the L=1 hold at step 13) are reproduced; notes differ by design (late quantise, SP6/SP7).
Reviewer, please look at (SPEC section 7 of each): (1) `hi(m)`: ADR-0003 section 20 gives max(SPAN,12) / max(RANGE,12), too low once jitter and the snap-up are counted; I used SPAN+3 and RANGE+1 (ADR not edited, P2-029 owns it); (2) EUCLID's "even" pace is not storable in PulseOut (one PO_PACE_DEC bit), so d==2 decelerates, the other two accelerate; (3) knob_morph for EUCLID {0,4,5,3,1,2,6,7} and STAGE {0,1,7,3,4,5,6,2} follow knob_map.tsv (3B re-lock), while ALGORITHM_SET 5.2 / FULL_CONTROL 3 show the older order; (4) STAGE's I- `hold` is computed by step() from the traversal (exact for FWD/BWD/PEND); (5) the HARMONY chord run in the driver marks strong steps from vel>=100 and holds only (EUCLID ordinal 0 and STAGE stage starts are the core's).
Not run: sh tools/check.sh (no firmware change; the firmware build needs Docker).

