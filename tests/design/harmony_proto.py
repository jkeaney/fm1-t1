#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""P2-012: reference prototype of the HARMONY maths (docs/design/HARMONY.md). Integer-only, deterministic.

It is the golden source for every worked example in HARMONY.md: the chord builder (VOICE), the voice leader (MOVE), the
drone, the chord-tone pull and the progression dice.  `python3 tests/design/harmony_proto.py` prints the tables; with
`--check` it asserts the golden values quoted in HARMONY.md and exits 1 on any difference.  Semitone space: 0 = the key
note at the base octave (MIDI 36 + key + 12 * OCTAVE), as ALGORITHM_SET SP9.
"""
import sys

SCALES = {  # pitch-class offsets from the key (T1_SCALE_MASKS order is in firmware; these are the ones the examples use)
    "MAJOR": [0, 2, 4, 5, 7, 9, 11],
    "NATMIN": [0, 2, 3, 5, 7, 8, 10],
    "MINPEN": [0, 3, 5, 7, 10],
}
TPL = [0, 4, 7, 11, 14]            # root, third, fifth, seventh, ninth above the chord root (target semitones)
# VOICE 0..6: list of (template index, octave shift); sorted ascending after building.  Default VOICE = 2.
VOICE = {
    0: [(0, 0)],
    1: [(0, 0), (2, 0)],
    2: [(0, 0), (1, 0), (2, 0)],
    3: [(0, 0), (1, 0), (2, 0), (3, 0)],
    4: [(0, 0), (1, 0), (2, 0), (3, 0), (4, 0)],
    5: [(0, 0), (2, 0), (3, 0), (4, 0), (1, 1)],
    6: [(0, 0), (3, 0), (1, 1), (2, 1), (4, 1)],
}
NAMES = ["C", "Db", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B"]


def tones(sc):
    return sc


def snap(sc, x):
    """nearest scale tone to semitone x (any octave), ties to the lower (SP6)"""
    best = None
    for q in range(-2, 9):
        for t in sc:
            c = t + 12 * q
            d = abs(c - x)
            if best is None or d < best[0] or (d == best[0] and c < best[1]):
                best = (d, c)
    return best[1]


def degree_root(sc, d):
    i = d - 1
    return sc[i % len(sc)] + 12 * (i // len(sc))


def root_position(sc, d, voice):
    r = degree_root(sc, d)
    out, pcs = [], set()
    for k, o in VOICE[voice]:
        n = snap(sc, r + TPL[k]) + 12 * o
        if n % 12 in pcs:
            continue            # a duplicate pitch class is dropped (keeps the lowest)
        pcs.add(n % 12)
        out.append(n)
    return sorted(out)


def invert(w, k):
    w = list(w)
    for _ in range(k):
        w.sort()
        w[0] += 12
    return sorted(w)


def lead(prev, w, move):
    """pick the placement of chord w after voicing prev: min cost = dist*(8-move) + move*4*inversion"""
    if prev is None or move == 8:
        return list(w)
    best = None
    for inv in range(len(w)):
        base = invert(w, inv)
        for q in (0, -12):
            c = [n + q for n in base]
            if min(c) < 0 or max(c) > 48:
                continue
            dist = sum(abs(a - b) for a, b in zip(c, prev))
            cost = dist * (8 - move) + move * 4 * inv
            key = (cost, inv, abs(q), q)
            if best is None or key < best[0]:
                best = (key, c)
    return best[1]


def progression(sc, degs, voice, move):
    out, prev = [], None
    for d in degs:
        w = root_position(sc, d, voice)
        v = lead(prev, w, move)
        out.append(v)
        prev = v
    return out


def drone_pitch(sc, p):
    if p == 0:
        return None
    np_ = min(7, len(sc))             # HARMONY.md 3.3: n' degrees, then the same again an octave up, clamped to 2n'
    p = min(p, 2 * np_)
    return degree_root(sc, (p - 1) % np_ + 1) + (12 if p > np_ else 0)


def drone_mark(sc, chord, ped):
    """'in' = drone pitch class is a chord tone, 'clash' = a semitone from one, else 'tension'"""
    pcs = {n % 12 for n in chord}
    if ped % 12 in pcs:
        return "in"
    if any((ped - n) % 12 in (1, 11) for n in chord):
        return "clash"
    return "tension"


def hsh(t):
    return ((t * 0x9E3779B1) & 0xFFFFFFFF) >> 29


def nearest_pc(pcs, x):
    """nearest pitch of a pitch-class set to x, any octave, ties lower"""
    best = None
    for q in range(-2, 9):
        for p in pcs:
            c = p + 12 * q
            d = abs(c - x)
            if best is None or d < best[0] or (d == best[0] and c < best[1]):
                best = (d, c)
    return best[1]


def pull_note(pcs, root, s, t, strong, pull, first, hi):
    """AUTO role: chord-tone pull of one generated note.  first = first hit after a chord change (the anchor)."""
    if first:
        x = root + 12 * (s // 12)
    else:
        peff = min(8, pull + (4 if strong else 0))
        x = nearest_pc(pcs, s) if hsh(t) < peff else s
    while x > hi:
        x -= 12
    while x < 0:
        x += 12
    return x


class Rng:
    def __init__(self, seed16):
        s = ((seed16 << 16) | seed16) ^ 0x4841524D
        self.s = s & 0xFFFFFFFF or 1

    def nxt(self):
        s = self.s
        s ^= (s << 13) & 0xFFFFFFFF
        s ^= s >> 17
        s ^= (s << 5) & 0xFFFFFFFF
        self.s = s
        return s

    def rnd(self, n):
        return (self.nxt() >> 8) % n


LT = [2, 4, 4, 4, 4, 4, 8, 8]
WT = [2, 2, 2, 4, 4, 3, 1]


def dice(seed16, nscale):
    r = Rng(seed16)
    dmax = min(7, nscale)
    bars = LT[r.rnd(8)]
    n = 2 + r.rnd(min(bars, 4) - 1)
    w = WT[:dmax]
    degs = []
    for i in range(n):
        for _try in range(5):
            x = r.rnd(sum(w))
            d = 0
            while x >= w[d]:
                x -= w[d]
                d += 1
            d += 1
            if not degs or d != degs[-1]:
                break
        else:
            d = degs[-1] % dmax + 1
        degs.append(d)
    a = r.rnd(4)
    if a < 3:
        degs[0] = 1
        if len(degs) > 1 and degs[1] == 1:
            degs[1] = min(5, dmax)
    else:
        degs[-1] = 1
        if degs[-2] == 1:
            degs[-2] = min(5, dmax)
    return bars, degs


def fmt(v):
    return "[" + " ".join(str(x) for x in v) + "]"


def names(v, key=0):
    return " ".join(NAMES[(key + x) % 12] for x in v)


GOLD = {}


def main():
    chk = "--check" in sys.argv
    for scn in ("MAJOR", "NATMIN"):
        sc = SCALES[scn]
        print("== %s key C, 1-4-5-1, VOICE 2" % scn)
        for mv in (8, 4, 0):
            p = progression(sc, [1, 4, 5, 1], 2, mv)
            print("MOVE %d:" % mv, "  ".join(fmt(v) + " " + names(v) for v in p))
            GOLD[(scn, 2, mv)] = p
    sc = SCALES["MAJOR"]
    print("== MAJOR key C, 1-4-5-1, VOICE 2, MOVE 0..8 (sum of voice distances between chords)")
    for mv in range(9):
        p = progression(sc, [1, 4, 5, 1], 2, mv)
        tot = sum(abs(a - b) for x, y in zip(p, p[1:]) for a, b in zip(x, y))
        print("MOVE %d:" % mv, "  ".join(fmt(v) for v in p), "dist", tot)
    print("== MAJOR key C, 1-4-5-1, MOVE 8, VOICE 0..6")
    for vo in range(7):
        p = progression(sc, [1, 4, 5, 1], vo, 8)
        print("VOICE %d:" % vo, "  ".join(fmt(v) for v in p))
        GOLD[("MAJOR", vo, 8)] = p
    print("== MAJOR key C, 1-4-5-1, MOVE 0, VOICE 3")
    print("  ".join(fmt(v) for v in progression(sc, [1, 4, 5, 1], 3, 0)))
    print("== DRONE, MAJOR, VOICE 2, MOVE 8, chords I IV V I")
    for ped in (1, 5, 8, 4):
        pp = drone_pitch(sc, ped)
        marks = [drone_mark(sc, c, pp) for c in progression(sc, [1, 4, 5, 1], 2, 8)]
        print("DRONE %d -> %d (%s)" % (ped, pp, NAMES[pp % 12]), marks)
    sc2 = SCALES["NATMIN"]
    print("== DRONE, NATMIN, chords i iv v i")
    for ped in (1, 5, 3):
        pp = drone_pitch(sc2, ped)
        marks = [drone_mark(sc2, c, pp) for c in progression(sc2, [1, 4, 5, 1], 2, 8)]
        print("DRONE %d -> %d (%s)" % (ped, pp, NAMES[pp % 12]), marks)
    print("== PULL, MAJOR chord IV (bar 2), ACID default pull 3, hi 24, s = 0 2 4 5 7 9 11 12 (t = 16..23)")
    iv = progression(sc, [1, 4, 5, 1], 2, 8)[1]
    pcs = sorted({n % 12 for n in iv})
    s_in = [0, 2, 4, 5, 7, 9, 11, 12]
    out = []
    for i, s in enumerate(s_in):
        t = 16 + i
        strong = (t % 4 == 0)
        out.append(pull_note(pcs, 5, s, t, strong, 3, i == 0, 24))
    print("hash h(t):", [hsh(16 + i) for i in range(8)])
    print("in :", s_in)
    print("out:", out)
    GOLD["pull"] = out
    print("== DICE seeds (nscale 7 / 5)")
    for sd in (0x0001, 0x1234, 0xBEEF, 0xFFFF):
        print("%04X major:" % sd, dice(sd, 7), " minpen:", dice(sd, 5))
        GOLD[("dice", sd)] = dice(sd, 7)
    if chk:
        want = {
            ("MAJOR", 2, 8): [[0, 4, 7], [5, 9, 12], [7, 11, 14], [0, 4, 7]],
            ("MAJOR", 2, 0): [[0, 4, 7], [0, 5, 9], [2, 7, 11], [4, 7, 12]],
        }
        for k, v in want.items():
            assert GOLD[k] == v, (k, GOLD[k], v)
        print("CHECK OK")


def dump():
    """--dump: one canonical integer-only text of every table in HARMONY.md 3.1, 3.2, 3.3, 4.2 and 6.1.  The C reference
    (docs/design/algorithms/common/harm.c, tests/design/harm_test.c) prints the same text; run_protos.sh diffs the two (H-GOLDEN)."""
    for scn in ("MAJOR", "NATMIN", "MINPEN"):
        sc = SCALES[scn]
        for vo in range(7):
            for mv in range(9):
                p = progression(sc, [1, 4, 5, 1], vo, mv)
                print("PROG %s v%d m%d %s" % (scn, vo, mv, " | ".join(" ".join(str(x) for x in c) for c in p)))
    for scn in ("MAJOR", "NATMIN", "MINPEN"):
        sc = SCALES[scn]
        chords = progression(sc, [1, 4, 5, 1], 2, 8)
        for ped in range(1, 15 if len(sc) == 7 else 12):   # 7-note scales walk p 1..14, 5-note scales p 1..10 (11 clamps)
            pp = drone_pitch(sc, ped)
            marks = "".join({"in": "+", "clash": "X", "tension": "-"}[drone_mark(sc, c, pp)] for c in chords)
            print("DRONE %s p%d pitch %d %s" % (scn, ped, pp, marks))
    for scn in ("MAJOR", "MINPEN"):
        sc = SCALES[scn]
        iv = progression(sc, [1, 4, 5, 1], 2, 8)[1]
        pcs = sorted({n % 12 for n in iv})
        root = degree_root(sc, 4)
        for pull in (3, 8):
            for first in (0, 1):
                for t in range(16):
                    for s in range(25):
                        print("PULL %s pull%d first%d t%d s%d -> %d" % (scn, pull, first, t, s,
                              pull_note(pcs, root, s, t, t % 4 == 0, pull, bool(first), 24)))
    for nscale in (7, 5):
        for sd in [0x0001, 0x1234, 0xBEEF, 0xFFFF] + list(range(2, 202, 3)):
            bars, degs = dice(sd, nscale)
            print("DICE n%d %04X bars %d degs %s" % (nscale, sd, bars, " ".join(str(d) for d in degs)))


if __name__ == "__main__":
    if "--dump" in sys.argv:
        dump()
    else:
        main()
