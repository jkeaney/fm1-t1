#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# One verification command for the Definition of Done (docs/project/PROCESS.md):
#   sh tools/check.sh
# Stages, run in order and stopping at the first failure:
#   build    ./build.sh (needs Docker, the JieLi toolchain and the AC79 SDK, see CLAUDE.md)
#   ram      the build's "RAM .data+.bss N of M" line must leave at least 4096 B of headroom
#   tests    sh tests/run_tests.sh (run after the build, it uses build/)
#   backlog  python3 tools/backlog.py check
#   docs     tools/check_docs.sh (stale wording from docs/design/forbidden_terms.txt)
# On pass it writes build/check/stamp (tree hash or DIRTY, then the RAM line).
# Full output of each stage goes to build/check/<stage>.log. The last line is CHECK PASS or
# CHECK FAIL: <stage>. Exit code 0 on pass, 1 on fail.
set -u
cd "$(dirname "$0")/.." || { echo "CHECK FAIL: setup"; exit 1; }
ROOT=$(pwd)
export AC79_SDK="${AC79_SDK:-$HOME/fw-AC79_AIoT_SDK}"
PATH="$ROOT/.venv/bin:$PATH"
export PATH
LOG="$ROOT/build/check"
mkdir -p "$LOG" || { echo "CHECK FAIL: setup"; exit 1; }

fail() {
    echo "CHECK FAIL: $1"
    exit 1
}

echo "== build (log: build/check/build.log)"
./build.sh > "$LOG/build.log" 2>&1 || fail build

echo "== ram headroom"
line=$(grep -o 'RAM .data+.bss [0-9]* B of [0-9]*' "$LOG/build.log" | tail -n 1)
[ -n "$line" ] || fail ram
set -- $line
used=$3
total=$6
echo "$line"
[ $((total - used)) -ge 4096 ] || fail ram

echo "== tests (log: build/check/tests.log)"
sh tests/run_tests.sh > "$LOG/tests.log" 2>&1 || fail tests

echo "== backlog (log: build/check/backlog.log)"
python3 tools/backlog.py check > "$LOG/backlog.log" 2>&1 || fail backlog

echo "== docs wording (tools/check_docs.sh)"
sh tools/check_docs.sh > "$LOG/docs.log" 2>&1 || { cat "$LOG/docs.log"; fail docs; }

# Stamp for the reviewer: tree hash (or DIRTY) + RAM line, so a clean pass is not rebuilt.
if [ -z "$(git status --porcelain --untracked-files=no 2>/dev/null)" ]; then
    th=$(git rev-parse 'HEAD^{tree}')
else
    th=DIRTY
fi
printf '%s\n%s\n' "$th" "$line" > "$LOG/stamp"
echo "CHECK PASS"
exit 0
