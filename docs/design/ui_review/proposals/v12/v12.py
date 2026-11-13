# SPDX-License-Identifier: GPL-3.0-only
"""v12.py: the accepted v11 direction (owner: "good enough, move on") + the remaining states. Originally v11.py: round after critique v8 (19/40) and the owner's three decisions: the selected channel gets the music row's
full width (others = thin hit strips), hold actions are named by a hint while held, SELECT past the last channel adds
one. The screen code names no algorithm: everything algorithm-specific comes from algos_v12 (one record per algorithm).
   python3 ../../mock/mock.py render v12.py --out out"""
import os, sys, copy
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "v5"))
sys.path.insert(0, HERE)
from mock import T, AF_S, AF_M, AF_L, text_w, rgb565
from el import P, paint, IC, BLACK, mix
from icons_v12 import ICONS, BUTTONS
import algos_v12 as A

PALETTE = "HI-CON"
W = 240
CH = (rgb565(230, 159, 0), rgb565(86, 180, 233), rgb565(0, 158, 115), rgb565(204, 121, 167))  # Okabe-Ito
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
    clk = S.get("clock", "INT")                       # tempo source (TIMING_SPEC 5): INT / EXT n / EXT -- (lost) / WAIT
    if clk == "INT":
        p.text(16, y + 4, AF_M, str(S.get("bpm", 120)), T.TEXT, T.BG)
    else:                                             # external: two small lines, EXT over the tempo (or WAIT / --)
        p.text(16, y, AF_S, "EXT", T.MID, T.BG)
        low = {"EXT": str(S.get("bpm", 120)), "WAIT": "WAIT", "LOST": "--"}[clk]
        p.text(16, y + 10, AF_S, low, T.REC if clk == "LOST" else T.TEXT, T.BG)
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
DEVICES = {   # a device profile's wire set supplies ROW 3 (HwSet: four parameter names); another device = another entry
    "TB-3 BITE": [("CUT", 64), ("RESO", 43), ("ENV-X", 64), ("ENV-Y", 64)],
}


def rig_row(S):
    return [dict(label=n, min=0, max=127, names=None, fmt=None, zero=None, **{"def": v})
            for n, v in DEVICES[S.get("rig", "TB-3 BITE")]]


CHAN_ROW = [dict(label="LENGTH", min=1, max=64, names=None, fmt=None, zero=None, **{"def": 16}),
            dict(label="MIDI", min=1, max=17, names=[str(i) for i in range(1, 17)] + ["OFF"], fmt=None, zero=None, **{"def": 2}),
            dict(label="SOURCE", min=0, max=0, names=["OWN"], fmt=None, zero=None, **{"def": 0}),
            dict(label="FOLLOW", min=0, max=1, names=["OFF", "ON"], fmt=None, zero=None, **{"def": 1})]


def _kd(label, lo, hi, d, names=None, fmt=None):
    return dict(label=label, min=lo, max=hi, names=names, fmt=fmt, zero=None, **{"def": d})


# SET (GLO tap, P4-016; knob_map SET rows): BPM SWING GATE DIV, all channels. While an external clock runs K1 is a read-only mirror: label HOST,
# no fill, a dashed frame, the host tempo (or -- while it is unknown) in MID.
SET_ROW = [_kd("BPM", 40, 240, 120), _kd("SWING", 50, 75, 50, fmt="%d%%"), _kd("GATE", 10, 100, 50, fmt="%d%%"),
           _kd("DIV", 0, 5, 2, names=["1/32", "1/16T", "1/16", "1/8T", "1/8", "1/4"])]
HOST_TILE = _kd("HOST", 40, 240, 120)


def ext(S):
    return S.get("clock", "INT") != "INT"


