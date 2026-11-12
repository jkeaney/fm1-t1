# SPDX-License-Identifier: GPL-3.0-only (evidence script; run from the repo root with .venv/bin/python)
import sys; sys.path.insert(0,"docs/design/ui_review/critique_v3")
from evidence_b_lib import *
mock.palette_set("GREY")
def lane(f): return [f(i) for i in range(16)]
def px_step(sc,n): return [sc.px[y*240+x] for y in range(72,142) for x in range(15*n,15*n+14)]
def pl(st,ph): n=st["channels"][0]["len"]; st["channels"][0]["p"]=(ph+1)%n; st["playing"]=1; return st
# rep>=6 identical to rep 1?
res={}
for rep in (1,2,3,4,5,6,7,8):
    s=scene("glance_playing"); s["lane"]=lane(lambda i: dict(on=1,vel=100,ed=0,acc=0,slide=0,rep=rep)); sc,w,r=render(s,"x"); res[rep]=px_step(sc,0)
for rep in (2,3,4,5,6,7,8): print("rep",rep,"pixel-identical to rep 1:",res[rep]==res[1])
print("rep6==rep7==rep8:",res[6]==res[7]==res[8])
# rest vs ON vel1..4, normal and playing column, muted
for vel in (1,3,4):
    for mode in ("normal","playing","muted"):
        s=scene("glance_playing"); s["lane"]=lane(lambda i: dict(on=1 if i%2==0 else 0,vel=vel,ed=0,acc=0,slide=0,rep=1))
        s["channels"][0]["p"]=6 if mode=="playing" else 12  # playhead idx 5 (step 6, odd=rest?) idx5 odd -> rest... set below
        if mode=="playing": pl(s,0)       # playhead on step 1 (ON)
        else: pl(s,15)                    # elsewhere
        if mode=="muted": s["channels"][0]["muted"]=1
        sc,w,r=render(s,"x")
        on=px_step(sc,2 if mode!="playing" else 0); rest=px_step(sc,1)
        if mode=="playing":
            s2=copy.deepcopy(s); s2["lane"]=lane(lambda i: dict(on=1 if i%2==1 else 0,vel=vel,ed=0,acc=0,slide=0,rep=1)); pl(s2,0)
            sc2,w,r=render(s2,"x"); rest=px_step(sc2,0)       # rest step under the playhead
        print("vel %d %-7s ON step vs rest step pixel-identical: %s"%(vel,mode,on==rest))
# all muted all off: what does the cell show
s=scene("glance_playing")
for c in s["channels"]: c["muted"]=1; c["midi_ch"]=16
sc,w,recs=render(s,"x"); print("all muted+OFF: roles drawn:",sorted({r['role'] for r in recs}),"M chips:",sum(1 for r in recs if r['role']=="MUTE chip"),"OFF chips:",sum(1 for r in recs if r['role']=="OFF chip"))
