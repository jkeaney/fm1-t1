import sys, traceback, copy, itertools, io, contextlib; sys.path.insert(0, "/home/claude/fm1/docs/design/ui_review/critique_v7/scripts")
from lib import *
import lib
import csv, collections
variant("mock")
out = []; P = out.append
TSV = "/home/claude/fm1/docs/design/knob_map.tsv"
rows = [r for r in csv.reader(open(TSV), delimiter="\t")][1:]
perf = [r for r in rows if r[0] == "PERF" and r[1] in ("ACID", "EUCLID", "SNAKE", "STAGE", "ARP", "PATTERNS")]
ALGOS = ["ACID", "EUCLID", "SNAKE", "STAGE", "ARP", "PATTERNS"]
ENUM = {("ACID", "S4"): ["FWD", "BWD", "PEND"], ("EUCLID", "S4"): ["AUTO", "SINE", "TRI", "SAW+", "SAW-", "ARCH", "INV"], ("SNAKE", "S4"): ["FWD", "BWD", "PEND"],
        ("STAGE", "K3"): ["FWD", "BWD", "PEND"], ("ARP", "K2"): ["LINE", "SKIP", "THUMB", "OCTV", "REPT", "WAVE"], ("ARP", "K3"): ["1 OCT", "2 OCT", "3 OCT", "4 OCT"],
        ("ARP", "K4"): ["1/4", "1/2", "1X", "2X", "3T", "4X"], ("ARP", "S4"): ["UP", "DOWN", "UPDN", "RAND", "ASPL", "CHORD"],
        ("STAGE", "K2"): None, ("PATTERNS", "S1"): ["HOUSE 3", "HOUSE 120", "HOUSE 99"]}
import re
def values(algo, knob, rng):
    if (algo, knob) in ENUM and ENUM[(algo, knob)]:
        v = list(ENUM[(algo, knob)])
    else:
        m = re.search(r"\b\w+ (\d+)\.\.(\d+)", rng)
        v = [str(n) for n in range(int(m.group(1)), int(m.group(2)) + 1)] if m else []
    if "0 = KEYS ONLY" in rng: v.append("KEYS")
    if algo == "STAGE" and knob == "K2": v += ["48 (32)", "64 (32)"]
    return v
P("# 5. Edge-case sweep")
P("## 5a. knob tile fit (tile 58 px wide; label AF_M, value AF_L as knobs_fill draws them; AF_L has glyphs for space # + - . / 0-9 A-Y only)")
P("| algo | knob | label | label px (AF_M) | widest value | value px (AF_L) | fits 58? | glyphs missing in AF_L (drawn as glyph 0) | note |")
P("|---|---|---|---|---|---|---|---|---|")
nfit = nover = nmiss = 0; tot = 0
detail = []
for r in perf:
    algo, knob, label, rng = r[1], r[2], r[3], r[5]
    vals = values(algo, knob, rng)
    lw = text_w(AF_M, label)
    wv = max(vals, key=lambda s: text_w(AF_L, s)) if vals else ""
    vw = text_w(AF_L, wv) if wv else 0
    miss = sorted(set(ch for v in vals for ch in v if AF_L.glyph_at(AF_L.fold(ord(ch))) < 0))
    ink = text_ink(AF_L, wv) if False else None
    ok = lw <= 58 and vw <= 58
    tot += 1; nfit += ok; nover += not ok; nmiss += bool(miss)
    P("| %s | %s | %s | %d | %s | %d | %s | %s | %s |" % (algo, knob, label, lw, wv, vw, "yes" if ok else "NO", "".join(miss) or "-", "range from TSV: " + rng[:60]))
