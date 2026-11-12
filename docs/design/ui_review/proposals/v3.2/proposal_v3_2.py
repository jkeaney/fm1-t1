# SPDX-License-Identifier: GPL-3.0-only
"""proposal_v3_2.py: UI PROPOSAL v3.2 (glance) for the FM-1 sequencer screen (mock tool, docs/design/ui_review/mock/).

v3.1 answered the first critique; v3.2 answers the independent critique of v3 (critique_v3/ASSESSMENT_A_v3.md + EVIDENCE_B_v3.md), P1 to P3.
It imports the DATA of v3.1 (knob tables, enum pictures, SNAKE routes, palette tokens, fixtures) and re-draws every zone.

THE GRID (this time measured, see report_v3_2.py -> geometry table):  every element is a whole number of modules minus a 1 px inset on each side,
so two neighbours always have a 2 px gap.  Horizontal module 15 px (one lane step = one white key; a channel / knob column = 4 modules = 60 px,
tile x = 60 k + 1, width 58; lane column x = 15 n + 1, width 13).  Vertical module 4 px.  Bars and rules (activity bar, ROW strip text rows) sit
flush on the module.  Text is centred in its cell by cap box (cap top = cell top + (cell h - cap h) // 2, so a 1 px odd remainder goes below).

    Z0 head   y   0 h24   transport shape 13x14, tempo (AF_M), preset (AF_M) + settle bar, page boxes 13x14 with numerals
    Z1 chans  y  24 h48   four 58x38 tiles (y 25), activity bar 58x4 below each tile (y 64, flush) : numeral AF_M, MIDI slot AF_M, M chip, algo name AF_M
    Z2 lane   y  72 h72   16 columns 13x70 (y 73): accent cap 13x8, slide tie, ratchet digit, bar, baseline / edited foot, ghost playhead
    Z3 knobs  y 144 h60   four 58x58 tiles (y 145): label band 18, value AF_L or picture + AF_M name, 6 px meter
    Z4 strip  y 204 h16   ROW 1 / ROW 2 / ROW 3 cells + the OCT hint (58x14, y 205)
    Z5 cue    y 220 h16   ONE AF_M line that follows the hand (printed-name chip + role); a toast slab (y 196, h 40) replaces Z4+Z5 for 1.2 s
    Z6 margin y 236 h 4   bezel overscan (nothing)

Run:  .venv/bin/python docs/design/ui_review/proposals/v3.2/render_all.py     (all PNGs + before/after sheets)
      .venv/bin/python docs/design/ui_review/proposals/v3.2/report_v3_2.py     (the tables of NOTES.md)
"""
import copy
import importlib.util
import math
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, "..", "..", "..", "..", ".."))
sys.path.insert(0, os.path.join(HERE, "..", "..", "mock"))
import mock                                                                  # noqa: E402
sys.modules.setdefault("mock", mock)
from mock import (T, AF_S, AF_M, AF_L, text_w, text_ink, tables, fixture, rgb565, contrast_ratio)   # noqa: E402


