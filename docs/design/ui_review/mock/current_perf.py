# SPDX-License-Identifier: GPL-3.0-only
"""current_perf.py: the CURRENT (shipping) PERF screen, redrawn with the mock primitives only. It is a line-by-line port of the draw code of
firmware/src/ui_sq.c (uisq_draw_z0..z6 and the three blits of ui_sq_draw), reading the labels from build/gen/ui_tables.h (mock.tables())
and the state from fixtures/perf_fixtures.json (the state tests/ui_sq_test.c had when it wrote build/ui_sq/perf*.png).

It is the fidelity gate of the tool: `python mock.py render current_perf.py --out DIR` must equal build/ui_sq/perf.png, perf_row2.png,
perf_playing.png and perf_snake_mute.png pixel for pixel (`python check_fidelity.py`). It is also the template for a proposal: copy it,
change draw().
"""
from mock import AF_S, T, ux, tables, text_w, fixture, text_fit_str

NAME = "current_perf"
PALETTE = "GREY"                           # tests/ui_sq_test.c: palette_set(0)
STATE = {"scenes": fixture()}              # perf, perf_row2, perf_playing, perf_snake_mute (see fixtures/perf_fixtures.json)

SQ_PAGE, SQ_NCH, SQ_MIDI_OFF, SQ_NMORPH, SQ_NALGO = 16, 4, 16, 8, 6
W = 240
Z0_Y, Z0_H, Z1_Y, Z1_H, Z2_Y, Z2_H, Z3_Y, Z3_H, Z4_Y, Z4_H, Z5_Y, Z5_H, Z6_Y, Z6_H = 0, 20, 20, 28, 48, 60, 108, 52, 160, 52, 212, 12, 224, 12
BLIT_A_H, BLIT_B_H, BLIT_C_H = Z0_H + Z1_H + Z2_H, Z3_H + Z4_H, Z5_H + Z6_H + 4
H1_X, H2_X, H2T_X, H3_X, H4_X, H4_W, H5_X, H6_X = 2, 12, 23, 68, 96, 76, 174, 214
LEG_X = (0, 21, 42, 65, 86, 109, 130, 151, 174, 195, 218)
SHORT = ("ACID", "EUCL", "SNAK", "STAG", "ARP", "PATT")
MARK_W = 6
Z6_TEXT_DY = -3

TB = tables()
E = TB.enums
PG_PERF = E["UI_PG_PERF"]
MODE_TAP, MODE_HOLD = E["UI_MODE_TAP"], E["UI_MODE_HOLD"]
GEST_TAP, GEST_HOLD = E["UI_GEST_TAP"], E["UI_GEST_HOLD"]
CTX_ANY = E["UI_CTX_ANY"]
LAYER_OF = {E["UI_C_B_SCL"]: 0, E["UI_C_B_SEQ"]: 1, E["UI_C_B_ENV"]: 2, E["UI_C_B_ARP"]: 3, E["UI_C_B_LFO"]: 4}


def fit(cv, x, y, s, fg, bg, maxw):                       # uisq_fit
    cv.text_fit(x, y, AF_S, s, fg, bg, maxw)


def wrap(s, n, maxw):                                     # uisq_wrap: (split?, l1, l2)
    l1 = s[:n - 1]
    if text_w(AF_S, s) <= maxw:
        return 0, l1, ""
    k = s.find(" ")
    if k < 0:
        return 0, l1, ""
    return 1, s[:min(k, n - 1)], s[k + 1:][:n - 1]


class Btn:                                                # uisq_btn_get
    def __init__(self, page, ctl, algo_i, algos):
        self.tap = tap = TB.btn_label(page, ctl, MODE_TAP)
        hold = TB.btn_label(page, ctl, MODE_HOLD)
        ly = LAYER_OF.get(ctl, -1)
        algo = algo_i % SQ_NALGO
        self.sub, self.sub_mark, self.dim = "", 0, 0
        self.accent = int(tap[:3] == "RST") or int(tap in ("BACK", "CANCEL"))
        if page == PG_PERF and ly >= 0 and algos[algo]["name"]:
            self.sub = algos[algo]["sub"][ly]
            if algos[algo]["layer_n"][ly] == 0:
                self.dim, self.sub = 1, "-"
        elif hold and hold != tap:
            self.sub, self.sub_mark = hold, 1

    def need(self):
        a = text_w(AF_S, self.tap)
        s = text_w(AF_S, self.sub) + (MARK_W if self.sub_mark else 0) if self.sub else 0
        return max(a, s)


def btn_widths(page, row, algo, algos):                   # uisq_btn_widths
    w = [Btn(page, row * 6 + i, algo, algos).need() for i in range(6)]
    pad = 3
    if sum(w) + 6 * pad > W:
        pad = 1
    w = [x + pad for x in w]
    spare = W - sum(w)
    if spare > 0:
        w[4] += spare // 2
        w[5] += spare - spare // 2
    return w


