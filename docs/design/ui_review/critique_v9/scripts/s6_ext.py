from common import *
import io, contextlib
palette_set("HI-CON")
K=lambda label,mn,mx,names=None,fmt=None,zero=None,d=None: dict(label=label,min=mn,max=mx,names=names,fmt=fmt,zero=zero,**{"def":mn if d is None else d})
long12=["ALPHA","BRAVO","CHARLI","DELTA","ECHO","FOXTRT","GOLF","HOTEL","INDIA","JULIET","KILO","LIMA"]
A.register("LONGALGO",[
 [K("LONGLABEL",0,16), K("TEMPO",0,9999), K("MODE",0,11,names=long12), K("STEPS",0,100,fmt="%d STEPS")],
 [K("X",0,19,names=["N%d"%i for i in range(20)]), K("WIDE",0,2,names=["WIDEWIDE","MMMMMMM","iiiiiii"]), K("NEG",-100,100), K("POS",0,3,names=["a","b","c","d"])]],
 short="LONGALGO")
print("A.check() with LONGALGO:"); [print("  ",p) for p in A.check(text_w,AF_M,AF_L)]
buf=io.StringIO()
for r in (0,1):
    for k in range(4):
        kd=A.ALGOS["LONGALGO"]["rows"][r][k]
        f=A.value_font(kd,text_w,AF_M,AF_L)
        widest=A.widest_value(kd,text_w,f)
        dots=""
        if A.is_list(kd):
            n=kd["max"]-kd["min"]+1; dw=min(8,(50-(n-1)*2)//n); dots=" n=%d dot_w=%d total=%d"%(n,dw,n*dw+(n-1)*2)
        print("r%d K%d %-9s font=%s widest %r w=%d (tile58/54)%s"%(r+1,k+1,kd["label"],f.name,widest,text_w(f,widest),dots))
s=scene("playing"); s["chans"][0]["algo"]="LONGALGO"
for r in (0,1):
  for midi in (1,16,None):
    s["row"]=r; s["chans"][0]["midi"]=midi
    with contextlib.redirect_stdout(buf): sc,log=render(s)
    cuts=[t for t in sc.texts if t[5]&2 or t[2]>240]
    nm=[t for t in sc.texts if t[4]=="LONGALGO"][0]
    ends=max(t[2] for t in sc.texts if t[1]<20)
    print("row",r+1,"midi",midi,"header name box",nm[:4],"rightmost header ink x",ends,"cut",cuts)
print("stdout warnings:",[l for l in buf.getvalue().splitlines()][:10])
# overlapping label/value boxes within tiles
sc,log=render(dict(s,row=0,vals={0:16,1:9999,2:11,3:100}))
for t in sc.texts:
    if 24<=t[1]<80 and (t[0]<0 or t[2]>240): print("tile text off-screen",t)
tiles=[(k*60+1,k*60+59) for k in range(4)]
for t in sc.texts:
    if 24<=t[1]<80:
        k=min(3,t[0]//60)
        if t[0]<tiles[k][0] or t[2]>tiles[k][1]: print("tile text overflows its 58px tile:",t[4],t[:4],"tile",tiles[k])
# 3-row (RIG) and missing-slot
for lab,st in (("rig row index 2",dict(s,row=2,rig=True)),("empty lane",dict(s,chans=[dict(s["chans"][0],lane=[None]*16)]+s["chans"][1:])),("lane len 8",dict(s,chans=[dict(s["chans"][0],lane=s["chans"][0]["lane"][:8])]+s["chans"][1:])),("5 channels used",dict(s,used=[0,1,2,3,4])),("pages=5",dict(s,pages=5,vpage=5)),("pages=17 dots",dict(s,pages=17))):
    try:
        with contextlib.redirect_stdout(buf): sc,_=render(st)
        xs=[ (238-(st.get('pages',1)-k)*11) for k in range(st.get('pages',1))]
        print(lab,"-> rendered; page-dot leftmost x",min(xs) if xs else None)
    except Exception as e: print(lab,"-> EXC",type(e).__name__,e)
print("widest STAGE TOTAL with heard:",max((A.ALGOS['STAGE']['rows'][0][1]['fn'](64,{'heard':w}),text_w(AF_M,A.ALGOS['STAGE']['rows'][0][1]['fn'](64,{'heard':w}))) for w in (1,32,63)))
