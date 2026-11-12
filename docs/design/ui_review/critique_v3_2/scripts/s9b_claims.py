# SPDX-License-Identifier: GPL-3.0-only (evidence script; repo root, .venv/bin/python): spot checks of NOTES.md numeric claims
import sys; sys.path.insert(0, "docs/design/ui_review/critique_v3_2")
from evidence_b_lib import *
sc, w, r = render(scene("cue_idle_playing"), "x")
print("non-BG px y220..239, cue_idle_playing:", sum(1 for y in range(220, 240) for x in range(240) if sc.px[y * 240 + x] != T.BG), "(NOTES claim 1269)")
sc, w, r = render(scene("lane_offpage"), "x"); hd = P.pal({})[0]
print("HEAD px in lane, lane_offpage:", sum(1 for y in range(72, 144) for x in range(240) if sc.px[y * 240 + x] == hd), "(NOTES claim 343)")
sc, w, r = render(scene("glance_playing"), "x")
ys = [y for y in range(145, 204) if sc.px[y * 240 + 30] in (T.LANE, T.TEXT)]; print("meter rows (LANE/TEXT) at x=30 in knob tile 1:", ys[0], "..", ys[-1], "(NOTES: meters end at y 195)")
for n in ("glance_playing", "glance_idle", "state_wait", "state_rec_stopped", "state_rec_playing", "stress"):
    sc, w, r = render(scene(n), "x")
    xs = [(x, y) for y in range(0, 24) for x in range(0, 16) if sc.px[y * 240 + x] != T.BG]
    x0 = min(a for a, b in xs); x1 = max(a for a, b in xs); y0 = min(b for a, b in xs); y1 = max(b for a, b in xs)
    print("transport", n, "bbox x y w h:", (x0, y0, x1 - x0 + 1, y1 - y0 + 1), "(TSV: 1 5 13 14)")
print("state_* scenes:", len([n for n in P.STATE["scenes"] if n.startswith("state_")]), "(NOTES: 15); scenes total:", len(P.STATE["scenes"]), "(NOTES: 71)")
