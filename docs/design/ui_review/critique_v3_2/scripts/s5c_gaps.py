# SPDX-License-Identifier: GPL-3.0-only (evidence script; repo root, .venv/bin/python): gaps between neighbouring cells, measured on pixels
import sys; sys.path.insert(0, "docs/design/ui_review/critique_v3_2")
from evidence_b_lib import *
W = 240
def run(scn):
    sc, w, recs = render(scene(scn), scn); return sc, T.BG
def runs(vals, pred):
    out, st = [], None
    for i, v in enumerate(list(vals) + [None]):
        ok = v is not None and pred(v)
        if ok and st is None: st = i
        if not ok and st is not None: out.append((st, i - 1)); st = None
    return out
sc, bg = run("stress")
nb = lambda v: v != bg
print("horizontal runs (first..last px) and the gap to the next run, per row, scene stress:")
for name, y in (("page boxes row y=5", 5), ("chan tiles row y=26 (row of tile top edge+1)", 26), ("lane row y=73", 73), ("knob tiles row y=146", 146), ("ROW cells row y=206", 206), ("cue chip row y=222", 222), ("activity bar row y=65 (scene glance_playing)", 65)):
    s2, b2 = (sc, bg) if "glance" not in name else run("glance_playing")
    r = runs([s2.px[y * W + x] for x in range(W)], lambda v, b2=b2: v != b2)
    gaps = [r[i + 1][0] - r[i][1] - 1 for i in range(len(r) - 1)]
    print(" %-48s runs %s | gaps %s | left margin %d right margin %d" % (name, r[:20], gaps, r[0][0], W - 1 - r[-1][1]))
print("\nvertical bands in glance_playing (rows with any non-BG px) and the gap between neighbours:")
sc, bg = run("glance_playing")
rows = [any(sc.px[y * W + x] != bg for x in range(W)) for y in range(240)]
r = runs(rows, lambda v: v)
print(" spans", r, "gaps", [r[i + 1][0] - r[i][1] - 1 for i in range(len(r) - 1)], "top margin", r[0][0], "bottom margin", 239 - r[-1][1])
print("\nvertical gaps at specific columns (glance_playing):")
def col(x): return runs([sc.px[y * W + x] for y in range(240)], lambda v: v != bg)
print(" x=2 (tile col 1) :", col(2), "gaps", [col(2)[i + 1][0] - col(2)[i][1] - 1 for i in range(len(col(2)) - 1)])
print(" x=230 (page box 4/ tile 4 / lane 16):", col(230))
sc, bg = run("stress"); print(" stress x=230:", col(230))
sc, bg = run("toast_dice_notes")
print("toast_dice_notes x=2:", col(2))
rows = [sc.px[y * W + 100] for y in range(190, 240)]
print(" toast column x=100, y190..239 colours:", [(190 + a, 190 + b, "%04X" % rows[a]) for a, b in runs(rows, lambda v: True)][:0] or [(y, "%04X" % c) for y, c in zip(range(190, 240), rows) if y in (194,195,196,197,198,199,235,236,237)])
rowc = [sc.px[y * W + 230] for y in range(190, 240)]
print(" toast x=230 (right empty part):", [(y, "%04X" % c) for y, c in zip(range(190, 240), rowc) if y in (194,195,196,197,198,199,235,236,237,239)])
print(" toast slab x extent at y=210:", runs([sc.px[210 * W + x] for x in range(W)], lambda v: v == T.SURF))
