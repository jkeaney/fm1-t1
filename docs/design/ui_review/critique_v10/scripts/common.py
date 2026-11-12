"""shared setup: loads mock + v10, records every text draw via Canvas.text_flags and every rect via Canvas.rect/pset-level fills"""
import os, sys
HERE = os.path.dirname(os.path.abspath(__file__))
UI = os.path.abspath(os.path.join(HERE, "..", ".."))
V10 = os.path.join(UI, "proposals", "v10")
sys.path.insert(0, os.path.join(UI, "mock"))
import mock
sys.modules["mock"] = mock
from mock import *
import mock as M
mod = M.load_proposal(os.path.join(V10, "v10.py"))
import algos_v10 as A
from PIL import Image

LOG = []
_orig = M.Canvas.text_flags
def _tf(self, x, y, f, s, fg, bg, flags=0):
    LOG.append(dict(s=s, font=f.name if not isinstance(f, str) else f, fg=fg, bg=bg, x=x, y=y, cy0=self.cy0, cy1=self.cy1))
    return _orig(self, x, y, f, s, fg, bg, flags)
M.Canvas.text_flags = _tf

def scene(name):
    st = mod.STATE["scenes"][name]
    return {**{a: b for a, b in mod.STATE.items() if a != "scenes"}, **st}

def render(name_or_state, palette="HI-CON"):
    """returns (screen, canvas, text log) for a scene name or a state dict"""
    S = scene(name_or_state) if isinstance(name_or_state, str) else name_or_state
    LOG.clear()
    import io, contextlib
    with contextlib.redirect_stdout(io.StringIO()):
        sc, cv = M.render_state(mod, S, palette)
    return sc, cv, list(LOG)

def rgb(c): return M.to_rgb888(c)
def hexc(c): return "#%02x%02x%02x" % rgb(c)

# ---- P-level recording (absolute coords, strip-independent)
import el
PT = []; PA = []; PR = []
_pt = el.P.text; _pa = el.P.art; _pr = el.P.rect; _pf = el.P.frame
def _ptext(self, x, ycap, f, s, fg, bg):
    PT.append(dict(s=s, font=f.name, fg=fg, bg=bg, x=x, ycap=ycap, w=text_w(f, s), strip=(self.y0, self.y1)))
    return _pt(self, x, ycap, f, s, fg, bg)
def _part(self, x, y, a, c):
    rows = a.strip("\n").split("\n")
    PA.append(dict(x=x, y=y, w=max(map(len, rows)), h=len(rows), c=c, bits=sum(r.count("#") for r in rows), art=a, strip=(self.y0, self.y1)))
    return _pa(self, x, y, a, c)
def _prect(self, x, y, w, h, c):
    PR.append(("rect", x, y, w, h, c, self.y0)); return _pr(self, x, y, w, h, c)
def _pframe(self, x, y, w, h, c):
    PR.append(("frame", x, y, w, h, c, self.y0)); return _pf(self, x, y, w, h, c)
el.P.text = _ptext; el.P.art = _part; el.P.rect = _prect; el.P.frame = _pframe

def rec(name_or_state, palette="HI-CON"):
    """render + return dict(texts=dedup'd by (s,x,ycap), arts, rects) in absolute coords, plus the screen"""
    PT.clear(); PA.clear(); PR.clear()
    sc, cv, lg = render(name_or_state, palette)
    seen = {}; texts = []
    for t in PT:
        k = (t["s"], t["x"], t["ycap"])
        if k in seen: seen[k]["strips"].append(t["strip"]); continue
        t = dict(t); t["strips"] = [t["strip"]]; seen[k] = t; texts.append(t)
    da = {}; 
    for a in PA: da.setdefault((a["x"], a["y"], a["art"]), a)
    dr = {}
    for r_ in PR: dr.setdefault(r_[:6], r_)
    return dict(sc=sc, cv=cv, texts=texts, arts=list(da.values()), rects=list(dr.values()))

def tokname(c):
    names = {}
    for tk in M.Theme.TOKENS:
        names.setdefault(getattr(M.ux, tk), tk.upper().rstrip("_"))
    for nm, v in (("WHITE", mod.WHITE), ("INK", mod.INK), ("PHC", mod.PHC), ("DICE_BG", mod.DICE_BG), ("OFF_RED", mod.OFF_RED)):
        names.setdefault(v, nm)
    for i, v in enumerate(mod.CH): names.setdefault(v, "CH%d" % (i + 1))
    return "%s(%s)" % (names.get(c, "?"), hexc(c))
