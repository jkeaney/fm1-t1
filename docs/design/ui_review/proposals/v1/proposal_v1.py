# SPDX-License-Identifier: GPL-3.0-only
"""proposal_v1.py: UI PROPOSAL v1 for the FM-1 PERF screen (a copy of mock/current_perf.py, zone by zone redrawn).
Same 240x240 tiling Z0..Z7 as SCREEN_SPEC 2, same fonts, same fixtures (+ long_names, undo_toast defined here).
   .venv/bin/python docs/design/ui_review/mock/mock.py render docs/design/ui_review/proposals/v1/proposal_v1.py --out docs/design/ui_review/proposals/v1
   .venv/bin/python docs/design/ui_review/proposals/v1/proposal_v1.py --report      # the measurement / contrast tables used in NOTES.md
"""
import os
import sys
import copy

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "..", "mock"))
from mock import AF_S, AF_M, T, tables, text_w, fixture, rgb565, contrast_ratio, text_fit_str   # noqa: E402

NAME = "proposal_v1"
PALETTE = "GREY"

SQ_PAGE, SQ_NCH, SQ_MIDI_OFF, SQ_NALGO = 16, 4, 16, 6
W = 240
Z0_Y, Z0_H, Z1_Y, Z1_H, Z2_Y, Z2_H, Z3_Y, Z3_H, Z4_Y, Z4_H, Z5_Y, Z5_H, Z6_Y, Z6_H = 0, 20, 20, 28, 48, 60, 108, 52, 160, 52, 212, 12, 224, 12
BLIT_A_H = Z0_H + Z1_H + Z2_H                    # 108
BLIT_B_H = Z3_H + Z4_H + 4                       # 108: 4 spare rows so no descender is ever clipped (blit C overwrites them)
BLIT_C_H = Z5_H + Z6_H + 4                       # 28
SHORT = ("ACID", "EUCL", "SNAK", "STAG", "ARP", "PATT")
LEG_X = (0, 21, 42, 65, 86, 109, 130, 151, 174, 195, 218)

TB = tables()
E = TB.enums
PG_PERF = E["UI_PG_PERF"]
MODE_TAP, MODE_HOLD = E["UI_MODE_TAP"], E["UI_MODE_HOLD"]
CTX_ANY = E["UI_CTX_ANY"]
LAYER_OF = {E["UI_C_B_SCL"]: 0, E["UI_C_B_SEQ"]: 1, E["UI_C_B_ENV"]: 2, E["UI_C_B_ARP"]: 3, E["UI_C_B_LFO"]: 4}   # NOTES HITS ACCNT ARTIC SHAPE


def hx(s):
    return rgb565(int(s[1:3], 16), int(s[3:5], 16), int(s[5:7], 16))


# --------------------------------------------------------------------------------------------------------- NEW colour tokens
# (everything else is an existing T_* token). One hue per dice layer + the playing-column tint. Mid-luminance, desaturated: no neon.
H_NOTES = hx("#e08aa8")      # rose
H_HITS = hx("#6fa8e6")       # blue
H_ACCNT = hx("#e8a840")      # amber
H_ARTIC = hx("#5cc08e")      # green
H_SHAPE = hx("#b190e8")      # violet
LAYER_HUE = (H_NOTES, H_HITS, H_ACCNT, H_ARTIC, H_SHAPE)
LAYER_NAME = ("NOTES", "HITS", "ACCNT", "ARTIC", "SHAPE")
PLAYCOL = hx("#4b5063")      # the playing column tint (LANE is #393839)
NEW_TOKENS = [("H_NOTES", H_NOTES), ("H_HITS", H_HITS), ("H_ACCNT", H_ACCNT), ("H_ARTIC", H_ARTIC), ("H_SHAPE", H_SHAPE), ("PLAYCOL", PLAYCOL)]

