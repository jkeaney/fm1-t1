from common import *
import itertools, math
import numpy as np
M.palette_set("HI-CON")
print("# HI-CON tokens"); 
for t in M.Theme.TOKENS: print(" ", t, hexc(getattr(M.ux, t)))
# ---------------- 3a text pairs across ALL scenes
pairs = {}
for n in mod.STATE["scenes"]:
    r = rec(n)
    for t in r["texts"]:
        k = (t["fg"], t["bg"], t["font"])
        pairs.setdefault(k, []).append((n, t["s"]))
print("\n# 3a text pairs, all 28 scenes (distinct fg/bg/font)")
print("| fg | bg | font | ratio | threshold | verdict | n strings | examples |")
print("|---|---|---|---|---|---|---|---|")
for (fg, bg, f), v in sorted(pairs.items(), key=lambda kv: contrast_ratio(kv[0][0], kv[0][1])):
    cr = contrast_ratio(fg, bg); th = 3.0 if f == "AF_L" else 4.5
    ex = sorted(set(s for _, s in v))[:5]
    print("| %s | %s | %s | %.2f | %.1f | %s | %d | %s |" % (tokname(fg), tokname(bg), f, cr, th, "BELOW" if cr < th else "ok", len(v), ", ".join(ex)))
# ---------------- 3b non-text
T_ = M.ux
BG, SURF, MID, DIM, TEXT, RAISE, KEY = T_.bg, T_.surf, T_.mid, T_.dim, T_.text, T_.raise_, T_.key
CHc = mod.CH
from el import mix
rows = []
def add(name, a, b): rows.append((name, a, b))
for i, c in enumerate(CHc):
    add("channel %d colour on BG (header chip frame, music bar, hit strip)" % (i + 1), c, BG)
    add("channel %d colour on SURF (knob tile dots, keybed lane block)" % (i + 1), c, SURF)
    add("channel %d colour on PLAYHEAD column PHC" % (i + 1), c, mod.PHC)
    add("channel %d header chip digit INK on filled chip" % (i + 1), mod.INK, c)
add("hit strip dash (muted/off other channel) DIM on BG", DIM, BG)
add("hollow muted bar frame MID on BG", MID, BG)
add("hollow muted bar frame MID on PHC (playhead)", MID, mod.PHC)
add("hollow MIDI-OFF bar frame OFF_RED on BG", mod.OFF_RED, BG)
add("hollow MIDI-OFF bar frame OFF_RED on PHC", mod.OFF_RED, mod.PHC)
add("rest tick DIM (3x1) on BG", DIM, BG)
add("OFF_RED cap on chip vs CH2 chip fill", mod.OFF_RED, CHc[1])
add("OFF_RED cap on chip vs BG", mod.OFF_RED, BG)
add("OFF_RED header chip vs BG", mod.OFF_RED, BG)
add("OFF_RED cap vs CH1 / CH3 / CH4 chip (min over)", mod.OFF_RED, min(CHc, key=lambda c: contrast_ratio(mod.OFF_RED, c)))
add("playhead column PHC vs BG", mod.PHC, BG)
add("playhead white bar vs BG", mod.WHITE, BG)
add("playhead white bar vs PHC", mod.WHITE, mod.PHC)
add("dice tile DICE_BG vs BG", mod.DICE_BG, BG)
add("plain tile SURF vs BG", SURF, BG)
add("dice tile DICE_BG vs plain tile SURF", mod.DICE_BG, SURF)
add("dice icon WHITE on DICE_BG", mod.WHITE, mod.DICE_BG)
add("plain icon TEXT on SURF", TEXT, SURF)
add("absent die DIM on DICE_BG", DIM, mod.DICE_BG)
add("lit button INK on WHITE", mod.INK, mod.WHITE)
add("lit button WHITE vs BG", mod.WHITE, BG)
add("REC icon T.REC on SURF", T_.rec, SURF)
add("REC armed icon vs BG", T_.rec, BG)
add("selection bar WHITE vs BG", mod.WHITE, BG)
add("top-bar message TEXT vs header BG", TEXT, BG)
add("top-bar toast WHITE vs header BG", mod.WHITE, BG)
add("top-bar name pill INK on TEXT bar", mod.INK, TEXT)
add("hold progress bar INK on TEXT", mod.INK, TEXT)
add("hold progress OFF_RED (>=100%) on TEXT", mod.OFF_RED, TEXT)
add("hold progress track TEXT vs BG", TEXT, BG)
add("row marker active (CH1) vs inactive MID frame (on BG)", CHc[0], MID)
add("row marker inactive MID on BG", MID, BG)
add("row marker active CH2 on BG", CHc[1], BG)
add("page dots active TEXT vs inactive MID", TEXT, MID)
add("page dots inactive MID on BG", MID, BG)
for i, c in enumerate(CHc):
    fc = mix(c, 120)
    add("knob fill (mix %d/255) vs empty SURF, ch%d" % (120, i + 1), fc, SURF)
    add("knob level edge ch%d full vs fill" % (i + 1), c, fc)
