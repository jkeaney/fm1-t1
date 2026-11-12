from lib import *
import collections
for n in ["playing", "dice", "turn_algo", "hold_home", "added_after", "rec_playing"]:
    sc, tx, arts = texts_with_bbox(scene(n))
    print("\n## scene", n)
    print("| # | string | font | fg | bg | ink bbox (x0,y0,x1,y1) |\n|--|--|--|--|--|--|")
    for i, r in enumerate(tx):
        print("| %d | %r | %s | %s | %s | %s |" % (i, r["s"], r["font"], hexc(r["fg"]), hexc(r["bg"]), r["bbox"]))
    c = collections.Counter(r["font"] for r in tx)
    ch = collections.Counter()
    for r in tx: ch[r["font"]] += len(r["s"])
    print("totals strings:", dict(c), "chars:", dict(ch), "all strings", len(tx), "total chars", sum(ch.values()))
    print("icon art calls (p.art):", len(arts), " (+ drawn-by-rect symbols: REC dot/play mark if rec header, tile glyphs excluded)")