# per-value overflow list
P("")
P("tiles checked=%d; label or widest value > 58 px = %d; knobs whose value set uses an AF_L-missing glyph = %d" % (tot, nover, nmiss))
P("values (any, not only widest) whose AF_L advance exceeds 58 px or that need a glyph AF_L lacks:")
for r in perf:
    algo, knob, label, rng = r[1], r[2], r[3], r[5]
    for v in values(algo, knob, rng):
        w = text_w(AF_L, v); miss = [ch for ch in v if AF_L.glyph_at(AF_L.fold(ord(ch))) < 0]
        if w > 58 or miss:
            P("- %s %s %s value %r: %d px AF_L; missing glyphs %r" % (algo, knob, label, v, w, miss))
P("")
P("labels (AF_M) wider than 58: " + (", ".join("%s %s %s=%d" % (r[1], r[2], r[3], text_w(AF_M, r[3])) for r in perf if text_w(AF_M, r[3]) > 58) or "none") + "; widest label: " + max(((text_w(AF_M, r[3]), r[3]) for r in perf)).__repr__())
P("PERF * rows: R1-R4 'WIRE1'..'WIRE4' (ROW 3 labels): AF_M width WIRE1=%d; the knob_map R rows say the cell shows the device's name for the parameter (not enumerated, not testable)" % text_w(AF_M, "WIRE1"))

# draw through the actual code path: inject TSV-derived rows into el.KN
P("")
P("## 5b. v7.screen with TSV-derived knob rows injected into el.KN (algo names, labels, widest values), and as-shipped KN")
P("el.KN as shipped has keys: %s ; ACID rows: %d ; EUCLID rows: %d" % (sorted(el.KN), len(el.KN["ACID"]), len(el.KN["EUCLID"])))
def tsv_rows(algo):
    d = collections.OrderedDict()
    for r in perf:
        if r[1] == algo: d[r[2]] = r
    mk = lambda k: (d[k][3], max(values(algo, k, d[k][5]), key=lambda s: text_w(AF_L, s)), 50)
    return [[mk("K%d" % i) for i in (1, 2, 3, 4)], [mk("S%d" % i) for i in (1, 2, 3, 4)]]
def base_state(**kw):
    s = mod.v6.S0(foot="keys", **kw); return s
shipped = copy.deepcopy(el.KN)
res = []
def run(state, tag):
    buf = io.StringIO()
    lib.LOG = dict(ops=[], texts=[])
    try:
        with contextlib.redirect_stdout(buf):
            sc, cv = mock.render_state(mod, state, "MONO")
        w = [l.strip() for l in buf.getvalue().splitlines() if l.strip()]
        return sc, cv, w, None
    except Exception as e:
        return None, None, [], "%s: %s" % (type(e).__name__, e)
for algo in ALGOS:
    for row in (0, 1, 2):
        # as shipped
        st = base_state(); st["chans"][0]["algo"] = algo; st["row"] = row
        el.KN.clear(); el.KN.update(shipped)
        sc, cv, w, err = run(st, "shipped")
        labs = None
        if sc is not None:
            kn = el.KN.get(algo, el.KN["ACID"])
            labs = [k[0] for k in kn[row]] if row < len(kn) else None
        P("- as shipped: algo %s row %d: %s; warnings=%s; knob labels drawn=%s (algo in KN: %s)" % (algo, row, ("EXCEPTION " + err) if err else "rendered", w or "none", labs, algo in shipped))
for algo in ALGOS:
    for row in (0, 1, 2):
        st = base_state(); st["chans"][0]["algo"] = algo; st["row"] = row
        el.KN.clear(); el.KN.update(shipped); el.KN[algo] = tsv_rows(algo) + [[("WIRE%d" % i, "127", 50) for i in (1, 2, 3, 4)]]
        sc, cv, w, err = run(st, "tsv")
        P("- TSV rows injected: algo %s row %d: %s; warnings=%s" % (algo, row, ("EXCEPTION " + err) if err else "rendered", w or "none"))
        if sc is not None:
            r = capture(st)
            bad = [(t["s"], t["f"], t["ink"]) for t in r["texts"] if t["cut"]]
            wide = [(t["s"], t["f"], t["adv"]) for t in r["texts"] if t["fn"] == "knobs_fill" and t["adv"] > 58]
            P("    cut texts=%s ; knob texts wider than 58=%s" % (bad or "none", wide or "none"))
