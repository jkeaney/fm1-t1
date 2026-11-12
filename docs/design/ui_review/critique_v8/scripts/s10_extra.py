import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from common import *
import el
palette_set("HI-CON")
# A. AA halo: value text drawn with bg=SURF over a partly filled tile
S = scenes()
sc, cv, rec = render(S["playing"]); px = sc.px
print("A. AF_L value text when fill height < 30 (text bg arg = SURF, but fill lies under part of the glyph box)")
for t in rec:
    if t["font"] == "AF_L":
        a,b,c,d = t["ink"]; k = a // 60; x0 = k*60+1
        fills = [px[y*240+x0+1] for y in range(41, 77)]
        fc = [f for f in fills if f != T.SURF]
        fh = len(fc)
        ramp = set(mock.ramp(t["fg"], T.SURF))
        fillc = fc[0] if fc else None
        halo = 0
        for y in range(b, d):
            for x in range(a, c):
                p = px[y*240+x]
                if p in ramp and p not in (T.SURF, mod.WHITE) and fh and y >= 77 - fh: halo += 1
        print("   %-4r fill height %2d px (text bg arg %s); AA pixels computed against SURF that sit on the fill: %d" % (t["s"], fh, "FILL" if fh >= 30 else "SURF", halo))
# B. dice icon stroke widths
print("\nB. minimum stroke width (shortest horizontal/vertical run of '#') per icon")
def minrun(art):
    rows = art.strip("\n").split("\n"); w = max(map(len, rows)); rows = [r.ljust(w) for r in rows]
    best = 99
    for r in rows:
        i = 0
        while i < w:
            if r[i] == "#":
                j = i
                while j < w and r[j] == "#": j += 1
                best = min(best, j - i); i = j
            else: i += 1
    return best
def minrun2(art):
    rows = art.strip("\n").split("\n"); w = max(map(len, rows)); rows = [r.ljust(w) for r in rows]
    cols = ["".join(rows[y][x] for y in range(len(rows))) for x in range(w)]
    return minrun("\n".join(cols))
for k in ("dice_all","dice_notes","dice_accnt","dice_shape","dice_artic","dice_hits","undo","settings","home","save","play","rec_ring","rec","midi_s","x7","play_s"):
    a = icons_v8.ICONS[k]; print("   %-11s min horizontal run %d, min vertical run %d (same-row/col runs; a diagonal 1-px stroke reports 1)" % (k, minrun(a), minrun2(a)))
# C. visual angle
print("\nC. visual angle at 35 cm (30 mm panel, 8 px/mm -> 0.125 mm/px; arcmin = mm/350*3438)")
for lab, px_ in (("1 px", 1), ("2 px", 2), ("AF_S cap 9 px", 9 - 3), ("AF_M cap (11 rows listed as cap 4..11)", 7), ("AF_L cap 14", 14)):
    pass
def am(mm): return mm / 350 * 3438
print("   note: task statement 30 mm at 8 px/mm = 240 px; 1 px = %.4f mm = %.2f arcmin" % (30/240, am(30/240)))
for fnt in (AF_S, AF_M, AF_L):
    h = measure("H", fnt)
    print("   %s cap height %d px = %.2f mm = %.1f arcmin ; line height %d px" % (fnt.name, h["cap_h"], h["cap_h"]*30/240, am(h["cap_h"]*30/240), h["line_h"]))
for lab, pxs in (("2 px stroke", 2), ("13 px icon", 13), ("5 px row marker", 5), ("3 px bar slot (4 ch)", 3), ("6 px slot (2 ch)", 6), ("1 px gap", 1), ("2 px gap", 2), ("2 px playhead bar", 2), ("4x4 hold notch", 4), ("7 px mute X", 7)):
    print("   %-24s %.3f mm = %.1f arcmin" % (lab, pxs*30/240, am(pxs*30/240)))