# --------------------------------------------------------------------------------------------------------- 1-bit glyphs
G_PLAY = """
#........
###......
#####....
#######..
#########
#######..
#####....
###......
#........
"""
G_STOP = "#########\n" * 9
G_REC = """
..#####..
.#######.
#########
#########
#########
#########
#########
.#######.
..#####..
"""
G_PLAY_S = """
#....
###..
#####
###..
#....
"""                                                          # 5x5 play triangle (button PLAY)
G_REC_S = """
.###.
#####
#####
#####
.###.
"""                                                          # 5x5 REC disc (button REC)
G_CHEV = """
##..##
.####.
..##..
"""                                                          # 6x3 hold chevron (the 6 px replacement of the 4 px triangle)
L_NOTES = ["..#..", ".###.", "#####", ".###.", "..#.."]      # 5x5 layer marks (buttons)
L_HITS = ["#....", "#...#", "#.#.#", "#.#.#", "#.#.#"]
L_ACCNT = ["..#..", ".###.", ".###.", "#####", "#####"]
L_ARTIC = ["..###", "...##", "..#.#", ".#...", "#...."]
L_SHAPE = [".#...", "#.#..", "..#.#", "...#."]
LAYER_ART = (L_NOTES, L_HITS, L_ACCNT, L_ARTIC, L_SHAPE)
M_ACC = ["...#...", "..###..", ".#####.", "#######"]          # lane marks: accent 7x4 triangle (5x5 "L_ACCNT" is the button version)
M_ACC5 = L_ACCNT                                               # compact accent (5x5) used when it shares the row
M_SLIDE = ["....###", ".....##", "....#.#", "...#...", "..#....", ".#....."]   # 7x6 rising arrow
G_UNDO = """
..###.
.#...#
##...#
.#..#.
..#.#.
..#.#.
"""                                                          # not used; kept simple: the toast uses G_UNDO2
G_UNDO2 = ["..#.....", ".##.....", "#######.", ".##....#", "..#....#", "....###."]
LG_SOLO = ["#########", "#.......#", "#.#####.#", "#.#.....#"[:9], "#.#####.#", "#.....#.#", "#.#####.#", "#.......#", "#########"]
LG_FILL = ["######", "##....", "##....", "#####.", "##....", "##....", "##....", "##....", "##...."]
LG_ROLL = ["#####.", "##..##", "##..##", "#####.", "####..", "##.##.", "##..##", "##..##", "##..##"]
LG_PGM = ["...##", "..##.", ".##..", "##...", ".##..", "..##.", "...##"]
LG_PGP = ["##...", ".##..", "..##.", "...##", "..##.", ".##..", "##..."]
LG_RST = ["##....#", "##...##", "##..###", "##.####", "##..###", "##...##", "##....#"]
LG_CHN = ["..###....", "#########", "..###....", ".........", ".....###.", "#########", ".....###."]
LG_SOLO = ["#########", "#.......#", "#..###..#", "#..#....#", "#..###..#", "#....#..#", "#..###..#", "#.......#", "#########"]   # S in a box (9x9)


def art(cv, x, y, a, c):
    cv.glyph1(x, y, a, c)
    rows = a.strip("\n").split("\n") if isinstance(a, str) else a
    return max(len(r) for r in rows), len(rows)


def glyph_w(a):
    rows = a.strip("\n").split("\n") if isinstance(a, str) else a
    return max(len(r) for r in rows)


def dot(cv, x, y, c):                                     # the 8x8 clock dot, drawn in its 'on' phase (as current_perf.dot)
    cv.rect(x + 1, y, 6, 8, c)
    cv.rect(x, y + 1, 8, 6, c)


# --------------------------------------------------------------------------------------------------------- state helpers
def playhead(S, ch):
    c = S["channels"][ch]
    n = c["len"] or 1
    return 255 if not S["playing"] else (c["p"] + n - 1) % n


def midi_txt(m):
    return "MOFF" if m >= SQ_MIDI_OFF else "M%d" % (m + 1)


# --------------------------------------------------------------------------------------------------------- Z0 HEAD
HX = dict(shape=2, dot=12, bpm=22, src=51, pre=75, algo=103, map=177, len=218)
H_ALGO_W = 73                                             # `4 STAG MOFF` 73 is the worst case in that slot


def tempo_parts(S):
    """(digits or None, source word) for the tempo group."""
    src = S.get("tempo_src", "INT")
    if src == "WAIT":
        return None, "WAIT"
    if src == "EXT --":
        return "--", "EXT"
    return str(S["bpm"]), src


