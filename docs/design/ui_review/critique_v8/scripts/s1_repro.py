import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from common import *
from PIL import Image, ImageChops
import subprocess
out = "/tmp/claude-0/v8b"
r = subprocess.run("cd %s && python3 ../../mock/mock.py render v8.py --out %s" % (V8D, out), shell=True, capture_output=True, text=True)
print("render rc", r.returncode, "stderr:", r.stderr.strip()[:300]); print([l for l in r.stdout.splitlines() if "warn" in l.lower()] or "no warnings")
tot = 0
for n in sorted(scenes()):
    a = Image.open("%s/v8_%s.png" % (out, n)).convert("RGB"); b = Image.open("%s/out/v8_%s.png" % (V8D, n)).convert("RGB")
    d = ImageChops.difference(a, b); bb = d.getbbox()
    nd = sum(1 for p in d.getdata() if p != (0,0,0))
    tot += nd
    print("%-14s size %s diff_px=%d bbox=%s" % (n, a.size, nd, bb))
print("TOTAL differing px", tot)