el.KN.clear(); el.KN.update(shipped)

P("")
P("## 5c. header: algorithm name where it is drawn (enc_row: icon x=114..125, text x=127 with algo[:6]; preset text right-aligned to x=236 ('P01' starts x=209); pending frame x=196..238)")
P("| algo | name | drawn [:6] | full width AF_M | drawn width | x end drawn | full x end | room to P01 text (x=209) | room to pending frame (x=196) | ALGO_IC size |")
P("|---|---|---|---|---|---|---|---|---|---|")
for a in ALGOS:
    d = a[:6]; fw = text_w(AF_M, a); dw = text_w(AF_M, d)
    ic = el.ALGO_IC[a].strip("\n").split("\n")
    P("| %s | %s | %s | %d | %d | %d | %d | %d | %d | %dx%d |" % (a, a, d, fw, dw, 127 + dw, 127 + fw, 209 - (127 + fw), 196 - (127 + fw), max(len(x) for x in ic), len(ic)))
P("preset text 'P16' width AF_M=%d (x start %d); 'P16?' with frame: width %d (x start %d, frame 196..238)" % (text_w(AF_M, "P16"), 236 - text_w(AF_M, "P16"), text_w(AF_M, "P16?"), 236 - text_w(AF_M, "P16?")))
P("BPM text x=16: '120' width %d (ends x=%d), '300' width %d, '40' width %d; first channel chip starts x=50 (room for '300' = %d px)" % (text_w(AF_M, "120"), 16 + text_w(AF_M, "120"), text_w(AF_M, "300"), text_w(AF_M, "40"), 50 - 16 - text_w(AF_M, "300")))
P("ALGO_PICK cue strip form per page_states.tsv uses 4-letter short names ACID EUCL SNAK STAG ARP PATT; the header here draws the first 6 letters of the long name: " + ", ".join("%s->%s" % (a, a[:6]) for a in ALGOS))
# render each algo header for real
for a in ALGOS:
    st = base_state(); st["chans"][0]["algo"] = a
    for pend in (None, 16):
        st["pend"] = pend
        r = capture(st)
        t = [x for x in r["texts"] if x["fn"] == "enc_row" and x["x"] == 127]
        pt = [x for x in r["texts"] if x["fn"] == "enc_row" and re.match(r"P\d\d", x["s"])]
        P("- header render algo %s pend=%s: algo text drawn %r ink %s; preset text %r ink %s; algo ink right edge %s vs preset ink left %s; overlap=%s; warnings=%s" % (a, pend, t[0]["s"] if t else None, t[0]["ink"] if t else None, pt[0]["s"] if pt else None, pt[0]["ink"] if pt else None, t[0]["ink"][2] if t else None, pt[0]["ink"][0] if pt else None, bool(t and pt and t[0]["ink"][2] > pt[0]["ink"][0]), r["warnings"] or "none"))

P("")
P("## 5d. channel combinations (render_state through v7.draw, palette MONO as the mock parses it)")
def mkstate(used, sel=None, **kw):
    s = mod.v6.S0(foot="keys", used=list(used), sel=(used[0] if sel is None else sel), **kw)
    return s
def analyse(tag, st):
    sc, cv, w, err = run(st, tag)
    if err:
        P("- %s: EXCEPTION %s" % (tag, err)); return None
    r = capture(st)
    tx = [t for t in r["texts"] if t["ink"] and not t["occluded"]]
    ov = []
    for i in range(len(tx)):
        for j in range(i + 1, len(tx)):
            a, b = tx[i]["ink"], tx[j]["ink"]
            if a[0] < b[2] and b[0] < a[2] and a[1] < b[3] and b[1] < a[3]: ov.append((tx[i]["s"], tx[j]["s"]))
    plus = [t for t in r["texts"] if t["s"] == "+" and not t["occluded"]]
    chips = [t["s"] for t in r["texts"] if t["fn"] == "enc_row" and t["s"] in "1234" and len(t["s"]) == 1]
    P("- %s: rendered; mock warnings=%s; text overlaps=%s; cut texts=%s; header chip digits=%s; '+' drawn at x=%s" % (tag, w or "none", ov or "none", [t["s"] for t in r["texts"] if t["cut"]] or "none", chips, [t["x"] for t in plus] or "none"))
    return r
