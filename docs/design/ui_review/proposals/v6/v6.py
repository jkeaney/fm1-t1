# SPDX-License-Identifier: GPL-3.0-only
"""v6.py: round 3. Owner picks: music = ONE slim row, each step split into 1-4 bars (one per channel playing there,
its colour, height = pitch); knobs = K-D (the tile is the meter); buttons = icons only; the keybed strip is gone and
the footer gets three options (F-1 mutes, F-2 last action + next, F-3 position).
   python3 ../../mock/mock.py render v6.py --out out"""
import os, sys, copy
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "v5"))
from mock import T, AF_S, AF_M, AF_L, text_w, rgb565
from el import *
import v5

PALETTE = "MONO"
PLAYHEAD = 5


def music_slim(p, y, h, S):
    used = S["used"]
    chans = S["chans"]
    ph = S.get("playhead", PLAYHEAD)
    rng = {}
    for ci in used:
        ns = [s["p"] for s in chans[ci]["lane"] if s]
        rng[ci] = (min(ns), max(ns))
    for i in range(16):
        x = i * 15 + 1
        if i == ph:
            p.rect(x - 1, y, 15, h, PH)
        on = [(ci, chans[ci]["lane"][i]) for ci in used if chans[ci]["lane"][i]]
        if not on:
            p.rect(x + 5, y + h - 2, 3, 1, T.DIM)
            continue
        k = len(on)
        bw = (13 - (k - 1)) // k
        for j, (ci, s) in enumerate(on):
            lo, hi = rng[ci]
            bh = 4 + (h - 6) * (s["p"] - lo) // max(1, hi - lo)
            bx = x + j * (bw + 1)
            if chans[ci]["muted"]:
                p.frame(bx, y + h - bh, bw, bh, T.DIM)
                continue
            col = WHITE if s.get("dice") else CH_COL[ci]
            p.rect(bx, y + h - bh, bw, bh, col)
            if s.get("dice"):
                p.hatch(bx, y + h - bh, bw, bh, CH_COL[ci], 2)
            if s.get("acc"):
                p.rect(bx, y + h - bh - 3, bw, 2, WHITE)
    for b in range(1, 4):
        p.rect(b * 60 - 1, y + h + 1, 1, 2, T.MID)


def row_marker(p, x, y, row, col):
    for r in range(3):
        (p.rect if r == row else p.frame)(x + r * 7, y, 5, 5, col if r == row else T.MID)


def footer_mutes(p, y, S):
    """F-1: one tile per channel in use: speaker in its colour (crossed + grey when muted), its number; the black
    key that mutes it is named by position (M1..M4 = the first four black keys)"""
    for ci in range(4):
        x = ci * 60 + 1
        if ci not in S["used"]:
            continue
        m = S["chans"][ci]["muted"]
        col = T.MID if m else CH_COL[ci]
        p.rect(x, y, 58, 50, T.RAISE)
        p.art(x + 8, y + 20, IC["spkx" if m else "spk"], col)
        p.text(x + 26, y + 14, AF_L, str(ci + 1), col, T.RAISE)
        if m:
            p.line(x + 2, y + 47, x + 55, y + 2, T.MID)


def footer_last(p, y, S):
    """F-2: a standing feedback line: the last thing that happened, and the next sensible action"""
    last = S.get("last", ("play", "PLAYING", "SCL DICE NOTES"))
    p.rect(0, y, W, 50, T.RAISE)
    p.art(6, y + 10, IC[last[0]], WHITE)
    p.text(22, y + 10, AF_M, last[1], WHITE, T.RAISE)
    xe = p.text(6, y + 31, AF_M, "NEXT", T.MID, T.RAISE)
    p.text(xe + 7, y + 31, AF_M, last[2], T.TEXT, T.RAISE)


def footer_pos(p, y, S):
    """F-3: where we are: bar.beat big, the page boxes, the length"""
    p.text(4, y + 14, AF_L, "3.2", WHITE, T.BG)
    p.text(64, y + 19, AF_M, "16 STEPS", T.MID, T.BG)
    for k in range(4):
        xx = 168 + k * 17
        (p.rect if k == 0 else p.frame)(xx, y + 19, 14, 10, T.MID) if k < 2 else None


FOOT = {"mutes": footer_mutes, "last": footer_last, "pos": footer_pos}


BK_X = (15, 30, 45, 75, 90, 120, 135, 150, 180, 195, 225)


