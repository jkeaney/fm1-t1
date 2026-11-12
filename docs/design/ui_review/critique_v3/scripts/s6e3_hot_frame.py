# SPDX-License-Identifier: GPL-3.0-only (evidence script; run from the repo root with .venv/bin/python)
import sys,os; sys.path.insert(0,"docs/design/ui_review/critique_v3")
from evidence_b_lib import *
mock.palette_set("GREY")
ALG=("ACID","EUCLID","SNAKE","STAGE","ARP","PATTERNS")
ink=lambda f,s:(lambda b:b[2]-b[0])(text_ink(f,s))
over=[]
for a in range(6):
  for row in (0,1):
    for k in range(4):
      idx=row*4+k; lab,lo,hi,en=P.KNOBS[a][idx]
      for v in range(lo,hi+1):
        l,t,f,m=P.knob_view(a,idx,v)
        lw=ink(AF_M,l); vw=ink(f,t)
        # hot frame interior = 54 px (2 px frame each side of 58)
        if lw>54 or vw>54: over.append((ALG[a],row+1,"K%d"%(k+1),l,lw,t,f.name,vw))
seen=set()
for o in over:
    key=(o[0],o[1],o[2],o[3] if o[4]>54 else "",o[5] if o[7]>54 else "")
    if key in seen: continue
    seen.add(key); print("hot-frame interior 54 px exceeded:",o)
OUT="docs/design/ui_review/critique_v3/edge"
s=scene("glance_playing"); s["channels"][0]["algo"]=4; s["row"]=1; s["knobs"]=[0,0,0,0,10,8,10,4]; s["hot"]=[1,1,1,1]
sc,w,recs=render(s,"h"); save(sc,OUT,"e_hot_ARP_row2_LEGATO_ORD"); print("warn",w)
s=scene("glance_playing"); s["channels"][0]["algo"]=1; s["row"]=1; s["knobs"]=[0,0,0,0,16,12,40,0]; s["hot"]=[0,0,0,1]
sc,w,recs=render(s,"h"); save(sc,OUT,"e_hot_EUCLID_row2_AUT"); print("warn",w)
# pixel proof: count pixels within 2 px of the knob-cell edge that differ from SURF/TEXT-frame in the ink row of ARP LEGATO
s=scene("glance_playing"); s["channels"][0]["algo"]=4; s["row"]=1; s["knobs"]=[0,0,0,0,10,8,10,4]; s["hot"]=[0,1,0,0]
sc,w,recs=render(s,"h")
lab=[r for r in recs if r["role"]=="knob label"][1]; print("ARP S2 LEGATO hot: label ink",lab["ink"],"cell x 61..119, frame columns 61,62 and 117,118 ; ink x0 %d x1(excl) %d"%(lab["ink"][0],lab["ink"][2]))
