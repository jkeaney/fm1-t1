# SPDX-License-Identifier: GPL-3.0-only
"""v5.py: round 2: section-by-section options (music, knobs, encoders, buttons, keys, feedback) and two fitted layouts.
   python3 ../../mock/mock.py render v5.py --out out ; python3 sheets.py"""
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from mock import T, AF_S, AF_M, AF_L, text_w, rgb565
from el import *

PALETTE = "MONO"
PLAYHEAD = 5


# ================================================================= MUSIC (all channels)
def music_lanes(p, x0, y, h, chans, sel, ph=PLAYHEAD):
    """M-A: one thin lane per channel in use, bar height = pitch"""
    n = len(chans)
    rh = (h - (n - 1)) // n
    for k, (ci, c) in enumerate(chans):
        yy = y + k * (rh + 1)
        if ci == sel:
            p.rect(0, yy, W, rh, T.RAISE)
        notes = [s["p"] for s in c["lane"] if s]
        lo, hi = min(notes), max(notes)
        for i, s in enumerate(c["lane"]):
            x = i * 15 + 2
            if i == ph:
                p.rect(x - 1, yy, 13, rh, PH)
            if not s:
                p.pset(x + 5, yy + rh - 2, T.DIM)
                continue
            bh = 2 + (rh - 4) * (s["p"] - lo) // max(1, hi - lo)
            col = T.DIM if c["muted"] else CH_COL[ci]
            (p.frame if c["muted"] else p.rect)(x, yy + rh - bh, 11, bh, col)


