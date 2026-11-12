import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from common import *
import el
S = scenes()
def runs(flags):
    out=[]; st=None
    for i,f in enumerate(flags+[False]):
        if f and st is None: st=i
        if not f and st is not None: out.append((st,i-1)); st=None
    return out
def occupancy(n):
    sc, cv, rec = render(S[n]); px = sc.px; bg = T.BG
    rows = [any(px[y*240+x] != bg for x in range(240)) for y in range(240)]
    return sc, rec, rows, px
sc, rec, rows, px = occupancy("playing")
r = runs(rows)
print("ROW OCCUPANCY (any non-BG pixel) scene playing: y-ranges:", r)
print("gaps between ranges:", [(r[i][1]+1, r[i+1][0]-1, r[i+1][0]-r[i][1]-1) for i in range(len(r)-1)])
for n in ["playing","two_channels","one_channel","sel_muted","dice","turning_k2","rec_armed","midi_off"]:
    sc, rec, rows, px = occupancy(n); print("  ", n, runs(rows))
print("\nBANDS from code (v8.py constants): header chips y=1..19 (h19); knob label y=24..39 (h16), 1px gap y=40, value tile y=41..76 (h36); row markers y=81..85 (5x5); music y=91..123 (h33) + playhead white bar y=87..88 + tick marks y=125..127; buttons row1 y=132..155, row2 y=158..181 (h24, 2px gap); keybed keys y=188..221 (h34), black keys y=188..207 (h20, 1px BG halo), playing bar y=222..223; footer text ink y=226..238")
print("Strip split: strips (0,120) and (120,240); strip edge y=120 (rows 119|120)")
cross = []
for n in ["playing","two_channels","one_channel","sel_muted","dice","turning_k2","rec_armed","midi_off","euclid_midi16","arp","demo_algo"]:
    sc, cv, rec = render(S[n])
    for t in rec:
        if t["ink"] and t["ink"][1] < 120 < t["ink"][3]: cross.append((n, t["s"], t["ink"]))
print("TEXT INK boxes crossing y=120:", cross if cross else "none (checked 11 scenes)")
# nearest text rows to 120
near = sorted({(t["s"], t["ink"][1], t["ink"][3]) for t in render(S["playing"])[2] if t["ink"]}, key=lambda z: abs((z[1]+z[2])/2-120))[:3]
print("nearest text to edge (playing):", near)
# knob tiles
print("\nKNOB TILES: x = k*60+1, w58 -> x ranges", [(k*60+1, k*60+58) for k in range(4)], "gap between tiles = 2 px (x=59,60 etc.); right margin 240-239=1 (tile ends x=238 -> 1 px to right edge 239) left margin 1")
sc, cv, rec = render(S["playing"])
for t in rec:
    if t["ink"] and 24 <= t["ink"][1] <= 70:
        k = t["ink"][0]//60; x0 = k*60+1
        print("  %-8r %s ink x %d..%d  margin L %d R %d (tile %d..%d) y %d..%d" % (t["s"], t["font"], t["ink"][0], t["ink"][2]-1, t["ink"][0]-x0, x0+58-t["ink"][2], x0, x0+57, t["ink"][1], t["ink"][3]-1))
# longest labels/values among all registered algorithms: margins
print("\nLabel adv widths all algos (AF_M), tile text area 58 (54 usable AF_L rule):")
mx = {}
for a in A.ORDER:
    for r_ in A.ALGOS[a]["rows"]:
        for k in r_:
            w = text_w(AF_M, k["label"]); mx[k["label"]] = w
top = sorted(mx.items(), key=lambda kv: -kv[1])[:6]; print("  widest labels:", top, " max label adv =", max(mx.values()), " tile 58 -> min margin per side", (58-max(mx.values()))/2)
# music-row slots
print("\nMUSIC-ROW SLOT WIDTHS (cell content 13 px of 15 px pitch; 2 px column gap)")
for n, sl in mod.SLOTS.items():
    gaps = [sl[i+1][0]-(sl[i][0]+sl[i][1]) for i in range(len(sl)-1)]
    print("  %d channel(s): (offset,width)=%s gaps between slots=%s px, right pad %d px" % (n, sl, gaps, 13-(sl[-1][0]+sl[-1][1])))
# verify with pixels: step 0 has notes on all 4 channels? print pixel colours across x for the row at y=122 in 'playing' (bottom row of bars)
sc, cv, rec = render(S["playing"]); px = sc.px
for step in (0, 1, 2, 3, 4, 7):
    xs = [(x, name_of(px[122*240+x])) for x in range(step*15, step*15+15)]
    print("  step", step+1, "row y=122 pixels:", "".join({"BG":".","INK":".","CH1":"1","CH2":"2","CH3":"3","CH4":"4","PHC":"p","DIM":"d","WHITE":"W"}.get(c, "?") for _, c in xs))
# bar heights
print("\nBAR HEIGHT: pitched = 4+(33-7)*(p-lo)//(hi-lo) -> 4..30 px; single-lane = 6 px; accent cap 2px white at by-3..by-2 (1 px gap above bar); min bar 4 px tall x slot width 3..13")
# header chips
print("\nHEADER CHIPS: x=46+14i, w13, h19 (y1..19); gap between chips 1 px; chip text AF_M digit adv 9 in 13 px -> 2 px each side")
print("header text right edges:")
for n in ["playing","midi_off","euclid_midi16","sel_muted"]:
    sc, cv, rec = render(S[n])
    hs = [(t["s"], t["ink"][0], t["ink"][2]) for t in rec if t["ink"] and t["ink"][1] < 21]
    print("  ", n, hs)
# icon sizes
print("\nICON SIZES (w x h px):")
ART.clear(); render(S["playing"]); seen={}
for a in ART: seen[a["name"]] = (a["w"], a["h"])
for k,v in sorted(seen.items()): print("   %-12s %dx%d" % (k, *v))
print("  rec disc (header armed): rects 9x9+11x7+7x11 = 11x11 octagon-ish; rec button icon 'rec' check:", len(icons_v8.ICONS["rec"].strip().split(chr(10))), "x", max(len(r_) for r_ in icons_v8.ICONS["rec"].strip().split(chr(10))))
print("  button tile 38x24; dice tile same; white keys 14x34 pitch 15 (1px gap); black keys 10x20 with 1 px BG halo (12 wide) ; row markers 5x5 pitch 8 (3 px gap) at x=109,117,125")
# <2 px apart
print("\nSEPARATIONS < 2 px (edge-to-edge):")
print("  - knob label band to value tile: 1 px (y=40)")
print("  - header chips: 1 px between neighbours (13 wide on 14 pitch)")
print("  - music 4-ch slots: ch slot1|slot2 0 px, slot2|slot3 1 px, slot3|slot4 0 px (offsets 0,3,7,10 w3)")
print("  - music 2-ch/3-ch slots: 1 px between slots")
print("  - accent cap (white 2 px) to bar top: 1 px; white keys: 1 px gap; row marker bottom (y=85) to playhead bar top (y=87): 1 px (y=86)")
print("  - music tick marks y=125..127 vs bar bottoms y=123: 1 px (y=124)")
print("  - ink right edge of 'HOLD = EDIT  PAGE 1/2' footer: 238 -> 1 px to right edge x=239 (see below)")
for n in ["playing","rec_armed"]:
    sc, cv, rec = render(S[n])
    for t in rec:
        if t["ink"] and (t["ink"][0] < 2 or t["ink"][2] > 238): print("  ink near screen edge:", n, t["s"], t["ink"])
# text to tile edge min margin across all scenes
