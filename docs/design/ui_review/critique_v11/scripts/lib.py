import os, sys, copy
R = "/home/claude/fm1/docs/design/ui_review"
sys.path[:0] = [R + "/mock", R + "/proposals/v11", R + "/proposals/v5", R + "/proposals/v7"]
import mock
from mock import T, AF_S, AF_M, AF_L, text_w, contrast_ratio, palette_set, rgb565, to_rgb888, Canvas, render_state
import el
os.chdir(R + "/proposals/v11")
mod = mock.load_proposal(R + "/proposals/v11/v11.py")
import algos_v11 as A
import icons_v11
FN = {id(AF_S): "AF_S", id(AF_M): "AF_M", id(AF_L): "AF_L"}
REC = []      # (scene-local) text records, absolute y
ARTS = []
_orig_tf = Canvas.text_flags
_orig_blit = Canvas.blit
_pending = []
def _tf(self, x, y, f, s, fg, bg, flags=0):
    ff = mock.font(f) if isinstance(f, str) else f
    _pending.append((self, x, y, FN.get(id(ff), str(ff.name)), s, fg, bg))
    return _orig_tf(self, x, y, f, s, fg, bg, flags)
def _blit(self, x, y):
    for (cv, tx, ty, fn, s, fg, bg) in list(_pending):
        if cv is self: REC.append(dict(x=tx + x, y=ty + y, font=fn, s=s, fg=fg, bg=bg))
    _pending[:] = [q for q in _pending if q[0] is not self]
    return _orig_blit(self, x, y)
Canvas.text_flags = _tf; Canvas.blit = _blit
_orig_art = el.P.art
def _art(self, x, y, a, c):
    ARTS.append((x, y, c)); return _orig_art(self, x, y, a, c)
el.P.art = _art
def render(state):
    REC.clear(); ARTS.clear(); _pending.clear()
    sc, cv = render_state(mod, state)
    return sc, cv, list(REC), list(ARTS)
def scene(n):
    return {**{a: b for a, b in mod.STATE.items() if a != "scenes"}, **mod.STATE["scenes"][n]}
palette_set("HI-CON")
def hexc(c): return "#%02x%02x%02x" % to_rgb888(c)

def render_notext(state):
    saved = Canvas.text_flags
    def nt(self, x, y, f, s, fg, bg, flags=0):
        ff = mock.font(f) if isinstance(f, str) else f
        return x + ((mock._text_run(ff, s, x, y, _Dummy(), lambda *a: None)[0] + 8) >> 4) if False else x + text_w(ff, s)
    Canvas.text_flags = nt
    try:
        sc, cv = render_state(mod, state)
    finally:
        Canvas.text_flags = saved
    return sc
def texts_with_bbox(state):
    sc, cv, rec, arts = render(state)
    rec = [r for r in rec if r['s']]
    assert len(sc.texts) == len(rec), (len(sc.texts), len(rec))
    out = []
    for r, t in zip(rec, sc.texts):
        assert r["s"] == t[4], (r["s"], t[4])
        r = dict(r); r["bbox"] = t[:4]; r["flags"] = t[5]; out.append(r)
    return sc, out, arts

def render_skip(state, idx):
    """render with text number idx (in draw order) suppressed"""
    saved = Canvas.text_flags
    cnt = [0]
    def f(self, x, y, fo, s, fg, bg, flags=0):
        i = cnt[0]; cnt[0] += 1
        if i == idx:
            ff = mock.font(fo) if isinstance(fo, str) else fo
            return x + text_w(ff, s)
        return saved(self, x, y, fo, s, fg, bg, flags)
    Canvas.text_flags = f
    try:
        sc, cv = render_state(mod, state)
    finally:
        Canvas.text_flags = saved
    return sc
def visible_flags(state, sc, tx):
    if not (state.get("toast") or state.get("hold")):
        return [True] * len(tx)
    out = []
    for i, r in enumerate(tx):
        x0, y0, x1, y1 = r["bbox"]
        s2 = render_skip(state, i)
        out.append(any(sc.px[y * 240 + x] != s2.px[y * 240 + x] for y in range(y0, y1) for x in range(x0, x1)))
    return out
