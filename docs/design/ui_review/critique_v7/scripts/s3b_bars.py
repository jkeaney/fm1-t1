import sys; sys.path.insert(0, "/home/claude/fm1/docs/design/ui_review/critique_v7/scripts")
from lib import *
import collections
variant("mock")
out=[]; P=out.append
P("## 4b. music row bar statistics (bars = music_slim rect/frame ops, 90<=y<124, h>=4, w<15; accent caps/ticks/dashes excluded)")
for sn in SCENES:
    r = capture(S()[sn])
    bars = [o for o in r["ops"] if o["fn"] == "music_slim" and o["k"] in ("rect", "frame") and 90 <= o["y"] < 124 and o["h"] >= 4 and o["w"] < 15]
    per = collections.defaultdict(list)
    for o in bars: per[(o["x"] - 1) // 15].append(o)
    kcount = collections.Counter(len(v) for v in per.values())
    ws = collections.Counter(o["w"] for o in bars)
    caps = [o for o in r["ops"] if o["fn"] == "music_slim" and o["k"] == "rect" and o["h"] == 2 and o["w"] > 1]
    dashes = [o for o in r["ops"] if o["fn"] == "music_slim" and o["k"] == "rect" and o["h"] == 1 and o["w"] == 3]
    ticks = [o for o in r["ops"] if o["fn"] == "music_slim" and o["k"] == "rect" and o["w"] == 1]
    fr = [o for o in bars if o["k"] == "frame"]
    P("- %s: steps with sound=%d (bars per step histogram %s); bar width px histogram %s; bar height range %d..%d; muted frames=%d (widths %s); accent caps=%d (widths %s); rest dashes=%d (3x1 px); bar-width-2 solid bars=%d; hatched=%d" % (sn, len(per), dict(sorted(kcount.items())), dict(sorted(ws.items())), min(o["h"] for o in bars), max(o["h"] for o in bars), len(fr), dict(collections.Counter(o["w"] for o in fr)), len(caps), sorted(set(o["w"] for o in caps)), len(dashes), sum(1 for o in bars if o["w"] == 2 and o["k"] == "rect"), sum(1 for o in r["ops"] if o["k"] == "hatch")))
    P("  step sets: used=%s; all-rest steps (no bars): %s" % (S()[sn]["used"], [i for i in range(16) if i not in per]))
    P("  bars with width<=3 : %d of %d; colours among w<=3: %s" % (sum(1 for o in bars if o["w"] <= 3), len(bars), sorted(set(tokname(o["c"]) for o in bars if o["w"] <= 3))))
    px = sorted(set(o["w"] / 8 for o in bars))
    P("  bar widths in mm: %s" % [round(x, 3) for x in px])
print("\n".join(out))
open("/home/claude/fm1/docs/design/ui_review/critique_v7/scripts/s3b_out.md","w").write("\n".join(out))
