import sys, csv; sys.path.insert(0, "/home/claude/fm1/docs/design/ui_review/critique_v7/scripts")
from lib import *
rows = [r for r in csv.reader(open("/home/claude/fm1/docs/design/knob_map.tsv"), delimiter="\t")][1:]
perf = [r for r in rows if r[0] == "PERF" and r[1] in ("ACID", "EUCLID", "SNAKE", "STAGE", "ARP", "PATTERNS")]
out = []; P = out.append
P("")
P("## 5f. labels drawn from el.KN versus knob_map.tsv, all 6 algorithms x 8 knobs (el.KN lookup is KN.get(algo, KN['ACID']); EUCLID has one row)")
mm = 0; tot = 0; miss = []
for r in perf:
    algo, k, lab = r[1], r[2], r[3]
    kn = el.KN.get(algo, el.KN["ACID"]); row = 0 if k[0] == "K" else 1; j = int(k[1]) - 1
    got = kn[row][j][0] if row < len(kn) else None
    tot += 1
    if got != lab:
        mm += 1; miss.append("%s %s: drawn %s, TSV %s" % (algo, k, got, lab))
P("tiles = %d; label differs or no data = %d; matches = %d" % (tot, mm, tot - mm))
P("per algorithm mismatches: " + ", ".join("%s %d" % (a, sum(1 for m in miss if m.startswith(a + " "))) for a in ("ACID", "EUCLID", "SNAKE", "STAGE", "ARP", "PATTERNS")))
for m in miss: P("- " + m)
P("")
P("knob_turning (v5) always reads kn[0][k] (row 1 labels) and draws 'K%d <label>' even when ROW 2 is live (S1-S4); v7.screen passes the same kn for both")
open("/home/claude/fm1/docs/design/ui_review/critique_v7/scripts/s5b_out.md", "w").write("\n".join(out))
print("\n".join(out))
