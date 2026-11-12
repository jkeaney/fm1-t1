#!/usr/bin/env python3
"""contrast.py: WCAG 2.x relative-luminance contrast of every (foreground, background) theme-colour pair that ui_sq.c draws, per palette.
Inputs (this directory): palettes.tsv (the 16 T_* values of each of the 10 palettes, dumped from the real palette_set()) and elements.tsv
(the `colours` column lists the T_fg/T_bg pairs each element is drawn with, text and graphics). RGB565 -> sRGB 8 bit as the PNG writer of
tests/ui_sq_test.c does it (r*255/31, g*255/63, b*255/31). Usage:  python3 docs/design/ui_review/contrast.py   (prints markdown)"""
import csv, os, re, collections
D = os.path.dirname(os.path.abspath(__file__))
pal = list(csv.DictReader(open(os.path.join(D, "palettes.tsv")), delimiter="\t"))
els = list(csv.DictReader(open(os.path.join(D, "elements.tsv"), encoding="utf-8"), delimiter="\t"))

def rgb(c): return (((c >> 11) & 31) * 255 // 31, ((c >> 5) & 63) * 255 // 63, (c & 31) * 255 // 31)
def lum(c):
    def f(v):
        v /= 255.0
        return v / 12.92 if v <= 0.03928 else ((v + 0.055) / 1.055) ** 2.4
    r, g, b = rgb(c); return 0.2126 * f(r) + 0.7152 * f(g) + 0.0722 * f(b)
def ratio(a, b):
    la, lb = lum(a), lum(b)
    if la < lb: la, lb = lb, la
    return (la + 0.05) / (lb + 0.05)
def hexs(c): r, g, b = rgb(c); return "#%02X%02X%02X" % (r, g, b)
def name(c):
    r, g, b = rgb(c); mx, mn = max(r, g, b), min(r, g, b); L = (mx + mn) / 2 / 255.0
    ln = "black" if L < 0.04 else "very dark" if L < 0.15 else "dark" if L < 0.35 else "mid" if L < 0.6 else "light" if L < 0.85 else "very light"
    if mx - mn < 24: return ln + " grey"
    if mx == r and g >= b: h = "orange/yellow" if g > r * 0.55 else "red-orange"
    elif mx == r: h = "red/pink"
    elif mx == g: h = "green"
    else: h = "blue"
    return ln + " " + h
def val(p, role): return int(p["T_" + role], 16)

# TEXT pairs: text_pairs.tsv (every text drawn in every page x algorithm scenario). GRAPHIC pairs: the `colours` column of the graphic rows of elements.tsv.
tp = list(csv.DictReader(open(os.path.join(D, "text_pairs.tsv")), delimiter="\t"))
tpairs = collections.OrderedDict(((r["fg"], r["bg"]), [r["elements"] + " [" + r["pages"] + "]"]) for r in tp)
gpairs = collections.OrderedDict()
for e in els:
    if e["kind"] != "graphic": continue
    for part in e["colours"].split(";"):
        m = re.match(r"\s*T_(\w+)/T_(\w+)", part)
        if m and (m[1], m[2]) not in tpairs: gpairs.setdefault((m[1], m[2]), []).append(e["id"])
pairs = collections.OrderedDict(list(tpairs.items()) + list(gpairs.items()))
roles = sorted({r for k in pairs for r in k})
g = next(p for p in pal if p["name"] == "GREY")

print("### Colours used by ui_sq.c (palette 0 GREY, the firmware default: panel.c sets settings.palette = UI_GREY_INDEX)\n")
print("| Role | RGB565 | sRGB hex | Plain name |\n| --- | --- | --- | --- |")
for r in roles: print("| T_%s | 0x%04X | %s | %s |" % (r, val(g, r), hexs(val(g, r)), name(val(g, r))))
print("| (fixed) clock dot dark | 0xFD80 | %s | %s |" % (hexs(0xFD80), name(0xFD80)))
print("| (fixed) clock dot light | 0xCB80 | %s | %s |" % (hexs(0xCB80), name(0xCB80)))
print()
names = [p["name"] for p in pal]
names = [p["name"] for p in pal]
def table(title, pr, note):
    print("### " + title + "\n"); print(note + "\n")
    print("| fg on bg | used by | " + " | ".join(names) + " | min (palette) |\n| --- | --- | " + " | ".join("---" for _ in names) + " | --- |")
    rows = []
    for (f, b), ids in pr.items():
        vals = [ratio(val(p, f), val(p, b)) for p in pal]
        rows.append((vals[0], f, b, ids, vals, min(range(len(vals)), key=lambda i: vals[i])))
    for v0, f, b, ids, vals, lo in sorted(rows, key=lambda r: r[0]):
        ids = sorted(set(ids)); s = "; ".join(ids[:3]) + (" +%d more" % (len(ids) - 3) if len(ids) > 3 else "")
        print("| T_%s on T_%s | %s | %s | %.2f (%s) |" % (f, b, s, " | ".join("%.2f" % v for v in vals), vals[lo], names[lo]))
    print()
    allv = [(v, f, b, names[i]) for v0, f, b, ids, vals, lo in rows for i, v in enumerate(vals)]
    print("Lowest ratio over all pairs and palettes: %.2f (T_%s on T_%s, %s)." % min(allv))
    print("Lowest ratio on palette 0 GREY: %.2f (T_%s on T_%s)." % min((r[0], r[1], r[2]) for r in rows))
    for thr in (3.0, 4.5, 7.0):
        n0 = [(f, b) for v0, f, b, ids, vals, lo in rows if v0 < thr]
        print("Pairs under %.1f:1 on GREY: %d of %d (%s)" % (thr, len(n0), len(rows), ", ".join("T_%s/T_%s" % x for x in n0)))
    print()
table("Contrast of TEXT (fg ink on the bg it is drawn on), every page and algorithm", tpairs, "Each row is a pair some text in ui_sq.c is drawn with; `used by` lists element classes (n = index) and the pages where it occurs.")
table("Contrast of non-text GRAPHICS (fills, outlines, bars, dividers)", gpairs, "Pairs from the graphic rows of elements.tsv that no text uses. A fill's ratio is against the background it sits on.")
