# SPDX-License-Identifier: GPL-3.0-only (run from repo root: .venv/bin/python <this>)
"""Pixel diff of the persistent parts: each note_X.png against note_X_perf.png = the same machine state drawn by the PERF page code (ui_sq.c, uisq_page = UI_PG_PERF).
Also note_X.png against the build/ui_sq/note_X.png the test wrote (font tables regenerated after that file was written)."""
import sys; sys.path.insert(0, "docs/design/ui_review/critique_note_edit/scripts")
from ne_lib import *
REG = [("header left (transport, tempo, chips, algo, MIDI)  x0-103 y0-21", 0, 0, 104, 22), ("message slot x104-239 y0-21", 104, 0, 240, 22), ("knob tiles y22-76", 0, 22, 240, 77), ("status line y77-91", 0, 77, 240, 92),
       ("music row y92-145", 0, 92, 240, 146), ("buttons y146-181", 0, 146, 240, 182), ("keybed y182-220", 0, 182, 240, 221), ("footer y221-239", 0, 221, 240, 240)]
def clusters(pts):
    S = set(pts); out = []
    while S:
        st = [S.pop()]; comp = list(st)
        while st:
            x, y = st.pop()
            for dx in (-1, 0, 1):
                for dy in (-1, 0, 1):
                    q = (x + dx, y + dy)
                    if q in S: S.remove(q); st.append(q); comp.append(q)
        xs = [p[0] for p in comp]; ys = [p[1] for p in comp]
        out.append((min(xs), min(ys), max(xs), max(ys), len(comp)))
    return sorted(out, key=lambda c: (c[1], c[0]))
def diffpts(a, b, box):
    pts = []
    for y in range(box[1], box[3]):
        for x in range(box[0], box[2]):
            if a.getpixel((x, y)) != b.getpixel((x, y)): pts.append((x, y))
    return pts
for n in RENDERS:
    a = img(n); b = img(n + "_perf")
    tot = diffpts(a, b, (0, 0, 240, 240))
    print("\n### %s vs %s (same state): %d of 57600 pixels differ" % (n, n + "_perf", len(tot)))
    print("| region | area px | differing px | % of region | clusters (x0,y0..x1,y1 : px) |\n|---|---|---|---|---|")
    for name, x0, y0, x1, y1 in REG:
        p = diffpts(a, b, (x0, y0, x1, y1)); area = (x1 - x0) * (y1 - y0)
        cl = clusters(p)
        print("| %s | %d | %d | %.1f | %s |" % (name, area, len(p), 100.0 * len(p) / area, "; ".join("%d,%d..%d,%d:%d" % c for c in cl[:12]) + (" ..." if len(cl) > 12 else "") if p else "identical"))
print("\n### note_X.png (this evidence's re-render with the current build/gen) vs build/ui_sq/note_X.png (written by tests/ui_sq_note_test.c at 08:39; build/gen/ui_fonts.h was regenerated 08:40)")
for n in RENDERS:
    a = img(n); b = Image.open(os.path.join("build", "ui_sq", n + ".png")).convert("RGB")
    p = diffpts(a, b, (0, 0, 240, 240)); cl = clusters(p)
    byreg = [(name.split("  ")[0].split(" x")[0], len(diffpts(a, b, (x0, y0, x1, y1)))) for name, x0, y0, x1, y1 in REG]
    print("- %s: %d px differ; by region %s" % (n, len(p), ", ".join("%s %d" % r for r in byreg if r[1])))
    ink = []
    for t in vtexts(n):
        x0, y0, x1, y1 = t["ink"]
        if any(x0 <= x < x1 and y0 <= y < y1 for x, y in p): ink.append(t["s"])
    outside = [q for q in p if not any(t["ink"][0] <= q[0] < t["ink"][2] and t["ink"][1] <= q[1] < t["ink"][3] for t in vtexts(n))]
    print("  differing pixels inside a text ink box: %d of %d; outside any text ink box: %d" % (len(p) - len(outside), len(p), len(outside)))
