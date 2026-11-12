from common import *
import itertools, io, contextlib, copy
palette_set("HI-CON")
buf=io.StringIO(); n=0; exc=0; cuts=[]
def go(label,s):
    global n,exc
    try:
        with contextlib.redirect_stdout(buf): sc,log=render(s)
    except Exception as e:
        exc+=1; print("EXC",label,repr(e)); return None
    n+=1
    for t in sc.texts:
        if t[5]&2: cuts.append((label,t))
    return sc
# all non-empty channel sets 1-4 (non-contiguous incl.), each selected, +/- muted, +/- midi off, rec, playing/stopped
for r in range(1,5):
    for used in itertools.combinations(range(4),r):
        for sel in used:
            for mut in (None,) + tuple(used):
                for off in (None,)+tuple(used):
                    for rec in (False,True):
                        s=scene("playing"); s["used"]=list(used); s["sel"]=sel; s["rec"]=rec
                        if mut is not None: s["chans"][mut]["muted"]=True
                        if off is not None: s["chans"][off]["midi"]=None
                        go((used,sel,mut,off,rec),s)
print("channel-set renders",n,"exceptions",exc,"cut texts",len(cuts),cuts[:3])
# unusual: all-rest channel, selected with sel outside used
s=scene("playing"); s["chans"][0]["lane"]=[None]*16; sc=go("allrest",s); print("all-rest ch1 selected: OK" if sc else "FAIL")
for pl in (False,True):
    s=scene("playing"); s["chans"][0]["lane"]=[None]*16; s["playing"]=pl; go("allrest%d"%pl,s)
# sel not in used (index of missing channel)
s=scene("playing"); s["used"]=[0,1]; s["sel"]=3
try: go("sel_unused",s); print("sel not in used: rendered (no exception)")
except Exception as e: print("sel not in used exc",e)
# playhead 0..15 with pend
palette_set("HI-CON")
for ph in range(16):
    s=scene("pending_p02"); s["playhead"]=ph; s["playing"]=True
    sc,log=render(s)
# overlap of P02? pill (x1..38,y77..93) with playhead bar/col
pill=(1,77,1+text_w(AF_M,"P02?")+6,17)
print("P02? pill x",pill[0],"..",pill[0]+pill[2]-1,"y 77..93 ; playhead white bar y90..91 and column y92.. at x=15*i..15*i+14; overlap for playhead cols:",[i for i in range(16) if i*15-1<pill[0]+pill[2]-1 and i*15+14>=pill[0]])
# verify by pixel: P02? scene with playhead 0 : pixel at (5,91)
s=scene("pending_p02"); s["playhead"]=0; sc,_=render(s); print("pixel(5,91) with playhead0 + pending:",hexc(sc.px[91*240+5]),"; pixel(5,85) pill:",hexc(sc.px[85*240+5]))
s=scene("pending_p02"); s["playhead"]=0; s["pend"]=12; sc,_=render(s); print("P12? width",text_w(AF_M,"P12?")+6)
# playhead column vs status text rows? playing: column top y90 vs status dots y82-88: gap
# single-channel stopped
s=scene("one_channel"); print("one_channel lit rows ok")
# sheet scenes
for nm in ("sheet_row1_a","sheet_row1_b","sheet_row2_a","sheet_row2_b"):
    sc,_=render(scene(nm))
print("warnings stdout:",buf.getvalue().count("warning"), buf.getvalue()[:300])
for ph in (0,1,2,3):
    s=scene("pending_p02"); s["playhead"]=ph; sc,_=render(s)
    print("pend+playhead",ph,"pixels row y=92 x=2,20,36:",[hexc(sc.px[92*240+x]) for x in (2,20,36)],"y=89:",[hexc(sc.px[89*240+x]) for x in (2,20,36)],"y=91:",[hexc(sc.px[91*240+x]) for x in (2,20,36)])
