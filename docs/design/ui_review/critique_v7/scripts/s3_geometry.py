import sys; sys.path.insert(0, "/home/claude/fm1/docs/design/ui_review/critique_v7/scripts")
from lib import *
import collections
out = []; P = out.append
variant("mock")
ICON_INK = {}

def art_bbox(o):
    rows = o["art"].strip("\n").split("\n")
    xs = [i for r in rows for i, c in enumerate(r) if c == "#"]; ys = [j for j, r in enumerate(rows) for c in r if c == "#"]
    return (o["x"] + min(xs), o["y"] + min(ys), o["x"] + max(xs) + 1, o["y"] + max(ys) + 1)

def elements(r):
    els = []
    for o in r["ops"]:
        if o["k"] == "hatch": continue
        if o["k"] == "art": bb = art_bbox(o)
        elif o["k"] == "line": continue
        else: bb = (o["x"], o["y"], o["x"] + o["w"], o["y"] + o["h"])
        els.append(dict(bb=bb, fn=o["fn"], k=o["k"], c=o["c"], seq=o["seq"]))
    for t in r["texts"]:
        if t["ink"] and not t["occluded"]:
            els.append(dict(bb=t["ink"], fn=t["fn"], k="text:" + t["s"], c=t["fg"], seq=t["seq"]))
    return els

def sep(a, b):
    gx = max(a[0] - b[2], b[0] - a[2]); gy = max(a[1] - b[3], b[1] - a[3])
    return gx, gy
def contains(a, b): return a[0] <= b[0] and a[1] <= b[1] and a[2] >= b[2] and a[3] >= b[3]

P("# 4. Geometry")
for sn in SCENES:
    r = capture(S()[sn]); sc = r["screen"]
    P("")
    P("## scene %s" % sn)
    els = elements(r)
    # bands by caller
    by = collections.defaultdict(list)
    for e in els: by[e["fn"]].append(e["bb"])
    P("| band (caller fn) | x-range | y-range (ink, half-open) | height px |")
    P("|---|---|---|---|")
    bands = []
    for fn, bbs in by.items():
        b = (min(x[0] for x in bbs), min(x[1] for x in bbs), max(x[2] for x in bbs), max(x[3] for x in bbs))
        bands.append((b[1], fn, b))
    bands.sort()
    for y0, fn, b in bands:
        P("| %s | %d..%d | %d..%d | %d |" % (fn, b[0], b[2], b[1], b[3], b[3] - b[1]))
    P("gaps between consecutive bands (empty px rows between bbox end and next bbox start; negative = overlap):")
    for (y0, fn, b), (y1, fn2, b2) in zip(bands, bands[1:]):
        P("- %s -> %s : %d px" % (fn, fn2, b2[1] - b[3]))
    # pixel row occupancy (non-BG rows)
    bgc = T.BG
    occ = [any(sc.px[y * 240 + x] != bgc for x in range(240)) for y in range(240)]
    runs = []; s = None
    for y in range(241):
        v = occ[y] if y < 240 else False
        if v and s is None: s = y
        if not v and s is not None: runs.append((s, y)); s = None
    P("rendered pixel rows with any non-BG pixel, runs (half-open): %s ; empty-row gaps: %s" % (runs, [(runs[i][1], runs[i + 1][0], runs[i + 1][0] - runs[i][1]) for i in range(len(runs) - 1)]))
    P("rows 0..239 empty at top: %d, bottom: %d" % (runs[0][0], 240 - runs[-1][1]))
    # strip edge
    cross_t = [(t["s"], t["ink"]) for t in r["texts"] if t["ink"] and t["ink"][1] < 120 < t["ink"][3]]
    P("text ink boxes crossing y=120: %s; text cut warnings: %s; mock warnings: %s" % (cross_t or "none", [t["s"] for t in r["texts"] if t["cut"]] or "none", r["warnings"] or "none"))
    near = sorted([(abs(((t["ink"][1] + t["ink"][3]) / 2) - 120), t["s"], t["ink"]) for t in r["texts"] if t["ink"] and not t["occluded"]])[:2]
    P("nearest visible text to y=120: %s" % (near,))
    cross_o = collections.Counter(o["fn"] + ":" + o["k"] for o in r["ops"] if o["k"] in ("rect", "frame", "art") and o["y"] < 120 < o["y"] + o["h"])
    P("ops (rect/frame/art) straddling y=120 (drawn in both strips): %s" % dict(cross_o))
    # tight gaps
    tight = collections.defaultdict(list); overl = collections.defaultdict(list)
    for i in range(len(els)):
        for j in range(i + 1, len(els)):
            a, b = els[i], els[j]
            if contains(a["bb"], b["bb"]) or contains(b["bb"], a["bb"]): continue
            gx, gy = sep(a["bb"], b["bb"])
            if gx < 0 and gy < 0:
                overl[(a["fn"], a["k"].split(":")[0], b["fn"], b["k"].split(":")[0])].append((a["bb"], b["bb"]))
            else:
                d = max(gx, gy)
                if gx >= 0 and gy >= 0: d = max(gx, gy)
                elif gx >= 0: d = gx
                else: d = gy
                if d < 2:
                    tight[(a["fn"], a["k"].split(":")[0], b["fn"], b["k"].split(":")[0], d)].append((a["bb"], b["bb"]))
    P("element pairs (non-contained) with 0 <= gap < 2 px (gap = empty px between bboxes; 0 = touching), grouped:")
    for k, v in sorted(tight.items()):
        if k[0] == k[2] == "music_slim": continue
        P("- %s/%s ~ %s/%s gap=%d : %d pair(s), e.g. %s" % (*k, len(v), v[0]))
    ms = [(k, v) for k, v in tight.items() if k[0] == k[2] == "music_slim"]
    P("music_slim intra-row pairs with gap<2: %s" % {("%s~%s gap=%d" % (k[1], k[3], k[4])): len(v) for k, v in ms})
    P("partially overlapping (not contained) element pairs, grouped:")
    for k, v in sorted(overl.items()):
        P("- %s/%s ~ %s/%s : %d pair(s), e.g. %s" % (*k, len(v), v[0]))
    if not overl: P("- none")

