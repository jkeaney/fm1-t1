---
id: P0-003
title: Add tools/check.sh: one command for build + tests + backlog
phase: P0
type: task
status: done
priority: 1
executor: haiku
depends_on: [P0-002]
gate: none
---
## Goal
One verification command that every task's Definition of Done can cite.

## Read first
- build.sh
- tests/run_tests.sh
- tools/backlog.py (check)
- CLAUDE.md (Commands)

## Deliverable
- tools/check.sh (POSIX sh, executable, GPL SPDX header) that: exports AC79_SDK (default ~/fw-AC79_AIoT_SDK) and prepends $PWD/.venv/bin to PATH; runs ./build.sh; extracts the `RAM .data+.bss N of M` line and fails if M-N < 4096; runs `sh tests/run_tests.sh`; runs `python3 tools/backlog.py check`; prints one final line `CHECK PASS` or `CHECK FAIL: <stage>`, with full logs in build/check/*.log
- CLAUDE.md Commands section: one line documenting tools/check.sh
- docs/project/PROCESS.md Definition of Done: replace the 'until then' clause with `tools/check.sh` prints CHECK PASS

## Acceptance criteria
- [x] `sh tools/check.sh` ends with `CHECK PASS` on a clean tree
- [x] Temporarily breaking a test (e.g. `exit 1` at the top of tests/run_tests.sh, reverted afterwards) makes it end with `CHECK FAIL: tests`
- [x] Exit code is 0 on pass and 1 on fail

## Verification
```sh
sh tools/check.sh; echo exit=$?   # last lines: CHECK PASS / exit=0
```

## Out of scope
- Changing build.sh or run_tests.sh behaviour.

## Log

### 2026-11-09 executor (haiku)

Commands and decisive output:

- `sh tools/check.sh > /tmp/p0003_check_run1.log` (first attempt): `== ram headroom` then `CHECK FAIL: ram`, exit=1. Cause: the RAM regex missed the ` B` unit in the build line (`RAM .data+.bss 55744 B of 73728`). Fixed the pattern and the total field (now `$6`). Build itself was fine.
- `sh tools/check.sh > /tmp/p0003_check_run2.log` (the one full host-suite run): `RAM .data+.bss 55744 B of 73728`, `ALL HOST TESTS PASSED` (build/check/tests.log), `29 items, 0 errors` (build/check/backlog.log), `CHECK PASS`, `exit=0`.
- Breakage test: `exit 1` inserted at line 2 of tests/run_tests.sh, `sh tools/check.sh`: `CHECK FAIL: tests`, `exit=1`. File restored from a backup; `git diff --stat -- tests/run_tests.sh` is empty.
- Also edited CLAUDE.md Commands (one line for tools/check.sh) and PROCESS.md Definition of Done (the "until then" clause replaced).
- Local git config not touched.

All three acceptance criteria verified and ticked.

### 2026-11-09 review (sonnet)

Verdict: PASS (gate none). Full suite not rerun, per LESSONS.md; executor's CHECK PASS and breakage log accepted.

Checks run by reviewer:
- `tools/check.sh` exists, mode `-rwxr-xr-x`, SPDX header present, `sh -n` clean.
- Stages in order: build, ram (headroom >= 4096), tests, backlog. Each failure path calls `fail <stage>`, which prints `CHECK FAIL: <stage>` and exits 1. Pass path prints `CHECK PASS` and exits 0.
- RAM parse against the existing log `build/check/build.log:27` (`RAM .data+.bss 55744 B of 73728`) gives used=55744, total=73728, PASS. Simulated used=70000 under `sh` gives `CHECK FAIL: ram`. Note the script must run under `sh`, not zsh: zsh does not word-split `$line`.
- `build/check/tests.log` ends with ALL HOST TESTS PASSED; `build/check/backlog.log` says `29 items, 0 errors`; `python3 tools/backlog.py check` exits 0.
- CLAUDE.md Commands line (`sh tools/check.sh ...`) matches the script's behaviour. Accurate.
- PROCESS.md:54 reads `tools/check.sh` prints `CHECK PASS` (orchestrator edit, expected).
- Scope: commit 190b428 touched only CLAUDE.md, tools/check.sh and the task file. No firmware or test changes.

Non-blocking nit (not a fail): tools/check.sh regex uses `[0-9]*`, which can match empty digits. Harmless, because the build line always has digits. Optional fix: `[0-9][0-9]*`.

Note: `backlog/BACKLOG.md` was stale in the committed tree (P1 rows still showed haiku). Regenerating the index picks up the sonnet executors from the committed task files, plus this task's status change.
