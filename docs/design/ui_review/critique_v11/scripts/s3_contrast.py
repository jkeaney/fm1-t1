from lib import *
import numpy as np, itertools
from skimage import color as skc
v = mod
CH = v.CH
def cr(a, b): return contrast_ratio(a, b)
print("## 3a text pairs (worst-case contrast vs every distinct background pixel under the ink bbox, from a no-text render)")
print("| scene | string | font | fg | bg passed | worst bg under ink | ratio(passed) | ratio(worst) | flag |\n|--|--|--|--|--|--|--|--|--|")
seen = {}
hidden = []
for n in v.STATE["scenes"]:
    if n.startswith("sheet"): continue
    st = scene(n)
    sc, tx, arts = texts_with_bbox(st)
    nt = render_notext(st)
    vis = visible_flags(st, sc, tx)
    for r, vv in zip(tx, vis):
        x0, y0, x1, y1 = r["bbox"]
        pts = [(x, y) for y in range(y0, y1) for x in range(x0, x1) if 0 <= x < 240 and 0 <= y < 240]
        if not vv:
            hidden.append((n, r["s"])); continue          # painted over by a later element (e.g. header text under the top-bar message)
        bgs = set(nt.px[y * 240 + x] for x, y in pts)
        worst = min(bgs, key=lambda b: cr(r["fg"], b))
        lim = 3.0 if r["font"] == "AF_L" else 4.5
        rp, rw = cr(r["fg"], r["bg"]), cr(r["fg"], worst)
        key = (r["s"], r["font"], r["fg"], r["bg"], worst)
        if key in seen: seen[key][0].append(n); continue
        seen[key] = ([n], r["s"], r["font"], hexc(r["fg"]), hexc(r["bg"]), hexc(worst), rp, rw, lim, len(bgs))
nflag = 0
for k, (ns, s, f, fg, bg, wb, rp, rw, lim, nb) in seen.items():
    flag = "FLAG(<%.1f)" % lim if min(rp, rw) < lim else ""
    if flag or rp != rw:
        if flag: nflag += 1
        print("| %s (x%d) | %r | %s | %s | %s | %s | %.2f | %.2f | %s |" % (ns[0], len(ns), s, f, fg, bg, wb, rp, rw, flag))
print("texts drawn but painted over by a later element (not counted):", len(hidden), sorted(set(hidden))[:40])
print("distinct (string,font,fg,bg,worst) combos:", len(seen), " flagged:", nflag)
allp = sorted(set((hexc(k[2]), hexc(k[3]), k[1]) for k in seen), key=lambda t: cr(rgb565_from(t[0]) if False else 0, 0))
mn = min(seen.values(), key=lambda t: min(t[6], t[7]))
print("lowest text ratio:", mn[1], mn[2], "%.2f/%.2f" % (mn[6], mn[7]), mn[3], "on", mn[4], mn[5])

print("\n## 3b meaning-bearing non-text pairs (WCAG ratio; flag <3:1)")
MIX = el.mix
col_names = ["CH1 amber", "CH2 sky", "CH3 green", "CH4 pink"]
pairs = []
def P(name, a, b): pairs.append((name, a, b))
for i, c in enumerate(CH):
    P("%s on BG (strip/bar/chip frame)" % col_names[i], c, T.BG)
    P("%s on SURF (tile text / chip? / dots)" % col_names[i], c, T.SURF)
    P("%s bar on playhead column PHC" % col_names[i], c, v.PHC)
    P("%s knob fill mix120 vs SURF empty" % col_names[i], MIX(c, 120), T.SURF)
    P("%s level edge (full) vs its fill mix120" % col_names[i], c, MIX(c, 120))
    P("%s level edge vs SURF (edge above fill)" % col_names[i], c, T.SURF)
    P("%s live row box fill vs BG" % col_names[i], c, T.BG)
    P("%s keybed step key colour vs SURF key" % col_names[i], c, T.SURF)
    P("%s blk-key mute square vs RAISE" % col_names[i], c, T.RAISE)
    P("%s selected chip fill vs white selection bar" % col_names[i], c, v.WHITE)
    P("%s vs OFF_RED cap (MIDI OFF cap on chip)" % col_names[i], c, v.OFF_RED)
    P("%s hatch on white (dice step: CH over WHITE)" % col_names[i], c, v.WHITE)
