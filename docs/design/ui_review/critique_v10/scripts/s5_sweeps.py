from common import *
import itertools, copy, traceback, time
from collections import Counter, defaultdict
t0 = time.time()
print("# 5 sweeps (HI-CON)")
ORDER = list(A.ORDER)
print("algos_v10.ORDER =", ORDER)
chk = A.check(text_w, AF_M, AF_L)
print("\n## algos_v10.check(text_w, AF_M, AF_L) ->", chk if chk else "[] (no problems)")
CAPH = {"S": 9, "M": 11, "L": 21}
def rows_of(name):
    return [A.ALGOS[name]["rows"][0], A.ALGOS[name]["rows"][1], mod.RIG_ROW]
issues = []
summary = []
print("\n## 5a every algorithm x row x knob x value (renders: all values of rows 1-2; row 3 = RIG_ROW 0..127 all values)")
nrender = 0
for name in ORDER:
    for r in range(3):
        row = rows_of(name)[r]
        for k in range(4):
            kd = row[k]
            if kd is None:
                issues.append("%s row %d K%d: missing knob" % (name, r + 1, k + 1)); continue
            vals = list(range(kd["min"], kd["max"] + 1))
            if r == 2 and name != ORDER[0]:
                vals = [kd["min"], 64, 100, kd["max"]]            # RIG_ROW is algorithm-independent: full range done under the first algorithm
            fonts = Counter(); worst = (0, None); inkmin = 999; inkmax = -999; warn = set(); texts_w = {}
            for v in vals:
                S = mod.base(sel=0, row=r, vals={k: v}, rig=(r == 2)); S["chans"][0]["algo"] = name
                try:
                    R = rec(S)
                except Exception as e:
                    issues.append("%s r%d K%d v=%s EXC %r" % (name, r + 1, k + 1, v, e)); continue
                nrender += 1
                for w in R["cv"].warnings:
                    if "CV_MAX" not in w: warn.add(w)
                x0 = k * 60 + 1
                vt = A.value_text(kd, v, None)
                for t in R["texts"]:
                    if t["s"] == vt and 26 < t["ycap"] < 60 and x0 <= t["x"] + 1 <= x0 + 58 and t["font"] in ("M", "L") and t["ycap"] >= 47:
                        f = M.font("AF_" + t["font"]) if False else (AF_L if t["font"] == "L" else AF_M)
                        ib = M.text_ink(f, vt)
                        l, rr_ = t["x"] + ib[0], t["x"] + ib[2]
                        inkmin = min(inkmin, l - x0); inkmax = max(inkmax, rr_ - (x0 + 58))
                        fonts[(t["font"], v == 0 and bool(kd["zero"]))] += 1
                        if t["w"] > worst[0]: worst = (t["w"], vt)
                        break
                else:
                    issues.append("%s r%d K%d v=%s: value text %r not found in render" % (name, r + 1, k + 1, v, vt))
                for t in R["texts"]:                         # label ink inside tile
                    if t["s"] == kd["label"] and 25 < t["ycap"] < 30 and x0 - 2 <= t["x"] <= x0 + 58:
                        ib = M.text_ink(AF_M, kd["label"]); l, rr_ = t["x"] + ib[0], t["x"] + ib[2]
                        if l < x0 or rr_ > x0 + 58: issues.append("%s r%d K%d label %r ink x %d..%d outside tile %d..%d" % (name, r + 1, k + 1, kd["label"], l, rr_, x0, x0 + 58))
            fset = {f for (f, z) in fonts if not z}
            zero_fonts = {f for (f, z) in fonts if z}
            stable = len(fset) <= 1
            ok = inkmin >= 0 and inkmax <= 0
            if not stable: issues.append("%s r%d %s: font changes across range %s" % (name, r + 1, kd["label"], dict(fonts)))
            if not ok: issues.append("%s r%d %s: value ink leaves the tile (left margin %d, right overflow %d, widest %r)" % (name, r + 1, kd["label"], inkmin, inkmax, worst))
            for w in warn: issues.append("%s r%d %s warning: %s" % (name, r + 1, kd["label"], w))
            summary.append((name, r + 1, k + 1, kd["label"], len(vals), "/".join(sorted(fset)) or "-", "+zero:" + "/".join(sorted(zero_fonts)) if zero_fonts else "", worst, inkmin, inkmax, stable and ok))
