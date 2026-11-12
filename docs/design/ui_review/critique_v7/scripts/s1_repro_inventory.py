import sys; sys.path.insert(0, "/home/claude/fm1/docs/design/ui_review/critique_v7/scripts")
from lib import *
from PIL import Image, ImageChops
import collections
out = []
P = out.append
P("## 1. Reproduction (1x PNG pixel diff vs proposals/v7/out)")
for sn in SCENES:
    sc = capture(S()[sn])["screen"]
    img = sc.to_image().convert("RGB")
    ref = Image.open(V7 + "/out/v7_%s.png" % sn).convert("RGB")
    d = ImageChops.difference(img, ref)
    n = sum(1 for p in d.get_flattened_data() if p != (0, 0, 0))
    P("- %s: size %s vs ref %s, differing pixels = %d" % (sn, img.size, ref.size, n))
# icons sheet too
sc = capture(S()["icons"])["screen"]; img = sc.to_image().convert("RGB"); ref = Image.open(V7 + "/out/v7_icons_4x.png")
P("- icons (sheet, 1x re-render; reference only exists at 4x: %s): not diffed at 1x" % (ref.size,))
P("")
P("## 2. Text inventory")
tot = {}
roles_all = collections.OrderedDict()
for sn in SCENES:
    r = capture(S()[sn])
    P("### scene %s: %d text draws; warnings: %s" % (sn, len(r["texts"]), r["warnings"] or "none"))
    P("| text | font | fg (token) | bg (token) | x | y(cap top) | ink bbox | cap mm | arcmin | caller | overpainted by later rect |")
    P("|---|---|---|---|---|---|---|---|---|---|---|")
    cnt = collections.Counter(); vis = collections.Counter()
    for t in sorted(r["texts"], key=lambda t: (t["ycap"], t["x"])):
        f = {"AF_S": AF_S, "AF_M": AF_M, "AF_L": AF_L}[t["f"]]
        cnt[t["f"]] += 1
        if not t["occluded"]: vis[t["f"]] += 1
        P("| %r | %s | %s %s | %s %s | %d | %d | %s | %.3f | %.1f | %s | %s |" % (t["s"], t["f"], hexc(t["fg"]), tokname(t["fg"]), hexc(t["bg"]), tokname(t["bg"]), t["x"], t["ycap"], t["ink"], mm(f), arcmin(f), t["fn"], "YES" if t["occluded"] else "no"))
        if not t["occluded"]: roles_all.setdefault((t["fn"], t["f"], tokname(t["fg"])), set()).add(sn)
    P("totals per font: " + ", ".join("%s=%d" % kv for kv in sorted(cnt.items())) + "; total=%d; characters=%d" % (sum(cnt.values()), sum(len(t["s"]) for t in r["texts"])))
    P("visible (not overpainted) text per font: " + ", ".join("%s=%d" % kv for kv in sorted(vis.items())) + "; visible total=%d" % sum(vis.values()))
    # non-text marks
    arts = [o for o in r["ops"] if o["k"] == "art"]
    P("icon (art) draws: %d; sizes: %s" % (len(arts), dict(collections.Counter("%dx%d" % (o["w"], o["h"]) for o in arts))))
    byfn = collections.Counter(o["fn"] for o in arts)
    P("icons by caller: %s" % dict(byfn))
    ic_btn = sum(1 for o in arts if o["fn"] == "buttons7")
    # key glyphs: footer_keybed arts (PG-, PG+, RST, rec) + CHN 4 dots + M squares + SOL/FIL/ROL letters
    kg = [o for o in arts if o["fn"] == "footer_keybed"]
    P("button icons=%d; keybed icon bitmaps=%d (art draws in footer_keybed); header/other icon draws=%d" % (ic_btn, len(kg), len(arts) - ic_btn - len(kg)))
    P("")
P("### distinct text roles (caller function, font, fg token) over all six scenes: %d" % len(roles_all))
for k, v in roles_all.items():
    P("- %s %s %s : in %s" % (k[0], k[1], k[2], sorted(v)))
open("/home/claude/fm1/docs/design/ui_review/critique_v7/scripts/s1_out.md", "w").write("\n".join(out))
print("\n".join(out))
