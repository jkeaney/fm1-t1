import sys, collections; sys.path.insert(0, "/home/claude/fm1/docs/design/ui_review/critique_v7/scripts")
from lib import *
import icons_v7 as IV
out = []; P = out.append
variant("mock")
P("# 8. Firmware-cost facts")
P("## 8a. bitmaps")
def dims(a):
    rows = a.strip("\n").split("\n"); return max(len(r) for r in rows), len(rows)
tot_pad = tot_bits = 0
for n, a in IV.ICONS.items():
    w, h = dims(a); tot_pad += h * ((w + 7) // 8); tot_bits += w * h
P("icons_v7.ICONS: %d bitmaps (6 dice 13x13, 7 others 13x11); bytes at 1 bpp row-padded to byte = %d; packed bits = %d (= %d bytes)" % (len(IV.ICONS), tot_pad, tot_bits, (tot_bits + 7) // 8))
w6 = [dims(a) for a in el.ALGO_IC.values()]
P("el.ALGO_IC (header algorithm glyphs): %d bitmaps, sizes %s, row-padded bytes = %d" % (len(el.ALGO_IC), {k: "%dx%d" % dims(v) for k, v in el.ALGO_IC.items()}, sum(h * ((w + 7) // 8) for w, h in w6)))
used_ic = {"play", "rec", "note", "left", "right", "rst"}
P("el.IC bitmaps drawn by the v7 code paths in the six scenes: %s sizes %s, row-padded bytes = %d" % (sorted(used_ic), {k: "%dx%d" % dims(el.IC[k]) for k in sorted(used_ic)}, sum(dims(el.IC[k])[1] * ((dims(el.IC[k])[0] + 7) // 8) for k in used_ic)))
used = set()
for sn in SCENES:
    for o in capture(S()[sn])["ops"]:
        if o["k"] == "art": used.add(o["art"])
pad = sum(dims(a)[1] * ((dims(a)[0] + 7) // 8) for a in used)
bits = sum(dims(a)[0] * dims(a)[1] for a in used)
P("distinct bitmaps actually drawn across the six scenes: %d; row-padded bytes = %d; packed bits = %d (%d bytes)" % (len(used), pad, bits, (bits + 7) // 8))
P("bitmap draws per scene (art ops): " + ", ".join("%s=%d" % (sn, sum(1 for o in capture(S()[sn])["ops"] if o["k"] == "art")) for sn in SCENES))
# overlap: header glyph bitmaps duplicating button icons?
P("duplicates by shape: IC.play (11x11) vs icons_v7.play (13x11) and IC.rec (11x11) vs icons_v7.rec (13x11) are separate bitmaps; both pairs drawn in the same scene")
P("")
P("## 8b. colours that are not palette tokens (RGB565)")
cols = collections.OrderedDict()
def reg(name, c): cols.setdefault(c, []).append(name)
for i, c in enumerate(el.CH_COL): reg("CH_COL[%d] (channel %d)" % (i, i + 1), c)
reg("WHITE rgb565(250,250,250)", el.WHITE); reg("INK rgb565(0,0,0)", el.INK); reg("PH rgb565(70,70,70) playhead column", el.PH)
reg("DICE_BG rgb565(48,40,16)", el.DICE_BG); reg("DICE amber rgb565(255,205,90) icon fg", rgb565(255, 205, 90))
for i, c in enumerate(el.CH_COL): reg("knob fill mix(CH_COL[%d],110)" % i, el.mix(c, 110))
def tokens_for(pal):
    variant("fw" if pal == "MONO-fw" else "mock")
    mock.palette_set("GREY" if pal == "GREY" else "MONO")
    u = mock.ux
    return {n: getattr(u, n) for n in ("bg", "surf", "text", "theme", "accent", "mid", "dim", "line", "sel", "tint", "ink", "rec", "raise_", "key", "lane", "grid")}
P("| RGB565 | RGB888 | role in v7 | equals a palette token in GREY (default) | MONO as mock parses | MONO per ui_palettes.h (idx 9) |")
P("|---|---|---|---|---|---|")
tg, tm, tf = tokens_for("GREY"), tokens_for("MONO"), tokens_for("MONO-fw")
eq = lambda c, t: ",".join(k for k, v in t.items() if v == c) or "no"
for c, names in cols.items():
    P("| 0x%04x | %s | %s | %s | %s | %s |" % (c, hexc(c), "; ".join(names), eq(c, tg), eq(c, tm), eq(c, tf)))
newc = [c for c in cols if eq(c, tg) == "no"]
P("distinct non-token colour values (vs GREY tokens): %d; of them chromatic (max-min channel > 6): %d" % (len(newc), sum(1 for c in newc if max(to_rgb888(c)) - min(to_rgb888(c)) > 6)))
P("colour constants defined in el.py/icons_v7.py/v7.py and not taken from T: CH_COL x4, WHITE, INK, PH, DICE_BG, amber, plus 4 derived mixes = %d values" % len(cols))
P("")
P("## 8c. GREY palette (firmware default, mono=1): coloured text")
src = open(UI + "/mock/mock.py").read()
import re
m = re.search(r"def _ramp\(fg, bg\):.*?return v\n", src, re.S)
P("mock._ramp mono branch (gfx.c ramp mirrored): `if ux.mono: x = bg >> 11; v.append(ux.gray(x + ((((fg >> 11) - x) * w + 128) >> 8)))` : only the red 5-bit channel of fg and bg is used, output is gray(x5) (r=g=b)")
variant("mock"); mock.palette_set("GREY")
P("GREY tokens: bg %s surf %s text %s mid %s dim %s line %s raise %s; mono flag=%s" % (hexc(mock.ux.bg), hexc(mock.ux.surf), hexc(mock.ux.text), hexc(mock.ux.mid), hexc(mock.ux.dim), hexc(mock.ux.line), hexc(mock.ux.raise_), mock.ux.mono))
P("| colour text | fg RGB | red5 used | GREY ramp peak (nibble 15) | peak vs bg pixel contrast | in v7 as |")
P("|---|---|---|---|---|---|")
for i, c in enumerate(el.CH_COL):
    ramp = mock.ramp(c, mock.ux.bg)
    peak = ramp[15]
    P("| CH%d | %s | %d/31 | %s (r=g=b) | %.2f | chip digit / algo name / turning title on BG |" % (i + 1, hexc(c), c >> 11, hexc(peak), contrast_ratio(peak, mock.ux.bg)))
for nm_, c in (("WHITE", el.WHITE), ("T.TEXT", mock.ux.text), ("T.MID", mock.ux.mid), ("INK on CH1", el.INK)):
    bgc = mock.ux.bg if nm_ != "INK on CH1" else el.CH_COL[0]
    peak = mock.ramp(c, bgc)[15]
    P("| %s | %s | %d/31 | %s | %.2f (vs real bg %s) | - |" % (nm_, hexc(c), c >> 11, hexc(peak), contrast_ratio(peak, bgc), hexc(bgc)))
# render under GREY and measure
res = collections.OrderedDict()
r = capture(S()["playing"], palette="GREY"); nt = capture(S()["playing"], palette="GREY", skip_text=True)["screen"]; sc = r["screen"]
P("")
P("GREY render of scene 'playing': text draws and the brightest pixel inside each ink box (screen after render) vs the pixel colour behind it (no-text render):")
P("| text | font | fg declared | brightest ink pixel in render | contrast vs declared bg (as drawn) | contrast vs pixel behind |")
P("|---|---|---|---|---|---|")
for t in sorted(r["texts"], key=lambda t: (t["ycap"], t["x"])):
    if not t["ink"] or t["occluded"]: continue
    a, b, c, d = t["ink"]
    pix = [(sc.px[y * 240 + x], nt.px[y * 240 + x]) for y in range(b, d) for x in range(a, c) if sc.px[y * 240 + x] != nt.px[y * 240 + x]]
    if not pix: continue
    lum = lambda p: mock.luminance(p)
    best = max(pix, key=lambda p: abs(lum(p[0]) - lum(p[1])))
    P("| %r | %s | %s %s | %s | %.2f | %.2f |" % (t["s"], t["f"], hexc(t["fg"]), tokname(t["fg"]), hexc(best[0]), contrast_ratio(best[0], t["bg"]), contrast_ratio(best[0], best[1])))
# non-neutral pixels in GREY render
neut = lambda p: (max(to_rgb888(p)) - min(to_rgb888(p))) <= 6   # gray(x5) has g6=2*x5 so g differs from r,b by <=5/255
chrom = sum(1 for p in sc.px if not neut(p))
P("")
P("GREY render 'playing': %d of %d pixels are non-neutral (chromatic): channel-colour rects/frames/bars/chips are still drawn in colour; text pixels are neutral grey ramps" % (chrom, len(sc.px)))
# are all text ink pixels neutral?
nn = 0; tt = 0
for t in r["texts"]:
    if not t["ink"] or t["occluded"]: continue
    a, b, c, d = t["ink"]
    for y in range(b, d):
        for x in range(a, c):
            if sc.px[y * 240 + x] != nt.px[y * 240 + x]:
                tt += 1; nn += not neut(sc.px[y * 240 + x])
P("text-ink pixels (changed by text) = %d; chromatic among them = %d" % (tt, nn))
# REC button state under palettes
P("")
P("## 8d. REC-armed button colour under each palette")
for pal, vn in (("mock-MONO", "mock"), ("fw-MONO (idx 9)", "fw"), ("GREY", "grey")):
    if vn == "grey":
        variant("mock"); pname = "GREY"
    else:
        variant(vn); pname = "MONO"
    s0 = mod.v6.S0(foot="keys"); s1 = mod.v6.S0(foot="keys", rec=True)
    a = capture(s0, palette=pname)["screen"]; b = capture(s1, palette=pname)["screen"]
    cell = (161 + 40, 158, 38, 24)  # REC cell is col 5 row 2: x=201
    x0, y0, w, h = 201, 158, 38, 24
    d = sum(1 for y in range(y0, y0 + h) for x in range(x0, x0 + w) if a.px[y * 240 + x] != b.px[y * 240 + x])
    icon_col = [b.px[(y0 + 12) * 240 + x] for x in range(x0 + 12, x0 + 26)]
    P("- %s: T.REC=%s T.TEXT=%s; REC cell pixels differing armed vs not = %d" % (pal, hexc(mock.ux.rec), hexc(mock.ux.text), d))
open("/home/claude/fm1/docs/design/ui_review/critique_v7/scripts/s8_out.md", "w").write("\n".join(out))
print("\n".join(out))
