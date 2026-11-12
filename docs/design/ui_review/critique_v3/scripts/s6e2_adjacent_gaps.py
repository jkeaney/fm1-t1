# SPDX-License-Identifier: GPL-3.0-only (evidence script; run from the repo root with .venv/bin/python)
import sys; sys.path.insert(0,"docs/design/ui_review/critique_v3")
from evidence_b_lib import *
mock.palette_set("GREY")
ALG=("ACID","EUCLID","SNAKE","STAGE","ARP","PATTERNS")
ink=lambda f,s:(lambda b:b[2]-b[0])(text_ink(f,s))
worstL=(99,);worstV=(99,)
for a in range(6):
  for row in (0,1):
    for mode in ("min","max","long"):
        kn=[]
        for k in range(8):
            lab,lo,hi,en=P.KNOBS[a][k]
            if mode=="min": kn.append(lo)
            elif mode=="max": kn.append(hi)
            else: kn.append(max(range(lo,hi+1),key=lambda v:(ink(P.knob_view(a,k,v)[2],P.knob_view(a,k,v)[1]),ink(AF_M,P.knob_view(a,k,v)[0]))))
        s=scene("glance_playing"); s["channels"][0]["algo"]=a; s["row"]=row; s["knobs"]=kn
        sc,w,recs=render(s,"k")
        for role,store in (("knob label",0),("knob value",1),("knob value (M)",1)):
            pass
        labs=[r for r in recs if r["role"]=="knob label"]
        vals=[r for r in recs if r["role"] in("knob value","knob value (M)")]
        for lst,nm in ((labs,"label"),(vals,"value")):
            for i in range(3):
                g=lst[i+1]["ink"][0]-lst[i]["ink"][2]
                if nm=="label" and g<worstL[0]: worstL=(g,ALG[a],row+1,mode,lst[i]["s"],lst[i+1]["s"])
                if nm=="value" and g<worstV[0]: worstV=(g,ALG[a],row+1,mode,lst[i]["s"],lst[i+1]["s"])
        if w: print("WARN",w)
print("tightest gap between adjacent knob LABEL inks (px):",worstL)
print("tightest gap between adjacent knob VALUE inks (px):",worstV)
