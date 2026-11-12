# SPDX-License-Identifier: GPL-3.0-only (evidence script; repo root, .venv/bin/python): 6(e) every knob cell, 6 algorithms, ROW 1/2 (+ROW 3), min / max / longest-name value
import sys, os; sys.path.insert(0, "docs/design/ui_review/critique_v3_2")
from evidence_b_lib import *
from PIL import Image
OUT = "docs/design/ui_review/critique_v3_2/edge"
ALG = ("ACID", "EUCLID", "SNAKE", "STAGE", "ARP", "PATTERNS")
mock.palette_set("GREY")
inkw = lambda f, s: (lambda b: b[2] - b[0])(text_ink(f, s))
P1 = P.P1
rows_out = []
sheets = {}
def pic_gap(sc, k, nameink, bg, body):
    """px rows between the lowest picture/meter pixel above the name text and the name ink top, in tile k (rows 163..)"""
    x0 = 60 * k + 1
    top = nameink[1]
    last = None
    for y in range(163, top):
        if any(sc.px[y * 240 + x] not in (body,) for x in range(x0, x0 + 58)): last = y
    return None if last is None else top - last - 1
for a in range(6):
    for row in (0, 1, 2):
        if row == 2 and a != 0: continue                    # ROW 3 exists per device (rig), not per algorithm: drawn once, on ACID
        sets = {"min": [], "max": [], "long": []}
        for k in range(8):
            lab, lo, hi, en = P1.KNOBS[a][k]
            sets["min"].append(lo); sets["max"].append(hi)
            def vw(v):
                s = scene("glance_playing"); s["channels"][0]["algo"] = a; s["knobs"] = [lo] * 8; s["knobs"][k] = v; s["row"] = 0 if k < 4 else 1
                lb, vl, f, m, pc = P.knob_view(s, 0, k % 4)
                return (inkw(f, vl) if not pc else inkw(AF_M, pc[3]), inkw(AF_M, lb))
            sets["long"].append(max(range(lo, hi + 1), key=vw))
        strips = []
        for nm in ("min", "max", "long"):
            s = scene("glance_playing"); s["channels"][0]["algo"] = a; s["row"] = row; s["knobs"] = sets[nm]
            if row == 2:
                s["channels"][0]["rig"] = 1
                s["rig_vals"] = {"min": [0] * 4, "max": [127] * 4, "long": [127, 100, 127, 127]}[nm]
            sc, w, recs = render(s, "k")
            strips.append(sc.to_image().crop((0, 144, 240, 220)))
            bg = T.BG
            for k in range(4):
                x0 = 60 * k + 1
                mine = [r for r in recs if r["role"] in ("knob label", "knob value", "knob value (M)", "knob heard") and x0 - 1 <= r["ink"][0] and r["ink"][2] <= x0 + 59]
                lab = [r for r in mine if r["role"] == "knob label"][0]
                val = [r for r in mine if r["role"].startswith("knob value")][0]
                lm = (lab["ink"][0] - x0, x0 + 58 - lab["ink"][2]); vm = (val["ink"][0] - x0, x0 + 58 - val["ink"][2])
                flags = [r["s"] for r in recs if r["flags"] & 3]
                body = T.RAISE if row == 2 else T.SURF
                pg = None
                if val["role"] == "knob value (M)" and val["ink"][1] > 175:
                    pg = pic_gap(sc, k, val["ink"], bg, body)
                rows_out.append(dict(algo=ALG[a], row=row + 1, case=nm, k=k + 1, label=lab["s"], lfont=lab["f"], lw=lab["ink"][2] - lab["ink"][0], lmargin=lm, value=val["s"], vfont=val["f"], vw=val["ink"][2] - val["ink"][0], vmargin=vm, picgap=pg, flags=flags, warn=w))
        sheet = Image.new("RGB", (240, 76 * 3), (255, 0, 255))
        for i, st in enumerate(strips): sheet.paste(st, (0, 76 * i))
        nm = "e_knobs_%s_row%d" % (ALG[a], row + 1)
        sheet.save(os.path.join(OUT, nm + ".png")); sheet.resize((960, 76 * 3 * 4), Image.NEAREST).save(os.path.join(OUT, nm + "_4x.png"))
print("| algo | row | case | K | label | lbl font | lbl ink | lbl margin L/R | value | val font | val ink | val margin L/R | pic-to-name gap | tool flags |\n|---|---|---|---|---|---|---|---|---|---|---|---|---|---|")
for r in rows_out:
    print("| %s | %d | %s | K%d | `%s` | %s | %d | %d / %d | `%s` | %s | %d | %d / %d | %s | %s |" % (r["algo"], r["row"], r["case"], r["k"], r["label"], r["lfont"], r["lw"], *r["lmargin"], r["value"], r["vfont"], r["vw"], *r["vmargin"], r["picgap"], r["flags"] or r["warn"] or "-"))
import collections
print("\nsummary")
print("cells rendered:", len(rows_out))
print("tool warnings/flags:", sum(1 for r in rows_out if r["flags"] or r["warn"]))
print("min label margin (px):", min(min(r["lmargin"]) for r in rows_out), [ (r["algo"], r["row"], r["case"], r["label"]) for r in rows_out if min(r["lmargin"]) <= 2][:12])
print("min value margin (px):", min(min(r["vmargin"]) for r in rows_out), [(r["algo"], r["row"], r["case"], r["value"], r["vmargin"]) for r in rows_out if min(r["vmargin"]) <= 3][:20])
print("pic gaps <= 2:", [(r["algo"], r["row"], r["case"], r["value"], r["picgap"]) for r in rows_out if r["picgap"] is not None and r["picgap"] <= 2])
vf = collections.Counter((r["vfont"]) for r in rows_out if r["case"] == "long"); print("value fonts in 'long' set:", dict(vf))
print("distinct (algo,row,K) cells whose value is AF_M in any case:", sorted({(r["algo"], r["row"], r["k"], r["label"]) for r in rows_out if r["vfont"] == "M"}))
print("widest value ink AF_L:", max((r["vw"], r["value"], r["algo"], r["row"]) for r in rows_out if r["vfont"] == "L"))
