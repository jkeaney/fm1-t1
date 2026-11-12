# SPDX-License-Identifier: GPL-3.0-only (evidence script; repo root, .venv/bin/python): diff fresh render (/tmp/v32r) vs committed
import os, sys, hashlib
sys.path.insert(0, "docs/design/ui_review/mock")
import mock
from PIL import Image
C = "docs/design/ui_review/proposals/v3.2"; N = "/tmp/v32r"
names = sorted(f[len("proposal_v3_2_"):-4] for f in os.listdir(N) if f.endswith(".png") and not f.endswith("_4x.png") and not f.endswith("_grid.png"))
tot = 0; byte_same = 0; byte_tot = 0; maxp = 0
print("| scene | differing px of 57600 |\n|---|---|")
bad = []
for n in names:
    a = "%s/proposal_v3_2_%s.png" % (N, n); b = "%s/proposal_v3_2_%s.png" % (C, n)
    if not os.path.exists(b): print("| %s | NO COMMITTED FILE |" % n); bad.append(n); continue
    A = Image.open(a).convert("RGB"); B = Image.open(b).convert("RGB")
    d = sum(1 for p, q in zip(A.getdata(), B.getdata()) if p != q)
    tot += d
    if d: bad.append(n)
    print("| %s | %d |" % (n, d))
for n in names:
    for suf in ("", "_4x", "_grid"):
        f = "proposal_v3_2_%s%s.png" % (n, suf); byte_tot += 1
        if os.path.exists("%s/%s" % (C, f)) and open("%s/%s" % (N, f), "rb").read() == open("%s/%s" % (C, f), "rb").read(): byte_same += 1
print("scenes", len(names), "total differing px", tot, "scenes with diff", bad, "files byte-identical", byte_same, "of", byte_tot)
