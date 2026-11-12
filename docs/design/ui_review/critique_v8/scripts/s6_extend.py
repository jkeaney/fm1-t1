import sys, os, io, contextlib, copy, re, traceback
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from common import *
import el
palette_set("HI-CON")
V8 = open(V8D + "/v8.py").read().split("\n")
AV = open(V8D + "/algos_v8.py").read().split("\n")
print("A. v8.py lines naming an algorithm (ACID EUCLID SNAKE STAGE ARP PATTERNS DEMO):")
for i, l in enumerate(V8, 1):
    if re.search(r"\b(ACID|EUCLID|SNAKE|STAGE|ARP|PATTERNS|DEMO)\b", l): print("   %3d: %s" % (i, l.strip()[:150]))
print("B. v8.py per-algorithm branches (reads of an algo attribute: 'lane' / 'short' / rows):")
for i, l in enumerate(V8, 1):
    if re.search(r'\["lane"\]|\["short"\]|\["rows"\]', l): print("   %3d: %s" % (i, l.strip()[:150]))
print("   -> the only behavioural branch on algorithm data in v8.py is lane == 'single' (lines 135, 149); no branch on a name.")
print("   algos_v8.py per-name entries (UI_EXTRA):", [ (i, l.strip()) for i, l in enumerate(AV, 1) if re.search(r'"(PATTERNS|EUCLID)":', l)])
print("\nC. AlgoDesc fields (sq_types.h) vs what algos_v8 mirrors / the screen uses")
rows = [
 ("name[8]", "yes: ALGOS[n]['name']; header uses 'short' (UI_EXTRA, PATTERNS->PATT) because header slot < 8 chars", "name only"),
 ("id", "no (ORDER list index is used implicitly)", "-"),
 ("n_seed / flags AF_* / cache_bytes", "no (not needed by the screen)", "-"),
 ("knob_morph[8] (slot->morph id)", "implicit: rows[r][k] is indexed by slot, remap pre-applied in knob_map.tsv", "mirrored by TSV rows"),
 ("morph[].label[7]", "yes (label; check() <=6 chars)", "yes"),
 ("morph[].min/max/def", "yes (parsed by regex from the TSV range_default text)", "yes"),
 ("morph[].step / fast", "no: TSV 'fast N' is parsed away (regex strips it); the screen never needs it", "not mirrored"),
 ("value names (enum text, e.g. FWD BWD PEND)", "yes, parsed from TSV; NOT in AlgoDesc/MorphDesc (no names field in sq_types.h)", "TSV only"),
 ("layer[5] (n, seed[2]) / absent layer", "NO: v8 draws all 6 dice icons identically for every algorithm; no dim-if-absent path", "not mirrored"),
 ("sub[5][8] (dice sub-labels, '-' = absent)", "NO: docstring says algos supplies them for the toast; no field, no code reads them", "not mirrored"),
 ("loop_len() / pages", "NO: footer string 'PAGE 1/2' is a literal; the music row always 16 steps", "not mirrored"),
 ("hi() / sub_note() / mod()", "no", "-"),
 ("lane 'pitched'/'single'", "algos_v8 only (UI_EXTRA); AlgoDesc has no such field", "extra in mirror"),
 ("ROW 3 (RIG)", "NO: rows has exactly 2 lists; S['row']=2 -> IndexError (see D); marker row draws 3 markers always", "not mirrored"),
]
print("   %-40s | %-95s | %s" % ("AlgoDesc field", "screen/algos_v8", "status"))
for r in rows: print("   %-40s | %-95s | %s" % r)
print("   algos_v8 'short' vs AlgoDesc.name[8]: header name slot adv limit 60 px in check(); longest actual header name EUCLID 51 px; 'ARP'..; PATTERNS is 8 chars -> UI_EXTRA short 'PATT' (a hand-kept alias).")
print("   sq_types.h: SQ_NALGO =", 6, "; algos_v8.ORDER (after v8.py import) =", A.ORDER, "(DEMO is not in SQ_NALGO)")
print("\nD. row=2 test (ROW 3 / RIG):")
st = mod.base(row=2)
try:
    render(st); print("   row=2 rendered OK")
except Exception as e:
    print("   row=2 ->", repr(e))