def draw_z0(cv, S, ch):
    c = S["channels"][ch]
    ph, ln = playhead(S, ch), c["len"]
    if S.get("rec"):
        art(cv, HX["shape"], 7, G_REC, T.REC)
    elif S["playing"]:
        art(cv, HX["shape"], 7, G_PLAY, T.ACCENT)
    else:
        art(cv, HX["shape"], 7, G_STOP, T.TEXT)
    dot(cv, HX["dot"], 7, 0xFD80)
    digits, word = tempo_parts(S)
    if digits is None:
        cv.text(HX["bpm"], 4, AF_S, word, T.TEXT)
    else:
        cv.text(HX["bpm"], 1, AF_M, digits, T.TEXT)
        cv.text(HX["src"] if digits != "--" else HX["src"], 4, AF_S, word, T.MID)
    cv.text(HX["pre"], 4, AF_S, "P%s%d%s" % ("0" if S["preset"] < 10 else "", S["preset"], "?" if S["preset_pend"] else ""), T.TEXT)
    cv.text_fit(HX["algo"], 4, AF_S, "%d %s %s" % (ch + 1, SHORT[c["algo"] % SQ_NALGO], midi_txt(c["midi_ch"])), T.TEXT, T.BG, H_ALGO_W)
    for k in range(4):                                    # page map: unchanged from current_perf (graphic)
        x, y = HX["map"] + 10 * k, 4
        shown = k * SQ_PAGE < ln
        used = (c["pg_used"] >> k) & 1
        playing = ph != 255 and ph // SQ_PAGE == k
        d = chr(ord("1") + k)
        if shown:
            partial = k == (ln - 1) // SQ_PAGE and ln % SQ_PAGE != 0
            if playing:
                cv.rect(x, y, 9, 11, T.SEL)
                cv.text_in(x, y - 1, 9, AF_S, d, T.INK, T.SEL)
            else:
                cv.text_in(x, y - 1, 9, AF_S, d, T.MID if k != S["vpage"] else T.TEXT, T.BG)
            cv.frame(x, y, 9, 11, T.TEXT if (k == S["vpage"] or playing) else T.MID)
            if partial:
                for i in range(1, 8, 2):
                    cv.pset(x + i, y + 10, T.BG)
        elif used:
            for i in range(0, 9, 2):
                cv.pset(x + i, y, T.MID)
                cv.pset(x + i, y + 10, T.MID)
            for i in range(0, 11, 2):
                cv.pset(x, y + i, T.MID)
                cv.pset(x + 8, y + i, T.MID)
        else:
            cv.rect(x + 3, y + 4, 2, 2, T.MID)
    cv.text(HX["len"], 4, AF_S, "L%d" % ln, T.TEXT)


# --------------------------------------------------------------------------------------------------------- Z1 CHANNELS
def draw_z1(cv, S):
    for i in range(SQ_NCH):
        c = S["channels"][i]
        x, y = 60 * i, Z1_Y
        sel, mut = i == S["sel"], c["muted"]
        bg = T.SEL if sel else T.BG
        ink = T.INK if sel else (T.MID if mut else T.TEXT)         # muted name: MID on BG = 6.4:1 (the current DIM is 2.9:1)
        ink2 = T.INK if sel else (T.TEXT if c["midi_ch"] >= SQ_MIDI_OFF else T.MID)
        if sel:
            cv.rect(x + 1, y, 58, Z1_H, T.SEL)
        l1 = "%d %s" % (i + 1, SHORT[c["algo"] % SQ_NALGO])
        l2 = "MIDI OFF" if c["midi_ch"] >= SQ_MIDI_OFF else "MIDI %d" % (c["midi_ch"] + 1)       # ALWAYS the MIDI channel
        cv.text_fit(x + 2, y + 2, AF_S, l1, ink, bg, 42)
        cv.text_fit(x + 2, y + 14, AF_S, l2, ink2, bg, 48)
        if mut:                                                   # solid chip at the end of line 1: 12x11, the key's own letter
            fill, fg = (T.INK, T.SEL) if sel else (T.MID, T.INK)
            cv.rect(x + 45, y + 2, 12, 11, fill)
            cv.text_in(x + 45, y + 2, 12, AF_S, "M", fg, fill)
        else:                                                     # activity: lit = solid 4x4, idle = 2x2 dot (size-coded, same colour)
            c_act = T.INK if sel else T.THEME
            if c["snd0"]:
                cv.rect(x + 52, y + 18, 4, 4, c_act)
            else:
                cv.rect(x + 53, y + 19, 2, 2, T.INK if sel else T.MID)


# --------------------------------------------------------------------------------------------------------- Z2 LANE
BAR_BASE = 30                                                 # bar bottom (cell y+30), max bar 28 px
MARK_Y = 32                                                   # mark row y+32..y+38 (7 px)


