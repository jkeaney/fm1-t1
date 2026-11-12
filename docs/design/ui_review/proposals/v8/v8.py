# SPDX-License-Identifier: GPL-3.0-only
"""v8.py: the fix round after the independent critique of v7 (20/40). Self-contained (uses el.py's painter only).
Palette HI-CON (an existing firmware palette: black, mono=0, so coloured text stays coloured; v7 used a mock MONO
that the tool had mis-indexed). Channel colours: Okabe-Ito (designed to stay distinct for colour-blind viewers).
   python3 ../../mock/mock.py render v8.py --out out"""
import os, sys, copy
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "v5"))
sys.path.insert(0, HERE)
from mock import T, AF_S, AF_M, AF_L, text_w, rgb565
import el
from el import P, paint, IC, BLACK, mix
from icons_v8 import ICONS, BUTTONS

PALETTE = "HI-CON"
W = 240
CH = (rgb565(230, 159, 0), rgb565(86, 180, 233), rgb565(240, 228, 66), rgb565(204, 121, 167))  # orange sky yellow purple
el.CH_COL = CH
WHITE = rgb565(255, 255, 255)
INK = rgb565(0, 0, 0)
PHC = rgb565(80, 80, 80)                    # playhead column
DICE_BG = rgb565(56, 44, 10)
DICE_FG = rgb565(255, 200, 80)

import algos_v8 as A                             # every algorithm-specific thing comes from here
A.register("DEMO", [                             # an algorithm the screen code has never seen (extensibility demo, not a product algorithm)
    [dict(label="LOCK", min=0, max=16, **{"def": 9}, names=None, fmt=None, zero=None),
     dict(label="BITS", min=1, max=16, **{"def": 8}, names=None, fmt=None, zero=None),
     dict(label="SCALE", min=0, max=3, **{"def": 1}, names=["MAJ", "MIN", "DOR", "PENT"], fmt=None, zero=None),
     dict(label="SPRD", min=1, max=4, **{"def": 2}, names=None, fmt="%d OCT", zero=None)],
    [dict(label="OFFSET", min=0, max=15, **{"def": 0}, names=None, fmt=None, zero=None),
     dict(label="GATE", min=1, max=8, **{"def": 4}, names=None, fmt=None, zero=None),
     dict(label="DYN", min=0, max=10, **{"def": 10}, names=None, fmt=None, zero=None),
     dict(label="DIR", min=0, max=2, **{"def": 0}, names=["FWD", "BWD", "PEND"], fmt=None, zero=None)]], short="DEMO")


def L(s, acc=(), sl=()):
    return [None if c == "." else {"p": int(c, 36), "acc": n in acc, "sl": n in sl} for n, c in enumerate(s)]


CHANS = [
    {"algo": "ACID", "midi": 2, "lane": L("c.j.fh.mf.cj.hp.", acc=(0, 7, 13)), "muted": False},
    {"algo": "EUCLID", "midi": 10, "lane": L("0..0..0...0..0.."), "muted": False},
    {"algo": "SNAKE", "midi": 3, "lane": L("g.j.l.n.e..j.k.l"), "muted": True},
    {"algo": "STAGE", "midi": 4, "lane": L("7...b...e...b..."), "muted": False},
]


