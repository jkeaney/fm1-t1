import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from common import *
S = scenes()
M = {"INK":".","SURF":"s","CH1":"1","CH2":"2","CH3":"3","CH4":"4","PHC":"p","DIM":"d","WHITE":"W"}
for n in ("dice","two_channels","playing","one_channel"):
    sc, cv, rec = render(S[n]); px = sc.px
    used = S[n]["used"]
    print(n, "used", used)
    # find step with max simultaneous colours at bottom row y=123 and print its pixel run & the next gap
    best=None
    for st in range(16):
        s = "".join(M.get(name_of(px[123*240+x]),"?") for x in range(st*15, st*15+16))
        c = len(set(s)-{"."})
        if best is None or c>best[0]: best=(c,st,s)
    print("   richest step", best[1]+1, "pixels x=%d..%d y=123:" % (best[1]*15, best[1]*15+15), best[2])