print("renders: %d, %.1fs" % (nrender, time.time() - t0))
print("| algo | row | K | label | #values | font(non-zero) | zero-word font | widest value (advance px) | min left margin to tile | max right overflow | stable & fits |"); print("|---|---|---|---|---|---|---|---|---|---|---|")
for s in summary:
    print("| %s | %d | %d | %s | %d | %s | %s | %s (%d) | %d | %d | %s |" % (s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7][1], s[7][0], s[8], s[9], "yes" if s[10] else "**NO**"))
print("\nissues found by the sweep (%d):" % len(issues))
for i in issues: print("  -", i)
print("knobs failing: %d of %d" % (sum(1 for s in summary if not s[10]), len(summary)))
# fit within AF_L tile 54 criterion of value_font vs the actually available 58
print("\n## value_font decisions (AF_L for whole knob when widest non-zero value <=54 px and only digits/caps/space)")
for name in ORDER:
    for r in range(2):
        for k in range(4):
            kd = A.ALGOS[name]["rows"][r][k]
            f = A.value_font(kd, text_w, AF_M, AF_L)
            wv = A.widest_value(kd, text_w, AF_L, zero=False)
            if f is AF_M and all(c.isdigit() or c.isupper() or c == " " for c in wv):
                pass
print("  knobs drawn in AF_M although all values are digits/caps (width forced): ", [(n, r + 1, A.ALGOS[n]["rows"][r][k]["label"], A.widest_value(A.ALGOS[n]["rows"][r][k], text_w, AF_L, zero=False), text_w(AF_L, A.widest_value(A.ALGOS[n]["rows"][r][k], text_w, AF_L, zero=False))) for n in ORDER for r in range(2) for k in range(4) if A.value_font(A.ALGOS[n]["rows"][r][k], text_w, AF_M, AF_L) is AF_M and all(c.isdigit() or c.isupper() or c == " " for c in A.widest_value(A.ALGOS[n]["rows"][r][k], text_w, AF_L, zero=False))])
print("  knobs drawn in AF_M for other reasons (lowercase/punct in widest value): ", [(n, r + 1, A.ALGOS[n]["rows"][r][k]["label"], A.widest_value(A.ALGOS[n]["rows"][r][k], text_w, AF_L, zero=False)) for n in ORDER for r in range(2) for k in range(4) if A.value_font(A.ALGOS[n]["rows"][r][k], text_w, AF_M, AF_L) is AF_M and not all(c.isdigit() or c.isupper() or c == " " for c in A.widest_value(A.ALGOS[n]["rows"][r][k], text_w, AF_L, zero=False))])
# ---- dot counts
print("\n## 5b value-list knobs (position dots): n dots, dot width dw=min(8,(50-(n-1)*2)//n), total width, tile 58")
print("| algo | row | label | n | dw | total w | dot y 47..49 |"); print("|---|---|---|---|---|---|---|")
dots = Counter()
for name in ORDER:
    for r in range(2):
        for kd in A.ALGOS[name]["rows"][r]:
            if A.is_list(kd):
                n = kd["max"] - kd["min"] + 1; dw = min(8, (50 - (n - 1) * 2) // n); tw = n * dw + (n - 1) * 2
                dots[n] += 1
                print("| %s | %d | %s | %d | %d | %d | %s |" % (name, r + 1, kd["label"], n, dw, tw, "dw<=0 BROKEN" if dw <= 0 else ("dw=1" if dw == 1 else "")))
print("distribution of list lengths:", dict(sorted(dots.items())))
print("lists with n>=2 whose dw<2:", [n for n in range(2, 40) if min(8, (50 - (n - 1) * 2) // n) < 2][:3], "(first n with dw<2; dw<=0 from n=%d)" % next(n for n in range(2, 60) if (50 - (n - 1) * 2) // n <= 0))
# ---- header and toast widths per algo
print("\n## 5c per-algorithm header/toast widths")
print("| algo | short | AF_M w | header 'MIDI n' ends x | MIDI OFF chip ends x | ALGO toast end x (`<NAME> AT NEXT STEP`) | SELECT toast end x (`CH2  <NAME>`) |"); print("|---|---|---|---|---|---|---|")
for name in ORDER:
    sh = A.ALGOS[name]["short"]; xe = 105 + text_w(AF_M, sh)
    xx = xe + 5 + text_w(AF_S, "MIDI") + 2 + text_w(AF_M, "16")
    chip = xe + 4 + text_w(AF_S, "MIDI") + text_w(AF_M, "OFF") + 10
    pill = text_w(AF_S, "ALGO") + 6; xa = pill + 13 + text_w(AF_M, "%s AT NEXT STEP" % name)
    pillS = text_w(AF_S, "SELECT") + 6; xs = pillS + 13 + text_w(AF_M, "CH2  %s" % name)
    print("| %s | %s | %d | %d | %d | %d%s | %d%s |" % (name, sh, text_w(AF_M, sh), xx, chip, xa, " **>240**" if xa > 240 else "", xs, " **>240**" if xs > 240 else ""))
# ---- channel sets
print("\n## 5d channel-set / state sweep")
ALG4 = ["ACID", "EUCLID", "SNAKE", "STAGE"]
cnt = 0; probs = []
def go(tag, S):
    global cnt
    cnt += 1
    try:
        R = rec(S)
        w = [x for x in R["cv"].warnings if "CV_MAX" not in x]
        if w: probs.append((tag, w))
        return R
    except Exception as e:
        probs.append((tag, "EXC " + repr(e))); traceback.print_exc(limit=2)
subsets = [list(c) for n in range(1, 5) for c in itertools.combinations(range(4), n)]
for used in subsets:
    for sel in used:
        for pl in (True, False):
            go("used=%s sel=%d playing=%s" % (used, sel, pl), mod.base(used=used, sel=sel, playing=pl))
        S = mod.base(used=used, sel=sel); S["chans"][sel]["muted"] = True; go("used=%s sel=%d selected muted" % (used, sel), S)
        S = mod.base(used=used, sel=sel); S["chans"][sel]["midi"] = None; go("used=%s sel=%d sel MIDI OFF" % (used, sel), S)
        S = mod.base(used=used, sel=sel); S["chans"][sel]["lane"] = [None] * 16; go("used=%s sel=%d sel all-rest" % (used, sel), S)
        S = mod.base(used=used, sel=sel)
        for c in S["chans"]: c["lane"] = [None] * 16
        go("used=%s sel=%d ALL lanes rest" % (used, sel), S)
        S = mod.base(used=used, sel=sel)
        for i in used:
            if i != sel: S["chans"][i]["muted"] = True
        go("used=%s sel=%d others muted" % (used, sel), S)
        S = mod.base(used=used, sel=sel)
        for i in used:
            if i != sel: S["chans"][i]["midi"] = None
        go("used=%s sel=%d others MIDI OFF" % (used, sel), S)
        for ph in (0, 15):
            go("used=%s sel=%d playhead=%d" % (used, sel, ph), mod.base(used=used, sel=sel, playhead=ph))
        go("used=%s sel=%d REC stopped" % (used, sel), mod.base(used=used, sel=sel, rec=True, playing=False))
        go("used=%s sel=%d REC playing" % (used, sel), mod.base(used=used, sel=sel, rec=True, playing=True))
        S = mod.base(used=used, sel=sel); S["chans"][sel]["midi"] = None; S["chans"][sel]["muted"] = True; go("used=%s sel=%d muted+OFF" % (used, sel), S)
        S = mod.base(used=used, sel=sel, diced={0, 3, 15}); go("used=%s sel=%d diced" % (used, sel), S)
# all-algo x header
for name in ORDER:
    for sel in range(4):
        S = mod.base(sel=sel); S["chans"][sel]["algo"] = name; go("algo %s on ch%d" % (name, sel + 1), S)
        S = mod.base(sel=sel, pend=12, pages=3, vpage=3); S["chans"][sel]["algo"] = name; go("algo %s on ch%d pend12 pages3" % (name, sel + 1), S)
        S = mod.base(sel=sel); S["chans"][sel]["algo"] = name; S["chans"][sel]["midi"] = None; go("algo %s on ch%d MIDI OFF" % (name, sel + 1), S)
# every scene incl. toasts of every algorithm name
for name in ORDER:
    S = mod.base(toast=("enc", "ALGO", "%s AT NEXT STEP" % name)); go("toast ALGO " + name, S)
    S = mod.base(sel=1, toast=("enc", "SELECT", "CH2  %s" % name)); go("toast SELECT " + name, S)
print("subsets of used channels: %d ; renders: %d ; renders with warnings or exceptions: %d ; time %.1fs" % (len(subsets), cnt, len(probs), time.time() - t0))
for p in probs[:40]: print("  -", p)
