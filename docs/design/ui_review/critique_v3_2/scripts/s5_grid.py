# SPDX-License-Identifier: GPL-3.0-only (evidence script; repo root, .venv/bin/python): geometry measured on the rendered pixels
import sys, csv; sys.path.insert(0, "docs/design/ui_review/critique_v3_2")
from evidence_b_lib import *
W = H = 240
SC = {}
def S(n):
    if n not in SC:
        sc, w, recs = render(scene(n), n); SC[n] = (sc, recs, mock.ux.bg if False else None)
    return SC[n]
def bgof(n):
    render(scene(n), n); return T.BG
def px(sc, x, y): return sc.px[y * W + x]
def bbox(sc, x0, y0, x1, y1, pred):
    xs, ys = [], []
    for y in range(y0, y1):
        for x in range(x0, x1):
            if pred(px(sc, x, y)): xs.append(x); ys.append(y)
    return (min(xs), min(ys), max(xs) - min(xs) + 1, max(ys) - min(ys) + 1) if xs else None
def runs(vals, pred):
    out, st = [], None
    for i, v in enumerate(vals + [None]):
        ok = v is not None and pred(v)
        if ok and st is None: st = i
        if not ok and st is not None: out.append((st, i - st)); st = None
    return out
print("## A. blit coverage (a pixel never written stays 0x0000; GREY/GREEN/... BG is non-zero)")
unwritten = {}
for n in sorted(P.STATE["scenes"]):
    sc, recs, _ = S(n); pal = P.STATE["scenes"][n].get("palette", "GREY")
    if pal in ("MONO", "NIGHT"): continue
    z = sum(1 for v in sc.px if v == 0)
    if z: unwritten[n] = z
print("scenes (excluding MONO/NIGHT whose BG is 0) with unwritten pixels:", unwritten or "none", "; blits per scene:", {S(n)[0].nblits for n in P.STATE["scenes"]})
print("\n## B. declared zone marks (the tool's `mark`) in glance_playing, and measured content bands")
sc, recs, _ = S("glance_playing"); bg = bgof("glance_playing")
for m in sc.marks:
    if m[0].startswith("Z"): print("mark", m)
rowsany = [any(px(sc, x, y) != bg for x in range(W)) for y in range(H)]
print("rows with any non-BG pixel -> spans:", [(a, a + l - 1) for a, l in runs(rowsany, lambda v: v)])
print("BG-only row spans between them:", [(a, a + l - 1) for a, l in runs(rowsany, lambda v: not v)])
print("\n## C. rectangles measured from the pixels (scene: glance_playing unless noted)")
meas = {}
def put(name, scn, bb): meas[(scn, name)] = bb
for scn in ("glance_playing", "mutes", "stress", "row3", "toast_dice_notes", "state_add_page", "toast_preset_pending", "state_first_run"):
    sc, recs, _ = S(scn); bg = bgof(scn); pal = lambda v: v != bg
    for i in range(4):
        put("chan tile %d" % (i + 1), scn, bbox(sc, 60 * i, 24, 60 * i + 60, 64, pal))
    head = P.pal({})[0]
    for i in range(4):
        put("activity bar %d" % (i + 1), scn, bbox(sc, 60 * i, 64, 60 * i + 60, 68, lambda v: v == head))
    for k in range(4):
        put("knob tile %d" % (k + 1), scn, bbox(sc, 60 * k, 144, 60 * k + 60, 196 if scn.startswith("toast") else 204, pal))
    row = [px(sc, x, 73) for x in range(W)]
    for n, (a, l) in enumerate(runs(row, pal)):
        put("lane col %d" % (n + 1), scn, (a, 73, l, None))
    for n in range(16):
        bb = bbox(sc, 15 * n + 6, 72, 15 * n + 7, 144, pal)
        meas[(scn, "lane col %d" % (n + 1))] = (meas.get((scn, "lane col %d" % (n + 1)), (None,) * 4)[0], bb[1] if bb else None, meas.get((scn, "lane col %d" % (n + 1)), (None, None, None))[2], bb[3] if bb else None)
    for r in range(3):
        put("row cell %d" % (r + 1), scn, bbox(sc, 60 * r, 204, 60 * r + 60, 220, pal) if not scn.startswith("toast") else None)
    for k in range(7):
        pass
    put("transport", scn, bbox(sc, 0, 0, 16, 24, pal))
    put("toast slab", scn, bbox(sc, 0, 190, 240, 240, lambda v: v == T.SURF) if scn.startswith("toast") else None)
    # page boxes: runs at row 5 for x >= 180; bars at row 21
    r5 = [px(sc, x, 5) for x in range(W)]; r21 = [px(sc, x, 21) for x in range(W)]
    pb = [(a, l) for a, l in runs(r5, pal) if a >= 150]
    for n, (a, l) in enumerate(pb):
        bb = bbox(sc, a, 0, a + l, 20, pal)
        put("page box (run %d)" % (n + 1), scn, bb)
    pbar = [(a, l) for a, l in runs(r21, pal) if a >= 150]
    for n, (a, l) in enumerate(pbar):
        put("page bar (run %d)" % (n + 1), scn, bbox(sc, a, 20, a + l, 24, pal))
