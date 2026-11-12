# SPDX-License-Identifier: GPL-3.0-only (run from repo root: .venv/bin/python <this>)
"""WCAG contrast of (a) every text fg/bg pair drawn (nominal RGB565 tokens, as the v3.2 evidence did), with the background that is actually
under the ink box in the final frame cross-checked, and (b) every non-text colour pair that touches on the final frame (4-neighbour pixel
edges, text ink boxes masked out), with the number of touching edge pixels."""
import sys; sys.path.insert(0, "docs/design/ui_review/critique_note_edit/scripts")
from ne_lib import *
print("### tokens on HI-CON (RGB565 -> sRGB as png_write expands)\n| token(s) | RGB565 | hex |\n|---|---|---|")
byv = collections.OrderedDict()
for k, v in TOK.items(): byv.setdefault(v, []).append(k.replace("T_", "").replace("UISQ_", ""))
for v, ks in byv.items(): print("| %s | 0x%04X | %s |" % (" = ".join(ks), v, hexof(v)))
print("\n### (a) text pairs (nominal fg/bg as passed to the draw call); '!' = under 4.5:1")
print("| render | string | font | fg | bg passed | ratio | actual dominant colour under the ink box | ratio vs that | flag |\n|---|---|---|---|---|---|---|---|---|")
allpairs = collections.OrderedDict()
for n in RENDERS + EXTRA:
    im = img(n)
    for t in vtexts(n):
        x0, y0, x1, y1 = t["ink"]; c = collections.Counter()
        for y in range(max(0, y0), min(240, y1)):
            for x in range(max(0, x0), min(240, x1)): c[px565(im, x, y)] += 1
        # the dominant colour that is not the fg
        dom = [k for k, _ in c.most_common() if k != t["fg"]]
        dom = dom[0] if dom else t["bg"]
        r = cr(t["fg"], t["bg"]); r2 = cr(t["fg"], dom)
        key = (t["fg"], t["bg"], t["f"])
        allpairs.setdefault(key, []).append((n, t["s"]))
        print("| %s | `%s` | %s | %s | %s | %.2f | %s | %.2f | %s |" % (n, t["s"], t["f"], names_of(t["fg"]), names_of(t["bg"]), r, names_of(dom), r2, ("!" if r < 4.5 else "") + (" bg!=dominant" if dom != t["bg"] else "")))
print("\n### (a2) distinct text colour pairs across the four renders + 2 worst cases, with every user")
print("| fg | bg | ratio | <4.5? | font(s) | users (render:string, first 8) |\n|---|---|---|---|---|---|")
agg = collections.OrderedDict()
for (fg, bg, f), u in allpairs.items(): agg.setdefault((fg, bg), [[], []]); agg[(fg, bg)][0].append(f); agg[(fg, bg)][1] += u
for (fg, bg), (fs, us) in sorted(agg.items(), key=lambda kv: cr(*kv[0])):
    r = cr(fg, bg); print("| %s %s | %s %s | %.2f | %s | %s | %s |" % (names_of(fg), hexof(fg), names_of(bg), hexof(bg), r, "YES" if r < 4.5 else "no", "/".join(sorted(set(fs))), ", ".join("%s:%s" % (a.replace("note_", ""), b) for a, b in us[:8]) + (" ..." if len(us) > 8 else "")))
