---
id: BUG-005
title: P3-009 rehearsal patch does not apply to HEAD (tests/hostsim.c comments changed by P3-008 commit 7a8b4eb)
phase: P3
type: bug
status: done
priority: 1
executor: sonnet
depends_on: [P3-008]
gate: none
severity: medium   # high: wrong notes, crash, a core feature broken · medium: a control misbehaves · low: cosmetic or latent
found_in: 8ae981b (HEAD when P3-009 was run)
---
## Goal
Regenerate docs/design/rehearsal/P3-009.patch on HEAD so that `git apply --check` passes, as BUG-004 did for P3-008.

## Read first
- docs/design/rehearsal/P3-009.patch, P3-009.deletes.txt, the BUG-004 entry (same failure mode, same fix)
- tests/hostsim.c around lines 100-135 and 820-830 (the hunks that fail)

## Deliverable
- Regenerated P3-009.patch on HEAD, with the same content as before (the P3-008 and P3-011 base changes are already in HEAD).

## Steps to reproduce
1. `git apply --check docs/design/rehearsal/P3-009.patch`

## Expected
Passes with no errors.

## Actual
```
error: while searching for:
    eng_mem_claim(t);   /* ... */
}
...
error: patch failed: tests/hostsim.c:133
error: tests/hostsim.c: patch does not apply
```
Only tests/hostsim.c fails. Commit 7a8b4eb (P3-008) rewrote the comments of the `host_legacy_sample_perc` block
("turns it into DRUM" is now "turns it into a melodic part") and the `host_drums` comment, after the patch was
regenerated on c6a6a16. Other hunks in the patch are not reported as failing by `git apply --check`.

## Acceptance criteria
- [ ] `git apply --check docs/design/rehearsal/P3-009.patch` exits 0 on HEAD.
- [ ] P3-009 can run its deliverable steps on the regenerated patch.

## Verification
```sh
git apply --check docs/design/rehearsal/P3-009.patch
```

## Log
- Found while running P3-009 (executor, 2026-11-11): P3-009 blocked on this.
- Fixed (2026-11-11, orchestrator): regenerated docs/design/rehearsal/P3-009.patch on HEAD 8ae981b in a scratch copy (only the tests/hostsim.c hunk differed: the `host_legacy_sample_perc` block removal). `git apply --check` passes on HEAD. P3-009 build line unchanged (image 189808, RAM 26676, pool 144052), whole suite passes. P3-010.patch needed no change: it fails on HEAD by design and applies on HEAD + P3-009; build 169084 / 23060 / 92500, whole suite passes.
