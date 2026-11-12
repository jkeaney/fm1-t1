from common import *
import itertools
print("# 5e extra: header chip layout for every channel set; STAGE TOTAL with 'heard'; PATTERNS PICK")
print("| used | '+' drawn at slot (x) | chip slots drawn | note |"); print("|---|---|---|---|")
for n in range(1, 5):
    for used in itertools.combinations(range(4), n):
        R = rec(mod.base(used=list(used), sel=used[0]))
        plus = [t["x"] for t in R["texts"] if t["s"] == "+"]
        chips = [t["x"] for t in R["texts"] if t["ycap"] == 5 and t["s"] in "1234" and len(t["s"]) == 1 and 46 <= t["x"] < 100]
        gap = [i for i in range(4) if i not in used]
        print("| %s | %s | %s | %s |" % (list(used), plus or "none", chips, "non-contiguous: empty slot(s) %s%s" % (gap, "" if (not plus or (plus[0] - 46 - 2) // 14 == min(gap)) else " but '+' elsewhere") if (max(used) - min(used) + 1 != len(used)) or min(used) != 0 else ""))
print()
k = [kd for kd in A.ALGOS["STAGE"]["rows"][0] if kd["label"] == "TOTAL"][0]
worst = 0; viol = []
for v in range(k["min"], k["max"] + 1):
    for heard in (None, v - 1, v):
        st = {"heard": heard} if heard is not None else None
        S = mod.base(sel=0, vals={1: v}, algo_state=st); S["chans"][0]["algo"] = "STAGE"
        R = rec(S); vt = A.value_text(k, v, st)
        for t in R["texts"]:
            if t["s"] == vt and t["ycap"] >= 47 and 61 <= t["x"] <= 121:
                ib = M.text_ink(AF_M if t["font"] == "M" else AF_L, vt); l = t["x"] + ib[0] - 61; r = t["x"] + ib[2] - 119
                worst = max(worst, t["w"])
                if l < 0 or r > 0: viol.append((v, heard, vt, t["font"], l, r))
print("STAGE TOTAL %d..%d x heard in {none, v-1, v}: widest text advance %d px; value ink leaving tile 2: %s" % (k["min"], k["max"], worst, viol or "none"))
k = [kd for kd in A.ALGOS["PATTERNS"]["rows"][1] if kd["label"] == "PICK"][0]
print("PATTERNS PICK 1..120: widest %r %d px in AF_M" % (A.widest_value(k, text_w, AF_M), text_w(AF_M, A.widest_value(k, text_w, AF_M))))
