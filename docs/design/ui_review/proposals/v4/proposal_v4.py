# SPDX-License-Identifier: GPL-3.0-only
"""proposal_v4.py: round 1 of the design session (2026-11-12). A main screen that SCALES with the channels in use
(1, 2 or 4 rows), a colour per channel, pitch-height lanes, the algorithm named above the knobs it drives, a small
knob-row marker, and a press flash (icon + printed button + layer) with the changed steps highlighted.
   python3 ../../mock/mock.py render proposal_v4.py --out out"""
from mock import T, AF_S, AF_M, AF_L, text_w, rgb565

PALETTE = "MONO"            # black/white, mono=0, so coloured text keeps its colour (GREY is mono: gfx.c:154)
W = 240
SPLIT = 124                     # strip A 0..124, strip B 124..240 (CV_MAX 124); no text may cross y=124

# channel colours (new tokens T_CH1..T_CH4; per-palette values + mono fallback to be specified)
CH_COL = (rgb565(255, 150, 40), rgb565(70, 190, 255), rgb565(150, 220, 80), rgb565(255, 110, 180))
INK = rgb565(8, 8, 8)
WHITE = rgb565(250, 250, 250)
PH = rgb565(70, 70, 70)            # playhead column

# font metrics (measured with mock.py measure): cap top / cap height inside the line box
CAP = {id(AF_S): (3, 9), id(AF_M): (4, 11), id(AF_L): (7, 21)}


