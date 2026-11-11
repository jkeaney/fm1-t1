#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""P2-016: an independent check of EUCLID's rhythm, written from docs/design/algorithms/euclid/SPEC.md section 3 (not from proto.c).
Usage: euclid_ref.py golden.txt. For every generated case (a '## ... m=... seeds=...' section) the sounding steps (flags bit 0) must be
  1. periodic in LEN (t and t + LEN agree),
  2. min(HITS, LEN) in number,
  3. a maximally even set: the one necklace Euclid's E(k, n) names, which is a rotation of { floor(i * n / k) } (Clough and Douthett),
and, with ROT seed 0 and OFFSET 0, E(5, 16) must be exactly steps 0 3 6 9 12 (the SPEC.md worked example).
It shares no code with proto.c, so Bjorklund's port (SP11) is checked against the mathematical definition, not against itself."""
import re, sys

def main(path):
    cases = bad = 0
    sec = None
    def check(sec):
        nonlocal cases, bad
        name, m, seeds, rows = sec
        n_hits, ln, off = m[0], m[1], m[4]
        hit = [(r & 1) for r in rows]
        cases += 1
        errs = []
        if any(hit[t] != hit[t % ln] for t in range(128)): errs.append("not periodic in LEN")
        pos = [i for i in range(ln) if hit[i]]
        k = min(n_hits, ln)
        if len(pos) != k: errs.append(f"{len(pos)} hits, wanted {k}")
        elif 0 < k < ln:
            ref = {(i * ln) // k for i in range(k)}
            if not any({(p + s) % ln for p in ref} == set(pos) for s in range(ln)): errs.append(f"{pos} is not maximally even for E({k},{ln})")
        if seeds[0] == 0 and off == 0 and (k, ln) == (5, 16) and pos != [0, 3, 6, 9, 12]: errs.append(f"E(5,16) is {pos}")
        if errs:
            bad += 1; print(f"euclid_ref: {name}: " + "; ".join(errs))
    for line in open(path):
        if line.startswith("## "):
            if sec: check(sec)
            mm = re.search(r" m=([\d,]+) seeds=([0-9a-f,]+)", line)
            sec = (line.strip(), [int(x) for x in mm.group(1).split(",")], [int(x, 16) for x in mm.group(2).split(",")], []) if mm else None
        elif sec and re.match(r"\s*\d+\s", line):
            f = line.split()
            sec[3].append(int(f[4], 16))
    if sec: check(sec)
    print(f"euclid_ref: {cases} cases, {bad} mismatches")
    return 1 if bad or not cases else 0

sys.exit(main(sys.argv[1]))