rows = list(csv.DictReader(open("docs/design/ui_review/proposals/v3.2/geometry_v3_2.tsv"), delimiter="\t"))
print("\n## D. TSV rows checked by edge test (inside edge pixels differ from BG on all four sides; outside ring equals BG), scene as in the TSV")
bad = []; skipped = []; ok = 0
for r in rows:
    scn = r["scene"]; kind = r["kind"]; nm = r["name"]
    x, y, w, h = int(r["x"]), int(r["y"]), int(r["w"]), int(r["h"])
    if kind in ("text", "cue"): skipped.append((scn, nm, "text cell: no drawn edge")); continue
    sc, recs, _ = S(scn); bg = bgof(scn)
    def inside(i, j): return px(sc, i, j) != bg
    def outside(i, j): return not (0 <= i < W and 0 <= j < H) or px(sc, i, j) == bg
    ins = all(inside(x + w // 2, y), ) if False else None
    sides = {"top": (inside(x + w // 2, y), outside(x + w // 2, y - 1)), "bottom": (inside(x + w // 2, y + h - 1), outside(x + w // 2, y + h)),
             "left": (inside(x, y + h // 2), outside(x - 1, y + h // 2)), "right": (inside(x + w - 1, y + h // 2), outside(x + w, y + h // 2))}
    problems = [s_ + (":inside-bg" if not a else ":outside-not-bg") for s_, (a, b) in sides.items() if not a] + [s_ + ":outside-not-bg" for s_, (a, b) in sides.items() if a and not b]
    if kind == "keybed" or kind == "bar" and False: pass
    if problems: bad.append((scn, nm, kind, (x, y, w, h), problems))
    else: ok += 1
print("TSV rows %d: edge-test pass %d, skipped (text cells) %d, fail %d" % (len(rows), ok, len(skipped), len(bad)))
for b in bad: print("FAIL", b)
print("\n## E. measured rect vs TSV rect (same scene, same name)")
tsv = {(r["scene"], r["name"]): (int(r["x"]), int(r["y"]), int(r["w"]), int(r["h"])) for r in rows}
diffs = 0
for (scn, nm), t in sorted(tsv.items()):
    m = meas.get((scn, nm))
    if m is None: continue
    if None in m: m = tuple(a if a is not None else b for a, b in zip(m, t)) if False else m
    cmp = [(a, b) for a, b in zip(m, t) if a is not None and a != b]
    print(("DIFF " if cmp else "same ") + scn, nm, "measured", m, "tsv", t)
    diffs += bool(cmp)
print("differences:", diffs)
print("\n## F. all measured rectangles (name: x,y,w,h) for the glance scenes -- the module test: x-1 mod 15, w+2 mod 15, y-1 mod 4, h+2 mod 4 (flush bars/cue: x mod 15, y mod 4)")
for (scn, nm), m in sorted(meas.items()):
    if scn in ("glance_playing", "stress", "mutes", "row3", "toast_dice_notes") and m and None not in m:
        x, y, w, h = m
        print("%-18s %-22s x%3d y%3d w%3d h%3d | xr %2d wr %2d yr %d hr %d | flush xr %2d yr %d" % (scn, nm, x, y, w, h, (x - 1) % 15, (w + 2) % 15, (y - 1) % 4, (h + 2) % 4, x % 15, y % 4))
