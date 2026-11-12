import sys; sys.path.insert(0, "/home/claude/fm1/docs/design/ui_review/critique_v7/scripts")
from lib import *
import collections, math
from PIL import Image, ImageChops
out = []; P = out.append
CH = el.CH_COL; WHITE = el.WHITE; INK = el.INK; PH = el.PH; DICE_BG = el.DICE_BG; AMBER = rgb565(255, 205, 90)
cr = contrast_ratio

def under_colours(sc_nt, ink):
    a, b, c, d = ink
    s = set()
    for y in range(max(0, b), min(240, d)):
        for x in range(max(0, a), min(240, c)):
            s.add(sc_nt.px[y * 240 + x])
    return s

def run(vname):
    info = variant(vname)
    P("")
    P("# VARIANT %s : palette=%s mono_flag=%s DIM=%s LINE=%s RAISE=%s MID=%s KEY=%s REC=%s TINT=%s" % ((vname,) + info + (hexc(T.KEY), hexc(T.REC), hexc(T.TINT))))
    P("## 3a. text fg/bg pairs actually drawn (WCAG ratio; threshold 4.5, AF_L 3.0), visible texts only, all six scenes")
    pairs = collections.OrderedDict()
    under_flags = []
    for sn in SCENES:
        r = capture(S()[sn]); nt = capture(S()[sn], skip_text=True)["screen"]
        for t in r["texts"]:
            if t["occluded"]: continue
            fgc, bgc = t["fg"], t["bg"]
            key = (t["f"], fgc, bgc)
            e = pairs.setdefault(key, dict(texts=set(), scenes=set()))
            e["texts"].add(t["s"]); e["scenes"].add(sn)
            if t["ink"]:
                uc = under_colours(nt, t["ink"])
                worst = min(uc, key=lambda u: cr(fgc, u))
                if worst != bgc and cr(fgc, worst) < cr(fgc, bgc) - 1e-9:
                    under_flags.append((sn, t["s"], t["f"], hexc(fgc), hexc(bgc), hexc(worst), cr(fgc, worst), len(uc)))
    P("| font | fg | bg | ratio | threshold | flag | texts | scenes |")
    P("|---|---|---|---|---|---|---|---|")
    nflag = 0
    for (f, fgc, bgc), e in sorted(pairs.items(), key=lambda kv: cr(kv[0][1], kv[0][2])):
        ratio = cr(fgc, bgc); th = 3.0 if f == "AF_L" else 4.5
        fl = "BELOW" if ratio < th else "ok"
        nflag += fl == "BELOW"
        P("| %s | %s %s | %s %s | %.2f | %.1f | %s | %s | %s |" % (f, hexc(fgc), tokname(fgc), hexc(bgc), tokname(bgc), ratio, th, fl, sorted(e["texts"]), sorted(e["scenes"])))
    P("distinct text pairs=%d, below threshold=%d" % (len(pairs), nflag))
    P("")
    P("text whose ink box covers a pixel colour that differs from the declared bg and gives a lower ratio (no-text render underneath):")
    seen = set()
    for u in under_flags:
        k = u[1:]
        if k in seen: continue
        seen.add(k)
        P("- scene %s text %r %s fg %s declared bg %s worst underlying %s ratio %.2f (distinct bg colours under ink: %d)" % u)
    if not under_flags: P("- none")

    P("")
    P("## 3b. meaning-bearing non-text pairs (WCAG ratio; flag < 3:1)")
    rows = []
    def add(name, a, b): rows.append((name, a, b))
    for i in range(4):
        add("CH%d vs BG (chip frame, bars, algo glyph)" % (i + 1), CH[i], T.BG)
        add("CH%d vs RAISE (knob tile / unlit key / button)" % (i + 1), CH[i], T.RAISE)
        add("CH%d vs LINE (glyph on black-key body)" % (i + 1), CH[i], T.LINE)
        add("CH%d vs PH (bar on playhead column)" % (i + 1), CH[i], PH)
        add("CH%d knob-fill mix(CH,110) vs RAISE (meter fill vs empty)" % (i + 1), el.mix(CH[i], 110), T.RAISE)
        add("CH%d vs WHITE (diced bar white base / playhead outline on lit key)" % (i + 1), CH[i], WHITE)
        add("CH%d vs T.MID (row-marker filled vs outline colour)" % (i + 1), CH[i], T.MID)
        add("CH%d vs T.DIM" % (i + 1), CH[i], T.DIM)
    add("T.DIM (muted bar outline) vs BG", T.DIM, T.BG)
    add("T.DIM (muted bar outline) vs PH (muted bar on playhead col)", T.DIM, PH)
    add("T.DIM (rest dash, 3x1 px) vs BG", T.DIM, T.BG)
    add("T.DIM (rest dash) vs PH", T.DIM, PH)
    add("T.MID (row-marker outline, page tick marks) vs BG", T.MID, T.BG)
    add("T.MID (muted-channel key glyph frame) vs T.LINE (black-key body)", T.MID, T.LINE)
    add("T.LINE (black-key body) vs BG", T.LINE, T.BG)
    add("T.RAISE (white-key body / knob tile / button) vs BG", T.RAISE, T.BG)
    add("T.KEY (white-key body in REC) vs BG", T.KEY, T.BG)
    add("T.DIM (black-key body in REC) vs BG", T.DIM, T.BG)
    add("PH (playhead column) vs BG", PH, T.BG)
    add("DICE icon amber (255,205,90) vs DICE_BG", AMBER, DICE_BG)
    add("DICE_BG tint vs BG (button group tint)", DICE_BG, T.BG)
    add("DICE_BG tint vs RAISE (dice vs non-dice button cell)", DICE_BG, T.RAISE)
    add("TEXT icon vs RAISE (non-dice button icon)", T.TEXT, T.RAISE)
    add("INK icon vs WHITE (lit button)", INK, WHITE)
    add("WHITE playhead outline vs RAISE (unlit key)", WHITE, T.RAISE)
    add("WHITE playhead outline vs BG", WHITE, T.BG)
    add("WHITE accent cap (2 px) vs BG", WHITE, T.BG)
    add("WHITE accent cap vs PH", WHITE, PH)
    add("lit-key front CH1 vs unlit key RAISE (step on/off)", CH[0], T.RAISE)
    add("T.LINE (turning-box scale track) vs BG", T.LINE, T.BG)
    add("T.MID (turning-box ticks) vs BG", T.MID, T.BG)
    add("pending-preset frame WHITE vs BG", WHITE, T.BG)
    add("REC button fg T.REC vs RAISE (rec armed)", T.REC, T.RAISE)
    add("WHITE playhead outline vs T.KEY (white key body in REC scene; outline still drawn when playing)", WHITE, T.KEY)
    add("T.DIM black-key body vs T.KEY white key (REC)", T.DIM, T.KEY)
    add("REC key-caption icon CH1 vs BG", CH[0], T.BG)
    add("TEXT vs T.LINE (S/F/R letters on black key)", T.TEXT, T.LINE)
    P("| pair | A | B | ratio | flag |")
    P("|---|---|---|---|---|")
    nb = 0
    for n, a, b in rows:
        ratio = cr(a, b); fl = "BELOW 3:1" if ratio < 3 else "ok"
        nb += ratio < 3
        P("| %s | %s | %s | %.2f | %s |" % (n, hexc(a), hexc(b), ratio, fl))
    P("pairs=%d below 3:1=%d" % (len(rows), nb))
    # hatch pixel facts
    r = capture(S()["dice"])
    hs = [o for o in r["ops"] if o["k"] == "hatch"]
    if hs:
        o = hs[0]; px = r["screen"].px
        tot = o["w"] * o["h"]; cnt = collections.Counter(px[(o["y"] + j) * 240 + o["x"] + i] for j in range(o["h"]) for i in range(o["w"]))
        P("hatched diced bar sample (%dx%d at %d,%d): pixel colours %s of %d" % (o["w"], o["h"], o["x"], o["y"], {hexc(k) + " " + tokname(k): v for k, v in cnt.items()}, tot))
        P("hatch ops in dice scene: %d; hatch step values: %s" % (len(hs), sorted(set(o["step"] for o in hs))))
    return

