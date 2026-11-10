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
- docs/design/algorithms/common/sq_types.h final for six algorithms (SQ_NSEED, cache sizes, any ChanSave field PATTERNS needs); every `/* P2-017 fills */` resolved
- A one-page LOCK.md in docs/design/algorithms/: the six algorithms, their RAM/pool/flash cost, the picker screen with six names (width_probe), and what the owner is approving

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

## Out of scope
- firmware/

## Log
