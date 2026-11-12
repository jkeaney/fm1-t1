# SPDX-License-Identifier: GPL-3.0-only (evidence script; repo root, .venv/bin/python): 6 channel tile matrix: 6 algorithms x MIDI 1..16/OFF x muted x selected
import sys, os; sys.path.insert(0, "docs/design/ui_review/critique_v3_2")
from evidence_b_lib import *
from PIL import Image
OUT = "docs/design/ui_review/critique_v3_2/edge"
mock.palette_set("GREY")
ALG = P.ALGO_NAMES
rows = []
sheets = {}
for sel in (1, 0):
    for mut in (0, 1):
        sh = Image.new("RGB", (60 * 17, 48 * 6), (255, 0, 255))
        for a in range(6):
            for m in range(17):                      # m = 0..15 -> MIDI 1..16, 16 -> OFF
                s = scene("glance_playing"); s["sel"] = 0 if sel else 1
                s["channels"][0].update(algo=a, midi_ch=m, muted=mut, snd0=1)
                if not sel: s["channels"][1].update(algo=0)
                a0 = s["channels"][s["sel"]]["algo"] % 6
                s["knobs"] = [(lo + hi) // 2 if en != "KEYS" else max(lo + 1, (lo + hi) // 2) for (lb, lo, hi, en) in P.P1.KNOBS[a0]]
                sc, w, recs = render(s, "t")
                sh.paste(sc.to_image().crop((0, 24, 60, 72)), (60 * m, 48 * a))
                mine = [r for r in recs if r["ink"][1] >= 25 and r["ink"][3] <= 64 and r["ink"][2] <= 62]
                d = {r["role"]: r for r in mine}
                tile_r = 59
                ink_name = d["algo name"]["ink"]; ink_mid = d["midi slot"]["ink"]; ink_num = d["chan numeral"]["ink"]
                chip = d.get("mute chip")
                chip_l = chip["x"] if chip else None
                rows.append(dict(sel=sel, mut=mut, algo=ALG[a], midi=("OFF" if m == 16 else m + 1), mid_s=d["midi slot"]["s"], name_r=ink_name[2], mid_r=ink_mid[2], gap_mid_chip=(chip_l - ink_mid[2]) if chip else None,
                                 chip_r=(chip["x"] + 14) if chip else None, name_margin=tile_r + 1 - ink_name[2], flags=[r["s"] for r in recs if r["flags"] & 3], warn=w, name_chip_overlap=(chip and ink_name[2] > chip_l and ink_name[1] < 29 + 14)))
        nm = "f_tiles_%s_%s" % ("sel" if sel else "unsel", "muted" if mut else "live")
        sh.save(os.path.join(OUT, nm + ".png")); sh.resize((60 * 17 * 4, 48 * 6 * 4), Image.NEAREST).save(os.path.join(OUT, nm + "_4x.png"))
print("combinations rendered:", len(rows), "tool warnings/flags:", sum(1 for r in rows if r["flags"] or r["warn"]))
print("\nper (selected, muted): min gap MIDI-ink right edge to the M chip left edge (px), widest case; algo-name right margin (tile right edge = x+59 inclusive-exclusive 60 grid: margin = tile_x+59-ink_right)")
for sel in (1, 0):
    for mut in (0, 1):
        R = [r for r in rows if r["sel"] == sel and r["mut"] == mut]
        g = [r for r in R if r["gap_mid_chip"] is not None]
        nm = min(R, key=lambda r: r["name_margin"])
        print(" sel=%d muted=%d: min name margin %d (%s, MIDI %s); " % (sel, mut, nm["name_margin"], nm["algo"], nm["midi"]) + ("min MIDI-ink to chip gap %d (%s MIDI %s)" % (min(r["gap_mid_chip"] for r in g), *[(r["algo"], r["midi"]) for r in g if r["gap_mid_chip"] == min(x["gap_mid_chip"] for x in g)][0]) if g else "no chip"))
print("\nwidest name ink right edge for muted tiles vs chip left (x+42=42 in tile 0 coordinates): name ink right edge by algo:")
for a in ALG:
    r = [x for x in rows if x["algo"] == a][0]
    print("  %s: ink right edge x=%d (tile 0 spans x 1..58); overlaps chip column (x>=43) at y of name? the name row is y 47..60 and the chip is y 29..43: %s" % (a, r["name_r"], "no (different rows)"))
print("\nMIDI-slot ink right edge vs M chip left edge (chip rect x=43..56), muted tiles, MIDI 10..16 and OFF (EUCLID):")
for r in rows:
    if r["mut"] == 1 and r["algo"] == "EUCLID" and r["sel"] == 1 and r["midi"] in (1, 9, 10, 11, 16, "OFF"):
        print("  MIDI %-3s slot `%s` ink right %d, chip left 43 -> gap %s" % (r["midi"], r["mid_s"], r["mid_r"], r["gap_mid_chip"]))
print("\nname-to-tile-edge margin and widest names:", sorted({(r["algo"], r["name_margin"]) for r in rows}))