def lane_marks(L):
    """The marks of a step as [(art rows, colour)] in the order accent, slide, ratchet. Accent is also on the bar (hue + cap)."""
    m = []
    if L["on"] and L["acc"]:
        m.append((M_ACC, H_ACCNT))
    if L["on"] and L["slide"]:
        m.append((M_SLIDE, H_ARTIC))
    if L["on"] and L["rep"] > 1:
        n = min(L["rep"], 3)
        rows = [".".join(["#"] * n)] * 7
        m.append((rows, H_ARTIC))
    return m


def draw_z2(cv, S, ch):
    c = S["channels"][ch]
    ln = c["len"]
    pages = (ln + SQ_PAGE - 1) // SQ_PAGE
    ph = playhead(S, ch)
    cv.text(2, Z2_Y + 1, AF_S, "PG %d/%d%s" % (S["vpage"] + 1, pages, "  FOLLOW" if S["follow"] else "  PINNED"), T.MID)
    if ph != 255:
        w = text_w(AF_S, "P%d" % (ph // SQ_PAGE + 1))
        cv.text_r(W - 4, Z2_Y + 1, AF_S, "P%d" % (ph // SQ_PAGE + 1), T.ACCENT, T.BG)
        art(cv, W - 4 - w - 8, Z2_Y + 4, G_PLAY_S, T.ACCENT)
    for n in range(SQ_PAGE):
        L = S["lane"][n]
        x, y = 15 * n, Z2_Y + 14
        p = S["vpage"] * SQ_PAGE + n
        live = p < ln
        isph = live and ph != 255 and p == ph
        cv.rect(x, y, 14, 46, (PLAYCOL if isph else T.LANE) if live else T.BG)     # the playing column is tinted over its FULL 46 px
        if live and L["on"]:
            hh = 2 + L["vel"] * 26 // 127
            top = y + BAR_BASE - hh
            cv.rect(x + 1, top, 12, hh, H_ACCNT if L["acc"] else H_HITS)
            if L["acc"]:
                cv.rect(x + 1, top, 12, 2, T.THEME)           # accent bars carry a 2 px light cap: shape, not only hue
        elif live:
            cv.rect(x + 1, y + BAR_BASE - 2, 12, 2, T.GRID if not isph else T.MID)
        if live:
            ms = lane_marks(L)
            if len(ms) >= 2 and ms[0][0] is M_ACC:                        # sharing the row: the compact 5 px accent triangle
                ms[0] = (M_ACC5, H_ACCNT)
            if sum(glyph_w(a) for a, _ in ms) + (len(ms) - 1) > 14:        # all three do not fit 14 px: the accent triangle yields
                ms = [m for m in ms if m[0] is not M_ACC5]               # to the bar cap + hue
            tot = sum(glyph_w(a) for a, _ in ms) + max(0, len(ms) - 1)
            mx = x + (14 - tot) // 2
            for a, col in ms:
                hgt = len(a)
                art(cv, mx, y + MARK_Y + (7 - hgt) // 2, a, col)
                mx += glyph_w(a) + 1
            if L["ed"]:
                cv.rect(x + 6, y + 40, 2, 2, T.TEXT)         # edited step: 2x2 dot under the marks
        if isph:
            cv.rect(x, y + 44, 14, 2, T.ACCENT)               # underline kept: the tint must not be the only cue


# --------------------------------------------------------------------------------------------------------- Z3 BAND
PEEK = {"OFFSET": "OFFS", "PUNCH": "PNC", "ACCNT": "ACNT", "RATCH": "RTCH", "LEGATO": "LEGA", "CURVE": "CURV", "SHAPE": "SHPE",
        "RANGE": "RNGE", "TOTAL": "TOTL", "PEDAL": "PDAL", "SLIDE": "SLID", "JUMP": "JUMP", "DENS": "DENS", "SPAN": "SPAN",
        "CTONE": "CTNE", "RDIV": "RDIV", "HITS": "HITS", "LEN": "LEN", "PATH": "PATH", "GLEN": "GLEN", "DYN": "DYN", "DIR": "DIR",
        "RATE": "RATE", "VARY": "VARY", "PICK": "PICK"}
CELL_W, TAG_W = 50, 40
ACT_H, INACT_H = 34, 18


def peek_pair(lab, val, maxw):
    """`LABEL value` on one AF_S line: the full label when it fits, else the 4-letter PEEK form (a proposed knob_map column)."""
    for l in (lab, PEEK.get(lab, lab[:4]), lab[:3]):
        if text_w(AF_S, l) + 2 + text_w(AF_S, val) <= maxw:
            return l
    return lab[:3]


def draw_z3(cv, S, ch):
    c = S["channels"][ch]
    algos = S["algos"]
    a = algos[c["algo"] % SQ_NALGO]
    ctx = c["algo"] if c["algo"] < SQ_NALGO else CTX_ANY
    row = S["row"]
    hs = (ACT_H, INACT_H) if row == 0 else (INACT_H, ACT_H)
    ys = (0, hs[0])
    for r in range(2):
        y, h, live = ys[r], hs[r], row == r
        if live:
            cv.rect(0, y, W, h, T.SEL)
        tag = "ROW %d" % (r + 1)
        cv.text_in(0, y + 1, TAG_W, AF_S, tag, T.INK if live else T.MID, T.SEL if live else T.BG)
        for k in range(4):
            x = TAG_W + CELL_W * k
            kn = r * 4 + k
            lab = TB.knob_label(PG_PERF, ctx, kn)
            m = a["knob_morph"][kn] if a["name"] else kn
            v = str(S["morph"][m % 8])
            hot = S["hot"][k]
            if not live:
                cv.rect(x, y + 3, 1, 12, T.LINE)                  # cell separators in the peek row group each `LABEL value`
            if live:
                cv.text_fit(x + 2, y + 1, AF_S, lab, T.INK, T.SEL, 46)
                cv.text_fit(x + 2, y + 14, AF_M, v, T.INK, T.SEL, 46)
            else:
                lb = peek_pair(lab, v, 47)
                px = cv.text(x + 3, y + 1, AF_S, lb, T.MID)
                cv.text(px + 2, y + 1, AF_S, v, T.ACCENT if hot else T.TEXT)


# --------------------------------------------------------------------------------------------------------- Z4 BUTTONS
RENAME = {"GT+SLD": "SLIDE", "RT+HLD": "RATCH", "HLP/MN": "MENU", "UNDO L": "REVERT"}
MARK_COL = 6                                                 # 5 px layer mark + 1 px gap
HOLD_COL = 7                                                 # 6 px chevron + 1 px gap


class Btn:
    def __init__(self, ctl, algo_i, algos):
        self.tap = RENAME.get(TB.btn_label(PG_PERF, ctl, MODE_TAP), TB.btn_label(PG_PERF, ctl, MODE_TAP))
        hold = RENAME.get(TB.btn_label(PG_PERF, ctl, MODE_HOLD), TB.btn_label(PG_PERF, ctl, MODE_HOLD))
        self.layer = LAYER_OF.get(ctl, -1)
        self.sub, self.chev, self.absent, self.glyph, self.dots = "", 0, 0, None, 0
        self.ctl = ctl
        algo = algos[algo_i % SQ_NALGO]
        if self.layer >= 0:
            sub = RENAME.get(algo["sub"][self.layer], algo["sub"][self.layer])
            if algo["layer_n"][self.layer] == 0:
                self.absent, self.sub = 1, "NONE"
            elif sub != self.tap:
                self.sub = sub                              # line 2 only when it adds meaning
        elif ctl == E["UI_C_B_FX"]:
            self.dots = 1                                    # ALL: the five layer hues as 2x2 chips on line 2
        elif hold and hold != self.tap:
            self.sub, self.chev = hold, 1
        if ctl == E["UI_C_B_PLAY"]:
            self.glyph = G_PLAY_S
        if ctl == E["UI_C_B_REC"]:
            self.glyph = G_REC_S

    def l1w(self):
        return text_w(AF_S, self.tap) + (MARK_COL if (self.layer >= 0 or self.glyph) else 0)

    def l2w(self):
        if self.dots:
            return 14
        return (text_w(AF_S, self.sub) + (HOLD_COL if self.chev else 0)) if self.sub else 0

    def need(self):
        return max(self.l1w(), self.l2w())


def btn_widths(row, algo, algos):
    bs = [Btn(row * 6 + i, algo, algos) for i in range(6)]
    w = [b.need() for b in bs]
    pad = 1 if sum(w) + 6 <= W else 0
    w = [x + pad for x in w]
    spare = W - sum(w)
    for i in range(spare if spare > 0 else 0):
        w[(i * 5) % 6 if spare < 6 else i % 6] += 1
    return w, bs


def draw_z4(cv, S, ch):
    c = S["channels"][ch]
    algos = S["algos"]
    for row in range(2):
        w, bs = btn_widths(row, c["algo"], algos)
        x = 0
        y = Z3_H + 26 * row
        for i, b in enumerate(bs):
            ty = y + 0                                        # line 1 y+0 (ink y+3..y+11), line 2 y+12 (ink y+15..y+23, y+26 would clip)
            w1 = b.l1w()
            x1 = x + (w[i] - w1) // 2
            col = T.MID if b.absent else T.TEXT
            mk = b.layer >= 0 or b.glyph is not None
            if mk:
                if b.layer >= 0:
                    a = LAYER_ART[b.layer]
                    hgt = len(a)
                    art(cv, x1, ty + 3 + (9 - hgt) // 2, a, T.MID if b.absent else LAYER_HUE[b.layer])
                elif b.glyph is G_REC_S:
                    art(cv, x1, ty + 5, b.glyph, T.REC)
                else:
                    art(cv, x1, ty + 5, b.glyph, T.ACCENT)
                cv.text(x1 + MARK_COL, ty, AF_S, b.tap, col)
            else:
                cv.text(x1, ty, AF_S, b.tap, col)
            if b.dots:
                x2 = x + (w[i] - 14) // 2
                for j, hue in enumerate(LAYER_HUE):
                    cv.rect(x2 + 3 * j, ty + 18, 2, 3, hue)
            elif b.sub:
                w2 = b.l2w()
                x2 = x + (w[i] - w2) // 2
                if b.chev:
                    art(cv, x2, ty + 17, G_CHEV, T.MID)
                    cv.text(x2 + HOLD_COL, ty + 12, AF_S, b.sub, T.MID)
                else:
                    cv.text(x2, ty + 12, AF_S, b.sub, T.MID)
            x += w[i]
            if i < 5:
                cv.rect(x - 1, y + 3, 1, 20, T.LINE)


# --------------------------------------------------------------------------------------------------------- Z5 LEGEND / Z6 CUE
def draw_z5(cv, S):
    for i in range(11):
        mut = i < SQ_NCH and S["channels"][i]["muted"]
        x = LEG_X[i]
        if i < 4:
            if mut:
                cv.rect(x, 0, 20, Z5_H, T.BG)
                cv.frame(x, 0, 20, Z5_H, T.LINE)
                cv.text_in(x, 0, 20, AF_S, "M%d" % (i + 1), T.MID, T.BG)
                cv.rect(x + 2, 6, 16, 1, T.TEXT)                      # strike: shape cue for muted (dim alone is not enough)
            else:
                cv.rect(x, 0, 20, Z5_H, T.RAISE)
                cv.text_in(x, 0, 20, AF_S, "M%d" % (i + 1), T.TEXT, T.RAISE)
            continue
        cv.rect(x, 0, 20, Z5_H, T.RAISE)
        a = (LG_CHN, LG_SOLO, LG_FILL, LG_ROLL, LG_PGM, LG_PGP, LG_RST)[i - 4]
        gw, gh = glyph_w(a), len(a)
        art(cv, x + (20 - gw) // 2, (Z5_H - gh) // 2, a, T.TEXT)


def cue_text(S):
    if S["playing"]:
        return "TAP STEP=ON/OFF  HOLD=EDIT NOTE"
    return TB.page_cue(PG_PERF)


def draw_z6(cv, S):
    ty = Z5_H - 3 + 2                                          # unchanged text, 2 px lower: a 2 px gap under the legend (ink rows 14..22)
    if S.get("toast"):
        cv.rect(0, Z5_H + 1, W, 11, T.TINT)                  # toast bar starts at row 13 (1 px gap)
        art(cv, 3, Z5_H + 4, G_UNDO2, T.ACCENT) if S["toast"].startswith("UNDO") else None
        cv.text_fit(14 if S["toast"].startswith("UNDO") else 4, ty, AF_S, S["toast"], T.ACCENT, T.TINT, 222)
        return
    cv.text_fit(0, ty, AF_S, cue_text(S), T.TEXT, T.BG, W)


def draw(cv, S):
    ch = S["sel"]
    cv.begin(W, BLIT_A_H, T.BG)
    draw_z0(cv, S, ch)
    draw_z1(cv, S)
    draw_z2(cv, S, ch)
    for n, (x, y, w, h) in (("Z0 HEAD", (0, 0, 240, 20)), ("Z1 CHANNELS", (0, 20, 240, 28)), ("Z2 LANE", (0, 48, 240, 60))):
        cv.mark(n, x, y, w, h)
    cv.blit(0, Z0_Y)
    cv.begin(W, BLIT_B_H, T.BG)
    draw_z3(cv, S, ch)
    draw_z4(cv, S, ch)
    cv.mark("Z3 BAND", 0, 0, 240, 52)
    cv.mark("Z4 BUTTONS", 0, 52, 240, 52)
    cv.blit(0, Z3_Y)
    cv.begin(W, BLIT_C_H, T.BG)
    draw_z5(cv, S)
    draw_z6(cv, S)
    cv.mark("Z5 LEGEND", 0, 0, 240, 12)
    cv.mark("Z6 CUE", 0, 12, 240, 12)
    cv.blit(0, Z5_Y)


# --------------------------------------------------------------------------------------------------------- fixtures (new ones)
def _long_names():
    s = fixture("perf_playing")
    s = copy.deepcopy(s)
    s.update(sel=3, vpage=3, playing=1, tempo_src="EXT", bpm=126, preset=16, playhead=51, row=0, follow=1)
    s["channels"] = [
        {"algo": 0, "midi_ch": 0, "muted": 0, "len": 20, "pg_used": 3, "snd0": 1, "p": 6},
        {"algo": 1, "midi_ch": 1, "muted": 0, "len": 16, "pg_used": 1, "snd0": 0, "p": 3},
        {"algo": 2, "midi_ch": 16, "muted": 0, "len": 16, "pg_used": 1, "snd0": 1, "p": 9},       # MIDI OFF, not muted
        {"algo": 3, "midi_ch": 15, "muted": 1, "len": 64, "pg_used": 15, "snd0": 0, "p": 52},     # selected, muted, M16, L64
    ]
    lane = []
    for n in range(16):
        lane.append({"on": 1 if n not in (2, 11) else 0, "vel": 30 + (n * 37) % 90, "ed": 1 if n in (1, 9) else 0, "acc": 0, "slide": 0, "rep": 1})
    lane[3].update(acc=1, vel=118)
    lane[4].update(slide=1, vel=70)
    lane[5].update(rep=2, vel=84)
    lane[6].update(acc=1, slide=1, vel=120)
    lane[7].update(acc=1, slide=1, rep=3, vel=125, ed=1)
    lane[8].update(rep=3, vel=60)
    s["lane"] = lane
    s["morph"] = [12, 100, 5, 3, 0, 4, 40, 7]
    return s


def _undo_toast():
    s = copy.deepcopy(fixture("perf_playing"))
    s["channels"][1]["snd0"] = 1
    s["toast"] = "UNDO: DICE ACCNT"
    return s


def scenes():
    pl = copy.deepcopy(fixture("perf_playing"))
    pl["channels"][1]["snd0"] = 1                                  # channel 2 also just sounded (the fixture only lights channel 1)
    return {"idle": fixture("perf"), "playing": pl, "row2": fixture("perf_row2"), "snake_mute": fixture("perf_snake_mute"),
            "long_names": _long_names(), "undo_toast": _undo_toast()}


STATE = {"scenes": scenes()}


# --------------------------------------------------------------------------------------------------------- --report
def report():
    from mock import measure
    print("== header: right edges (px) for the longest strings")
    for tag, S in (("EXT 126 / P16 / 4 STAG M16 / L64", dict(playing=1, bpm=126, tempo_src="EXT", preset=16, preset_pend=0)),
                   ("EXT -- ", dict(tempo_src="EXT --")), ("WAIT", dict(tempo_src="WAIT"))):
        d, wd = tempo_parts({"bpm": 126, "tempo_src": S.get("tempo_src", "INT")})
        end = HX["src"] + text_w(AF_S, wd) if d not in (None,) else HX["bpm"] + text_w(AF_S, wd)
        print("  %-34s tempo group ends x=%d (preset slot starts %d)" % (tag, end, HX["pre"]))
    print("  AF_M '126' %d, AF_M '--' %d, AF_S 'EXT' %d 'WAIT' %d" % (text_w(AF_M, "126"), text_w(AF_M, "--"), text_w(AF_S, "EXT"), text_w(AF_S, "WAIT")))
    print("  preset 'P16' %d, 'P05?' %d -> slot %d..%d" % (text_w(AF_S, "P16"), text_w(AF_S, "P05?"), HX["pre"], HX["algo"]))
    for s in ("4 STAG M16", "4 STAG MOFF", "1 PATT MOFF", "1 ACID M1"):
        print("  header algo %-12s %d px  (slot %d..%d = %d) -> %s" % (s, text_w(AF_S, s), HX["algo"], HX["algo"] + H_ALGO_W, H_ALGO_W,
                                                                  "fits" if text_w(AF_S, s) <= H_ALGO_W else "OVER"))
    print("  page map %d..%d, L64 %d..%d (advance %d), right edge %d of 240" % (HX["map"], HX["map"] + 39, HX["len"], HX["len"] + text_w(AF_S, "L64"), text_w(AF_S, "L64"), HX["len"] + text_w(AF_S, "L64")))
    print("== Z1 cells (ink budget: name slot 42 + chip 12, MIDI slot 48)")
    for s in ("1 ACID", "3 SNAK", "4 STAG", "2 EUCL", "4 PATT"):
        print("  line1 %-8s %d" % (s, text_w(AF_S, s)))
    for s in ("MIDI 3", "MIDI 16", "MIDI OFF", "MUTE"):
        print("  line2 %-9s %d" % (s, text_w(AF_S, s)))
    print("  MUTE chip with the word: %d + 2 pad = %d px; name %d -> %d > 56 ink px: does NOT fit; 12 px chip with `M` used" % (text_w(AF_S, "MUTE"), text_w(AF_S, "MUTE") + 2, text_w(AF_S, "4 STAG"), text_w(AF_S, "4 STAG") + text_w(AF_S, "MUTE") + 2))
    print("== Z3 band")
    ids = [CTX_ANY] + list(range(6))
    worst = 0
    for ctx in range(6):
        for k in range(8):
            lab = TB.knob_label(PG_PERF, ctx, k)
            full = text_w(AF_S, lab) + 2 + text_w(AF_S, "40")
            used = peek_pair(lab, "40", 47)
            worst = max(worst, text_w(AF_S, used) + 2 + text_w(AF_S, "40"))
            if full > 47:
                print("  peek pair %-7s +'40' = %d > 47 -> %-5s (%d)" % (lab, full, used, text_w(AF_S, used) + 2 + text_w(AF_S, "40")))
    print("  worst peek pair after PEEK forms: %d of 47; AF_M '100' = %d, '--' %d (cell 46)" % (worst, text_w(AF_M, "100"), text_w(AF_M, "--")))
    print("  active labels (AF_S) longest: LEGATO %d of 46" % text_w(AF_S, "LEGATO"))
    print("== Z4 button widths (sum must be <= 240)")
    algos = fixture("perf")["algos"]
    for ai in range(6):
        for row in range(2):
            w, bs = btn_widths(row, ai, algos)
            need = sum(b.need() for b in bs)
            print("  algo %d row %d need %3d  widths %s sum %d" % (ai + 1, row + 1, need, w, sum(w)))
    print("== contrast (RGB565 expanded, WCAG)")
    toks = [("T.TEXT", T.TEXT), ("T.MID", T.MID), ("T.DIM", T.DIM), ("T.INK", T.INK)] + NEW_TOKENS
    bgs = [("BG", T.BG), ("LANE", T.LANE), ("PLAYCOL", PLAYCOL), ("SEL", T.SEL), ("TINT", T.TINT), ("RAISE", T.RAISE)]
    print("  %-10s" % "" + "".join("%9s" % n for n, _ in bgs))
    for n, c in toks:
        print("  %-10s 0x%04X" % (n, c) + "".join("%9.2f" % contrast_ratio(c, b) for _, b in bgs))
    print("  T.INK on SEL %.2f, T.MID on BG %.2f, T.TEXT on RAISE %.2f, T.TEXT on BG %.2f, T.ACCENT on TINT %.2f, T.MID on LANE %.2f" % (
        contrast_ratio(T.INK, T.SEL), contrast_ratio(T.MID, T.BG), contrast_ratio(T.TEXT, T.RAISE), contrast_ratio(T.TEXT, T.BG), contrast_ratio(T.ACCENT, T.TINT), contrast_ratio(T.MID, T.LANE)))
    print("  chip M: INK on MID %.2f ; SEL on INK %.2f" % (contrast_ratio(T.INK, T.MID), contrast_ratio(T.SEL, T.INK)))
    print("  PLAYCOL vs LANE %.2f ; T.THEME vs PLAYCOL %.2f" % (contrast_ratio(PLAYCOL, T.LANE), contrast_ratio(T.THEME, PLAYCOL)))
    print("== cue strip: T.TEXT on T.BG %.2f" % contrast_ratio(T.TEXT, T.BG))
    print("== strings")
    for s in ("MENU", "REVERT", "SLIDE", "RATCH", "RESET", "HARM", "NONE", "OCTAVE"):
        print("  %-7s %d" % (s, text_w(AF_S, s)))


if __name__ == "__main__":
    from mock import palette_set
    palette_set("GREY")
    report()