def knob_row(S):
    if S.get("rec"):
        return REC_ROW
    if S.get("page") == "SET":
        return [HOST_TILE] + SET_ROW[1:] if ext(S) else SET_ROW
    if S.get("page") == "CHAN":
        return CHAN_ROW
    if S.get("row", 0) == 2:
        return rig_row(S)
    return A.ALGOS[S["chans"][S["sel"]]["algo"]]["rows"][S.get("row", 0)]


def knobs(p, S):
    S0 = S
    sel = S["sel"]
    row = knob_row(S)
    vals = S.get("vals", {})
    if S.get("page") == "SET":
        vals = {0: S.get("bpm", 120), **vals}         # K1 shows the tempo
        if ext(S):
            vals = {k: v for k, v in vals.items() if k}
            if S.get("turning") and S["turning"][0] == 0:
                S = dict(S, turning=None)                   # the white frame of a touched mirror is drawn below
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
        if text_w(f, val) > 54:
            f = AF_S
        p.text_c(x, 58, y + 24 if f is AF_L else y + 26, f, val, WHITE, bgv)
        if tr and tr[0] == k:
            p.frame(x - 1, y - 1, 60, 55, col)
            p.frame(x, y, 58, 53, col)
            if not A.is_list(kd):
                oh = 36 * tr[1] // 100
                p.rect(x + 1, y + 53 - oh - 1, 6, 2, WHITE)
                p.rect(x + 51, y + 53 - oh - 1, 6, 2, WHITE)
    if S.get("page") == "SET" and ext(S):
        host_tile(p, S0)


def host_tile(p, S):
    """SET, external clock: K1 is a mirror, not a control (the shape changes, not only the colour)"""
    x, y = 1, S.get("ky", 24)
    p.rect(x, y + 17, 58, 36, T.SURF)
    for i in range(0, 58, 6):                           # a dashed frame: nothing to turn here
        p.rect(x + i, y + 17, 3, 1, T.MID)
        p.rect(x + i, y + 52, 3, 1, T.MID)
    for j in range(17, 53, 6):
        p.rect(x, y + j, 1, 3, T.MID)
        p.rect(x + 57, y + j, 1, 3, T.MID)
    host = str(S.get("bpm", 120)) if S["clock"] == "EXT" else "--"
    p.text_c(x, 58, y + 24, AF_L, host, T.MID, T.SURF)
    tr = S.get("turning")
    if tr and tr[0] == 0:                               # K1 touched: a white frame (it is not a channel value)
        p.frame(x - 1, y - 1, 60, 55, WHITE)
        p.frame(x, y, 58, 53, WHITE)


# ------------------------------------------------------------------ status line (y 79..90): preset | knob row | page
def clock_out(S):
    """the clock-out word (SCREEN_SPEC 6.4): MASTER only while playing on the own clock; ON PLAY when stopped; FOLLOW on any external source"""
    if ext(S):
        return "FOLLOW"
    return "MASTER" if S.get("playing", True) else "ON PLAY"


def status_set(p, S, y):
    """SET: [preset] [EXT USB] OUT word ... OCT n (ROW and PAGE mean nothing here)"""
    blocks = ([("EXT", S.get("port", "USB"))] if ext(S) else []) + [("OUT", clock_out(S))]
    x = 42
    for a, b in blocks:                                  # label AF_S MID + word AF_M: the pairing of `MIDI n` and `PAGE v/n`
        xa = p.text(x, y + 2, AF_S, a, T.MID, T.BG)
        x = p.text(xa + 3, y, AF_M, b, T.TEXT, T.BG) + 7
    o = S.get("octave", 0)
    ot = "%+d" % o if o else "0"
    assert x - 7 < 236 - text_w(AF_M, ot) - text_w(AF_S, "OCT") - 4, "clock-out blocks run into the OCT readout"
    p.text_r(238, y, AF_M, ot, T.TEXT, T.BG)
    p.text_r(236 - text_w(AF_M, ot), y + 2, AF_S, "OCT", T.MID, T.BG)