# icon sizes
P("")
P("## icon pixel sizes (icons_v7.ICONS, 1 bit/px)")
import icons_v7 as IV
P("| icon | rows x cols (nominal) | ink bbox w x h | ink px (set) | bytes at 1 bpp (rows*ceil(cols/8)) |")
P("|---|---|---|---|---|")
tot_bytes = 0
for n, a in IV.ICONS.items():
    rows = a.strip("\n").split("\n"); R = len(rows); C = max(len(r) for r in rows)
    xs = [i for r in rows for i, c in enumerate(r) if c == "#"]; ys = [j for j, r in enumerate(rows) for c in r if c == "#"]
    byt = R * ((C + 7) // 8); tot_bytes += byt
    P("| %s | %dx%d | %dx%d | %d | %d |" % (n, C, R, max(xs) - min(xs) + 1, max(ys) - min(ys) + 1, len(xs), byt))
P("total distinct button icon bitmaps=%d, bytes (row-padded to byte) = %d, bits (w*h) = %d" % (len(IV.ICONS), tot_bytes, sum(max(len(r) for r in a.strip("\n").split("\n")) * len(a.strip("\n").split("\n")) for a in IV.ICONS.values())))
# other bitmaps drawn on the screen (el.IC / ALGO_IC)
used = set()
for sn in SCENES:
    for o in capture(S()[sn])["ops"]:
        if o["k"] == "art": used.add(o["art"])
P("distinct art strings drawn across the six scenes: %d" % len(used))
shapes = collections.Counter()
inv = {v: n for n, v in IV.ICONS.items()}
for sym, a in el.IC.items(): inv.setdefault(a, "IC." + sym)
for sym, a in el.ALGO_IC.items(): inv.setdefault(a, "ALGO." + sym)
rowsum = 0
P("| bitmap used | nominal cols x rows | bytes@1bpp |")
for a in sorted(used, key=lambda a: inv.get(a, "?")):
    rows = a.strip("\n").split("\n"); R = len(rows); C = max(len(r) for r in rows)
    b = R * ((C + 7) // 8); rowsum += b
    P("| %s | %dx%d | %d |" % (inv.get(a, "?"), C, R, b))
P("total bytes of bitmaps drawn (row-padded) = %d" % rowsum)
# placement in button cell
P("")
P("## button cells")
r = capture(S()["playing"])
cells = [o for o in r["ops"] if o["fn"] == "buttons7" and o["k"] == "rect"]
P("cells: %d; sizes %s; x starts %s; y starts %s" % (len(cells), set((o["w"], o["h"]) for o in cells), sorted(set(o["x"] for o in cells)), sorted(set(o["y"] for o in cells))))
P("cell 38x24 px = %.2f x %.2f mm at 8 px/mm; pitch 40 x 26 px; gaps between cells 2 px (x) and 2 px (y); block spans y %d..%d" % (38 / 8, 24 / 8, min(o["y"] for o in cells), max(o["y"] + o["h"] for o in cells)))
arts = [o for o in r["ops"] if o["fn"] == "buttons7" and o["k"] == "art"]
for o, c in zip(arts, cells):
    bb = art_bbox(o)
    P("- cell(%d,%d) icon ink bbox %s margins L%d R%d T%d B%d (ink %dx%d)" % (c["x"], c["y"], bb, bb[0] - c["x"], c["x"] + c["w"] - bb[2], bb[1] - c["y"], c["y"] + c["h"] - bb[3], bb[2] - bb[0], bb[3] - bb[1]))
# keys
P("")
P("## keybed vs the 15 px column grid (playing scene)")
rr = [o for o in r["ops"] if o["fn"] == "footer_keybed" and o["k"] == "rect"]
whites = [o for o in rr if o["w"] == 14]
P("white-key rects (w=14): %d; x starts %s ... pitch %s; widths %s; heights %s" % (len(whites), [o["x"] for o in whites][:4], sorted(set(b["x"] - a["x"] for a, b in zip(sorted(set(o["x"] for o in whites)) and [dict(x=x) for x in sorted(set(o["x"] for o in whites))], [dict(x=x) for x in sorted(set(o["x"] for o in whites))][1:]))), set(o["w"] for o in whites), set(o["h"] for o in whites)))
blk = [o for o in rr if o["w"] == 10]
P("black-key bodies (w=10): %d; x starts %s; centres (x+5) %s" % (len(blk), [o["x"] for o in blk], [o["x"] + 5 for o in blk]))
gaps = [o for o in rr if o["w"] == 12]
P("black-key BG surrounds (w=12): %d at x %s" % (len(gaps), [o["x"] for o in gaps]))
P("white-key x spans: %s" % [(o["x"], o["x"] + o["w"] - 1) for o in sorted(whites, key=lambda o: o["x"])])
P("1 px empty column between adjacent white keys (pitch 15, width 14); black-key BODY 10 px wide = 0.67 of pitch; body+surround 12 px = 0.8 of pitch; white key 14 px = 1.75 mm, black body 10 px = 1.25 mm")
P("black-key centre = boundary between white keys N and N+1 (white keys N at x=15(N-1)+1..15(N-1)+14, the empty column at x=15N): body spans cx-5..cx+4, i.e. centred 0.5 px left of the boundary column")
mus = [o for o in r["ops"] if o["fn"] == "music_slim" and o["k"] in ("rect", "frame")]
P("music row column pitch 15 px (x=i*15+1), inner 13 px; bars per step widths used (playing): %s" % dict(collections.Counter(o["w"] for o in mus if o["h"] > 1)))
for sn in SCENES:
    rq = capture(S()[sn])
    m = [o for o in rq["ops"] if o["fn"] == "music_slim" and o["k"] in ("rect", "frame") and o["h"] > 1 and o["c"] != el.PH]
    P("scene %s: bar width histogram (px:count) %s; min bar height %s; muted outline frames at width<=2 (no interior): %d; hatch ops %d (widths %s)" % (sn, dict(sorted(collections.Counter(o["w"] for o in m if o["w"] < 14).items())), min(o["h"] for o in m), sum(1 for o in m if o["k"] == "frame" and o["w"] <= 2), sum(1 for o in rq["ops"] if o["k"] == "hatch"), sorted(set(o["w"] for o in rq["ops"] if o["k"] == "hatch"))))
open("/home/claude/fm1/docs/design/ui_review/critique_v7/scripts/s3_out.md", "w").write("\n".join(out))
print("\n".join(out))