def lab(rgb):
    def f(t): return t ** (1 / 3) if t > 0.008856 else 7.787 * t + 16 / 116
    r, g, b = [lin(c) for c in rgb]
    X = 0.4124 * r + 0.3576 * g + 0.1805 * b; Y = 0.2126 * r + 0.7152 * g + 0.0722 * b; Z = 0.0193 * r + 0.1192 * g + 0.9505 * b
    fx, fy, fz = f(X / 0.95047), f(Y), f(Z / 1.08883)
    return (116 * fy - 16, 500 * (fx - fy), 200 * (fy - fz))
def lin(v):
    v /= 255.0
    return v / 12.92 if v <= 0.04045 else ((v + 0.055) / 1.055) ** 2.4
def enc(l):
    l = max(0.0, min(1.0, l)); return 255 * (12.92 * l if l <= 0.0031308 else 1.055 * l ** (1 / 2.4) - 0.055)
M = ((0.367322, 0.860646, -0.227968), (0.280085, 0.672501, 0.047413), (-0.011820, 0.042940, 0.968881))
def deut(rgb):
    l = [lin(c) for c in rgb]
    o = [max(0, min(1, sum(M[i][k] * l[k] for k in range(3)))) for i in range(3)]
    return tuple(enc(x) for x in o)
