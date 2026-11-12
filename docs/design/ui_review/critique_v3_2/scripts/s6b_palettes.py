# SPDX-License-Identifier: GPL-3.0-only (evidence script; repo root, .venv/bin/python): the full lane with the playhead on step 8 on GREY / PAPER / MONO, column profiles
import sys; sys.path.insert(0, "docs/design/ui_review/critique_v3_2")
from evidence_b_lib import *
OUT = "docs/design/ui_review/critique_v3_2/edge"
W = 240
for pal in ("GREY", "PAPER", "MONO"):
    s = P.scene_lane_full(7); s["palette"] = pal
    sc, w, recs = render(s, "b"); save(sc, OUT, "b_full_ph8_%s" % pal)
    def prof(x):
        out = []; cur = None; st = 0
        for y in range(73, 143):
            c = sc.px[y * W + x]
            if c != cur:
                if cur is not None: out.append((st, y - 1 - 73, "%04X" % cur))
                st, cur = y - 73, c
        out.append((st, 69, "%04X" % cur)); return out
    print(pal, "warnings", w or "none", "BG=%04X TEXT=%04X" % (T.BG, T.TEXT))
    print("  playing column (x=112) rel-row runs:", prof(112))
    print("  other column (x=127):", prof(127))
    print("  distinct colours in the playing column / in a normal column: %d / %d" % (len({sc.px[y * W + x] for y in range(73, 143) for x in range(106, 119)}), len({sc.px[y * W + x] for y in range(73, 143) for x in range(121, 134)})))
