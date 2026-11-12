from common import *
from PIL import Image, ImageChops
res = []
for n in mod.STATE["scenes"]:
    sc, _ = render(scene(n))
    im = sc.to_image()
    ref = Image.open(os.path.join(V9, "out", "v9_%s.png" % n)).convert("RGB")
    if im.size != ref.size: res.append((n, "size", im.size, ref.size)); continue
    d = ImageChops.difference(im, ref); bb = d.getbbox()
    cnt = sum(1 for p in d.getdata() if p != (0,0,0))
    res.append((n, cnt, bb))
for r in res: print(r)
