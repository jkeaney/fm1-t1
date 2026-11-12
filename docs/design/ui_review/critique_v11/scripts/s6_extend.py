from lib import *
import contextlib, io
v = mod
def kd(label, mn, mx, names=None, fmt=None, zero=None, d=0): return dict(label=label, min=mn, max=mx, names=names, fmt=fmt, zero=zero, **{"def": d})
twenty = ["N%02d" % i for i in range(20)]
ten = ["T%d" % i for i in range(10)]
eleven = ["E%d" % i for i in range(11)]
A.register("LONGX", [[kd("LONGLABEL", 0, 5), kd("WIDEWD", 0, 3, names=["ABCDEF", "WWWWWW", "MMMMMM", "ABCDEF"]), kd("LIST20", 0, 19, names=twenty), kd("LIST10", 0, 9, names=ten)],
                     [kd("LIST11", 0, 10, names=eleven), kd("LONGFMT", 0, 99, fmt="%d STEPSXXX"), kd("NEG", -64, 63), kd("ZERO", 0, 16, zero="KEYSONLYLONG")]],
           short="LONGERNAME")
bad = A.check(text_w, AF_M, AF_L)
print("A.check() on LONGX:"); [print("  ", b) for b in bad]
print("is_list: LIST20", A.is_list(A.ALGOS["LONGX"]["rows"][0][2]), "LIST10", A.is_list(A.ALGOS["LONGX"]["rows"][0][3]), "LIST11", A.is_list(A.ALGOS["LONGX"]["rows"][1][0]))
S = v.base(); S["chans"][0]["algo"] = "LONGX"
for row in (0, 1):
    S["row"] = row; S["vals"] = {}
    buf = io.StringIO()
    with contextlib.redirect_stdout(buf):
        sc, cv = render_state(v, S)
    print("row", row + 1, "warnings:", [w for w in cv.warnings if not w.startswith("canvas")][:6])
    sc.to_image().save("/tmp/claude-0/longx_row%d.png" % (row + 1))
# texts that overflow: scan every value of every knob
over = []
for row in (0, 1):
    for k, kdd in enumerate(A.ALGOS["LONGX"]["rows"][row]):
        for val in range(kdd["min"], kdd["max"] + 1):
            S = v.base(); S["chans"][0]["algo"] = "LONGX"; S["row"] = row; S["vals"] = {k: val}
            with contextlib.redirect_stdout(io.StringIO()):
                sc, tx, arts = texts_with_bbox(S)
            x = k * 60 + 1
            for r in tx:
                if 24 <= r["bbox"][1] < 80 and x - 2 <= (r["bbox"][0] + r["bbox"][2]) / 2 <= x + 60 and (r["bbox"][0] < x or r["bbox"][2] > x + 58) :
                    over.append((row + 1, kdd["label"], val, r["s"], r["font"], r["bbox"]))
print("text outside its 58 px tile (any value):", len(over)); [print("  ", o) for o in over[:12]]
hd = [(r["s"], r["bbox"]) for r in texts_with_bbox(dict(v.base(), chans=[dict(c, algo="LONGX") if i == 0 else c for i, c in enumerate(v.CHANS)]))[1] if r["bbox"][1] < 22][:9]
print("header with short 'LONGERNAME':", hd)
# dots for 10
n = 10; print("10-entry dots: dw", min(8, (50 - (n - 1) * 2) // n), "px, row span", n * 3 + (n - 1) * 2)
# ROW 3 table
print("DEVICES keys:", list(v.DEVICES), "; rig_row labels:", [d['label'] for d in v.rig_row({})]);
v.DEVICES["BIG"] = [("CUTOFFLONG", 64), ("R", 1), ("A", 2), ("B", 3)]
S = v.base(row=2, rig="BIG")
with contextlib.redirect_stdout(io.StringIO()):
    sc, tx, arts = texts_with_bbox(S)
print("ROW 3 with a 10-char label from a device entry: ink", [(r["s"], r["bbox"]) for r in tx if r["s"] == "CUTOFFLONG"], "tile x1..58; A.check() does not look at DEVICES:", A.check(text_w, AF_M, AF_L) == bad)
