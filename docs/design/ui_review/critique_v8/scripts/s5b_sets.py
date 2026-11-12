import sys, os, itertools, copy, io, contextlib, traceback
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from common import *
import el
palette_set("HI-CON")
base = mod.base
res = {"ok": 0, "warn": [], "exc": []}
def run(label, st):
    buf = io.StringIO()
    try:
        with contextlib.redirect_stdout(buf):
            sc, cv, rec = render(st)
        w = [l for l in buf.getvalue().splitlines() if "warning" in l]
        if w: res["warn"].append((label, w))
        else: res["ok"] += 1
        return sc, rec
    except Exception as e:
        res["exc"].append((label, repr(e), traceback.format_exc().splitlines()[-3:]))
        return None, None
n = 0
for k in (1, 2, 3, 4):
    for used in itertools.combinations(range(4), k):
        for sel in range(4):
            for muted_sel in (False, True):
                st = base(used=list(used), sel=sel)
                if muted_sel: st["chans"][sel]["muted"] = True
                run("used=%s sel=%d muted_sel=%s" % (list(used), sel, muted_sel), st); n += 1
print("A. channel-set sweep: all 15 non-empty subsets x sel 0..3 x selected-muted off/on = %d renders" % n)
print("   ok:", res["ok"], "| warnings:", len(res["warn"]), "| exceptions:", len(res["exc"]))
nonsel = [r for r in res["warn"]][:3]; print("   first warnings:", nonsel)
print("   exceptions (first 4):", res["exc"][:4])
# which combos have sel not in used
bad_sel = [(u, s) for k in (1,2,3,4) for u in itertools.combinations(range(4), k) for s in range(4) if s not in u]
print("   combos with sel not in used (renders without exception? see above):", len(bad_sel))
# what the header shows when sel not in used
st = base(used=[0, 1], sel=3); sc, rec = run("sel3 unused", st)
print("   sel=3 not in used=[0,1]: header texts:", [(r["s"], name_of(r["fg"])) for r in rec if r["y"] < 21])
st = base(used=[1, 3], sel=1); sc, rec = run("noncontig", st)
print("   used=[1,3] sel=1 header chip texts:", [(r["s"], r["ink"][0]) for r in rec if r["y"] < 21 and 46 <= r["ink"][0] < 104])
# slot assignment is by order in used, not channel index
print("   music-row slot for channel = index within `used` (not channel number): used=[1,3] -> CH2 slot 0, CH4 slot 1 (2-slot layout, widths 6/6)")
# special cases
print("\nB. special cases")
# all-rest channel
st = base(used=[0, 1, 3]); st["chans"][1]["lane"] = [None]*16
sc, rec = run("allrest", st); print("   all-rest channel (CH2 empty lane) in 3-ch: render ok" if sc else "   all-rest: FAILED")
st = base(used=[1], sel=1); st["chans"][1]["lane"] = [None]*16
sc, rec = run("allrest_only", st); print("   only channel all-rest (EUCLID, empty): render", "ok" if sc else "FAILED")
# every channel all-rest
st = base(used=[0,1,2,3]);
for c in st["chans"]: c["lane"] = [None]*16
sc, rec = run("allrest4", st); print("   4 channels all-rest: render", "ok" if sc else "FAILED")
# 4 channels on one step: every step has all 4
st = base(used=[0,1,2,3])
for ci, c in enumerate(st["chans"]): c["lane"] = [{"p": (i*3+ci*7) % 36, "acc": ci == 0, "sl": False} for i in range(16)]; c["muted"] = False
sc, rec = run("all4steps", st)
px = sc.px
M = {"INK":".","SURF":"s","CH1":"1","CH2":"2","CH3":"3","CH4":"4","PHC":"p","DIM":"d","WHITE":"W"}
print("   4 channels on every step: bottom row y=123 steps 1-3:", "".join(M.get(name_of(px[123*240+x]), "?") for x in range(0, 46)))
print("   4 channels, single-lane CH2 EUCLID present: EUCLID bar 6 px tall regardless of pitch")
# single lane only (EUCLID)
st = base(used=[1], sel=1); sc, rec = run("euclid_only", st)
px = sc.px; hs = []
for x in range(1, 14):
    col = [y for y in range(91, 124) if name_of(px[y*240+x]) == "CH2"]
    if col: hs.append((x, col[0], col[-1], len(col)))
print("   EUCLID only (single lane, used=[1]) step 1 bar:", hs[:1], "(height = 6)")
# pitched lane with every note same pitch
st = base(used=[0]); st["chans"][0]["lane"] = [{"p": 10, "acc": False, "sl": False} for _ in range(16)]
sc, rec = run("flatpitch", st); px = sc.px
col = [y for y in range(91, 124) if name_of(px[y*240+2]) == "CH1"]; print("   all notes same pitch (hi==lo): bar height", len(col), "px (floor 4)")
# lane list shorter than 16 or algo single with sel muted
st = base(used=[1], sel=1); st["chans"][1]["muted"] = True; run("euclid_muted", st)
# pending preset P16? and rec-armed + pending, toast over header
for lab, st in (("pend16", base(pend=16)), ("rec+pend", base(rec=True, pend=3, used=[0, 1])), ("toast+pend", base(toast=("dice_notes","NOTES DICED","EDIT=UNDO"), pend=16)), ("rec+toast", base(rec=True, toast=("dice_notes","X","Y")))):
    sc, rec = run(lab, st)
print("   pending/rec/toast combos rendered without exception/warning:", len(res["exc"]) == 0 and not res["warn"])
# algorithms x sheet scenes
for sn in ("sheet_row1_a","sheet_row1_b","sheet_row2_a","sheet_row2_b","arp","demo_algo","euclid_midi16"):
    run(sn, scenes()[sn])
print("   total renders:", res["ok"] + len(res["warn"]) + len(res["exc"]), "| warnings:", len(res["warn"]), "| exceptions:", len(res["exc"]))
print("   warnings:", res["warn"][:5]); print("   exceptions:", res["exc"][:5])
