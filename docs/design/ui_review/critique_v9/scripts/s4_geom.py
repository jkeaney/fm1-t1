from common import *
palette_set("HI-CON")
BG=T.BG
# empirical bands from 'playing' and 'one_channel'
for n in ["playing","one_channel","two_channels"]:
    sc,log=render(scene(n)); px=sc.px
    occ=[any(px[y*240+x]!=BG for x in range(240)) for y in range(240)]
    runs=[];s=None
    for y,o in enumerate(occ+[False]):
        if o and s is None: s=y
        if not o and s is not None: runs.append((s,y-1)); s=None
    print(n,"non-BG row runs:",runs, "gaps:",[runs[i+1][0]-runs[i][1]-1 for i in range(len(runs)-1)])
# text boxes in all scenes crossing y=120 or leaving screen
print("\n## text crossing y=120 / touching screen edge, all scenes")
for n in mod.STATE["scenes"]:
    sc,log=render(scene(n))
    for t in sc.texts:
        x0,y0,x1,y1,s,f=t
        if y0<120<y1: print(n,"CROSS120",t)
        if x0<=0 or x1>=240 or y1>=240 or y0<=0: print(n,"EDGE",t)
        if f&2: print(n,"CUT",t)
# music geometry per channel count
print("\n## music geometry per channel count (code constants: y0=94,y1=127,strip_h=2,gap 1)")
for n in (1,2,3,4):
    oth=n-1; sh=oth*3; bhmax=(127-94)-sh-1
    strips=[(94+bhmax+2+k*3, 94+bhmax+2+k*3+1) for k in range(oth)]
    print("channels",n,"bar area h",bhmax,"bar y",94,"..",94+bhmax-1,"strips y",strips,"strip last row",(strips[-1][1] if strips else None),"playhead col y 92..126; strips cross y=120:",[s for s in strips if s[0]<=120<=s[1]])
    if n==1: print("  min bar h 6 single=8; pitched h=6+(bhmax-9)*frac -> range",6,"..",6+bhmax-9)
    else: print("  pitched bar range",6,"..",6+bhmax-9,"; single 8; accent cap 4px above bar (y by-4..by-3); strips 13x2 hit /5x2 dash")
# headline overlap sweep
print("\n## header sweep")
sc,cv=None,None
algs=list(A.ALGOS)
bad=0;rows=[]
chip_right=46+3*14+13
for a in algs:
    s=A.ALGOS[a]["short"]
    xe=105+text_w(AF_M,s)
    for m in list(range(1,17))+["OFF"]:
        if m=="OFF":
            w=text_w(AF_S,"MIDI")+text_w(AF_M,"OFF")+10
            chip_x=xe+4; end=chip_x+w; 
            xx=chip_x+3+text_w(AF_S,"MIDI"); txtend=xx+3+text_w(AF_M,"OFF")
            ok=end<=240
            rows.append((a,s,m,xe,end,txtend))
        else:
            xx=xe+5+text_w(AF_S,"MIDI"); end=xx+2+text_w(AF_M,str(m))
            rows.append((a,s,m,xe,end,end))
    mx=max(r[4] for r in rows if r[0]==a)
    print("%-9s short=%-6s nameEnd=%d  max right edge over MIDI1..16/OFF=%d  (>=240 off-screen? %s)  gap chips->name=%d"%(a,s,xe,mx,mx>240,105-chip_right))
# actual render check for worst-case: render each algo x midi values and collect warnings/cut/overlap with 240
print("\n## header render sweep: cut/warn")
cnt=0;warn=0
for a in algs:
    for m in list(range(1,17))+[None]:
        s=scene("playing"); s["chans"][0]["algo"]=a; s["chans"][0]["midi"]=m
        sc,log=render(s); cnt+=1
        for t in sc.texts:
            if t[5]&2 or t[2]>240: print("  CUT",a,m,t); warn+=1
print("renders",cnt,"cut/overflow",warn)
# bpm
print("bpm text widths AF_M:",{b:text_w(AF_M,str(b)) for b in (9,10,99,100,120,300)}," bpm end x for 3 digits =",16+text_w(AF_M,"300")," chip1 x=46")
print("DEMO/ACID header chip right",chip_right,"name x=105")
