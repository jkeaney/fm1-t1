"""lib.py: shared instrumentation for the v7 evidence scripts (measures only; edits nothing)."""
import os, sys, inspect, copy, json
UI = "/home/claude/fm1/docs/design/ui_review"
V7 = UI + "/proposals/v7"
sys.path.insert(0, UI + "/mock")
import mock
from mock import T, AF_S, AF_M, AF_L, text_w, rgb565, to_rgb888, contrast_ratio, luminance
sys.modules.setdefault("mock", mock)
mod = mock.load_proposal(V7 + "/v7.py")
import el
mock.palette_set("MONO")

CAPH = {AF_S.name: 9, AF_M.name: 11, AF_L.name: 21}
FN = {AF_S.name: "AF_S", AF_M.name: "AF_M", AF_L.name: "AF_L"}

def mm(f):  return CAPH[f.name] / 8.0
def arcmin(f): return CAPH[f.name] / 8.0 / 350.0 * 3438.0

LOG = None
_orig = {}
_SEQ = [0]
def _seq():
    _SEQ[0] += 1; return _SEQ[0]

def _caller():
    for fr in inspect.stack()[2:]:
        n = fr.function
        if n not in ("text", "text_c", "text_r", "rect", "frame", "art", "hatch", "line", "pset", "dots", "<lambda>", "paint", "_w"):
            return n
    return "?"

def install():
    P = el.P
    for name in ("rect", "frame", "art", "hatch", "line", "text"):
        _orig[name] = getattr(P, name)
    def w_rect(self, x, y, w, h, c):
        LOG["ops"].append(dict(k="rect", x=x, y=y, w=w, h=h, c=c, fn=_caller(), seq=_seq(), s0=self.y0)); return _orig["rect"](self, x, y, w, h, c)
    def w_frame(self, x, y, w, h, c):
        LOG["ops"].append(dict(k="frame", x=x, y=y, w=w, h=h, c=c, fn=_caller(), seq=_seq(), s0=self.y0)); return _orig["frame"](self, x, y, w, h, c)
    def w_art(self, x, y, a, c):
        rows = a.strip("\n").split("\n")
        LOG["ops"].append(dict(k="art", x=x, y=y, w=max(len(r) for r in rows), h=len(rows), c=c, fn=_caller(), seq=_seq(), s0=self.y0, art=a)); return _orig["art"](self, x, y, a, c)
    def w_hatch(self, x, y, w, h, c, step=3):
        LOG["ops"].append(dict(k="hatch", x=x, y=y, w=w, h=h, c=c, fn=_caller(), seq=_seq(), s0=self.y0, step=step)); return _orig["hatch"](self, x, y, w, h, c, step)
    def w_line(self, x0, y0, x1, y1, c):
        LOG["ops"].append(dict(k="line", x=min(x0,x1), y=min(y0,y1), w=abs(x1-x0)+1, h=abs(y1-y0)+1, c=c, fn=_caller(), seq=_seq(), s0=self.y0)); return _orig["line"](self, x0, y0, x1, y1, c)
    def w_text(self, x, ycap, f, s, fg, bg):
        cv = self.cv
        n0 = len(cv.texts)
        r = _orig["text"](self, x, ycap, f, s, fg, bg)
        rec = dict(s=s, f=FN[f.name], x=x, ycap=ycap, fg=fg, bg=bg, fn=_caller(), seq=_seq(), s0=self.y0, adv=text_w(f, s), ink=None, cut=False)
        if len(cv.texts) > n0:
            a, b, c, d, _s, fl = cv.texts[-1]
            rec["ink"] = (a, b + self.y0, c, d + self.y0)
            rec["cut"] = bool(fl & 2)
        LOG["texts"].append(rec)
        return r
    P.rect, P.frame, P.art, P.hatch, P.line, P.text = w_rect, w_frame, w_art, w_hatch, w_line, w_text
install()

