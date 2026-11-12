# SPDX-License-Identifier: GPL-3.0-only (run from repo root: .venv/bin/python <this>)
"""Grid / band / alignment facts from the op log of the real draw code + the final frame. Spec: SCREEN_SPEC_V2 s2 (16 columns of 15 px = the white keys,
4 x 60 knob tiles, 6 x 40 buttons, bands by y)."""
import sys; sys.path.insert(0, "docs/design/ui_review/critique_note_edit/scripts")
from ne_lib import *
BANDS = [("header", 0, 21), ("knob tiles", 24, 76), ("status line", 77, 91), ("music row", 92, 141), ("buttons", 146, 180), ("keybed", 186, 220), ("footer", 221, 239)]
def band(y):
    for n, a, b in BANDS:
        if a <= y <= b: return n
    return "gap"
def _rgb(c): return (c >> 11 & 31) * 255 / 31.0, (c >> 5 & 63) * 255 / 63.0, (c & 31) * 255 / 31.0
def lit(name, t, thr=0.04):
    """actual lit extents of a text: pixels in its ink box that are the text's own colour or an alpha blend of fg over the stated bg (a pixel of any other colour,
    e.g. the BG row under a tile, is not the text)"""
    im = img(name); x0, y0, x1, y1 = t["ink"]; xs = []; ys = []
    F = _rgb(t["fg"]); B = _rgb(t["bg"])
    for y in range(max(0, y0), min(240, y1)):
        for x in range(max(0, x0), min(240, x1)):
            c = px565(im, x, y)
            if c == t["bg"] or c < 0: continue
            C = _rgb(c); al = [(C[k] - B[k]) / (F[k] - B[k]) for k in range(3) if abs(F[k] - B[k]) > 40]
            if al and min(al) > thr and max(al) - min(al) < 0.12 and max(al) < 1.08: xs.append(x); ys.append(y)
    return (min(xs), min(ys), max(xs), max(ys)) if xs else None