print("\nE. register a new algorithm outside algos_v8 (6-char labels, long enum values), render, no edit to v8.py/icons/el")
W6 = "WWWWWW"
rows = [[dict(label="MMMMMM", min=0, max=5, **{"def": 2}, names=["WWWWWW", "MMMMMM", "WIDEST", "SQUARE", "OFFSET", "LEGATO"], fmt=None, zero=None),
         dict(label="WWWWWW", min=0, max=99, **{"def": 50}, names=None, fmt="%d OCTAVE", zero=None),
         dict(label="LEGATO", min=0, max=3, **{"def": 1}, names=["MAJOR", "MINOR", "DORIAN", "PHRYGI"], fmt=None, zero=None),
         dict(label="OFFSET", min=-8, max=8, **{"def": -8}, names=None, fmt=None, zero=None)],
        [dict(label="ABCDEF", min=0, max=127, **{"def": 64}, names=None, fmt="%d / 127", zero=None),
         dict(label="GHIJKL", min=1, max=16, **{"def": 8}, names=None, fmt=None, zero="KEYS"),
         dict(label="MNOPQR", min=0, max=2, **{"def": 0}, names=["INTERLEAVED", "XX", "YY"], fmt=None, zero=None),
         dict(label="STUVWX", min=0, max=1, **{"def": 0}, names=["OFF", "ON"], fmt=None, zero=None)]]
A.register("NEWALG", rows, short="NEWALG")
print("   check():", A.check(text_w, AF_M, AF_L))
for r in range(2):
    st = mod.base(sel=0, row=r, used=[0, 1]); st["chans"][0]["algo"] = "NEWALG"
    buf = io.StringIO()
    try:
        with contextlib.redirect_stdout(buf):
            sc, cv, rec = render(st)
        tiles = [(t["s"], t["font"], t["ink"]) for t in rec if t["ink"] and 24 <= t["ink"][1] <= 70]
        print("   row", r + 1, "rendered; warnings:", [l for l in buf.getvalue().splitlines() if "warning" in l] or "none")
        print("     tile texts:", tiles)
        out = []
        for t in rec:
            if t["ink"] and 24 <= t["ink"][1] <= 70:
                k = t["ink"][0] // 60; x0 = k * 60 + 1
                if t["ink"][0] < x0 or t["ink"][2] > x0 + 58: out.append(("INK OUTSIDE TILE", t["s"], t["ink"]))
        print("     ink outside tile:", out or "none")
    except Exception as e:
        print("   row", r + 1, "EXC", repr(e), traceback.format_exc().splitlines()[-3:])
# every value in the new algo
bad = []
for r in range(2):
    for i, k in enumerate(rows[r]):
        for v in range(k["min"], k["max"] + 1):
            t = A.value_text(k, v)
            if text_w(AF_L, t) > 54 and text_w(AF_M, t) > 54: bad.append((r + 1, i + 1, k["label"], v, t, text_w(AF_M, t)))
print("   values fitting neither AF_L nor AF_M(<=54):", len(bad), bad[:5])
print("   worst: 'INTERLEAVED' AF_M adv", text_w(AF_M, "INTERLEAVED"), "; '99 OCTAVE' AF_M", text_w(AF_M, "99 OCTAVE"), "; '127 / 127' AF_M", text_w(AF_M, "127 / 127"))
# does the tile code clip it? knobs() has no ellipsis path: check ink
print("   knobs() has no text_fit/ellipsis call (value drawn with p.text_c, AF_M fallback unclamped):", not any("text_fit" in l for l in V8))
st = mod.base(sel=0, row=1, used=[0], vals={2: 0}); st["chans"][0]["algo"] = "NEWALG"
sc, cv, rec = render(st); print("   INTERLEAVED drawn:", [(t["s"], t["font"], t["ink"]) for t in rec if t["s"] == "INTERLEAVED"])
# register a long short name
A.register("LONGNAME", rows, short="LONGNAME"); print("   short 'LONGNAME' -> check():", [x for x in A.check(text_w, AF_M, AF_L) if "LONGNAME" in x and "short" in x])
print("   registering needs: A.register(...) (+optional hints short/lane) ; changes outside algos_v8: none for knobs/header/music; dice sub-labels & layer-absent dim: not supported by screen.")
