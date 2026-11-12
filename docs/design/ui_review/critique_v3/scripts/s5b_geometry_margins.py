# SPDX-License-Identifier: GPL-3.0-only (evidence script; run from the repo root with .venv/bin/python)
import sys; sys.path.insert(0,"docs/design/ui_review/critique_v3")
from evidence_b_lib import *
print("### geometry vs 5 px module (screen coords)")
els=[]
def add(n,x,y,w,h): els.append((n,x,y,w,h))
add("Z0 transport glyph",6,4,15,16)
for k in range(4): add("page box %d"%(k+1),172+16*k,5,14,14)
add("page playing marker (box1)",172+5,10,4,4)
for i in range(4):
    x=60*i+1;y=26
    add("Z1 cell %d"%(i+1),x,y,58,44)
add("Z1 number box (cell1)",4,31,24,21)
add("Z1 strike (cell1)",7,40,20,3)
add("Z1 OFF chip (cell1)",29,30,29,18)
add("Z1 M chip (cell1)",35,30,22,18)
add("Z1 activity square (cell1)",41,32,14,14)
add("Z2 lane column n=0",0,72,14,70)
add("Z2 lane column n=15",225,72,14,70)
add("Z2 accent triangle",1,75,11,6)
add("Z2 tie bar",0,83,15,6)
add("Z2 bar (full width)",1,72+62-44,12,44)
add("Z2 rest dash",1,130,12,4)
add("Z2 beat pip",4,137,6,3)
for k in range(4):
    add("Z3 knob cell %d"%(k+1),60*k+1,144,58,66)
add("Z3 meter (cell1)",7,194,46,10)
add("Z3 ROW bar left",1,210,118,10)
add("Z3 ROW bar right",121,210,118,10)
add("Z4 cue strip",0,220,240,16)
add("toast block",0,146,240,88)
add("toast countdown",40,214,160,10)
add("toast algo pips",(240-(6*20+5*6))//2,144+68,20,14)
print("| element | x | y | w | h | x%5 | y%5 | w%5 | h%5 | right edge | bottom edge | on 5px grid? |"); print("|---|---|---|---|---|---|---|---|---|---|---|---|")
for n,x,y,w,h in els:
    ok = all(v%5==0 for v in (x,y,w,h))
    print("| %s | %d | %d | %d | %d | %d | %d | %d | %d | %d | %d | %s |"%(n,x,y,w,h,x%5,y%5,w%5,h%5,x+w,y+h,"yes" if ok else "no"))
print()
print("### ink margins of text in its cell (glance_playing, stress, mutes)")
print("| scene | role | string | cell x,y,w,h | ink x0..x1 | L margin | R margin | ink centre - cell centre (x) | T margin | B margin | ink centre - cell centre (y) |"); print("|---|---|---|---|---|---|---|---|---|---|---|")
for sn in ("glance_playing","stress","mutes"):
    sc,w,recs=render(scene(sn),sn)
    for r in recs:
        ink=r["ink"]
        if r["role"] in ("chan number","algo name","OFF chip","MUTE chip"):
            i=min(3,ink[0]//60); cell=(60*i+1,26,58,44)
            if r["role"]=="OFF chip": cell=(60*i+1+28,26+4,29,18)
            if r["role"]=="MUTE chip": cell=(60*i+1+34,26+4,22,18)
        elif r["role"].startswith("knob"):
            i=min(3,ink[0]//60); cell=(60*i+1,144,58,66)
        else: continue
        cx,cy,cw,ch=cell
        lm=ink[0]-cx; rm=cx+cw-ink[2]; tm=ink[1]-cy; bm=cy+ch-ink[3]
        print("| %s | %s | `%s` | %d,%d,%d,%d | %d..%d | %d | %d | %+.1f | %d | %d | %+.1f |"%(sn,r["role"],r["s"],cx,cy,cw,ch,ink[0],ink[2],lm,rm,(ink[0]+ink[2])/2-(cx+cw/2),tm,bm,(ink[1]+ink[3])/2-(cy+ch/2)))
print()
# vertical stack within cells: label/value/meter gaps
sc,w,recs=render(scene("glance_playing"),"gp")
kl=[r for r in recs if r["role"]=="knob label"]; kv=[r for r in recs if r["role"]=="knob value"]
for a,b in zip(kl,kv):
    print("knob cell label ink y %d..%d, value ink y %d..%d -> gap %d px; value bottom to meter top(194) = %d px; meter bottom 204 to cell bottom 210 = 6"%(a['ink'][1],a['ink'][3],b['ink'][1],b['ink'][3],b['ink'][1]-a['ink'][3],194-b['ink'][3]))
cn=[r for r in recs if r["role"]=="chan number"]; an=[r for r in recs if r["role"]=="algo name"]
for a,b in zip(cn,an):
    print("channel cell number ink y %d..%d, algo ink y %d..%d -> gap %d px; algo bottom to cell bottom(70) = %d; number top from cell top(26) = %d"%(a['ink'][1],a['ink'][3],b['ink'][1],b['ink'][3],b['ink'][1]-a['ink'][3],70-b['ink'][3],a['ink'][1]-26))
# header text vs page boxes
for sn in ("glance_playing","stress"):
    sc,w,recs=render(scene(sn),sn)
    h=[r for r in recs if r['role'] in ('bpm','preset')]
    print(sn,[(r['s'],r['ink']) for r in h],"page boxes start at x=172")
# pip meters
for n in (3,6,7,14):
    g=1 if n>8 else 2; pw=(46-(n-1)*g)//n; tot=n*pw+(n-1)*g
    print("pips n=%d gap %d pw %d total %d left margin in 46: %d right: %d"%(n,g,pw,tot,(46-tot)//2,46-tot-(46-tot)//2))
# comb gaps
for n_r in (2,3,4,5,6,8):
    w={2:5,3:3}.get(n_r,2)
    gaps=[round(k*(12-w)/float(n_r-1)) for k in range(n_r)]
    segs=[(g,g+w) for g in gaps]
    print("ratchet x%d: bar width %d, spans (rel to x+1) %s, gaps between bars %s"%(n_r,w,segs,[segs[i+1][0]-segs[i][1] for i in range(n_r-1)]))
