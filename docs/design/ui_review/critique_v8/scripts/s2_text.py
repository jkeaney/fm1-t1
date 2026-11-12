import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from common import *
MAIN = ["playing","two_channels","one_channel","sel_muted","dice","turning_k2","rec_armed","midi_off"]
S = scenes()
out = []
allc = {}
for n in MAIN:
    ART.clear()
    sc, cv, rec = render(S[n])
    arts = list(ART)
    print("=== %s: %d text draws, %d icon draws" % (n, len(rec), len(arts)))
    tot = {}
    for r in rec:
        tot.setdefault(r["font"], [0, 0]); tot[r["font"]][0] += 1; tot[r["font"]][1] += len(r["s"])
        a = r["ink"]
        print("  %-22r %s fg=%s bg=%s ink=%s adv=%d" % (r["s"], r["font"], name_of(r["fg"]), name_of(r["bg"]), a, r["adv"]))
    print("  totals per font (draws, chars):", tot)
    roles = sorted({(r["font"], name_of(r["fg"]), name_of(r["bg"])) for r in rec})
    print("  distinct (font,fg,bg) roles:", len(roles), roles)
    print("  distinct fonts:", sorted({r['font'] for r in rec}), " distinct strings:", len({r['s'] for r in rec}))
    from collections import Counter
    c = Counter(a["name"] for a in arts)
    print("  icons: %d draws, %d distinct: %s" % (len(arts), len(c), dict(c)))
