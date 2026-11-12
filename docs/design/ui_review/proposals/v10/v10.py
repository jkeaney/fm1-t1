# SPDX-License-Identifier: GPL-3.0-only
"""v10.py: round after critique v8 (19/40) and the owner's three decisions: the selected channel gets the music row's
full width (others = thin hit strips), hold actions are named by a hint while held, SELECT past the last channel adds
one. The screen code names no algorithm: everything algorithm-specific comes from algos_v10 (one record per algorithm).
   python3 ../../mock/mock.py render v10.py --out out"""
import os, sys, copy
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "v5"))
sys.path.insert(0, HERE)
from mock import T, AF_S, AF_M, AF_L, text_w, rgb565
from el import P, paint, IC, BLACK, mix
from icons_v10 import ICONS, BUTTONS
import algos_v10 as A

PALETTE = "HI-CON"
W = 240
CH = (rgb565(230, 159, 0), rgb565(86, 180, 233), rgb565(240, 228, 66), rgb565(204, 121, 167))  # Okabe-Ito
WHITE, INK = rgb565(255, 255, 255), rgb565(0, 0, 0)
PHC = rgb565(80, 80, 80)
DICE_BG = rgb565(72, 72, 72)            # dice tiles: a lighter neutral grey (no hue: it must not read as a channel)
OFF_RED = rgb565(220, 40, 40)

# page-level knob rows that do not belong to an algorithm (knob_map.tsv REC rows, '*')
REC_ROW = [dict(label="CURSOR", min=1, max=16, names=None, fmt=None, zero=None, **{"def": 1}),
           dict(label="ACC", min=0, max=1, names=["OFF", "ON"], fmt=None, zero=None, **{"def": 0}),
           dict(label="SLIDE", min=0, max=1, names=["OFF", "ON"], fmt=None, zero=None, **{"def": 0}),
           dict(label="OCT", min=-2, max=2, names=["-2", "-1", "0", "+1", "+2"], fmt=None, zero=None, **{"def": 0})]

A.register("DEMO", [
    [dict(label="LOCK", min=0, max=16, names=None, fmt=None, zero=None, **{"def": 9}),
     dict(label="BITS", min=1, max=16, names=None, fmt=None, zero=None, **{"def": 8}),
     dict(label="SCALE", min=0, max=3, names=["MAJ", "MIN", "DOR", "PENT"], fmt=None, zero=None, **{"def": 1}),
     dict(label="SPRD", min=1, max=4, names=None, fmt="%d OCT", zero=None, **{"def": 2})],
    [dict(label="OFFSET", min=0, max=15, names=None, fmt=None, zero=None, **{"def": 0}),
     dict(label="GATE", min=1, max=8, names=None, fmt=None, zero=None, **{"def": 4}),
     dict(label="DYN", min=0, max=10, names=None, fmt=None, zero=None, **{"def": 10}),
     dict(label="DIR", min=0, max=2, names=["FWD", "BWD", "PEND"], fmt=None, zero=None, **{"def": 0})]],
    short="DEMO", absent=("ACCNT",))                  # DEMO has no accent layer: its ENV die is drawn absent


def L(s, acc=(), sl=()):
    return [None if c == "." else {"p": int(c, 36), "acc": n in acc, "sl": n in sl} for n, c in enumerate(s)]


CHANS = [
    {"algo": "ACID", "midi": 2, "lane": L("c.j.fh.mf.cj.hp.", acc=(0, 7, 13), sl=(4, 10)), "muted": False},
    {"algo": "EUCLID", "midi": 10, "lane": L("0..0..0...0..0.."), "muted": False},
    {"algo": "SNAKE", "midi": 3, "lane": L("g.j.l.n.e..j.k.l"), "muted": True},
    {"algo": "STAGE", "midi": 4, "lane": L("7...b...e...b..."), "muted": False},
]


def chan_off(c):
    return c.get("midi") is None


