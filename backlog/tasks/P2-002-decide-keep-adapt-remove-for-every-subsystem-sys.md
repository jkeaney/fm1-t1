---
id: P2-002
title: Decide keep / adapt / remove for every subsystem (system map)
phase: P2
type: task
status: todo
priority: 1
executor: opus
depends_on: [P1-002, P2-001]
gate: human-review
---
## Goal
Define the 'clean house' target: only UI, system, algorithms and setup that drive the vision remain (ADR-0001).

## Read first
- docs/design/SYSTEM_INVENTORY.md
- docs/design/CONTROL_MAP.md
- docs/design/ALGORITHM_SET.md
- docs/project/decisions/ADR-0001-fork-felucca-and-keep-t1.md

## Deliverable
- docs/design/SYSTEM_MAP.md: each inventory row marked KEEP / ADAPT (what changes) / REMOVE, with a reason; the removal order (leaves first, by the dependency column); the expected flash and RAM freed; the host test suites that go or change with each removal; which engine stays as the preview synth and why

## Acceptance criteria
- [ ] Every SYSTEM_INVENTORY row has a decision
- [ ] The platform-core list is all KEEP; the update loader and installer path are KEEP
- [ ] The removal order never removes something that a KEEP item still depends on
- [ ] The owner approved it (log line)

## Verification
```sh
grep -cE 'KEEP|ADAPT|REMOVE' docs/design/SYSTEM_MAP.md
```

## Out of scope
- Removing code (that's P3).

## Log
