#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# P2-008: build and test the algorithm prototypes in docs/design/algorithms/<name>/ against common/.
#   sh tests/design/run_protos.sh              check everything, print PROTOS PASS
#   GOLDEN_UPDATE=1 sh tests/design/run_protos.sh   accept an intended change of an algorithm's golden.txt
# Stages: header hygiene, -fsyntax-only on every common/ and <algo>/ file, primitives, HARMONY (H-GOLDEN against
# harmony_proto.py), then per algorithm: SPEC table == proto.c table, matrix + purity + table tests, golden diff, a second
# run that must be byte-identical, and (ACID) the independent Python re-implementation.
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
ALG="$ROOT/docs/design/algorithms"
CMN="$ALG/common"
T=$(mktemp -d "${TMPDIR:-/tmp}/protos.XXXXXX")
trap 'rm -rf "$T"' EXIT
CC=${CC:-cc}
CF="-std=c11 -Wall -Wextra -Werror -O1 -fno-strict-aliasing"
fail() { echo "PROTOS FAIL: $1" >&2; exit 1; }

# 1. only stdint / stddef / stdbool / stdarg in the firmware-bound files
for f in "$CMN"/*.c "$CMN"/*.h "$ALG"/*/proto.c; do
    bad=$(grep -n '#include *<' "$f" | grep -v -e '<stdint.h>' -e '<stddef.h>' -e '<stdbool.h>' || true)
    [ -z "$bad" ] || fail "hosted include in $f: $bad"
done

# 2. syntax check of every common/ and algorithm file (headers as headers, sources as sources)
for f in "$CMN"/*.h; do $CC $CF -fsyntax-only -x c-header "$f" || fail "syntax $f"; done
for f in "$CMN"/*.c "$ALG"/*/proto.c; do $CC $CF -fsyntax-only "$f" || fail "syntax $f"; done

# 3. primitives
$CC $CF -I"$CMN" -o "$T/prims" "$HERE/prims_test.c" "$CMN/sq_prims.c" || fail "build prims_test"
"$T/prims" || fail "primitive tests"

# 4. HARMONY: C against Python, every table of HARMONY.md 3.1 3.2 3.3 4.2 6.1 (H-GOLDEN)
python3 "$HERE/harmony_proto.py" --check > /dev/null || fail "harmony_proto.py --check"
python3 "$HERE/harmony_proto.py" --dump > "$T/harm_py.txt" || fail "harmony_proto.py --dump"
$CC $CF -I"$CMN" -o "$T/harm" "$HERE/harm_test.c" "$CMN/sq_prims.c" "$CMN/harm.c" || fail "build harm_test"
"$T/harm" > "$T/harm_c.txt" || fail "harm tests"
cmp -s "$T/harm_c.txt" "$T/harm_py.txt" || { diff "$T/harm_c.txt" "$T/harm_py.txt" | head -20; fail "H-GOLDEN: harm.c differs from harmony_proto.py"; }
echo "H-GOLDEN: $(wc -l < "$T/harm_c.txt" | tr -d ' ') table lines equal harmony_proto.py"

# 5. each algorithm folder that has a proto.c
n=0
for d in "$ALG"/*/; do
    name=$(basename "$d")
    [ -f "$d/proto.c" ] || continue
    [ "$name" = common ] && continue
    up=$(echo "$name" | tr '[:lower:]' '[:upper:]')
    # the AlgoDesc table of SPEC.md is the one proto.c reads
    sed -n '/ALGODESC-BEGIN/,/ALGODESC-END/p' "$d/SPEC.md" | grep -v -e 'ALGODESC-BEGIN' -e 'ALGODESC-END' > "$T/spec_tab.txt"
    sed -n '/ALGODESC-BEGIN/,/ALGODESC-END/p' "$d/proto.c" | grep -v -e 'ALGODESC-BEGIN' -e 'ALGODESC-END' > "$T/proto_tab.txt"
    [ -s "$T/proto_tab.txt" ] || fail "$name: no ALGODESC block in proto.c"
    cmp -s "$T/spec_tab.txt" "$T/proto_tab.txt" || { diff "$T/spec_tab.txt" "$T/proto_tab.txt" | head; fail "$name: SPEC.md AlgoDesc table differs from proto.c"; }
    grep -q 'TBD' "$d/SPEC.md" && fail "$name: SPEC.md contains TBD"
    $CC $CF -I"$CMN" -DPROTO_SYM=ALGO_$up -DPROTO_ID=SQ_ALGO_$up -o "$T/$name" "$HERE/proto_main.c" "$CMN/sq_prims.c" "$CMN/harm.c" "$d/proto.c" || fail "build $name"
    "$T/$name" > "$T/$name.1" || fail "$name tests"
    "$T/$name" > "$T/$name.2" 2>/dev/null || fail "$name second run"
    cmp -s "$T/$name.1" "$T/$name.2" || fail "$name: two runs differ (not deterministic)"
    if [ -n "$GOLDEN_UPDATE" ]; then cp "$T/$name.1" "$d/golden.txt"; echo "$name: golden.txt updated"; fi
    [ -f "$d/golden.txt" ] || fail "$name: no golden.txt (run with GOLDEN_UPDATE=1)"
    cmp -s "$T/$name.1" "$d/golden.txt" || { diff "$d/golden.txt" "$T/$name.1" | head -20; fail "$name: output differs from golden.txt"; }
    [ -f "$HERE/${name}_ref.py" ] && { python3 "$HERE/${name}_ref.py" "$d/golden.txt" || fail "$name: independent reference differs"; }
    echo "$name: $(grep -c '^## ' "$d/golden.txt") golden sections, golden.txt equal, two runs byte-identical"
    n=$((n + 1))
done
[ "$n" -ge 1 ] || fail "no algorithm prototype found"
echo "PROTOS PASS"