for n in RENDERS + EXTRA:
    O = ops(n); T = vtexts(n)
    print("\n### %s" % n)
    print("\n#### texts: band, lit pixel extents (x0,y0..x1,y1 inclusive), column index of the ink left edge (15 px columns), offset into that column, distance from the right edge")
    print("| string | font | band(by lit y0..y1) | lit box (any blend) | strong box (alpha>=0.5) | col (x0//15) | x0 mod 15 | right margin to x=239 | crosses y=120? | outside its band? |\n|---|---|---|---|---|---|---|---|---|---|")
    for t in T:
        L = lit(n, t)
        if not L: print("| `%s` | %s | - | no lit pixel | | | | | |" % (t["s"], t["f"])); continue
        S5 = lit(n, t, 0.5) or L
        b0, b1 = band(L[1]), band(L[3])
        print("| `%s` | %s | %s%s | %d,%d..%d,%d | %d,%d..%d,%d | %d | %d | %d | %s | %s |" % (t["s"], t["f"], b0, "" if b0 == b1 else " -> " + b1, *L, *S5, L[0] // 15, L[0] % 15, 239 - L[2], "YES" if L[1] < 120 <= L[3] else "no", "" if b0 == b1 else "faint blend row outside the band"))
    # overlaps between visible texts
    ov = []
    for i in range(len(T)):
        for j in range(i + 1, len(T)):
            a, b = T[i]["ink"], T[j]["ink"]
            if a[0] < b[2] and b[0] < a[2] and a[1] < b[3] and b[1] < a[3] and T[i]["s"] + "@%d" % T[i]["x"] != T[j]["s"] + "@%d" % T[j]["x"]:
                ov.append("`%s`@%d,%d x `%s`@%d,%d" % (T[i]["s"], T[i]["x"], T[i]["y"], T[j]["s"], T[j]["x"], T[j]["y"]))
    print("\ntext ink-box overlaps (bitmap boxes): %s" % ("; ".join(ov) or "none"))
    # horizontal gaps between neighbouring texts on the same row (ink boxes sharing rows), gap < 4
    tight = []
    for i in range(len(T)):
        for j in range(len(T)):
            a, b = T[i]["ink"], T[j]["ink"]
            if i != j and min(a[3], b[3]) - max(a[1], b[1]) > 3 and b[0] >= a[2]:
                g = b[0] - a[2]
                if g < 6: tight.append("`%s` -> `%s` gap %d px" % (T[i]["s"], T[j]["s"], g))
    print("neighbouring text gaps < 6 px on shared rows: %s" % ("; ".join(tight) or "none"))
    # elements crossing y=120 (drawn by both strips)
    cross = [o for o in O if o["op"] in "RF" and o["y"] < 120 < o["y"] + o["h"]]
    print("rect/frame ops crossing y=120 (drawn in both strips): %s" % (", ".join("%s %d,%d %dx%d %s" % (o["op"], o["x"], o["y"], o["w"], o["h"], names_of(o["c"])) for o in cross) or "none"))
    print("text ops crossing y=120 by bitmap box: %s" % (", ".join("`%s`" % t["s"] for t in T if t["ink"][1] < 120 < t["ink"][3]) or "none"))
    print("text ink boxes with x1 > 239 or x0 < 0: %s" % (", ".join("`%s`" % t["s"] for t in T if t["ink"][2] > 240 or t["ink"][0] < 0) or "none"))

# ---------------- structural geometry (render-independent where the code is) ----------------
print("\n## geometry of the NOTE_EDIT-specific elements vs the grid (from the op log)")
for n in RENDERS + ["worst_w1", "worst_w2"]:
    O = ops(n); T = vtexts(n)
    print("\n### %s" % n)
    tiles = [o for o in O if o["op"] == "R" and o["h"] in (16, 36) and o["w"] == 58 and 20 <= o["y"] <= 70]
    print("tile rects (label band 58x16 / value area 58x36): " + "; ".join("%d,%d %dx%d %s" % (o["x"], o["y"], o["w"], o["h"], names_of(o["c"])) for o in tiles))
    print("| tile | text | font | tile x | tile centre x | ink centre x | offset (ink - tile centre) | text rows lit y0..y1 (any blend) | strong rows (alpha>=0.5) | position in tile (value area rows 41..76, label band 24..39) |\n|---|---|---|---|---|---|---|---|---|---|")
    for k in range(4):
        x = 1 + 60 * k
        for t in T:
            if 24 <= t["ink"][1] and t["ink"][3] <= 79 and x <= t["ink"][0] < x + 60:
                L = lit(n, t)
                if not L: continue
                S5 = lit(n, t, 0.5) or L
                print("| K%d | `%s` | %s | %d | %.1f | %.1f | %+.1f | %d..%d | %d..%d | %s |" % (k + 1, t["s"], t["f"], x, x + 29, (L[0] + L[2] + 1) / 2.0, (L[0] + L[2] + 1) / 2.0 - (x + 29), L[1], L[3], S5[1], S5[3], "label band 24..39 (bottom pad %d)" % (39 - S5[3]) if L[3] <= 39 else ("inside; bottom pad %d rows (strong), faint blend reaches row %d" % (76 - S5[3], L[3]) if 41 <= L[1] and L[3] <= 76 else ("faint blend row %d is past the tile's last row 76; strong rows end %d (pad %d)" % (L[3], S5[3], 76 - S5[3]) if L[3] > 76 else "spans"))))
    st = [t for t in T if 76 <= t["ink"][1] <= 80 or (t["ink"][1] >= 78 and t["ink"][3] <= 93)]
    print("status line items: " + "; ".join("`%s` x%d..%d y%d..%d" % (t["s"], t["ink"][0], t["ink"][2], t["ink"][1], t["ink"][3]) for t in T if t["ink"][1] >= 78 and t["ink"][3] <= 93 and t["ink"][1] >= 76))
    chips = [o for o in O if o["op"] == "R" and o["y"] == 79 and o["h"] == 13]
    print("status chips (rect x,y,w,h): " + (", ".join("%d,%d,%d,%d (x mod 15 = %d)" % (o["x"], o["y"], o["w"], o["h"], o["x"] % 15) for o in chips) or "none") + "; chip text insets (ink x0 - rect x, rect right - ink x1): " + ", ".join("`%s` %d,%d" % (t["s"], t["ink"][0] - c["x"], c["x"] + c["w"] - t["ink"][2]) for c in chips for t in T if t["ink"][1] >= 78 and c["x"] <= t["ink"][0] < c["x"] + c["w"] and t["ink"][3] <= 92))
    if chips:
        for a, b in zip(chips, chips[1:]): print("  chip gap %d..%d = %d px" % (a["x"] + a["w"], b["x"], b["x"] - a["x"] - a["w"]))
        print("  last chip right edge x=%d; legend present: %s" % (chips[-1]["x"] + chips[-1]["w"], any(t["s"] == "= SET BY YOU" for t in T)))
    leg = [o for o in O if o["op"] in "RF" and o["w"] == 6 and o["h"] == 6]
    print("legend square(s): " + (", ".join("%s %d,%d %s" % (o["op"], o["x"], o["y"], names_of(o["c"])) for o in leg) or "none"))
    fr = [o for o in O if o["op"] == "F" and o["c"] == 0xFFFF]
    print("white frames: " + ", ".join("%d,%d %dx%d" % (o["x"], o["y"], o["w"], o["h"]) for o in fr))
    for o in fr:
        col = (o["x"] + (2 if o["w"] == 17 else 1 if o["w"] == 15 else 0)) 
    # frame vs columns
    if fr:
        o = [o for o in fr if o["w"] == 17]
        if o:
            o = o[0]; c = (o["x"] + 2 - 1) // 15
            print("music-row frame: outer x=%d..%d (w17), inner x=%d..%d (w15), y=%d..%d outer; column %d spans x=%d..%d, bar x=%d..%d (13 px); frame outer overhangs the column by %d px left and %d px right; the next/previous bars end/start %d px / %d px from it" % (o["x"], o["x"] + 16, o["x"] + 1, o["x"] + 15, o["y"], o["y"] + o["h"] - 1, c, 15 * c, 15 * c + 14, 15 * c + 1, 15 * c + 13, 15 * c - o["x"], o["x"] + 16 - (15 * c + 14), (o["x"]) - (15 * (c - 1) + 13) - 1, (15 * (c + 1) + 1) - (o["x"] + 16) - 1))
        k = [o for o in fr if o["w"] == 14]
        if k: print("key frame: x=%d..%d y=%d..%d (white key body x=%d..%d, 14 px, pitch 15)" % (k[0]["x"], k[0]["x"] + 13, k[0]["y"], k[0]["y"] + k[0]["h"] - 1, k[0]["x"], k[0]["x"] + 13))
    # buttons / keys: grid
    btn = [o for o in O if o["op"] == "R" and o["w"] == 38 and o["h"] == 17]
    print("button tiles: %d, x = %s, y = %s (spec 6 x 40 pitch, 2 rows pitch 18)" % (len(btn), sorted({o["x"] for o in btn}), sorted({o["y"] for o in btn})))
    wk = [o for o in O if o["op"] == "R" and o["w"] == 14 and o["h"] == 33 and o["c"] == TOK["T_SURF"]]
    print("white keys: %d, x = %s (x - 15*i = %s), y=%s h=33" % (len(wk), [o["x"] for o in wk][:16], sorted({o["x"] - 15 * i for i, o in enumerate(wk[:16])}), sorted({o["y"] for o in wk})))
    ic = [o for o in O if o["op"] == "I" and o["y"] >= 146 and o["y"] < 181]
    offs = []
    for o in ic:
        col = (o["x"] - 1) // 40; row = 0 if o["y"] < 164 else 1
        tx = 1 + 40 * col; ty = 146 + 18 * row
        offs.append("%s dx=%d(l)/%d(r) dy=%d(t)/%d(b)" % (icon_name(o["id"]).replace("SQI_", ""), o["x"] - tx, tx + 38 - (o["x"] + o["w"]), o["y"] - ty, ty + 17 - (o["y"] + o["h"])))
    print("button icon insets in their 38x17 tile: " + "; ".join(offs))
# arithmetic of the status row for every chip combination the test proves
print("\n## status line capacity arithmetic (firmware formula uisq_ne_status_layout; text_w from the mock = firmware advance)")
def sw(f, s): return text_w(FONT[f], s)
for step in (9, 64):
    for chips in (["SLIDE"], ["NO SLIDE", "PROB 50", "OCT-2"], ["NO SLIDE", "PROB 50", "OCT+2", "OFF"], ["ON", "SLIDE", "PROB 50", "OCT+2"], ["OFF", "NO SLIDE", "PROB 50", "OCT-2"]):
        x = 3 + sw("S", "STEP") + 3 + sw("M", str(step)) + 5
        for c in chips: x += sw("S", c) + 4 + 2
        lg = sw("S", "= SET BY YOU")
        print("STEP %d chips %s: end x=%d; legend needs xe+8+%d = %d <= 237? %s" % (step, chips, x, lg, x + 8 + lg, "yes (drawn)" if x + 8 + lg <= 237 else "NO (legend not drawn)"))
