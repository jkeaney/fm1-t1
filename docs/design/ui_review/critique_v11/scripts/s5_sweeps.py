from lib import *
import traceback, itertools, io, contextlib
v = mod
palette_set("HI-CON")
print("## 5a algorithm x row x value sweep (knob tiles)")
def knobs_draw(S):
    def d(cv, _S):
        el.paint(cv, [(0, 124)], lambda p: v.knobs(p, S))
    return d
class M: pass
def render_knobs(S):
    m = M(); m.draw = lambda cv, st: el.paint(cv, [(0, 124)], lambda p: v.knobs(p, S)); m.PALETTE = "HI-CON"
    REC.clear(); _pending.clear()
    with contextlib.redirect_stdout(io.StringIO()) as buf:
        sc, cv = render_state(m, {})
    return sc, cv, list(REC), buf.getvalue()
from lib import _pending
nrender = 0; problems = []; fontsumm = []
rig_dev = v.DEVICES["TB-3 BITE"]
for name in A.ORDER:
    rows = [A.ALGOS[name]["rows"][0], A.ALGOS[name]["rows"][1], None]
    for r in range(3):
        for k in range(4):
            if r == 2:
                kd = v.rig_row({"rig": "TB-3 BITE"})[k]
            else:
                kd = A.ALGOS[name]["rows"][r][k]
                if kd is None: problems.append((name, r + 1, k + 1, "missing knob")); continue
            fonts = {}
            if r == 2 and name != A.ORDER[0]: continue      # ROW 3 comes from the device table, same for every algorithm: swept once
            for val in range(kd["min"], kd["max"] + 1):
                S = v.base(); S["chans"][0]["algo"] = name; S["row"] = r; S["vals"] = {k: val}
                if r == 2: S["rig"] = "TB-3 BITE"
                S["turning"] = (k, A.frac(kd, val))
                try:
                    sc, cv, rec, out = render_knobs(S)
                except Exception as e:
                    problems.append((name, r + 1, kd["label"], val, "EXC " + repr(e))); continue
                nrender += 1
                x = k * 60 + 1
                vt = A.value_text(kd, val, None)
                for t, tx in zip(rec, sc.texts):
                    if t["s"] == vt and t["font"] in ("AF_L", "AF_M") and tx[1] > 40 and x - 2 <= (tx[0] + tx[2]) / 2 <= x + 60:
                        fonts.setdefault(t["font"], []).append(val)
                        if tx[0] < x or tx[2] > x + 58: problems.append((name, r + 1, kd["label"], val, "value %r ink %s outside tile %d..%d" % (vt, tx[:4], x, x + 58)))
                for t, tx in zip(rec, sc.texts):
                    if t["s"] == kd["label"] and x - 2 <= (tx[0] + tx[2]) / 2 <= x + 60 and (tx[0] < x or tx[2] > x + 58): problems.append((name, r + 1, kd["label"], val, "label outside tile"))
                ws = [w for w in cv.warnings if not w.startswith("canvas")]
                if ws:
                    problems.append((name, r + 1, kd["label"], val, "warn " + str(ws)))
                if any((tx[5] & 3) for tx in sc.texts): problems.append((name, r + 1, kd["label"], val, "cut/ellipsis flag"))
            fl = {f: len(vs) for f, vs in fonts.items()}
            nz = [f for f, vs in fonts.items()]
            zeroonly = (len(fonts) == 2 and kd["zero"] and fonts.get("AF_M") == [kd["min"]])
            fontsumm.append((name, r + 1, kd["label"], kd["max"] - kd["min"] + 1, fl, "LIST->dots" if A.is_list(kd) else "meter", "STABLE" if len(fonts) <= 1 else ("zero-word AF_M only (by design)" if zeroonly else "UNSTABLE")))
print("renders:", nrender)
print("| algo | row | knob | n values | fonts used (count) | widget | font stable |\n|--|--|--|--|--|--|--|")
for f in fontsumm: print("| %s | %d | %s | %d | %s | %s | %s |" % f)
print("problems:", len(problems))
for p in problems[:60]: print("  ", p)
print("\nA.check():", A.check(text_w, AF_M, AF_L))
print("lists > MAX_DOTS (%d) fall back to meter:" % A.MAX_DOTS, [(n, r + 1, kd["label"], len(kd["names"])) for n in A.ORDER for r in range(2) for kd in A.ALGOS[n]["rows"][r] if kd and kd["names"] and len(kd["names"]) > A.MAX_DOTS])
print("lists drawn as dots:", [(n, r + 1, kd["label"], len(kd["names"]), "dot w=%d" % min(8, (50 - (len(kd["names"]) - 1) * 2) // len(kd["names"]))) for n in A.ORDER for r in range(2) for kd in A.ALGOS[n]["rows"][r] if kd and A.is_list(kd)])
print("MAX_DOTS=10 dot width:", min(8, (50 - 9 * 2) // 10), "px; 11 entries would give", min(8, (50 - 10 * 2) // 11))

print("\n## 5b channel-set sweep (full screen)")
nr = 0; prob2 = []
import io
for used_n in range(1, 5):
    for used in itertools.combinations(range(4), used_n):
        for sel in used:
            for muted_sel in (False, True):
                for off in (False, True):
                    for newc in (False, True):
                        for playing in (True, False):
                            for rec in (False, True):
                                S = v.base(used=list(used), sel=sel, playing=playing)
                                S["chans"][sel]["muted"] = muted_sel
                                if off: S["chans"][sel]["midi"] = None
                                if newc: S["new_chan"] = True
                                if rec:
                                    S["rec"] = True
                                    curs = range(1, 17)
                                else:
                                    curs = [None]
                                for cu in curs:
                                    if cu: S["cursor"] = cu
                                    buf = io.StringIO()
                                    try:
                                        with contextlib.redirect_stdout(buf):
                                            sc, cv = render_state(v, S)
                                        nr += 1
                                        if [w for w in cv.warnings if not w.startswith('canvas')]: prob2.append((used, sel, muted_sel, off, newc, playing, rec, cu, cv.warnings[:2]))
                                    except Exception as e:
                                        prob2.append((used, sel, muted_sel, off, newc, playing, rec, cu, "EXC " + repr(e)))
print("full-screen renders:", nr, "with warnings/exceptions:", len(prob2)); print(prob2[:10])
# non-contiguous
print("non-contiguous sets tested: (0,2) (0,3) (1,3) (0,1,3) (0,2,3) (1,2,3 is contiguous)")
