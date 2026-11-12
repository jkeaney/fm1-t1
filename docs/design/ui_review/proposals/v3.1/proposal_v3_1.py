# SPDX-License-Identifier: GPL-3.0-only
"""proposal_v3_1.py: UI PROPOSAL v3.1 (glance) for the FM-1 sequencer screen (mock tool, docs/design/ui_review/mock/).

v3-glance refined (see NOTES.md for each change and its verdict). Zones tile 240x240 exactly:

    Z0 head   y   0 h24    transport shape, BPM (AF_M), preset (AF_M), page boxes (shapes); a toast banner replaces it
    Z1 chans  y  24 h48    four 58x44 cells (pitch 60, margin 1, gap 2): number AF_L, algorithm AF_M, mute / OFF marks, activity square
    Z2 lane   y  72 h72    16 columns of 13 px at pitch 15 (the 16 white keys, 4 per cell group), marks, full-height playhead column
    Z3 knobs  y 144 h76    four 58x58 cells (label AF_M, value AF_L or a picture + AF_M name, meter) + a 16 px ROW bar (ROW 1 / ROW 2)
    Z4 cue    y 220 h16    one optional AF_M phrase (stopped only)
    Z5 margin y 236 h 4    bezel overscan (nothing)

One grid for everything: margin 1, gap 2, cell 58 (channel cells, knob cells, ROW halves) and lane columns 13 at pitch 15, so the 4 columns of
a lane group have the same outer edges (x 1..58) as a cell. Vertical gaps are 2 px too (cells 26..70, lane 72..142, knobs 144..202).
No rounded rectangles anywhere (no rrect call in this file: the rounded look of v3 came from AF_L glyphs and discs).

Run:  .venv/bin/python docs/design/ui_review/proposals/v3.1/render_all.py          (all scenes, all PNGs, before/after sheets)
      .venv/bin/python docs/design/ui_review/proposals/v3.1/report_v3_1.py        (the tables of NOTES.md)
"""
import copy
import math
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, "..", "..", "..", "..", ".."))
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
CELL_W, CELL_PITCH, MARGIN = 58, 60, 1            # the one horizontal grid: x = 1 + 60 k, 58 wide, gap 2
KNOB_H = 58                                       # knob cell height (Z3: 58 cell + 2 gap + 16 ROW bar = 76)
ROWBAR_Y, ROWBAR_H = 60, 16                       # inside blit C
LANE_COL, LANE_PITCH = 13, 15                     # columns x = 1 + 15 n, 13 wide, gap 2 (same outer edges as the cells)

# ---------------------------------------------------------------------------------------------------------------- tokens per palette
# (T_PLAYHEAD, T_ACCSTEP, T_REC) as RGB565. T_REC already exists in every colour palette (FA49; PAPER B8C5); GREY and MONO have it white,
# so GREY gets the literal red of the colour palettes here (a palette-table change if adopted), MONO keeps the monochrome fallback (all T_TEXT).
AMBER_ = rgb565(255, 176, 0)           # 0xFD80
CYAN_ = rgb565(80, 200, 255)           # 0x565F
RED_ = 0xFA49
PAL_TOKENS = {
    "GREY": (AMBER_, CYAN_, RED_), "GREEN": (AMBER_, CYAN_, RED_), "AMBER": (AMBER_, CYAN_, RED_), "ICE": (AMBER_, CYAN_, RED_),
    "VIOLET": (AMBER_, CYAN_, RED_), "ROSE": (AMBER_, CYAN_, RED_), "NIGHT": (AMBER_, CYAN_, RED_),
    "PAPER": (rgb565(131, 89, 16), rgb565(16, 97, 131), 0xB8C5),          # light surface: dark tokens, 3:1 or better against BG / LANE / RAISE
    "MONO": None,                                                         # None = monochrome fallback: all three are T_TEXT
}
ALGO_NAMES = ("ACID", "EUCLID", "SNAKE", "STAGE", "ARP", "PATT")      # AF_M, <= 50 px in a 58 px cell
ALGO_FULL = ("ACID", "EUCLID", "SNAKE", "STAGE", "ARP", "PATTERNS")   # AF_L in the algorithm toast (PATTERNS 135 px)
ALGO_L3 = ("ACD", "EUC", "SNK", "STG", "ARP", "PAT")                  # AF_L 3-letter forms for the selected-cell experiment (item 9)

TB = tables()
E = TB.enums
PG_PERF = E["UI_PG_PERF"]

# ---------------------------------------------------------------------------------------------------------------- SNAKE paths (the real tables)
def _load_paths():
    src = open(os.path.join(REPO, "docs", "design", "algorithms", "snake", "proto.c")).read()
    m = re.search(r"SHAPE\[14\]\[16\]\s*=\s*\{(.*?)\n\};", src, re.S)
    rows = re.findall(r"\{([^}]*)\}", m.group(1))
    tab = [[int(v) for v in r.replace("\n", " ").split(",") if v.strip()] for r in rows]
    assert len(tab) == 14 and all(sorted(r) == list(range(1, 17)) for r in tab)
    return tab


SNAKE_SHAPE = _load_paths()               # SHAPE[n-1][k] = the 1-based step at which cell k (reading order, 4x4) plays


def snake_route(n):
    """The route of path n (1..14) as a list of 16 (row, col) cells in step order."""
    pos = [None] * 16
    for k, s in enumerate(SNAKE_SHAPE[n - 1]):
        pos[s - 1] = (k // 4, k % 4)
    return pos


# ---------------------------------------------------------------------------------------------------------------- knob value table
# (label, lo, hi, enum-key or None). Labels and ranges are those of docs/design/knob_map.tsv.
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
    (DENS, ("SHAPE", 0, 5, "SHAPE"), ("OCTS", 1, 4, None), ("RATE", 0, 5, "RATE"),                                      # ARP (K3 label SPAN -> OCTS)
     ("OFFSET", 0, 15, None), ("LEGATO", 0, 8, None), ("DYN", 0, 10, None), ("DIR", 0, 5, "ARPDIR")),
    (DENS, ("VARY", 0, 8, None), ("SPAN", 1, 4, None), ("ACCNT", 0, 8, None),
     ("PICK", 1, 120, "PICK"), ("SLIDE", 0, 8, None), ("DYN", 0, 14, None), ("CTONE", 0, 8, None)),                     # PATTERNS
)
# Enumerations. FIRMWARE names are kept (no invented abbreviations). kind "text": AF_L when every name fits 56 px. kind "pic": a picture of the
# value in the cell + the firmware name in AF_M under it. kind "path": the route picture + the path number.
ENUMS = {
    "DIR3": ("pic", ("FWD", "BWD", "PEND")),
    "ARPDIR": ("pic", ("UP", "DOWN", "UPDN", "RAND", "ASPL", "CHORD")),
    "CURVE": ("pic", ("AUTO", "SINE", "TRI", "SAW+", "SAW-", "ARCH", "INV")),
    "SHAPE": ("pic", ("LINE", "SKIP", "THUMB", "OCTV", "REPT", "WAVE")),
    "RATE": ("text", ("1/4", "1/2", "1X", "2X", "3T", "4X")),
    "PATH": ("path", tuple("path %d" % n for n in range(1, 15))),
    "KEYS": ("text", ()),
}
PICK_CAT = ("ACID", "DUB", "FUNK", "HOUSE", "MEL", "TECH")                         # PATTERNS categories (MELODY / TECHNO are 59-60 px in AF_M)
L_INK_MAX = 56