def _load(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    mod = importlib.util.module_from_spec(spec)
    sys.modules[name] = mod
    spec.loader.exec_module(mod)
    return mod


P1 = _load("proposal_v3_1", os.path.join(HERE, "..", "v3.1", "proposal_v3_1.py"))      # data tables, enum pictures, SNAKE routes, tokens

PALETTE = "GREY"
W = 240
ALGO_NAMES = P1.ALGO_NAMES                       # ACID EUCLID SNAKE STAGE ARP PATT (AF_M, 58 px tile)
ALGO_FULL = P1.ALGO_FULL
TB = P1.TB

# ---------------------------------------------------------------------------------------------------------------- geometry constants
HEAD_Y, HEAD_H = 0, 24
TILE_Y, TILE_H = 25, 38                          # channel tile (inner of Z1 outer 24..64)
ACT_Y, ACT_H = 64, 4                             # activity bar, flush on the module
LANE_Y, LANE_H = 73, 70                          # lane column (inner of Z2 outer 72..144)
KNOB_Y, KNOB_H = 145, 58                         # knob tile (inner of Z3 outer 144..204)
BAND_H = 18                                      # label band of a knob tile
STRIP_Y, STRIP_H = 205, 14                       # ROW strip cell (inner of outer 204..220)
CUE_Y, CUE_H = 221, 14                           # cue cell (inner of outer 220..236)
SLAB_Y, SLAB_H = 196, 40                         # toast slab (flush: x 0..240, y 196..236)
LANE_COL = 13


def colx(k):
    return 60 * k + 1


def stepx(n):
    return 15 * n + 1


def ctop(y, h, f=AF_M):
    """Cap top of a text of font f centred in a cell y..y+h: the odd px goes below."""
    return y + (h - f.cap_h) // 2


# ---------------------------------------------------------------------------------------------------------------- bookkeeping
LOG = []                 # every text drawn (the text-item tables)
GEOM = []                # every element drawn (the geometry table)
CUR = [""]


class Sv:
    """A strip seen in SCREEN coordinates: draws into the canvas cv, whose top is screen row oy."""

    def __init__(self, cv, oy):
        self.cv, self.oy = cv, oy

    def rect(self, x, y, w, h, c):
        self.cv.rect(x, y - self.oy, w, h, c)

    def frame(self, x, y, w, h, c, t=1):
        for i in range(t):
            self.cv.frame(x + i, y + i - self.oy, w - 2 * i, h - 2 * i, c)

    def pset(self, x, y, c):
        self.cv.pset(x, y - self.oy, c)

    def line(self, x0, y0, x1, y1, c):
        self.cv.line(x0, y0 - self.oy, x1, y1 - self.oy, c)

    def mark(self, name, x, y, w, h):
        self.cv.mark(name, x, y - self.oy, w, h)


def el(name, x, y, w, h, kind="tile"):
    GEOM.append({"scene": CUR[0], "name": name, "x": x, "y": y, "w": w, "h": h, "kind": kind})


def tx(sv, role, f, s, fg, bg, x, ycap, mode="l", w=0):
    """Draws s so that its CAPITALS start at screen row ycap. mode: l pen at x, il ink starts at x, in ink centred across x..x+w, r advance ends at x,
    ir ink ends at x.  Logs the item (role, font, colours) for the report."""
    y = ycap - f.cap_y
    b = text_ink(f, s)
    LOG.append({"scene": CUR[0], "role": role, "s": s, "f": f.name, "fg": fg, "bg": bg, "ink": b[2] - b[0]})
    cv = sv.cv
    y -= sv.oy
    if mode == "in":
        cv.text_in(x, y, w, f, s, fg, bg)
    elif mode == "r":
        cv.text_r(x, y, f, s, fg, bg)
    elif mode == "il":
        cv.text_on(x - b[0], y, f, s, fg, bg)
    elif mode == "ir":
        cv.text_on(x - b[2], y, f, s, fg, bg)
    else:
        cv.text_on(x, y, f, s, fg, bg)


def ink_w(f, s):
    b = text_ink(f, s)
    return b[2] - b[0]


def pal(S):
    return P1.pal(S)


def lane2():
    """Second lane shade (every other beat): LANE mixed 12 % towards TEXT.  A palette-derived value, no table row needed."""
    return mock.ux.mix(mock.ux.lane, mock.ux.text, 12)


def mutec():
    """Colour of every mark of a muted channel's lane (bar outline, cap, tie, digit, foot): MID mixed 30 % towards TEXT, so it stays 3:1 on both lane shades."""
    return mock.ux.mix(mock.ux.mid, mock.ux.text, 30)


def dimink():
    """Dimmed ink on the selected (SEL) fill, e.g. the numeral of a muted selected channel (3.2:1 on PAPER .. 6.9:1 on MONO, against 4.7 .. 12.9 for full ink)."""
    return mock.ux.mix(mock.ux.sel, mock.ux.ink, 70)


# ---------------------------------------------------------------------------------------------------------------- shapes
def tri_right(sv, x, y, w, h, c):
    for r in range(h):
        k = min(r, h - 1 - r)
        sv.rect(x, y + r, max(1, int(round((k + 0.5) * 2 * w / float(h)))), 1, c)


def tri_hollow(sv, x, y, w, h, c):
    tri_right(sv, x, y, w, h, c)
    for r in range(h):
        k = min(r, h - 1 - r)
        ww = int(round((k + 0.5) * 2 * w / float(h)))
        if r >= 4 and r < h - 4 and ww > 6:
            sv.rect(x + 3, y + r, ww - 7, 1, mock.ux.bg)


def tri_up(sv, x, y, w, h, c):
    for r in range(h):
        wd = max(1, int(round((r + 0.5) * w / float(h))))
        wd += (wd + w) & 1
        wd = min(wd, w)
        sv.rect(x + (w - wd) // 2, y + r, wd, 1, c)


def arrow_r(sv, x, y, c):
    """8x7 right arrow (stem 5x1... drawn 2 px thick): the 'sends to' mark in the MIDI slot."""
    sv.rect(x, y + 2, 5, 3, c)
    for k in range(7):
        w = 4 - abs(k - 3)
        sv.rect(x + 4, y + k, w, 1, c)


def ellipse(sv, x, y, w, h, c, ring=0):
    for j in range(h):
        for i in range(w):
            dx, dy = (i + 0.5 - w / 2.0) / (w / 2.0), (j + 0.5 - h / 2.0) / (h / 2.0)
            d = dx * dx + dy * dy
            if d <= 1.0:
                if ring:
                    ix, iy = (i + 0.5 - w / 2.0) / (w / 2.0 - ring), (j + 0.5 - h / 2.0) / (h / 2.0 - ring)
                    if ix * ix + iy * iy <= 1.0:
                        continue
                sv.pset(x + i, y + j, c)


def clamp(v, lo, hi):
    return max(lo, min(hi, v))


# ---------------------------------------------------------------------------------------------------------------- Z0 head
def playhead(S, ch):
    c = S["channels"][ch]
    n = c["len"] or 1
    return 255 if not S["playing"] else (c["p"] + n - 1) % n


def draw_header(sv, S, ch):
    head, acc, red = pal(S)
    c = S["channels"][ch]
    ph, ln = playhead(S, ch), c["len"]
    clk = S.get("clock", "ext" if S.get("ext") else "int")
    # transport: stop square / play triangle / hollow triangle (armed, WAIT) / REC ring (armed, stopped) / REC disc (recording, playing): 13x14
    if S.get("rec"):
        ellipse(sv, 1, 5, 13, 14, red, ring=0 if S["playing"] else 3)
    elif clk == "wait":
        tri_hollow(sv, 1, 5, 13, 14, head)
    elif S["playing"]:
        tri_right(sv, 1, 5, 13, 14, head)
    else:
        sv.rect(1, 5, 13, 14, T.MID)
    el("transport", 1, 5, 13, 14, "shape")
    s = {"int": str(S["bpm"]), "ext": "EXT %d" % S["bpm"], "sync": "EXT --", "wait": "WAIT"}[clk]
    tx(sv, "tempo", AF_M, s, T.TEXT, T.BG, 17, ctop(5, 14), "il")
    el("tempo cell", 16, 5, 58, 14, "text")
    pend, load = S.get("preset_pend"), S.get("preset_load")
    pre = "P%02d%s" % (S["preset"], "?" if pend else ">" if load else "")
    tx(sv, "preset", AF_M, pre, T.TEXT, T.BG, 121, ctop(5, 14), "il")
    el("preset", 121, 5, 58, 14, "text")
    if pend:                                                                 # the settle countdown stays visible in the header even if a knob turn dismisses the toast
        sv.rect(121, 20, 34, 2, T.MID)
        sv.rect(121, 20, int(34 * S["toast"]["frac"]) if S.get("toast") and "frac" in S["toast"] else 17, 2, T.TEXT)
    # page boxes, right aligned, 13x14 with the numeral.  Fill = the PLAYING page (head colour); an under-bar (TEXT, 13x4) = the VIEWED page.
    # Two independent marks, both shapes, so a playhead on a page that is not viewed is a filled box away from the bar.
    pages = max(1, (ln + 15) // 16)
    for k in range(pages):
        x = 226 - 15 * (pages - 1 - k)
        viewed = k == S["vpage"]
        playing = ph != 255 and ph // 16 == k
        if playing:
            sv.rect(x, 5, 13, 14, head)
            fg, bgc = T.BG, head
        else:
            sv.rect(x, 5, 13, 14, T.SURF)
            fg, bgc = T.TEXT if viewed else T.MID, T.SURF
        tx(sv, "page numeral", AF_M, str(k + 1), fg, bgc, x, ctop(5, 14), "in", 13)
        if viewed:
            sv.rect(x, 20, 13, 4, T.TEXT)
        el("page box %d" % (k + 1), x, 5, 13, 14, "shape")
        el("page bar %d" % (k + 1), x, 20, 13, 4, "bar")
    if S.get("add_page") and pages < 4:                                      # ADD PAGE prompt: a dashed ghost box left of the first page box
        gx = 226 - 15 * pages
        for i in range(0, 13, 4):
            sv.rect(gx + i, 5, 2, 1, T.TEXT)
            sv.rect(gx + i, 18, 2, 1, T.TEXT)
        for j in range(0, 14, 4):
            sv.rect(gx, 5 + j, 1, 2, T.TEXT)
            sv.rect(gx + 12, 5 + j, 1, 2, T.TEXT)
        tx(sv, "page numeral", AF_M, "+", T.TEXT, T.BG, gx, ctop(5, 14), "in", 13)
    sv.mark("Z0 head", 0, 0, 240, 24)


# ---------------------------------------------------------------------------------------------------------------- Z1 channels
def draw_chan(sv, S, i):
    head, acc, red = pal(S)
    c = S["channels"][i]
    x, y = colx(i), TILE_Y
    sel, mut = i == S["sel"], c["muted"]
    off = c["midi_ch"] >= 16
    solo_dim = S.get("solo") and not sel
    if sel:
        bg, ink = T.SEL, T.INK
        sv.rect(x, y, 58, TILE_H, bg)
    elif mut or solo_dim:
        bg, ink = T.BG, T.MID
        sv.frame(x, y, 58, TILE_H, T.MID if mut else T.DIM, 2)
        if solo_dim and not mut:                                              # SOL held: the others are 'silenced' (hatched), not muted (no chip)
            for jj in range(2, TILE_H - 2):
                for ii in range(2, 56):
                    if (ii + jj) % 4 == 0:
                        sv.pset(x + ii, y + jj, T.DIM)
    else:
        bg, ink = T.RAISE, T.TEXT
        sv.rect(x, y, 58, TILE_H, bg)
    numc = (dimink() if sel else T.MID) if mut else ink
    tx(sv, "chan numeral", AF_M, str(i + 1), numc, bg, x + 4, ctop(y + 4, 14), "il")
    if off:
        tx(sv, "midi slot", AF_M, "--", ink, bg, x + 14, ctop(y + 4, 14), "il")
    else:
        arrow_r(sv, x + 14, y + 6, ink)                                     # a drawn arrow (not '>' : '1 >2' reads as an inequality)
        tx(sv, "midi slot", AF_M, "%d" % (c["midi_ch"] + 1), ink, bg, x + 23, ctop(y + 4, 14), "il")
    if S.get("first_run") and sel:                                           # first run: an attention frame on the missing fact
        sv.frame(x + 12, y + 3, 27, 16, T.INK, 2)
    if mut:
        chip = T.INK if sel else T.MID
        sv.rect(x + 42, y + 4, 14, 14, chip)
        tx(sv, "mute chip", AF_M, "M", T.SEL if sel else T.INK, chip, x + 42, ctop(y + 4, 14), "in", 14)
    tx(sv, "algo name", AF_M, ALGO_NAMES[c["algo"] % 6], ink, bg, x + 4, y + 22, "il")
    if c["snd0"] and not mut and not off and not solo_dim:                   # activity: ONE colour, ONE place (a 58x4 bar on the BG under the tile)
        sv.rect(x, ACT_Y, 58, ACT_H, head)
    el("chan tile %d" % (i + 1), x, y, 58, TILE_H)
    el("activity bar %d" % (i + 1), x, ACT_Y, 58, ACT_H, "bar")
    sv.mark("chan %d" % (i + 1), x - 1, 24, 60, 44)


# ---------------------------------------------------------------------------------------------------------------- Z2 lane
# rows inside a column (relative to LANE_Y): accent cap 1..8, slide tie 10..14, ratchet digit 16..26, bars 28..63, baseline 64..65, edited foot 64..68
ACC_Y, SLIDE_Y, RAT_Y, BASE_Y = 1, 10, 16, 64
BAR_MAX = 36


def bar_h(vel):
    return 4 + (clamp(vel, 1, 127) * (BAR_MAX - 4)) // 127


def draw_lane_col(sv, S, n, L, p, ln, ph, mut, head, acc, x=None, y=None, playing=None):
    x = stepx(n) if x is None else x
    y0 = LANE_Y if y is None else y
    ghost = S.get("ghost_col")
    if p >= ln:                                                              # past LENGTH: hatched, no fill
        for jj in range(LANE_H):
            for ii in range(LANE_COL):
                if (ii + jj) % 4 == 0:
                    sv.pset(x + ii, y0 + jj, T.DIM)
        return
    if playing is None:
        playing = ph != 255 and p == ph
    colr = head if playing else (mock.ux.lane if (n // 4) % 2 == 0 else lane2())
    sv.rect(x, y0, LANE_COL, LANE_H, colr)
    fg = T.BG if playing else (mutec() if mut else T.TEXT)
    accfg = T.BG if playing else (mutec() if mut else acc)
    edc = T.BG if playing else (mutec() if mut else acc)
    if L.get("ed"):                                                          # edited (hand-entered) step: a 5 px foot in the second colour (baseline is 2 px)
        sv.rect(x, y0 + BASE_Y, LANE_COL, 5, edc)
    else:
        sv.rect(x, y0 + BASE_Y, LANE_COL, 2, T.BG if playing else mutec())
    if not L["on"]:
        return                                                               # a rest is an EMPTY column (baseline only)
    hh = bar_h(L["vel"])
    top = y0 + BASE_Y - hh
    if mut and hh >= 6:                                                      # muted: 2 px outline, hollow
        sv.rect(x + 1, top, 11, hh, T.BG if playing else mutec())
        sv.rect(x + 3, top + 2, 7, hh - 4, colr)
    else:
        sv.rect(x + 1, top, 11, hh, fg)
    if L["acc"]:                                                             # accent: a filled up-triangle cap 13x8
        tri_up(sv, x, y0 + ACC_Y, 13, 8, accfg)
    if L["slide"]:                                                           # slide: a 5 px tie, 15 wide (crosses the gap); in the playing column it keeps TEXT + a dark outline
        wd = 15 if n < 15 else 13
        if playing:
            sv.rect(x, y0 + SLIDE_Y - 1, 13, 1, T.BG)
            sv.rect(x, y0 + SLIDE_Y + 5, 13, 1, T.BG)
            sv.rect(x, y0 + SLIDE_Y, wd, 5, T.TEXT)
        else:
            sv.rect(x, y0 + SLIDE_Y, wd, 5, mutec() if mut else T.TEXT)
    rep = L["rep"]
    if rep and rep > 1:                                                      # ratchet 2..4: the count as an AF_M digit (firmware range 1..4: gr bits 4..6 = 0 AUTO, 1..4)
        tx(sv, "ratchet digit", AF_M, str(min(rep, 4)), fg, colr, x, y0 + RAT_Y, "in", 13)


def draw_lane(sv, S, ch):
    head, acc, red = pal(S)
    c = S["channels"][ch]
    ln, ph, mut = c["len"], playhead(S, ch), c["muted"]
    vp = S["vpage"]
    for n in range(16):
        draw_lane_col(sv, S, n, S["lane"][n], vp * 16 + n, ln, ph, mut, head, acc)
        el("lane col %d" % (n + 1), stepx(n), LANE_Y, LANE_COL, LANE_H, "col")
    if ph != 255 and ph // 16 != vp:                                         # the playhead is on ANOTHER page: a ghost column (2 px head frame, full height)
        gx = stepx(ph % 16)
        sv.frame(gx, LANE_Y, LANE_COL, LANE_H, head, 2)
        sv.rect(gx, LANE_Y, LANE_COL, 5, head)
    sv.mark("Z2 lane", 0, 72, 240, 72)


# ---------------------------------------------------------------------------------------------------------------- Z3 knobs
RIG_TB3 = (("CUT", 74), ("RESO", 40), ("ENV-X", 64), ("PAD-Z", 64))          # TB-3 profile names (5 letters max), LEVEL 0..127


def knob_view(S, ch, k):
    """(label, value text, value font, meter, pic) of knob k (0..3) in the live row."""
    c = S["channels"][ch]
    algo = c["algo"] % 6
    row = S["row"]
    if row == 2:
        nm, v = RIG_TB3[k]
        return nm, str(S.get("rig_vals", [v] * 4)[k]), AF_L, ("bar", S.get("rig_vals", [v] * 4)[k] / 127.0), None
    idx = row * 4 + k
    label, val, f, meter, pic = P1.knob_view(algo, idx, S["knobs"][idx])
    if label == "OCTS":
        label = "SPAN"                                                       # firmware label (v3.1 renamed it OCTS; see NOTES owner question)
    if val == "KEY":
        val, f, meter = "KEYS", AF_M, ("bar", 0.0)                           # the firmware cell reads KEYS (68 px in AF_L does not fit 56)
    return label, val, f, meter, pic


def draw_meter(sv, x, y, meter, fg, track):
    """48x6 meter: a fill bar, or pips (the chosen one tall)."""
    if meter[0] == "bar":
        sv.rect(x, y, 48, 6, track)
        fw = int(round(48 * meter[1]))
        if meter[1] > 0 and fw < 2:
            fw = 2
        sv.rect(x, y, fw, 6, fg)
    else:
        n, i = meter[1], meter[2]
        g = 1 if n > 8 else 2
        pw = (48 - (n - 1) * g) // n
        tot = n * pw + (n - 1) * g
        x0 = x + (48 - tot) // 2
        for k in range(n):
            if k == i:
                sv.rect(x0 + k * (pw + g), y, pw, 6, fg)
            else:
                sv.rect(x0 + k * (pw + g), y + 3, pw, 3, T.MID)


def draw_dots2(sv, x, y, pts, fg, join=True):
    """Dot plot in a 42x16 box at (x, y): 4x4 dots joined by a 2 px line."""
    maxc = max(c_ for c_, l_ in pts) or 1
    maxl = max(l_ for c_, l_ in pts) or 1
    px = min(9.0, 38.0 / maxc)
    py = 12.0 / maxl
    wd = int(round(px * maxc))
    cs = [(x + 2 + (38 - wd) // 2 + int(round(px * c_)) + 2, y + 2 + int(round(py * (maxl - l_)))) for c_, l_ in pts]
    if join:
        for a, b in zip(cs, cs[1:]):
            sv.cv.line_t(a[0], a[1] - 1 - sv.oy, b[0], b[1] - 1 - sv.oy, fg, 2)
    for cx, cy in cs:
        sv.rect(cx - 2, cy - 2, 4, 4, fg)


def draw_curve2(sv, x, y, i, fg):
    pts = P1.curve_pts(i, 43, 15)
    for a, b in zip(pts, pts[1:]):
        sv.cv.line_t(x + a[0], y + a[1] - sv.oy, x + b[0], y + b[1] - sv.oy, fg, 2)


def draw_path2(sv, x, y, n, fg, dim):
    """P1.draw_path routed through the strip offset (31x31 at x, y)."""
    route = P1.snake_route(n)
    P = 9

    def ctr(rc):
        return x + 2 + P * rc[1], y + 2 + P * rc[0]
    for a, b in zip(route, route[1:]):
        (ax, ay), (bx, by) = ctr(a), ctr(b)
        if abs(a[0] - b[0]) + abs(a[1] - b[1]) == 1:
            sv.rect(min(ax, bx) - 1, min(ay, by) - 1, abs(ax - bx) + 3, abs(ay - by) + 3, fg)
        else:
            sv.line(ax, ay, bx, by, dim)
    for rc in route:
        cx, cy = ctr(rc)
        sv.rect(cx - 1, cy - 1, 3, 3, fg)
    sx, sy = ctr(route[0])
    sv.rect(sx - 2, sy - 2, 5, 5, fg)
    ex, ey = ctr(route[-1])
    sv.rect(ex - 2, ey - 2, 5, 5, fg)
    sv.rect(ex - 1, ey - 1, 3, 3, T.SURF)


def draw_knob(sv, S, ch, k):
    x = colx(k)
    row = S["row"]
    label, val, f, meter, pic = knob_view(S, ch, k)
    body = T.RAISE if row == 2 else T.SURF
    sv.rect(x, KNOB_Y, 58, KNOB_H, body)
    fil_k1 = S.get("fil") and row == 0 and k == 0
    if row >= 1 or fil_k1:                                                    # ROW 2 / ROW 3 (and K1 while FIL is held): the label band is INVERTED (the whole zone changes character)
        sv.rect(x, KNOB_Y, 58, BAND_H, T.TEXT)
        lfg, lbg = T.INK, T.TEXT
    else:
        lfg, lbg = T.TEXT, body
    if row == 2:
        sv.frame(x, KNOB_Y, 58, KNOB_H, T.TEXT, 2)
    tx(sv, "knob label", AF_M, label, lfg, lbg, x, ctop(KNOB_Y, BAND_H), "in", 58)
    vy = KNOB_Y + BAND_H + 2                                                  # value cap top 165
    if pic:
        kind, en, v, name = pic
        if kind == "path":
            draw_path2(sv, x + 4, KNOB_Y + BAND_H + 1, v + 1, T.TEXT, T.MID)
            tx(sv, "knob value (M)", AF_M, name, T.TEXT, body, x + 54, KNOB_Y + BAND_H + 10, "ir")
        else:
            if en == "CURVE":
                draw_curve2(sv, x + 7, KNOB_Y + BAND_H + 2, v, T.TEXT)
            else:
                draw_dots2(sv, x + 6, KNOB_Y + BAND_H + 1, P1.PICS[en][v], T.TEXT, join=not (en == "ARPDIR" and v == 5))
            tx(sv, "knob value (M)", AF_M, name, T.TEXT, body, x, KNOB_Y + BAND_H + 21, "in", 58)
    else:
        tx(sv, "knob value" if f is AF_L else "knob value (M)", f, val, T.TEXT, body, x, vy if f is AF_L else vy + 5, "in", 58)
        if S.get("heard") and label == "TOTAL" and S["heard"] < S["knobs"][S["row"] * 4 + k]:
            tx(sv, "knob heard", AF_M, "(%d)" % S["heard"], T.MID, body, x, KNOB_Y + 44, "in", 58)
        else:
            draw_meter(sv, x + 5, KNOB_Y + 45, meter, T.TEXT, T.LANE)
    if fil_k1:                                                                # FIL held: K1 is pushed to its maximum: inverted band + an up-triangle
        tri_up(sv, x + 2, KNOB_Y + 6, 7, 6, lfg)
    el("knob tile %d" % (k + 1), x, KNOB_Y, 58, KNOB_H)
    sv.mark("knob %d" % (k + 1), x - 1, KNOB_Y - 1, 60, 60)


def draw_strip(sv, S, ch):
    """ROW 1 / ROW 2 / ROW 3 cells (58x14; the fourth cell stays empty, the OCT button is named by the cue): the live cell is solid TEXT; ROW 3 exists only on a channel with a device (S['rig'])."""
    rig = S["channels"][ch].get("rig") or S.get("rig")
    row = S["row"]
    if S.get("reset"):                                                       # EDIT held 0.4 s: RESET latched (one shot): a full-width inverted banner
        sv.rect(1, STRIP_Y, 238, STRIP_H, T.TEXT)
        tx(sv, "row label", AF_M, "RESET", T.INK, T.TEXT, 1, ctop(STRIP_Y, STRIP_H), "in", 238)
        el("reset banner", 1, STRIP_Y, 238, STRIP_H)
        sv.mark("Z4 ROW strip", 0, 204, 240, 16)
        return
    for r in range(3):
        if r == 2 and not rig:
            continue
        x = colx(r)
        live = row == r
        sv.rect(x, STRIP_Y, 58, STRIP_H, T.TEXT if live else T.SURF)
        tx(sv, "row label", AF_M, "ROW %d" % (r + 1), T.INK if live else T.MID, T.TEXT if live else T.SURF, x, ctop(STRIP_Y, STRIP_H), "in", 58)
        el("row cell %d" % (r + 1), x, STRIP_Y, 58, STRIP_H)
    sv.mark("Z4 ROW strip", 0, 204, 240, 16)


# ---------------------------------------------------------------------------------------------------------------- Z5 cue (follows the hand)
# (chip, text) per control; chip = the PRINTED name on the panel / the key name, text = what it does + what to do next.  Every string <= 238 - chip px.
CUES = {
    "idle_stopped": ("", "HOLD HOME = MAP"),
    "idle_playing": ("", "FX SCL ENV LFO ARP SEQ = DICE"),
    "dice_all": ("FX", "ALL LAYERS  EDIT = UNDO"),
    "dice_notes": ("SCL", "NOTES  EDIT = UNDO"),
    "dice_accnt": ("ENV", "ACCNT  EDIT = UNDO"),
    "dice_shape": ("LFO", "SHAPE  EDIT = UNDO"),
    "dice_artic": ("ARP", "ARTIC  EDIT = UNDO"),
    "dice_hits": ("SEQ", "HITS  EDIT = UNDO"),
    "undo": ("EDIT", "UNDONE  AGAIN = REDO"),
    "redo": ("EDIT", "REDONE  AGAIN = UNDO"),
    "edit_hold": ("EDIT", "RESET: TOUCH A CONTROL"),
    "mute": ("M2", "CH2 MUTED  TAP AGAIN = ON"),
    "unmute": ("M2", "CH2 ON  TAP AGAIN = MUTE"),
    "mute_hold": ("M2", "HOLD = PREVIEW CH2"),
    "solo": ("SOL", "ONLY CH1 SOUNDS"),
    "fill": ("FIL", "DENSITY UP WHILE HELD"),
    "roll": ("ROL", "STEPS REPEAT 2X-4X"),
    "pg_next": ("PG+", "PAGE 2 OF 4  FOLLOW OFF"),
    "pg_add": ("PG+", "AGAIN = ADD PAGE 2"),
    "restart": ("RST", "ALL CHANNELS TO STEP 1"),
    "chn": ("CHN", "MIDI CHANNEL AND LENGTH"),
    "oct_up": ("OCT+", "OFFSET PEDAL PUNCH DIR"),
    "oct_dn": ("OCT-", "DENS SLIDE JUMP ACCNT"),
    "oct_peek": ("OCT+", "HELD: ROW 2 LIVE"),
    "save_hold": ("SAVE", "HOLD = UNDO PRESET LOAD"),
    "save_tap": ("SAVE", "SAVED TO P01"),
    "home": ("HOME", "TAP: CANCEL  HOLD: MAP"),
    "play": ("PLAY", "STOP: ALL NOTES OFF"),
    "rec_stopped": ("REC", "PLAY KEYS TO WRITE NOTES"),
    "rec_playing": ("REC", "KEYS WRITE NOTES"),
    "ext_sync": ("", "WAITING FOR THE HOST TEMPO"),
    "wait": ("PLAY", "WAITING FOR HOST START"),
    "ext_lost": ("", "CLOCK LOST: OWN TEMPO 120"),
    "follow_off": ("PG-", "VIEW 2 OF 4  FOLLOW OFF"),
    "past_end": ("", "PAST THE END: CHN K1 = LENGTH"),
    "first_run": ("CHN", "K2 = MIDI CHANNEL"),
    "gloset": ("GLO", "SET: BPM SWING GATE DIV"),
}


CUE_ORDER = ("idle_stopped", "idle_playing", "dice_all", "dice_notes", "dice_accnt", "dice_shape", "dice_artic", "dice_hits", "undo", "redo",
             "mute", "unmute", "solo", "fill", "roll", "pg_next", "pg_add", "restart", "oct_up", "oct_dn", "oct_peek", "save_hold", "save_tap", "home",
             "edit_hold", "rec_stopped", "ext_sync", "wait", "ext_lost", "follow_off", "past_end", "first_run")


def cue_oct(S, ch, up=True):
    """OCT+ / OCT- : the four knob names of the row the press moves to (they are also the new tile labels; this is the off-row preview)."""
    algo = S["channels"][ch]["algo"] % 6
    row = S["row"]
    r = min(row + 1, 2 if S["channels"][ch].get("rig") else 1) if up else 0
    if r == 2:
        return ("OCT+", " ".join(n for n, _v in RIG_TB3))
    return ("OCT+" if up else "OCT-", " ".join(P1.KNOBS[algo][r * 4 + k][0].replace("OCTS", "SPAN") for k in range(4)))


def draw_cue(sv, S, CUE_Y=CUE_Y):
    key = S.get("cue") or ("idle_playing" if S["playing"] and not S.get("rec") else "idle_stopped")
    chip, text = CUES[key] if isinstance(key, str) else key
    if key == "oct_up":
        chip, text = cue_oct(S, S["sel"], True)
    elif key == "oct_dn":
        chip, text = cue_oct(S, S["sel"], False)
    x = 0
    if chip:                                                                 # the printed name as a flush-left tab (2 px side padding), then the role text
        cw = ink_w(AF_M, chip) + 4
        sv.rect(x, CUE_Y, cw, CUE_H, T.TEXT)
        tx(sv, "cue chip", AF_M, chip, T.INK, T.TEXT, x, ctop(CUE_Y, CUE_H), "in", cw)
        el("cue chip", x, CUE_Y, cw, CUE_H, "text")
        assert x + cw + 4 + ink_w(AF_M, text) <= 240, ("cue too long", chip, text)
        tx(sv, "cue", AF_M, text, T.TEXT, T.BG, x + cw + 4, ctop(CUE_Y, CUE_H), "il")
    else:
        tx(sv, "cue", AF_M, text, T.TEXT, T.BG, 0, ctop(CUE_Y, CUE_H), "in", 240)
    el("cue line", 0, CUE_Y, 240, CUE_H, "cue")
    sv.mark("Z5 cue", 0, CUE_Y - 1, 240, 16)


# ---------------------------------------------------------------------------------------------------------------- toast slab
LAYERS = ("ALL", "NOTES", "ACCNT", "SHAPE", "ARTIC", "HITS")
PRINTED = {"ALL": "FX", "NOTES": "SCL", "ACCNT": "ENV", "SHAPE": "LFO", "ARTIC": "ARP", "HITS": "SEQ"}
# per algorithm (ACID EUCLID SNAKE STAGE ARP PATT) and layer: (algorithm's own sub-label, what changed).  None = the algorithm has no such layer (ALGORITHM_SET 4.4).
# Sources: docs/design/ALGORITHM_SET.md 4.2 / 4.4 (matrix, sub-labels) and section 13 / PATTERNS for ARP / PATT.
SUBS = (
    {"NOTES": ("NOTES", "NEW PITCHES"), "HITS": ("HITS", "NEW HIT PATTERN"), "ACCNT": ("ACCNT", "NEW ACCENTS"), "ARTIC": ("GT+SLD", "NEW GATES + SLIDES"), "SHAPE": ("OCTAVE", "NEW OCTAVE JUMPS")},
    {"NOTES": ("NOTES", "NEW PITCHES"), "HITS": ("ROTATE", "HITS ROTATED"), "ACCNT": ("ACCNT", "NEW ACCENTS"), "ARTIC": ("RATCH", "NEW RATCHETS"), "SHAPE": ("MOD", "NEW MOD LANES")},
    {"NOTES": ("NOTES", "NEW PITCHES"), "HITS": ("GATES", "NEW GATES"), "ACCNT": ("VELO", "NEW VELOCITIES"), "ARTIC": None, "SHAPE": ("DRIFT", "NEW DRIFT")},
    {"NOTES": ("NOTES", "NEW PITCHES"), "HITS": ("RESTS", "NEW RESTS"), "ACCNT": ("VELO", "NEW VELOCITIES"), "ARTIC": ("RT+HLD", "NEW RATCHETS + HOLDS"), "SHAPE": ("LONG", "NEW LONG STAGES")},
    {"NOTES": ("VARY", "NEW OCTAVE HOPS"), "HITS": ("RESTS", "NEW RESTS"), "ACCNT": ("VELO", "NEW VELOCITIES"), "ARTIC": ("LENGTH", "NEW NOTE LENGTHS"), "SHAPE": ("PHASE", "NEW START PHASE")},
    {"NOTES": ("VARY", "NEW VARIATIONS"), "HITS": ("ORDER", "NEW THINNING ORDER"), "ACCNT": ("ACCNT", "NEW ACCENTS"), "ARTIC": ("SLIDES", "NEW SLIDES"), "SHAPE": ("UP/DN", "NEW UP/DOWN")},
)


def dice_text(algo, layer, ch):
    """(title, detail, changed) of the dice toast: the title LEADS with the printed button name."""
    title = "%s: %s" % (PRINTED[layer], layer)
    if layer == "ALL":
        miss = [l_ for l_ in LAYERS[1:] if SUBS[algo][l_] is None]
        return title, ("CH%d ALL BUT %s NEW" % (ch + 1, miss[0])) if miss else "CH%d EVERY LAYER NEW" % (ch + 1), True
    sub = SUBS[algo][layer]
    if sub is None:
        return title, "NO %s IN %s" % (layer, ALGO_NAMES[algo]), False
    return title, "CH%d %s" % (ch + 1, sub[1]), True


def lead(sv, chip, text, x, ct, fg=None, dimmed=False):
    """Toast title that LEADS with the printed button name: an inverted AF_L tab (the panel print) then the role in AF_L.  Returns the end x."""
    fg = T.TEXT if fg is None else fg
    cw = ink_w(AF_L, chip) + 8
    sv.rect(x, ct - 1, cw, 23, T.MID if dimmed else T.TEXT)
    tx(sv, "toast chip", AF_L, chip, T.INK, T.MID if dimmed else T.TEXT, x, ct, "in", cw)
    tx(sv, "toast title", AF_L, text, fg, T.SURF, x + cw + 8, ct, "il")
    return x + cw + 8 + ink_w(AF_L, text)


def draw_slab(sv, S):
    """The toast: a 240x40 slab at y 196 (SURF, a 2 px TEXT rule on top: it must not outshine the playhead).  Row 1 = title (AF_L, 21 cap), row 2 = detail (AF_M)."""
    t = S["toast"]
    k = t["kind"]
    x0, y0 = 0, SLAB_Y
    sv.rect(x0, y0, 240, SLAB_H, T.SURF)
    sv.rect(x0, y0, 240, 2, T.TEXT)
    ct = y0 + 4                                                              # AF_L cap top (rows 200..220)
    dt = y0 + 28                                                             # AF_M cap top (rows 224..234)
    lx = 3
    right = 237
    ch = S["sel"]
    dim = T.MID
    if k == "dice":
        title, detail, changed = dice_text(t["algo"], t["layer"], ch)
        lead(sv, PRINTED[t["layer"]], t["layer"], lx, ct, T.TEXT if changed else dim, not changed)
        tx(sv, "toast detail", AF_M, detail, T.TEXT if changed else dim, T.SURF, lx, dt, "il")
        hint = "EDIT: UNDO" if changed else "NOTHING CHANGED"
        if lx + ink_w(AF_M, detail) + 12 + ink_w(AF_M, hint) <= right:
            tx(sv, "toast hint", AF_M, hint, T.MID, T.SURF, right, dt, "ir")
    elif k in ("undo", "redo"):
        lead(sv, "EDIT", "UNDONE" if k == "undo" else "REDONE", lx, ct)
        tx(sv, "toast detail", AF_M, t["what"], T.TEXT, T.SURF, lx, dt, "il")
        tx(sv, "toast hint", AF_M, "EDIT AGAIN = " + ("REDO" if k == "undo" else "UNDO"), T.MID, T.SURF, right, dt, "ir")
    elif k == "algo":
        a = t["algo"]
        tx(sv, "toast title", AF_L, ALGO_FULL[a], T.TEXT, T.SURF, lx, ct, "il")
        tx(sv, "toast detail", AF_M, "ALGORITHM  CH%d" % (ch + 1), T.TEXT, T.SURF, lx, dt, "il")
        for j in range(6):                                                    # position pips (the list), the chosen one tall
            px = right - 6 * 14 + 4 + j * 14
            if j == a:
                sv.rect(px, dt, 10, 11, T.TEXT)
            else:
                sv.rect(px, dt + 6, 10, 5, T.MID)
    elif k == "preset":
        s = "P%02d" % t["slot"]
        tx(sv, "toast title", AF_L, s, T.TEXT, T.SURF, lx, ct, "il")
        tx(sv, "toast detail", AF_M, t["when"], T.TEXT, T.SURF, 64, ct + 5, "il")
        bx = 64 + ink_w(AF_M, t["when"]) + 8
        sv.frame(bx, ct + 5, right - bx, 11, T.TEXT)                           # the countdown bar, labelled by the text on its left
        sv.rect(bx + 2, ct + 7, int((right - bx - 4) * t["frac"]), 7, T.TEXT)
        tx(sv, "toast detail", AF_M, "SAVE: KEEP HERE  HOME: CANCEL", T.TEXT, T.SURF, lx, dt, "il")
    elif k == "loading":
        s = "P%02d" % t["slot"]
        tx(sv, "toast title", AF_L, s, T.TEXT, T.SURF, lx, ct, "il")
        tri_right(sv, lx + ink_w(AF_L, s) + 8, ct + 4, 13, 14, T.TEXT)
        tx(sv, "toast detail", AF_M, "LOADING", T.TEXT, T.SURF, 80, ct + 5, "il")
        lead_chip = "SAVE"
        cw = ink_w(AF_M, lead_chip) + 4
        sv.rect(lx, dt - 2, cw, 14, T.TEXT)
        tx(sv, "toast chip", AF_M, lead_chip, T.INK, T.TEXT, lx, dt, "in", cw)
        tx(sv, "toast detail", AF_M, "HOLD = UNDO LAST LOAD", T.TEXT, T.SURF, lx + cw + 4, dt, "il")
    elif k == "midi":
        m = t["midi"]
        title = "MIDI OFF" if m >= 16 else "MIDI %d" % (m + 1)
        tx(sv, "toast title", AF_L, title, T.TEXT, T.SURF, lx, ct, "il")
        tx(sv, "toast detail", AF_M, t["what"], T.TEXT, T.SURF, lx, dt, "il")
    elif k == "text":
        tx(sv, "toast title", AF_L, t["title"], T.TEXT, T.SURF, lx, ct, "il")
        tx(sv, "toast detail", AF_M, t["what"], T.TEXT, T.SURF, lx, dt, "il")
        if t.get("hint"):
            tx(sv, "toast hint", AF_M, t["hint"], T.MID, T.SURF, right, dt, "ir")
    el("toast slab", 0, SLAB_Y, 240, SLAB_H, "toast")
    sv.mark("toast slab", 0, SLAB_Y, 240, SLAB_H)


# ---------------------------------------------------------------------------------------------------------------- the glance screen
def draw_glance(cv, S):
    ch = S["sel"]
    a = Sv(cv, 0)
    cv.begin(W, 72, T.BG)
    draw_header(a, S, ch)
    for i in range(4):
        draw_chan(a, S, i)
    cv.blit(0, 0)
    cv.begin(W, 72, T.BG)
    b = Sv(cv, 72)
    draw_lane(b, S, ch)
    cv.blit(0, 72)
    cv.begin(W, 96, T.BG)
    c = Sv(cv, 144)
    for k in range(4):
        draw_knob(c, S, ch, k)
    t = S.get("toast")
    if t:
        draw_slab(c, S)
    else:
        draw_strip(c, S, ch)
        draw_cue(c, S)
    c.mark("Z6 margin", 0, 236, 240, 4)
    cv.blit(0, 144)


# ---------------------------------------------------------------------------------------------------------------- the MAP (HOME held)
# Split in two screens, switched by OCT- / OCT+ while HOME is held (touching OCT then shows the help of OCT, as for any control; it does not latch a row).
#   A = encoders, OCT, K1..K4, the 12 buttons (printed name small AF_S, role AF_M);  B = the 27 keys + the lane key + the channel key (all AF_M).
MAP_BTN = (("FX", "ALL", ""), ("SCL", "NOTES", ""), ("ENV", "ACCNT", ""), ("LFO", "SHAPE", ""),
           ("EDIT", "UNDO", "RESET"), ("GLO", "SET", "HARM"), ("HOME", "CANCEL", "MENU"), ("SAVE", "SAVE", "UNDO L"),
           ("ARP", "ARTIC", ""), ("SEQ", "HITS", ""), ("PLAY", "PLAY", ""), ("REC", "REC", ""))
MAP_BTN_CTL = ("B_FX", "B_SCL", "B_ENV", "B_LFO", "B_EDIT", "B_GLO", "B_HOME", "B_SAVE", "B_ARP", "B_SEQ", "B_PLAY", "B_REC")
MAP_ENC = (("MASTER", "VOL", "MASTER"), ("SELECT", "CH 1-4", "ENC_SEL"), ("PRESETS", "P01-16", "ENC_PRE"), ("ALGO", "OF CH %d", "EN_ALGO"))
MAP_BLACK = (("M1", ""), ("M2", ""), ("M3", ""), ("M4", ""), ("CHN", ""), ("", "SOL"), ("", "FIL"), ("", "ROL"), ("PG-", ""), ("PG+", ""), ("RST", ""))
MAP_BLACK_C = (15, 30, 45, 75, 90, 120, 135, 150, 180, 195, 225)             # black key centres on the 15 px white-key pitch: 3-2-3-2-1 (gaps 21 px between groups)
MENU_HOLD_MS, MENU_COUNT_MS = 1500, 1000


def boxed(sv, role, x, y, w, label, fg, bg, f=AF_M, h=15):
    """HOLD role = the word inside an outline box (the ONE hold encoding, buttons and keys)."""
    sv.frame(x, y, w, h, T.MID)
    tx(sv, role, f, label, fg, bg, x, y + (h - f.cap_h) // 2, "in", w)


def map_header(sv, S, page):
    cd = S.get("countdown")
    touch = S.get("touch")
    sv.rect(0, 15, 240, 1, T.LINE)
    if cd is not None:                                                       # hold HOME on: the MENU word and a filling bar
        tx(sv, "map menu word", AF_M, "MENU", T.TEXT, T.BG, 1, 3, "il")
        sv.frame(52, 3, 187, 11, T.TEXT)
        sv.rect(54, 5, int(183 * cd), 7, T.TEXT)
    elif touch:
        txt = touch[1]
        f = AF_M if ink_w(AF_M, txt) <= 236 else AF_S
        tx(sv, "map help line", f, txt, T.TEXT, T.BG, 0, 3, "in", 240)
    else:
        tx(sv, "map title", AF_M, "TOUCH = HELP", T.TEXT, T.BG, 1, 3, "il")
        hint = "OCT+ = KEYS >" if page == "A" else "< OCT- = BUTTONS"
        tx(sv, "map page hint", AF_M, hint, T.MID, T.BG, 239, 3, "ir")


def map_tile(sv, x, y, w, h, printed, role, hold, hot, role_fg=None):
    bg = T.SEL if hot else T.SURF
    sv.rect(x, y, w, h, bg)
    tx(sv, "map printed", AF_S, printed, T.INK if hot else T.MID, bg, x, y + 3, "in", w)
    tx(sv, "map role", AF_M, role, T.INK if hot else (role_fg or T.TEXT), bg, x, y + 14, "in", w)
    if hold:
        boxed(sv, "map hold", x, y + 27, w, hold, T.INK if hot else T.TEXT, bg, AF_M, 15)
    el("map tile " + printed, x, y, w, h)


def draw_map_A(cv, S):
    ch = S["sel"]
    algo = S["channels"][ch]["algo"] % 6
    touch = S.get("touch")
    row = S["row"]
    cv.begin(W, 124, T.BG)
    a = Sv(cv, 0)
    map_header(a, S, "A")
    for k, (nm, role, ctl) in enumerate(MAP_ENC):                           # encoders: the panel's block as one row (printed name AF_S, role AF_M)
        map_tile(a, colx(k), 17, 58, 30, nm, role % (ch + 1) if "%d" in role else role, "", touch and touch[0] == ctl)
    for k, (nm, role) in enumerate((("OCT-", "ROW 1"), ("OCT+", "ROW+"))):    # the live row is the inverted one
        live = row == k or (k == 1 and row == 2)
        bg = T.SEL if live else T.SURF
        a.rect(colx(k), 49, 58, 30, bg)
        tx(a, "map printed", AF_S, nm, T.INK if live else T.MID, bg, colx(k), 52, "in", 58)
        tx(a, "map role", AF_M, role, T.INK if live else T.TEXT, bg, colx(k), 63, "in", 58)
        el("map tile " + nm, colx(k), 49, 58, 30)
    a.frame(colx(2) + 4, 56, 20, 15, T.MID)                                  # the hold legend: an empty box = a HOLD role
    tx(a, "map legend", AF_M, "= HOLD", T.TEXT, T.BG, colx(2) + 30, 58, "il")
    tx(a, "map legend", AF_M, "HOME AGAIN = MENU", T.MID, T.BG, colx(2) + 4, 69, "il") if False else None
    for k in range(4):                                                       # K1..K4: both row names; the live row bright
        x = colx(k)
        a.rect(x, 81, 58, 42, T.SURF)
        tx(a, "map printed", AF_S, "K%d" % (k + 1), T.MID, T.SURF, x, 84, "in", 58)
        n1, n2 = P1.KNOBS[algo][k][0], P1.KNOBS[algo][4 + k][0]
        n1, n2 = n1.replace("OCTS", "SPAN"), n2.replace("OCTS", "SPAN")
        tx(a, "map knob row1", AF_M, n1, T.TEXT if row == 0 else T.MID, T.SURF, x, 95, "in", 58)
        tx(a, "map knob row2", AF_M, n2, T.TEXT if row >= 1 else T.MID, T.SURF, x, 108, "in", 58)
        el("map tile K%d" % (k + 1), x, 81, 58, 42)
    cv.blit(0, 0)
    cv.begin(W, 116, T.BG)
    b = Sv(cv, 124)
    ys = ((125, 30), (157, 46), (205, 30))
    for r in range(3):
        y, h = ys[r]
        for c in range(4):
            i = 4 * r + c
            pr, role, hold = MAP_BTN[i]
            map_tile(b, colx(c), y, 58, h, pr, role, hold, touch and touch[0] == MAP_BTN_CTL[i])
    cv.blit(0, 124)


def draw_map_B(cv, S):
    head, acc, red = pal(S)
    cv.begin(W, 124, T.BG)
    a = Sv(cv, 0)
    map_header(a, S, "B")
    last_end = {0: -99, 1: -99}
    for i, (tap, hold) in enumerate(MAP_BLACK):                             # black keys: 9x14 at the keybed centres (3-2-3-2-1), labels on two tiers with leader lines
        cx = MAP_BLACK_C[i]
        a.rect(cx - 4, 17, 8, 14, T.KEY)
        tier = i % 2
        ty = 38 if tier == 0 else 60
        a.rect(cx, 31, 1, ty - 31, T.MID)
        lab = tap or hold
        w = ink_w(AF_M, lab) + (6 if hold else 0)
        lx = clamp(max(cx - w // 2, last_end[tier] + 3), 0, 240 - w)
        assert lx <= cx - 1 and lx + w >= cx + 1, ("leader line misses its label", lab)
        last_end[tier] = lx + w
        if hold:
            boxed(a, "map black key hold", lx, ty, w, lab, T.TEXT, T.BG, AF_M, 15)
        else:
            tx(a, "map black key", AF_M, lab, T.TEXT, T.BG, lx, ty + 2, "il")
        el("map black key %s" % lab, cx - 4, 17, 8, 14, "keybed")
    for n in range(16):                                                      # 16 white keys (13 px, pitch 15), beat shading; the beat starts are numbered below
        x = stepx(n)
        a.rect(x, 81, 13, 26, mock.ux.lane if (n // 4) % 2 == 0 else lane2())
        if n % 4 == 0:
            tx(a, "map white key", AF_M, str(n + 1), T.TEXT, T.BG, x, 110, "il")
        el("map white key %d" % (n + 1), x, 81, 13, 26, "col")
    tx(a, "map caption", AF_M, "WHITE KEYS = STEPS 1-16", T.MID, T.BG, 0, 108, "r") if False else None
    cv.blit(0, 0)
    cv.begin(W, 116, T.BG)
    b = Sv(cv, 124)
    tx(b, "map caption", AF_M, "TAP = STEP ON/OFF", T.TEXT, T.BG, 1, 127, "il")
    boxed(b, "map hold", 1, 141, 48, "HOLD", T.TEXT, T.BG, AF_M, 15)
    tx(b, "map caption", AF_M, "= EDIT THAT NOTE", T.TEXT, T.BG, 54, 143, "il")
    # lane key: the real marks at real size + one word each (AF_M), two per row
    y1, y2 = 162, 180
    tri_up(b, 1, y1 + 3, 13, 8, acc)
    tx(b, "map key word", AF_M, "ACCENT", T.TEXT, T.BG, 19, y1 + 2, "il")
    b.rect(121, y1 + 4, 15, 5, T.TEXT)
    tx(b, "map key word", AF_M, "SLIDE", T.TEXT, T.BG, 141, y1 + 2, "il")
    tx(b, "map key word", AF_M, "3", T.TEXT, T.BG, 1, y2 + 2, "il")
    tx(b, "map key word", AF_M, "RATCHET 2-4", T.TEXT, T.BG, 19, y2 + 2, "il")
    b.rect(121, y2 + 6, 13, 5, acc)
    tx(b, "map key word", AF_M, "EDITED", T.TEXT, T.BG, 141, y2 + 2, "il")
    # channel key
    y3, y4 = 202, 220
    b.rect(1, y3, 14, 14, T.MID)
    tx(b, "map key word", AF_M, "M", T.INK, T.MID, 1, y3 + 1, "in", 14)
    tx(b, "map key word", AF_M, "MUTED", T.TEXT, T.BG, 19, y3 + 2, "il")
    arrow_r(b, 121, y3 + 3, T.TEXT)
    tx(b, "map key word", AF_M, "2", T.TEXT, T.BG, 131, y3 + 2, "il")
    tx(b, "map key word", AF_M, "MIDI OUT", T.TEXT, T.BG, 145, y3 + 2, "il")
    tx(b, "map key word", AF_M, "--", T.TEXT, T.BG, 1, y4 + 2, "il")
    tx(b, "map key word", AF_M, "MIDI OFF", T.TEXT, T.BG, 19, y4 + 2, "il")
    b.rect(121, y4 + 4, 14, 4, head)
    tx(b, "map key word", AF_M, "SOUNDED", T.TEXT, T.BG, 141, y4 + 2, "il")
    cv.blit(0, 124)


def draw_map(cv, S):
    if S["map"] == "A":
        draw_map_A(cv, S)
    else:
        draw_map_B(cv, S)


# ---------------------------------------------------------------------------------------------------------------- draw
def draw(cv, S):
    CUR[0] = S.get("_name", "")
    del GEOM[:]
    mock.palette_set(S.get("palette", "GREY"))
    if S.get("map"):
        draw_map(cv, S)
        return
    if S.get("cue_table"):                                                   # review sheet: every cue string as the 16 px band it is drawn in
        for j in range(2):                                                   # two strips of 6 bands (16 px each), under the firmware's 240x124
            cv.begin(W, 96, T.BG)
            sv = Sv(cv, 96 * j)
            for i, key in enumerate(S["cue_table"][6 * j:6 * j + 6]):
                draw_cue(sv, dict(S, cue=key), CUE_Y=96 * j + 16 * i + 1)
            cv.blit(0, 96 * j)
        return
    draw_glance(cv, S)
    LASTGEOM[:] = list(GEOM)


LASTGEOM = []

# ---------------------------------------------------------------------------------------------------------------- fixtures
def base(**kw):
    s = copy.deepcopy(fixture("perf_playing"))
    s.update(kw)
    s.setdefault("heard", 0)
    return s


# (on, vel, ed, acc, slide, rep): a demo ACID line that shows every lane mark once
DEMO = [(1, 100, 0, 1, 0, 1), (1, 60, 0, 0, 0, 1), (1, 80, 1, 0, 0, 1), (1, 70, 0, 0, 1, 1), (1, 95, 0, 1, 0, 1), (0, 1, 0, 0, 0, 1),
        (1, 50, 1, 0, 0, 1), (1, 85, 0, 0, 1, 1), (1, 110, 0, 1, 0, 1), (0, 1, 0, 0, 0, 1), (1, 75, 0, 0, 0, 3), (1, 40, 0, 0, 0, 1),
        (1, 100, 0, 1, 1, 1), (1, 65, 0, 0, 0, 2), (1, 3, 0, 0, 0, 1), (1, 90, 0, 0, 0, 1)]


def lane_of(rows):
    return [dict(on=a, vel=b, ed=ed, acc=c, slide=d, rep=e) for (a, b, ed, c, d, e) in rows]


ACID_K = P1.ACID_K
EUC_K = P1.EUC_K


def chans(spec):
    out = []
    for (algo, midi, muted, ln, p, snd) in spec:
        out.append(dict(algo=algo, midi_ch=midi, muted=muted, len=ln, pg_used=(ln + 15) // 16, snd0=snd, p=p))
    return out


CH_DEFAULT = chans([(0, 1, 0, 16, 6, 1), (1, 2, 0, 16, 10, 1), (2, 3, 0, 16, 3, 0), (3, 9, 0, 16, 12, 0)])


def scene_idle():
    return base(playing=0, bpm=120, preset=1, knobs=list(ACID_K), channels=chans([(0, 1, 0, 16, 0, 0), (1, 2, 0, 16, 0, 0), (2, 3, 0, 16, 0, 0), (3, 9, 0, 16, 0, 0)]),
                lane=lane_of(DEMO), hot=[0] * 4, rec=0, cue="idle_stopped")


def scene_playing(**kw):
    s = base(playing=1, bpm=120, preset=1, knobs=list(ACID_K), channels=copy.deepcopy(CH_DEFAULT), lane=lane_of(DEMO), hot=[0] * 4, rec=0, cue="idle_playing")
    s.update(kw)
    return s


def scene_row2():
    s = scene_playing(row=1, cue="oct_up")
    return s


def scene_row3():
    s = scene_playing(row=2, cue="idle_playing")
    s["channels"][0]["rig"] = 1
    return s


def scene_mutes():
    s = scene_playing(sel=2, knobs=list(P1.SNAKE_K))
    ch = s["channels"]
    ch[0].update(algo=0, midi_ch=1, muted=0, snd0=1)
    ch[1].update(algo=1, midi_ch=2, muted=1, snd0=0)
    ch[2].update(algo=2, midi_ch=3, muted=0, snd0=0, len=20)
    ch[3].update(algo=3, midi_ch=16, muted=1, snd0=0)
    s["cue"] = "mute"
    return s


def scene_stress(**kw):
    """ARP selected AND muted AND MIDI OFF (channel 4), EXT 126, P16, L64 page 4/4, REC, enumerated knobs SHAPE (OCTV) and RATE (2X)."""
    s = base(sel=3, vpage=3, playing=1, rec=1, ext=1, clock="ext", bpm=126, preset=16, knobs=[8, 3, 3, 3, 15, 8, 10, 4], row=0, hot=[0] * 4, cue="rec_playing")
    s["channels"] = chans([(0, 15, 0, 16, 3, 1), (5, 1, 0, 32, 2, 0), (2, 16, 0, 16, 2, 0), (4, 16, 1, 64, 56, 0)])
    s["lane"] = lane_of([(a, b, ed, c, d, e) for (a, b, c, d, e, ed) in P1.STRESS_STEPS])
    s.update(kw)
    return s


def scene_stress2():
    s = scene_stress(row=1, knobs=[8, 3, 3, 3, 15, 8, 10, 5], cue="oct_up")
    return s


def toast_scene(toast, cue=None, **kw):
    s = scene_playing(**kw)
    s["toast"] = toast
    if cue:
        s["cue"] = cue
    return s


def scene_dice(layer, algo=0):
    s = toast_scene(dict(kind="dice", layer=layer, algo=algo))
    s["channels"][0]["algo"] = algo
    return s


def scene_cue(key, **kw):
    s = scene_playing(cue=key, **kw)
    return s


def scene_lane_full(ph_col=7, **kw):
    """16 steps ON, accent + slide + ratchet 4 on every step (the critique's worst lane), playhead on column ph_col."""
    s = scene_playing(lane=lane_of([(1, 120, 0, 1, 1, 4)] * 16), cue="idle_playing")
    s["channels"][0]["p"] = ph_col + 1
    s["channels"][0]["len"] = 16
    s.update(kw)
    return s


def scene_lane_vel():
    """vel 1, 2, 3 beside rests and a normal step, playhead on a vel-1 step (v3: pixel-identical to a rest)."""
    rows = [(1, 1, 0, 0, 0, 1), (0, 1, 0, 0, 0, 1), (1, 2, 0, 0, 0, 1), (0, 1, 0, 0, 0, 1), (1, 3, 0, 0, 0, 1), (0, 1, 0, 0, 0, 1), (1, 40, 0, 0, 0, 1), (0, 1, 0, 0, 0, 1)] * 2
    s = scene_playing(lane=lane_of(rows))
    s["channels"][0]["p"] = 6          # playhead on column 5 (a rest: pairs with a vel 1 step left of it)
    s["channels"][0]["p"] = 5          # playhead on column 4 (vel 3)
    return s


def scene_lane_sweep():
    rows = [(1, 100, 0, 0, 0, r) for r in range(1, 9)] + [(1, 100, 0, 1, 0, r) for r in range(1, 9)]
    s = scene_playing(lane=lane_of(rows))
    s["channels"][0]["p"] = 4          # playhead on column 3 (ratchet 3)
    return s


def scene_lane_muted():
    s = scene_playing(lane=lane_of(DEMO), sel=0, cue="mute")
    s["channels"][0]["muted"] = 1
    s["channels"][0]["p"] = 9
    return s


def scene_follow_off():
    s = scene_playing(vpage=1, cue="follow_off")
    s["channels"][0].update(len=64, pg_used=15, p=38)       # playhead step 38 -> page 3 (index 2), viewed page 2
    s["lane"] = lane_of(DEMO[8:] + DEMO[:8])
    return s


def scene_state(name):
    s = scene_idle()
    if name == "ext_sync":
        s.update(clock="sync", bpm=0, cue="ext_sync")
    elif name == "wait":
        s.update(clock="wait", bpm=126, cue="wait")
    elif name == "ext_lost":
        s = scene_playing(clock="int", bpm=120, cue="ext_lost")
    elif name == "rec_stopped":
        s.update(rec=1, cue="rec_stopped")
    elif name == "rec_playing":
        s = scene_playing(rec=1, cue="rec_playing")
    elif name == "add_page":
        s = scene_playing(add_page=1, vpage=1, toast=dict(kind="text", title="ADD PAGE 3", what="PG+ AGAIN: COPY OF 2  HOME: NO"))
        s["channels"][0].update(len=32, pg_used=3, p=20)
        s["lane"] = lane_of(DEMO[::-1])
    elif name == "past_length":
        s = scene_playing(vpage=1, cue="past_end", toast=dict(kind="text", title="PAST THE END", what="CHN K1 = LENGTH", hint="20 STEPS"))
        s["channels"][0].update(len=20, pg_used=3, p=18)
    elif name == "first_run":
        s.update(first_run=1, cue="first_run")
        s["channels"] = chans([(0, 0, 0, 16, 0, 0), (1, 1, 0, 16, 0, 0), (2, 2, 0, 16, 0, 0), (3, 3, 0, 16, 0, 0)])
    elif name == "reset":
        s.update(reset=1, cue="edit_hold", playing=1)
    elif name == "sol":
        s = scene_playing(solo=1, cue="solo")
    elif name == "fil":
        s = scene_playing(fil=1, cue="fill")
        s["knobs"][0] = 16
    elif name == "rol":
        rows = [(a, b, ed, c, d, 2 if a else 1) for (a, b, ed, c, d, _e) in DEMO]
        s = scene_playing(lane=lane_of(rows), cue="roll")
    return s


MIDI_TOAST = dict(kind="midi", midi=1, what="CH1 SENDS ON MIDI CHANNEL 2")


def scenes():
    d = {
        "glance_idle": scene_idle(), "glance_playing": scene_playing(), "row2": scene_row2(), "row3": scene_row3(), "mutes": scene_mutes(),
        "stress": scene_stress(), "stress_row2": scene_stress2(),
        "stress_pal_paper": scene_stress(palette="PAPER"), "stress_pal_mono": scene_stress(palette="MONO"), "stress_pal_ice": scene_stress(palette="ICE"),
    }
    # the cue-follows-the-hand series
    d["cue_idle_stopped"] = scene_idle()
    d["cue_idle_playing"] = scene_cue("idle_playing")
    for ly in ("all", "notes", "accnt", "shape", "artic", "hits"):
        d["cue_dice_" + ly] = scene_cue("dice_" + ly)
    d["cue_mute_held"] = scene_cue("mute")
    d["cue_mute_held"]["channels"][1]["muted"] = 1
    d["cue_solo"] = scene_state("sol")
    d["cue_fill"] = scene_state("fil")
    d["cue_roll"] = scene_state("rol")
    d["cue_undo"] = scene_cue("undo")
    d["cue_oct_latch"] = scene_row2()
    d["cue_oct_latch"]["cue"] = "oct_up"
    d["cue_save_hold"] = scene_cue("save_hold")
    d["cue_page"] = scene_cue("pg_next")
    # toasts
    for ly in LAYERS:
        d["toast_dice_" + ly.lower()] = scene_dice(ly)
    snake = scene_dice("ARTIC", algo=2)
    d["toast_dice_artic_snake"] = snake
    d["toast_undo"] = toast_scene(dict(kind="undo", what="CH1 NOTES"))
    d["toast_redo"] = toast_scene(dict(kind="redo", what="CH1 NOTES"))
    sp = toast_scene(dict(kind="preset", slot=4, frac=0.55, when="LOADS AT BAR"))
    sp.update(preset=4, preset_pend=1)
    d["toast_preset_pending"] = sp
    sp2 = toast_scene(dict(kind="preset", slot=4, frac=0.4, when="LOADS IN 1.5 S"), playing=0)
    sp2.update(preset=4, preset_pend=1, cue=None)
    d["toast_preset_pending_stopped"] = sp2
    sl = toast_scene(dict(kind="loading", slot=4))
    sl.update(preset=4, preset_load=1)
    d["toast_preset_loading"] = sl
    ta = toast_scene(dict(kind="algo", algo=1))
    ta["channels"][0]["algo"] = 1
    ta["knobs"] = list(EUC_K)
    d["toast_algo"] = ta
    d["toast_midi_channel"] = toast_scene(MIDI_TOAST)
    d["toast_midi_off"] = toast_scene(dict(kind="midi", midi=16, what="CH1 SENDS NOTHING"))
    # lane fidelity
    d["lane_full_playhead"] = scene_lane_full(7)
    d["lane_full_playhead_first"] = scene_lane_full(0)
    d["lane_vel_low"] = scene_lane_vel()
    d["lane_ratchet_sweep"] = scene_lane_sweep()
    d["lane_muted"] = scene_lane_muted()
    d["lane_offpage"] = scene_follow_off()
    # undefined states
    for n in ("ext_sync", "wait", "ext_lost", "rec_stopped", "rec_playing", "add_page", "past_length", "first_run", "reset", "sol", "fil", "rol"):
        d["state_" + n] = scene_state(n)
    d["state_follow_off"] = scene_follow_off()
    d["state_midi_channel"] = toast_scene(MIDI_TOAST)
    d["state_sel_muted_off"] = scene_stress()
    stg = scene_playing(sel=3, knobs=[6, 48, 0, 2, 0, 3, 8, 12], heard=32, cue="idle_playing")
    d["stage_total"] = stg
    for n in (0, 1, 2):
        t = scene_playing(cue_table=[(("OCT-", " ".join(P1.KNOBS[1][k][0] for k in range(4))) if kk == "oct_dn" else
                                      ("OCT+", " ".join(P1.KNOBS[4][4 + k][0] for k in range(4))) if kk == "oct_up" else kk)
                                     for kk in CUE_ORDER[11 * n:11 * n + 11]])
        d["cue_table_%d" % (n + 1)] = t
    base_map = scene_playing()
    base_map["toast"] = None
    d["map_A"] = dict(base_map, map="A")
    d["map_B"] = dict(base_map, map="B")
    d["map_A_row3"] = dict(base_map, map="A", row=2)
    d["map_menu_countdown"] = dict(base_map, map="A", countdown=0.6)
    d["map_touch"] = dict(base_map, map="A", touch=("B_FX", P1.help_line("B_FX")))
    for n, s in d.items():
        s["_name"] = n
    return d


STATE = {"scenes": scenes()}
