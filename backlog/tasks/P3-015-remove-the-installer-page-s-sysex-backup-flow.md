---
id: P3-015
title: Remove the installer page's SysEx backup flow (keep plain restore to V15)
phase: P3
type: task
status: done
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
- [x] No SysEx backup code or text remains on the installer page
- [x] Restore to official V15 still present and unchanged
- [x] `sh tools/check.sh` ends CHECK PASS (the web pages step has the known spaces-in-path failure; note it if seen)

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
### 2026-11-12 executor (sonnet)
Removed:
- web/index_pkg.html: backup branch of the stock-restore click handler (BackupConnection, archiveInfo, captureBackup, saveArchive), the `stockBackup`, `stockRecovery`, `stockNeedBackup`, `stockUnsupported` and `backup` strings (ja and en), the "resume" checkbox, and the "use the editor" text. "Back up and" dropped from the button label. Added `stockWarn` ("Official V15 cannot open Felucca music") and reworded `stockConfirm` to match.
- web/fm1backup.js and web/test_backup.mjs (git rm); the fm1backup.js inlining in web/make_site.py; the node backup test step and its comment in tests/run_tests.sh.
Kept: file pick and validation, update-mode resume, plain `up.install` to V15, with a confirm.
Verification:
- `sh tools/check.sh` -> CHECK PASS (RAM .data+.bss 23060 of 73728)
- `grep -rniI "backup\|use the editor" web/` (excluding web/emu) -> no output
- The installer script block parses (`node --check`).
Note: the page's restore flow was not run in a browser.

### 2026-11-12 review (sonnet)
Verdict: PASS (gate none).
- `sh tools/check.sh` -> CHECK PASS; RAM .data+.bss 23060 of 73728 (headroom 50668 B, above 8192).
- grep for backup / use the editor / BackupConnection / captureBackup / stock-recovery / fm1backup in web, tools, tests: no hits outside web/emu.
- Restore click handler read in full: the stock-recovery reference is gone, every `$("id")` used has a matching element, every sayK/t key (stockWarn, stockConfirm, stockValid, stockError, ...) is defined in both ja and en. Resume branch (update loader) now confirms then `up.resume`; normal branch closes the link, confirms, then `up.install` to V15. `link` variable and finally-clause cleanly removed. lockInstaller no longer touches the removed checkbox.
- make_site.py and run_tests.sh diffs are minimal and consistent (fm1backup.js and test_backup.mjs deleted, no dangling refs).
- UX (brief): restore reachable in the details section, warning (stockWarn) shown before and in the confirm, no dead UI. Not run in a browser (executor's note stands); the owner can try it when next flashing.
