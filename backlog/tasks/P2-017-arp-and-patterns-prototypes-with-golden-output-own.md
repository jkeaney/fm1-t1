---
id: P2-017
title: ARP and PATTERNS prototypes with golden output; owner locks all six prototypes
phase: P2
type: task
status: todo
priority: 1
executor: sonnet
depends_on: [P2-013, P2-014, P2-016]
gate: human-review
---
## Goal
Prototype ARP and PATTERNS against the locked P2-013/P2-014 designs, finalise `sq_types.h` for six algorithms, and present all six prototypes for the owner's design lock (ADR-0004). After this, every algorithm dev task is a copy-and-wire job.

## Read first
- docs/design/algorithms/arp/SPEC.md (P2-013) and docs/design/algorithms/patterns/ (P2-014: RESEARCH.md, the pattern data, owner decision 90)
- docs/design/algorithms/common/ (P2-008)
- docs/design/HARMONY.md §9 (query interface ARP and PATTERNS call)
- docs/project/decisions/ADR-0003-sequencer-core.md §3, §4, §11 (budget)

## Deliverable
- docs/design/algorithms/arp/ and patterns/: SPEC.md, proto.c, golden.txt; patterns/ also holds the pattern data as a C const table (degree-relative, structural/decoration flags) and a generator script if the data is produced from RESEARCH.md
- ARP golden cases (scheduled by P2-013, SPEC 11): the default of SPEC 10.1; every DIR x SHAPE x RATE at SPAN 1..4 for chord I and the `1451` bars of 6.7; the single-note cases of 6.8; ROLE OFF and ROOT; each dice layer from a fixed seed; the RAND permutation property for P = 1..24; cache 48 B and the cost bounds of SPEC 9; the picker strings of SPEC 7.3 stay in tests/design/widths.tsv
- docs/design/algorithms/common/sq_types.h final for six algorithms (SQ_NSEED, cache sizes, any ChanSave field PATTERNS needs); every `/* P2-017 fills */` resolved
- A one-page LOCK.md in docs/design/algorithms/: the six algorithms, their RAM/pool/flash cost, the picker screen with six names (width_probe), and what the owner is approving

- From the P2-016 review: P2-022 made EUCLID SHAPE and SNAKE DRIFT real layers (MOD, second seed `s_mod`, mod lanes). Add `s_mod` and the mod lanes to the EUCLID and SNAKE prototypes (and ACID/STAGE if affected) and regenerate their goldens before the owner locks all six; the "absent layer" goldens are stale.
- From the P2-016 review: the harness chord run is a test approximation (strong steps from velocity >= 100 and holds); EUCLID ordinal 0 and STAGE stage starts are left to the core. Note this in the SPECs.

## Acceptance criteria
- [ ] `sh tests/design/run_protos.sh` prints `PROTOS PASS` with six algorithms
- [ ] No `P2-017 fills` marker left (`grep -r 'P2-017 fills' docs/design/algorithms` is empty)
- [ ] PATTERNS golden has morph min/mid/max for at least one pattern per category, and every shipped pattern stays in scale under three keys
- [ ] Pattern names are style names, never song titles (decision 90)
- [ ] RAM, pool and flash totals for all six are within PROGRAM_PLAN §5, else the overrun and a proposal are stated
- [ ] Design locked: owner line in the log

## Verification
```sh
sh tests/design/run_protos.sh   # PROTOS PASS
sh tests/design/width_probe.sh
python3 tools/backlog.py check
```

## UX gate
Producer-facing. Use case: `docs/design/features/P2-017-arp-and-patterns-prototypes-with-golden.md` from `_TEMPLATE.md` (docs/project/UX_GATE.md). Gate A (steps, predicted ease score, five-surface sync, live checklist) is filled before the build or cited from the locked design; Gate B is filled by the reviewer.

## Out of scope
- firmware/

## Log