# ------------------------------------------------------------------ header: state, BPM, channels, algo, MIDI, preset
def header(p, S):
    y = 1
    if S.get("rec"):
        p.rect(4, y + 6, 9, 9, T.REC); p.rect(3, y + 7, 11, 7, T.REC); p.rect(5, y + 5, 7, 11, T.REC)
    elif S.get("playing", True):
        p.art(3, y + 5, ICONS["play_s"], WHITE)
    else:
        p.art(3, y + 5, ICONS["stop_s"], T.MID)
    p.text(16, y + 4, AF_M, str(S.get("bpm", 120)), T.TEXT, T.BG)
    plus = False
    for i in range(4):
        cx = 46 + i * 14
        c = S["chans"][i]
        if i in S["used"]:
            if c["muted"]:                            # muted: grey, struck through, in any selection state
                p.rect(cx, y, 13, 19, T.SURF)
                p.text_c(cx, 13, y + 4, AF_M, str(i + 1), T.MID, T.SURF)
                p.line(cx, y + 18, cx + 12, y, T.MID)
                if i == S["sel"]:
                    p.frame(cx, y, 13, 19, CH[i])
            elif i == S["sel"]:
                p.rect(cx, y, 13, 19, CH[i])
                p.text_c(cx, 13, y + 4, AF_M, str(i + 1), INK, CH[i])
            else:
                p.frame(cx, y, 13, 19, CH[i])
                p.text_c(cx, 13, y + 4, AF_M, str(i + 1), CH[i], T.BG)
        elif not plus:
            plus = True                               # SELECT past the last channel adds one
            p.text_c(cx, 13, y + 4, AF_M, "+", T.MID, T.BG)
    sel = S["sel"]
    c = S["chans"][sel]
    xe = p.text(105, y + 4, AF_M, A.ALGOS[c["algo"]]["short"], CH[sel], T.BG)
    xe = p.text(xe + 4, y + 6, AF_S, "MIDI", T.MID, T.BG)   # MIDI out channel of the selected channel ("--" = OFF)
    m = c.get("midi")
    p.text(xe + 2, y + 4, AF_M, "--" if m is None else str(m), WHITE if m else T.REC, T.BG)
    if S.get("pend"):
        p.frame(197, y - 1, 42, 21, WHITE)
        p.text_r(236, y + 4, AF_M, "P%02d?" % S["pend"], WHITE, T.BG)
    else:
        p.text_r(236, y + 4, AF_M, "P%02d" % S.get("preset", 1), T.TEXT, T.BG)


# ------------------------------------------------------------------ knob tiles: the tile is the meter
def knobs(p, S):
    sel = S["sel"]
    row = A.ALGOS[S["chans"][sel]["algo"]]["rows"][S.get("row", 0)]
    vals = S.get("vals", {})
    tr = S.get("turning")                             # (knob, old fraction)
    y = S.get("ky", 24)
    for k in range(4):
        x = k * 60 + 1
        kd = row[k]
        lab, val, fr = kd["label"], A.value_text(kd, vals.get(k)), A.frac(kd, vals.get(k))
        p.rect(x, y, 58, 16, T.SURF)                  # the label band never takes the fill
        p.text_c(x, 58, y + 3, AF_M, lab, T.MID, T.SURF)
        p.rect(x, y + 17, 58, 36, T.SURF)
        fh = 36 * fr // 100
        fc = mix(CH[sel], 120)
        p.rect(x, y + 53 - fh, 58, fh, fc)
        f = AF_L if text_w(AF_L, val) <= 54 else AF_M
        bgv = fc if fh >= 30 else T.SURF
        p.text_c(x, 58, y + 24 if f is AF_L else y + 30, f, val, WHITE, bgv)
        if tr and tr[0] == k:                         # turning: the tile itself answers; a white tick = where it was
            p.frame(x - 1, y - 1, 60, 55, CH[sel])
            p.frame(x, y, 58, 53, CH[sel])
            oh = 36 * tr[1] // 100
            p.rect(x + 1, y + 53 - oh - 1, 6, 2, WHITE)
            p.rect(x + 51, y + 53 - oh - 1, 6, 2, WHITE)
    rmk = S.get("row", 0)
    if S.get("no_marker"):
        return
    for r in range(3):
        (p.rect if r == rmk else p.frame)(109 + r * 8, 81, 5, 5, CH[sel] if r == rmk else T.MID)


# ------------------------------------------------------------------ music: one slim row, a fixed slot per channel
SLOTS = {1: [(0, 13)], 2: [(0, 6), (7, 6)], 3: [(0, 4), (5, 4), (10, 4)], 4: [(0, 3), (3, 3), (7, 3), (10, 3)]}