import itertools
for n in (1, 2, 3, 4):
    for used in itertools.combinations(range(4), n):
        for sel in sorted(set([used[0], used[-1]])):
            st = mkstate(used, sel)
            for ci in used:
                if st["chans"][ci]["lane"] is None: pass
            analyse("used=%s sel=%d" % ([u + 1 for u in used], sel + 1), st)
P("")
P("### special cases")
# selected channel muted
st = mkstate((0, 1, 3), 0); st["chans"][0]["muted"] = True
a0 = capture(mkstate((0, 1, 3), 0))["screen"]; r = analyse("selected channel (CH1) muted, used 1,2,4", st)
a1 = capture(st)["screen"]
diff = [(x, y) for y in range(240) for x in range(240) if a0.px[y * 240 + x] != a1.px[y * 240 + x]]
ys = [d[1] for d in diff]
P("  pixels changed vs unmuted: %d; y-range of changes %s..%s; changed outside music row (y 90..127) and key glyph rows (y 188..222): %d" % (len(diff), min(ys), max(ys), sum(1 for x, y in diff if not (90 <= y < 128 or 188 <= y < 222))))
bands = collections.Counter((("header" if y < 21 else "knobs" if y < 78 else "marker" if y < 88 else "music" if y < 128 else "buttons" if y < 184 else "keys" if y < 223 else "caption")) for x, y in diff)
P("  changed pixels by band: %s" % dict(bands))
# all rests
st = mkstate((0, 1), 0); st["chans"][1]["lane"] = [None] * 16
analyse("CH2 lane all rests (used 1,2)", st)
st = mkstate((0, 1), 1); st["chans"][1]["lane"] = [None] * 16
analyse("selected CH2 lane all rests (used 1,2, sel 2)", st)
st = mkstate((0,), 0); st["chans"][0]["lane"] = [None] * 16
analyse("only channel, all rests", st)
# all four play at step 0
st = mkstate((0, 1, 2, 3), 0)
for ci in range(4): st["chans"][ci]["muted"] = False
for ci in range(4):
    st["chans"][ci]["lane"][0] = {"p": 3 + ci, "acc": ci == 0, "sl": False, "dice": ci in (0, 2)}
    st["chans"][ci]["lane"][1] = {"p": 5, "acc": False, "sl": False, "dice": False}
r = analyse("all 4 channels play steps 0 and 1 (steps 0: accent on CH1, dice on CH1+CH3)", st)
mus = [o for o in r["ops"] if o["fn"] == "music_slim" and o["k"] in ("rect", "frame") and 90 <= o["y"] < 124 and o["h"] >= 4 and o["w"] < 15 and 1 <= o["x"] <= 15]
P("  step 0 bars: %s ; hatch ops on step 0: %s" % ([(o["x"], o["w"], o["h"], tokname(o["c"])) for o in mus], [(o["x"], o["w"], o["h"]) for o in r["ops"] if o["k"] == "hatch" and o["x"] < 16]))
# all muted
st = mkstate((0, 1, 2, 3), 0)
for ci in range(4): st["chans"][ci]["muted"] = True
analyse("all four muted", st)
# MIDI 16
sa = mkstate((0, 1, 3), 0); sb = mkstate((0, 1, 3), 0); sb["chans"][0]["midi"] = 16
da = capture(sa)["screen"]; db = capture(sb)["screen"]
nd = sum(1 for i in range(240 * 240) if da.px[i] != db.px[i])
mid_texts = [t["s"] for t in capture(sb)["texts"] if "16" in t["s"] or "MIDI" in t["s"] or "CH" in t["s"]]
P("- MIDI channel 16 on CH1 vs 2: pixels differing = %d; texts containing '16'/'MIDI'/'CH' = %s (the chans[].midi field is %s by v5/v6/v7 drawing code)" % (nd, mid_texts or "none", "not read" if nd == 0 else "read"))
# sel not in used
st = mkstate((0, 1), 0); st["sel"] = 3
analyse("sel=CH4 not in used {1,2}", st)
# row 1 for ACID: marker squares
for row in (0, 1, 2):
    st = mkstate((0, 1, 3), 0); st["row"] = row
    el.KN.clear(); el.KN.update(shipped)
    sc, cv, w, err = run(st, "row")
    P("- ACID row=%d (row_marker squares drawn: 3, filled index %d): %s" % (row, row, ("EXCEPTION " + err) if err else "rendered"))
