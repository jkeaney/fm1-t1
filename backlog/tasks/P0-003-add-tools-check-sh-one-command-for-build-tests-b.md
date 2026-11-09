---
id: P0-003
title: Add tools/check.sh: one command for build + tests + backlog
phase: P0
type: task
status: todo
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
- [ ] `sh tools/check.sh` ends with `CHECK PASS` on a clean tree
- [ ] Temporarily breaking a test (e.g. `exit 1` at the top of tests/run_tests.sh, reverted afterwards) makes it end with `CHECK FAIL: tests`
- [ ] Exit code is 0 on pass and 1 on fail

## Verification
```sh
sh tools/check.sh; echo exit=$?   # last lines: CHECK PASS / exit=0
```

## Out of scope
- Changing build.sh or run_tests.sh behaviour.

## Log
