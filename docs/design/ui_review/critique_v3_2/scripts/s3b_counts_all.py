# SPDX-License-Identifier: GPL-3.0-only (evidence script; repo root, .venv/bin/python): strings by font in all 71 scenes
import sys; sys.path.insert(0, "docs/design/ui_review/critique_v3_2")
from evidence_b_lib import *
print("| scene | strings | AF_L | AF_M | AF_S | roles | flagged (cut/ellipsis) |\n|---|---|---|---|---|---|---|")
tot = {"L": 0, "M": 0, "S": 0}; withS = []
for n in sorted(P.STATE["scenes"]):
    sc, w, recs = render(scene(n), n)
    c = {f: sum(1 for r in recs if r["f"][-1] == f) for f in "LMS"}
    for f in "LMS": tot[f] += c[f]
    if c["S"]: withS.append(n)
    print("| %s | %d | %d | %d | %d | %d | %d |" % (n, len(recs), c["L"], c["M"], c["S"], len({r["role"] for r in recs}), sum(1 for r in recs if r["flags"] & 3)))
print("\nTOTAL over 71 scenes: AF_L %d, AF_M %d, AF_S %d; scenes with AF_S: %s" % (tot["L"], tot["M"], tot["S"], withS))
