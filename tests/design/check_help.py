#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""P2-011 (F-06): check docs/design/help_text.tsv.
  1. one row per (control of control_map.tsv) x (PERF NOTE_EDIT CHAN SET SCALE REC RESET), nothing else;
  2. line 1 never empty; each line at most 34 characters (SCREEN_SPEC 5) and at most 224 px in AF_S (the help box,
     SCREEN_SPEC 3), measured with the real text_w through tests/design/width_probe.sh; the {r1} / {r2} placeholders of
     the PERF knob rows are expanded with every ROW 1 / ROW 2 label of knob_map.tsv;
Prints OK and exits 0 when all hold."""
import csv, os, subprocess, sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(__file__), "..", ".."))
D = os.path.join(ROOT, "docs", "design")
PAGES = ["PERF", "NOTE_EDIT", "CHAN", "SET", "SCALE", "REC", "RESET"]
rd = lambda n: list(csv.DictReader(open(os.path.join(D, n)), delimiter="\t"))
cm, ht, km = rd("control_map.tsv"), rd("help_text.tsv"), rd("knob_map.tsv")
bad = []
ctls = []
for r in cm:
    if r["control"] not in ctls:
        ctls.append(r["control"])
have = {}
for r in ht:
    k = (r["control"], r["context"])
    if k in have:
        bad.append("duplicate %s" % (k,))
    have[k] = r
for c in ctls:
    for p in PAGES:
        if (c, p) not in have:
            bad.append("missing %s %s" % (c, p))
for k in have:
    if k[0] not in ctls or k[1] not in PAGES:
        bad.append("unknown row %s" % (k,))
r1 = sorted({r["label"] for r in km if r["page"] == "PERF" and r["knob"] in ("K1", "K2", "K3", "K4") and r["context"] != "*"})
r2 = sorted({r["label"] for r in km if r["page"] == "PERF" and r["knob"] in ("S1", "S2", "S3", "S4")})
probe = []
for (c, p), r in have.items():
    if not r["line1"].strip():
        bad.append("empty line 1 %s %s" % (c, p))
    for n in ("line1", "line2"):
        t = r[n]
        subs = [t]
        if "{r1}" in t or "{r2}" in t:
            subs = [t.replace("{r1}", a).replace("{r2}", b) for a in r1 for b in r2]
        for s in subs:
            if len(s) > 34:
                bad.append("%s %s %s: %d characters: %s" % (c, p, n, len(s), s))
            if s:
                probe.append("S\t224\t%s\t%s %s %s" % (s, c, p, n))
out = subprocess.run(["sh", os.path.join(ROOT, "tests/design/width_probe.sh"), "-"], input="\n".join(probe) + "\n",
                     capture_output=True, text=True)
for l in out.stdout.splitlines():
    if "OVER" in l:
        bad.append("width " + l.strip())
if out.returncode not in (0, 1):
    bad.append("probe failed: " + out.stderr[-300:])
if bad:
    print("\n".join(bad[:40]))
    print("FAIL: %d problems" % len(bad))
    sys.exit(1)
print("OK: %d help rows, %d strings measured" % (len(have), len(probe)))
