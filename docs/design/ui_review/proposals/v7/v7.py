# SPDX-License-Identifier: GPL-3.0-only
"""v7.py: round 4: the reviewed icon set in the v6 screen, plus an icon review sheet.
   python3 ../../mock/mock.py render v7.py --out out"""
import os, sys, copy
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "v5"))
sys.path.insert(0, os.path.join(HERE, "..", "v6"))
sys.path.insert(0, HERE)
from mock import T, AF_S, AF_M, AF_L, text_w, rgb565
from el import *
import v5, v6
from icons_v7 import ICONS, BUTTONS

PALETTE = "MONO"


def buttons7(p, y, lit=None, playing=True, rec=False, ch=24):
    for i, (pn, ic, role, hold, dice) in enumerate(BUTTONS):
        r, c = divmod(i, 6)
        x, yy = c * 40, y + r * (ch + 2)
        bg, fg = (DICE_BG if dice else T.RAISE), (rgb565(255, 205, 90) if dice else T.TEXT)
        if pn == "PLAY" and playing:
            ic = "stop"                                  # PLAY shows what a press does now
        if pn == "REC" and rec:
            fg = T.REC
        if lit == pn:
            bg, fg = WHITE, INK
        p.rect(x + 1, yy, 38, ch, bg)
        p.art(x + 13, yy + (ch - 13) // 2 if ic.startswith("dice") else yy + (ch - 11) // 2, ICONS[ic], fg)


def screen(p, S):
    sel = S["sel"]
    algo = S["chans"][sel]["algo"]
    v5.enc_row(p, 1, sel, S["used"], algo, 1, playing=S.get("playing", True), pend=S.get("pend"))
    kn = KN.get(algo, KN["ACID"])
    if S.get("turning"):
        v5.knob_turning(p, 24, sel, kn)
    else:
        v5.knobs_fill(p, 24, sel, kn, S.get("row", 0))
    v6.row_marker(p, 109, 80, S.get("row", 0), CH_COL[sel])
    v6.music_slim(p, 90, 34, S)
    buttons7(p, 132, S.get("lit"), S.get("playing", True), S.get("rec", False))
    if S.get("toast"):
        v5.toast_top(p, 0, *S["toast"])
    v6.footer_keybed(p, 188, S)


def sheet(p, S):
    """the review: each button cell at 1x as drawn on the screen, in panel order, with its printed name"""
    for i, (pn, ic, role, hold, dice) in enumerate(BUTTONS):
        r, c = divmod(i, 6)
        x, y = c * 40, 4 + r * 46
        bg, fg = (DICE_BG if dice else T.RAISE), (rgb565(255, 205, 90) if dice else T.TEXT)
        p.rect(x + 1, y, 38, 24, bg)
        p.art(x + 13, y + (24 - 13) // 2 if ic.startswith("dice") else y + 6, ICONS[ic], fg)
        p.text_c(x, 40, y + 28, AF_S, pn, T.MID, T.BG)


def draw(cv, S):
    fn = sheet if S.get("sheet") else screen
    paint(cv, [(0, 120), (120, 240)], lambda p: fn(p, S))


STATE = {"scenes": {
    "icons": {"sheet": True},
    "playing": v6.muted4(foot="keys"),
    "dice": v6.diced(foot="keys"),
    "one_channel": v6.S0(used=[0], foot="keys"),
    "two_channels_stopped": v6.S0(used=[0, 1], foot="keys", playing=False),
    "turning_k2": v6.S0(foot="keys", turning=True),
    "rec_armed": v6.S0(foot="keys", rec=True),
}}