def music_shades(p, x0, y, h, chans, sel, ph=PLAYHEAD, legend=False):
    """M-B (owner's idea): ONE row; each step is a stack of cells, one per channel in use; the cell's SHADE is the
    pitch class (12 shades: C dark .. B bright); hatched = just diced, dotted = muted, white tick = accent,
    a bridge into the next step = slide; the playhead is a white bar under its step"""
    n = len(chans)
    ch_h = (h - 5 - (n - 1)) // n
    for i in range(16):
        x = i * 15 + 1
        if i == ph:
            p.rect(x - 1, y - 1, 15, n * (ch_h + 1) + 1, PH)
        for k, (ci, c) in enumerate(chans):
            s = c["lane"][i]
            yy = y + k * (ch_h + 1)
            if not s:
                p.rect(x + 5, yy + ch_h // 2, 3, 1, T.LINE)
                continue
            col = shade(ci, pr(s["p"]))
            if c["muted"]:
                p.frame(x, yy, 13, ch_h, T.DIM)
                p.dots(x + 1, yy + 1, 11, ch_h - 2, T.DIM)
                continue
            p.rect(x, yy, 13, ch_h, col)
            if s.get("sl") and i < 15:
                p.rect(x + 13, yy + 2, 2, ch_h - 4, col)
            if s.get("acc"):
                p.rect(x, yy, 4, 2, WHITE)
                p.rect(x, yy, 2, 4, WHITE)
            if s.get("dice"):
                p.hatch(x, yy, 13, ch_h, INK, 3)
    yb = y + n * (ch_h + 1) + 1
    p.rect(ph * 15 + 1, yb, 13, 3, WHITE)
    for b in range(4):
        p.rect(b * 60 + 1, yb + 1, 1, 1, T.MID)


def music_roll(p, x0, y, h, chans, sel, ph=PLAYHEAD):
    """M-C: a condensed piano roll of ALL channels: 12 pitch-class rows (B top .. C bottom), 16 steps; each note a
    cell in its channel's colour; two channels on one cell split it; black-key rows shaded like a piano roll"""
    rh = h // 12
    top = y + h - rh * 12
    for r in range(12):
        pc = 11 - r
        if pc in (1, 3, 6, 8, 10):
            p.rect(0, top + r * rh, W, rh, T.LANE)
    p.rect(ph * 15, top, 15, rh * 12, PH)
    for i in range(16):
        x = i * 15 + 1
        cells = {}
        for ci, c in chans:
            s = c["lane"][i]
            if s:
                cells.setdefault(pr(s["p"]), []).append((ci, c, s))
        for pc, lst in cells.items():
            yy = top + (11 - pc) * rh
            w = 13 // len(lst)
            for j, (ci, c, s) in enumerate(lst):
                xx = x + j * w
                if c["muted"]:
                    p.frame(xx, yy, w, rh, T.DIM)
                else:
                    p.rect(xx, yy, w, rh, CH_COL[ci])
                    if s.get("acc"):
                        p.rect(xx, yy, 2, rh, WHITE)
    for b in range(1, 4):
        p.rect(b * 60 - 1, top, 1, rh * 12, T.RAISE)


# ================================================================= KNOBS
def knobs_num(p, y, ci, kn, row=0, hot=None):
    """K-A: label + big value + meter (v4)"""
    for k in range(4):
        x = k * 60 + 1
        lab, val, fr = kn[row][k]
        p.rect(x, y, 58, 52, T.RAISE)
        p.text_c(x, 58, y + 4, AF_M, lab, T.MID, T.RAISE)
        p.text_c(x, 58, y + 20, AF_L, val, WHITE, T.RAISE)
        p.rect(x + 5, y + 46, 48, 3, T.LINE)
        p.rect(x + 5, y + 46, 48 * fr // 100, 3, CH_COL[ci])


def arc(p, cx, cy, r, t, frac, fg, bg):
    import math
    a0, a1 = math.radians(225), math.radians(-45)
    for rr in range(r - t + 1, r + 1):
        steps = int(rr * 5)
        for s in range(steps + 1):
            a = a0 + (a1 - a0) * s / steps
            x, y = cx + rr * math.cos(a), cy - rr * math.sin(a)
            p.pset(int(round(x)), int(round(y)), fg if s / steps <= frac else bg)


def knobs_arc(p, y, ci, kn, row=0):
    """K-B: a dial that looks like the knob: arc = position in the range, value inside, label under"""
    for k in range(4):
        x = k * 60 + 1
        lab, val, fr = kn[row][k]
        arc(p, x + 29, y + 22, 20, 3, fr / 100, CH_COL[ci], T.LINE)
        p.text_c(x, 58, y + 17, AF_M, val, WHITE, T.BG)
        p.text_c(x, 58, y + 44, AF_S, lab, T.MID, T.BG)


def knobs_tworow(p, y, ci, kn, row=0):
    """K-C: BOTH rows visible: the live row big, the other row small under it (OCT-/OCT+ swaps them)"""
    knobs_num(p, y, ci, kn, row)
    other = kn[1 - row] if len(kn) > 1 else None
    if other:
        for k in range(4):
            x = k * 60 + 1
            lab, val, fr = other[k]
            p.text(x + 3, y + 57, AF_S, lab, T.DIM if False else T.MID, T.BG)
            p.text_r(x + 56, y + 57, AF_S, val, T.MID, T.BG)


def knobs_fill(p, y, ci, kn, row=0):
    """K-D: the whole tile is the meter (fill height = value); label and value on it"""
    for k in range(4):
        x = k * 60 + 1
        lab, val, fr = kn[row][k]
        p.rect(x, y, 58, 52, T.RAISE)
        fh = 52 * fr // 100
        p.rect(x, y + 52 - fh, 58, fh, mix(CH_COL[ci], 110))
        p.text_c(x, 58, y + 5, AF_M, lab, WHITE, T.RAISE if fh < 47 else mix(CH_COL[ci], 110))
        p.text_c(x, 58, y + 22, AF_L, val, WHITE, T.RAISE if fh < 24 else mix(CH_COL[ci], 110))


def knob_turning(p, y, ci, kn, k=1, old="4", new="6", fr=75):
    """feedback while turning: the turned knob takes two cells: label, old -> new, full-range scale"""
    knobs_num(p, y, ci, kn, 0)
    x = 60 * min(k, 2)
    p.rect(x, y - 2, 120, 56, T.BG)
    p.frame(x, y - 2, 120, 56, CH_COL[ci])
    lab = kn[0][k][0]
    p.text(x + 6, y + 3, AF_M, "K%d %s" % (k + 1, lab), CH_COL[ci], T.BG)
    p.text(x + 6, y + 20, AF_M, old, T.MID, T.BG)
    p.art(x + 20, y + 21, IC["right"], T.MID)
    p.text(x + 32, y + 15, AF_L, new, WHITE, T.BG)
    for t in range(9):
        p.rect(x + 66 + t * 6, y + 40 - (4 if t % 4 == 0 else 2), 1, 4 if t % 4 == 0 else 2, T.MID)
    p.rect(x + 66, y + 43, 48, 3, T.LINE)
    p.rect(x + 66, y + 43, 48 * fr // 100, 3, CH_COL[ci])


# ================================================================= ENCODERS (left block of the panel)
def enc_block(p, x, y, sel_ci, used, algo, preset, pend=None):
    """E-A: the left 2x2 block, shaped like the panel: MASTER (volume) SELECT (channel chips) / PRESETS ALGO"""
    w, h = 58, 26
    # MASTER
    p.rect(x, y, w, h, T.RAISE)
    p.art(x + 4, y + 10, IC["vol"], T.MID)
    p.text(x + 18, y + 9, AF_S, "VOL", T.MID, T.RAISE)
    # SELECT: one chip per channel in use, + for the next free one
    xs = x + w + 2
    p.rect(xs, y, w, h, T.RAISE)
    for i in range(4):
        cx = xs + 3 + i * 14
        if i in used:
            if i == sel_ci:
                p.rect(cx, y + 4, 12, 18, CH_COL[i])
                p.text_c(cx, 12, y + 8, AF_M, str(i + 1), INK, CH_COL[i])
            else:
                p.frame(cx, y + 4, 12, 18, CH_COL[i])
                p.text_c(cx, 12, y + 8, AF_M, str(i + 1), CH_COL[i], T.RAISE)
        elif i == len(used):
            p.text_c(cx, 12, y + 8, AF_M, "+", T.MID, T.RAISE)
    # PRESETS
    y2 = y + h + 2
    p.rect(x, y2, w, h, T.RAISE)
    if pend:
        p.frame(x, y2, w, h, WHITE)
        p.text(x + 6, y2 + 8, AF_M, "P%02d?" % pend, WHITE, T.RAISE)
    else:
        p.text(x + 6, y2 + 8, AF_M, "P%02d" % preset, T.TEXT, T.RAISE)
    # ALGO
    p.rect(xs, y2, w, h, T.RAISE)
    p.art(xs + 4, y2 + 10, ALGO_IC[algo], CH_COL[sel_ci])
    p.text(xs + 17, y2 + 8, AF_S, algo[:6], CH_COL[sel_ci], T.RAISE)


def enc_row(p, y, sel_ci, used, algo, preset, playing=True, bpm=120, pend=None):
    """E-B: one header row: transport+BPM | channel chips (SELECT) | algo glyph+name (ALGO) | preset (PRESETS)"""
    (p.art if True else None)(3, y + 4, IC["play"] if playing else IC["rec"], WHITE if playing else T.MID)
    p.text(16, y + 4, AF_M, str(bpm), T.TEXT, T.BG)
    for i in range(4):
        cx = 50 + i * 15
        if i in used:
            if i == sel_ci:
                p.rect(cx, y, 13, 19, CH_COL[i])
                p.text_c(cx, 13, y + 4, AF_M, str(i + 1), INK, CH_COL[i])
            else:
                p.frame(cx, y, 13, 19, CH_COL[i])
                p.text_c(cx, 13, y + 4, AF_M, str(i + 1), CH_COL[i], T.BG)
        elif i == len(used):
            p.text_c(cx, 13, y + 4, AF_M, "+", T.MID, T.BG)
    p.art(114, y + 6, ALGO_IC[algo], CH_COL[sel_ci])
    p.text(127, y + 4, AF_M, algo[:6], CH_COL[sel_ci], T.BG)
    if pend:
        p.frame(196, y, 43, 19, WHITE)
        p.text_r(236, y + 4, AF_M, "P%02d?" % pend, WHITE, T.BG)
    else:
        p.text_r(236, y + 4, AF_M, "P%02d" % preset, T.TEXT, T.BG)


# ================================================================= BUTTONS (the 2x6 block)
BTN_H = [20]
def buttons(p, y, mode="icon", lit=None):
    """B-A icons only; B-B icon + printed name; B-C icon + role. Dice buttons share a tint (+ the dice shape)."""
    ch = (20 if mode != "role" else 26) if mode != "icon" else BTN_H[0]
    for i, (pn, ic, role, dice) in enumerate(BTN):
        r, c = divmod(i, 6)
        x, yy = c * 40, y + r * (ch + 2)
        bg = DICE_BG if dice else T.RAISE
        fg = T.TEXT
        if lit == pn:
            bg, fg = WHITE, INK
        p.rect(x + 1, yy, 38, ch, bg)
        if mode == "icon":
            p.art(x + 14, yy + (ch - 11) // 2 + 1, IC[ic], fg)
        elif mode == "printed":
            p.art(x + 4, yy + 5, IC[ic], fg)
            p.text(x + 17, yy + 6, AF_S, pn if len(pn) < 4 else pn[:4], T.MID if lit != pn else INK, bg)
        else:
            p.art(x + 14, yy + 3, IC[ic], fg)
            p.text_c(x + 1, 38, yy + 16, AF_S, role, T.MID if lit != pn else INK, bg)


# ================================================================= KEYS (the keybed as a ruler)
def keys(p, y, chans_all, used, page=1, pages=1, held=None):
    """the 16 white keys are the 16 step columns (15 px each, same as the music row); the 11 black keys sit between
    them in the 3-2-3-2-1 grouping with their role as a glyph; mute keys wear their channel's colour"""
    for i in range(16):
        p.rect(i * 15 + 1, y + 10, 13, 6, T.RAISE)
    for cx, role in BLACK:
        x = cx - 6
        p.rect(x, y, 12, 12, T.BG)
        p.rect(x + 1, y, 10, 11, T.LINE)
        if role[0] == "M":
            ci = int(role[1]) - 1
            if ci in used:
                m = chans_all[ci]["muted"]
                p.art(x + 1, y + 1, IC["spkx" if m else "spk"], T.MID if m else CH_COL[ci])
        elif role == "CHN":
            p.rect(x + 3, y + 2, 2, 2, CH_COL[0]); p.rect(x + 6, y + 2, 2, 2, CH_COL[1])
            p.rect(x + 3, y + 5, 2, 2, CH_COL[2]); p.rect(x + 6, y + 5, 2, 2, CH_COL[3])
        elif role in ("SOL", "FIL", "ROL"):
            p.text_c(x + 1, 10, y + 1, AF_S, role[0], WHITE if held == role else T.MID, T.LINE)
        elif role == "PG-":
            p.art(x + 3, y + 2, IC["left"], T.MID)
        elif role == "PG+":
            p.art(x + 3, y + 2, IC["right"], T.MID)
        elif role == "RST":
            p.art(x + 3, y + 2, IC["rst"], T.MID)
    for k in range(4):                                  # page boxes (1..4 pages of 16)
        xx = 196 + k * 11
        if k < pages:
            (p.rect if k == page - 1 else p.frame)(xx, y + 19, 9, 4, T.MID)


# ================================================================= FEEDBACK
def toast_top(p, y, icon, word, hint):
    """F-B: a 20 px slab over the header for ~1 s: icon, what happened, the next sensible action"""
    p.rect(0, y, W, 20, WHITE)
    p.art(4, y + 4, IC[icon], INK)
    xe = p.text(20, y + 5, AF_M, word, INK, WHITE)
    p.text_r(236, y + 5, AF_M, hint, INK, WHITE)


# ================================================================= scene plumbing
def used_chans(S):
    return [(i, c) for i, c in enumerate(S["chans"]) if i in S["used"]]


def draw(cv, S):
    fn = SCENES_FN[S["show"]]
    paint(cv, S.get("strips", [(0, 120), (120, 240)]), lambda p: fn(p, S))


def swatch_music(p, S):
    ch = used_chans(S)
    {"lanes": music_lanes, "shades": music_shades, "roll": music_roll}[S["opt"]](p, 0, 4, S.get("h", 64), ch, S["sel"])


def swatch_knobs(p, S):
    kn = KN[S["algo"]]
    o = S["opt"]
    if o == "num": knobs_num(p, 4, S["sel"], kn)
    if o == "arc": knobs_arc(p, 4, S["sel"], kn)
    if o == "tworow": knobs_tworow(p, 4, S["sel"], kn)
    if o == "fill": knobs_fill(p, 4, S["sel"], kn)
    if o == "turning": knob_turning(p, 6, S["sel"], kn)


def swatch_enc(p, S):
    if S["opt"] == "block":
        enc_block(p, 2, 4, S["sel"], S["used"], "ACID", 1, S.get("pend"))
    else:
        enc_row(p, 4, S["sel"], S["used"], "ACID", 1, pend=S.get("pend"))


def swatch_buttons(p, S):
    buttons(p, 4, S["opt"], S.get("lit"))


def swatch_keys(p, S):
    keys(p, 4, S["chans"], S["used"], 1, 2, S.get("held"))


def swatch_toast(p, S):
    toast_top(p, 4, "note", "NOTES DICED", "EDIT=UNDO")


# ------------------------------------------------------------------ fitted layouts
def layout_A(p, S):
    """A 'owner grid': header (encoders as one row) / knobs (both rows) / music (shade stack) / 12 buttons / keybed"""
    ch = used_chans(S)
    sel = S["sel"]
    enc_row(p, 1, sel, S["used"], S["chans"][sel]["algo"], 1, pend=S.get("pend"))
    if S.get("toast"):
        toast_top(p, 0, *S["toast"])
    kn = KN.get(S["chans"][sel]["algo"], KN["ACID"])
    if S.get("turning"):
        knob_turning(p, 24, sel, kn)
    else:
        knobs_tworow(p, 24, sel, kn, S.get("row", 0))
    music_shades(p, 0, 93, 58, ch, sel, legend=False)
    buttons(p, 156, "icon", S.get("lit"))
    keys(p, 204, S["chans"], S["used"], 1, 2, S.get("held"))
    # thin labels for the knob row marker
    for r in range(2):
        (p.rect if r == S.get("row", 0) else p.frame)(232, 28 + r * 0, 0, 0, T.MID)


def layout_B(p, S):
    """B 'panel mirror': encoders as the left 2x2 block beside the knobs (as on the panel), piano roll in the middle,
    buttons as icons, keybed at the bottom"""
    ch = used_chans(S)
    sel = S["sel"]
    p.art(3, 4, IC["play"], WHITE)
    p.text(16, 4, AF_M, "120", T.TEXT, T.BG)
    p.text_r(236, 4, AF_M, "BAR 3.2", T.MID, T.BG)
    if S.get("toast"):
        toast_top(p, 0, *S["toast"])
    enc_block(p, 0, 22, sel, S["used"], S["chans"][sel]["algo"], 1, S.get("pend"))
    kn = KN.get(S["chans"][sel]["algo"], KN["ACID"])
    for k in range(4):                                   # compact knobs to the right of the encoder block (2x2)
        r, c = divmod(k, 2)
        x, y = 120 + c * 60, 22 + r * 28
        lab, val, fr = kn[0][k]
        p.rect(x + 1, y, 58, 26, T.RAISE)
        p.text(x + 4, y + 4, AF_S, lab, T.MID, T.RAISE)
        p.text_r(x + 56, y + 12, AF_M, val, WHITE, T.RAISE)
        p.rect(x + 4, y + 22, 52 * fr // 100, 2, CH_COL[sel])
    music_roll(p, 0, 80, 72, ch, sel)
    buttons(p, 156, "icon", S.get("lit"))
    keys(p, 204, S["chans"], S["used"], 1, 2, S.get("held"))


SCENES_FN = {"music": swatch_music, "knobs": swatch_knobs, "enc": swatch_enc, "buttons": swatch_buttons,
             "keys": swatch_keys, "toast": swatch_toast, "A": layout_A, "B": layout_B}


def S0(**kw):
    import copy
    s = {"chans": copy.deepcopy(CHANS), "used": [0, 1, 2, 3], "sel": 0, "algo": "ACID"}
    s.update(kw)
    return s


def diced():
    s = S0(show="A", used=[0, 1, 3], lit="SCL", toast=("note", "NOTES DICED", "EDIT=UNDO"))
    for n in (2, 4, 5, 8, 10, 13):
        if s["chans"][0]["lane"][n]:
            s["chans"][0]["lane"][n]["dice"] = True
    return s


STATE = {"scenes": {
    "music_lanes": S0(show="music", opt="lanes"),
    "music_shades": S0(show="music", opt="shades", h=46),
    "music_shades2": S0(show="music", opt="shades", h=46, used=[0, 1]),
    "music_roll": S0(show="music", opt="roll", h=60),
    "knobs_num": S0(show="knobs", opt="num"),
    "knobs_arc": S0(show="knobs", opt="arc"),
    "knobs_tworow": S0(show="knobs", opt="tworow"),
    "knobs_fill": S0(show="knobs", opt="fill"),
    "knobs_turning": S0(show="knobs", opt="turning"),
    "enc_block": S0(show="enc", opt="block", used=[0, 1]),
    "enc_row": S0(show="enc", opt="row", used=[0, 1]),
    "enc_row_pending": S0(show="enc", opt="row", used=[0, 1], pend=2),
    "buttons_icon": S0(show="buttons", opt="icon"),
    "buttons_printed": S0(show="buttons", opt="printed"),
    "buttons_role": S0(show="buttons", opt="role"),
    "buttons_lit": S0(show="buttons", opt="icon", lit="SCL"),
    "keys": S0(show="keys"),
    "toast": S0(show="toast"),
    "A_1ch": S0(show="A", used=[0]),
    "A_3ch": S0(show="A", used=[0, 1, 3]),
    "A_diced": diced(),
    "A_turning": S0(show="A", used=[0, 1, 3], turning=True),
    "B_3ch": S0(show="B", used=[0, 1, 3]),
    "B_pending": S0(show="B", used=[0, 1, 3], pend=2),
}}