def footer_keybed(p, y, S):
    """F-4: the keybed as it looks, in its CURRENT MODE. White keys = the 16 steps of the viewed page of the selected
    channel (lit in its colour = step on; the playing step outlined white); black keys carry their role as a glyph.
    The mode of each key row is named on the line under it (key_map.tsv: PERF white STEP tap / EDIT hold; REC NOTE)."""
    sel = S["sel"]
    col = CH_COL[sel]
    rec = S.get("rec")
    ph = S.get("playhead", PLAYHEAD)
    kh, bh = 34, 20
    lane = S["chans"][sel]["lane"]
    for i in range(16):
        x = i * 15
        on = (not rec) and lane[i] is not None
        p.rect(x + 1, y, 14, kh, T.KEY if rec else T.RAISE)
        if on:                                        # the step is on: the key's front (below the black keys) lights
            p.rect(x + 1, y + bh + 2, 14, kh - bh - 2, col)
        if i == ph and S.get("playing", True):
            p.frame(x + 1, y, 14, kh, WHITE)
            p.frame(x + 2, y + 1, 12, kh - 2, WHITE)
    for k, cx in enumerate(BK_X):
        x = cx - 5
        p.rect(x - 1, y, 12, bh + 1, T.BG)
        p.rect(x, y, 10, bh, T.LINE if not rec else T.DIM)
        if rec:
            continue
        role = BLACK[k][1]
        gy = y + 8
        if role[0] == "M":
            ci = int(role[1]) - 1
            if ci in S["used"]:
                m = S["chans"][ci]["muted"]
                c = T.MID if m else CH_COL[ci]
                p.rect(x + 2, gy, 6, 6, c) if not m else p.frame(x + 2, gy, 6, 6, c)
        elif role == "CHN":
            p.rect(x + 2, gy, 2, 2, CH_COL[0]); p.rect(x + 6, gy, 2, 2, CH_COL[1])
            p.rect(x + 2, gy + 4, 2, 2, CH_COL[2]); p.rect(x + 6, gy + 4, 2, 2, CH_COL[3])
        elif role in ("SOL", "FIL", "ROL"):
            p.text_c(x, 10, gy - 1, AF_S, role[0], T.TEXT, T.LINE)
        elif role == "PG-":
            p.art(x + 3, gy - 1, IC["left"], T.TEXT)
        elif role == "PG+":
            p.art(x + 3, gy - 1, IC["right"], T.TEXT)
        elif role == "RST":
            p.art(x + 3, gy - 1, IC["rst"], T.TEXT)
    ym = y + kh + 3
    if rec:
        p.art(2, ym + 1, IC["rec"], CH_COL[sel])
        p.text(16, ym, AF_S, "ALL KEYS PLAY NOTES", T.TEXT, T.BG)
    else:
        p.text(2, ym, AF_S, "KEYS = STEPS", T.TEXT, T.BG)
        p.text_r(238, ym, AF_S, "HOLD = EDIT  PAGE 1/2", T.MID, T.BG)


FOOT["keys"] = footer_keybed


def screen(p, S):
    sel = S["sel"]
    algo = S["chans"][sel]["algo"]
    v5.enc_row(p, 1, sel, S["used"], algo, 1, playing=S.get("playing", True), pend=S.get("pend"))
    kn = KN.get(algo, KN["ACID"])
    if S.get("turning"):
        v5.knob_turning(p, 24, sel, kn)
    else:
        v5.knobs_fill(p, 24, sel, kn, S.get("row", 0))
    row_marker(p, 109, 80, S.get("row", 0), CH_COL[sel])
    music_slim(p, 90, 34, S)
    v5.BTN_H[0] = 24
    v5.buttons(p, 132, "icon", S.get("lit"))
    if S.get("toast"):
        v5.toast_top(p, 0, *S["toast"])
    FOOT[S.get("foot", "mutes")](p, 188, S)


def draw(cv, S):
    paint(cv, [(0, 120), (120, 240)], lambda p: screen(p, S))


def S0(**kw):
    s = {"chans": copy.deepcopy(CHANS), "used": [0, 1, 3], "sel": 0}
    s["chans"][3]["muted"] = False
    s.update(kw)
    return s


def diced(**kw):
    s = S0(lit="SCL", toast=("note", "NOTES DICED", "EDIT=UNDO"), last=("note", "NOTES DICED", "EDIT UNDO"), **kw)
    for n in (2, 4, 5, 8, 10, 13):
        if s["chans"][0]["lane"][n]:
            s["chans"][0]["lane"][n]["dice"] = True
    return s


def muted4(**kw):
    s = S0(used=[0, 1, 2, 3], **kw)
    s["chans"][2]["muted"] = True
    return s


STATE = {"scenes": {
    "keys_perf": muted4(foot="keys"),
    "keys_rec": S0(foot="keys", rec=True),
    "keys_dice": diced(foot="keys"),
    "F1_mutes": muted4(foot="mutes"),
    "F2_last": S0(foot="last"),
    "F3_pos": S0(foot="pos"),
    "dice_F2": diced(foot="last"),
    "one_ch_F1": S0(used=[0], foot="mutes"),
}}
