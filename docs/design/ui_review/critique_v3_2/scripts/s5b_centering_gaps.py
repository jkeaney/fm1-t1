# SPDX-License-Identifier: GPL-3.0-only (evidence script; repo root, .venv/bin/python): ink centring in cells and gaps between neighbours, from pixels
import sys; sys.path.insert(0, "docs/design/ui_review/critique_v3_2")
from evidence_b_lib import *
W = 240
CELLY = {"knob label": (145, 18), "row label": (205, 14), "page numeral": (5, 14), "mute chip": (29, 14), "cue chip": (221, 14), "cue": (221, 14), "tempo": (5, 14), "preset": (5, 14),
         "chan numeral": (29, 14), "midi slot": (29, 14)}
def cells(scn):
    sc, w, recs = render(scene(scn), scn)
    out = []
    for r in recs:
        role = r["role"]; ink = r["ink"]
        f = r["f_obj"]; cap = f.cap_h
        yc = CELLY.get(role)
        ix0, ix1 = ink[0], ink[2]
        if r["mode"] == "in" and r["w"]:
            cx0, cw = r["x"], r["w"]
            dx2 = (ix0 + ix1) - (2 * cx0 + cw)            # twice the centre offset (ink right edge exclusive in ink[2]?)
            out.append((scn, role, r["s"], r["f"], "centred", (cx0, cw), (ix0, ix1), ml_mr(ix0, ix1, cx0, cw), vert(r, yc, cap)))
        else:
            out.append((scn, role, r["s"], r["f"], r["mode"], None, (ix0, ix1), None, vert(r, yc, cap)))
    return out
def ml_mr(ix0, ix1, cx0, cw):
    # ink box = x0..x1 where x1 is the last ink column + 1 (ink[2] is exclusive per screen.texts convention: width = x1 - x0)
    return (ix0 - cx0, cx0 + cw - ix1)
def vert(r, yc, cap):
    if not yc: return None
    return (r["ycap"] - yc[0], yc[0] + yc[1] - (r["ycap"] + cap))
print("| scene | role | string | font | kind | cell x,w | ink x0..x1 | margin L / R (px) | cap margin T / B (px) |\n|---|---|---|---|---|---|---|---|---|")
seen = set()
for scn in ("glance_playing", "mutes", "stress", "row2", "row3", "toast_dice_notes", "toast_undo", "map_A", "map_B"):
    for c in cells(scn):
        k = (c[1], c[2], c[7], c[8])
        if (c[1], c[2], c[4], c[7]) in seen: continue
        seen.add((c[1], c[2], c[4], c[7]))
        if c[4] == "centred" or c[8]:
            print("| %s | %s | `%s` | %s | %s | %s | %s | %s | %s |" % (c[0], c[1], c[2], c[3], c[4], c[5], c[6], c[7], c[8]))
