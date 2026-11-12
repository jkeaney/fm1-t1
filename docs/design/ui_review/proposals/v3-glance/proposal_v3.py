# SPDX-License-Identifier: GPL-3.0-only
"""proposal_v3.py: UI PROPOSAL v3-GLANCE for the FM-1 sequencer screen (mock tool, docs/design/ui_review/mock/).

The main screen is a GLANCE screen: four facts, big. Everything a producer reads while playing is AF_L (digits, capitals) or a SHAPE;
AF_M only names position-identified controls (K1..K4 labels, algorithm names, the header's BPM and preset); AF_S only on the MAP
(HOME held). Zones tile 240x240 exactly:

    Z0 head   y   0 h24    transport shape, BPM (AF_M), preset (AF_M), page boxes (shapes)
    Z1 chans  y  24 h48    four 60 px cells: channel number AF_L, algorithm AF_M, mute chip / OFF chip, activity square
    Z2 lane   y  72 h72    16 cells of 15 px (the 16 white keys), accent / slide / ratchet shapes, full-height playhead column
    Z3 knobs  y 144 h76    four 60 px cells: label AF_M, VALUE AF_L, meter shape; ROW 1 / ROW 2 latch bar
    Z4 cue    y 220 h16    one optional AF_M phrase (stopped only)
    Z5 margin y 236 h 4    bezel overscan (nothing)
    blits: A = Z0+Z1 (240x72), B = Z2 (240x72), C = Z3+Z4+Z5 (240x96); a toast replaces Z3+Z4 inside blit C.

Run:  .venv/bin/python docs/design/ui_review/mock/mock.py render docs/design/ui_review/proposals/v3-glance/proposal_v3.py --out DIR
      .venv/bin/python docs/design/ui_review/proposals/v3-glance/proposal_v3.py --report      (text inventory tables of NOTES.md)
"""
import copy
import math
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "..", "mock"))
import mock                                                                  # noqa: E402
sys.modules.setdefault("mock", mock)
from mock import (T, AF_S, AF_M, AF_L, text_w, text_ink, tables, fixture, rgb565, contrast_ratio)   # noqa: E402

PALETTE = "GREY"
W = 240

# ---------------------------------------------------------------------------------------------------------------- zones (tile 240)
Z0_Y, Z0_H = 0, 24
Z1_Y, Z1_H = 24, 48
Z2_Y, Z2_H = 72, 72
Z3_Y, Z3_H = 144, 76
Z4_Y, Z4_H = 220, 16
Z5_Y, Z5_H = 236, 4
BLIT_A_H, BLIT_B_H, BLIT_C_H = Z0_H + Z1_H, Z2_H, Z3_H + Z4_H + Z5_H
ZONES = (("Z0 head", Z0_Y, Z0_H), ("Z1 channels", Z1_Y, Z1_H), ("Z2 lane", Z2_Y, Z2_H), ("Z3 knobs", Z3_Y, Z3_H),
         ("Z4 cue", Z4_Y, Z4_H), ("Z5 margin", Z5_Y, Z5_H))
assert sum(z[2] for z in ZONES) == 240 and all(ZONES[i][1] + ZONES[i][2] == ZONES[i + 1][1] for i in range(len(ZONES) - 1))

# ---------------------------------------------------------------------------------------------------------------- new tokens (3)
# GREY has T_ACCENT = T_REC = T_TEXT. Proposed NEW palette tokens (literal RGB565 here; the monochrome fallback maps all three to T_TEXT):
T_PLAYHEAD = rgb565(255, 176, 0)        # amber: the playhead column, the playing transport triangle, the activity square, the playing page
T_ACCSTEP = rgb565(80, 200, 255)        # cyan: accented steps (bar colour). The triangle mark carries the same fact in shape
T_RECRED = rgb565(255, 72, 72)          # red: REC disc (shape also differs: disc vs triangle vs square)
ALGO_NAMES = ("ACID", "EUCLID", "SNAKE", "STAGE", "ARP", "PATT")      # AF_M, <= 58 px; PATT is 34 px (PATTRN 54 px is too tight in a 58 px cell)
ALGO_FULL = ("ACID", "EUCLID", "SNAKE", "STAGE", "ARP", "PATTERNS")   # AF_L in the algorithm toast (PATTERNS 135 px)

TB = tables()
E = TB.enums
PG_PERF = E["UI_PG_PERF"]
MODE_TAP, MODE_HOLD = E["UI_MODE_TAP"], E["UI_MODE_HOLD"]
GEST_TAP, GEST_HOLD = E["UI_GEST_TAP"], E["UI_GEST_HOLD"]

