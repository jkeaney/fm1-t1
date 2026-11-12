# SPDX-License-Identifier: GPL-3.0-only (evidence script; run from the repo root with .venv/bin/python)
import sys; sys.path.insert(0,"docs/design/ui_review/critique_v3")
from evidence_b_lib import *
mock.palette_set("GREY")
nm={}
for n in "BG SURF TEXT MID SEL INK RAISE LANE".split(): nm.setdefault(getattr(T,n),n)
nm[P.T_PLAYHEAD]="PH"; nm[P.T_ACCSTEP]="AC"; nm[P.T_RECRED]="RD"
def runs(sc,y,x0=0,x1=240):
    out=[];s=x0;c=sc.px[y*240+x0]
    for x in range(x0+1,x1+1):
        cc=sc.px[y*240+x] if x<x1 else None
        if cc!=c: out.append((s,x-s,nm.get(c,hex(c)))); s=x;c=cc
    return out
def col(sc,x,y0,y1):
    out=[];s=y0;c=sc.px[y0*240+x]
    for y in range(y0+1,y1+1):
        cc=sc.px[y*240+x] if y<y1 else None
        if cc!=c: out.append((s,y-s,nm.get(c,hex(c)))); s=y;c=cc
    return out
exec(open("docs/design/ui_review/critique_v3/evidence_b_edge.py").read().split("base = lambda")[0].split("def R(")[0].replace("from evidence_b_lib import *","")) if False else None
def lane_all(on=1,vel=127,acc=1,slide=1,rep=4): return [dict(on=on,vel=vel,ed=0,acc=acc,slide=slide,rep=rep) for _ in range(16)]
def pl(st,ph): n=st["channels"][0]["len"]; st["channels"][0]["p"]=(ph+1)%n; st["playing"]=1; return st
for ph in (0,7,15):
    s=scene("glance_playing"); s["lane"]=lane_all(); pl(s,ph); sc,w,r=render(s,"b")
    print("ph",ph+1,"tie row y=85:",[x for x in runs(sc,85) if x[2] in("TEXT","MID","BG","PH") and x[1]>1][:40])
s=scene("glance_playing"); s["lane"]=lane_all(); pl(s,7); sc,w,r=render(s,"b")
print("col x=7 (step 1):",col(sc,7,72,144)); print("col x=3 (step 1 comb bar):",col(sc,3,72,144)); print("col x=118 (playing, step 8) :",col(sc,118,72,144))
print("playing page-box marker pixels etc")
# d: head colour pixels in lane
s=scene("glance_playing"); c=s["channels"][0]; c["len"]=64; s["vpage"]=1; pl(s,55); sc,w,r=render(s,"d")
lane=sc.px[72*240:142*240]; print("d: PH pixels in lane:",sum(1 for p in lane if p==P.T_PLAYHEAD)," PH pixels in header page boxes:",sum(1 for y in range(0,24) for x in range(160,240) if sc.px[y*240+x]==P.T_PLAYHEAD),"(4x4 marker = 16 expected)")
print("d: header page boxes runs y=12:",runs(sc,12,160,240))
# a: all muted: hollow or not
# arcmin of shapes
sh=[("playhead column",14,70),("comb bar width (x4+)",2,0),("comb bar width x3",3,0),("accent triangle",11,6),("slide tie bar height",15,6),("rest dash",12,4),("beat pip",6,3),("page box",14,14),("page playing marker",4,4),("page box frame stroke",2,0),("muted cell frame stroke",2,0),("hot knob frame stroke",2,0),("strike line thickness",3,0),("M chip",22,18),("OFF chip frame stroke",2,0),("activity square",14,14),("transport glyph",15,16),("ROW bar numeral I (w x h)",3,6),("ROW bar height",118,10),("meter",46,10),("enum pip selected (w x h)",6,10),("enum pip idle (w x h, 14-pip: 2 x 4)",6,4),("enum pip idle 14-pip width",2,4)]
for n,w_,h_ in sh:
    print("%-34s %3dx%-3d px = %.3f x %.3f mm ; short side %.1f arcmin"%(n,w_,h_,w_/8,h_/8,(min(x for x in (w_,h_) if x) /8)/500*3438))
