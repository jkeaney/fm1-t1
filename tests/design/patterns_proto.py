#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""P2-014: reference prototype of the PATTERNS algorithm (docs/design/algorithms/patterns/SPEC.md). Integer-only, deterministic.

  python3 tests/design/patterns_proto.py            print the library summary and storage cost
  python3 tests/design/patterns_proto.py --check    all checks (library shape, neutral identity, 16 scales x 12 keys x 5 octaves range
                                                    sweep, morph goldens in patterns_golden.txt, song-title grep); exit 1 on any failure
  python3 tests/design/patterns_proto.py --golden   rewrite patterns_golden.txt (an intended change only)
Semitone space as ALGORITHM_SET SP9: 0 = the key note at the base octave; midi = clamp(36 + key + 12 * octave + s, 0, 127).
"""
import os, re, sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(__file__), "..", ".."))
PD = os.path.join(ROOT, "docs", "design", "algorithms", "patterns")
MASKS = [0x0FFF, 0x0AB5, 0x05AD, 0x06AD, 0x05AB, 0x0AD5, 0x06B5, 0x05AD, 0x056B, 0x0295, 0x04A9, 0x04E9, 0x0555, 0x09AD, 0x0AAD, 0x06DB]
SCNAME = ["CHROM", "MAJOR", "NATMIN", "DORIAN", "PHRYG", "LYDIAN", "MIXOLY", "AEOLIAN", "LOCRIAN", "PENMAJ", "MINPEN", "BLUES", "WHOLE", "HARMIN", "MELMIN", "DIM"]
MINPEN = 10
NEUTRAL = dict(dens=8, vary=0, span=4, accnt=4, slide=4, dyn=10, ctone=2)
RANGE = dict(dens=(0, 16), vary=(0, 8), span=(1, 4), accnt=(0, 8), slide=(0, 8), dyn=(0, 14), ctone=(0, 8))
TOK = re.compile(r"^(~)?(-?\d+)([ASTslC]*)(?:\^?([123]))?$")
CATS = ["ACID", "HOUSE", "FUNK", "TECHNO", "DUB", "MELODY"]


def parse_steps(txt):
    out = []
    for t in txt.replace("|", " ").split():
        if t == ".":
            out.append(None)
            continue
        m = TOK.match(t)
        if not m:
            raise ValueError("bad token %r" % t)
        opt, p, fl, rk = m.groups()
        rank = int(rk) if rk else (2 if opt else 0)
        out.append(dict(opt=bool(opt), p=int(p), acc="A" in fl, sld="S" in fl, tie="T" in fl, gate=("s" in fl) and 0 or (("l" in fl) and 2 or 1),
                        ct="C" in fl, rank=rank))
    return out


def load():
    lib = []
    for ln in open(os.path.join(PD, "patterns.tsv"), encoding="utf-8"):
        if ln.startswith("#") or ln.startswith("id\t") or not ln.strip():
            continue
        c = ln.rstrip("\n").split("\t")
        st = parse_steps(c[7])
        assert len(st) == int(c[6]), (c[3], len(st), c[6])
        lib.append(dict(id=int(c[0]), cat=c[1], num=int(c[2]), name=c[3], mode=c[4], src=c[5], len=len(st), steps=st, raw=c[7]))
    return lib


# ---- shared maths ------------------------------------------------------------------------------------------------------------
def tones(mask):
    return [k for k in range(12) if mask >> k & 1]


def snap(mask, x):
    """SP6: nearest scale tone, ties to the lower (any octave)."""
    t = tones(mask)
    best = None
    for q in range(-2, 9):
        for k in t:
            c = k + 12 * q
            d = abs(c - x)
            if best is None or d < best[0] or (d == best[0] and c < best[1]):
                best = (d, c)
    return best[1]


def neighbour(mask, s, up):
    """next scale tone strictly above (up) or below s (s is in scale)."""
    t = tones(mask)
    for d in range(1, 13):
        x = s + d if up else s - d
        if (x % 12) in t:
            return x
    return s


def deg_to_semi(mask, d):
    t = tones(mask)
    return t[d % len(t)] + 12 * (d // len(t))


def h16(seed, i, salt):
    x = (seed * 0x9E37 + i * 0x85EB + salt * 0xC2B3 + 0x1234) & 0xFFFF
    x ^= x >> 7
    x = (x * 0x2C1B) & 0xFFFF
    x ^= x >> 9
    x = (x * 0x6B43) & 0xFFFF
    x ^= x >> 8
    return x & 0xFFFF


def chord_pull(pcs, s):
    """HARMONY.md 9 harm_snap(HQ_PC): nearest chord-tone pitch class in any octave, ties lower, clamped 0..96."""
    best = None
    for q in range(0, 9):   # candidates are kept >= 0 so a tie never falls below the key's base octave
        for k in range(12):
            if pcs >> k & 1:
                c = k + 12 * q
                d = abs(c - s)
                if best is None or d < best[0] or (d == best[0] and c < best[1]):
                    best = (d, c)
    return max(0, min(96, best[1]))


def order(items, key):
    return sorted(items, key=key)


def render(pat, mask, prm=None, seeds=(1, 2, 3, 4, 5), chord_pcs=0):
    """One pass of the pattern: list of per-step dicts (or None) = hit, s (semitones above key), vel, gate, slide, tie."""
    P = dict(NEUTRAL)
    P.update(prm or {})
    S = pat["steps"]
    n = len(S)
    s_notes, s_hits, s_acc, s_art, s_shape = seeds
    if P["dens"] == 0:
        return [None] * n
    # 1. which steps sound (DENS): deco (rank>0, ON) thin out, optional (opt) fill in; anchors always stay
    on = [st is not None and not st["opt"] for st in S]
    deco = order([i for i in range(n) if on[i] and S[i]["rank"] > 0], key=lambda i: (S[i]["rank"], h16(s_hits, i, 1) >> 8, i))
    opts = order([i for i in range(n) if S[i] is not None and S[i]["opt"]], key=lambda i: (S[i]["rank"], h16(s_hits, i, 1) >> 8, i))
    if P["dens"] < 8:
        keep = len(deco) * (P["dens"] - 1) // 7
        for i in deco[::-1][:len(deco) - keep]:   # highest rank / priority key removed first
            on[i] = False
    elif P["dens"] > 8:
        add = len(opts) * (P["dens"] - 8) // 8
        for i in opts[:add]:
            on[i] = True
    # 2. pitch: base semitone, SPAN fold, VARY, SP6 snap
    base = {}
    for i in range(n):
        if S[i] is not None:
            base[i] = deg_to_semi(mask, S[i]["p"]) if pat["mode"] == "DEG" else snap(mask, S[i]["p"])
    pmin = min(base.values())
    pitch = {}
    for i in range(n):
        if not on[i] and not (S[i] is not None and S[i]["opt"]):
            continue
        s = base[i]
        while s - pmin >= 12 * P["span"]:
            s -= 12
        if S[i]["rank"] > 0 and (h16(s_notes, i, 2) % 8) < P["vary"]:
            s = neighbour(mask, snap(mask, s), h16(s_shape, i, 3) & 1)
        s = snap(mask, s)
        if s < 0:
            s += 12
        pitch[i] = s
    # 3. chord-tone bias (only while HARMONY is active: chord_pcs != 0)
    pulled = set()
    if chord_pcs:
        flag = [i for i in range(n) if on[i] and S[i]["ct"]]
        other = order([i for i in range(n) if on[i] and not S[i]["ct"]], key=lambda i: (S[i]["rank"], h16(s_notes, i, 5), i))
        c = P["ctone"]
        if c == 0:
            pass
        elif c == 1:
            pulled = set(flag[:(len(flag) + 1) // 2])
        else:
            pulled = set(flag)
            if c > 2:
                pulled |= set(other[:-(-len(other) * (c - 2) // 6)])
        for i in pulled:
            pitch[i] = chord_pull(chord_pcs, pitch[i])
    # 4. accents (ACCNT 4 = as written), slides (SLIDE 4 = as written), velocity (DYN)
    acc = {i: S[i]["acc"] for i in range(n) if on[i]}
    sld = {i: S[i]["sld"] for i in range(n) if on[i]}
    a = P["accnt"]
    A = order([i for i in acc if acc[i]], key=lambda i: (h16(s_acc, i, 4), i))
    N = order([i for i in acc if not acc[i]], key=lambda i: (h16(s_acc, i, 4), i))
    if a < 4:
        for i in A[:len(A) * (4 - a) // 4]:
            acc[i] = False
    elif a > 4:
        for i in N[:len(N) * (a - 4) // 8]:
            acc[i] = True
    sl = P["slide"]
    SS = order([i for i in sld if sld[i]], key=lambda i: (h16(s_art, i, 6), i))
    cand = order([i for i in sld if not sld[i] and on[(i + 1) % n]], key=lambda i: (h16(s_art, i, 6), i))
    if sl < 4:
        for i in SS[:len(SS) * (4 - sl) // 4]:
            sld[i] = False
    elif sl > 4:
        for i in cand[:len(cand) * (sl - 4) // 4]:
            sld[i] = True
    out = []
    for i in range(n):
        if not on[i]:
            out.append(None)
            continue
        v = 120 if acc[i] else 80
        d = P["dyn"]
        v = 100 + (v - 100) * min(d, 10) // 10
        if d > 10:
            v += ((h16(s_acc, i, 7) % 7) - 3) * (d - 10) // 2
        out.append(dict(s=pitch[i], vel=max(1, min(127, v)), gate=3 if S[i]["tie"] else S[i]["gate"], sld=sld[i], acc=acc[i]))
    return out


def fmt(pat, steps, mask=MASKS[MINPEN]):
    cells = []
    for r in steps:
        if r is None:
            cells.append(".")
        else:
            cells.append("%d%s%s%s:%d" % (r["s"], "A" if r["acc"] else "", "S" if r["sld"] else "", "T" if r["gate"] == 3 else "", r["vel"]))
    return " ".join(cells)


# ---- checks ------------------------------------------------------------------------------------------------------------------
def library_checks(lib, bad):
    if sorted({p["cat"] for p in lib}) != sorted(CATS):
        bad.append("categories differ from SPEC")
    for c in CATS:
        if sum(1 for p in lib if p["cat"] == c) != 20:
            bad.append("%s does not have 20 patterns" % c)
    names = set()
    for p in lib:
        if p["name"] in names:
            bad.append("duplicate name " + p["name"])
        names.add(p["name"])
        if p["len"] not in (12, 16, 32):
            bad.append("%s: len %d" % (p["name"], p["len"]))
        S = p["steps"]
        anchors = [s for s in S if s and not s["opt"] and s["rank"] == 0]
        extra = [s for s in S if s and (s["opt"] or s["rank"] > 0)]
        if not anchors:
            bad.append("%s: no anchor step" % p["name"])
        if len(extra) < 1:
            bad.append("%s: no decoration/optional step" % p["name"])
        if not any(s and s["ct"] for s in S):
            bad.append("%s: no chord-tone flag" % p["name"])
        for s in S:
            if s and not (0 <= s["p"] <= (14 if p["mode"] == "DEG" else 48)):
                bad.append("%s: pitch %d out of range" % (p["name"], s["p"]))
        if p["mode"] == "SEMI":
            if max(s["p"] for s in S if s) - min(s["p"] for s in S if s) >= 48:
                bad.append("%s: span >= 4 octaves" % p["name"])
        if p["src"] not in ("F", "G"):
            bad.append("%s: src" % p["name"])


def sweep(lib, bad):
    """every pattern x 16 scales x 12 keys x 5 octaves: all notes in the scale (unless pulled by a chord) and inside MIDI 0..127,
    at the neutral dials and at every dial extreme."""
    extremes = [dict(NEUTRAL)]
    for k, (lo, hi) in RANGE.items():
        for v in (lo, hi):
            d = dict(NEUTRAL)
            d[k] = v
            extremes.append(d)
    extremes.append(dict(dens=16, vary=8, span=1, accnt=8, slide=8, dyn=14, ctone=8))
    n = 0
    for p in lib:
        for sc, mask in enumerate(MASKS):
            for prm in extremes:
                for pcs in (0, 0b000010010001):  # inert, and a major triad chord
                    steps = render(p, mask, prm, chord_pcs=pcs)
                    for r in steps:
                        if r is None:
                            continue
                        n += 1
                        if not (0 <= r["s"] <= 96):
                            bad.append("%s %s: s=%d outside 0..96" % (p["name"], SCNAME[sc], r["s"]))
                            return
                        if pcs == 0 and not (mask >> (r["s"] % 12) & 1):
                            bad.append("%s %s %s: s=%d not in scale" % (p["name"], SCNAME[sc], prm, r["s"]))
                            return
                        for key in range(12):
                            for octv in range(-2, 3):
                                m = 36 + key + 12 * octv + r["s"]
                                if not 0 <= m <= 127:
                                    bad.append("%s: midi %d out of range (key %d oct %d)" % (p["name"], m, key, octv))
                                    return
    return n


def neutral_identity(lib, bad):
    """dials at their defaults = the stored pattern with only the SP6 snap applied (anchor, decoration, accent, slide kept)."""
    for p in lib:
        for sc in (1, MINPEN, 0):
            mask = MASKS[sc]
            got = render(p, mask)
            for i, st in enumerate(p["steps"]):
                g = got[i]
                if st is None or st["opt"]:
                    if g is not None:
                        bad.append("%s: neutral sounds a rest/optional at %d" % (p["name"], i))
                    continue
                want = deg_to_semi(mask, st["p"]) if p["mode"] == "DEG" else snap(mask, st["p"])
                if g is None or g["s"] != want or g["acc"] != st["acc"] or g["sld"] != st["sld"]:
                    bad.append("%s: neutral differs at step %d" % (p["name"], i))
                    break
        # the 12-key rotation is absorbed by the global key: s is key-independent
    # dens 1 = anchors only
    for p in lib:
        got = render(p, MASKS[MINPEN], dict(dens=1))
        want = [i for i, st in enumerate(p["steps"]) if st and not st["opt"] and st["rank"] == 0]
        have = [i for i, g in enumerate(got) if g]
        if want != have:
            bad.append("%s: DENS 1 is not the anchors" % p["name"])


GOLD_PATTERNS = ["ACID 1", "HOUSE 3", "FUNK 4", "TECHNO 1", "DUB 3", "MELODY 1"]
GOLD_CASES = [("DENS", "dens", (1, 8, 16)), ("VARY", "vary", (0, 4, 8)), ("SPAN", "span", (1, 2, 4)), ("ACCNT", "accnt", (0, 4, 8)),
              ("SLIDE", "slide", (0, 4, 8)), ("DYN", "dyn", (0, 10, 14)), ("CTONE", "ctone", (0, 2, 8))]
CHORD_G = 0b000010010001 << 0  # placeholder replaced below


def golden_text(lib):
    mask = MASKS[MINPEN]
    # chord for the CTONE cases: degree 4 of MINPEN (G), triad G Bb D -> pitch classes {7, 10, 2}... MINPEN snaps D to Eb/F: use G Bb F
    pcs = (1 << 7) | (1 << 10) | (1 << 5)
    by = {p["name"]: p for p in lib}
    L = ["# patterns_golden.txt: MINPEN, key C, seeds 1 2 3 4 5; output = semitone above the key, A accent, S slide, T tie, :velocity, . rest.",
         "# CTONE cases use the chord G Bb F (pitch classes 7 10 5). Regenerate: python3 tests/design/patterns_proto.py --golden"]
    for nm in GOLD_PATTERNS:
        p = by[nm]
        L.append("")
        L.append("== %s (%s, len %d): %s" % (nm, p["mode"], p["len"], p["raw"]))
        for lab, k, vals in GOLD_CASES:
            for v in vals:
                st = render(p, mask, {k: v}, chord_pcs=pcs if k == "ctone" else 0)
                L.append("%-6s %-2d | %s" % (lab, v, fmt(p, st)))
    return "\n".join(L) + "\n"


def title_grep(bad):
    txt = open(os.path.join(PD, "RESEARCH.md"), encoding="utf-8").read()
    m = re.search(r"<!-- TITLES -->(.*?)<!-- /TITLES -->", txt, re.S)
    if not m:
        bad.append("RESEARCH.md has no TITLES block")
        return 0
    titles = [ln[2:].split("|")[0].strip() for ln in m.group(1).splitlines() if ln.startswith("- ")]
    if len(titles) < 10:
        bad.append("TITLES block too short")
    files = [os.path.join(PD, "patterns.tsv")]
    for base, _, fs in os.walk(os.path.join(ROOT, "firmware", "src")):
        files += [os.path.join(base, f) for f in fs if f.endswith((".c", ".h", ".tsv"))]
    for f in os.listdir(os.path.join(ROOT, "docs", "design")):
        if f.endswith(".tsv"):
            files.append(os.path.join(ROOT, "docs", "design", f))
    for f in files:
        low = open(f, encoding="utf-8", errors="replace").read().lower()
        for t in titles:
            if t.lower() in low:
                bad.append("song title %r found in %s" % (t, os.path.relpath(f, ROOT)))
    return len(titles)


def storage(lib):
    steps = sum(p["len"] for p in lib)
    return dict(patterns=len(lib), steps=steps, blob=2 * steps, index=4 * len(lib), total=2 * steps + 4 * len(lib))


def main():
    lib = load()
    st = storage(lib)
    print("library: %(patterns)d patterns, %(steps)d steps, step blob %(blob)d B + index %(index)d B = %(total)d B flash" % st)
    if "--golden" in sys.argv:
        open(os.path.join(PD, "patterns_golden.txt"), "w").write(golden_text(lib))
        print("wrote patterns_golden.txt")
        return 0
    if "--check" not in sys.argv:
        return 0
    bad = []
    library_checks(lib, bad)
    neutral_identity(lib, bad)
    n = sweep(lib, bad)
    gp = os.path.join(PD, "patterns_golden.txt")
    if not os.path.exists(gp) or open(gp).read() != golden_text(lib):
        bad.append("patterns_golden.txt differs from the prototype (run --golden for an intended change)")
    nt = title_grep(bad)
    if bad:
        print("\n".join(bad[:40]))
        print("FAIL: %d problems" % len(bad))
        return 1
    print("OK: library, neutral identity, %s note checks over 16 scales x 12 keys x 5 octaves, goldens, %d song titles absent from firmware strings" % (n, nt))
    return 0


if __name__ == "__main__":
    sys.exit(main())