# ------------------------------------------------------------------ header (y 0..21)
def header(p, S):
    y = 1
    if S.get("rec"):
        p.rect(4, y + 6, 9, 9, T.REC); p.rect(3, y + 7, 11, 7, T.REC); p.rect(5, y + 5, 7, 11, T.REC)
        if S.get("playing"):
            p.rect(6, y + 8, 2, 5, WHITE); p.rect(8, y + 9, 2, 3, WHITE)   # playing while recording: a play mark in the dot
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
            sel = i == S["sel"]
            if c["muted"]:                            # muted: dark chip, grey digit, a diagonal (never a frame style)
                p.rect(cx, y, 13, 19, T.SURF)
                p.text_c(cx, 13, y + 4, AF_M, str(i + 1), T.MID, T.SURF)
                p.line(cx, y + 18, cx + 12, y, T.MID)
            elif sel:
                p.rect(cx, y, 13, 19, CH[i])
                p.text_c(cx, 13, y + 4, AF_M, str(i + 1), INK, CH[i])
            else:
                p.frame(cx, y, 13, 19, CH[i])
                p.text_c(cx, 13, y + 4, AF_M, str(i + 1), CH[i], T.BG)
            if sel:
                p.rect(cx, y + 20, 13, 2, WHITE)      # selection = a white bar under the chip, whatever its state
            if chan_off(c):
                p.rect(cx + 1, y + 1, 11, 3, OFF_RED)  # MIDI OFF: a red cap on the chip
        elif not plus:
            plus = True
            p.text_c(cx, 13, y + 4, AF_M, "+", T.MID, T.BG)
    c = S["chans"][S["sel"]]
    xe = p.text(105, y + 4, AF_M, A.ALGOS[c["algo"]]["short"], CH[S["sel"]], T.BG)
    if chan_off(c):                                   # loud: an inverted red chip that says what is wrong
        w = text_w(AF_S, "MIDI") + text_w(AF_M, "OFF") + 10
        p.rect(xe + 4, y, w, 19, OFF_RED)
        xx = p.text(xe + 7, y + 6, AF_S, "MIDI", WHITE, OFF_RED)
        p.text(xx + 3, y + 4, AF_M, "OFF", WHITE, OFF_RED)
    else:
        xx = p.text(xe + 5, y + 6, AF_S, "MIDI", T.MID, T.BG)
        p.text(xx + 2, y + 4, AF_M, str(c["midi"]), WHITE, T.BG)


# ------------------------------------------------------------------ knob tiles (y 24..76)
RIG_ROW = [dict(label="WIRE%d" % (i + 1), min=0, max=127, names=None, fmt=None, zero=None, **{"def": 64}) for i in range(4)]


def knob_row(S):
    if S.get("rec"):
        return REC_ROW
    if S.get("row", 0) == 2:
        return RIG_ROW
    return A.ALGOS[S["chans"][S["sel"]]["algo"]]["rows"][S.get("row", 0)]


