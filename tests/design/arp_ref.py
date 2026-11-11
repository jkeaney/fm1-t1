#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""P2-017: an independent re-implementation of ARP written from docs/design/algorithms/arp/SPEC.md 6.3 to 6.5 (not from proto.c).
Usage: arp_ref.py golden.txt. Checks every line of the two `## ARP GRID` sections (DIR x SHAPE x RATE x SPAN 1..4, chord I and the 1451 bars):
the chord of each bar is read from the `chord ... vt=` lines of the golden file itself (HARMONY is checked on its own, H-GOLDEN); everything else
(pool, direction, shape, rate, the RAND permutation, the repeats) is recomputed here. With DENS 16 and seeds 0 for NOTES and SHAPE there are no hops and
no phase, so a line is a pure function of (chord, SPAN, DIR, SHAPE, RATE)."""
import re, sys

M = 0xFFFFFFFF
DIRS = ["UP", "DOWN", "UPDN", "RAND", "ASPL", "CHORD"]
SHAPES = ["LINE", "SKIP", "THUMB", "OCTV", "REPT", "WAVE"]
RATES = ["1/4", "1/2", "1X", "2X", "3T", "4X"]
STP = [4, 2, 1, 1, 1, 1]
SUB = [1, 1, 1, 2, 3, 4]


def mix(x):
    x &= M
    x ^= x >> 16; x = (x * 0x7FEB352D) & M
    x ^= x >> 15; x = (x * 0x846CA68B) & M
    x ^= x >> 16
    return x


def perm(P, x):
    x = (x & M) or 1
    def nx():
        nonlocal x
        x ^= (x << 13) & M; x ^= x >> 17; x ^= (x << 5) & M
        return x
    nx(); nx()
    a = list(range(P))
    for i in range(P - 1, 0, -1):
        j = (nx() >> 8) % (i + 1)
        a[i], a[j] = a[j], a[i]
    return a


def tones(ch, d):
    vt, root, nt = ch["vt"], ch["root"], len(ch["vt"])
    if d == "ASPL":
        r = next((k for k in range(nt) if (vt[k] - root) % 12 == 0), 0)
        return [vt[(r + i) % nt] + 12 * ((r + i) // nt) for i in range(nt)]
    if d == "CHORD":
        return [root + x for x in sorted((v - root) % 12 for v in vt)]
    return list(vt)


def note_n(ch, n, span, d, sh):
    nt = len(ch["vt"]); P = nt * span
    LB = max(1, 2 * P - 2) if d == "UPDN" else P
    F = {"LINE": LB, "SKIP": 2 * LB, "THUMB": 2 * max(1, LB - 1), "OCTV": 2 * LB, "REPT": 2 * LB, "WAVE": 3 * LB}[sh]
    m, y = n % F, n // F
    T = tones(ch, d)
    pm = perm(P, mix(0 * 0x9E3779B1 ^ y * 0x85EBCA6B)) if d == "RAND" else None
    def jmap(b):
        if d == "DOWN": return P - 1 - b
        if d == "UPDN": return b if b < P else 2 * P - 2 - b
        if d == "RAND": return pm[b % P]
        return b
    homep = ch["drone"] if ch["drone"] >= 0 else ch["vt"][0]
    jh = next((k for k in range(nt) if T[k] == homep), 0)      # the home note's pool index
    bh = next((k for k in range(LB) if jmap(k) == jh), 0)      # the figure position that reads it
    home = octv = False
    if sh == "LINE": b = m
    elif sh == "SKIP": b = (m // 2) % LB if m % 2 == 0 else (m // 2 + 2) % LB
    elif sh == "THUMB":
        if m % 2 == 0: home, b = True, 0
        elif LB > 1:
            b = (m // 2) % (LB - 1)
            if b >= bh: b += 1                                  # the walking tones skip the HOME note's position (owner O-5)
        else: b = 0
    elif sh == "OCTV": b, octv = m // 2, m % 2 == 1
    elif sh == "REPT": b = m // 2
    else: b = (m // 3 + m % 3) % LB
    j = jmap(b)
    s = T[j % nt] + 12 * (j // nt) + (12 if octv else 0)
    if home: s = ch["drone"] if ch["drone"] >= 0 else ch["vt"][0]
    while s > 60: s -= 12
    return 36 + s


def line(chords, span, d, sh, r, steps):
    out = []
    for t in range(steps):
        ch = chords[(t // 16) % len(chords)]
        if t % STP[r]:
            out.append("-"); continue
        n0 = (t // STP[r]) * SUB[r]
        ns = [note_n(ch, n0 + k, span, d, sh) for k in range(SUB[r])]
        out.append("%d" % ns[0] + "".join(("+" if k == 1 else "/") + "%d" % x for k, x in enumerate(ns[1:], 1)))
    return " ".join(out)


def main(path):
    sec, chords, cases, bad = None, [], 0, 0
    for ln in open(path):
        if ln.startswith("## "):
            sec = "I" if ln.startswith("## ARP GRID chord I") else "1451" if ln.startswith("## ARP GRID 1451") else None
            chords = []
        elif sec and ln.startswith("chord "):
            m = re.search(r"root=(-?\d+) drone=(-?\d+) n=\d+ vt=([\d,-]+)", ln)
            chords.append(dict(root=int(m.group(1)), drone=int(m.group(2)), vt=[int(x) for x in m.group(3).split(",")]))
        elif sec and ln.startswith("SPAN="):
            m = re.match(r"SPAN=(\d) DIR=(\w+) SHAPE=(\w+) RATE=(\S+) \|(.*)", ln)
            span, d, sh, rt, cells = int(m.group(1)), m.group(2), m.group(3), m.group(4), m.group(5).split()
            want = line(chords if sec == "1451" else chords[:1], span, d, sh, RATES.index(rt), 64 if sec == "1451" else 16).split()
            cases += 1
            if cells != want:
                bad += 1
                if bad <= 5: print("arp_ref: %s SPAN=%d %s %s %s\n  golden: %s\n  ref:    %s" % (sec, span, d, sh, rt, " ".join(cells), " ".join(want)))
    print("arp_ref: %d cases, %d mismatches" % (cases, bad))
    return 1 if bad or cases != 2 * 864 else 0


sys.exit(main(sys.argv[1]))