print("\n### (b) non-text touching colour pairs on the final frame (4-neighbour edges; text ink boxes masked); '!' = under 3:1")
for n in RENDERS + EXTRA:
    im = img(n); W, H = im.size; mask = set()
    for t in vtexts(n):
        x0, y0, x1, y1 = t["ink"]
        for y in range(y0 - 1, y1 + 1):
            for x in range(x0 - 1, x1 + 1): mask.add((x, y))
    cnt = collections.Counter(); ex = {}
    P = [[px565(im, x, y) for x in range(W)] for y in range(H)]
    for y in range(H):
        for x in range(W):
            for dx, dy in ((1, 0), (0, 1)):
                x2, y2 = x + dx, y + dy
                if x2 >= W or y2 >= H: continue
                a, b = P[y][x], P[y2][x2]
                if a == b or (x, y) in mask or (x2, y2) in mask: continue
                k = (min(a, b), max(a, b)); cnt[k] += 1; ex.setdefault(k, (x, y))
    print("\n#### %s: %d distinct touching pairs" % (n, len(cnt)))
    print("| colour A | colour B | ratio | edge px | first at | flag |\n|---|---|---|---|---|---|")
    for k, v in sorted(cnt.items(), key=lambda kv: cr(*kv[0])):
        r = cr(*k); print("| %s %s | %s %s | %.2f | %d | %d,%d | %s |" % (names_of(k[0]), hexof(k[0]), names_of(k[1]), hexof(k[1]), r, v, ex[k][0], ex[k][1], "!" if r < 3 else ""))

print("\n### (c) named non-text pairs that carry NOTE_EDIT meaning (not all touch; nominal tokens); '!' = under 3:1")
NT = [
 ("set tile fill vs AUTO tile fill (the shape cue that says 'set')", "CHF1", "SURF"),
 ("set tile fill vs the BG gap between tiles", "CHF1", "BG"),
 ("set tile level edge (2 px, both sides) vs the set fill", "CH1", "CHF1"),
 ("AUTO tile fill vs BG", "SURF", "BG"),
 ("turned-tile frame (2 px) CH1 vs BG", "CH1", "BG"),
 ("turned-tile frame CH1 vs AUTO tile SURF", "CH1", "SURF"),
 ("turned-tile frame CH1 vs set fill CHF1", "CH1", "CHF1"),
 ("edited-step frame (music row) WHITE vs BG", "WHITE", "BG"),
 ("edited-step frame WHITE vs a CH1 bar it touches", "WHITE", "CH1"),
 ("edited-step frame WHITE vs rest dash DIM", "WHITE", "DIM"),
 ("edited-key frame WHITE vs white key body SURF", "WHITE", "SURF"),
 ("edited-key frame WHITE vs key front CH1", "WHITE", "CH1"),
 ("status chip fill CH1 vs BG", "CH1", "BG"),
 ("legend square fill CHF1 vs BG", "CHF1", "BG"),
 ("legend square frame CH1 vs BG", "CH1", "BG"),
 ("legend square fill CHF1 vs the tile set fill CHF1 (same colour: the legend key IS the fill)", "CHF1", "CHF1"),
 ("key front (step on) CH1 vs white key SURF", "CH1", "SURF"),
 ("music bar CH1 vs BG", "CH1", "BG"),
 ("black key RAISE vs BG", "RAISE", "BG"),
 ("black key RAISE vs white key SURF", "RAISE", "SURF"),
 ("button tile (dice) DICE vs BG", "DICE", "BG"),
 ("button tile (dice) DICE vs button tile SURF (grouping)", "DICE", "SURF"),
 ("button tile SURF vs BG", "SURF", "BG"),
 ("dice icon WHITE on DICE", "WHITE", "DICE"),
 ("icon TEXT on SURF", "TEXT", "SURF"),
 ("muted-chip digit MID on SURF (header 2 3 4)", "MID", "SURF"),
 ("transport stop square MID on BG", "MID", "BG"),
 ("rest dash DIM vs BG", "DIM", "BG"),
 ("toast slab WHITE vs BG (note_dice)", "WHITE", "BG"),
]
print("| pair | A | B | ratio | flag |\n|---|---|---|---|---|")
for d, a, b in NT:
    A, B = TOK["T_" + a] if "T_" + a in TOK else TOK["UISQ_" + a], TOK["T_" + b] if "T_" + b in TOK else TOK["UISQ_" + b]
    r = cr(A, B); print("| %s | %s %s | %s %s | %.2f | %s |" % (d, a, hexof(A), b, hexof(B), r, "!" if r < 3 else ""))