def music(p, S):
    y, h = 91, 33
    used = S["used"]
    ph = S.get("playhead", 5)
    playing = S.get("playing", True)
    ps = [s["p"] for ci in used for s in S["chans"][ci]["lane"]
          if s and A.ALGOS[S["chans"][ci]["algo"]]["lane"] != "single"]
    lo, hi = (min(ps), max(ps)) if ps else (0, 1)
    for i in range(16):
        x = i * 15 + 1
        if i == ph and playing:
            p.rect(x - 1, y, 15, h, PHC)
            p.rect(x - 1, y - 4, 15, 2, WHITE)        # the playhead: a lit column + a white bar over it
        any_on = False
        for k, ci in enumerate(used):
            s = S["chans"][ci]["lane"][i]
            ox, sw = SLOTS[len(used)][k]
            if not s:
                continue
            any_on = True
            single = A.ALGOS[S["chans"][ci]["algo"]]["lane"] == "single"
            bh = 6 if single else 4 + (h - 7) * (s["p"] - lo) // max(1, hi - lo)
            bx, by = x + ox, y + h - bh
            if S["chans"][ci]["muted"]:               # muted: grey with gaps (shape, not just colour)
                for r in range(0, bh, 2):
                    p.rect(bx, by + r, sw, 1, T.DIM)
                continue
            if S.get("diced") and ci == S["sel"] and i in S["diced"]:
                p.rect(bx, by, sw, bh, WHITE)
                p.hatch(bx, by, sw, bh, CH[ci], 2)
            else:
                p.rect(bx, by, sw, bh, CH[ci])
            if s.get("acc"):
                p.rect(bx, by - 3, sw, 2, WHITE)
        if not any_on:
            p.rect(x + 5, y + h - 1, 3, 1, T.DIM)
    for b in range(1, 4):
        p.rect(b * 60 - 1, y + h + 1, 1, 3, T.MID)