add("knob fill WHITE(REC) vs empty", mix(mod.WHITE, 120), SURF)
add("knob level edge WHITE (REC) vs its fill", mod.WHITE, mix(mod.WHITE, 120))
add("turning frame/ghost WHITE vs fill (ch1)", mod.WHITE, mix(CHc[0], 120))
add("position dot current ch1 vs inactive DIM frame", CHc[0], DIM)
add("position dot inactive DIM on SURF", DIM, SURF)
add("knob tile SURF vs BG", SURF, BG)
add("keybed white key SURF vs BG", SURF, BG)
add("black key RAISE vs BG", RAISE, BG)
add("black key RAISE vs white key SURF", RAISE, SURF)
add("REC white key KEY vs BG", KEY, BG)
add("REC black key RAISE vs KEY", RAISE, KEY)
add("slide bridge ch1 vs BG", CHc[0], BG)
add("accent cap WHITE vs BG", mod.WHITE, BG)
add("diced bar WHITE vs hatch ch1", mod.WHITE, CHc[0])
add("muted chip: MID digit on SURF", MID, SURF)
add("muted chip diagonal MID on SURF", MID, SURF)
add("muted chip SURF vs BG", SURF, BG)
add("muted x7 glyph MID on RAISE", MID, RAISE)
add("pending pill INK on WHITE", mod.INK, mod.WHITE)
add("pending pill WHITE vs BG", mod.WHITE, BG)
print("\n# 3b meaning-bearing non-text pairs (flag <3:1)")
print("| item | fg | bg | ratio | <3:1 |"); print("|---|---|---|---|---|")
for name, a, b in rows:
    cr = contrast_ratio(a, b)
    print("| %s | %s | %s | %.2f | %s |" % (name, tokname(a), tokname(b), cr, "**FLAG**" if cr < 3 else ""))
# ---------------- 3c CVD
def srgb2lin(v): v = v / 255.0; return np.where(v <= 0.04045, v / 12.92, ((v + 0.055) / 1.055) ** 2.4)
def lin2srgb(v): v = np.clip(v, 0, 1); return np.where(v <= 0.0031308, v * 12.92, 1.055 * v ** (1 / 2.4) - 0.055) * 255
MACH = {"deuteranopia": np.array([[0.367322, 0.860646, -0.227968], [0.280085, 0.672501, 0.047413], [-0.011820, 0.042940, 0.968881]]),
        "protanopia": np.array([[0.152286, 1.052583, -0.204868], [0.114503, 0.786281, 0.099216], [-0.003882, -0.048116, 1.051998]])}
