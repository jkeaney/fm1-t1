from lib import *
from PIL import Image
import sys
rows = []
tot = 0
for n in mod.STATE["scenes"]:
    sc, cv, rec, arts = render(scene(n))
    img = sc.to_image()
    ref = R + "/proposals/v11/out/v11_%s.png" % n
    if not os.path.exists(ref): rows.append((n, "NO REF")); continue
    d = mock.diff_images(img, Image.open(ref).convert("RGB"))
    rows.append((n, d[0], d[2][:3] if d[0] else ""))
for r in rows: print(*r)
print("scenes", len(rows), "total diff px", sum(r[1] for r in rows if isinstance(r[1], int)))