def status(p, S):
    y = 80
    if S.get("pend"):
        w = text_w(AF_M, "P%02d?" % S["pend"]) + 6
        p.rect(1, y - 2, w, 14, WHITE)
        p.text(4, y, AF_M, "P%02d?" % S["pend"], INK, WHITE)
    else:
        p.text(3, y, AF_M, "P%02d" % S.get("preset", 1), T.TEXT, T.BG)
    if S.get("page") == "SET":
        status_set(p, S, y)
        return
    if not S.get("rec"):
        nrow = 3 if S.get("rig") else 2              # ROW 3 exists only when the channel has a RIG wire
        rm = S.get("row", 0)
        x0 = 128 - (nrow * 13) // 2
        p.text_r(x0 - 3, y + 2, AF_S, "ROW", T.MID, T.BG)
        for r in range(nrow):                         # the knob row, numbered: the live row filled
            bx = x0 + r * 13
            if r == rm:
                p.rect(bx, y - 1, 11, 13, CH[S["sel"]])
                p.text_c(bx, 11, y + 1, AF_S, str(r + 1), INK, CH[S["sel"]])
            else:
                p.frame(bx, y - 1, 11, 13, T.MID)
                p.text_c(bx, 11, y + 1, AF_S, str(r + 1), T.MID, T.BG)
    pages, vp = S.get("pages", 1), S.get("vpage", 1)
    pt = "%d/%d" % (vp, pages)
    if S.get("follow_off"):                           # FOLLOW off: the viewed page is boxed; the playing page is named
        w = text_w(AF_M, pt) + 4
        p.frame(238 - w, y - 2, w + 1, 14, WHITE)
        p.text_r(237, y, AF_M, pt, T.TEXT, T.BG)
        pl = "PLAY %d" % S.get("ppage", 2)
        p.text_r(234 - w, y + 2, AF_S, pl, T.TEXT, T.BG)
    else:
        p.text_r(238, y, AF_M, pt, T.TEXT, T.BG)
        p.text_r(236 - text_w(AF_M, pt), y + 2, AF_S, "PAGE", T.MID, T.BG)


# ------------------------------------------------------------------ music: selected channel full width + hit strips
def rec_cursor(p, S):
    if S.get("rec"):
        c = S.get("cursor", 1) - 1
        x = c * 15 + 1
        p.frame(x - 2, 93, 17, 49, WHITE)            # REC: the write cursor (K1 CURSOR) frames its step
        p.frame(x - 1, 94, 15, 47, WHITE)


def music(p, S):
    y0, y1 = 96, 141
    sel = S["sel"]
    others = [ci for ci in S["used"] if ci != sel]
    strip_h = 3
    nrows = 4 if others else 0                        # a fixed row per channel number (CH1..CH4); own/unused rows stay empty
    sh = nrows * (strip_h + 1)
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
        if S.get("length") is not None and i >= S["length"]:   # past LENGTH: a hatched, empty step
            p.hatch(x, y0, 13, y1 - y0, T.LINE, 3)
            continue
        if i == ph and playing and not S.get("follow_off"):
            p.rect(x - 1, y0 - 1, 15, y1 - y0 + 1, PHC)
            for d in range(3):
                p.rect(x + 3 + d, y0 - 4 + d, 7 - 2 * d, 1, WHITE)   # the playhead: a lit column + a white pointer
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
        for ci in others:                             # the other channels: their hit strip in their own fixed row
            o = S["chans"][ci]
            if o["lane"][i]:
                yy = y0 + bh_max + 2 + ci * (strip_h + 1)
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