P("hollow muted bars MID on BG", T.MID, T.BG)
P("hollow MIDI-OFF bars OFF_RED on BG", v.OFF_RED, T.BG)
P("muted/off strip dash DIM on BG", T.DIM, T.BG)
P("empty-step tick DIM on BG", T.DIM, T.BG)
P("muted chip SURF on BG", T.SURF, T.BG)
P("muted chip diagonal MID on SURF", T.MID, T.SURF)
P("OFF_RED chip (MIDI OFF) vs BG", v.OFF_RED, T.BG)
P("playhead column PHC vs BG", v.PHC, T.BG)
P("playhead white bar vs BG", v.WHITE, T.BG)
P("playhead white bar vs PHC", v.WHITE, v.PHC)
P("dice tile DICE_BG vs BG", v.DICE_BG, T.BG)
P("plain tile SURF vs BG", T.SURF, T.BG)
P("dice tile DICE_BG vs plain tile SURF", v.DICE_BG, T.SURF)
P("dice glyph WHITE on DICE_BG", v.WHITE, v.DICE_BG)
P("dice glyph DIM (absent layer) on DICE_BG", T.DIM, v.DICE_BG)
P("plain glyph TEXT on SURF", T.TEXT, T.SURF)
P("knob tile SURF vs BG", T.SURF, T.BG)
P("empty list dots frame DIM on SURF", T.DIM, T.SURF)
P("numbered row box frame MID on BG", T.MID, T.BG)
P("numbered row digit INK on live fill: see 3a", T.BG, T.BG)
P("selection bar WHITE vs BG", v.WHITE, T.BG)
P("message panel WHITE vs header BG", v.WHITE, T.BG)
P("message panel TEXT vs header BG", T.TEXT, T.BG)
P("toast/hold panel INK glyph on TEXT", v.INK, T.TEXT)
P("hold progress bar INK on TEXT panel", v.INK, T.TEXT)
P("enc name chip INK vs TEXT panel", v.INK, T.TEXT)
P("REC cursor frame WHITE vs BG", v.WHITE, T.BG)
P("REC cursor frame WHITE vs PHC column", v.WHITE, v.PHC)
P("lit CHN key WHITE vs RAISE (unlit)", v.WHITE, T.RAISE)
P("lit button WHITE vs SURF", v.WHITE, T.SURF)
P("lit button WHITE vs DICE_BG", v.WHITE, v.DICE_BG)
P("black key RAISE vs BG", T.RAISE, T.BG)
P("white key SURF vs BG", T.SURF, T.BG)
P("REC keys KEY vs BG", T.KEY, T.BG)
P("REC red T.REC vs BG", T.REC, T.BG)
P("REC red disc (header) vs BG", T.REC, T.BG)
P("accent cap WHITE vs CH (cap sits over BG)", v.WHITE, T.BG)
P("slide bridge CH vs BG (=CH on BG)", CH[0], T.BG)
P("diced step WHITE vs CH hatch (alternating)", v.WHITE, CH[0])
P("knob turning frame col vs BG", CH[0], T.BG)
P("pending preset WHITE chip vs BG", v.WHITE, T.BG)
nfl = 0
print("| pair | ratio | flag |\n|--|--|--|")
for name, a, b in pairs:
    if name.endswith("see 3a"): continue
    r = cr(a, b)
    f = "FLAG <3" if r < 3 else ""
    nfl += bool(f)
    print("| %s (%s / %s) | %.2f | %s |" % (name, hexc(a), hexc(b), r, f))
print("pairs", len(pairs) - 1, "flagged", nfl)

print("\n## 3c colour-vision simulation of the 4 channel colours")
M = {"deuteranopia": [[0.367322, 0.860646, -0.227968], [0.280085, 0.672501, 0.047413], [-0.011820, 0.042940, 0.968881]],
     "protanopia": [[0.152286, 1.052583, -0.204868], [0.114503, 0.786281, 0.099216], [-0.003882, -0.048116, 1.051998]],
     "tritanopia": [[1.255528, -0.076749, -0.178779], [-0.078411, 0.930809, 0.147602], [0.004733, 0.691367, 0.303900]]}
print("method: Machado, Oliveira & Fernandes 2009 matrices, severity 1.0, applied to linear sRGB of the 565-expanded colours; deltaE = CIEDE2000 (skimage) and CIE76 in brackets; Lab D65")
def lin3(c): return np.array([lin(x) for x in to_rgb888(c)])
def lin(x):
    x /= 255.0; return x / 12.92 if x <= 0.04045 else ((x + 0.055) / 1.055) ** 2.4
def gam(x):
    x = np.clip(x, 0, 1); return np.where(x <= 0.0031308, 12.92 * x, 1.055 * x ** (1 / 2.4) - 0.055)
def lab(rgb_lin): return skc.rgb2lab(gam(rgb_lin).reshape(1, 1, 3))[0, 0]
for mode in ["normal"] + list(M):
    labs = []
    for c in CH:
        l = lin3(c)
        if mode != "normal": l = np.array(M[mode]) @ l
        labs.append(lab(l))
    print("\n%s:" % mode, ", ".join("%s sim %s" % (col_names[i], "#%02x%02x%02x" % tuple(int(round(x * 255)) for x in gam(labs and (np.array(M[mode]) @ lin3(CH[i]) if mode != 'normal' else lin3(CH[i])))) ) for i in range(4)))
    for i, j in itertools.combinations(range(4), 2):
        d2000 = skc.deltaE_ciede2000(labs[i].reshape(1, 1, 3), labs[j].reshape(1, 1, 3))[0, 0]
        d76 = np.linalg.norm(labs[i] - labs[j])
        print("  CH%d-CH%d: dE2000 %.1f [dE76 %.1f] %s" % (i + 1, j + 1, d2000, d76, "FLAG <20 (2000)" if d2000 < 20 else ""))
# also luminance contrast between channel colours (value difference) for context
print("\nluminance ratio between channel colours (normal vision):")
for i, j in itertools.combinations(range(4), 2): print("  CH%d-CH%d %.2f" % (i + 1, j + 1, cr(CH[i], CH[j])))
