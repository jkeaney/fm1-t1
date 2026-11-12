from common import *
import io, contextlib
palette_set("HI-CON")
print("A.check():", A.check(text_w, AF_M, AF_L))
print("ORDER:",A.ORDER)
print("\n## knob sweep: algo row knob label font[min..max widths] fits(54 / 58) font-stable dots")
issues=[]; nvals=0
for a in A.ORDER:
    for r,row in enumerate(A.ALGOS[a]["rows"]):
        for k,kd in enumerate(row):
            fs=set(); maxw=0; worst=None; nums=[]
            f0=A.value_font(kd,text_w,AF_M,AF_L)
            for v in range(kd["min"],kd["max"]+1):
                nvals+=1
                val=A.value_text(kd,v)
                f=AF_M if (v==0 and kd["zero"]) else f0
                fs.add(f.name)
                w=text_w(f,val)
                if w>maxw: maxw,worst=w,val
                if w>54: issues.append("%s r%d K%d %s v=%d %r w=%d>54 font %s"%(a,r+1,k+1,kd["label"],v,val,w,f.name))
            # also STAGE total with heard
            lw=text_w(AF_M,kd["label"])
            ex=""
            if A.is_list(kd):
                n=kd["max"]-kd["min"]+1; dw=min(8,(50-(n-1)*2)//n); tot=n*dw+(n-1)*2
                ex=" LIST n=%d(names %d) dot_w=%d total=%d"%(n,len(kd["names"]),dw,tot)
                if dw<2: issues.append("%s r%d K%d dots dw=%d"%(a,r+1,k+1,dw))
            print("%-9s r%d K%d %-7s label_w=%2d font=%s range %s..%s maxw=%d (%r) fonts_used=%s%s"%(a,r+1,k+1,kd["label"],lw,f0.name,kd["min"],kd["max"],maxw,worst,sorted(fs),ex))
            if lw>54: issues.append("%s label %s w=%d>54"%(a,kd["label"],lw))
print("values swept",nvals); print("ISSUES:",*issues,sep="\n")
# render every algo/row/value (full render) and catch exceptions and warnings
exc=0;warn=0;renders=0;cutlist=[]
buf=io.StringIO()
for a in A.ORDER:
    for r in (0,1):
        for k in range(4):
            kd=A.ALGOS[a]["rows"][r][k]
            for v in range(kd["min"],kd["max"]+1):
                s=scene("playing"); s["chans"][0]["algo"]=a; s["row"]=r; s["vals"]={k:v}; s["turning"]=(k,50)
                if a=="STAGE": s["algo_state"]={"heard":max(kd["min"],v-1)}
                try:
                    with contextlib.redirect_stdout(buf): sc,log=render(s)
                except Exception as e:
                    exc+=1; print("EXC",a,r,k,v,repr(e)); continue
                renders+=1
                for t in sc.texts:
                    if t[5]&2: warn+=1; cutlist.append((a,r,k,v,t))
print("full renders",renders,"exceptions",exc,"cut texts",warn,cutlist[:5])
print("captured stdout warnings:",[l for l in buf.getvalue().splitlines() if 'warning' in l][:10], "n=",buf.getvalue().count("warning"))
