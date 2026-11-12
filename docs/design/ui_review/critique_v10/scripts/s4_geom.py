from common import *
import itertools
from collections import defaultdict, Counter
print("# 4 geometry (HI-CON, 240x240)")
# ---- band y-ranges from the rendered image (rows containing any non-BG pixel), scene playing
def bands(name):
    r = rec(name); img = r["sc"].to_image(); px = img.load()
    bg = rgb(M.ux.bg)
    rows = [any(px[x, y] != bg for x in range(240)) for y in range(240)]
    out = []; y = 0
    while y < 240:
        if rows[y]:
            y0 = y
            while y < 240 and rows[y]: y += 1
            out.append((y0, y - 1))
        else: y += 1
    return out
for n in ("playing", "one_channel", "rec_armed", "dice", "hold_glo"):
    b = bands(n)
    print("\n## non-empty row bands, scene %s: %s" % (n, b))
    print("   gaps between bands (px of pure BG): %s" % [b[i+1][0] - b[i][1] - 1 for i in range(len(b) - 1)])
# ---- element ranges (computed from code constants & drawn rects)
r = rec("playing"); R = r["rects"]
def rr(pred): return [x for x in R if pred(x)]
def bbox(L): return (min(x[1] for x in L), min(x[2] for x in L), max(x[1]+x[3] for x in L) - 1, max(x[2]+x[4] for x in L) - 1)
print("\n## element y-ranges (inclusive), scene playing")
hdr = rr(lambda x: x[2] <= 22 and x[6] == 0)
print("header chips/bar: y %d..%d" % (min(x[2] for x in hdr), max(x[2]+x[4]-1 for x in hdr)))
kt = rr(lambda x: x[0]=="rect" and x[5]==M.ux.surf and 24 <= x[2] < 80 and x[3] == 58)
print("knob tiles: label tile y %d..%d, body tile y %d..%d, row gap between = %d px, tile gap horizontally = %d px (x step 60, w 58)" % (24, 39, 41, 76, 1, 2))
print("status line: text cap y 80..90 (AF_M cap 11); row markers y 82..88; page dots y 82..88; pending pill y 78..91 (h14)")
mu = rr(lambda x: 90 < x[2] < 142 and x[3] in (13,15) )
print("music region: code y0=96 y1=141; playhead column y 95..140 (PHC), white bar y 93..94; bars end y 140 at most")
print("buttons: row1 y 146..162, row2 y 164..180 (cell 38x17, pitch 40x18: 2 px gap horizontally, 1 px vertically), x 1..39 .. 201..239")
print("keybed: white keys y 186..218 (h33, w14, pitch 15 = 1px gap), lane block y 207..218 (h12), playhead bar y 219..220; black keys y 186..205 (10x19 + 1px BG halo)")
print("message line: M cap 11 at y 225..235 ; S cap 9 at y 226..234")
print("top bar (toast/hold): y 0..21 full width; hold progress bar y 20..21 (2 px)")
print("measured gaps (BG rows) in playing: header chips 1..19, selection bar 21..22, knob label 24..39, knob body 41..76, status 79..91, music 93..140, buttons 146..162 / 164..180, keybed 186..220, message line 224..236; gaps = [1,1,1,2,1,5,1,5,3] (see bands above); pending: see pill section")
# ---- text crossing y=120 and rects crossing y=120
cross = [t for t in r["texts"] if t["ycap"] < 120 <= t["ycap"] + {"S":9,"M":11,"L":21}[t["font"]]]
print("\ntexts whose cap box crosses y=120 (playing):", [(t["s"], t["ycap"]) for t in cross] or "none")
allx = []
CAP = {"S": (3, 9), "M": (4, 11), "L": (7, 21)}
for n in mod.STATE["scenes"]:
    rr_ = rec(n)
    strips = [(0, 116), (116, 232), (232, 240)] if mod.STATE["scenes"][n].get("sheet") else [(0, 120), (120, 240)]
    for t in rr_["texts"]:
        ch = CAP[t["font"]][1]
        n_in = sum(1 for (a, b) in strips if not (t["ycap"] + ch <= a or t["ycap"] >= b))
        if n_in > 1: allx.append((n, t["s"], t["ycap"], "straddles a strip boundary"))