# ---------------------------------------------------------------------------------------------------------------- knob value table
# (label, lo, hi, enum-key or None). Labels and ranges are those of docs/design/knob_map.tsv (checked against the generated label table).
DENS = ("DENS", 0, 16, "KEYS")
KNOBS = (
    (DENS, ("SLIDE", 0, 8, None), ("JUMP", 0, 20, None), ("ACCNT", 0, 8, None),
     ("OFFSET", 0, 15, None), ("PEDAL", 0, 12, None), ("PUNCH", 0, 47, None), ("DIR", 0, 2, "DIR3")),                   # ACID
    (("HITS", 0, 16, "KEYS"), ("OFFSET", 0, 15, None), ("RATCH", 0, 8, None), ("ACCNT", 0, 8, None),
     ("LEN", 2, 16, None), ("SPAN", 0, 24, None), ("PUNCH", 0, 47, None), ("CURVE", 0, 6, "CURVE")),                    # EUCLID
    (DENS, ("LEN", 1, 16, None), ("RANGE", 0, 48, None), ("PATH", 0, 13, "PATH"),
     ("OFFSET", 0, 15, None), ("GLEN", 1, 16, None), ("DYN", 0, 10, None), ("DIR", 0, 2, "DIR3")),                      # SNAKE
    (("DENS", 0, 8, "KEYS"), ("TOTAL", 8, 64, None), ("DIR", 0, 2, "DIR3"), ("RATCH", 0, 8, None),
     ("OFFSET", 0, 63, None), ("RDIV", 1, 4, None), ("DYN", 0, 10, None), ("RANGE", 0, 36, None)),                      # STAGE
    (DENS, ("SHAPE", 0, 5, "SHAPE"), ("OCTS", 1, 4, None), ("RATE", 0, 5, "RATE"),                                      # ARP (K3 label SPAN -> OCTS: "1 OCT" cannot fit AF_L)
     ("OFFSET", 0, 15, None), ("LEGATO", 0, 8, None), ("DYN", 0, 10, None), ("DIR", 0, 5, "ARPDIR")),
    (DENS, ("VARY", 0, 8, None), ("SPAN", 1, 4, None), ("ACCNT", 0, 8, None),
     ("PICK", 1, 120, "PICK"), ("SLIDE", 0, 8, None), ("DYN", 0, 14, None), ("CTONE", 0, 8, None)),                     # PATTERNS
)
# Display names of the ENUMERATED knobs: (AF_L candidate, AF_M fallback). A name is drawn in AF_L when its ink fits 56 px, else AF_M.
ENUMS = {
    "DIR3": (("UP", "UP"), ("DN", "DN"), ("P-P", "PEND")),                       # was FWD BWD PEND (FWD ink 60 px in AF_L: too wide)
    "ARPDIR": (("UP", "UP"), ("DN", "DOWN"), ("U-D", "UPDN"), ("RND", "RAND"), ("ORD", "ASPL"), ("CRD", "CHRD")),   # was UP DOWN UPDN RAND ASPL CHORD
    "CURVE": (("AUT", "AUTO"), ("SIN", "SINE"), ("TRI", "TRI"), ("RP+", "SAW+"), ("RP-", "SAW-"), ("ARC", "ARCH"), ("INV", "INV")),
    "SHAPE": (("LIN", "LINE"), ("SKP", "SKIP"), ("THB", "THMB"), ("OCV", "OCTV"), ("RPT", "REPT"), ("WAV", "WAVE")),
    "RATE": (("1/4", "1/4"), ("1/2", "1/2"), ("1X", "1X"), ("2X", "2X"), ("3T", "3T"), ("4X", "4X")),
    # SNAKE PATH: the 14 traversals of snake.md 6.3 by family (the spec gives no names; these are proposed). 4 letters: AF_M.
    "PATH": tuple((None, n) for n in ("ROW1", "ROW2", "COL1", "COL2", "SNK1", "SNK2", "SNK3", "SNK4", "DIA1", "DIA2", "SPI1", "SPI2", "SPI3", "SPI4")),
    "KEYS": (),
}
# firmware strings of knob_map.tsv, for the NOTES table
FW_NAMES = {"DIR3": ("FWD", "BWD", "PEND"), "ARPDIR": ("UP", "DOWN", "UPDN", "RAND", "ASPL", "CHORD"),
            "CURVE": ("AUTO", "SINE", "TRI", "SAW+", "SAW-", "ARCH", "INV"), "SHAPE": ("LINE", "SKIP", "THUMB", "OCTV", "REPT", "WAVE"),
            "RATE": ("1/4", "1/2", "1X", "2X", "3T", "4X")}
PICK_CAT = ("ACID", "DUB", "FUNK", "HOUSE", "MEL", "TECH")                         # PATTERNS categories (MELODY / TECHNO are 59-60 px in AF_M)
L_INK_MAX = 56                                                                     # AF_L ink limit in a 60 px cell (2 px each side)


def l_fits(s):
    b = text_ink(AF_L, s)
    return b[2] - b[0] <= L_INK_MAX


def enum_in_L(en):
    """True when EVERY name of the enum fits AF_L in a 60 px cell (else the whole enum is drawn in AF_M with its <= 4-letter names)."""
    return all(ln and l_fits(ln) for ln, mn in ENUMS[en])


