# SPDX-License-Identifier: GPL-3.0-only
"""evidence_b_lib.py: shared helpers for the EVIDENCE_B_v3 scripts (renders proposal_v3 scenes and fixtures, records every text drawn).
Run from the repo root with .venv/bin/python."""
import copy, io, os, sys, math, contextlib
ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "..", ".."))
MOCKDIR = os.path.join(ROOT, "docs", "design", "ui_review", "mock")
PROP = os.path.join(ROOT, "docs", "design", "ui_review", "proposals", "v3-glance")
sys.path.insert(0, MOCKDIR)
import mock
from mock import T, AF_S, AF_M, AF_L, text_w, text_ink, contrast_ratio, rgb565, to_rgb888
P = mock.load_proposal(os.path.join(PROP, "proposal_v3.py"))

REC = []                      # every tx() call: dict(role, s, font, fg, bg, x, ycap, mode, w)
_orig_tx = P.tx
def _tx(cv, role, f, s, fg, bg, x, ycap, mode="l", w=0):
    REC.append(dict(scene=P.CUR[0], role=role, s=s, f=f.name, fg=fg, bg=bg, x=x, ycap=ycap, mode=mode, w=w, f_obj=f))
    return _orig_tx(cv, role, f, s, fg, bg, x, ycap, mode, w)
P.tx = _tx

def render(state, name="x", palette="GREY"):
    """-> (screen, canvas-warnings list, REC entries of this render with their ink boxes)"""
    state = copy.deepcopy(state)
    state["_name"] = name
    REC.clear()
    buf = io.StringIO()
    with contextlib.redirect_stdout(buf):
        sc, cv = mock.render_state(P, state, palette)
    warns = [l.strip() for l in buf.getvalue().splitlines() if l.strip()]
    recs = [dict(r) for r in REC]
    # screen.texts are in draw order (blit order A,B,C); the REC order is draw order too
    inks = list(sc.texts)
    assert len(inks) == len(recs), (len(inks), len(recs), name)
    for r, t in zip(recs, inks):
        assert r["s"] == t[4] or t[5] & 1, (r["s"], t[4])
        r["ink"] = t[:4]; r["flags"] = t[5]
    return sc, warns, recs

def save(sc, outdir, base, k=4):
    os.makedirs(outdir, exist_ok=True)
    img = sc.to_image()
    img.save(os.path.join(outdir, base + ".png"))
    mock._scale(img, k).save(os.path.join(outdir, base + "_4x.png"))
    return img

def scene(name):
    return copy.deepcopy(P.STATE["scenes"][name])

def arcmin(px):
    mm = px / 8.0
    return mm, mm / 500.0 * 3438.0

def cr(a, b):
    return contrast_ratio(a, b)