def setup_cue(S):
    c = S["chans"][S["sel"]]
    return S.get("new_chan") or chan_off(c)


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
        if S.get("length") is not None and i >= S["length"]:
            p.hatch(x + 1, y, 14, kh, T.LINE, 3)   # this key is past LENGTH
            continue
        if not rec and lane[i]:
            p.rect(x + 1, y + bh + 2, 14, kh - bh - 2, CH[sel])
        if i == ph and S.get("playing", True) and not rec and not S.get("follow_off"):
            p.rect(x + 1, y + kh, 14, 2, WHITE)
    for k, (cx, role) in enumerate(BLACK):
        x = cx - 5
        p.rect(x - 1, y, 12, bh + 1, T.BG)
        p.rect(x, y, 10, bh, T.RAISE)
        if rec:
            continue                                  # REC: plain playable keys
        gy = y + 7
        if role[0] == "M" and S.get("held_m") == int(role[1]) - 1:
            p.rect(x, y, 10, bh, WHITE)                   # the held mute key
            p.rect(x + 2, gy, 6, 7, CH[S["held_m"]])
        elif role[0] == "M":
            ci = int(role[1]) - 1
            if ci in S["used"]:
                if S["chans"][ci]["muted"]:
                    p.art(x + 2, gy, ICONS["x7"], T.MID)
                else:
                    p.rect(x + 2, gy, 6, 7, CH[ci])
        elif role == "CHN" and S.get("page") == "CHAN":
            p.rect(x, y, 10, bh, WHITE)
            p.art(x + 3, gy, IC["left"], INK)            # on the CHAN page D#4 = BACK
        elif role == "CHN":
            if setup_cue(S):
                p.rect(x, y, 10, bh, WHITE)
                p.art(x + 2, gy + 1, ICONS["midi_s"], INK)
            else:
                p.art(x + 2, gy + 1, ICONS["midi_s"], T.TEXT)       # CHN = channel setup (MIDI): the header's MIDI field is set here
        elif role in ("SOL", "FIL", "ROL"):
            if S.get("held") == role:                     # a held move: its key inverted
                p.rect(x, y, 10, bh, WHITE)
                p.text_c(x, 10, gy - 1, AF_S, role[0], INK, WHITE)
            else:
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
    elif S.get("page") == "SET":                      # SET: the clock state is the footer (INT: the keys line + the way back)
        c = S.get("clock", "INT")
        left = {"INT": "KEYS = STEPS", "EXT": "HOST SETS TEMPO", "WAIT": "WAIT FOR HOST START", "LOST": "CLOCK LOST: STOPPED"}[c]
        right = {"INT": "GLO = BACK", "EXT": "GLO = BACK", "WAIT": "PLAY=CANCEL", "LOST": "PLAY=START"}[c]
        fl = AF_M if c in ("INT", "EXT") else AF_S
        fr = AF_M if c in ("INT", "EXT") else AF_S
        assert text_w(fl, left) + text_w(fr, right) + 12 <= W, (left, right)
        p.text(2, ym + (0 if fl is AF_M else 2), fl, left, T.REC if c == "LOST" else T.TEXT, T.BG)
        p.text_r(238, ym + (0 if fr is AF_M else 2), fr, right, T.MID, T.BG)
    elif S.get("page") == "CHAN":
        p.text(2, ym, AF_M, "K2 = MIDI", T.TEXT, T.BG)
        p.text_r(238, ym, AF_M, "CHN = BACK", T.MID, T.BG)
    elif setup_cue(S):                                # the one standing cue: how to give this channel its MIDI channel
        p.rect(0, ym - 3, W, 16, WHITE)
        p.text(4, ym, AF_M, "CHN KEY, K2 = MIDI", INK, WHITE)
        p.text_r(236, ym, AF_M, "CH%d" % (S["sel"] + 1), INK, WHITE)
    else:
        p.text(2, ym, AF_M, "KEYS = STEPS", T.TEXT, T.BG)
        p.text_r(238, ym, AF_M, "HOLD = EDIT", T.MID, T.BG)


# ------------------------------------------------------------------ top bar messages: toast, hold hint
TB_X = 104                                           # messages use the right side: play, BPM and the channel chips stay visible


