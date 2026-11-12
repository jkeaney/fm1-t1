# SPDX-License-Identifier: GPL-3.0-only (evidence script; repo root, .venv/bin/python): per-role summary over the six glance screens
import sys; sys.path.insert(0, "docs/design/ui_review/critique_v3_2")
from evidence_b_lib import *
GL = ["glance_idle", "glance_playing", "row2", "row3", "mutes", "stress"]
CAP = {"S": 9, "M": 11, "L": 21}
roles = {}
for n in GL:
    sc, w, recs = render(scene(n), n)
    for r in recs: roles.setdefault((r["role"], r["f"]), []).append((n, r))
print("| role | font | cap px | cap mm | arcmin @50cm | instances over the 6 screens | strings (distinct) | widest advance px | widest ink px | lowest fg/bg contrast | in screens |\n|---|---|---|---|---|---|---|---|---|---|---|")
for (role, f), v in sorted(roles.items()):
    mm, am = arcmin(CAP[f[-1]])
    print("| %s | %s | %d | %.3f | %.1f | %d | %s | %d | %d | %.2f | %s |" % (role, f, CAP[f[-1]], mm, am, len(v), ", ".join(sorted({"`%s`" % r["s"] for n, r in v}, key=len)[:8]) + (" ..." if len({r["s"] for n, r in v}) > 8 else ""), max(text_w(r["f_obj"], r["s"]) for n, r in v), max(r["ink"][2] - r["ink"][0] for n, r in v), min(cr(r["fg"], r["bg"]) for n, r in v), len({n for n, r in v})))
