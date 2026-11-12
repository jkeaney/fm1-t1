#!/bin/sh
# reproduces every number in ../EVIDENCE_B_v8.md (HI-CON palette); outputs to stdout
cd "$(dirname "$0")" || exit 1
for s in s1_repro s2_text s3_contrast s3b_nontext s4_geom s4b_slots s5_sweep s5b_sets s6_extend s7_accuracy s7b_enums s89_cov_cost s10_extra; do
  echo "##### $s"; python3 $s.py 2>&1
done
