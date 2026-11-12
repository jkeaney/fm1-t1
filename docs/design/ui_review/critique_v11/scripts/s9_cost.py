from lib import *
import math, re, contextlib, io
v = mod
orig = el.P.art
used = {}
def art2(self, x, y, a, c):
    used.setdefault(a, set()).add(c); return orig(self, x, y, a, c)
el.P.art = art2
cols = set(); nontext = set()
for n in mod.STATE["scenes"]:
    with contextlib.redirect_stdout(io.StringIO()):
        sc, cv, rec, arts = render(scene(n))
        nt = render_notext(scene(n))
    cols |= set(sc.px); nontext |= set(nt.px)
names = {}
for k, a in list(icons_v11.ICONS.items()) + list(el.IC.items()):
    names.setdefault(a, []).append(k)
tot1 = tot8 = 0; rows = []
for a in used:
    L = a.strip("\n").split("\n"); w = max(map(len, L)); h = len(L)
    b1 = math.ceil(w / 8) * h; tot1 += b1; tot8 += w * h
    rows.append((sorted(names.get(a, ["?"]))[0], "%dx%d" % (w, h), b1))
print("distinct icon bitmaps drawn across all 29 scenes:", len(used), "| bytes 1bpp (row-padded):", tot1, "| bytes 8bpp:", tot8)
print(sorted(rows))
print("distinct bitmaps defined in ICONS+IC (incl. unused):", len(set(list(icons_v11.ICONS.values()) + list(el.IC.values()))))
toks = {}
for t in ["BG", "SURF", "MID", "DIM", "TEXT", "RAISE", "KEY", "REC", "ACCENT"]:
    toks[t] = getattr(T, t)
tokset = set(toks.values())
print("HI-CON tokens:", {k: hexc(x) for k, x in toks.items()})
lit = {"CH1": v.CH[0], "CH2": v.CH[1], "CH3": v.CH[2], "CH4": v.CH[3], "WHITE": v.WHITE, "INK": v.INK, "PHC": v.PHC, "DICE_BG": v.DICE_BG, "OFF_RED": v.OFF_RED}
for i, c in enumerate(v.CH): lit["mix(CH%d,120)" % (i + 1)] = el.mix(c, 120)
print("colours defined in v11.py vs HI-CON tokens:")
for k, c in lit.items(): print("  %-14s %s %s" % (k, hexc(c), "= a token" if c in tokset else "OUTSIDE tokens"))
print("distinct RGB565 values over 29 scenes incl. text ramps:", len(cols), "| in no-text renders:", len(nontext), "| of those not a HI-CON token:", len(nontext - tokset))
print(sorted(hexc(c) for c in nontext - tokset))