def rl(rgb):
    r, g, b = [lin(c) for c in rgb]; return 0.2126 * r + 0.7152 * g + 0.0722 * b
def ratio_rgb(a, b):
    la, lb = rl(a), rl(b); hi, lo = max(la, lb), min(la, lb); return (hi + .05) / (lo + .05)

if __name__ == "__main__":
    P("# 3. Contrast (evidence)")
    P("Note: variant 'mock' is what mock.py renders for palette 'MONO' (its palette regex skips the HI-CON row, so MONO is index 8, not UI_BW_INDEX 9, and the BW special case in Theme.set is not applied). Variant 'fw' re-parses ui_palettes.h including HI-CON so MONO is index 9 (mid/sel/key/rec = text; dim/line/raise/lane = UI_BW_GREY 0x528a; tint/grid = bg). out/*.png were produced by 'mock'.")
    for v in ("mock", "fw"):
        run(v)
    variant("mock")
    P("")
    P("## 3c. deuteranopia simulation of the four channel colours")
    P("Method: expand RGB565 to 8-bit as mock.to_rgb888, sRGB->linear, apply Machado, Oliveira & Fernandes (2009) deuteranopia matrix at severity 1.0 [[0.367322,0.860646,-0.227968],[0.280085,0.672501,0.047413],[-0.011820,0.042940,0.968881]] in linear RGB, clip 0..1, back to sRGB. Luminance = WCAG relative luminance (0.2126R+0.7152G+0.0722B, linear). deltaE = CIE76 on the simulated colours (D65).")
    cols = [tuple(c // 1 for c in to_rgb888(c)) for c in CH]
    sims = [deut(c) for c in cols]
    P("| ch | RGB565->RGB888 | normal rel. luminance | simulated RGB | simulated rel. luminance |")
    P("|---|---|---|---|---|")
    for i in range(4):
        P("| CH%d | %s %s | %.4f | (%d,%d,%d) | %.4f |" % (i + 1, cols[i], hexc(CH[i]), rl(cols[i]), *[round(x) for x in sims[i]], rl(sims[i])))
    P("| pair | normal: dL (rel lum) | normal: WCAG ratio | deut: dL | deut: WCAG ratio | deut: deltaE76 | normal: deltaE76 |")
    P("|---|---|---|---|---|---|---|")
    for i in range(4):
        for j in range(i + 1, 4):
            dE = lambda a, b: math.dist(lab(a), lab(b))
            P("| CH%d-CH%d | %.4f | %.2f | %.4f | %.2f | %.1f | %.1f |" % (i + 1, j + 1, abs(rl(cols[i]) - rl(cols[j])), ratio_rgb(cols[i], cols[j]), abs(rl(sims[i]) - rl(sims[j])), ratio_rgb(sims[i], sims[j]), dE(sims[i], sims[j]), dE(cols[i], cols[j])))
    P("")
    P("Simulated channel colours vs black BG (WCAG): " + ", ".join("CH%d %.2f" % (i + 1, ratio_rgb(sims[i], (0, 0, 0))) for i in range(4)))
    open("/home/claude/fm1/docs/design/ui_review/critique_v7/scripts/s2_out.md", "w").write("\n".join(out))
    print("\n".join(out))