def hold_mark(cv, x, y, c):                               # 5x7 filled down-triangle
    for r, wd in enumerate((5, 5, 3, 3, 3, 1, 1)):
        cv.rect(x + (5 - wd) // 2, y + r, wd, 1, c)


def dot(cv, x, y, c):                                     # the 8x8 clock dot
    cv.rect(x + 1, y, 6, 8, c)
    cv.rect(x, y + 1, 8, 6, c)


def playhead(S, ch):                                      # the step now sounding, 255 = stopped
    c = S["channels"][ch]
    n = c["len"] or 1
    return 255 if not S["playing"] else (c["p"] + n - 1) % n


def draw_z0(cv, S, ch):
    c = S["channels"][ch]
    ph, ln = playhead(S, ch), c["len"]
    cv.text(H1_X, 4, AF_S, ">" if S["playing"] else ".", T.TEXT)
    dot(cv, H2_X, 6, 0xCB80 if ux.light else 0xFD80)
    cv.text(H2T_X, 4, AF_S, "INT %d" % S["bpm"], T.TEXT)
    cv.text(H3_X, 4, AF_S, "P%s%d%s" % ("0" if S["preset"] < 10 else "", S["preset"], "?" if S["preset_pend"] else ""), T.TEXT)
    midi = "MOFF" if c["midi_ch"] >= SQ_MIDI_OFF else "M%d" % (c["midi_ch"] + 1)
    cv.text_fit(H4_X, 4, AF_S, "%d %s %s" % (ch + 1, SHORT[c["algo"] % SQ_NALGO], midi), T.TEXT, T.BG, H4_W)
    for k in range(4):
        x, y = H5_X + 10 * k, 4
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
                cv.text_in(x, y - 1, 9, AF_S, d, T.DIM, T.BG)
            cv.frame(x, y, 9, 11, T.TEXT if (k == S["vpage"] or playing) else T.DIM)
            if partial:
                for i in range(1, 8, 2):
                    cv.pset(x + i, y + 10, T.BG)
        elif used:
            for i in range(0, 9, 2):
                cv.pset(x + i, y, T.DIM)
                cv.pset(x + i, y + 10, T.DIM)
            for i in range(0, 11, 2):
                cv.pset(x, y + i, T.DIM)
                cv.pset(x + 8, y + i, T.DIM)
        else:
            cv.rect(x + 3, y + 4, 2, 2, T.DIM)
    cv.text(H6_X, 4, AF_S, "L%d" % ln, T.TEXT)


def draw_z1(cv, S):
    for i in range(SQ_NCH):
        c = S["channels"][i]
        x, y = 60 * i, Z1_Y
        sel, mut = i == S["sel"], c["muted"]
        ink = T.INK if sel else T.DIM if mut else T.TEXT
        ink2 = T.INK if sel else T.DIM if mut else T.MID
        l1 = "%d %s" % (i + 1, SHORT[c["algo"] % SQ_NALGO])
        m = c["midi_ch"]
        l2 = ("MUTE " if mut else "MIDI ") + (("-" if mut else "OFF") if m >= SQ_MIDI_OFF else str(m + 1))
        if sel:
            cv.rect(x + 1, y, 58, Z1_H, T.SEL)
        cv.text_fit(x + 2, y + 2, AF_S, l1, ink, T.SEL if sel else T.BG, 56)
        cv.text_fit(x + 2, y + 14, AF_S, l2, T.ACCENT if (not mut and m >= SQ_MIDI_OFF and not sel) else ink2, T.SEL if sel else T.BG, 56)
        if c["snd0"]:
            cv.rect(x + 54, y + 4, 4, 4, T.INK if sel else T.ACCENT)


def draw_z2(cv, S, ch):
    c = S["channels"][ch]
    ln = c["len"]
    pages = (ln + SQ_PAGE - 1) // SQ_PAGE
    ph = playhead(S, ch)
    b = "PG %d/%d%s  hold=EDIT" % (S["vpage"] + 1, pages, "  FOLLOW" if S["follow"] else "  hold")
    cv.text(2, Z2_Y + 1, AF_S, b, T.MID)
    if ph != 255:
        cv.text_r(W - 4, Z2_Y + 1, AF_S, ">P%d" % (ph // SQ_PAGE + 1), T.ACCENT, T.BG)
    for n in range(SQ_PAGE):
        L = S["lane"][n]
        x, y = 15 * n, Z2_Y + 14
        p = S["vpage"] * SQ_PAGE + n
        live = p < ln
        cv.rect(x, y, 14, 46, T.LANE if live else T.BG)
        if live and L["on"]:
            hh = 2 + L["vel"] * 30 // 127
            cv.rect(x + 1, y + 32 - hh, 12, hh, T.ACCENT if L["acc"] else T.THEME)
        elif live:
            cv.rect(x + 1, y + 30, 12, 2, T.GRID)
        if live:
            g = ""
            if L["rep"] > 1:
                g = "x%d" % L["rep"]
            elif L["slide"]:
                g = "~"
            elif L["on"] and L["acc"]:
                g = "!"
            elif L["ed"]:
                g = "'"
            if g:
                cv.text_in(x, y + 33, 14, AF_S, g, T.TEXT, T.LANE)
        if live and ph != 255 and p == ph:
            cv.rect(x, y + 44, 14, 2, T.ACCENT)


def draw_z3(cv, S, ch):                                   # canvas y = screen y - 108; PERF is the two-row page
    c = S["channels"][ch]
    algos = S["algos"]
    algo = c["algo"] % SQ_NALGO
    a = algos[algo]
    ctx = c["algo"] if c["algo"] < SQ_NALGO else CTX_ANY
    two = True
    for h in range(2):
        b = Btn(PG_PERF, E["UI_C_B_OCTUP"] if h else E["UI_C_B_OCTDN"], c["algo"], algos)
        y = 26 * h
        live = two and S["row"] == h
        if live:
            cv.rect(0, y, 36, 26, T.SEL)
        sp, l1, l2 = wrap(b.tap, 8, 33)
        if sp and not b.sub:
            fit(cv, 2, y + 1, l1, T.INK if live else T.TEXT, T.SEL if live else T.BG, 33)
            fit(cv, 2, y + 13, l2, T.INK if live else T.TEXT, T.SEL if live else T.BG, 33)
            continue
        fit(cv, 0, y + 1, b.tap, T.INK if live else T.TEXT, T.SEL if live else T.BG, 36)
        if b.sub:
            if b.sub_mark:
                hold_mark(cv, 2, y + 15, T.MID)
            fit(cv, 2 + (MARK_W if b.sub_mark else 0), y + 13, b.sub, T.MID, T.SEL if live else T.BG, 33 - (MARK_W if b.sub_mark else 0))
    for k in range(4):
        x = 36 + 51 * k
        hot = S["hot"][k]
        for r in range(2):
            kn = r * 4 + k
            lab = TB.knob_label(PG_PERF, ctx, kn)
            y, ly, vy = 26 * r, 1, 13
            live = S["row"] == r
            m = a["knob_morph"][kn] if a["name"] else kn
            v = str(S["morph"][m % SQ_NMORPH])
            if live:
                cv.rect(x, y, 51, 26, T.SEL)
            fit(cv, x + 2, y + ly, lab, T.INK if live else T.DIM, T.SEL if live else T.BG, 49)
            fit(cv, x + 2, y + vy, v, T.INK if live else T.ACCENT if hot else T.DIM, T.SEL if live else T.BG, 49)
    # (two-row page: the status lines are not drawn)


def draw_z4(cv, S, ch):
    c = S["channels"][ch]
    algos = S["algos"]
    for row in range(2):
        w = btn_widths(PG_PERF, row, c["algo"], algos)
        x = 0
        for i in range(6):
            b = Btn(PG_PERF, row * 6 + i, c["algo"], algos)
            y = Z3_H + 26 * row
            cv.text_in(x, y + 1, w[i], AF_S, b.tap, T.DIM if b.dim else T.ACCENT if b.accent else T.TEXT, T.BG)
            if b.sub:
                sw = text_w(AF_S, b.sub) + (MARK_W if b.sub_mark else 0)
                q = w[i] - sw
                x0 = x + (q // 2 if q >= 0 else -((-q) // 2))          # C int division
                if b.sub_mark:
                    hold_mark(cv, x0, y + 15, T.MID)
                fit(cv, x0 + (MARK_W if b.sub_mark else 0), y + 13, b.sub, T.DIM if b.dim else T.MID, T.BG, w[i])
            x += w[i]
            if i < 5:
                cv.rect(x - 1, y + 3, 1, 20, T.LINE)


def draw_z5(cv, S):
    for i in range(11):
        ctl = E["UI_C_KEY_B01_FS3"] + i
        lab = TB.key_label(PG_PERF, ctl, GEST_TAP)
        if not lab:
            lab = TB.key_label(PG_PERF, ctl, GEST_HOLD)
        mut = i < SQ_NCH and S["channels"][i]["muted"]
        cv.rect(LEG_X[i], 0, 20, Z5_H, T.RAISE)
        cv.text_fit(LEG_X[i] + 1, 0, AF_S, lab, T.DIM if mut else T.TEXT, T.RAISE, 18)


def cue_text(S):
    if S["playing"]:                                       # (PERF; no ALGO_PICK, no toast in these scenes)
        return "TAP STEP=ON/OFF  HOLD=EDIT NOTE"
    return TB.page_cue(PG_PERF)


def draw_z6(cv, S):
    ty = Z5_H + Z6_TEXT_DY
    fit(cv, 0, ty, cue_text(S), T.TEXT, T.BG, W)


def draw(cv, S):
    ch = S["sel"]
    cv.begin(W, BLIT_A_H, T.BG)                           # blit A = Z0..Z2
    draw_z0(cv, S, ch)
    draw_z1(cv, S)
    draw_z2(cv, S, ch)
    cv.blit(0, Z0_Y)
    cv.begin(W, BLIT_B_H, T.BG)                           # blit B = Z3..Z4
    draw_z3(cv, S, ch)
    draw_z4(cv, S, ch)
    cv.blit(0, Z3_Y)
    cv.begin(W, BLIT_C_H, T.BG)                           # blit C = Z5..Z7
    draw_z5(cv, S)
    draw_z6(cv, S)
    cv.blit(0, Z5_Y)
