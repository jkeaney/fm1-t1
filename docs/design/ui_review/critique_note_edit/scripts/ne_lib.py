# SPDX-License-Identifier: GPL-3.0-only
"""ne_lib.py: loaders shared by the critique_note_edit evidence scripts (run from the repo root with .venv/bin/python).
Inputs are the files make_trace.py wrote to critique_note_edit/data/ (op logs of the real ui_sq.c draw code, re-rendered PNGs, tokens)."""
import os, sys, collections
ROOT = os.getcwd()
D = os.path.join("docs", "design", "ui_review", "critique_note_edit")
DATA = os.path.join(D, "data")
sys.path.insert(0, os.path.join("docs", "design", "ui_review", "mock"))
import mock
from mock import text_w, contrast_ratio, AF_S, AF_M, AF_L
from PIL import Image
FONT = {"S": AF_S, "M": AF_M, "L": AF_L}
CAP = {"S": 9, "M": 11, "L": 21}          # cap-height px measured with mock.measure("H", font); 8 px per mm (240 px = 30 mm)
PXMM = 8.0
RENDERS = ["note_open", "note_set", "note_dice", "note_final"]
EXTRA = ["worst_w1", "worst_w2"]
def cap_mm(f): return CAP[f] / PXMM
def arcmin(f, dist_cm): return cap_mm(f) / (dist_cm * 10.0) * 3438.0

def tokens():
    t = collections.OrderedDict()
    for l in open(os.path.join(DATA, "tokens.tsv")):
        k, v = l.split("\t"); t[k] = int(v)
    return t
TOK = tokens()
def names_of(c565):
    n = [k for k, v in TOK.items() if v == c565]
    return n[0].replace("T_", "").replace("UISQ_", "") if n else "0x%04X" % c565      # first alias (BG = INK = 0x0000; see the token table)
def hexof(c565):
    r, g, b = c565 >> 11 & 31, c565 >> 5 & 63, c565 & 31
    return "#%02X%02X%02X" % (r * 255 // 31, g * 255 // 63, b * 255 // 31)
def cr(a, b): return contrast_ratio(a, b)
# RGB888 (as png_write expands) back to RGB565
REV = {}
for c in range(65536):
    REV[((c >> 11 & 31) * 255 // 31, (c >> 5 & 63) * 255 // 63, (c & 31) * 255 // 31)] = c

def ops(name):
    """-> list of dicts, drawn ops de-duplicated across the two strips (a strip draws an element again when it crosses y 120), in draw order"""
    seen = set(); out = []
    for l in open(os.path.join(DATA, name + ".ops")):
        p = l.rstrip("\n").split("\t")
        key = tuple(p[2:]) if p[0] != "T" else tuple(p[2:12])      # a text's flags (cut by the strip) differ per strip: not part of the key
        k2 = (p[0],) + key
        if k2 in seen: continue
        seen.add(k2)
        if p[0] == "T":
            out.append(dict(op="T", strip=int(p[1]), x=int(p[2]), y=int(p[3]), f=p[4], s=p[5], fg=int(p[6]), bg=int(p[7]), ink=tuple(int(v) for v in p[8:12]), fl=int(p[12])))
        elif p[0] in "RF":
            out.append(dict(op=p[0], x=int(p[2]), y=int(p[3]), w=int(p[4]), h=int(p[5]), c=int(p[6])))
        elif p[0] == "I":
            out.append(dict(op="I", x=int(p[2]), y=int(p[3]), id=int(p[4]), w=int(p[5]), h=int(p[6]), c=int(p[7])))
        elif p[0] == "L":
            out.append(dict(op="L", x0=int(p[2]), y0=int(p[3]), x1=int(p[4]), y1=int(p[5]), c=int(p[6])))
    return out
def texts(name): return [o for o in ops(name) if o["op"] == "T"]
def img(name): return Image.open(os.path.join(DATA, name + ".png")).convert("RGB")
def px565(im, x, y): return REV.get(im.getpixel((x, y)), -1)
ICON = {}
for l in open("build/gen/ui_sqicons.h"):
    pass
_names = []
_in = False
for l in open("build/gen/ui_sqicons.h"):
    s = l.strip()
    if s.startswith("SQI_") and s.endswith(",") and "SQI_N" not in s: _names.append(s[:-1])
def icon_name(i): return _names[i] if i < len(_names) else "?"

def cover(name):
    """-> {index in ops(name): fraction of that text's ink box painted over by a LATER filled rect (R op, screen coords)}"""
    O = ops(name); out = {}
    for i, t in enumerate(O):
        if t["op"] != "T": continue
        x0, y0, x1, y1 = t["ink"]; area = max(1, (x1 - x0) * (y1 - y0)); pts = set()
        for o in O[i + 1:]:
            if o["op"] != "R": continue
            for y in range(max(y0, o["y"]), min(y1, o["y"] + o["h"])):
                for x in range(max(x0, o["x"]), min(x1, o["x"] + o["w"])): pts.add((x, y))
        out[i] = len(pts) / area
    return out
def vtexts(name):
    O = ops(name); c = cover(name)
    return [t for i, t in enumerate(O) if t["op"] == "T" and c[i] < 1.0]
def hidden(name):
    O = ops(name); c = cover(name)
    return [(t, c[i]) for i, t in enumerate(O) if t["op"] == "T" and c[i] > 0]