def fit(p, x, y, txt, fg, bg, maxw):
    """AF_M when it fits, else AF_S (never cut)"""
    f = AF_M if text_w(AF_M, txt) <= maxw else AF_S
    p.text(x, y + (0 if f is AF_M else 2), f, txt, fg, bg)
    return x + text_w(f, txt)


def topbar(p, S):
    t = S.get("toast")
    h = S.get("hold")
    if not (t or h) and S.get("page") == "CHAN":     # a page names itself in the message slot while it is open
        t = ("enc", "CHN", "CH%d SETUP" % (S["sel"] + 1))
    if not (t or h) and S.get("page") == "SET":
        t = ("enc", "GLO", "ALL CHANNELS")
    if not (t or h):
        return
    x0, w = TB_X, W - TB_X
    if h:                                             # a hold, from 0.25 s: name it before it fires; the bar fills to the fire time
        icon, text, frac = h
        p.rect(x0, 0, w, 22, T.TEXT)
        p.art(x0 + 3, 4, ICONS[icon], INK)
        fit(p, x0 + 20, 5, text, INK, T.TEXT, w - 22)
        p.rect(x0, 20, w * frac // 100, 2, INK)
        return
    if t[0] == "enc":                                 # an encoder or OCT being touched: its printed name + what it does now
        _, name, what = t
        p.rect(x0, 0, w, 22, T.TEXT)
        nw = text_w(AF_S, name) + 6
        p.rect(x0 + 2, 3, nw, 16, INK)
        p.text(x0 + 5, 7, AF_S, name, T.TEXT, INK)
        fit(p, x0 + nw + 6, 5, what, INK, T.TEXT, w - nw - 8)
        return
    word, hint = t[1], t[2]
    ci = t[3] if len(t) > 3 else None
    p.rect(x0, 0, w, 22, WHITE)
    x = x0 + 3
    if ci is not None:                                # which channel it happened to: its chip
        p.rect(x, 2, 13, 18, CH[ci])
        p.text_c(x, 13, 6, AF_M, str(ci + 1), INK, CH[ci])
        x += 17
    hw = text_w(AF_S, hint)
    fit(p, x, 5, word, INK, WHITE, W - 4 - hw - 6 - x)
    p.text_r(237, 7, AF_S, hint, INK, WHITE)


MESSAGES = [  # (kind, name-or-word, text-or-hint): every top-bar message; check_messages() proves each one fits
    ("toast", "NOTES", "EDIT=UNDO"), ("toast", "ALL", "EDIT=UNDO"), ("toast", "ACCNT", "EDIT=UNDO"),
    ("toast", "SHAPE", "EDIT=UNDO"), ("toast", "ARTIC", "EDIT=UNDO"), ("toast", "HITS", "EDIT=UNDO"),
    ("toast", "UNDO", "EDIT=REDO"), ("toast", "REDO", "EDIT=UNDO"), ("toast", "MIDI OFF", "2 TO CH2"),
    ("toast", "NEW", "ALGO=PICK"), ("toast", "GONE", "EDIT=UNDO"), ("toast", "PAGE 2?", "PG+=ADD"),
    ("toast", "SAVED P02", ""), ("toast", "LAST ROW", "OCT-=1"), ("toast", "P01", "SAVE=REDO"),
    ("enc", "SELECT", "CH2 EUCLID"), ("enc", "SELECT", "CH4 PATT"), ("enc", "ALGO", "PATT AT STEP"),
    ("enc", "ALGO", "EUCLID AT STEP"), ("enc", "PRESETS", "P16 AT BAR"), ("enc", "PRESETS", "P16 IN 1.5S"),
    ("enc", "OCT+", "ROW 2 OF 2"), ("enc", "OCT+", "ROW 3 TB-3"), ("enc", "OCT-", "ROW 1"),
    ("enc", "SOLO", "CH1 SOLO"), ("enc", "FILL", "60%  LET GO"), ("enc", "ROLL", "3X  LET GO"),
    ("enc", "CHN", "CH2 SETUP"), ("enc", "GLO", "ALL CHANNELS"), ("enc", "K1", "HOST SETS TEMPO"),
    ("enc", "OCT+", "OCTAVE +1"), ("enc", "OCT-", "OCTAVE -2"),
    ("enc", "GLO", "REC = DONE"),
    ("hold", "RESET", ""), ("hold", "HARMONY", ""), ("hold", "UNDO LOAD", ""), ("hold", "HELP / MENU", ""),
    ("hold", "REMOVE CH4", ""), ("hold", "PREVIEW CH4", ""),
]


def check_messages():
    """the message slot is W - TB_X wide; AF_M, else AF_S, must fit with the chip/icon/name box; returns the failures"""
    bad = []
    room = W - TB_X
    for kind, a, b in MESSAGES:
        if kind == "enc":
            need = text_w(AF_S, a) + 6 + 8 + min(text_w(AF_M, b), text_w(AF_S, b))
        elif kind == "hold":
            need = 22 + min(text_w(AF_M, a), text_w(AF_S, a))
        else:
            need = 3 + 17 + min(text_w(AF_M, a), text_w(AF_S, a)) + 6 + text_w(AF_S, b) + 3
        if need > room:
            bad.append((kind, a, b, need, room))
    return bad


def screen(p, S):
    header(p, S)
    knobs(p, S)
    status(p, S)
    music(p, S)
    rec_cursor(p, S)
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
    s = base(used=[0, 1], sel=1, toast=("din", "MIDI OFF", "TAKEN", 1))
    s["chans"][1]["midi"] = None
    return s


def added():
    s = base(used=[0, 1], sel=1, toast=("dice_all", "NEW", "ALGO=PICK", 1), new_chan=True)
    return s


STATE = {"scenes": {
    "playing": base(),
    "two_channels": base(used=[0, 1]),
    "one_channel": base(used=[0], playing=False, pages=1),
    "sel_muted": base(sel=2),
    "dice": base(used=[0, 1, 3], lit="SCL", toast=("dice_notes", "NOTES", "EDIT=UNDO", 0), diced={2, 4, 5, 8, 10, 13}),
    "turning_k2": base(used=[0, 1, 3], turning=(1, 30), vals={1: 6}),
    "hold_edit": base(used=[0, 1, 3], lit="EDIT", hold=("undo", "RESET", 60)),
    "hold_home": base(used=[0, 1, 3], lit="HOME", hold=("home", "HELP / MENU", 100)),
    "rec_armed": base(used=[0, 1], rec=True, playing=False),
    "midi_off": midi_off(),
    "channel_added": added(),
    "midi_off_after": dict(midi_off(), toast=None),
    "added_after": dict(added(), toast=None),
    "pending_p02": base(used=[0, 1, 3], pend=2, playhead=1),
    "turn_select": base(used=[0, 1, 3], sel=1, toast=("enc", "SELECT", "CH2 EUCLID")),
    "turn_algo": with_algo(base(used=[0, 1, 3], toast=("enc", "ALGO", "SNAKE AT STEP")), 0, "ACID"),
    "turn_presets": base(used=[0, 1, 3], pend=2, toast=("enc", "PRESETS", "P02 AT BAR")),
    "oct_row2": base(used=[0, 1, 3], row=1, toast=("enc", "OCT+", "ROW 2 OF 2")),
    "row3_rig": base(used=[0, 1, 3], row=2, rig="TB-3 BITE", toast=("enc", "OCT+", "ROW 3 TB-3")),
    "hold_glo": base(used=[0, 1, 3], lit="GLO", hold=("settings", "HARMONY", 60)),
    "hold_save": base(used=[0, 1, 3], lit="SAVE", hold=("save", "UNDO LOAD", 80)),
    "rec_playing": base(used=[0, 1], rec=True, playing=True, cursor=4, vals={0: 4}),
    # ---- the remaining states (owner: "finish it off")
    "first_run": base(used=[0], playing=False, pages=1, new_chan=True),
    "chan_page": base(used=[0, 1, 3], sel=1, page="CHAN"),
    "chan_midi_clash": dict(base(used=[0, 1, 3], sel=1, page="CHAN", toast=("din", "MIDI OFF", "2 TO CH2", 0)), vals={1: 2}),
    "hold_mute_remove": base(used=[0, 1, 3], held_m=3, hold=("spkx", "REMOVE CH4", 70)),
    "removed": base(used=[0, 1], sel=1, toast=("dice_all", "GONE", "EDIT=UNDO", 3)),
    "ext_sync": base(used=[0, 1, 3], clock="EXT", bpm=124),
    "ext_wait": base(used=[0, 1, 3], clock="WAIT", playing=False),
    "ext_lost": base(used=[0, 1, 3], clock="LOST"),
    "follow_off": base(used=[0, 1, 3], follow_off=True, vpage=1, ppage=2),
    "add_page": base(used=[0, 1, 3], pages=1, toast=("dice_all", "PAGE 2?", "PG+=ADD")),
    "past_length": base(used=[0, 1, 3], pages=1, length=12),
    "held_solo": base(used=[0, 1, 3], held="SOL", toast=("enc", "SOLO", "CH1 SOLO")),
    "held_fill": base(used=[0, 1, 3], held="FIL", toast=("enc", "FILL", "60%  LET GO")),
    "held_roll": base(used=[0, 1, 3], held="ROL", toast=("enc", "ROLL", "3X  LET GO")),
    "undo": base(used=[0, 1, 3], lit="EDIT", toast=("undo", "UNDO", "EDIT=REDO", 0)),
    "arp_row2": with_algo(base(used=[0, 1], row=1), 0, "ARP"),
    "demo_algo": with_algo(base(used=[0, 1, 3]), 0, "DEMO"),
    "stage_heard": with_algo(base(used=[0, 1], vals={1: 48}, algo_state={"heard": 32}), 0, "STAGE"),
    # ---- SET (P4-016, owner-approved 2026-11-13)
    "set_int": base(used=[0, 1, 3], page="SET"),
    "set_int_stopped": base(used=[0, 1, 3], page="SET", playing=False),
    "set_turn_swing": base(used=[0, 1, 3], page="SET", turning=(1, 40), vals={1: 62}),
    "set_octave": base(used=[0, 1, 3], page="SET", octave=1, toast=("enc", "OCT+", "OCTAVE +1")),
    "set_ext_live": base(used=[0, 1, 3], page="SET", clock="EXT", bpm=124),
    "set_ext_k1_turned": base(used=[0, 1, 3], page="SET", clock="EXT", bpm=124, turning=(0, 40), toast=("enc", "K1", "HOST SETS TEMPO")),
    "set_ext_wait": base(used=[0, 1, 3], page="SET", clock="WAIT", playing=False),
    "set_ext_lost": base(used=[0, 1, 3], page="SET", clock="LOST"),
    "sheet_row1_a": {"sheet": True, "row": 0, "algos": ["ACID", "EUCLID", "SNAKE", "STAGE"]},
    "sheet_row1_b": {"sheet": True, "row": 0, "algos": ["ARP", "PATTERNS", "DEMO"]},
    "sheet_row2_a": {"sheet": True, "row": 1, "algos": ["ACID", "EUCLID", "SNAKE", "STAGE"]},
    "sheet_row2_b": {"sheet": True, "row": 1, "algos": ["ARP", "PATTERNS", "DEMO"]},
}}

STATE["scenes"]["chan_midi_clash"]["chans"][0]["midi"] = None   # CH1 lost MIDI 2 to CH2
STATE["scenes"]["chan_midi_clash"]["chans"][1]["midi"] = 2
