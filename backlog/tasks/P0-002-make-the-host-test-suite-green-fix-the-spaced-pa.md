---
id: P0-002
title: Make the host test suite green: fix the spaced-path web test and gate wall-clock benchmarks
phase: P0
type: task
status: todo
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
- [ ] `grep -n "import.meta.url).pathname" web/*.mjs` prints nothing
- [ ] With `T1_BENCH` unset, `sh tests/run_tests.sh` exits 0 (after `./build.sh`)
- [ ] With `T1_BENCH=1`, the two benchmarks still assert (check the source: the TEST_ASSERT is still there, inside the getenv branch)

## Verification
```sh
./build.sh >/dev/null && export AC79_SDK=~/fw-AC79_AIoT_SDK PATH="$PWD/.venv/bin:$PATH"
sh tests/run_tests.sh > /tmp/tests.log 2>&1; echo exit=$?     # exit=0
grep -c 'STEP FAILED' /tmp/tests.log                            # 0
```

## Out of scope
- Any other test or firmware change.

## Log
