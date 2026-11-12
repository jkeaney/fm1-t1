from common import *
import copy
print("# 6 extensibility")
print("""
## 6a where v10.py / algos_v10.py name individual algorithms (grep, line numbers)
v10.py:29-38   A.register("DEMO", ...) demo algorithm with its own rows; absent=("ACCNT",) at line 38
v10.py:46-49   CHANS fixture: algo names ACID / EUCLID / SNAKE / STAGE (data, not drawing code)
v10.py:391-405 scene table: names SNAKE (392), EUCLID (391), ARP (399), DEMO (400), STAGE (401), sheet lists (402-405)
v10.py drawing functions (header 93, knob_row 113, music 191/205, buttons 232) read A.ALGOS[...] fields only: short, rows, lane=="single", absent
algos_v10.py:28-34  patt_pick (PATTERNS) and stage_total (STAGE) value callbacks; :36-38 UI_EXTRA keys PATTERNS (short PATT, PICK fn), EUCLID (lane single), STAGE (TOTAL fn); :142 ORDER list of the six + DEMO appended by register()
algos_v10.py:93  load() sets absent=() for every TSV-parsed algorithm (the dimmed-die hint is never read from a TSV; only DEMO sets it, v10.py:38)
firmware sq_types.h: AlgoDesc.layer[SQ_NLAYER=5] n=0 -> absent, sub[5][8] '-' ; SQ_NALGO 6; AlgoDesc.name[8] (comment '<= 6 chars')
""")
M.palette_set("HI-CON")
def kd(label, lo=0, hi=9, names=None, fmt=None, zero=None, d=0, fn=None):
    k = dict(label=label, min=lo, max=hi, names=names, fmt=fmt, zero=zero, **{"def": d})
    if fn: k["fn"] = fn
    return k
good = lambda i: kd("OK%d" % i)
cases = {}
def case(name, k, **hints):
    rows = [[k, good(2), good(3), good(4)], [good(5), good(6), good(7), good(8)]]
    hints.setdefault("short", "XT")
    A.register(name, rows, **hints); cases[name] = k
case("X_LONGLBL", kd("LONGLABEL"))                                   # 9 chars label
case("X_LBL7", kd("ABCDEFG"))                                        # 7 chars label (> 6)
case("X_WIDELBL", kd("MMMMMM"))                                      # 6 wide chars
case("X_LONGVAL", kd("VAL", 0, 3, names=["ONE", "TWO", "THREEE", "SEVENTEEN"]))
case("X_FMT", kd("FMT", 0, 99, fmt="%d SEMIS"))
case("X_LIST20", kd("LST", 0, 19, names=["N%d" % i for i in range(20)], d=3))
case("X_LIST8", kd("L8", 0, 7, names=["A%d" % i for i in range(8)], d=3))
case("X_LIST16", kd("L16", 0, 15, names=["A%d" % i for i in range(16)], d=3))
case("X_LIST17", kd("L17", 0, 16, names=["A%d" % i for i in range(17)], d=3))
case("X_LIST25", kd("L25", 0, 24, names=["A%d" % i for i in range(25)], d=3))
case("X_FN", kd("FNV", 0, 9, fn=lambda v, st: "VERY LONG VALUE %d" % v))
case("X_SHORT", kd("S"), short="LONGNAME")
case("X_NEG", kd("NEG", -20, 20, d=0))
case("X_BIG", kd("BIG", 0, 9999, d=8888))
case("X_LOWER", kd("LOW", 0, 2, names=["a", "bb", "ccc"]))
case("X_ONEVAL", kd("ONE", 5, 5, d=5))
case("X_BADLANE", kd("LN"), lane="chord")
case("X_ABSENT", kd("AB"), absent=("ZZZ",))
case("X_ZERO", kd("ZRO", 0, 16, zero="SILENTLY", d=3))
print("## 6b registered test algorithms: one knob (K1 row 1) stressed each; check() output and render sweep")
chk = A.check(text_w, AF_M, AF_L)
print("A.check() returned %d problem(s):" % len(chk))
for c in chk: print("   check>", c)
caught = {}
for c in chk:
    for n in cases: 
        if c.startswith(n): caught.setdefault(n, []).append(c)