# ------------------------------------------------------------------ the 12 buttons
def buttons(p, S):
    y, ch = 132, 24
    lit = S.get("lit")
    for i, (pn, ic, role, hold, dice) in enumerate(BUTTONS):
        r, c = divmod(i, 6)
        x, yy = c * 40, y + r * (ch + 2)
        bg, fg = (DICE_BG, DICE_FG) if dice else (T.SURF, T.TEXT)
        if pn == "REC" and S.get("rec"):
            ic, fg = "rec", T.REC
        if lit == pn:
            bg, fg = WHITE, INK
        p.rect(x + 1, yy, 38, ch, bg)
        art = ICONS[ic]
        aw = max(len(r_) for r_ in art.strip("\n").split("\n"))
        ah = len(art.strip("\n").split("\n"))
        p.art(x + 1 + (38 - aw) // 2, yy + (ch - ah) // 2, art, fg)
        if hold:                                      # a corner notch: this button also has a HOLD action
            for d in range(4):
                p.rect(x + 35 - d, yy + d, d + 1, 1, T.MID if lit != pn else INK)


# ------------------------------------------------------------------ keybed in its mode
def keybed(p, S):
    y, kh, bh = 188, 34, 20
    sel = S["sel"]
    rec = S.get("rec")
    ph = S.get("playhead", 5)
    lane = S["chans"][sel]["lane"]
    for i in range(16):
        x = i * 15
        p.rect(x + 1, y, 14, kh, T.KEY if rec else T.SURF)
        if not rec and lane[i]:
            p.rect(x + 1, y + bh + 2, 14, kh - bh - 2, CH[sel])
        if i == ph and S.get("playing", True):
            p.rect(x + 1, y + kh, 14, 2, WHITE)  # playing step: a bar under the key, the key keeps its state
    for k, (cx, role) in enumerate(BLACK):
        x = cx - 5
        p.rect(x - 1, y, 12, bh + 1, T.BG)
        p.rect(x, y, 10, bh, T.RAISE if not rec else T.DIM)
        if rec:
            continue
        gy = y + 7
        if role[0] == "M":
            ci = int(role[1]) - 1
            if ci in S["used"]:
                if S["chans"][ci]["muted"]:
                    p.art(x + 2, gy, ICONS["x7"], T.MID)
                else:
                    p.rect(x + 2, gy, 6, 7, CH[ci])
        elif role == "CHN":
            p.art(x + 2, gy + 1, ICONS["midi_s"], T.TEXT)
        elif role in ("SOL", "FIL", "ROL"):
            p.text_c(x, 10, gy - 1, AF_S, role[0], T.TEXT, T.RAISE)
        elif role == "PG-":
            p.art(x + 3, gy, IC["left"], T.TEXT)
        elif role == "PG+":
            p.art(x + 3, gy, IC["right"], T.TEXT)
        elif role == "RST":
            p.art(x + 3, gy, IC["rst"], T.TEXT)
    ym = y + kh + 6
    if rec:
        p.rect(2, ym + 2, 5, 5, T.REC)
        p.text(10, ym - 1, AF_S, "REC: ALL KEYS PLAY NOTES", T.TEXT, T.BG)
    else:
        p.text(2, ym - 1, AF_S, "KEYS = STEPS", T.TEXT, T.BG)
        p.text_r(238, ym - 1, AF_S, "HOLD = EDIT  PAGE 1/2", T.MID, T.BG)


def toast(p, S):
    t = S.get("toast")
    if not t:
        return
    ic, word, hint = t
    p.rect(0, 0, W, 21, WHITE)
    p.art(4, 4, ICONS[ic], INK)
    p.text(22, 5, AF_M, word, INK, WHITE)
    p.text_r(236, 5, AF_M, hint, INK, WHITE)


def screen(p, S):
    header(p, S)
    knobs(p, S)
    music(p, S)
    buttons(p, S)
    keybed(p, S)
    toast(p, S)


def sheet(p, S):
    """every registered algorithm's two knob rows, drawn by the unchanged knobs() code (the extensibility proof)"""
    for n, name in enumerate(S["algos"]):
        for r in range(2):
            st = base(sel=0, row=r, ky=n * 58 + r * 0, no_marker=True)
            st["chans"][0]["algo"] = name
            if r == 1:
                st["ky"] = n * 58
            knobs(p, st) if r == S["row"] else None


def draw(cv, S):
    if S.get("sheet"):
        paint(cv, [(0, 116), (116, 232), (232, 240)], lambda p: sheet(p, S))
        return
    paint(cv, [(0, 120), (120, 240)], lambda p: screen(p, S))


def base(**kw):
    s = {"chans": copy.deepcopy(CHANS), "used": [0, 1, 2, 3], "sel": 0, "playing": True}
    s.update(kw)
    return s


STATE = {"scenes": {
    "playing": base(),
    "one_channel": base(used=[0], playing=False),
    "two_channels": base(used=[0, 1]),
    "sel_muted": base(sel=2),
    "dice": base(used=[0, 1, 3], lit="SCL", toast=("dice_notes", "NOTES DICED", "EDIT=UNDO"), diced={2, 4, 5, 8, 10, 13}),
    "turning_k2": base(used=[0, 1, 3], turning=(1, 30)),
    "rec_armed": base(used=[0, 1], rec=True, playing=False),
    "arp": base(used=[0, 1], **{}),
    "demo_algo": base(used=[0, 1, 3]),
    "sheet_row1_a": {"sheet": True, "row": 0, "algos": ["ACID", "EUCLID", "SNAKE", "STAGE"]},
    "sheet_row1_b": {"sheet": True, "row": 0, "algos": ["ARP", "PATTERNS", "DEMO"]},
    "sheet_row2_a": {"sheet": True, "row": 1, "algos": ["ACID", "EUCLID", "SNAKE", "STAGE"]},
    "sheet_row2_b": {"sheet": True, "row": 1, "algos": ["ARP", "PATTERNS", "DEMO"]},
    "euclid_midi16": base(used=[0, 1], sel=1),
    "midi_off": base(used=[0, 1], sel=1),
}}
STATE["scenes"]["arp"]["chans"][0]["algo"] = "ARP"
STATE["scenes"]["demo_algo"]["chans"][0]["algo"] = "DEMO"
STATE["scenes"]["euclid_midi16"]["chans"][1]["midi"] = 16
STATE["scenes"]["midi_off"]["chans"][1]["midi"] = None
