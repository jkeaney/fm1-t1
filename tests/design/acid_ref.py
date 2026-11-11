#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""P2-008: an independent re-implementation of ACID written from docs/design/algorithms/acid/SPEC.md (not from proto.c).
Usage: acid_ref.py golden.txt   (checks every plain case of the golden file: t note vel gate flags for t = 0..127).
It shares no code with proto.c, so a golden file that passes both is the spec's output, not one reading of it."""
import re
import sys

M = 0xFFFFFFFF


class R:
    def __init__(s, seed, salt):
        x = ((seed << 16) | seed) ^ salt
        s.x = (x & M) or 1
        for _ in range(4):
            s.nx()

    def nx(s):
        x = s.x
        x ^= (x << 13) & M
        x ^= x >> 17
        x ^= (x << 5) & M
        s.x = x
        return x

    def rnd(s, n):
        return (s.nx() >> 8) % n

    def bern(s, p):
        return s.rnd(100) < p

    def shuf(s, a):
        for i in range(len(a) - 1, 0, -1):
            j = s.rnd(i + 1)
            a[i], a[j] = a[j], a[i]


BP = [10, 11, 12, 13, 15, 16, 17, 18, 20, 21, 22, 23, 25, 26, 27, 28]
PA = [0, 8, 11, 14, 17, 20, 23, 26, 29, 32, 35, 38, 41, 44, 47, 50]
PB = [0, 11, 13, 14, 16, 18, 20, 22, 24, 26, 28, 31, 33, 35, 38, 40]
PS = [0, 20, 10, 25, 8, 20, 10, 25, 8, 20, 10, 25, 8, 20, 10, 30]
MASKS = [0x0FFF, 0x0AB5, 0x05AD, 0x06AD, 0x05AB, 0x0AD5, 0x06B5, 0x05AD, 0x056B, 0x0295, 0x04A9, 0x04E9, 0x0555, 0x09AD, 0x0AAD, 0x06DB]
SC = {"CHROM": 0, "MAJOR": 1, "MINPEN": 10}


def acid(m, seed, scale, key, octv):
    den, typ, jmp, acc, off, sl, pun, dr = m          # morph ids 0..7
    r = R(seed[0], 0x5A17C3E1)
    n = 3 + r.rnd(3)
    bag = list(range(12))
    r.shuf(bag)
    tile = [bag[:n][i % n] for i in range(16)]
    r.shuf(tile)
    ot = list(range(16))
    r.shuf(ot)
    pc = tile[:]
    for k in range(typ):
        pc[ot[k]] = 0
    r = R(seed[1], 0x2C9E6B07)
    rest = list(range(1, 16))
    r.shuf(rest)
    rank = {c: i for i, c in enumerate([0] + rest)}
    hit = [rank[i] < den for i in range(16)]
    r = R(seed[2], 0x71D4A5F3)
    octs = []
    for i in range(16):
        u1, u2, d = r.rnd(100), r.rnd(100), r.rnd(3) - 1
        pe = min(100, BP[i] * jmp // 5)
        octs.append(max(-1, d * ((u1 < pe) + (u2 < pe))))
    r = R(seed[3], 0x3B8F1D29)
    gate = [79 if r.rnd(2) == 0 else 55]
    for i in range(1, 16):
        gate.append([24, 55, 100][int(r.bern(PA[i])) + int(r.bern(PB[i]))])
    r = R(seed[4], 0x68E2B94D)
    oa = list(range(16))
    r.shuf(oa)
    accd, k = set(), 0
    for c in oa:
        if k < acc and hit[c]:
            accd.add(c)
            k += 1
    vel = [min(127, 80 + pun) if i in accd else 80 for i in range(16)]
    r = R(seed[5], 0x4F6A9C15)
    sld = [r.rnd(100) < min(100, PS[i] * sl // 4) for i in range(16)]
    tones = [t for t in range(12) if MASKS[scale] >> t & 1]

    def snap(x):
        best = None
        for sh in (-12, 0, 12):
            for t in tones:
                c = t + sh
                d = abs(c - x)
                if best is None or d < best[0] or (d == best[0] and c < best[1]):
                    best = (d, c)
        return best[1]

    out = []
    for t in range(128):
        u = t + off
        if dr == 0:
            i = u % 16
        elif dr == 1:
            i = 15 - u % 16
        else:
            q = u % 30
            i = q if q < 16 else 30 - q
        semi = pc[i] + 12 * octs[i]
        q = semi // 12
        note = min(127, max(0, 36 + key + 12 * octv + 12 * q + snap(semi - 12 * q)))
        out.append((t, note, vel[i], gate[i], (1 if hit[i] else 0) | (2 if sld[i] else 0)))
    return out


def main():
    blocks = open(sys.argv[1]).read().split("\n## ")
    n = bad = 0
    for blk in blocks:
        lines = blk.split("\n")
        mm = re.search(r"scale=(\w+) key=(\d+) oct=(-?\d+) m=([\d,]+) seeds=([0-9a-f,]+)", lines[0])
        if not mm or not lines[1].startswith("t note"):
            continue
        m = list(map(int, mm.group(4).split(",")))
        sd = [int(x, 16) for x in mm.group(5).split(",")]
        ref = acid(m, sd, SC[mm.group(1)], int(mm.group(2)), int(mm.group(3)))
        got = [tuple(int(x, 16) if j == 4 else int(x) for j, x in enumerate(l.split()[:5])) for l in lines[2:130]]
        n += 1
        if got != ref:
            bad += 1
            print("MISMATCH", lines[0])
    print("acid_ref: %d cases, %d mismatches" % (n, bad))
    sys.exit(1 if bad or not n else 0)


if __name__ == "__main__":
    main()