def knobs(p, S):
    sel = S["sel"]
    row = knob_row(S)
    vals = S.get("vals", {})
    tr = S.get("turning")
    y = S.get("ky", 24)
    col = WHITE if S.get("rec") else CH[sel]
    for k in range(4):
        x = k * 60 + 1
        kd = row[k]
        v = vals.get(k)
        val = A.value_text(kd, v, S.get("algo_state"))
        p.rect(x, y, 58, 16, T.SURF)
        p.text_c(x, 58, y + 3, AF_M, kd["label"], T.MID, T.SURF)
        p.rect(x, y + 17, 58, 36, T.SURF)
        if A.is_list(kd):                             # a value list: position dots, no meter
            n = kd["max"] - kd["min"] + 1
            cur = (kd["def"] if v is None else v) - kd["min"]
            dw = min(8, (50 - (n - 1) * 2) // n)
            x0 = x + (58 - (n * dw + (n - 1) * 2)) // 2
            for d in range(n):
                (p.rect if d == cur else p.frame)(x0 + d * (dw + 2), y + 47, dw, 3, col if d == cur else T.DIM)
            bgv = T.SURF
        else:
            fh = 36 * A.frac(kd, v) // 100
            fc = mix(col, 120)
            p.rect(x, y + 53 - fh, 58, fh, fc)
            if fh >= 2:
                p.rect(x, y + 53 - fh, 7, 2, col)   # the level edge, full colour, at both sides (clear of the value)
                p.rect(x + 51, y + 53 - fh, 7, 2, col)
            bgv = fc if fh >= 30 else T.SURF
        f = AF_M if (v == 0 and kd["zero"]) or (v is None and kd["def"] == 0 and kd["zero"]) else \
            A.value_font(kd, text_w, AF_M, AF_L)
        p.text_c(x, 58, y + 24 if f is AF_L else y + 26, f, val, WHITE, bgv)
        if tr and tr[0] == k:
            p.frame(x - 1, y - 1, 60, 55, col)
            p.frame(x, y, 58, 53, col)
            if not A.is_list(kd):
                oh = 36 * tr[1] // 100
                p.rect(x + 1, y + 53 - oh - 1, 6, 2, WHITE)
                p.rect(x + 51, y + 53 - oh - 1, 6, 2, WHITE)


# ------------------------------------------------------------------ status line (y 79..90): preset | knob row | page
def status(p, S):
    y = 80
    if S.get("pend"):
        w = text_w(AF_M, "P%02d?" % S["pend"]) + 6
        p.rect(1, y - 2, w, 14, WHITE)
        p.text(4, y, AF_M, "P%02d?" % S["pend"], INK, WHITE)
    else:
        p.text(3, y, AF_M, "P%02d" % S.get("preset", 1), T.TEXT, T.BG)
    if not S.get("rec"):
        nrow = 3 if S.get("rig") else 2              # ROW 3 exists only when the channel has a RIG wire
        rm = S.get("row", 0)
        x0 = 120 - (nrow * 9) // 2
        for r in range(nrow):
            (p.rect if r == rm else p.frame)(x0 + r * 9, y + 2, 7, 7, CH[S["sel"]] if r == rm else T.MID)
    pages, vp = S.get("pages", 1), S.get("vpage", 1)
    for k in range(pages):
        xx = 238 - (pages - k) * 11
        (p.rect if k == vp - 1 else p.frame)(xx, y + 2, 9, 7, T.TEXT if k == vp - 1 else T.MID)


# ------------------------------------------------------------------ music: selected channel full width + hit strips
def music(p, S):
    y0, y1 = 96, 141
    sel = S["sel"]
    others = [ci for ci in S["used"] if ci != sel]
    strip_h = 3
    sh = len(others) * (strip_h + 1)
    bh_max = (y1 - y0) - sh - 1
    ph = S.get("playhead", 5)
    playing = S.get("playing", True)
    c = S["chans"][sel]
    a = A.ALGOS[c["algo"]]
    lane = c["lane"]
    ps = [s["p"] for s in lane if s]
    lo, hi = (min(ps), max(ps)) if ps else (0, 1)
    dim = c["muted"] or chan_off(c)
    for i in range(16):
        x = i * 15 + 1
        if i == ph and playing:
            p.rect(x - 1, y0 - 1, 15, y1 - y0 + 1, PHC)
            p.rect(x - 1, y0 - 3, 15, 2, WHITE)
        s = lane[i]
        if not s:
            p.rect(x + 5, y0 + bh_max - 1, 3, 1, T.DIM)
        else:
            bh = 8 if a["lane"] == "single" else 6 + (bh_max - 9) * (s["p"] - lo) // max(1, hi - lo)
            by = y0 + bh_max - bh
            if dim:                                   # muted or MIDI OFF: hollow, nothing is sounding
                p.frame(x, by, 13, bh, T.MID if c["muted"] else OFF_RED)
            elif S.get("diced") and i in S["diced"]:
                p.rect(x, by, 13, bh, WHITE)
                p.hatch(x, by, 13, bh, CH[sel], 2)
            else:
                p.rect(x, by, 13, bh, CH[sel])
            if s.get("acc") and not dim:
                p.rect(x, by - 4, 13, 2, WHITE)      # accent: a full-width white cap
            if s.get("sl") and i < 15 and lane[i + 1] and not dim:
                p.rect(x + 13, by + 2, 2, 3, CH[sel])  # slide: a bridge into the next step
        for k, ci in enumerate(others):              # the other channels: one thin hit strip each, in channel order
            o = S["chans"][ci]
            if o["lane"][i]:
                yy = y0 + bh_max + 2 + k * (strip_h + 1)
                if o["muted"] or chan_off(o):
                    p.rect(x, yy, 5, strip_h, T.DIM)  # a short grey dash: silent
                else:
                    p.rect(x, yy, 13, strip_h, CH[ci])


# ------------------------------------------------------------------ the 12 buttons (y 132..182)
def buttons(p, S):
    y, ch = 146, 17
    lit = S.get("lit")
    absent = A.ALGOS[S["chans"][S["sel"]]["algo"]].get("absent", ())
    for i, (pn, ic, role, hold, dice) in enumerate(BUTTONS):
        r, c = divmod(i, 6)
        x, yy = c * 40, y + r * (ch + 1)
        bg, fg = (DICE_BG, WHITE) if dice else (T.SURF, T.TEXT)
        if dice and role in absent:
            fg = T.DIM                                # the algorithm has no such layer: the die is greyed
        if pn == "REC" and S.get("rec"):
            ic, fg = "rec", T.REC
        if lit == pn:
            bg, fg = WHITE, INK
        p.rect(x + 1, yy, 38, ch, bg)
        art = ICONS[ic]
        rows = art.strip("\n").split("\n")
        p.art(x + 1 + (38 - max(map(len, rows))) // 2, yy + (ch - len(rows)) // 2, art, fg)


# ------------------------------------------------------------------ keybed in its mode (y 188..236)
def keybed(p, S):
    y, kh, bh = 186, 33, 19
    sel = S["sel"]
    rec = S.get("rec")
    ph = S.get("playhead", 5)
    lane = S["chans"][sel]["lane"]
    for i in range(16):
        x = i * 15
        p.rect(x + 1, y, 14, kh, T.KEY if rec else T.SURF)
        if not rec and lane[i]:
            p.rect(x + 1, y + bh + 2, 14, kh - bh - 2, CH[sel])
        if i == ph and S.get("playing", True) and not rec:
            p.rect(x + 1, y + kh, 14, 2, WHITE)
    for k, (cx, role) in enumerate(BLACK):
        x = cx - 5
        p.rect(x - 1, y, 12, bh + 1, T.BG)
        p.rect(x, y, 10, bh, T.RAISE)
        if rec:
            continue                                  # REC: plain playable keys
        gy = y + 7
        if role[0] == "M":
            ci = int(role[1]) - 1
            if ci in S["used"]:
                if S["chans"][ci]["muted"]:
                    p.art(x + 2, gy, ICONS["x7"], T.MID)
                else:
                    p.rect(x + 2, gy, 6, 7, CH[ci])
        elif role == "CHN":
            p.art(x + 2, gy + 1, ICONS["midi_s"], T.TEXT)       # CHN = channel setup (MIDI): the header's MIDI field is set here
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
        p.text(2, ym, AF_M, "PLAY KEYS TO WRITE", T.TEXT, T.BG)      # page_states.tsv REC cue_strip (REC is in the header)
        p.text_r(238, ym + 1, AF_S, "REC = DONE", T.MID, T.BG)
    else:
        p.text(2, ym, AF_M, "KEYS = STEPS", T.TEXT, T.BG)
        p.text_r(238, ym, AF_M, "HOLD = EDIT", T.MID, T.BG)


# ------------------------------------------------------------------ top bar messages: toast, hold hint
def topbar(p, S):
    t = S.get("toast")
    h = S.get("hold")
    if not (t or h):
        return
    if h:                                             # a hold, from 0.25 s: name it before it fires; the bar fills to the fire time
        icon, text, frac = h
        p.rect(0, 0, W, 22, T.TEXT)
        p.art(4, 4, ICONS[icon], INK)
        p.text(22, 5, AF_M, text, INK, T.TEXT)
        p.rect(0, 20, W * frac // 100, 2, OFF_RED if frac >= 100 else INK)
        return
    if t[0] == "enc":                                 # an encoder or OCT being touched: its printed name + what it does now
        _, name, what = t
        p.rect(0, 0, W, 22, T.TEXT)
        p.rect(2, 3, text_w(AF_S, name) + 6, 16, INK)
        p.text(5, 7, AF_S, name, T.TEXT, INK)
        p.text(text_w(AF_S, name) + 13, 5, AF_M, what, INK, T.TEXT)
        return
    icon, word, hint = t[:3]
    ci = t[3] if len(t) > 3 else None
    p.rect(0, 0, W, 22, WHITE)
    x = 4
    if ci is not None:                                # which channel it happened to: its chip
        p.rect(x, 2, 13, 18, CH[ci])
        p.text_c(x, 13, 6, AF_M, str(ci + 1), INK, CH[ci])
        x += 17
    p.art(x, 5, ICONS[icon], INK)
    xe = p.text(x + 17, 5, AF_M, word, INK, WHITE)
    p.text_r(236, 5, AF_M, hint, INK, WHITE)


def screen(p, S):
    header(p, S)
    knobs(p, S)
    status(p, S)
    music(p, S)
    buttons(p, S)
    keybed(p, S)
    topbar(p, S)


def sheet(p, S):
    for n, name in enumerate(S["algos"]):
        st = base(sel=0, row=S["row"], ky=n * 58)
        st["chans"][0]["algo"] = name
        knobs(p, st)


def draw(cv, S):
    if S.get("sheet"):
        paint(cv, [(0, 116), (116, 232), (232, 240)], lambda p: sheet(p, S))
        return
    paint(cv, [(0, 120), (120, 240)], lambda p: screen(p, S))


def base(**kw):
    s = {"chans": copy.deepcopy(CHANS), "used": [0, 1, 2, 3], "sel": 0, "playing": True, "pages": 2, "vpage": 1}
    s.update(kw)
    return s


def with_algo(s, ci, a):
    s["chans"][ci]["algo"] = a
    return s


def midi_off():
    s = base(used=[0, 1], sel=1, toast=("din", "MIDI OFF", "CHN=SET", 1))
    s["chans"][1]["midi"] = None
    return s


def added():
    s = base(used=[0, 1], sel=1, toast=("dice_all", "NEW", "ALGO=PICK", 1))
    s["chans"][1]["lane"] = [None] * 16
    s["chans"][1]["lane"][0] = {"p": 0, "acc": False, "sl": False}
    return s


STATE = {"scenes": {
    "playing": base(),
    "two_channels": base(used=[0, 1]),
    "one_channel": base(used=[0], playing=False, pages=1),
    "sel_muted": base(sel=2),
    "dice": base(used=[0, 1, 3], lit="SCL", toast=("dice_notes", "NOTES", "EDIT=UNDO", 0), diced={2, 4, 5, 8, 10, 13}),
    "turning_k2": base(used=[0, 1, 3], turning=(1, 30), vals={1: 6}),
    "hold_edit": base(used=[0, 1, 3], lit="EDIT", hold=("undo", "HOLD EDIT: RESET", 60)),
    "hold_home": base(used=[0, 1, 3], lit="HOME", hold=("home", "TOUCH=HELP  LET GO=MENU", 100)),
    "rec_armed": base(used=[0, 1], rec=True, playing=False),
    "midi_off": midi_off(),
    "channel_added": added(),
    "midi_off_after": dict(midi_off(), toast=None),
    "pending_p02": base(used=[0, 1, 3], pend=2, playhead=1),
    "turn_select": base(used=[0, 1, 3], sel=1, toast=("enc", "SELECT", "CH2  EUCLID")),
    "turn_algo": with_algo(base(used=[0, 1, 3], toast=("enc", "ALGO", "SNAKE AT NEXT STEP")), 0, "SNAKE"),
    "turn_presets": base(used=[0, 1, 3], pend=2, toast=("enc", "PRESETS", "P02 AT NEXT BAR")),
    "oct_row2": base(used=[0, 1, 3], row=1, toast=("enc", "OCT+", "ROW 2 OF 2")),
    "row3_rig": base(used=[0, 1, 3], row=2, rig=True, toast=("enc", "OCT+", "ROW 3: RIG")),
    "hold_glo": base(used=[0, 1, 3], lit="GLO", hold=("settings", "HOLD GLO: HARMONY", 60)),
    "hold_save": base(used=[0, 1, 3], lit="SAVE", hold=("save", "HOLD SAVE: UNDO LOAD", 80)),
    "rec_playing": base(used=[0, 1], rec=True, playing=True),
    "arp_row2": with_algo(base(used=[0, 1], row=1), 0, "ARP"),
    "demo_algo": with_algo(base(used=[0, 1, 3]), 0, "DEMO"),
    "stage_heard": with_algo(base(used=[0, 1], vals={1: 48}, algo_state={"heard": 32}), 0, "STAGE"),
    "sheet_row1_a": {"sheet": True, "row": 0, "algos": ["ACID", "EUCLID", "SNAKE", "STAGE"]},
    "sheet_row1_b": {"sheet": True, "row": 0, "algos": ["ARP", "PATTERNS", "DEMO"]},
    "sheet_row2_a": {"sheet": True, "row": 1, "algos": ["ACID", "EUCLID", "SNAKE", "STAGE"]},
    "sheet_row2_b": {"sheet": True, "row": 1, "algos": ["ARP", "PATTERNS", "DEMO"]},
}}
