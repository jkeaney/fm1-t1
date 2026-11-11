---
id: P3-015
title: Remove the installer page's SysEx backup flow (keep plain restore to V15)
phase: P3
type: task
status: todo
priority: 2
executor: sonnet
depends_on: [P3-013]
gate: none
---
## Goal
Decision 114: the installer page's SysEx backup flow times out on the cleaned firmware and is not wanted. Keep only the plain restore to official V15, and drop the stale "use the editor" text. Origin: FB-005 follow-up noted in the P3-012 review.

## Read first
- docs/project/OWNER_QUESTIONS.md (decision 114)
- docs/design/CLEAN_HOUSE_REPORT.md section 5
- The installer page under web/ (find it with grep for "backup" and "SysEx")

## Deliverable
- Installer page without the backup flow or "use the editor" text; restore-to-V15 path unchanged; a log entry listing what was removed.

## Acceptance criteria
- [ ] No SysEx backup code or text remains on the installer page
- [ ] Restore to official V15 still present and unchanged
- [ ] `sh tools/check.sh` ends CHECK PASS (the web pages step has the known spaces-in-path failure; note it if seen)

## Verification
```sh
sh tools/check.sh
grep -rni "backup\|use the editor" web/ | head
```

## UX gate
n/a, internal work (tooling, plumbing, docs, process); no producer-visible control, screen or sound change.

## Out of scope
- Firmware changes; the web emulator.

## Log
