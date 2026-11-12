# SPDX-License-Identifier: GPL-3.0-only (evidence script; repo root, .venv/bin/python)
import sys; sys.path.insert(0, "docs/design/ui_review/critique_v3_2")
from evidence_b_lib import *
CAP = {"S": 9, "M": 11, "L": 21}
GL = ["glance_idle", "glance_playing", "row2", "row3", "mutes", "stress"]
CUES = [n for n in sorted(P.STATE["scenes"]) if n.startswith("cue_") and not n.startswith("cue_table")]
TOASTS = [n for n in sorted(P.STATE["scenes"]) if n.startswith("toast_")]
ALL = {}
def get(n):
    if n not in ALL:
        sc, w, recs = render(scene(n), n); ALL[n] = (recs, w)
    return ALL[n]
def fk(r): return r["f"][-1]
def row(i, r):
    f = r["f_obj"]; ink = r["ink"]; cp = CAP[fk(r)]; mm, am = arcmin(cp)
    return "| %d | %s | `%s` | %s | %d | %dx%d | %d | %.3f | %.1f | %d,%d,%d,%d | %.2f |" % (i, r["role"], r["s"], r["f"], text_w(f, r["s"]), ink[2]-ink[0], ink[3]-ink[1], cp, mm, am, *ink, cr(r["fg"], r["bg"]))
HDR = "| # | role | string | font | advance px | ink px (w x h) | cap px | cap mm | arcmin@50cm | ink box (x0,y0,x1,y1) | fg/bg contrast |\n|---|---|---|---|---|---|---|---|---|---|---|"
print("cap_h measured:", {f: mock.measure("H", f)["cap_h"] for f in "SML"})
print("### counts per scene (glance screens)\n| scene | strings | AF_L | AF_M | AF_S | distinct roles |\n|---|---|---|---|---|---|")
for n in GL + ["stress_row2", "stress_pal_paper", "stress_pal_mono", "stress_pal_ice", "stage_total"]:
    recs, w = get(n)
    print("| %s | %d | %d | %d | %d | %d |" % (n, len(recs), *(sum(1 for r in recs if fk(r) == f) for f in "LMS"), len({r['role'] for r in recs})))
print("\nwarnings any scene:", {n: get(n)[1] for n in P.STATE["scenes"] if get(n)[1]})
for n in GL:
    recs, w = get(n)
    print("\n#### %s (roles: %s)" % (n, ", ".join(sorted({r['role'] for r in recs}))))
    print(HDR)
    for i, r in enumerate(recs, 1): print(row(i, r))
print("\n### cue line, every cue_* scene (rows with role 'cue' / 'cue chip' only)\n" + HDR)
for n in CUES:
    recs, w = get(n)
    for i, r in enumerate(recs, 1):
        if r["role"] in ("cue", "cue chip"): print(row(i, r).replace("| %d |" % i, "| %s |" % n, 1))
print("\n### toasts (rows of toast roles + the AF_S/M/L counts of the whole scene)\n" + HDR)
for n in TOASTS + ["state_add_page", "state_past_length"]:
    recs, w = get(n)
    print("<!-- %s: %d strings L%d M%d S%d -->" % (n, len(recs), *(sum(1 for r in recs if fk(r) == f) for f in "LMS")))
    for i, r in enumerate(recs, 1):
        if r["role"].startswith("toast"): print(row(i, r).replace("| %d |" % i, "| %s |" % n, 1))
for m in ("map_A", "map_A_row3", "map_B", "map_menu_countdown", "map_touch"):
    recs, w = get(m)
    print("\n### %s: %d strings, fonts %s, roles %d" % (m, len(recs), {f: sum(1 for r in recs if fk(r) == f) for f in "LMS"}, len({r['role'] for r in recs})))
    rows = {}
    for r in recs: rows.setdefault((r["role"], r["f"]), []).append(r)
    print("| role | font | count | strings | min contrast | max advance | max ink | cap mm | arcmin |\n|---|---|---|---|---|---|---|---|---|")
    for (role, f), v in rows.items():
        mm, am = arcmin(CAP[f[-1]])
        print("| %s | %s | %d | %s | %.2f | %d | %d | %.3f | %.1f |" % (role, f, len(v), ", ".join("`%s`" % x["s"] for x in v[:12]) + (" ..." if len(v) > 12 else ""), min(cr(x["fg"], x["bg"]) for x in v), max(text_w(x["f_obj"], x["s"]) for x in v), max(x["ink"][2]-x["ink"][0] for x in v), mm, am))
    if m in ("map_A", "map_B"):
        print("full list:", "; ".join("%s|%s|%s" % (r["role"], r["s"], r["f"]) for r in recs))
        # neighbour gaps < 2 px, edge distance
        ink = [(r["ink"], r["s"]) for r in recs]
        tight = []
        for i in range(len(ink)):
            for j in range(i+1, len(ink)):
                a, b = ink[i][0], ink[j][0]
                vo = min(a[3], b[3]) - max(a[1], b[1])
                if vo > 0:
                    gap = max(b[0]-a[2], a[0]-b[2]); 
                    if gap < 2 and (a[0] < b[2] and b[0] < a[2] or gap > -1) : tight.append((ink[i][1], ink[j][1], gap))
        print("ink pairs on overlapping rows with horizontal gap < 2:", tight)
        print("ink reaching x>=238 or x<=1:", [(r["s"], r["ink"]) for r in recs if r["ink"][2] >= 238 or r["ink"][0] <= 1])
