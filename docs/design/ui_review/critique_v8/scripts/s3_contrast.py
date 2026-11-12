import sys, os, colorsys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from common import *
import numpy as np
palette_set("HI-CON")
print("PALETTE HI-CON tokens:")
for k in ("BG","SURF","TEXT","MID","DIM","RAISE","KEY","REC","LINE","SEL","TINT","INK"):
    print("  %-5s 0x%04X %s" % (k, getattr(T,k), hexc(getattr(T,k))))
for n,v in (("WHITE",mod.WHITE),("PHC",mod.PHC),("DICE_BG",mod.DICE_BG),("DICE_FG",mod.DICE_FG)): print("  %-7s 0x%04X %s" % (n,v,hexc(v)))
for i,v in enumerate(mod.CH): print("  CH%d    0x%04X %s" % (i+1,v,hexc(v)))
import el
print("  knob fill mix(CHn,120):", [hexc(el.mix(c,120)) for c in mod.CH])
cr = contrast_ratio
# ---------- text pairs from actual pixels under the ink box
S = scenes()
NOTEXT = [False]
_o = mock.Canvas.text_flags
def _nt(self, x, y, f, s, fg, bg, flags=0):
    if NOTEXT[0]:
        n0=len(self.texts); r=_o(self, x, y, f, s, fg, bg, flags)   # keep ink box bookkeeping, then...
        return r
    return _o(self, x, y, f, s, fg, bg, flags)
# simpler: render normally to get records, then render with draw of ramp disabled
import mock as M
MAIN = ["playing","two_channels","one_channel","sel_muted","dice","turning_k2","rec_ring" if False else "rec_armed","midi_off"]
orig_run = M._text_run
orig_alpha = M.Canvas._alpha
rows = {}
for n in MAIN:
    sc, cv, rec = render(S[n])
    M.Canvas._alpha = lambda *a, **k: None   # no glyph pixels => background-only screen
    sc2, _, rec2 = render(S[n])
    M.Canvas._alpha = orig_alpha
    px = sc2.px; W = 240
    for r in rec:
        if not r["ink"]: continue
        a,b,c,d = r["ink"]
        cols = {}
        for yy in range(max(b,0), min(d,240)):
            for xx in range(max(a,0), min(c,240)):
                cols[px[yy*W+xx]] = cols.get(px[yy*W+xx],0)+1
        # occluded by toast? header text in dice scene is under the toast rect
        occl = (n == "dice" and r["y"] < 21 and r["s"] not in ("NOTES DICED","EDIT=UNDO"))
        key = (r["font"], r["fg"], tuple(sorted(cols)))
        e = rows.setdefault(key, dict(scenes=set(), strs=set(), occl=True))
        e["scenes"].add(n); e["strs"].add(r["s"]); e["occl"] &= occl
print("\nTEXT PAIRS (fg vs every distinct background colour actually under the glyph box, no-text render)")
print("%-5s %-8s %-30s %-7s %s" % ("font","fg","bg set (ratio each)","worst","strings / scenes"))
flags = []
for (fo, fg, bgs), e in sorted(rows.items(), key=lambda kv: min(cr(kv[0][1], b) for b in kv[0][2])):
    rr = [(name_of(b), cr(fg, b)) for b in bgs]
    w = min(x for _, x in rr)
    lim = 3.0 if fo == "AF_L" else 4.5
    fl = "FAIL<%.1f" % lim if w < lim else ""
    if e["occl"]: fl += " (occluded by toast)"
    print("%-5s %-8s %-40s %5.2f %s  %s | %s" % (fo, name_of(fg), ",".join("%s:%.2f" % x for x in rr), w, fl, sorted(e["strs"])[:8], sorted(e["scenes"])))