print("all 28 scenes: texts whose cap box straddles a firmware strip boundary (120 for screens; 116/232 for sheet scenes):", allx or "none")
cr = [x for x in R if x[2] < 120 < x[2] + x[4] and x[6] == 0 or (x[2] < 120 <= x[2] + x[4]-1)]
cl = Counter((x[0], x[3], x[4], tokname(x[5])) for x in R if x[2] < 120 <= x[2] + x[4] - 1)
print("rects spanning y=119/120 boundary (playing), type/w/h/colour:count ->", dict(cl))
# ---- buttons: cell and icon fit
import icons_v10 as IV
print("\n## buttons: cell 38x17 (x=c*40+1, y=146+r*18), 12 cells")
print("| button | icon | w x h | margin L/R | margin T/B | lit px | stroke notes |"); print("|---|---|---|---|---|---|---|")
for pn, ic, role, hold, dice in IV.BUTTONS:
    rows = IV.ICONS[ic].strip("\n").split("\n"); w = max(map(len, rows)); h = len(rows)
    ml = (38 - w) // 2; mr = 38 - w - ml; mt = (17 - h) // 2; mb = 17 - h - mt
    print("| %s | %s | %dx%d | %d/%d | %d/%d | %d | %s |" % (pn, ic, w, h, ml, mr, mt, mb, sum(r_.count("#") for r_ in rows), "TIGHT (<3 px margin T/B)" if min(mt, mb) < 3 else ""))
# ---- music sizes per channel count
print("\n## music bars/strips per channel count (code: y0=96,y1=141, strip_h=3, pitch 4)")
print("| channels | others | bh_max | pitched bar h range | single bar h | bar y-top (tallest) | strip rows y | strip bottom | accent cap top (tallest bar) | gap cap-top to playhead white bar (rows 93-94) |"); print("|---|---|---|---|---|---|---|---|---|---|")
for nch in range(1, 5):
    oth = nch - 1; sh = oth * 4; bhm = 45 - sh - 1
    tall = bhm - 3; by = 96 + bhm - tall
    ys = [96 + bhm + 2 + k * 4 for k in range(oth)]
    print("| %d | %d | %d | 6..%d | 8 | %d | %s | %s | %d | %d |" % (nch, oth, bhm, tall, by, ys or "-", (ys[-1]+2) if ys else "-", by - 4, (by - 4) - 95))
print("  (gap 0 = the accent cap on the top-pitch bar touches the playhead's white bar when that step is under the playhead; bars: w13 pitch15 -> 2 px gap; slide bridge 2x3; strip 13x3, dash 5x3)")
# ---- pending pill vs playhead
print("\n## pending pill P0n? (y 78..91) vs playhead white bar (y 93..94) / column (y 95..)")
for nn in (2, 9, 12):
    w = text_w(AF_M, "P%02d?" % nn) + 6
    steps = [i + 1 for i in range(16) if (i*15+1-1) < 1 + w and (i*15+1-1+15) > 1]
    print("pill P%02d?: x 1..%d (w %d), vertical gap to playhead white bar = 92-91 = 1 px (row 92); steps whose playhead column x-range overlaps the pill in x: %s" % (nn, w, w, steps))
print("the pill is 1 px above the playhead white bar for steps %s; for all other steps no x-overlap." % steps)
# measured: render pending with each playhead
bad = []
for ph in range(16):
    S = scene("pending_p02"); S = dict(S, playhead=ph)
    rr_ = rec(S); img = rr_["sc"].to_image(); px = img.load()
    # rows 78..91 white pill vs rows 92 (should be BG) ; any white at y=92 inside pill x range
    w = text_w(AF_M, "P02?") + 6
    touch = any(px[x, 92] != (0,0,0) for x in range(1, 1 + w))
    white_pix_93 = sum(1 for x in range(0, 40) if px[x, 93] == (255,255,255))
    bad.append((ph + 1, touch, white_pix_93))
print("measured per playhead step (step, non-BG pixel on row 92 under pill, white px on row 93 in x<40):", bad)
# ---- <2px apart: rects of different colours with gap 0/1 (playing, dice, midi_off)
def gaps(name):
    R = rec(name)["rects"]
    out = Counter()
    rects = [x for x in R if x[0] == "rect"]
    for a, b in itertools.combinations(rects, 2):
        if a[5] == b[5]: continue
        ax0, ay0, ax1, ay1 = a[1], a[2], a[1]+a[3], a[2]+a[4]; bx0, by0, bx1, by1 = b[1], b[2], b[1]+b[3], b[2]+b[4]
        xo = min(ax1, bx1) - max(ax0, bx0); yo = min(ay1, by1) - max(ay0, by0)
        if xo > 0 and 0 < (max(ay0, by0) - min(ay1, by1)) <= 1 or yo > 0 and 0 < (max(ax0, bx0) - min(ax1, bx1)) <= 1:
            out[(tokname(a[5]).split("(")[0], tokname(b[5]).split("(")[0], "v" if xo > 0 else "h", min(a[3:5]) , min(b[3:5]))] += 1
    return out
print("\n## rect pairs of different colours exactly 1 px apart (playing): (colourA, colourB, axis, minDimA, minDimB): count")
for k, v in sorted(gaps("playing").items(), key=lambda kv: -kv[1])[:40]: print("  ", k, v)
print("(1px gaps are by construction: header chips 13w pitch 14; music-column/bar; hit strips pitch 4 h3; button rows pitch 18 h17; knob label->body; selection bar under chip; pending pill->playhead)")
