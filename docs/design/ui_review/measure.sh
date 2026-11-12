#!/bin/sh
# measure.sh: rebuild the UI-review measurements. Run from the repo root. Needs build/gen/ui_tables.h and build/gen/ui_fonts.h (already generated).
# Does not touch firmware or TSV sources; builds into a temp dir and writes raw TSVs to $1 (default build/ui_review).
set -e
OUT=${1:-build/ui_review}; T=$(mktemp -d); mkdir -p "$OUT"
D=docs/design/ui_review
# the test's stand-in ACID row uses an identity knob_morph; the shipped row (sq_algo_acid.c) is { 0, 5, 2, 3, 4, 1, 6, 7 }: use the shipped one so ACID values match the label of their knob
sed -n '1,/include "..\/firmware\/src\/ui_sq.c"/p' tests/ui_sq_test.c | sed '$d' | sed '/\.name = "ACID"/s/\.knob_morph = ID8/.knob_morph = { 0, 5, 2, 3, 4, 1, 6, 7 }/' > "$T/prefix.c"
grep -q 'knob_morph = { 0, 5, 2, 3, 4, 1, 6, 7 }' "$T/prefix.c"
cat "$T/prefix.c" "$D/measure_body.c" > "$T/measure_all.c"
cc -std=gnu11 -O1 -w -Ibuild/gen -Ifirmware/src -Itests -I"$D" -include "$D/mh_decl.h" \
  '-DGFX_HOOK_TEXT(a,b,c,d,s,f)=mh_text(a,b,c,d,s,f)' '-DGFX_HOOK_INK(i,b)=mh_ink(i,b)' '-DGFX_HOOK_BLIT(x,y,r)=mh_blit(x,y)' \
  '-DGFX_HOOK_BEGIN()=((void)0)' '-DGFX_HOOK_PIXELS(n)=((void)0)' '-DGFX_HOOK_PEN(y,f)=mh_pen(y,f)' '-DGFX_HOOK_ALIGN(a,b,c,d,m,t)=((void)0)' \
  '-DGFX_HOOK_ITEM(a,b,c,d)=((void)0)' '-DGFX_HOOK_ICON(x,y,s,i)=((void)0)' \
  -o "$T/measure" "$T/measure_all.c" -lm
"$T/measure" "$OUT"
rm -rf "$T"