# Pictures: dot plots (col, level) at 5 px x pitch and 4 px y pitch, 3x3 dots. Sequences are the spec's own figures (arp SPEC 6.5 / 6.8).
PICS = {
    "DIR3": ([(i, i) for i in range(6)], [(i, 5 - i) for i in range(6)], [(0, 0), (1, 1), (2, 2), (3, 3), (4, 4), (5, 3), (6, 2), (7, 1)]),
    "ARPDIR": ([(i, i) for i in range(6)], [(i, 5 - i) for i in range(6)], [(0, 0), (1, 1), (2, 2), (3, 3), (4, 4), (5, 3), (6, 2), (7, 1)],
               [(0, 3), (1, 0), (2, 5), (3, 1), (4, 4), (5, 2), (6, 5), (7, 0)],
               [(0, 1), (1, 3), (2, 2), (3, 4), (4, 0), (5, 2)],
               [(c_, l_) for c_ in (0, 2, 4) for l_ in range(4)]),                       # CHORD: the chord (4 tones stacked), three times
    "SHAPE": ([(i, (0, 1, 2, 0, 1, 2)[i]) for i in range(6)],                                    # LINE  0 1 2 0 1 2
              [(i, (0, 2, 1, 0, 2, 1)[i]) for i in range(6)],                                    # SKIP  tone, tone+2, tone+1
              [(i, (0, 1, 0, 2, 0, 1, 0, 2)[i]) for i in range(8)],                              # THUMB the home note alternates with each tone
              [(i, (0, 3, 1, 4, 2, 5)[i]) for i in range(6)],                                    # OCTV  each tone, then its octave
              [(i, (0, 0, 1, 1, 2, 2)[i]) for i in range(6)],                                    # REPT  each tone twice
              [(i, (0, 1, 2, 1, 2, 0, 2, 0, 1)[i]) for i in range(9)]),                          # WAVE  three-note windows rolling
}


def curve_pts(i, w=43, h=21):
    """Polyline (x, y) in a w x h box for euclid CURVE i (0 AUTO seeded, 1 SINE, 2 TRI, 3 SAW+, 4 SAW-, 5 ARCH, 6 INV)."""
    n = 15
    pts = []
    for k in range(n):
        t = k / float(n - 1)
        if i == 0:
            v = (0.2, 0.8, 0.45, 0.95, 0.1, 0.6, 0.3, 0.9, 0.5, 0.15, 0.75, 0.35, 0.85, 0.25, 0.6)[k]
        elif i == 1:
            v = 0.5 + 0.5 * math.sin(2 * math.pi * t)
        elif i == 2:
            v = 1 - abs(2 * t - 1) * 1.0 if t <= 0.5 else 1 - (t - 0.5) * 2
            v = 2 * t if t <= 0.5 else 2 - 2 * t
        elif i == 3:
            v = t
        elif i == 4:
            v = 1 - t
        elif i == 5:
            v = math.sin(math.pi * t)
        else:
            v = 1 - math.sin(math.pi * t)
        pts.append((int(round(t * (w - 1))), int(round((1 - v) * (h - 1)))))
    return pts


def l_fits(s):
    b = text_ink(AF_L, s)
    return b[2] - b[0] <= L_INK_MAX


def knob_view(algo, idx, v):
    """(label, value text, font, meter, pic) for knob idx 0..7 of an algorithm. meter = ('bar', frac) or ('pips', n, i);
    pic = None or (kind, enum-key, value index, name text)."""
    label, lo, hi, en = KNOBS[algo % 6][idx]
    if en == "KEYS":
        if v == 0:
            return label, "KEY", AF_L, ("bar", 0.0), None
        return label, str(v), AF_L, ("bar", (v - lo) / float(hi - lo)), None
    if en == "PICK":
        cat, num = divmod(v - 1, 20)
        return PICK_CAT[cat], str(num + 1), AF_L, ("pips", 6, cat), None
    if en:
        kind, names = ENUMS[en]
        if kind == "path":
            return label, str(v + 1), AF_M, None, ("path", en, v, str(v + 1))
        if kind == "pic":
            return label, names[v], AF_M, None, ("pic", en, v, names[v])
        return label, names[v], AF_L, ("pips", len(names), v), None
    return label, str(v), AF_L, ("bar", (v - lo) / float(hi - lo)), None


for _a in range(6):                                                                # the label column must equal the generated table
    for _k in range(8):
        _lab = TB.knob_label(PG_PERF, _a, _k)
        if _lab != KNOBS[_a][_k][0] and not (_a == 4 and _k == 2) and not (_lab == "SPAN" and KNOBS[_a][_k][0] == "OCTS"):
            print("  note: label table differs, algo %d knob %d: %r vs %r" % (_a, _k, _lab, KNOBS[_a][_k][0]))

# ---------------------------------------------------------------------------------------------------------------- text inventory
LOG = []
CUR = [""]


def tx(cv, role, f, s, fg, bg, x, ycap, mode="l", w=0):
    """Draws s so that its CAPITALS start at screen row ycap (canvas coordinates). Logs the item for the report.
    mode: l = pen at x, il = ink starts at x, in = ink centred across x..x+w, r = advance ends at x, ir = ink ends at x."""
    y = ycap - f.cap_y
    LOG.append({"scene": CUR[0], "role": role, "s": s, "f": f.name, "fg": fg, "bg": bg, "w": (w or 0)})
    if mode == "in":
        cv.text_in(x, y, w, f, s, fg, bg)
    elif mode == "r":
        cv.text_r(x, y, f, s, fg, bg)
    elif mode == "il":
        cv.text_on(x - text_ink(f, s)[0], y, f, s, fg, bg)
    elif mode == "ir":
        b = text_ink(f, s)
        cv.text_on(x - b[2], y, f, s, fg, bg)
    else:
        cv.text_on(x, y, f, s, fg, bg)


