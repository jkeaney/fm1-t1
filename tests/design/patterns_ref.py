#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""P2-017: the independent check of PATTERNS. Usage: patterns_ref.py golden.txt
patterns/proto.c (C) prints the section `## PATTERNS-COMPACT` for the first pattern of the golden set of each category, every dial at min / mid / max, in the
text format of patterns_golden.txt. That file is written by tests/design/patterns_proto.py (Python, P2-014), which shares no code with proto.c. This script
requires the two texts to be equal, line by line, once two things a PulseOut cannot carry are removed from the Python text (the accent letter, and a slide flag on a step that is followed by a rest): two implementations
of SPEC 4.1 agreeing on 126 cases x 12..32 steps (semitone, slide, tie, velocity, rest) is the proof that the C port is the specified algorithm."""
import os, re, sys

HERE = os.path.dirname(os.path.abspath(__file__))
PYG = os.path.join(HERE, "..", "..", "docs", "design", "algorithms", "patterns", "patterns_golden.txt")


def cases_py(path):
    out, name = {}, None
    for ln in open(path):
        if ln.startswith("== "):
            name = ln[3:].split(" (")[0].strip()
        elif name and "|" in ln and not ln.startswith("#"):
            lab, cells = ln.split("|", 1)
            # remove the accent letter A from a cell: "0AS:120" -> "0S:120"
            cells = re.sub(r"(-?\d+)A", r"\1", cells)
            tok = cells.split()
            # a stored slide into a rest glides into nothing: PulseOut flags SLIDE_IN on the sounding step it reaches, so the Python S before a rest has no C counterpart
            tok = [re.sub(r"S(?=[T:])", "", t) if tok[(i + 1) % len(tok)] == "." else t for i, t in enumerate(tok)]
            out[(name, " ".join(lab.split()))] = " ".join(tok)
    return out


def cases_c(path):
    out, name, on = {}, None, False
    for ln in open(path):
        if ln.startswith("## "):
            on = ln.startswith("## PATTERNS-COMPACT")
        elif on and ln.startswith("== "):
            name = ln[3:].strip()
        elif on and name and "|" in ln:
            lab, cells = ln.split("|", 1)
            out[(name, " ".join(lab.split()))] = " ".join(cells.split())
    return out


def main(path):
    py, c = cases_py(PYG), cases_c(path)
    bad = 0
    for k in sorted(set(py) | set(c)):
        if py.get(k) != c.get(k):
            bad += 1
            if bad <= 6:
                print("patterns_ref: %s %s\n  python: %s\n  C:      %s" % (k[0], k[1], py.get(k), c.get(k)))
    print("patterns_ref: %d cases, %d mismatches" % (len(py), bad))
    return 1 if bad or len(py) != 126 or len(c) != 126 else 0


sys.exit(main(sys.argv[1]))
