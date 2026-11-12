from common import *
from collections import Counter
CAPH = {"AF_S": 9, "AF_M": 11, "AF_L": 21}
SC = ["playing", "dice", "turn_select", "hold_save", "midi_off_after", "rec_playing"]
for n in SC:
    r = rec(n)
    print("\n## %s" % n)
    print("| string | font | x | cap-y | advance w | fg | bg | contrast |")
    print("|---|---|---|---|---|---|---|---|")
    cnt = Counter(); chars = Counter()
    for t in sorted(r["texts"], key=lambda t: (t["ycap"], t["x"])):
        cr = contrast_ratio(t["fg"], t["bg"])
        print("| `%s` | %s | %d | %d | %d | %s | %s | %.2f |" % (t["s"], t["font"], t["x"], t["ycap"], t["w"], tokname(t["fg"]), tokname(t["bg"]), cr))
        cnt[t["font"]] += 1; chars[t["font"]] += len(t["s"])
    print("totals strings by font:", dict(cnt), " chars:", dict(chars), " total strings", sum(cnt.values()))
    import icons_v10 as IV
    nm = {}
    for k_, v_ in list(IV.ICONS.items()) + list(el.IC.items()): nm.setdefault(v_.strip("\n"), k_)
    arts = r["arts"]
    print("icons drawn (distinct x,y,bitmap): %d ; button icons %d ; other %d ; lit pixels total %d" % (len(arts), sum(1 for a in arts if 146 <= a["y"] < 182), sum(1 for a in arts if not 146 <= a["y"] < 182), sum(a["bits"] for a in arts)))
    print("  non-button: " + ", ".join("%s@%d,%d %dx%d" % (nm.get(a["art"].strip("\n"), "?"), a["x"], a["y"], a["w"], a["h"]) for a in arts if not 146 <= a["y"] < 182))
    print("draw warnings (excl. the initial 240x240 begin):", [w for w in r["cv"].warnings if "CV_MAX" not in w])
