from common import *
import io, contextlib
OUT = "/tmp/claude-0/v10b"
tot = 0
print("# 1 reproduction: CLI render (--out /tmp/claude-0/v10b) vs proposals/v10/out/v10_<scene>.png, pixel diff via mock.diff_images")
for n in mod.STATE["scenes"]:
    a = Image.open(os.path.join(OUT, "v10_%s.png" % n)).convert("RGB")
    p = os.path.join(V10, "out", "v10_%s.png" % n)
    if not os.path.exists(p):
        print("%-16s MISSING shipped png" % n); continue
    b = Image.open(p).convert("RGB")
    r = M.diff_images(a, b)
    print("%-16s %s" % (n, "SIZE DIFF %s %s" % (a.size, b.size) if r is None else "%d differing px %s" % (r[0], r[2][:5])))
    tot += 0 if r is None else r[0]
print("TOTAL differing px", tot)
