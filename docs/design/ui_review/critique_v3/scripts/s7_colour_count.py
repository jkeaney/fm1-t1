# SPDX-License-Identifier: GPL-3.0-only (evidence script; run from the repo root with .venv/bin/python)
import sys; sys.path.insert(0,"docs/design/ui_review/critique_v3")
from evidence_b_lib import *
mock.palette_set("GREY")
tok={n:getattr(T,n) for n in "BG SURF TEXT THEME ACCENT MID DIM LINE SEL TINT INK REC RAISE KEY LANE GRID".split()}
tokv={}
for n,v in tok.items(): tokv.setdefault(v,[]).append("T_"+n)
new={P.T_PLAYHEAD:"T_PLAYHEAD",P.T_ACCSTEP:"T_ACCSTEP",P.T_RECRED:"T_RECRED"}
used={}
C=mock.Canvas
orig=(C.rect,C.frame,C.pset,C.text_flags,C.begin)
def wrap(name):
    o=getattr(C,name)
    def f(self,*a,**k):
        used.setdefault(cur[0],set())
        return o(self,*a,**k)
    return f
cur=[None]
rec_col=[]
def rect(self,x,y,w,h,c): rec_col.append(c); return orig[0](self,x,y,w,h,c)
def pset(self,x,y,c): rec_col.append(c); return orig[2](self,x,y,c)
def begin(self,w,h,bg=None):
    if bg is not None: rec_col.append(bg)
    return orig[4](self,w,h,bg)
def tf(self,x,y,f,s,fg,bg,flags=0): rec_col.append(fg); rec_col.append(bg); return orig[3](self,x,y,f,s,fg,bg,flags)
C.rect=rect; C.pset=pset; C.begin=begin; C.text_flags=tf
# frame() calls self.rect so it is covered
for n in ("glance_idle","glance_playing","row2","mutes","stress","stress_row2","glance_playing_mono","stress_mono"):
    rec_col.clear()
    sc,w,recs=render(scene(n),n)
    cols=set(rec_col)
    t=sorted("/".join(tokv[c]) for c in cols if c in tokv); nw=sorted(new[c] for c in cols if c in new); other=[hex(c) for c in cols if c not in tokv and c not in new]
    px=set(sc.px)
    print("%-20s drawn flat colours: %d = %d token-valued (%s) + %d new (%s) + %d other %s ; distinct pixels %d (the rest are text anti-alias ramp values: %d)"%(n,len(cols),len(t),", ".join(t),len(nw),", ".join(nw) or "-",len(other),other,len(px),len(px)-len(cols)))
