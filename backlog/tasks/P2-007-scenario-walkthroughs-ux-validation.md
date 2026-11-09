---
id: P2-007
title: Scenario walkthroughs: validate the UX design end to end
phase: P2
type: task
status: todo
priority: 1
executor: haiku
depends_on: [P2-001, P2-003]
gate: human-review
---
## Goal
Script the target workflow as step-by-step scenarios so the control map and screen spec are proven complete before development (ADR-0004).

## Read first
- docs/project/decisions/ADR-0004-design-lock.md
- docs/project/VISION.md
- docs/design/CONTROL_MAP.md and the screen spec from P2-001 and P2-003

## Deliverable
- docs/design/scenarios/S*.md, one per scenario, each a numbered table: step | user action | exact screen text | MIDI out | control-map row id
- docs/design/scenarios/COVERAGE.md mapping every control-map row to the scenarios that use it

## Acceptance criteria
- [ ] Scenarios cover: TB-3 first-run flow; step entry on white keys; dialing a sequence with knobs; morph; dice per layer; switching algorithm; muting; save/load
- [ ] Every control-map row appears in COVERAGE.md (rows with no scenario listed as gaps and sent back to P2-001)
- [ ] No step contains 'TBD', 'choose' or 'e.g.'
- [ ] Any menu-diving step (>1 press away) is flagged as a vision violation
- [ ] Design locked: owner line in the log

## Verification
```sh
python3 tools/backlog.py check
grep -rEn 'TBD|choose|e\.g\.' docs/design/scenarios || echo clean
```

## Out of scope
- Changing the locked designs, or touching firmware outside docs/design and tests/design.

## Log