def knob_view(algo, idx, v):
    """(label, value text, font, meter) for knob idx 0..7 of an algorithm. meter = ('bar', frac) or ('pips', n, i)."""
    label, lo, hi, en = KNOBS[algo % 6][idx]
    if en == "KEYS":
        if v == 0:
            return label, "KEY", AF_L, ("bar", 0.0)
        return label, str(v), AF_L, ("bar", (v - lo) / float(hi - lo))
    if en == "PICK":
        cat, num = divmod(v - 1, 20)
        return PICK_CAT[cat], str(num + 1), AF_L, ("pips", 6, cat)
    if en:
        names = ENUMS[en]
        if enum_in_L(en):                                                   # one font per enum, so the cell never changes size while you turn
            return label, names[v][0], AF_L, ("pips", len(names), v)
        return label, names[v][1], AF_M, ("pips", len(names), v)
    return label, str(v), AF_L, ("bar", (v - lo) / float(hi - lo))


for _a in range(6):                                                                # the label column must equal the generated table
    for _k in range(8):
        _lab = TB.knob_label(PG_PERF, _a, _k)
        if _lab != KNOBS[_a][_k][0] and not (_a == 4 and _k == 2) and not (_lab == "SPAN" and KNOBS[_a][_k][0] == "OCTS"):
            print("  note: label table differs, algo %d knob %d: %r vs %r" % (_a, _k, _lab, KNOBS[_a][_k][0]))

# ---------------------------------------------------------------------------------------------------------------- text inventory
LOG = []
CUR = [""]


def tx(cv, role, f, s, fg, bg, x, ycap, mode="l", w=0):
    """Draws s so that its CAPITALS start at screen row ycap (canvas coordinates). Logs the item (role, string, font, colours) for the report.
    mode: l = left at x, in = ink centred across x..x+w, r = advance ends at x."""
    y = ycap - f.cap_y
    LOG.append({"scene": CUR[0], "role": role, "s": s, "f": f.name, "fg": fg, "bg": bg, "w": (w or 0)})
    if mode == "in":
        cv.text_in(x, y, w, f, s, fg, bg)
    elif mode == "r":
        cv.text_r(x, y, f, s, fg, bg)
    else:
        cv.text_on(x, y, f, s, fg, bg)


def pal(S):
    """The three new tokens, or the monochrome fallback (all three = T_TEXT)."""
    if S.get("mono"):
        return T.TEXT, T.TEXT, T.TEXT
    return T_PLAYHEAD, T_ACCSTEP, T_RECRED


# ---------------------------------------------------------------------------------------------------------------- shapes
def tri_right(cv, x, y, w, h, c):
    for r in range(h):
        k = min(r, h - 1 - r)
        cv.rect(x, y + r, max(1, int(round((k + 0.5) * 2 * w / float(h)))), 1, c)


