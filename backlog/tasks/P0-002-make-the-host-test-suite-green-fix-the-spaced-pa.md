---
id: P0-002
title: Make the host test suite green: fix the spaced-path web test and gate wall-clock benchmarks
phase: P0
type: task
status: review
priority: 0
executor: haiku
depends_on: [P0-001]
gate: none
---
## Goal
`sh tests/run_tests.sh` must exit 0 on a clean tree, so 'tests pass' means something. Two known false failures: the web test breaks on folder names with spaces, and two T-1 wall-clock benchmarks fail on a busy host.

## Read first
- web/test_web.mjs:25 (`const HERE = new URL(".", import.meta.url).pathname;` gives `%20` for spaces)
- tests/torso_phase3_test.c:500 (`TEST_ASSERT(max_us < 80u, ...)`)
- tests/torso_phase4_test.c:151 (`TEST_ASSERT(max_us < 10u, ...)`)
- docs/T1_ISSUES.md F18

## Deliverable
- web/test_web.mjs: compute HERE with `fileURLToPath(new URL(".", import.meta.url))` (import `fileURLToPath` from "node:url"); apply the same fix anywhere else in web/*.mjs that uses `.pathname` of import.meta.url
- tests/torso_phase3_test.c and tests/torso_phase4_test.c: the two timing assertions run only when the environment variable `T1_BENCH=1` is set (`getenv`); otherwise print the measured value and the line `(benchmark not enforced: set T1_BENCH=1)`
- docs/T1_ISSUES.md: F18 status changed to FIXED with one line saying how

## Acceptance criteria
- [x] `grep -n "import.meta.url).pathname" web/*.mjs` prints nothing
- [x] With `T1_BENCH` unset, `sh tests/run_tests.sh` exits 0 (after `./build.sh`)
- [x] With `T1_BENCH=1`, the two benchmarks still assert (check the source: the TEST_ASSERT is still there, inside the getenv branch)

## Verification
```sh
./build.sh >/dev/null && export AC79_SDK=~/fw-AC79_AIoT_SDK PATH="$PWD/.venv/bin:$PATH"
sh tests/run_tests.sh > /tmp/tests.log 2>&1; echo exit=$?     # exit=0
grep -c 'STEP FAILED' /tmp/tests.log                            # 0
```

## Out of scope
- Any other test or firmware change.

## Log

### 2026-11-09 executor (claude-haiku-5-5)

Changes:
- `web/test_web.mjs`: `HERE = fileURLToPath(new URL(".", import.meta.url))`, `fileURLToPath` imported from `node:url`. No other `web/*.mjs` used `import.meta.url).pathname`.
- `tests/torso_phase3_test.c` and `tests/torso_phase4_test.c`: the `max_us` TEST_ASSERT runs inside `if (getenv("T1_BENCH") && getenv("T1_BENCH")[0] == '1')`; otherwise prints `(benchmark not enforced: set T1_BENCH=1)`.
- `docs/T1_ISSUES.md`: F18 status FIXED (row, heading, counts line 3 fixed / 14 open) plus one "Status fix" line.

Verification:
```
$ grep -n "import.meta.url).pathname" web/*.mjs; echo "grep-exit=$?"
grep-exit=1
$ ./build.sh >/dev/null; echo build=$?
build=0     (RAM .data+.bss 55744 B of 73728; pool 342876 B of 368640)
$ sh tests/run_tests.sh > /tmp/tests.log 2>&1; echo exit=$?
exit=0
$ grep -c 'STEP FAILED' /tmp/tests.log
0
$ grep -n "benchmark not enforced" /tmp/tests.log
333:   (benchmark not enforced: set T1_BENCH=1)
349:   (benchmark not enforced: set T1_BENCH=1)
$ grep -n "web tests passed\|backup web tests passed\|ALL HOST TESTS PASSED" /tmp/tests.log
5697:web tests passed
5719:backup web tests passed
5721:ALL HOST TESTS PASSED
$ grep -n "TEST_ASSERT(max_us < 80u\|TEST_ASSERT(max_us < 10u" tests/torso_phase3_test.c tests/torso_phase4_test.c
tests/torso_phase3_test.c:502:        TEST_ASSERT(max_us < 80u, ...)     (inside the T1_BENCH branch, lines 501-506)
tests/torso_phase4_test.c:153:        TEST_ASSERT(max_us < 10u, ...)     (inside the T1_BENCH branch, lines 152-156)
```

The T1_BENCH=1 path was checked by reading the source, not by running it: the asserts are the same statements as before, only moved under the getenv branch. Not run with T1_BENCH=1 on this host.

Criteria: all three ticked, each verified above. Suite run took about 20 minutes on this machine, so the full output is in /tmp/tests.log, not in this log.

Reviewer note: the F18 "Phase 3 passes apart from the timing test (F18)" sentence at T1_ISSUES.md ~line 322 is an older status note and was left unchanged.

### 2026-11-09 review (sonnet)

Verdict: FAIL (one finding, a one-line fix).

Re-run by reviewer:
- `./build.sh`: exit 0, RAM .data+.bss 55744 of 73728, pool 342876 of 368640.
- `sh tests/run_tests.sh` (full, T1_BENCH unset): `ALL HOST TESTS PASSED`, exit=0, `STEP FAILED` count 0. Log: /tmp/review_tests.log.
- `node web/test_web.mjs` and `node web/test_backup.mjs`: both pass.
- `grep -rn "import.meta.url).pathname" web`: nothing. Only `location.pathname` in web/editor.html:2747 remains, which is a browser URL, not import.meta, so it is out of scope.
- Phase 3 and 4 compiled directly. With T1_BENCH unset: exit 0 and the "(benchmark not enforced: set T1_BENCH=1)" line prints. With T1_BENCH=1: exit 0, the assert is live and passes (phase 3 max 12 us of 80; phase 4 max 1 us of 10). Both `<stdlib.h>` includes are present.
- Diff matches the Deliverable. No other firmware or test change.

Finding:
1. docs/T1_ISSUES.md:322 still reads "Unit tests: four of five suites pass; Phase 3 passes apart from the timing test (F18)." F18 is now FIXED and the suite is green, so this sentence is stale and contradicts the F18 status. The executor left it on purpose, but the task owner asked for it to be fixed. Fix: change it to "Unit tests: all suites pass on the host (F18 fixed; wall-clock benchmarks run only with T1_BENCH=1)." or delete the sentence, then re-run the review.

Checklist for the next executor:
- [ ] Fix docs/T1_ISSUES.md line 322 as above.
- [ ] No other change is needed. Re-verify with `grep -n "F18" docs/T1_ISSUES.md` and `sh tests/run_tests.sh` if time allows.

Criteria and verification otherwise confirmed by the reviewer. Gate: none.

### 2026-11-09 executor (haiku-5.5) — review fix

Reviewer finding: docs/T1_ISSUES.md line 322 said "four of five suites pass; Phase 3 passes apart from the timing test (F18)", which contradicts F18 FIXED. Changed that single sentence to:
"all suites pass on the host, including the Phase 3 timing test (F18 FIXED; wall-clock benchmarks are enforced only with `T1_BENCH=1`)."

Verification (per task owner instruction, no full test rerun):
$ grep -n "Unit tests:" docs/T1_ISSUES.md
322:- **Unit tests:** all suites pass on the host, including the Phase 3 timing test (F18 FIXED; wall-clock benchmarks are enforced only with `T1_BENCH=1`).
