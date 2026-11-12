# SPDX-License-Identifier: GPL-3.0-only (evidence script; repo root, .venv/bin/python): 6 (l) both MAP pages, every algorithm / row / channel context, tile text clearances
import sys, os; sys.path.insert(0, "docs/design/ui_review/critique_v3_2")
from evidence_b_lib import *
OUT = "docs/design/ui_review/critique_v3_2/edge"
mock.palette_set("GREY")
ALG = P.ALGO_NAMES
print("| page | algo | row | sel | warnings | K1..K4 row-1 / row-2 ink widths | tightest ink-to-tile-edge margin (58 px tiles) | strings |\n|---|---|---|---|---|---|---|---|")
for a in range(6):
    for row in (0, 1, 2):
        s = scene("map_A"); s["channels"][0]["algo"] = a; s["row"] = row; s["sel"] = 0
        sc, w, recs = render(s, "m"); nm = "l_mapA_%s_row%d" % (ALG[a], row + 1); save(sc, OUT, nm)
        kn = [r for r in recs if r["role"] in ("map knob row1", "map knob row2")]
        mar = min(min(r["ink"][0] - (60 * (i // 2) + 1), 60 * (i // 2) + 59 - r["ink"][2]) for i, r in enumerate(kn)) if False else None
        # K tiles: x = 60k+1 .. +58
        margins = []
        for r in kn:
            k = (r["x"] // 60); margins.append(min(r["ink"][0] - (60 * k + 1), (60 * k + 59) - r["ink"][2]))
        print("| A | %s | %d | 1 | %s | %s | %d | %s |" % (ALG[a], row + 1, w or "none", ", ".join("%s %d" % (r["s"], r["ink"][2] - r["ink"][0]) for r in kn), min(margins), "-"))
for ch in range(4):
    s = scene("map_A"); s["sel"] = ch
    for i, c in enumerate(s["channels"]): c["algo"] = i
    a = s["channels"][ch]["algo"]
    s["knobs"] = [(lo + hi) // 2 for (lb, lo, hi, en) in P.P1.KNOBS[a]]
    sc, w, recs = render(s, "m"); save(sc, OUT, "l_mapA_sel%d" % (ch + 1))
    print("  sel CH%d: ALGO tile role %s ink %s, warnings %s" % (ch + 1, [r["s"] for r in recs if r["s"].startswith("OF CH")], [r["ink"] for r in recs if r["s"].startswith("OF CH")], w or "none"))
print("\nMAP A / B tool warnings:", [render(scene(n), n)[1] or "none" for n in ("map_A", "map_B", "map_A_row3", "map_menu_countdown", "map_touch")])
# role text in 58 px tiles (map_A): widest and margins
sc, w, recs = render(scene("map_A"), "m")
tile_roles = [r for r in recs if r["role"] in ("map role", "map hold", "map printed")]
print("map_A tile text: widest ink per role:", {role: max(r["ink"][2] - r["ink"][0] for r in tile_roles if r["role"] == role) for role in ("map role", "map hold", "map printed")})
print("tile text with ink margin <= 2 px to the 58 px tile edge:")
for r in tile_roles:
    k = (r["ink"][0] + r["ink"][2]) // 2 // 60
    m = min(r["ink"][0] - (60 * k + 1), (60 * k + 59) - r["ink"][2])
    if m <= 2: print("   `%s` (%s) margin %d, ink %s" % (r["s"], r["role"], m, r["ink"]))
# map_B: black-key labels vs leader lines; labels clipping to screen edge; white key numerals
sc, w, recs = render(scene("map_B"), "mb")
print("\nmap_B black key labels: ", [(r["s"], r["ink"]) for r in recs if r["role"].startswith("map black")])
print("map_B ink touching the screen edge (x<=1 or x>=238):", [(r["s"], r["ink"]) for r in recs if r["ink"][0] <= 1 or r["ink"][2] >= 239])
# key-group structure: black key rects measured from pixels (row y=20)
row = [sc.px[20 * 240 + x] for x in range(240)]
runs = []; st = None
for i, v in enumerate(row + [None]):
    ok = v == T.KEY
    if ok and st is None: st = i
    if not ok and st is not None: runs.append((st, i - st)); st = None
print("map_B black key rectangles on row y=20 (x, w):", runs, " gaps:", [runs[i + 1][0] - runs[i][0] - runs[i][1] for i in range(len(runs) - 1)])
# touch-help line for every PERF control: font and width
