# SPDX-License-Identifier: GPL-3.0-only
"""evidence_b_sweep.py: EVIDENCE_B_v3.md 6(e) knob-cell contact sheets (per algorithm: min / max / longest-name value, ROW 1 and ROW 2) and
6(j) MAP renders with every algorithm context. Writes docs/design/ui_review/critique_v3/edge/e_knobs_ALGO(.png|_4x.png) and j_map_ALGO_rowN.
Run from the repo root:  .venv/bin/python docs/design/ui_review/critique_v3/evidence_b_sweep.py"""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from evidence_b_lib import *
from PIL import Image
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "edge")
mock.palette_set("GREY")
ALG = ("ACID", "EUCLID", "SNAKE", "STAGE", "ARP", "PATTERNS")
ink = lambda f, s: (lambda b: b[2] - b[0])(text_ink(f, s))
for a in range(6):
    sets = {"min": [], "max": [], "long": []}
    for k in range(8):
        lab, lo, hi, en = P.KNOBS[a][k]
        sets["min"].append(lo); sets["max"].append(hi)
        sets["long"].append(max(range(lo, hi + 1), key=lambda v: (ink(P.knob_view(a, k, v)[2], P.knob_view(a, k, v)[1]), ink(AF_M, P.knob_view(a, k, v)[0]))))
    strips = []
    for row in (0, 1):
        for nm in ("min", "max", "long"):
            s = scene("glance_playing"); s["channels"][0]["algo"] = a; s["row"] = row; s["knobs"] = sets[nm]
            sc, w, recs = render(s, "k")
            if w: print("WARN", ALG[a], row, nm, w)
            strips.append(sc.to_image().crop((0, 144, 240, 220)))
    sheet = Image.new("RGB", (240, 76 * 6), (255, 0, 255))
    for i, st in enumerate(strips): sheet.paste(st, (0, 76 * i))
    sheet.save(os.path.join(OUT, "e_knobs_%s.png" % ALG[a]))
    sheet.resize((960, 76 * 6 * 4), Image.NEAREST).save(os.path.join(OUT, "e_knobs_%s_4x.png" % ALG[a]))
    print("sheet", ALG[a], "strips: ROW1 min,max,long then ROW2 min,max,long (each 240x76 = y144..220 of the screen)")
# (j) MAP per algorithm and row
print("--- MAP")
for a in range(6):
    for row in (0, 1):
        s = scene("map"); s["channels"][0]["algo"] = a; s["row"] = row; s["sel"] = 0
        sc, w, recs = render(s, "m")
        nm = "j_map_%s_row%d" % (ALG[a], row + 1)
        save(sc, OUT, nm)
        knob = [(r["role"], r["s"], r["ink"]) for r in recs if r["role"].startswith("map knob")]
        right = max(r["ink"][2] for r in recs); 
        # overlaps among map knob texts (row1 label vs row2 label on the same line)
        worst = None
        for y in range(4):
            r1 = [r for r in recs if r["role"] == "map knob row1"][y]; r2 = [r for r in recs if r["role"] == "map knob row2"][y]
            gap = r2["ink"][0] - r1["ink"][2]
            worst = gap if worst is None else min(worst, gap)
            print("  %-8s row%d K%d  row1 `%s` ink %d..%d  row2 `%s` ink %d..%d  gap %d  row2 right edge %d" % (ALG[a], row + 1, y + 1, r1["s"], r1["ink"][0], r1["ink"][2], r2["s"], r2["ink"][0], r2["ink"][2], gap, r2["ink"][2]))
        print("RENDER %s warnings: %s min gap row1->row2 label %d" % (nm, w or "none", worst))