def tri_up(cv, x, y, w, h, c):
    for r in range(h):
        wd = max(1, int(round((r + 0.5) * w / float(h))))
        wd += (wd + w) & 1                                                    # same parity as w so it stays centred
        wd = min(wd, w)
        cv.rect(x + (w - wd) // 2, y + r, wd, 1, c)


def disc(cv, cx, cy, r, c):
    for j in range(-r, r + 1):
        for i in range(-r, r + 1):
            if i * i + j * j <= r * r + r // 2:
                cv.pset(cx + i, cy + j, c)


def art(cv, x, y, rows, c, k=1):
    for j, row in enumerate(rows):
        for i, ch in enumerate(row):
            if ch == "#":
                cv.rect(x + i * k, y + j * k, k, k, c)


QMARK = (".###.", "#...#", "....#", "..##.", "..#..", ".....", "..#..")        # a '?' (AF_L has no '?'), 5x7 drawn at x3 = 15x21
DIE = ("#########", "#.......#", "#.#...#.#", "#.......#", "#...#...#", "#.......#", "#.#...#.#", "#.......#", "#########")
UNDO_ARROW = ("...#.......", "..##.......", ".#########.", "..##......#", "...#......#", ".........##", "....#####..")


def clamp(v, lo, hi):
    return max(lo, min(hi, v))


# ---------------------------------------------------------------------------------------------------------------- Z0 head
def playhead(S, ch):
    c = S["channels"][ch]
    n = c["len"] or 1
    return 255 if not S["playing"] else (c["p"] + n - 1) % n


def draw_z0(cv, S, ch):
    head, acc, red = pal(S)
    cv.begin(W, BLIT_A_H, T.BG)
    c = S["channels"][ch]
    ph, ln = playhead(S, ch), c["len"]
    # transport: a drawn shape 16x16 (play triangle / stop square / REC disc), never a character
    if S.get("rec"):
        disc(cv, 14, 12, 8, red)
    elif S["playing"]:
        tri_right(cv, 6, 4, 15, 16, head)
    else:
        cv.rect(6, 4, 15, 16, T.MID)
    bpm = ("EXT %d" % S["bpm"]) if S.get("ext") else str(S["bpm"])
    tx(cv, "bpm", AF_M, bpm, T.TEXT, T.BG, 30, 7)
    pre = "P%02d%s" % (S["preset"], "?" if S["preset_pend"] else "")
    tx(cv, "preset", AF_M, pre, T.TEXT, T.BG, 110, 7)
    # page boxes: only pages that exist; viewed = solid, others hollow; the playing page carries a 4x4 marker
    pages = (ln + 15) // 16
    for k in range(pages):
        x = 240 - 4 - 4 * 16 + 16 * k + 2 + 16 * (4 - pages) * 0
        x = 172 + 16 * k
        viewed = k == S["vpage"]
        playing = ph != 255 and ph // 16 == k
        if viewed:
            cv.rect(x, 5, 14, 14, T.TEXT)
        else:
            cv.frame(x, 5, 14, 14, T.MID)
            cv.frame(x + 1, 6, 12, 12, T.MID)
        if playing:
            cv.rect(x + 5, 10, 4, 4, T.BG if viewed else head)
            if not viewed:
                pass
    cv.mark("Z0 head", 0, 0, 240, 24)
    cv.mark("transport", 6, 4, 15, 16)
    cv.mark("page boxes", 172, 5, 62, 14)


# ---------------------------------------------------------------------------------------------------------------- Z1 channels
def draw_z1(cv, S):
    head, acc, red = pal(S)
    for i in range(4):
        c = S["channels"][i]
        x = 60 * i + 1
        y = Z1_Y - Z1_Y + 24 + 2                                            # canvas y (blit A is y 0..72; Z1 starts at 24)
        sel, mut = i == S["sel"], c["muted"]
        off = c["midi_ch"] >= 16
        if sel:
            bg, ink = T.SEL, T.INK
            cv.rect(x, y, 58, 44, bg)
        elif mut:
            bg, ink = T.BG, T.MID
            cv.frame(x, y, 58, 44, T.MID)
            cv.frame(x + 1, y + 1, 56, 42, T.MID)
        else:
            bg, ink = T.RAISE, T.TEXT
            cv.rect(x, y, 58, 44, bg)
        tx(cv, "chan number", AF_L, str(i + 1), ink, bg, x + 3, y + 5, "in", 24)
        if mut:                                                              # struck-through number: survives colour blindness
            cv.rect(x + 6, y + 14, 20, 3, T.INK if sel else T.MID)
        tx(cv, "algo name", AF_M, ALGO_NAMES[c["algo"] % 6], ink, bg, x, y + 29, "in", 58)
        if off:                                                              # MIDI OFF: hollow chip with the word (nothing leaves this channel)
            cv.frame(x + 28, y + 4, 29, 18, ink)
            cv.frame(x + 29, y + 5, 27, 16, ink)
            tx(cv, "OFF chip", AF_M, "OFF", ink, bg, x + 28, y + 8, "in", 29)
        elif mut:                                                            # MUTE chip: solid, letter M (the M1..M4 key legend)
            chip = T.INK if sel else T.MID
            cv.rect(x + 34, y + 4, 22, 18, chip)
            tx(cv, "MUTE chip", AF_M, "M", T.SEL if sel else T.INK, chip, x + 34, y + 8, "in", 22)
        elif c["snd0"]:                                                      # just sounded: solid square
            cv.rect(x + 40, y + 6, 14, 14, T.INK if sel else head)
        cv.mark("chan %d" % (i + 1), 60 * i, 24, 60, 48)


# ---------------------------------------------------------------------------------------------------------------- Z2 lane
def draw_z2(cv, S, ch):
    head, acc, red = pal(S)
    cv.begin(W, BLIT_B_H, T.BG)
    c = S["channels"][ch]
    ln = c["len"]
    ph = playhead(S, ch)
    mut = c["muted"]
    for n in range(16):
        L = S["lane"][n]
        x = 15 * n
        p = S["vpage"] * 16 + n
        live = p < ln
        if not live:
            continue
        playing = ph != 255 and p == ph
        col = head if playing else T.LANE
        cv.rect(x, 0, 14, 70, col)                                           # the whole column: the playhead is a full-height bright column
        fg = T.BG if playing else (T.MID if mut else T.TEXT)
        accfg = T.BG if playing else (T.MID if mut else acc)
        if L["on"]:
            hh = 4 + L["vel"] * 40 // 127
            top = 62 - hh
            n_r = L["rep"] if L["rep"] > 1 else 1
            barc = accfg if L["acc"] else fg
            if n_r == 1:
                segs = [(x + 1, 12)]
            else:
                w = {2: 5, 3: 3}.get(n_r, 2)
                gaps = [round(k * (12 - w) / float(n_r - 1)) for k in range(n_r)]
                segs = [(x + 1 + g, w) for g in gaps]
            for sx, sw in segs:
                if mut and not playing:
                    cv.frame(sx, top, sw, hh, barc) if sw >= 3 else cv.rect(sx, top, sw, hh, barc)
                else:
                    cv.rect(sx, top, sw, hh, barc)
            if L["acc"]:                                                     # accent: triangle 11x6 in mark row A
                tri_up(cv, x + 1, 3, 11, 6, accfg)
            if L["slide"]:                                                   # slide: a 15x6 tie bar in mark row B; runs of slides chain into a line
                cv.rect(x, 11, 15 if n < 15 else 14, 6, T.BG if playing else (T.MID if mut else T.TEXT))
        else:
            cv.rect(x + 1, 58, 12, 4, T.BG if playing else T.MID)           # a rest: low flat dash
        if n % 4 == 0:                                                       # beat pips (shape): a short bar under steps 1, 5, 9, 13
            cv.rect(x + 4, 65, 6, 3, T.BG if playing else T.MID)
    cv.mark("Z2 lane", 0, 0, 240, 72)
    for n in range(16):
        cv.mark("", 15 * n, 0, 15, 70) if False else None


# ---------------------------------------------------------------------------------------------------------------- Z3 knobs
def draw_row_bar(cv, y, row, head):
    """ROW 1 / ROW 2 latch: two halves under the knob cells; left half = OCT-, right half = OCT+. Active = solid with I / II drawn in INK."""
    for h in range(2):
        x = 1 + 120 * h
        live = row == h
        if live:
            cv.rect(x, y, 118, 10, T.TEXT)
        else:
            cv.frame(x, y, 118, 10, T.MID)
        cx = x + 59
        c = T.INK if live else T.MID
        if h == 0:
            cv.rect(cx - 1, y + 2, 3, 6, c)
        else:
            cv.rect(cx - 4, y + 2, 3, 6, c)
            cv.rect(cx + 2, y + 2, 3, 6, c)


def draw_meter(cv, x, y, meter, fg, track):
    if meter[0] == "bar":
        cv.rect(x, y, 46, 10, track)
        fw = int(round(46 * meter[1]))
        if meter[1] > 0 and fw < 2:
            fw = 2
        cv.rect(x, y, fw, 10, fg)
    else:
        n, i = meter[1], meter[2]
        g = 1 if n > 8 else 2
        pw = (46 - (n - 1) * g) // n
        tot = n * pw + (n - 1) * g
        x0 = x + (46 - tot) // 2
        for k in range(n):
            if k == i:
                cv.rect(x0 + k * (pw + g), y, pw, 10, fg)
            else:
                cv.rect(x0 + k * (pw + g), y + 6, pw, 4, T.MID)


def draw_z3(cv, S, ch, y0=0):
    head, acc, red = pal(S)
    c = S["channels"][ch]
    algo = c["algo"] % 6
    for k in range(4):
        x = 60 * k + 1
        idx = S["row"] * 4 + k
        label, val, f, meter = knob_view(algo, idx, S["knobs"][idx])
        hot = S["hot"][k]
        bg = T.SURF
        cv.rect(x, y0, 58, 66, bg)
        if hot:
            cv.frame(x, y0, 58, 66, T.TEXT)
            cv.frame(x + 1, y0 + 1, 56, 64, T.TEXT)
        tx(cv, "knob label", AF_M, label, T.TEXT, bg, x, y0 + 6, "in", 58)
        if f is AF_L:
            tx(cv, "knob value", AF_L, val, T.TEXT, bg, x, y0 + 21, "in", 58)
        else:
            tx(cv, "knob value (M)", AF_M, val, T.TEXT, bg, x, y0 + 26, "in", 58)
        draw_meter(cv, x + 6, y0 + 50, meter, T.TEXT, T.LANE if not hot else T.LANE)
        cv.mark("knob %d" % (k + 1), 60 * k, y0, 60, 66)
    draw_row_bar(cv, y0 + 66, S["row"], head)


# ---------------------------------------------------------------------------------------------------------------- toasts (replace Z3+Z4)
def draw_toast(cv, S, ch):
    head, acc, red = pal(S)
    t = S["toast"]
    cv.rect(0, 2, 240, 88, T.SEL)
    k = t["kind"]
    big_y, small_y = 2 + 16, 2 + 50
    if k == "algo":
        a = t["algo"]
        tx(cv, "toast title", AF_L, ALGO_FULL[a], T.INK, T.SEL, 0, big_y, "in", 240)
        tx(cv, "toast detail", AF_M, "CH %d ALGORITHM" % (ch + 1), T.INK, T.SEL, 0, small_y, "in", 240)
        n = 6
        pw, g = 20, 6
        x0 = (240 - (n * pw + (n - 1) * g)) // 2
        for j in range(n):                                                   # the six algorithms as pips: the chosen one is tall
            if j == a:
                cv.rect(x0 + j * (pw + g), 68, pw, 14, T.INK)
            else:
                cv.rect(x0 + j * (pw + g), 76, pw, 6, T.INK if False else T.MID)
    elif k == "preset":
        s = "P%02d" % t["slot"]
        x0 = (240 - (text_w(AF_L, s) + 6 + 15)) // 2
        tx(cv, "toast title", AF_L, s, T.INK, T.SEL, x0, big_y, "l")
        art(cv, x0 + text_w(AF_L, s) + 6, big_y, QMARK, T.INK, 3)             # AF_L has no '?': drawn 5x7 at x3
        tx(cv, "toast detail", AF_M, "SAVE = SAVE HERE", T.INK, T.SEL, 0, small_y, "in", 240)
        cv.rect(40, 70, 160, 10, T.MID)                                       # settle countdown (1.5 s), shape only
        cv.rect(40, 70, int(160 * t["frac"]), 10, T.INK)
        cv.rect(40, 70, 1, 10, T.INK)
    elif k == "dice":
        art(cv, 22, 28, DIE, T.INK, 4)
        tx(cv, "toast title", AF_L, t["layer"], T.INK, T.SEL, 66, big_y, "in", 160)
        tx(cv, "toast detail", AF_M, t["detail"], T.INK, T.SEL, 66, small_y, "in", 160)
    elif k == "undo":
        art(cv, 20, 30, UNDO_ARROW, T.INK, 4)
        tx(cv, "toast title", AF_L, "UNDO", T.INK, T.SEL, 66, big_y, "in", 160)
        tx(cv, "toast detail", AF_M, t["detail"], T.INK, T.SEL, 66, small_y, "in", 160)
    cv.mark("toast (replaces Z3+Z4)", 0, 2, 240, 88)


def draw_z4(cv, S, y):
    if S["playing"] or S.get("rec"):
        return                                                               # nothing while playing: the cue only earns its place stopped
    tx(cv, "cue", AF_M, "HOLD HOME = MAP", T.TEXT, T.BG, 0, y + 3, "in", 240)
    cv.mark("Z4 cue", 0, y, 240, 16)


# ---------------------------------------------------------------------------------------------------------------- MAP (HOME held)
BTN_ROWS = (("FX", "ALL", ""), ("SCL", "NOTES", ""), ("ENV", "ACCNT", ""), ("LFO", "SHAPE", ""), ("EDIT", "UNDO", "RESET"), ("GLO", "SET", "HARM"),
            ("HOME", "HOME", "MENU"), ("SAVE", "SAVE", "BACK"), ("ARP", "ARTIC", ""), ("SEQ", "HITS", ""), ("PLAY", "PLAY", ""), ("REC", "REC", ""))
BLACK = (("M1", None), ("M2", None), ("M3", None), ("M4", None), ("CHN", None), (None, "SOL"), (None, "FIL"), (None, "ROL"),
         ("PG-", None), ("PG+", None), ("RST", None))
BLACK_X = (0, 21, 42, 65, 86, 109, 130, 151, 174, 195, 218)               # SCREEN_SPEC 2.6 LEG_X: 3-2-3-2-1 like the keybed


def boxed(cv, role, x, y, w, label, fg, bg):
    """HOLD role = the word inside an outline box (one device for buttons and black keys)."""
    cv.frame(x, y, w, 13, T.MID)
    tx(cv, role, AF_S, label, fg, bg, x, y + 2, "in", w)


def draw_map(cv, S):
    """Strips: A y0..92 (title, encoders 2x2, OCT, K1..K4), B y92..165 (2x6 buttons), C y165..234 (black keys on two tiers, caption, white keys),
    then a 6 px margin. Every text is AF_S; roles are the CURRENT ones of the selected channel (ROW state, algorithm)."""
    ch = S["sel"]
    algo = S["channels"][ch]["algo"] % 6
    cv.begin(W, 92, T.BG)
    tx(cv, "map title", AF_S, "TOUCH = HELP   LET GO = MENU", T.TEXT, T.BG, 0, 3, "in", 240)
    cv.rect(0, 14, 240, 1, T.LINE)
    enc = (("MASTER", "VOLUME"), ("SELECT", "CHANNEL"), ("PRESETS", "P01-P16"), ("ALGORITHM", "OF CH %d" % (ch + 1)))
    for i, (nm, role) in enumerate(enc):                                     # the panel's 2x2 encoder block
        x, y = 66 * (i % 2), 16 + 25 * (i // 2)
        cv.rect(x, y, 65, 24, T.SURF)
        tx(cv, "map encoder name", AF_S, nm, T.MID, T.SURF, x, y + 2, "in", 65)
        tx(cv, "map encoder role", AF_S, role, T.TEXT, T.SURF, x, y + 13, "in", 65)
    for i, nm in enumerate(("OCT-", "OCT+")):                                # OCT- OCT+ under the encoders; the live row is the inverted one
        x, y = 66 * i, 67
        live = S["row"] == i
        bg = T.SEL if live else T.SURF
        cv.rect(x, y, 65, 24, bg)
        tx(cv, "map OCT name", AF_S, nm, T.INK if live else T.MID, bg, x, y + 2, "in", 65)
        tx(cv, "map OCT role", AF_S, "ROW %d" % (i + 1), T.INK if live else T.TEXT, bg, x, y + 13, "in", 65)
    for k in range(4):                                                       # K1..K4: ROW 1 name, ROW 2 name; the live row is bright
        y = 16 + 12 * k
        tx(cv, "map knob id", AF_S, "K%d" % (k + 1), T.MID, T.BG, 134, y + 1, "l")
        tx(cv, "map knob row1", AF_S, KNOBS[algo][k][0], T.TEXT if S["row"] == 0 else T.MID, T.BG, 134 + 17, y + 1, "l")
        tx(cv, "map knob row2", AF_S, KNOBS[algo][4 + k][0], T.TEXT if S["row"] == 1 else T.MID, T.BG, 134 + 62, y + 1, "l")
    cv.mark("A title", 0, 0, 240, 15)
    cv.mark("A encoders 2x2", 0, 16, 131, 49)
    cv.mark("A OCT- OCT+", 0, 67, 131, 24)
    cv.mark("A K1..K4", 133, 16, 107, 49)
    boxed(cv, "map legend", 134, 74, 14, "", T.MID, T.BG) if False else None
    cv.frame(134, 70, 12, 13, T.MID)
    tx(cv, "map legend", AF_S, "= HOLD", T.TEXT, T.BG, 150, 72, "l")
    cv.blit(0, 0)
    cv.begin(W, 73, T.BG)                                                    # strip B: the 2x6 buttons, row 1 FX..GLO, row 2 HOME..REC
    cv.rect(0, 0, 240, 1, T.LINE)
    for i, (printed, tap, hold) in enumerate(BTN_ROWS):
        x, y = 40 * (i % 6), 1 + 36 * (i // 6)
        cv.rect(x + 1, y, 38, 36, T.SURF)
        tx(cv, "map button printed", AF_S, printed, T.MID, T.SURF, x + 1, y + 2, "in", 38)
        tx(cv, "map button tap", AF_S, tap, T.TEXT, T.SURF, x + 1, y + 13, "in", 38)
        if hold:
            boxed(cv, "map button hold", x + 2, y + 23, 36, hold, T.TEXT, T.SURF)
    cv.mark("B buttons 2x6", 0, 0, 240, 73)
    cv.blit(0, 92)
    cv.begin(W, 69, T.BG)                                                    # strip C: black keys (two tiers + leader lines), caption, white keys
    cv.rect(0, 0, 240, 1, T.LINE)
    for i, (tap, hold) in enumerate(BLACK):
        x = BLACK_X[i]
        cx = x + 10
        cv.rect(x + 2, 3, 16, 8, T.KEY if tap else T.RAISE)
        ty = 13 if i % 2 == 0 else 27
        cv.rect(cx, 11, 1, ty - 11 + 1, T.MID)
        lab = tap or hold
        w = text_w(AF_S, lab) + (4 if hold else 0)
        lx = clamp(cx - w // 2, 0, 240 - w)
        if hold:
            boxed(cv, "map black key hold", lx, ty, w, lab, T.TEXT, T.BG)
        else:
            tx(cv, "map black key", AF_S, lab, T.TEXT, T.BG, lx, ty + 2, "l")
    cv.mark("C black keys (2 tiers)", 0, 1, 240, 40)
    tx(cv, "map caption", AF_S, "STEPS 1-16: TAP ON/OFF  HOLD EDIT", T.TEXT, T.BG, 0, 44, "in", 240)
    cv.mark("C caption", 0, 42, 240, 12)
    for n in range(16):
        x = 15 * n
        cv.rect(x, 55, 14, 13, T.RAISE)
        tx(cv, "map white key", AF_S, str(n + 1), T.TEXT, T.RAISE, x, 55 + 2, "in", 14)
    cv.mark("C white keys", 0, 55, 240, 13)
    cv.blit(0, 165)


# ---------------------------------------------------------------------------------------------------------------- draw
def draw(cv, S):
    CUR[0] = S.get("_name", "")
    if S.get("map"):
        draw_map(cv, S)
        return
    ch = S["sel"]
    draw_z0(cv, S, ch)                                                       # blit A = Z0 + Z1
    draw_z1(cv, S)
    cv.blit(0, 0)
    draw_z2(cv, S, ch)                                                       # blit B = Z2
    cv.blit(0, Z2_Y)
    cv.begin(W, BLIT_C_H, T.BG)                                              # blit C = Z3 + Z4 + Z5
    if S.get("toast"):
        draw_toast(cv, S, ch)
    else:
        draw_z3(cv, S, ch)
        draw_z4(cv, S, Z4_Y - Z3_Y)
    cv.mark("Z5 margin", 0, 92, 240, 4)
    cv.blit(0, Z3_Y)


# ---------------------------------------------------------------------------------------------------------------- fixtures
def base(name, **kw):
    s = copy.deepcopy(fixture(name))
    s.update(kw)
    return s


def knobs_for(vals):
    return list(vals)


ACID_K = (12, 6, 5, 3, 0, 4, 40, 0)
EUC_K = (5, 0, 4, 2, 16, 12, 40, 0)
SNAKE_K = (10, 7, 12, 7 - 1 + 0, 0, 16, 10, 0)              # PATH index 6 = SNK3 (the default path 7)
STAGE_K = (6, 16, 0, 2, 0, 4, 10, 12)


def lane_snake():
    return copy.deepcopy(fixture("perf_snake_mute")["lane"])


def scene_idle():
    return base("perf", knobs=list(ACID_K), rec=0, hot=[0, 0, 0, 0])


def scene_playing():
    s = base("perf_playing", knobs=list(ACID_K), rec=0)
    s["channels"][1]["snd0"] = 1                                             # channels 1 and 2 just sounded
    s["channels"][1]["p"] = 9
    return s


def scene_row2():
    s = base("perf_row2", knobs=list(ACID_K))
    s["hot"] = [0, 0, 1, 0]
    return s


def scene_mutes():
    s = base("perf_snake_mute", knobs=list(SNAKE_K), sel=2, playing=1, playhead=9)
    ch = s["channels"]
    ch[0].update(algo=0, midi_ch=0, muted=0, len=16, p=10, snd0=1)
    ch[1].update(algo=1, midi_ch=1, muted=1, len=16, p=4)
    ch[2].update(algo=2, midi_ch=2, muted=0, len=20, p=10, snd0=0, pg_used=3)
    ch[3].update(algo=3, midi_ch=3, muted=1, len=16, p=7)
    return s


def scene_stress():
    s = base("perf_snake_mute", knobs=[8, 64, 2, 8, 63, 4, 10, 36], sel=3, vpage=3, playing=1, rec=1, ext=1, bpm=126, preset=16,
             hot=[0, 1, 0, 0])
    ch = s["channels"]
    ch[0].update(algo=4, midi_ch=15, muted=0, len=16, snd0=1, p=3)
    ch[1].update(algo=5, midi_ch=1, muted=0, len=32, snd0=0, p=2, pg_used=3)
    ch[2].update(algo=2, midi_ch=16, muted=0, len=16, p=2)                  # MIDI OFF
    ch[3].update(algo=3, midi_ch=15, muted=1, len=64, p=56, pg_used=15)     # selected AND muted, page 4/4, playhead on step 56 (index 7)
    steps = [
        (1, 30, 0, 0, 1), (1, 55, 0, 0, 1), (0, 1, 0, 0, 1), (1, 100, 1, 0, 1),                       # 49..52: rest, accent
        (1, 80, 0, 1, 1), (1, 90, 0, 0, 2), (1, 127, 1, 1, 1), (1, 110, 1, 1, 3),                       # 53..56: slide, ratchet x2, accent+slide, accent+slide+ratchet x3
        (1, 70, 0, 0, 4), (1, 40, 1, 0, 1), (0, 1, 0, 0, 1), (1, 60, 0, 1, 1),
        (1, 90, 0, 1, 1), (1, 20, 0, 0, 1), (1, 120, 1, 0, 2), (0, 1, 0, 0, 1)]
    s["lane"] = [dict(on=a, vel=b, ed=0, acc=c, slide=d, rep=e) for (a, b, c, d, e) in steps]
    return s


def scene_stress2():
    s = scene_stress()
    s["row"] = 1
    s["knobs"] = [9, 0, 10, 5, 63, 4, 10, 36]                                  # row 2: OFFSET RDIV DYN RANGE ... (first 4 unused)
    s["knobs"] = [8, 64, 2, 8, 63, 4, 10, 36]
    s["hot"] = [0, 0, 0, 0]
    return s


def toast_scene(kind, **kw):
    s = scene_playing()
    s["toast"] = dict(kind=kind, **kw)
    return s


def scene_toast_algo():
    s = toast_scene("algo", algo=1)
    s["channels"][0]["algo"] = 1
    s["knobs"] = list(EUC_K)
    return s


def scene_toast_preset():
    s = toast_scene("preset", slot=4, frac=0.55)
    s["preset"], s["preset_pend"] = 4, 1
    return s


def scene_toast_dice():
    return toast_scene("dice", layer="NOTES", detail="CH 1  NEW PITCHES")


def scene_toast_undo():
    return toast_scene("undo", detail="DICE NOTES")


def scene_map():
    s = scene_playing()
    s["map"] = True
    s["toast"] = None
    return s


def scenes():
    d = {
        "glance_idle": scene_idle(), "glance_playing": scene_playing(), "row2": scene_row2(), "mutes": scene_mutes(), "stress": scene_stress(),
        "stress_row2": scene_stress2(), "toast_algo": scene_toast_algo(), "toast_preset_pending": scene_toast_preset(),
        "toast_dice_notes": scene_toast_dice(), "toast_undo": scene_toast_undo(), "map": scene_map(),
    }
    m1 = scene_playing()
    m1["mono"] = True
    d["glance_playing_mono"] = m1
    m2 = scene_stress()
    m2["mono"] = True
    d["stress_mono"] = m2
    for n, s in d.items():
        s["_name"] = n
    return d


STATE = {"scenes": scenes()}


if __name__ == "__main__" and "--report" in sys.argv:
    import report_v3
    report_v3.main()
