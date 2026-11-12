import sys, os, copy
UR = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
V9 = os.path.join(UR, "proposals", "v9")
sys.path.insert(0, os.path.join(UR, "mock")); sys.path.insert(0, V9)
os.chdir(V9)
import mock
from mock import *
mod = mock.load_proposal(os.path.join(V9, "v9.py"))
import algos_v9 as A
LOG = []
_orig = mock.Canvas.text_flags
def _tf(self, x, y, f, s, fg, bg, flags=0):
    r = _orig(self, x, y, f, s, fg, bg, flags)
    if self.texts: 
        t = self.texts[-1]
        LOG.append(dict(s=s, font=getattr(f, 'name', f), fg=fg, bg=bg, box=t[:4], flags=t[5]))
    return r
mock.Canvas.text_flags = _tf
def scene(name):
    st = mod.STATE; s = {**{a: b for a, b in st.items() if a != "scenes"}, **st["scenes"][name]}
    return s
def render(s):
    del LOG[:]
    sc, cv = mock.render_state(mod, s)
    return sc, list(LOG)
def hexc(c):
    return "#%02x%02x%02x" % tuple(to_rgb888(c))
