#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Usage: sh tests/design/width_probe.sh [file.tsv]   (default tests/design/widths.tsv; "-" = stdin)
# Builds the probe from the real gfx.c text_w and build/gen/ui_fonts.h (run ./build.sh once so build/gen exists).
set -e
rc=0
R=$(cd "$(dirname "$0")/../.." && pwd)
T=$(mktemp -d)
sed -n '/^static int32_t glyph_at/,/^static int32_t text_run/p' "$R/firmware/src/gfx.c" | sed '$d' > "$T/text_w.inc"
cc -O1 -I"$T" -I"$R/build/gen" -o "$T/p" "$R/tests/design/width_probe.c"
F=${1:-$R/tests/design/widths.tsv}
if [ "$F" = "-" ]; then "$T/p" || rc=$?; else "$T/p" < "$F" || rc=$?; fi
rm -rf "$T"
exit $rc
