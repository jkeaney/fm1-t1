# SPDX-License-Identifier: GPL-3.0-only
"""el.py: v5 element library (design session 2026-11-12, round 2). Every element is drawn with the mock tool's real
fonts and primitives, in screen coordinates, through a painter that splits the screen into firmware strips (<=124 px).
Grid: 16 columns of 15 px (= the 16 white keys, so step n sits over white key n), 4 quarters of 60 px, rows on 4 px."""
from mock import T, AF_S, AF_M, AF_L, text_w, rgb565

W = 240
CH_COL = (rgb565(255, 150, 40), rgb565(70, 190, 255), rgb565(150, 220, 80), rgb565(255, 110, 180))
INK = rgb565(0, 0, 0)
WHITE = rgb565(250, 250, 250)
PH = rgb565(70, 70, 70)
CAP = {}


def _cap(f):
    return {12: (3, 9), 15: (4, 11)}.get(f.h, (7, 21)) if False else CAP[f.name]


def init_caps():
    CAP[AF_S.name] = (3, 9)
    CAP[AF_M.name] = (4, 11)
    CAP[AF_L.name] = (7, 21)


init_caps()


def mix(c, k):
    """c scaled to k/255 over black"""
    r, g, b = (c >> 11) & 31, (c >> 5) & 63, c & 31
    return ((r * k // 255) << 11) | ((g * k // 255) << 5) | (b * k // 255)


def shade(ci, pc):
    """12 shades of the channel colour: pitch class 0 (C) darkest .. 11 (B) brightest"""
    return mix(CH_COL[ci], 120 + pc * 12)


class P:
    def __init__(self, cv, y0, y1):
        self.cv, self.y0, self.y1 = cv, y0, y1

    def pset(self, x, y, c):
        if self.y0 <= y < self.y1 and 0 <= x < W:
            self.cv.pset(x, y - self.y0, c)

    def rect(self, x, y, w, h, c):
        self.cv.rect(x, y - self.y0, w, h, c)

    def frame(self, x, y, w, h, c):
        self.cv.frame(x, y - self.y0, w, h, c)

    def line(self, x0, y0, x1, y1, c):
        self.cv.line(x0, y0 - self.y0, x1, y1 - self.y0, c)

    def text(self, x, ycap, f, s, fg, bg):
        top, ch = CAP[f.name]
        if ycap + ch <= self.y0 or ycap >= self.y1:
            return x + text_w(f, s)
        self.cv.text_on(x, ycap - top - self.y0, f, s, fg, bg)
        return x + text_w(f, s)

    def text_c(self, x0, w, ycap, f, s, fg, bg):
        self.text(x0 + (w - text_w(f, s)) // 2, ycap, f, s, fg, bg)

    def text_r(self, xr, ycap, f, s, fg, bg):
        self.text(xr - text_w(f, s), ycap, f, s, fg, bg)

    def art(self, x, y, a, c):
        for j, row in enumerate(a.strip("\n").split("\n")):
            for i, ch in enumerate(row):
                if ch == "#":
                    self.pset(x + i, y + j, c)

    def hatch(self, x, y, w, h, c, step=3):
        for j in range(h):
            for i in range(w):
                if (i + j) % step == 0:
                    self.pset(x + i, y + j, c)

    def dots(self, x, y, w, h, c):
        for j in range(h):
            for i in range(w):
                if (i + j) % 2 == 0:
                    self.pset(x + i, y + j, c)


def paint(cv, strips, fn):
    for (a, b) in strips:
        cv.begin(W, b - a, T.BG)
        fn(P(cv, a, b))
        cv.blit(0, a)


# ------------------------------------------------------------------ icons (pixel art, 11-13 px)
IC = {
"dice": """
###########
#.........#
#.##...##.#
#.##...##.#
#....#....#
#...###...#
#....#....#
#.##...##.#
#.##...##.#
#.........#
###########""",
"note": """
.....##....
.....###...
.....#.##..
.....#..#..
.....#.....
.....#.....
..####.....
.#####.....
.#####.....
..###......
...........""",
"accent": """
##.........
.###.......
...###.....
.....###...
.......###.
.....###...
...###.....
.###.......
##.........
...........
...........""",
"wave": """
...........
..##.......
.#..#......
#....#.....
#....#....#
......#...#
......#..#.
.......##..
...........
...........
...........""",
"artic": """
...........
..#######..
.#.......#.
#.........#
...........
.##.....##.
.##.....##.
...........
...........
...........
...........""",
"hits": """
...........
...........
...........
##..##..##.
##..##..##.
...........
...........
##......##.
##......##.
...........
...........""",
"undo": """
...#.......
..##.......
.#########.
..##......#
...#.......#
..........#
.........#.
....#####..
...........
...........
...........""",
"gear": """
....###....
.#.#####.#.
..#######..
.###...###.
####...####
####...####
.###...###.
..#######..
.#.#####.#.
....###....
...........""",
"home": """
.....#.....
....###....
...#####...
..#######..
.#########.
..#######..
..##...##..
..##...##..
..##...##..
..##...##..
...........""",
"save": """
.....#.....
.....#.....
.....#.....
...#####...
....###....
.....#.....
#.........#
#.........#
###########
...........
...........""",
"play": """
##.........
####.......
######.....
########...
##########.
########...
######.....
####.......
##.........
...........
...........""",
"rec": """
...#####...
..#######..
.#########.
.#########.
.#########.
.#########.
.#########.
..#######..
...#####...
...........
...........""",
"spk": """
....#......
...##......
####.#.....
#....#.....
#....#.....
####.#.....
...##......
....#......
...........""",
"spkx": """
....#......
...##......
####.#.#..#
#....#..##.
#....#..##.
####.#.#..#
...##......
....#......
...........""",
"left": """
....#
...##
..###
.####
..###
...##
....#""",
"right": """
#....
##...
###..
####.
###..
##...
#....""",
"rst": """
#...#
#..##
#.###
#####
#.###
#..##
#...#""",
"vol": """
.........#.
.......###.
.....#####.
...#######.
.#########.
...........""",
}

ALGO_IC = {
"ACID": """
#....#....#
##...##...#
#.#..#.#..#
#..#.#..#.#
#...##...##
#....#....#""",
"EUCLID": """
...###...
.#.....#.
#...#...#
#.......#
#.......#
.#.....#.
...#.#...""",
"SNAKE": """
#########
........#
#########
#........
#########""",
"STAGE": """
......###
......#..
...####..
...#.....
####.....""",
"ARP": """
....#....
...#.#...
..#...#..
.#.....#.
#.......#""",
"PATTERNS": """
##.##.##.
##.##.##.
.........
##....##.
##....##.""",
}

# the 12 buttons in panel order (row 1 FX SCL ENV LFO EDIT GLO, row 2 HOME SAVE ARP SEQ PLAY REC); roles from button_modes.tsv PERF
BTN = (("FX", "dice", "ALL", True), ("SCL", "note", "NOTES", True), ("ENV", "accent", "ACCNT", True),
       ("LFO", "wave", "SHAPE", True), ("EDIT", "undo", "UNDO", False), ("GLO", "gear", "SET", False),
       ("HOME", "home", "HOME", False), ("SAVE", "save", "SAVE", False), ("ARP", "artic", "ARTIC", True),
       ("SEQ", "hits", "HITS", True), ("PLAY", "play", "PLAY", False), ("REC", "rec", "REC", False))
DICE_BG = rgb565(48, 40, 16)       # the dice group's tint (shape cue: the dice icon in the corner too)

# black keys: x centre on the 15 px white-key pitch (3-2-3-2-1), role, glyph (key_map.tsv PERF)
BLACK = ((15, "M1"), (30, "M2"), (45, "M3"), (75, "M4"), (90, "CHN"), (120, "SOL"), (135, "FIL"), (150, "ROL"),
         (180, "PG-"), (195, "PG+"), (225, "RST"))


# ------------------------------------------------------------------ demo data
def L(s, acc=(), sl=(), diced=()):
    out = []
    for n, c in enumerate(s):
        out.append(None if c == "." else {"p": int(c, 36), "acc": n in acc, "sl": n in sl, "dice": n in diced})
    return out


CHANS = [
    {"algo": "ACID", "midi": 2, "lane": L("0.7.35.a3.07.5c.", acc=(0, 7, 13), sl=(4,)), "muted": False},
    {"algo": "EUCLID", "midi": 10, "lane": L("0..0..0...0..0..", acc=(0,)), "muted": False},
    {"algo": "SNAKE", "midi": 3, "lane": L("g.j.l.n.e..j.k.l"), "muted": True},
    {"algo": "STAGE", "midi": 4, "lane": L("7...b...e...b..."), "muted": False},
]
KN = {"ACID": [[("DENS", "12", 75), ("SLIDE", "4", 50), ("JUMP", "5", 25), ("ACCNT", "3", 37)],
               [("OFFSET", "0", 0), ("PEDAL", "6", 50), ("PUNCH", "40", 85), ("DIR", "FWD", 0)]],
      "EUCLID": [[("HITS", "5", 31), ("OFFSET", "0", 0), ("RATCH", "4", 50), ("ACCNT", "2", 25)]]}


def pr(p):
    return p % 12