P("")
P("## 5e. turning-knob box (v5.knob_turning as v7 calls it: box 120x56 at x=60*min(k,2); title 'K%d LABEL' AF_M at x+6; old AF_M at x+6; arrow 5x7 at x+20; new AF_L at x+32; scale track starts x+66, 48 px; reads kn[0][k] = row 1 labels always)")
class _M:
    PALETTE = "MONO"
    @staticmethod
    def draw(cv, S):
        def fn(p):
            mod.v5.knob_turning(p, 24, 0, S["kn"], S["k"], S["old"], S["new"], 50)
        el.paint(cv, [(0, 120), (120, 240)], fn)
for (old, new) in [("4", "6"), ("11", "12"), ("46", "47"), ("KEYS", "1"), ("FWD", "BWD"), ("PEND", "FWD"), ("1 OCT", "2 OCT"), ("48 (32)", "50 (32)"), ("HOUSE 3", "HOUSE 4")]:
    for k in (0, 1, 2, 3):
        kn = [[("DENS", "0", 50), ("SLIDE", "0", 50), ("JUMP", "0", 50), ("ACCNT", "0", 50)]]
        lib.LOG = dict(ops=[], texts=[])
        buf = io.StringIO()
        with contextlib.redirect_stdout(buf):
            sc, cv = mock.render_state(_M, dict(kn=kn, k=k, old=old, new=new), "MONO")
        x0 = 60 * min(k, 2)
        t = {tx["f"] + ":" + tx["s"]: tx for tx in lib.LOG["texts"] if tx["ink"]}
        oldt = [tx for tx in lib.LOG["texts"] if tx["f"] == "AF_M" and tx["s"] == old and tx["ink"]]
        newt = [tx for tx in lib.LOG["texts"] if tx["f"] == "AF_L" and tx["s"] == new and tx["ink"]]
        o_r = oldt[0]["ink"][2] if oldt else None; n_r = newt[0]["ink"][2] if newt else None
        flag = []
        if o_r is not None and o_r > x0 + 20: flag.append("old text right edge %d > arrow x %d" % (o_r, x0 + 20))
        if n_r is not None and n_r > x0 + 66: flag.append("new text right edge %d > scale x %d" % (n_r, x0 + 66))
        if n_r is not None and n_r > x0 + 119: flag.append("new text right edge %d > box right %d" % (n_r, x0 + 119))
        w = [l.strip() for l in buf.getvalue().splitlines() if l.strip()]
        P("- K%d old=%r new=%r: box x=%d..%d; old ink right=%s, new ink right=%s; flags=%s; warnings=%s" % (k + 1, old, new, x0, x0 + 119, o_r, n_r, flag or "none", w or "none"))
lens = []
for r in perf:
    if r[2][0] == "K":
        t = "K%s %s" % (r[2][1], r[3]); lens.append((text_w(AF_M, t), t, r[1]))
P("title widths 'Kn LABEL' for K1-K4 labels: max %s (box inner width 108)" % (max(lens),))
open("/home/claude/fm1/docs/design/ui_review/critique_v7/scripts/s5_out.md", "w").write("\n".join(out))
print("\n".join(out))