def lab(rgb8):
    l = srgb2lin(np.array(rgb8, float)); X = np.array([[0.4124564, 0.3575761, 0.1804375], [0.2126729, 0.7151522, 0.0721750], [0.0193339, 0.1191920, 0.9503041]]) @ l
    X = X / np.array([0.95047, 1, 1.08883]); f = np.where(X > 216 / 24389, np.cbrt(X), (24389 / 27 * X + 16) / 116)
    return np.array([116 * f[1] - 16, 500 * (f[0] - f[1]), 200 * (f[1] - f[2])])
def de76(a, b): return float(np.linalg.norm(a - b))
def de2000(l1, l2):
    L1, a1, b1 = l1; L2, a2, b2 = l2
    C1, C2 = math.hypot(a1, b1), math.hypot(a2, b2); Cb = (C1 + C2) / 2
    G = 0.5 * (1 - math.sqrt(Cb ** 7 / (Cb ** 7 + 25 ** 7))); a1p, a2p = (1 + G) * a1, (1 + G) * a2
    C1p, C2p = math.hypot(a1p, b1), math.hypot(a2p, b2)
    h1 = math.degrees(math.atan2(b1, a1p)) % 360 if C1p else 0; h2 = math.degrees(math.atan2(b2, a2p)) % 360 if C2p else 0
    dL = L2 - L1; dC = C2p - C1p
    dh = 0 if C1p * C2p == 0 else (h2 - h1 + 180) % 360 - 180
    dH = 2 * math.sqrt(C1p * C2p) * math.sin(math.radians(dh) / 2)
    Lb = (L1 + L2) / 2; Cbp = (C1p + C2p) / 2
    if C1p * C2p == 0: hb = h1 + h2
    elif abs(h1 - h2) <= 180: hb = (h1 + h2) / 2
    else: hb = (h1 + h2 + 360) / 2 if h1 + h2 < 360 else (h1 + h2 - 360) / 2
    T = 1 - 0.17 * math.cos(math.radians(hb - 30)) + 0.24 * math.cos(math.radians(2 * hb)) + 0.32 * math.cos(math.radians(3 * hb + 6)) - 0.20 * math.cos(math.radians(4 * hb - 63))
    dth = 30 * math.exp(-(((hb - 275) / 25) ** 2)); Rc = 2 * math.sqrt(Cbp ** 7 / (Cbp ** 7 + 25 ** 7))
    Sl = 1 + 0.015 * (Lb - 50) ** 2 / math.sqrt(20 + (Lb - 50) ** 2); Sc = 1 + 0.045 * Cbp; Sh = 1 + 0.015 * Cbp * T; Rt = -math.sin(math.radians(2 * dth)) * Rc
    return math.sqrt((dL / Sl) ** 2 + (dC / Sc) ** 2 + (dH / Sh) ** 2 + Rt * (dC / Sc) * (dH / Sh))
def sim(rgb8, kind):
    if kind == "normal": return np.array(rgb8, float)
    return lin2srgb(MACH[kind] @ srgb2lin(np.array(rgb8, float)))
print("\n# 3c CVD: Machado et al. 2009, severity 1.0, applied in linear sRGB; Lab D65; deltaE76 and CIEDE2000 (JND ~2.3)")
cols = {"CH%d" % (i + 1): rgb(c) for i, c in enumerate(CHc)}
for kind in ("normal", "deuteranopia", "protanopia"):
    print("\n## %s" % kind)
    print("simulated: " + ", ".join("%s %s" % (k, "#%02x%02x%02x" % tuple(int(round(x)) for x in sim(v, kind))) for k, v in cols.items()))
    print("| pair | dE76 | dE2000 | lightness L* a / b |"); print("|---|---|---|---|")
    for (ka, va), (kb, vb) in itertools.combinations(cols.items(), 2):
        la, lb = lab(sim(va, kind)), lab(sim(vb, kind))
        print("| %s-%s | %.1f | %.1f | %.0f / %.0f |" % (ka, kb, de76(la, lb), de2000(la, lb), la[0], lb[0]))