class P:
    """Draws in screen coordinates into the current strip (offset oy)."""
    def __init__(self, cv, oy):
        self.cv, self.oy = cv, oy

    def rect(self, x, y, w, h, c):
        self.cv.rect(x, y - self.oy, w, h, c)

    def frame(self, x, y, w, h, c):
        self.cv.frame(x, y - self.oy, w, h, c)

    def line(self, x0, y0, x1, y1, c):
        self.cv.line(x0, y0 - self.oy, x1, y1 - self.oy, c)

    def _vis(self, ycap, f):
        return ycap + CAP[id(f)][1] - self.oy > 0 and ycap - self.oy < SPLIT_H[self.oy]

    def text(self, x, ycap, f, s, fg, bg):
        """Text placed by its CAP TOP (ycap), so sizes align by what the eye sees."""
        if not self._vis(ycap, f):
            return
        self.cv.text_on(x, ycap - CAP[id(f)][0] - self.oy, f, s, fg, bg)

    def text_c(self, x0, w, ycap, f, s, fg, bg):
        self.text(x0 + (w - text_w(f, s)) // 2, ycap, f, s, fg, bg)

    def text_r(self, xr, ycap, f, s, fg, bg):
        self.text(xr - text_w(f, s), ycap, f, s, fg, bg)


SPLIT_H = {0: SPLIT, SPLIT: W - SPLIT}


# ---------- icons, from primitives ----------
def icon_play(p, x, y, h, c):
    for i in range(h):
        d = i if i <= h // 2 else h - 1 - i
        p.rect(x, y + i, d + 1, 1, c)


def icon_stop(p, x, y, h, c):
    p.rect(x, y, h, h, c)


def icon_dice(p, x, y, s, fg, bg):
    p.rect(x, y, s, s, fg)
    p.rect(x + 2, y + 2, s - 4, s - 4, bg)
    pip = max(2, s // 6)
    for (fx, fy) in ((0.25, 0.25), (0.75, 0.75), (0.5, 0.5), (0.75, 0.25), (0.25, 0.75)):
        p.rect(x + int(fx * s) - pip // 2, y + int(fy * s) - pip // 2, pip, pip, fg)


def icon_mute(p, x, y, c):
    """speaker with a cross, 13x11"""
    p.rect(x, y + 3, 3, 5, c)
    for i in range(4):
        p.rect(x + 3 + i, y + 3 - i, 1, 5 + 2 * i, c)
    for i in range(5):
        p.rect(x + 8 + i, y + 3 + i, 1, 1, c)
        p.rect(x + 12 - i, y + 3 + i, 1, 1, c)


# ---------- zones ----------
def header(p, S):
    p.rect(0, 0, W, 26, T.BG)
    if S["playing"]:
        icon_play(p, 4, 6, 15, WHITE)
    else:
        icon_stop(p, 4, 7, 12, T.MID)
    p.text(24, 8, AF_M, "%d" % S["bpm"], T.TEXT, T.BG)
    p.text_r(236, 8, AF_M, "P%02d" % S["preset"], T.TEXT, T.BG)


def lane_geom(x0, x1):
    """16 steps across x0..x1 with a wider gap every 4 steps; returns step x list and step width."""
    avail = x1 - x0
    gap, bgap = 1, 4
    sw = (avail - 12 * gap - 3 * bgap) // 16
    xs, x = [], x0
    for n in range(16):
        xs.append(x)
        x += sw + (bgap if n % 4 == 3 else gap)
    return xs, sw


def draw_lane(p, S, ci, x0, y, x1, h, sel):
    ch = S["chans"][ci]
    col = CH_COL[ci]
    xs, sw = lane_geom(x0, x1)
    muted = ch["muted"]
    ph = S["playhead"] if S["playing"] else -1
    changed = S.get("changed", set()) if sel else set()
    lo, hi = min(s[1] for s in ch["lane"] if s[0]), max(s[1] for s in ch["lane"] if s[0])
    rng = max(1, hi - lo)
    minb = max(3, h // 6)
    for n, (on, pitch, acc, slide) in enumerate(ch["lane"]):
        x = xs[n]
        if n == ph:                                   # playhead: the whole step column lit
            p.rect(x - 1, y, sw + 2, h, PH)
        if not on:
            p.rect(x + sw // 2 - 1, y + h - 3, 2, 2, T.DIM)   # rest = a dot on the floor
            continue
        bh = minb + (h - minb - (4 if h > 40 else 2)) * (pitch - lo) // rng
        fg = T.DIM if muted else (WHITE if n == ph or n in changed else col)
        if muted:
            p.frame(x, y + h - bh, sw, bh, fg)
        else:
            p.rect(x, y + h - bh, sw, bh, fg)
        if acc and not muted:                         # accent = a separate cap floating over the bar (shape, not colour)
            p.rect(x, y + h - bh - 5, sw, 3, WHITE)
        if slide and n < 15 and ch["lane"][n + 1][0] and not muted:
            p.rect(x + sw, y + h - bh + 1, xs[n + 1] - x - sw, 2, fg)
        if n in changed:
            p.rect(x, y + h + 1, sw, 2, WHITE)


def chip(p, ci, x, y, w, h, sel, muted, big, midi):
    col = CH_COL[ci]
    if muted:
        p.frame(x, y, w, h, T.DIM)                    # muted: grey, hollow, struck through (shape, not colour)
        f = AF_L if h >= 26 else AF_M
        p.text_c(x, w, y + 4 if big else y + (h - CAP[id(f)][1]) // 2, f, "%d" % (ci + 1), T.MID, T.BG)
        p.line(x + 1, y + h - 2, x + w - 2, y + 1, T.MID)
        if big:
            icon_mute(p, x + (w - 13) // 2, y + h - 16, T.MID)
        return
    if sel:
        p.rect(x, y, w, h, col)
        fg, bg = INK, col
    else:
        p.frame(x, y, w, h, col)
        fg, bg = col, T.BG
    f = AF_L if h >= 26 else AF_M
    cap = CAP[id(f)][1]
    ytop = y + 4 if big else y + (h - cap) // 2
    p.text_c(x, w, ytop, f, "%d" % (ci + 1), fg, bg)
    if big:                                           # the MIDI channel, only when there is room
        p.text_c(x, w, y + h - 26, AF_S, "MIDI", fg, bg)
        p.text_c(x, w, y + h - 15, AF_M, "%d" % midi, fg, bg)


def channels(p, S):
    used = [i for i, c in enumerate(S["chans"]) if c["used"]]
    n = len(used)
    top, bot = 28, 154
    gap = 3 if n > 1 else 0
    rh = (bot - top - gap * (n - 1)) // n
    big = rh >= 50
    cw = 30
    for k, ci in enumerate(used):
        y = top + k * (rh + gap)
        sel = ci == S["sel"]
        ch = S["chans"][ci]
        if sel and n > 1:
            p.rect(cw + 2, y, W - cw - 2, rh, T.RAISE)
        chip(p, ci, 0, y, cw, rh, sel, ch["muted"], big, ch["midi"])
        draw_lane(p, S, ci, cw + 6, y + 3, W - 3, rh - 7, sel)


def row_marker(p, x, y, row, col):
    for r in range(3):
        if r == row:
            p.rect(x + r * 9, y, 6, 6, col)
        else:
            p.frame(x + r * 9, y, 6, 6, T.MID)


def knobs(p, S):
    ci = S["sel"]
    ch = S["chans"][ci]
    col = CH_COL[ci]
    y0 = 158
    p.text(4, y0 + 2, AF_M, ch["algo"], col, T.BG)        # the algorithm the four knobs belong to
    row_marker(p, 214, y0 + 4, S["row"], col)
    cw, g, yc = 58, 2, y0 + 20
    for k in range(4):
        x = k * (cw + g) + 1
        lab, val, frac = ch["knobs"][S["row"]][k]
        p.rect(x, yc, cw, 60, T.RAISE)
        p.text_c(x, cw, yc + 5, AF_M, lab, T.MID, T.RAISE)
        hot = S.get("hot") == k
        p.text_c(x, cw, yc + 23, AF_L, val, WHITE if hot else T.TEXT, T.RAISE)
        p.rect(x + 6, yc + 52, cw - 12, 3, T.LINE)
        p.rect(x + 6, yc + 52, (cw - 12) * frac // 100, 3, col)


def flash(p, S):
    """a press: printed button (boxed) + icon + what happened, over the channel area for ~1 s"""
    fl = S.get("flash")
    if not fl:
        return
    printed, word = fl
    x, y, w, h = 0, 156, 240, 84                      # replaces the knob band for ~1 s; the lanes stay visible
    p.rect(x, y, w, h, T.BG)
    p.frame(x, y, w, h, WHITE)
    p.frame(x + 1, y + 1, w - 2, h - 2, WHITE)
    icon_dice(p, x + 14, y + 20, 30, WHITE, T.BG)
    p.text(x + 58, y + 18, AF_L, word, WHITE, T.BG)
    bw = text_w(AF_M, printed) + 10
    p.frame(x + 58, y + 50, bw, 19, T.MID)
    p.text(x + 63, y + 54, AF_M, printed, T.MID, T.BG)
    p.text(x + 66 + bw, y + 54, AF_M, "DICED", T.MID, T.BG)


def draw(cv, S):
    for oy in (0, SPLIT):
        cv.begin(W, SPLIT_H[oy], T.BG)
        p = P(cv, oy)
        header(p, S)
        channels(p, S)
        knobs(p, S)
        flash(p, S)
        cv.blit(0, oy)


# ---------- scenes ----------
def lane(spec):
    """spec: 16 chars: '.' rest, digit/letter = pitch 0..15 (hex); uppercase/'!' suffix not used: accents/slides as sets"""
    out = []
    for c in spec:
        out.append((0, 0, 0, 0) if c == "." else (1, int(c, 16), 0, 0))
    return out


def deco(l, acc=(), slide=()):
    return [(o, pi, 1 if n in acc else 0, 1 if n in slide else 0) for n, (o, pi, a, s) in enumerate(l)]


ACID = [[("DENS", "12", 75), ("SLIDE", "4", 50), ("JUMP", "5", 25), ("ACCNT", "3", 37)],
        [("OFFSET", "0", 0), ("PEDAL", "6", 50), ("PUNCH", "40", 85), ("DIR", "FWD", 0)], None]
EUCL = [[("HITS", "5", 31), ("OFFSET", "0", 0), ("RATCH", "4", 50), ("ACCNT", "2", 25)], None, None]
SNAK = [[("DENS", "10", 62), ("LEN", "7", 43), ("RANGE", "12", 25), ("PATH", "7", 50)], None, None]
STAG = [[("DENS", "6", 75), ("TOTAL", "16", 14), ("DIR", "FWD", 0), ("RATCH", "2", 25)], None, None]

L1 = deco(lane("3.7.35.a3.07.5c."), acc=(0, 7, 13), slide=(4,))
L2 = deco(lane("0..0..0...0..0.."), acc=(0,))
L3 = deco(lane("5.8.a.c.3..7.8.a"))
L4 = deco(lane("0...4...7...4..."))
L1_DICED = deco(lane("5.a.38.c5.27.8e."), acc=(0, 7, 13), slide=(4,))


def ch(algo, midi, lanes, knobs_, used=True, muted=False):
    return {"algo": algo, "midi": midi, "lane": lanes, "knobs": knobs_, "used": used, "muted": muted}


def base(n, **kw):
    chans = [ch("ACID", 2, L1, ACID), ch("EUCLID", 10, L2, EUCL), ch("SNAKE", 3, L3, SNAK), ch("STAGE", 4, L4, STAG)]
    for i in range(n, 4):
        chans[i]["used"] = False
    s = {"playing": True, "bpm": 120, "preset": 1, "playhead": 5, "sel": 0, "row": 0, "chans": chans}
    s.update(kw)
    return s


def four_muted():
    s = base(4, sel=1)
    s["chans"][2]["muted"] = True
    return s


def dice():
    s = base(2, hot=None)
    s["chans"][0]["lane"] = L1_DICED
    s["changed"] = {2, 3, 4, 5, 8, 10, 13, 14}
    s["flash"] = ("SCL", "NOTES")
    return s


def dice_after():
    s = dice()
    s["flash"] = None
    return s


STATE = {"scenes": {
    "one_channel": base(1),
    "one_channel_stopped": base(1, playing=False),
    "two_channels": base(2),
    "four_channels": four_muted(),
    "dice_flash": dice(),
    "dice_after": dice_after(),
}}
