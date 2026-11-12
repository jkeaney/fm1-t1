"""common.py: loads mock + v8 proposal, records every text draw (font, fg, bg, ink box in screen coords)."""
import os, sys
ROOT = "/home/claude/fm1"
MOCKD = ROOT + "/docs/design/ui_review/mock"
V8D = ROOT + "/docs/design/ui_review/proposals/v8"
sys.path.insert(0, MOCKD)
import mock
from mock import T, AF_S, AF_M, AF_L, text_w, measure, contrast_ratio, palette_set, render_state, to_rgb888, rgb565
sys.modules["mock"] = mock
mod = mock.load_proposal(V8D + "/v8.py")
import algos_v8 as A
import icons_v8
FN = {id(AF_S): "AF_S", id(AF_M): "AF_M", id(AF_L): "AF_L"}
REC = []      # records for current render
_pending = []
_orig_tf = mock.Canvas.text_flags
_orig_blit = mock.Canvas.blit
def _tf(self, x, y, f, s, fg, bg, flags=0):
    n0 = len(self.texts)
    r = _orig_tf(self, x, y, f, s, fg, bg, flags)
    ink = self.texts[-1][:4] if len(self.texts) > n0 else None
    _pending.append(dict(s=s, font=FN.get(id(f), getattr(f, "name", "?")), fg=fg, bg=bg, ink=ink, adv=text_w(f, s), x=x, y=y, flags=flags))
    return r
def _blit(self, x, y):
    for p in _pending:
        if p["ink"]:
            a, b, c, d = p["ink"]; p["ink"] = (a + x, b + y, c + x, d + y)
        p["y"] += y; p["x"] += x
        REC.append(p)
    _pending.clear()
    return _orig_blit(self, x, y)
mock.Canvas.text_flags = _tf
mock.Canvas.blit = _blit
def render(state, pal="HI-CON"):
    REC.clear(); _pending.clear()
    sc, cv = render_state(mod, state, pal)
    return sc, cv, list(REC)
def scenes():
    st = mod.STATE
    return {n: {**{a: b for a, b in st.items() if a != "scenes"}, **s} for n, s in st["scenes"].items()}
def hexc(c): return "#%02x%02x%02x" % to_rgb888(c)
def name_of(c):
    pal = {}
    for k in ("BG","SURF","TEXT","MID","DIM","RAISE","KEY","REC","LINE","SEL","TINT","INK"):
        pal[getattr(T,k)] = k
    for n, v in (("WHITE", mod.WHITE), ("PHC", mod.PHC), ("DICE_BG", mod.DICE_BG), ("DICE_FG", mod.DICE_FG)):
        pal.setdefault(v, n)
    for i, v in enumerate(mod.CH): pal.setdefault(v, "CH%d" % (i+1))
    return pal.get(c, hexc(c))
# --- icon (art) recording
import el
ART = []
_orig_art = el.P.art
_ICN = {}
for k, v in icons_v8.ICONS.items(): _ICN.setdefault(v, k)
for k, v in el.IC.items(): _ICN.setdefault(v, "IC." + k)
def _art(self, x, y, a, c):
    rows = a.strip("\n").split("\n")
    ART.append(dict(name=_ICN.get(a, "?"), x=x, y=y + self.y0 * 0, w=max(len(r) for r in rows), h=len(rows), c=c, y0=self.y0))
    return _orig_art(self, x, y, a, c)
el.P.art = _art