print()
import io, contextlib
print("| case | stressed property | check() flags it | render: warnings / exception | value ink vs tile (K1) | verdict |"); print("|---|---|---|---|---|---|")
for name, k in cases.items():
    res = []; ex = None; warn = set(); inkL = 999; inkR = -999; fonts = set()
    vals = list(range(k["min"], k["max"] + 1))
    if len(vals) > 40: vals = vals[:6] + vals[-6:] + [k["def"]]
    for v in vals:
        S = mod.base(sel=0, vals={0: v}); S["chans"][0]["algo"] = name
        try:
            R = rec(S)
        except Exception as e:
            ex = repr(e); break
        for w in R["cv"].warnings:
            if "CV_MAX" not in w: warn.add(w)
        vt = A.value_text(k, v)
        for t in R["texts"]:
            if t["s"] == vt and t["ycap"] >= 47 and t["x"] < 60:
                f = AF_L if t["font"] == "L" else AF_M; ib = M.text_ink(f, vt)
                inkL = min(inkL, t["x"] + ib[0] - 1); inkR = max(inkR, t["x"] + ib[2] - 59); fonts.add(t["font"])
                break
    # header check for short
    S = mod.base(sel=0); S["chans"][0]["algo"] = name
    try:
        R = rec(S); hdr = [t for t in R["texts"] if t["ycap"] == 5 and t["x"] == 105]
        hw = 105 + hdr[0]["w"] if hdr else None
        for w in R["cv"].warnings:
            if "CV_MAX" not in w: warn.add(w)
    except Exception as e:
        ex = ex or repr(e); hw = None
    # label ink
    lab = k["label"]; lb = M.text_ink(AF_M, lab); lw = lb[2] - lb[0]
    n = k["max"] - k["min"] + 1
    prop = {"X_LONGLBL": "label 9 chars (%d px ink, tile 58)" % lw, "X_LBL7": "label 7 chars (%d px)" % lw, "X_WIDELBL": "label 6 wide chars (%d px)" % lw,
            "X_LONGVAL": "list value 'SEVENTEEN' (9 chars)", "X_FMT": "fmt '%d SEMIS'", "X_LIST20": "20-entry list",
            "X_LIST8": "8-entry list", "X_LIST16": "16-entry list", "X_LIST17": "17-entry list", "X_LIST25": "25-entry list", "X_FN": "callback returns 17-char string",
            "X_SHORT": "short name 'LONGNAME' (8 chars; header ends x=%s)" % hw, "X_NEG": "range -20..20", "X_BIG": "range 0..9999 (4 digits)",
            "X_LOWER": "lowercase value names", "X_ONEVAL": "min == max", "X_BADLANE": "lane='chord' (unknown)", "X_ABSENT": "absent=('ZZZ',) (unknown layer)", "X_ZERO": "zero word 'SILENTLY' (8 chars)"}[name]
    dots = ""
    if A.is_list(k):
        dw = min(8, (50 - (n - 1) * 2) // n)
        S = mod.base(sel=0, vals={0: k["min"] + 1}); S["chans"][0]["algo"] = name
        img = rec(S)["sc"].to_image(); pxs = img.load(); surf = rgb(M.ux.surf)
        lit = sum(1 for x in range(1, 59) if pxs[x, 72] != surf); cur = rgb(mod.CH[0]); ncur = sum(1 for x in range(1, 59) if pxs[x, 72] == cur)
        dots = "dot width dw=%d; pixels on dot row y=72: %d non-SURF, %d in channel colour (current dot)" % (dw, lit, ncur)
    ink = ("L %d / R %+d" % (inkL, inkR)) if inkL != 999 else "-"
    fl = "/".join(sorted(fonts)) or "-"
    out_tile = (inkL != 999 and (inkL < 0 or inkR > 0))
    bad = out_tile or ex or warn or (A.is_list(k) and (50 - (n - 1) * 2) // n < 3) or (lw > 58)
    verdict = ("BREAKS" if bad else "ok")
    flagged = name in caught
    print("| %s | %s%s | %s | %s | %s font %s | %s |" % (name, prop, ("; " + dots) if dots else "", "yes: " + caught[name][0] if flagged else "no", ex or ("; ".join(sorted(warn))[:90] or "none"), ink, fl, verdict + (" and check() silent" if bad and not flagged else "")))
# which broken cases check() misses
print()
print("## 6c summary of what check() does NOT test (derived above)")
print("- number of list entries / dot width (position dots): X_LIST20 gives dw=%d; check() silent: %s" % ((50 - 19 * 2) // 20, "X_LIST20" not in caught))
print("- header room for the MIDI OFF chip and the ALGO toast with the FULL name; check() tests only AF_M width of 'short' <= 60 and len(short) <= 6")
print("- value font consistency / the rendered position of the value over the knob meter")
print("- 'lane' value, 'absent' layer names, rows with fewer than 4 knobs are only partly tested (missing knob None is tested)")
