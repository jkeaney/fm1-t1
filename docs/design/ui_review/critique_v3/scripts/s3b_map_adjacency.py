# SPDX-License-Identifier: GPL-3.0-only (evidence script; run from the repo root with .venv/bin/python)
import sys; sys.path.insert(0,"docs/design/ui_review/critique_v3")
from evidence_b_lib import *
for s in ("ACCNT","NOTES","SHAPE","ALGORITHM","10","16","OFFSET","PUNCH","CURVE","PRESETS"):
    m=mock.measure(s,"S"); print(s,"adv",m["advance"],"ink",m["ink_w"])
sc,w,recs=render(scene("map"),"map")
print("MAP texts",len(recs))
# touching/overlap: any two ink boxes within 1 px (gap<=0 horizontally while overlapping vertically) or overlapping
inks=[(r["ink"],r["s"],r["role"]) for r in recs]
bad=[]
for i in range(len(inks)):
    for j in range(i+1,len(inks)):
        a,b=inks[i][0],inks[j][0]
        vo = min(a[3],b[3])-max(a[1],b[1])
        ho = min(a[2],b[2])-max(a[0],b[0])
        if vo>0 and ho>-2: bad.append((inks[i][1],inks[j][1],"hgap",-ho,"vov",vo,a,b))
        elif ho>0 and vo>-1 and vo<=0: bad.append((inks[i][1],inks[j][1],"vgap",-vo,"hov",ho,a,b))
for b in bad: print(b)
# screen edge margins
mn=min(r["ink"][0] for r in recs); mx=max(r["ink"][2] for r in recs); my=max(r["ink"][3] for r in recs)
print("ink min x",mn,"max x1",mx,"max y1",my)
# rows beyond y 234 (bezel overscan 236..240)?
print([ (r['s'],r['ink']) for r in recs if r['ink'][3]>232 or r['ink'][2]>238 or r['ink'][0]<2])
