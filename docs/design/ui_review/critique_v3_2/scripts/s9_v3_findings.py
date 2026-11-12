# SPDX-License-Identifier: GPL-3.0-only (evidence script; repo root, .venv/bin/python): re-measure the pixel claims of EVIDENCE_B_v3 findings 7 and 8 on v3.2
import sys; sys.path.insert(0, "docs/design/ui_review/critique_v3_2")
from evidence_b_lib import *
W = 240
def lane_scene(step, ph_col, **kw):
    rows = [(0, 1, 0, 0, 0, 1)] * 16
    rows = list(rows); rows[ph_col] = step
    s = P.scene_playing(lane=P.lane_of(rows)); s["channels"][0]["p"] = ph_col + 1; s.update(kw); return s
def col_px(sc, n, y0=73, y1=143):
    return [sc.px[y * W + x] for y in range(y0, y1) for x in range(15 * n + 1, 15 * n + 14)]
def diff(a, b): return sum(1 for p, q in zip(a, b) if p != q)
print("## finding 7: step velocity 1/2/3 ON vs REST, both under the playhead (column 1) and not under it (column 3)")
for vel in (1, 2, 3, 4, 8):
    on = render(lane_scene((1, vel, 0, 0, 0, 1), 0), "a")[0]; rest = render(lane_scene((0, 1, 0, 0, 0, 1), 0), "b")[0]
    print(" under playhead: vel %d vs rest: %d differing px in the 13x70 column" % (vel, diff(col_px(on, 0), col_px(rest, 0))), end="; ")
    on = render(lane_scene((1, vel, 0, 0, 0, 1), 2), "a")[0]; rest = render(lane_scene((0, 1, 0, 0, 0, 1), 2), "b")[0]
    print("not under playhead (column 3, playhead col 3): n/a" if False else "")
for vel in (1, 2, 3):
    s1 = lane_scene((1, vel, 0, 0, 0, 1), 5); s2 = lane_scene((0, 1, 0, 0, 0, 1), 5); s1["channels"][0]["p"] = 1; s2["channels"][0]["p"] = 1   # playhead on col 1, test col 6
    a = render(s1, "a")[0]; b = render(s2, "b")[0]
    print(" NOT under playhead: vel %d vs rest, column 6: %d differing px (bar height %d px)" % (vel, diff(col_px(a, 5), col_px(b, 5)), P.bar_h(vel)))
print("\n## finding 7: slide chain under the playhead (all 16 steps slide, playhead on 8): longest BG run on the tie row (v3: 15 px)")
s = P.scene_lane_full(7); sc = render(s, "a")[0]
best = cur = 0
for x in range(240):
    cur = cur + 1 if sc.px[85 * W + x] == T.BG else 0; best = max(best, cur)
print(" longest BG run on y85:", best)
print("\n## finding 7: accent vs plain bar inside the playing column (all-BG marks): cap pixel rows vs bar pixel rows at the centre column")
s = P.scene_playing(lane=P.lane_of([(1, 100, 0, 1, 0, 1)] * 2 + [(1, 100, 0, 0, 0, 1)] * 14)); s["channels"][0]["p"] = 1; sc = render(s, "a")[0]
s2 = P.scene_playing(lane=P.lane_of([(1, 100, 0, 0, 0, 1)] * 16)); s2["channels"][0]["p"] = 1; sc2 = render(s2, "a")[0]
print(" playing column, accent ON vs accent OFF: differing px =", diff(col_px(sc, 0), col_px(sc2, 0)), "(cap triangle 13x8 = %d px of BG on HEAD)" % sum(1 for p, q in zip(col_px(sc, 0), col_px(sc2, 0)) if p != q))
print("\n## finding 8: ratchet counts 1..8 (digit min(rep,4)), differing px in the column against rep 1 and against rep 4")
cols = {}
for r in range(1, 9):
    s = P.scene_playing(lane=P.lane_of([(1, 100, 0, 0, 0, r)] * 16)); s["channels"][0]["p"] = 4; cols[r] = col_px(render(s, "a")[0], 0)
print(" vs rep 1:", {r: diff(cols[1], cols[r]) for r in range(2, 9)}, " vs rep 4:", {r: diff(cols[4], cols[r]) for r in (1, 2, 3, 5, 6, 7, 8)})
print("\n## finding 9: amber/head pixels in the lane when the playhead is on a page that is not viewed: see s6_edge_abcd.txt (d)")