def pal(S):
    """(T_PLAYHEAD, T_ACCSTEP, T_REC) for the palette in force, or the monochrome fallback (all T_TEXT)."""
    if S.get("mono"):
        return T.TEXT, T.TEXT, T.TEXT
    t = PAL_TOKENS.get(mock.ux.palette)
    return t if t else (T.TEXT, T.TEXT, T.TEXT)


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


def ring_slash(cv, x, y, c, n=14):
    """OFF mark: a 14x14 ring (2 px) with a 2 px slash from lower left to upper right (the 'no' sign). Pixel-exact, even size."""
    r0 = n / 2.0
    for j in range(n):
        for i in range(n):
            dx, dy = i + 0.5 - r0, j + 0.5 - r0
            d2 = dx * dx + dy * dy
            ring = (r0 - 2.0) ** 2 < d2 <= r0 * r0
            slash = d2 <= (r0 - 1.0) ** 2 and abs(dx + dy) <= 1.4
            if ring or slash:
                cv.pset(x + i, y + j, c)


def diamond(cv, x, y, c):
    """5x5 solid diamond (edited step mark)."""
    for j, w in enumerate((1, 3, 5, 3, 1)):
        cv.rect(x + (5 - w) // 2, y + j, w, 1, c)


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


# ---------------------------------------------------------------------------------------------------------------- toast banner (header)
BANNER = {"header": (0, 0, 240, 24, 1, 6), "bottom": (1, 60, 238, 32, 5, 10)}     # x, y, w, h, AF_L cap top, AF_M cap top (all inside the band)
DETAIL = {"algo": ("CH %d ALGORITHM", "ALGORITHM"), "preset": ("SAVE = SAVE HERE", "SAVE HERE"), "dice": ("CH %d  NEW PITCHES", "NEW PITCHES"),
          "undo": ("%s", "%s")}


def draw_banner(cv, S, ch, t, place):
    """A one-line toast: inverted band, square corners, [icon] title in AF_L at the left, detail in AF_M at the right (the short form when the long
    one would touch the title). place = header (Z0, 24 px), bottom (ROW bar + cue, 32 px, in blit C coordinates) or lane (a 70 px block over Z2)."""
    k = t["kind"]
    if place == "lane":
        return draw_lane_toast(cv, S, ch, t)
    x0, y0, w, h, capT, detT = BANNER[place]
    cv.rect(x0, y0, w, h, T.SEL)
    ix = x0 + 5                                                              # content starts here
    iy = y0 + (h - 18) // 2
    if k == "algo":
        title = ALGO_FULL[t["algo"]]
    elif k == "preset":
        title = "P%02d" % t["slot"]
    elif k == "dice":
        title = t["layer"]
        art(cv, ix, y0 + (h - 18) // 2, DIE, T.INK, 2)
        ix += 18 + 5
    else:
        title = "UNDO"
        art(cv, ix, y0 + (h - 14) // 2, UNDO_ARROW, T.INK, 2)
        ix += 22 + 5
    tw = text_ink(AF_L, title)[2] - text_ink(AF_L, title)[0]
    tx(cv, "toast title", AF_L, title, T.INK, T.SEL, ix, y0 + capT, "il")
    ex = ix + tw
    if k == "preset":                                                        # AF_L has no '?': drawn 5x7 at x3
        art(cv, ex + 5, y0 + capT, QMARK, T.INK, 3)
        ex += 5 + 15
    longd, shortd = DETAIL[k]
    if k == "undo":
        longd = shortd = t["detail"]
    elif "%d" in longd:
        longd = longd % (ch + 1)
    right = x0 + w - 6
    for d in (longd, shortd):
        dw = text_ink(AF_M, d)[2] - text_ink(AF_M, d)[0]
        if right - dw - ex >= 8:
            break
    tx(cv, "toast detail", AF_M, d, T.INK, T.SEL, right, y0 + detT, "ir")
    if k == "preset":                                                        # settle countdown (1.5 s): a 2 px bar on the band's bottom edge
        cv.rect(x0, y0 + h - 2, w, 2, T.MID)
        cv.rect(x0, y0 + h - 2, int(w * t["frac"]), 2, T.INK)
    cv.mark("toast band (%s)" % place, x0, y0, w, h)


def draw_lane_toast(cv, S, ch, t):
    k = t["kind"]
    cv.rect(0, 0, 240, 70, T.SEL)
    big_y, small_y = 12, 42
    if k == "algo":
        tx(cv, "toast title", AF_L, ALGO_FULL[t["algo"]], T.INK, T.SEL, 0, big_y, "in", 240)
        tx(cv, "toast detail", AF_M, "CH %d ALGORITHM" % (ch + 1), T.INK, T.SEL, 0, small_y, "in", 240)
    elif k == "preset":
        s = "P%02d" % t["slot"]
        x0 = (240 - (text_w(AF_L, s) + 5 + 15)) // 2
        tx(cv, "toast title", AF_L, s, T.INK, T.SEL, x0, big_y, "l")
        art(cv, x0 + text_w(AF_L, s) + 5, big_y, QMARK, T.INK, 3)
        tx(cv, "toast detail", AF_M, "SAVE = SAVE HERE", T.INK, T.SEL, 0, small_y, "in", 240)
        cv.rect(40, 58, 160, 6, T.MID)
        cv.rect(40, 58, int(160 * t["frac"]), 6, T.INK)
    elif k == "dice":
        art(cv, 20, 17, DIE, T.INK, 4)
        tx(cv, "toast title", AF_L, t["layer"], T.INK, T.SEL, 66, big_y, "in", 160)
        tx(cv, "toast detail", AF_M, t["detail"], T.INK, T.SEL, 66, small_y, "in", 160)
    elif k == "undo":
        art(cv, 18, 19, UNDO_ARROW, T.INK, 4)
        tx(cv, "toast title", AF_L, "UNDO", T.INK, T.SEL, 66, big_y, "in", 160)
        tx(cv, "toast detail", AF_M, t["detail"], T.INK, T.SEL, 66, small_y, "in", 160)
    cv.mark("toast block (replaces Z2)", 0, 0, 240, 70)


def draw_toast_full(cv, S, ch):
    """v3's placement for comparison: replaces Z3+Z4 (square corners)."""
    t = S["toast"]
    cv.rect(0, 2, 240, 88, T.SEL)
    k = t["kind"]
    big_y, small_y = 2 + 16, 2 + 50
    if k == "algo":
        a = t["algo"]
        tx(cv, "toast title", AF_L, ALGO_FULL[a], T.INK, T.SEL, 0, big_y, "in", 240)
        tx(cv, "toast detail", AF_M, "CH %d ALGORITHM" % (ch + 1), T.INK, T.SEL, 0, small_y, "in", 240)
        n, pw, g = 6, 20, 6
        x0 = (240 - (n * pw + (n - 1) * g)) // 2
        for j in range(n):
            if j == a:
                cv.rect(x0 + j * (pw + g), 68, pw, 14, T.INK)
            else:
                cv.rect(x0 + j * (pw + g), 76, pw, 6, T.MID)
    cv.mark("toast full (replaces Z3+Z4)", 0, 2, 240, 88)


# ---------------------------------------------------------------------------------------------------------------- Z0 head
def playhead(S, ch):
    c = S["channels"][ch]
    n = c["len"] or 1
    return 255 if not S["playing"] else (c["p"] + n - 1) % n


def draw_z0(cv, S, ch):
    head, acc, red = pal(S)
    cv.begin(W, BLIT_A_H, T.BG)
    t = S.get("toast")
    if t and t.get("place") == "header":
        draw_banner(cv, S, ch, t, "header")
        return
    c = S["channels"][ch]
    ph, ln = playhead(S, ch), c["len"]
    # transport: a drawn 15x15 shape (play triangle / stop square / REC disc), left edge on the cell grid (x = 1)
    if S.get("rec"):
        disc(cv, 8, 11, 7, red)
    elif S["playing"]:
        tri_right(cv, 1, 4, 15, 15, head)
    else:
        cv.rect(1, 4, 15, 15, T.MID)
    bpm = ("EXT %d" % S["bpm"]) if S.get("ext") else str(S["bpm"])
    tx(cv, "bpm", AF_M, bpm, T.TEXT, T.BG, 22, 6, "il")
    pre = "P%02d%s" % (S["preset"], "?" if S["preset_pend"] else "")
    tx(cv, "preset", AF_M, pre, T.TEXT, T.BG, 121, 6, "il")                      # starts on the left edge of cell 3
    # page boxes: only pages that exist; viewed = solid, others hollow; the playing page carries a 4x4 marker. Right edge on x = 239.
    pages = (ln + 15) // 16
    for k in range(pages):
        x = 177 + 16 * k
        viewed = k == S["vpage"]
        playing = ph != 255 and ph // 16 == k
        if viewed:
            cv.rect(x, 5, 14, 14, T.TEXT)
        else:
            cv.frame(x, 5, 14, 14, T.MID)
            cv.frame(x + 1, 6, 12, 12, T.MID)
        if playing:
            cv.rect(x + 5, 10, 4, 4, T.BG if viewed else head)
    cv.mark("Z0 head", 0, 0, 240, 24)
    cv.mark("transport", 1, 4, 15, 15)
    cv.mark("page boxes", 177, 5, 62, 14)


# ---------------------------------------------------------------------------------------------------------------- Z1 channels
def draw_z1(cv, S):
    head, acc, red = pal(S)
    for i in range(4):
        c = S["channels"][i]
        x = CELL_PITCH * i + MARGIN
        y = 2 + 24                                                          # canvas y of the cell (blit A is y 0..72; Z1 starts at 24)
        sel, mut = i == S["sel"], c["muted"]
        off = c["midi_ch"] >= 16
        if sel:
            bg, ink = T.SEL, T.INK
            cv.rect(x, y, CELL_W, 44, bg)
        elif mut:
            bg, ink = T.BG, T.MID
            cv.frame(x, y, CELL_W, 44, T.MID)
            cv.frame(x + 1, y + 1, CELL_W - 2, 42, T.MID)
        else:
            bg, ink = T.RAISE, T.TEXT
            cv.rect(x, y, CELL_W, 44, bg)
        algo = c["algo"] % 6
        if sel and S.get("selL"):                                            # item 9 experiment: AF_L 3-letter name, number demoted to AF_M
            tx(cv, "chan number (M)", AF_M, str(i + 1), ink, bg, x + 4, y + 4, "il")
            tx(cv, "algo name (L)", AF_L, ALGO_L3[algo], ink, bg, x + 3, y + 19, "il")
            if mut:
                cv.rect(x + 3, y + 29, 52, 3, T.INK)
            ms = y + 3
            if off:
                ring_slash(cv, x + 24, ms, ink)
            if mut:
                cv.rect(x + 40, ms, 14, 14, T.INK)
                tx(cv, "MUTE chip", AF_M, "M", T.SEL, T.INK, x + 40, ms + 2, "in", 14)
            elif c["snd0"] and not off:
                cv.rect(x + 40, ms, 14, 14, T.INK)
            cv.mark("chan %d" % (i + 1), CELL_PITCH * i, 24, 60, 48)
            continue
        tx(cv, "chan number", AF_L, str(i + 1), ink, bg, x + 4, y + 4, "il")
        tx(cv, "algo name", AF_M, ALGO_NAMES[algo], ink, bg, x + 4, y + 28, "il")   # same left edge as the digit block: ink at x + 4/5
        if mut:                                                              # struck-through number: survives colour blindness
            cv.rect(x + 4, y + 13, 18, 3, T.INK if sel else T.MID)
        if off:                                                              # MIDI OFF: ring with a slash, left of the mute slot
            ring_slash(cv, x + 24, y + 4, ink)
        if mut:                                                              # MUTE: solid chip with M in the right-most slot (x + 40)
            chip = T.INK if sel else T.MID
            cv.rect(x + 40, y + 4, 14, 14, chip)
            tx(cv, "MUTE chip", AF_M, "M", T.SEL if sel else T.INK, chip, x + 40, y + 6, "in", 14)
        elif c["snd0"] and not off:                                          # just sounded: solid square, same slot
            cv.rect(x + 40, y + 4, 14, 14, T.INK if sel else head)
        cv.mark("chan %d" % (i + 1), CELL_PITCH * i, 24, 60, 48)


# ---------------------------------------------------------------------------------------------------------------- Z2 lane
def draw_z2(cv, S, ch):
    head, acc, red = pal(S)
    cv.begin(W, BLIT_B_H, T.BG)
    t = S.get("toast")
    if t and t.get("place") == "lane":
        draw_banner(cv, S, ch, t, "lane")
        return
    c = S["channels"][ch]
    ln = c["len"]
    ph = playhead(S, ch)
    mut = c["muted"]
    for n in range(16):
        L = S["lane"][n]
        x = LANE_PITCH * n + 1
        p = S["vpage"] * 16 + n
        if p >= ln:
            continue
        playing = ph != 255 and p == ph
        col = head if playing else T.LANE
        cv.rect(x, 0, LANE_COL, 70, col)                                     # the whole column: the playhead is a full-height bright column
        fg = T.BG if playing else (T.MID if mut else T.TEXT)
        accfg = T.BG if playing else (T.MID if mut else acc)
        if L.get("ed"):                                                      # EDITED (hand-entered) step: 5x5 diamond at the top of its column
            diamond(cv, x + 4, 0, T.BG if playing else (T.MID if mut else T.TEXT))
        if L["on"]:
            hh = 4 + L["vel"] * 38 // 127
            top = 62 - hh
            n_r = L["rep"] if L["rep"] > 1 else 1
            barc = accfg if L["acc"] else fg
            if n_r == 1:
                segs = [(x + 1, 11)]
            else:
                w = {2: 4, 3: 3}.get(n_r, 2)
                gaps = [round(k * (11 - w) / float(n_r - 1)) for k in range(n_r)]
                segs = [(x + 1 + g, w) for g in gaps]
            for sx, sw in segs:
                if mut and not playing:
                    cv.frame(sx, top, sw, hh, barc) if sw >= 3 else cv.rect(sx, top, sw, hh, barc)
                else:
                    cv.rect(sx, top, sw, hh, barc)
            if L["acc"]:                                                     # accent: triangle 11x6 in mark row A (y 7..12)
                tri_up(cv, x + 1, 7, 11, 6, accfg)
            if L["slide"]:                                                   # slide: a 15x6 tie cap in mark row B (y 14..19); runs of slides chain into a line
                cv.rect(x, 14, 15 if n < 15 else 13, 6, T.BG if playing else (T.MID if mut else T.TEXT))
        else:
            cv.rect(x + 1, 58, 11, 4, T.BG if playing else T.MID)           # a rest: low flat dash
        if n % 4 == 0:                                                       # beat pips (shape): a 5x3 bar under steps 1, 5, 9, 13, centred in the column
            cv.rect(x + 4, 65, 5, 3, T.BG if playing else T.MID)
    cv.mark("Z2 lane", 0, 0, 240, 72)


# ---------------------------------------------------------------------------------------------------------------- Z3 knobs
def draw_row_bar(cv, y, row, head):
    """ROW 1 / ROW 2 latch: two 118 px halves under the cell pairs (left = OCT-, right = OCT+); the live half is solid with its label in INK."""
    for h in range(2):
        x = 1 + 120 * h
        live = row == h
        if live:
            cv.rect(x, y, 118, ROWBAR_H, T.TEXT)
        else:
            cv.frame(x, y, 118, ROWBAR_H, T.MID)
        tx(cv, "row label", AF_M, "ROW %d" % (h + 1), T.INK if live else T.MID, T.TEXT if live else T.BG, x, y + 3, "in", 118)
    cv.mark("ROW bar", 1, y, 238, ROWBAR_H)


def draw_meter(cv, x, y, meter, fg, track):
    """48x8 meter at x..x+48: a fill bar, or pips (the chosen one tall)."""
    if meter[0] == "bar":
        cv.rect(x, y, 48, 8, track)
        fw = int(round(48 * meter[1]))
        if meter[1] > 0 and fw < 2:
            fw = 2
        cv.rect(x, y, fw, 8, fg)
    else:
        n, i = meter[1], meter[2]
        g = 1 if n > 8 else 2
        pw = (48 - (n - 1) * g) // n
        tot = n * pw + (n - 1) * g
        x0 = x + (48 - tot) // 2
        for k in range(n):
            if k == i:
                cv.rect(x0 + k * (pw + g), y, pw, 8, fg)
            else:
                cv.rect(x0 + k * (pw + g), y + 4, pw, 4, T.MID)


def draw_dots(cv, x, y, pts, fg, join=True):
    """Dot plot of (col, level) points, scaled to fill a 42 x 20 px box (x pitch at most 9 px): 4x4 dots joined by a 2 px line in play order.
    (x, y) = top-left of the 46 x 24 box the plot is centred in."""
    maxc = max(c_ for c_, l_ in pts) or 1
    maxl = max(l_ for c_, l_ in pts) or 1
    px = min(9.0, 42.0 / maxc)
    py = 20.0 / maxl
    wd = int(round(px * maxc))
    cs = [(x + 2 + (42 - wd) // 2 + int(round(px * c_)), y + 2 + int(round(py * (maxl - l_)))) for c_, l_ in pts]
    if join:
        for a, b in zip(cs, cs[1:]):
            cv.line_t(a[0], a[1] - 1, b[0], b[1] - 1, fg, 2)
    for cx, cy in cs:
        cv.rect(cx - 2, cy - 2, 4, 4, fg)


def pic_size(pic):
    kind, en, v, name = pic
    return (31, 31) if kind == "path" else (43, 23)


def draw_path(cv, x, y, n, fg, dim):
    """The 4x4 route of SNAKE path n (1..14) in a 31x31 box at (x, y): nodes on a 9 px pitch; contiguous steps (orthogonal neighbours) are 3 px strokes,
    jumps a 1 px dim line; step 1 is a solid 5x5 square, step 16 a hollow 5x5. The route is read from snake/proto.c's SHAPE table."""
    route = snake_route(n)
    P = 9

    def ctr(rc):
        return x + 2 + P * rc[1], y + 2 + P * rc[0]
    for a, b in zip(route, route[1:]):
        (ax, ay), (bx, by) = ctr(a), ctr(b)
        if abs(a[0] - b[0]) + abs(a[1] - b[1]) == 1:
            cv.rect(min(ax, bx) - 1, min(ay, by) - 1, abs(ax - bx) + 3, abs(ay - by) + 3, fg)
        else:
            cv.line(ax, ay, bx, by, dim)
    for rc in route:
        cx, cy = ctr(rc)
        cv.rect(cx - 1, cy - 1, 3, 3, fg)
    sx, sy = ctr(route[0])
    cv.rect(sx - 2, sy - 2, 5, 5, fg)
    ex, ey = ctr(route[-1])
    cv.rect(ex - 2, ey - 2, 5, 5, fg)
    cv.rect(ex - 1, ey - 1, 3, 3, T.SURF)


def draw_curve(cv, x, y, i, fg):
    pts = curve_pts(i)
    for a, b in zip(pts, pts[1:]):
        cv.line_t(x + a[0], y + a[1], x + b[0], y + b[1], fg, 2)


def draw_knob_cell(cv, x, y0, label, val, f, meter, pic, hot):
    bg = T.SURF
    cv.rect(x, y0, CELL_W, KNOB_H, bg)
    if hot:
        cv.frame(x, y0, CELL_W, KNOB_H, T.TEXT)
        cv.frame(x + 1, y0 + 1, CELL_W - 2, KNOB_H - 2, T.TEXT)
    tx(cv, "knob label", AF_M, label, T.TEXT, bg, x, y0 + 5, "in", CELL_W)
    if pic:
        kind, en, v, name = pic
        if kind == "path":                                                   # route picture (33x33) left, path number AF_M right
            draw_path(cv, x + 4, y0 + 19, v + 1, T.TEXT, T.MID)
            tx(cv, "knob value (M)", AF_M, name, T.TEXT, bg, x + CELL_W - 5, y0 + 31, "ir")
        else:                                                                # picture 43x23 + the firmware name in AF_M
            if en == "CURVE":
                draw_curve(cv, x + 7, y0 + 18, v, T.TEXT)
            else:
                draw_dots(cv, x + 6, y0 + 17, PICS[en][v], T.TEXT, join=not (en == "ARPDIR" and v == 5))
            tx(cv, "knob value (M)", AF_M, name, T.TEXT, bg, x, y0 + 43, "in", CELL_W)
    else:
        tx(cv, "knob value", AF_L, val, T.TEXT, bg, x, y0 + 19, "in", CELL_W)
        draw_meter(cv, x + 5, y0 + 45, meter, T.TEXT, T.LANE)


def draw_z3(cv, S, ch, y0=0, bar=True):
    head, acc, red = pal(S)
    c = S["channels"][ch]
    algo = c["algo"] % 6
    for k in range(4):
        x = CELL_PITCH * k + MARGIN
        idx = S["row"] * 4 + k
        label, val, f, meter, pic = knob_view(algo, idx, S["knobs"][idx])
        draw_knob_cell(cv, x, y0, label, val, f, meter, pic, S["hot"][k])
        cv.mark("knob %d" % (k + 1), CELL_PITCH * k, y0, 60, KNOB_H)
    if bar:
        draw_row_bar(cv, y0 + ROWBAR_Y, S["row"], head)


def draw_z4(cv, S, y):
    if S["playing"] or S.get("rec"):
        return                                                               # nothing while playing: the cue only earns its place stopped
    tx(cv, "cue", AF_M, "HOLD HOME = MAP", T.TEXT, T.BG, 0, y + 3, "in", 240)
    cv.mark("Z4 cue", 0, y, 240, 16)


# ---------------------------------------------------------------------------------------------------------------- MAP (HOME held)
BTN_ROWS = (("FX", "ALL", ""), ("SCL", "NOTES", ""), ("ENV", "ACCNT", ""), ("LFO", "SHAPE", ""), ("EDIT", "UNDO", "RESET"), ("GLO", "SET", "HARM"),
            ("HOME", "HOME", "MENU"), ("SAVE", "SAVE", "BACK"), ("ARP", "ARTIC", ""), ("SEQ", "HITS", ""), ("PLAY", "PLAY", ""), ("REC", "REC", ""))
BTN_CTL = ("B_FX", "B_SCL", "B_ENV", "B_LFO", "B_EDIT", "B_GLO", "B_HOME", "B_SAVE", "B_ARP", "B_SEQ", "B_PLAY", "B_REC")
BLACK = (("M1", None), ("M2", None), ("M3", None), ("M4", None), ("CHN", None), (None, "SOL"), (None, "FIL"), (None, "ROL"),
         ("PG-", None), ("PG+", None), ("RST", None))
BLACK_X = (0, 21, 42, 65, 86, 109, 130, 151, 174, 195, 218)               # SCREEN_SPEC 2.6 LEG_X: 3-2-3-2-1 like the keybed
MAP_IDLE = "TOUCH A CONTROL TO SEE WHAT IT DOES"                     # the header while nothing is touched
MENU_HOLD_MS, MENU_COUNT_MS = 1500, 1000                                  # map at 0 ms; countdown shown from 1.5 s, menu opens 1.0 s later


def help_line(ctl, ctx="PERF"):
    for ln in open(os.path.join(REPO, "docs", "design", "help_text.tsv")).read().splitlines()[1:]:
        c = ln.split("\t")
        if c[0] == ctl and c[1] == ctx:
            return c[2]
    return ""


def boxed(cv, role, x, y, w, label, fg, bg, h=13):
    """HOLD role = the word inside an outline box (one device for buttons and black keys)."""
    cv.frame(x, y, w, h, T.MID)
    tx(cv, role, AF_S, label, fg, bg, x, y + (h - 9) // 2 + 0, "in", w)


def draw_map(cv, S):
    """Strips: A y0..85 (header, encoders 2x2, OCT, K1..K4, keys), B y85..153 (2x6 buttons), C y153..233 (black keys, caption, white keys, LANE KEY).
    Every text is AF_S (the MAP is a page you bring your face to); roles are the CURRENT ones of the selected channel (ROW state, algorithm)."""
    ch = S["sel"]
    algo = S["channels"][ch]["algo"] % 6
    touch = S.get("touch")
    cv.begin(W, 85, T.BG)
    # header: idle text, or the help line of the touched control, or the MENU countdown (bar + the word)
    cd = S.get("countdown")
    if cd is not None:
        tx(cv, "map menu word", AF_M, "MENU", T.TEXT, T.BG, 1, 1, "il")
        bx, bw = 52, 187
        cv.frame(bx, 2, bw, 9, T.TEXT)
        cv.rect(bx + 2, 4, int((bw - 4) * cd), 5, T.TEXT)
    elif touch:
        tx(cv, "map help line", AF_S, touch[1], T.TEXT, T.BG, 0, 3, "in", 240)
    else:
        tx(cv, "map title", AF_S, MAP_IDLE, T.TEXT, T.BG, 0, 3, "in", 240)
    cv.rect(0, 13, 240, 1, T.LINE)
    enc = (("MASTER", "VOLUME", "ENC_MASTER"), ("SELECT", "CHANNEL", "ENC_SEL"), ("PRESETS", "P01-P16", "ENC_PRE"),
           ("ALGORITHM", "OF CH %d" % (ch + 1), "ENC_ALGO"))
    for i, (nm, role, ctl) in enumerate(enc):                                # the panel's 2x2 encoder block
        x, y = 66 * (i % 2), 15 + 23 * (i // 2)
        hot = touch and touch[0] == ctl
        bg = T.SEL if hot else T.SURF
        cv.rect(x, y, 65, 22, bg)
        tx(cv, "map encoder name", AF_S, nm, T.INK if hot else T.MID, bg, x, y + 2, "in", 65)
        tx(cv, "map encoder role", AF_S, role, T.INK if hot else T.TEXT, bg, x, y + 12, "in", 65)
    for i, nm in enumerate(("OCT-", "OCT+")):                                # OCT- OCT+ under the encoders; the live row is the inverted one
        x, y = 66 * i, 62
        live = S["row"] == i
        bg = T.SEL if live else T.SURF
        cv.rect(x, y, 65, 22, bg)
        tx(cv, "map OCT name", AF_S, nm, T.INK if live else T.MID, bg, x, y + 2, "in", 65)
        tx(cv, "map OCT role", AF_S, "ROW %d" % (i + 1), T.INK if live else T.TEXT, bg, x, y + 12, "in", 65)
    for k in range(4):                                                       # K1..K4: ROW 1 name, ROW 2 name; the live row is bright
        y = 15 + 11 * k
        tx(cv, "map knob id", AF_S, "K%d" % (k + 1), T.MID, T.BG, 134, y + 1, "l")
        tx(cv, "map knob row1", AF_S, KNOBS[algo][k][0], T.TEXT if S["row"] == 0 else T.MID, T.BG, 134 + 17, y + 1, "l")
        tx(cv, "map knob row2", AF_S, KNOBS[algo][4 + k][0], T.TEXT if S["row"] == 1 else T.MID, T.BG, 134 + 62, y + 1, "l")
    # two key rows at the right of the OCT tiles: HOLD box, and the channel-cell marks
    cv.frame(134, 62, 12, 11, T.MID)
    tx(cv, "map legend", AF_S, "= HOLD", T.TEXT, T.BG, 150, 63, "l")
    cv.rect(134, 74, 11, 11, T.MID)
    tx(cv, "map key word", AF_S, "M", T.INK, T.MID, 134, 75, "in", 11)
    tx(cv, "map key word", AF_S, "MUTE", T.TEXT, T.BG, 148, 75, "l")
    ring_slash(cv, 190, 73, T.TEXT, 12)                                      # 12 px here (14 px on the channel cell)
    tx(cv, "map key word", AF_S, "OFF", T.TEXT, T.BG, 206, 75, "l")
    cv.mark("A header", 0, 0, 240, 13)
    cv.mark("A encoders 2x2", 0, 15, 131, 45)
    cv.mark("A OCT- OCT+", 0, 62, 131, 22)
    cv.mark("A K1..K4", 133, 15, 107, 44)
    cv.blit(0, 0)
    cv.begin(W, 71, T.BG)                                                    # strip B: the 2x6 buttons, row 1 FX..GLO, row 2 HOME..REC
    cv.rect(0, 0, 240, 1, T.LINE)
    for i, (printed, tap, hold) in enumerate(BTN_ROWS):
        x, y = 40 * (i % 6), 1 + 35 * (i // 6)
        hot = touch and touch[0] == BTN_CTL[i]
        bg = T.SEL if hot else T.SURF
        cv.rect(x + 1, y, 38, 34, bg)
        tx(cv, "map button printed", AF_S, printed, T.INK if hot else T.MID, bg, x + 1, y + 2, "in", 38)
        tx(cv, "map button tap", AF_S, tap, T.INK if hot else T.TEXT, bg, x + 1, y + 12, "in", 38)
        if hold:
            boxed(cv, "map button hold", x + 2, y + 21, 36, hold, T.INK if hot else T.TEXT, bg, 13)
    cv.mark("B buttons 2x6", 0, 0, 240, 71)
    cv.blit(0, 85)
    cv.begin(W, 78, T.BG)                                                    # strip C: black keys (two tiers + leader lines), caption, white keys, LANE KEY
    cv.rect(0, 0, 240, 1, T.LINE)
    for i, (tap, hold) in enumerate(BLACK):
        x = BLACK_X[i]
        cx = x + 10
        cv.rect(x + 2, 2, 16, 7, T.KEY if tap else T.RAISE)
        ty = 10 if i % 2 == 0 else 23
        cv.rect(cx, 9, 1, ty - 9 + 1, T.MID)
        lab = tap or hold
        w = text_w(AF_S, lab) + (4 if hold else 0)
        lx = clamp(cx - w // 2, 0, 240 - w)
        if hold:
            boxed(cv, "map black key hold", lx, ty, w, lab, T.TEXT, T.BG, 12)
        else:
            tx(cv, "map black key", AF_S, lab, T.TEXT, T.BG, lx, ty + 2, "l")
    cv.mark("C black keys (2 tiers)", 0, 1, 240, 35)
    tx(cv, "map caption", AF_S, "STEPS 1-16: TAP ON/OFF  HOLD EDIT", T.TEXT, T.BG, 0, 38, "in", 240)
    cv.mark("C caption", 0, 36, 240, 12)
    for n in range(16):
        x = 15 * n + 1
        cv.rect(x, 48, 13, 13, T.RAISE)
        tx(cv, "map white key", AF_S, str(n + 1), T.TEXT, T.RAISE, x, 50, "in", 13)
    cv.mark("C white keys", 0, 48, 240, 13)
    draw_lane_key(cv, S, 63)
    cv.blit(0, 156)


def draw_lane_key(cv, S, y):
    """LANE KEY: the real marks drawn at their real size with one word each (AF_S): accent triangle, slide cap, ratchet comb, edited diamond."""
    head, acc, red = pal(S)
    items = (("tri", "ACCENT", 11), ("cap", "SLIDE", 15), ("comb", "RATCH", 11), ("dia", "EDITED", 5))
    widths = [it[2] + 4 + (text_ink(AF_S, it[1])[2] - text_ink(AF_S, it[1])[0]) for it in items]
    gap = (238 - sum(widths)) // (len(items) - 1)
    x = 1
    cv.rect(0, y, 240, 14, T.RAISE)
    cy = y + 4
    for (kind, word, sw), wd in zip(items, widths):
        if kind == "tri":
            tri_up(cv, x, cy + 0, 11, 6, acc)
        elif kind == "cap":
            cv.rect(x, cy, 15, 6, T.TEXT)
        elif kind == "comb":
            for g in (0, 4, 8):
                cv.rect(x + g, cy - 1, 3, 8, T.TEXT)
        else:
            diamond(cv, x, cy, T.TEXT)
        tx(cv, "map key word", AF_S, word, T.TEXT, T.RAISE, x + sw + 4, y + 3, "il")
        x += wd + gap
    cv.mark("LANE KEY", 0, y, 240, 14)


# ---------------------------------------------------------------------------------------------------------------- gallery (verification sheets)
def draw_gallery(cv, S):
    kind = S["gallery"]
    cells = []
    if kind == "path":
        for v in range(14):
            cells.append(("PATH", "PATH", v))
    elif kind == "arp":
        for v in range(6):
            cells.append(("SHAPE", "SHAPE", v))
        for v in range(6):
            cells.append(("DIR", "ARPDIR", v))
        for v in range(3):
            cells.append(("DIR", "DIR3", v))
    else:
        for v in range(7):
            cells.append(("CURVE", "CURVE", v))
    for strip in range(2):                                                   # two 240x120 strips (CV_MAX): 2 rows of cells each
        cv.begin(W, 120, T.BG)
        for i, (label, en, v) in enumerate(cells):
            if i // 8 != strip:
                continue
            x, y = 1 + 60 * (i % 4), 60 * ((i % 8) // 4)
            kd, names = ENUMS[en]
            pic = ("path", en, v, str(v + 1)) if kd == "path" else ("pic", en, v, names[v])
            draw_knob_cell(cv, x, y, label, "", AF_M, None, pic, False)
        cv.blit(0, 120 * strip)


# ---------------------------------------------------------------------------------------------------------------- draw
def draw(cv, S):
    CUR[0] = S.get("_name", "")
    mock.palette_set(S.get("palette", "GREY"))
    if S.get("gallery"):
        draw_gallery(cv, S)
        return
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
    t = S.get("toast")
    if t and t.get("place") == "full":
        draw_toast_full(cv, S, ch)
    else:
        draw_z3(cv, S, ch, bar=not (t and t.get("place") == "bottom"))
        if t and t.get("place") == "bottom":
            draw_banner(cv, S, ch, t, "bottom")
        else:
            draw_z4(cv, S, Z4_Y - Z3_Y)
    cv.mark("Z5 margin", 0, 92, 240, 4)
    cv.blit(0, Z3_Y)


# ---------------------------------------------------------------------------------------------------------------- fixtures
def base(name, **kw):
    s = copy.deepcopy(fixture(name))
    s.update(kw)
    return s


ACID_K = (12, 6, 5, 3, 0, 4, 40, 0)
EUC_K = (5, 0, 4, 2, 16, 12, 40, 0)
SNAKE_K = (10, 7, 12, 6, 0, 16, 10, 0)                      # PATH index 6 = path 7, the default (row serpentine)


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
    ch[3].update(algo=3, midi_ch=16, muted=1, len=16, p=7)                  # muted AND OFF, not selected
    return s


STRESS_STEPS = [
    (1, 30, 0, 0, 1, 0), (1, 55, 0, 0, 1, 1), (0, 1, 0, 0, 1, 0), (1, 100, 1, 0, 1, 0),                      # 49..52: rest, accent
    (1, 80, 0, 1, 1, 0), (1, 90, 0, 0, 2, 0), (1, 127, 1, 1, 1, 1), (1, 110, 1, 1, 3, 0),                    # 53..56: slide, ratchet x2, accent+slide (edited), accent+slide+ratchet x3
    (1, 70, 0, 0, 4, 0), (1, 40, 1, 0, 1, 0), (0, 1, 0, 0, 1, 0), (1, 60, 0, 1, 1, 1),
    (1, 90, 0, 1, 1, 0), (1, 20, 0, 0, 1, 0), (1, 120, 1, 0, 2, 0), (0, 1, 0, 0, 1, 0)]


def scene_stress():
    """The worst case: ARP selected AND muted AND MIDI OFF, EXT 126, P16, L64 page 4/4, enumerated knobs (SHAPE, RATE), REC, a hot knob."""
    s = base("perf_snake_mute", knobs=[8, 3, 3, 3, 15, 8, 10, 4], sel=3, vpage=3, playing=1, rec=1, ext=1, bpm=126, preset=16,
             hot=[0, 1, 0, 0])
    ch = s["channels"]
    ch[0].update(algo=0, midi_ch=15, muted=0, len=16, snd0=1, p=3)
    ch[1].update(algo=5, midi_ch=1, muted=0, len=32, snd0=0, p=2, pg_used=3)
    ch[2].update(algo=2, midi_ch=16, muted=0, len=16, p=2)                  # OFF only
    ch[3].update(algo=4, midi_ch=16, muted=1, len=64, p=56, pg_used=15)     # ARP: selected AND muted AND OFF, page 4/4, playhead on step 56 (index 7)
    s["lane"] = [dict(on=a, vel=b, ed=ed, acc=c, slide=d, rep=e) for (a, b, c, d, e, ed) in STRESS_STEPS]
    return s


def scene_stress2():
    s = scene_stress()
    s["row"] = 1
    s["hot"] = [0, 0, 0, 0]
    s["knobs"] = [8, 3, 3, 3, 15, 8, 10, 5]                                  # ROW 2 of ARP: OFFSET 15, LEGATO 8, DYN 10, DIR = CHORD
    return s


def toast_scene(kind, place="bottom", **kw):
    s = scene_playing()
    s["toast"] = dict(kind=kind, place=place, **kw)
    return s


def scene_toast_algo(place="bottom"):
    s = toast_scene("algo", place, algo=1)
    s["channels"][0]["algo"] = 1
    s["knobs"] = list(EUC_K)
    return s


def scene_toast_preset(place="bottom"):
    s = toast_scene("preset", place, slot=4, frac=0.55)
    s["preset"], s["preset_pend"] = 4, 1
    return s


def scene_toast_dice(place="bottom"):
    return toast_scene("dice", place, layer="NOTES", detail="CH 1  NEW PITCHES")


def scene_toast_undo(place="bottom"):
    return toast_scene("undo", place, detail="DICE NOTES")


def scene_map(**kw):
    s = scene_playing()
    s["map"] = True
    s["toast"] = None
    s.update(kw)
    return s


def scene_snake():
    s = scene_mutes()
    s["sel"] = 2
    s["channels"][3]["muted"] = 0
    s["channels"][3]["midi_ch"] = 3
    s["knobs"] = list(SNAKE_K)
    s["hot"] = [0, 0, 0, 1]
    return s


def scenes():
    d = {
        "glance_idle": scene_idle(), "glance_playing": scene_playing(), "row2": scene_row2(), "mutes": scene_mutes(), "stress": scene_stress(),
        "stress_row2": scene_stress2(), "toast_algo": scene_toast_algo(), "toast_preset_pending": scene_toast_preset(),
        "toast_dice_notes": scene_toast_dice(), "toast_undo": scene_toast_undo(),
        "map": scene_map(), "map_menu_countdown": scene_map(countdown=0.6),
        "map_touch": scene_map(touch=("B_FX", help_line("B_FX"))),
    }
    # extra scenes: toast placements compared, snake path cell, the item-9 experiment, galleries, palettes
    d["toast_algo_header"] = scene_toast_algo("header")
    d["toast_preset_header"] = scene_toast_preset("header")
    d["toast_dice_header"] = scene_toast_dice("header")
    d["toast_algo_lane"] = scene_toast_algo("lane")
    d["toast_algo_full"] = scene_toast_algo("full")
    d["toast_preset_lane"] = scene_toast_preset("lane")
    d["toast_dice_lane"] = scene_toast_dice("lane")
    d["snake_path"] = scene_snake()
    d["stress_selL"] = dict(scene_stress(), selL=True)
    d["gallery_path"] = dict(scene_playing(), gallery="path")
    d["gallery_arp"] = dict(scene_playing(), gallery="arp")
    d["gallery_curve"] = dict(scene_playing(), gallery="curve")
    for pn in ("PAPER", "AMBER", "ICE", "GREEN", "VIOLET", "ROSE", "NIGHT", "MONO"):
        d["stress_pal_" + pn.lower()] = dict(scene_stress(), palette=pn)
    d["glance_playing_mono"] = dict(scene_playing(), palette="MONO")
    for n, s in d.items():
        s["_name"] = n
    return d


STATE = {"scenes": scenes()}