def capture(state, palette="MONO", skip_text=False):
    """render a state; returns dict(screen, cv, texts (deduped), ops (deduped), warnings)"""
    global LOG
    LOG = dict(ops=[], texts=[])
    mock.palette_set(palette)
    sc = mock.Screen(); cv = mock.Canvas(sc); cv.begin(240, 240, mock.ux.bg)
    w0 = len(cv.warnings)
    if skip_text:
        cv.text_flags = lambda *a, **k: a[0]
    mod.draw(cv, state)
    warns = list(cv.warnings[w0:])
    # dedupe (strips draw the same call twice when it straddles y=120)
    texts, seen = [], {}
    for t in LOG["texts"]:
        key = (t["s"], t["x"], t["ycap"], t["f"], t["fg"], t["bg"])
        if key in seen:
            seen[key]["strips"].append(t["s0"])
            if t["ink"] and not seen[key]["ink"]:
                seen[key]["ink"] = t["ink"]
            seen[key]["cut"] = seen[key]["cut"] or t["cut"]
            continue
        t = dict(t); t["strips"] = [t["s0"]]
        seen[key] = t; texts.append(t)
    ops, seen2 = [], set()
    for o in LOG["ops"]:
        key = (o["k"], o["x"], o["y"], o["w"], o["h"], o["c"], o.get("step"))
        if key in seen2: continue
        seen2.add(key); ops.append(o)
    for t in texts:
        t["occluded"] = False
        if t["ink"]:
            a, b, c, d = t["ink"]
            for o in ops:
                if o["k"] == "rect" and o["seq"] > t["seq"] and o["x"] <= a and o["y"] <= b and o["x"] + o["w"] >= c and o["y"] + o["h"] >= d:
                    t["occluded"] = True; break
    return dict(screen=sc, cv=cv, texts=texts, ops=ops, warnings=warns)

def S():
    return mod.STATE["scenes"]

SCENES = ["playing", "one_channel", "two_channels_stopped", "dice", "turning_k2", "rec_armed"]

def hexc(c):
    r, g, b = to_rgb888(c); return "#%02x%02x%02x" % (r, g, b)

def tokname(c):
    tk = {"BG": T.BG, "TEXT": T.TEXT, "MID": T.MID, "DIM": T.DIM, "LINE": T.LINE, "RAISE": T.RAISE, "KEY": T.KEY, "REC": T.REC, "WHITE": el.WHITE, "INK": el.INK, "PH": el.PH, "DICE_BG": el.DICE_BG,
          "CH1": el.CH_COL[0], "CH2": el.CH_COL[1], "CH3": el.CH_COL[2], "CH4": el.CH_COL[3], "DICE_AMBER": rgb565(255, 205, 90)}
    names = [k for k, v in tk.items() if v == c]
    return "/".join(names) if names else "mix"

# ---- palette variants: the mock's palette regex drops "HI-CON" (hyphen), so MONO lands at index 8 instead of UI_BW_INDEX 9
def fw_palettes():
    import re
    src = mock._read_gen("ui_palettes.h")
    pals = [(m.group(1),) + tuple(int(m.group(i), 16) for i in range(2, 7))
            for m in re.finditer(r'\{"([\w-]+)",\s*(0x\w+),\s*(0x\w+),\s*(0x\w+),\s*(0x\w+),\s*(0x\w+)\}', src)]
    defs = {m.group(1): int(m.group(2), 0) for m in re.finditer(r"#define (UI_\w+)\s+(0x[0-9a-fA-F]+|\d+)u?\b", src)}
    return pals, defs

def variant(name):
    """'mock' = as the mock parses it (what produced out/*.png); 'fw' = palette table parsed with HI-CON, MONO at idx 9"""
    mock._PAL = None
    if name == "fw":
        mock._PAL = fw_palettes()
    else:
        mock.palettes()
    mock._rc.clear()
    mock.palette_set("MONO")
    return mock.ux.palette, mock.ux.mono, hexc(mock.ux.dim), hexc(mock.ux.line), hexc(mock.ux.raise_), hexc(mock.ux.mid)
