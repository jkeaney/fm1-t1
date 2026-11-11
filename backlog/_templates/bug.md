---
id: BUG-000
title: What is wrong, in one line
phase: P2
type: bug
status: todo
priority: 1
executor: sonnet
depends_on: []
gate: none
severity: medium   # high: wrong notes, crash, a core feature broken · medium: a control misbehaves · low: cosmetic or latent
found_in: <git SHA or build>
---
## Goal
Fix: <behaviour> so that <expected>.

## Read first
- Source files involved; the related T1_ISSUES.md entry, if any.

## Deliverable
- The fix, plus a host test that failed before it (where host-testable).

## Steps to reproduce
1.

## Expected

## Actual

## Acceptance criteria
- [ ] The new test fails on the old code and passes on the new.
- [ ] `tools/check.sh` passes.

## Verification
```sh
```

## UX gate
n/a, or the use case this bug breaks (docs/design/features/); a fix that changes a control or screen needs Gate A.

## Log
