# SPDX-License-Identifier: GPL-3.0-only (evidence script; repo root, .venv/bin/python): condenses data/s6e_knob_sweep.txt per algorithm x row
import re, collections
rows = []
for l in open("docs/design/ui_review/critique_v3_2/data/s6e_knob_sweep.txt"):
    if not l.startswith("| ") or l.startswith("| algo") or l.startswith("|---"): continue
    c = [x.strip() for x in l.strip().strip("|").split("|")]
    if len(c) < 14: continue
    rows.append(c)
g = collections.OrderedDict()
for c in rows:
    algo, row, case, k, label, lf, lw, lm, val, vf, vw, vm, pg, fl = c
    key = (algo, row); d = g.setdefault(key, dict(labels=collections.OrderedDict(), lmax=(0, ""), lmin=99, vmax=(0, ""), vmin=99, drop=set(), pic=99, flags=0, n=0))
    d["n"] += 1
    d["labels"][label.strip("`")] = 1
    d["lmax"] = max(d["lmax"], (int(lw), label.strip("`")))
    d["lmin"] = min(d["lmin"], min(int(x) for x in lm.split("/")))
    d["vmax"] = max(d["vmax"], (int(vw), val.strip("`")))
    d["vmin"] = min(d["vmin"], min(int(x) for x in vm.split("/")))
    if vf == "M": d["drop"].add("%s=%s" % (label.strip("`"), val.strip("`")) if not val.strip("`").isdigit() else label.strip("`"))
    if pg != "None": d["pic"] = min(d["pic"], int(pg))
    if fl != "-": d["flags"] += 1
print("| algorithm | row | K1..K4 labels | widest label (ink px) | min label margin L/R (of 58) | widest value (ink px) | min value margin | value drawn in AF_M (min/max/long cases) | picture-to-name gap min | tool flags | cells |\n|---|---|---|---|---|---|---|---|---|---|---|")
for (a, r), d in g.items():
    print("| %s | %s | %s | `%s` %d | %d | `%s` %d | %d | %s | %s | %d | %d |" % (a, r, " ".join(d["labels"].keys()), d["lmax"][1], d["lmax"][0], d["lmin"], d["vmax"][1], d["vmax"][0], d["vmin"], ", ".join(sorted(d["drop"])) or "-", d["pic"] if d["pic"] != 99 else "-", d["flags"], d["n"]))
