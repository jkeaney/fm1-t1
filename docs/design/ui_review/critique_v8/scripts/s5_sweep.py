import re, sys, os, io, itertools, copy, contextlib, traceback
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from common import *
import el
S = scenes()
palette_set("HI-CON")
print("A. algos_v8.ORDER =", A.ORDER)
print("   algos_v8.check() ->", A.check(text_w, AF_M, AF_L))
print("\nB. VALUE-TEXT SWEEP: every algorithm x row x knob x value (tile usable width 54 px; tile 58 px)")
n_vals = 0; n_L = 0; n_M = 0; neither = []; wide_M = []; maxL = (0, ""); label_w = []
ink_over = []
for a in A.ORDER:
    for r, row in enumerate(A.ALGOS[a]["rows"]):
        for i, k in enumerate(row):
            lw = text_w(AF_M, k["label"]); label_w.append((lw, a, r+1, i+1, k["label"]))
            for v in range(k["min"], k["max"]+1):
                t = A.value_text(k, v); n_vals += 1
                wl, wm = text_w(AF_L, t), text_w(AF_M, t)
                if wl <= 54: n_L += 1; maxL = max(maxL, (wl, t))
                else:
                    n_M += 1
                    if wm > 54: neither.append((a, r+1, i+1, k["label"], v, t, wl, wm))
                    wide_M.append((a, r+1, i+1, k["label"], v, t, wl, wm))
                    ink = text_ink(AF_M, t) if False else None
print("   value strings checked:", n_vals, "| drawn in AF_L:", n_L, "| fell back to AF_M:", n_M, "| fit neither (AF_M adv >54):", len(neither))
print("   widest AF_L accepted:", maxL)
print("   AF_M fallbacks (algo,row,knob,label,value,text,AF_L adv,AF_M adv) distinct texts:")
seen = {}
for w in wide_M: seen.setdefault((w[0], w[1], w[2], w[3], w[5]), w)
for key, w in sorted(seen.items(), key=lambda kv: -kv[1][7])[:3]: print('     ', w)
print("   ...total distinct AF_M-fallback (algo,row,knob,text):", len(seen))
print("   fit-neither: %d values, all PATTERNS PICK CAT 100..CAT 120 (AF_M adv 58 = tile width 58, usable 54)" % len(neither) if neither else "   fit-neither: none")
print("   ink width (px) of AF_M 'CAT 100':", (lambda b: b[2]-b[0])(mock.text_ink(AF_M, "CAT 100")), " AF_M 'CAT 99' adv", text_w(AF_M, "CAT 99"), " AF_M 'CAT 10' adv", text_w(AF_M, "CAT 10"))
lw = sorted(label_w, reverse=True)[:6]
print("   widest labels (AF_M adv; tile 58, usable 54):", lw, " labels >54:", [x for x in label_w if x[0] > 54])
# real render check: knob band with each value, canvas warnings
import mock as M
cut = []; nrend = 0
for a in A.ORDER:
    for r in range(2):
        row = A.ALGOS[a]["rows"][r]
        top = max(k["max"]-k["min"] for k in row)
        for step in range(top+1):
            vals = {i: min(k["max"], k["min"]+step) for i, k in enumerate(row)}
            st = {"chans": copy.deepcopy(mod.CHANS), "used": [0], "sel": 0, "playing": True, "row": r, "vals": vals}
            st["chans"][0]["algo"] = a
            sc = M.Screen(); cv = M.Canvas(sc); cv.begin(240, 120, T.BG)
            el.paint(cv, [(0, 120)], lambda p: mod.knobs(p, st))
            nrend += 1
            for w in cv.warnings:
                if 'CV_MAX' not in w: cut.append((a, r+1, step, w))
            # ink vs tile boundary check from recorded text boxes
            for t in sc.texts:
                a0, b0, c0, d0 = t[:4]
                k = (a0 + c0)//2 // 60; x0 = k*60+1
                if a0 < x0 or c0 > x0+58: ink_over.append((a, r+1, step, t[4], (a0, c0), (x0, x0+58)))
print("   real knobs() renders:", nrend, "| canvas warnings (cut/ellipsis):", len(cut), cut[:5], "| ink outside tile:", len(ink_over), ink_over[:5])

print("\nC. HEADER SWEEP: every algo short name x MIDI (OFF,1..16) x preset (P01 / pending P16?)")
hits = []; n = 0; maxend = (0,)
def header_rects(algo, midi, pend):
    st = {"chans": copy.deepcopy(mod.CHANS), "used": [0,1,2,3], "sel": 0, "playing": True, "pend": pend, "preset": 16}
    st["chans"][0]["algo"] = algo; st["chans"][0]["midi"] = midi
    import common; common.REC.clear(); common._pending.clear()
    sc = M.Screen(); cv = M.Canvas(sc); cv.begin(240, 120, T.BG)
    el.paint(cv, [(0, 120)], lambda p: mod.header(p, st))
    return [t for t in sc.texts], cv.warnings
for algo in A.ORDER:
    for midi in [None] + list(range(1, 17)):
        for pend in (None, 16):
            n += 1
            tx, warn = header_rects(algo, midi, pend)
            boxes = [(t[4], t[0], t[2]) for t in tx if t[1] < 21]
            nm = [b for b in boxes if not re.fullmatch(r"P\d\d\??", b[0]) and b[1] >= 100]       # algo, MIDI, value
            ptxt = [b for b in boxes if re.fullmatch(r"P\d\d\??", b[0])]
            warn=[w for w in warn if 'CV_MAX' not in w]
            if warn: hits.append((algo, midi, pend, "warn", warn))
            right_end = max(b[2] for b in nm)
            maxend = max(maxend, (right_end, algo, midi))
            limit = 197 if pend else ptxt[0][1]
            if right_end >= limit - 0: hits.append((algo, midi, pend, "overlap: left group ends %d, right group starts %d" % (right_end, limit)))
            # pairwise
            for x, y in itertools.combinations(boxes, 2):
                if x[1] < y[2] and y[1] < x[2]: hits.append((algo, midi, pend, "ink overlap", x, y))
print("   combos:", n, "| problems:", len(hits))
for h in hits[:12]: print("     ", h)
print("   rightmost end of algo+MIDI group:", maxend, "| pending frame starts x=197; P01 text starts x=209; 'P16?' ink:", [ (t[4],t[0],t[2]) for t in header_rects('ACID',1,16)[0] if re.fullmatch(r'P\d\d\??', t[4])])
print("   per-algo header group end (MIDI OFF / MIDI 16 / MIDI 1):")
for algo in A.ORDER:
    ends = []
    for midi in (None, 16, 1):
        tx, _ = header_rects(algo, midi, None)
        ends.append(max(t[2] for t in tx if t[1] < 21 and t[0] >= 100 and not re.fullmatch(r"P\d\d\??", t[4])))
    print("     %-9s short=%-6r adv=%d  group ends: OFF %d / 16 %d / 1 %d" % (algo, A.ALGOS[algo]["short"], text_w(AF_M, A.ALGOS[algo]["short"]), *ends))
by = {}
for h in hits:
    by.setdefault((h[0], h[2]), []).append(h[1])
print("   problem summary (algo, pending) -> MIDI values with overlap/touch:")
for k, v in sorted(by.items(), key=lambda kv: str(kv[0])): print("     ", k, sorted(set(map(str, v))))
print("   pending=None problems:", [h for h in hits if h[2] is None] or "none")
