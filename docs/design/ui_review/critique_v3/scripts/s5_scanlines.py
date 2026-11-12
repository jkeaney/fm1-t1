# SPDX-License-Identifier: GPL-3.0-only (evidence script; run from the repo root with .venv/bin/python)
import sys; sys.path.insert(0,"docs/design/ui_review/critique_v3")
from evidence_b_lib import *
mock.palette_set("GREY")
names={}
for n in "BG SURF TEXT MID SEL INK RAISE LANE LINE".split(): names.setdefault(getattr(T,n),n)
names[P.T_PLAYHEAD]="PH"; names[P.T_ACCSTEP]="AC"
def runs_row(sc,y,x0=0,x1=240):
    out=[];s=x0;c=sc.px[y*240+x0]
    for x in range(x0+1,x1+1):
        cc=sc.px[y*240+x] if x<x1 else None
        if cc!=c: out.append((s,x-s,names.get(c,"0x%04X"%c))); s=x;c=cc
    return out
def runs_col(sc,x,y0=0,y1=240):
    out=[];s=y0;c=sc.px[y0*240+x]
    for y in range(y0+1,y1+1):
        cc=sc.px[y*240+x] if y<y1 else None
        if cc!=c: out.append((s,y-s,names.get(c,"0x%04X"%c))); s=y;c=cc
    return out
sc,w,recs=render(scene("glance_playing"),"gp")
print("zones tile:", [(z[0],z[1],z[2],z[1]+z[2]) for z in P.ZONES], "sum", sum(z[2] for z in P.ZONES))
print("blits A,B,C heights", P.BLIT_A_H,P.BLIT_B_H,P.BLIT_C_H,"offsets",0,P.Z2_Y,P.Z3_Y,"sum",P.BLIT_A_H+P.BLIT_B_H+P.BLIT_C_H)
print("marks (screen coords) in glance_playing:"); 
for m in sc.marks: print("  ",m)
print("--- channel row scan y=60 (below text) glance_playing"); print(runs_row(sc,60))
print("--- channel col scan x=5 y 20..80"); print(runs_col(sc,5,0,80))
print("--- lane row scan y=100"); print(runs_row(sc,100))
print("--- lane col scan x=7 (y 72..146)"); print(runs_col(sc,7,70,146))
print("--- lane col scan x=82 (playhead col)"); print(runs_col(sc,82,70,146))
print("--- knob row scan y=150"); print(runs_row(sc,150,0,240))
print("--- knob col scan x=3 y 140..240"); print(runs_col(sc,3,140,240))
print("--- row bar scan y=213"); print(runs_row(sc,213))
print("--- row bar col scan x=60 y 205..225"); print(runs_col(sc,60,205,225))
print("--- header scan y=12 x 0..240"); print(runs_row(sc,12))
print("--- meter row y=196"); print(runs_row(sc,196,0,240))
print("--- meter col x=20 y 190..210"); print(runs_col(sc,20,185,215))
sc2,w,recs2=render(scene("stress"),"st")
print("--- STRESS lane row y=100 (hollow/rest/ratchet)"); print(runs_row(sc2,100))
print("--- STRESS channel cell scan y=60"); print(runs_row(sc2,60))
