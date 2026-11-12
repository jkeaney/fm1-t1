#!/bin/sh
# builds ../EVIDENCE_B_v10.md = hand-written summary (summary.md) + the raw output of every script
# prerequisite: (cd ../../proposals/v10 && python3 ../../mock/mock.py render v10.py --out /tmp/claude-0/v10b)
cd "$(dirname "$0")"
OUT=../EVIDENCE_B_v10.md
cp summary.md $OUT
for s in s1_repro s2_text s3_contrast s4_geom s4b_arcmin s5_sweeps s5b_extra s6_extend s7_accuracy s8_coverage s9_cost; do
  printf '\n\n---\n\n<!-- scripts/%s.py -->\n' $s >> $OUT
  python3 -W ignore $s.py 2>&1 | sed 's/^\(# \)/## [\1/; ' | sed 's/^## \[# \(.*\)$/## \1  (scripts\/'$s'.py)/' >> $OUT
done
