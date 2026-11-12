# SPDX-License-Identifier: GPL-3.0-only (evidence script; run from the repo root with .venv/bin/python)
import sys,os,re,csv; sys.path.insert(0,"docs/design/ui_review/critique_v3")
from evidence_b_lib import *
mock.palette_set("GREY")
ALG=("ACID","EUCLID","SNAKE","STAGE","ARP","PATTERNS")
# tsv ranges
tsv={}
for r in csv.reader(open("docs/design/knob_map.tsv"),delimiter="\t"):
    if r[0]=="PERF" and r[1] in ALG:
        k=("K1","K2","K3","K4","S1","S2","S3","S4").index(r[2]); m=re.search(r"(\d+)\.\.(\d+)",r[5])
        tsv[(ALG.index(r[1]),k)]=(r[3],int(m.group(1)),int(m.group(2))) if m else (r[3],None,None)
print("### proposal KNOBS vs knob_map.tsv (label, lo, hi)")
bad=0
for a in range(6):
    for k in range(8):
        lab,lo,hi,en=P.KNOBS[a][k]; t=tsv[(a,k)]
        if t[1] is None:
            exp="(enum / list in tsv: %s)"%"n/a"
            # enums: names count must be hi+1
            names=P.ENUMS.get(en) if en else None
            if names is not None and en not in ("KEYS","PICK") and len(names)!=hi+1: print("ENUM COUNT MISMATCH",a,k,en,len(names),hi); bad+=1
            continue
        if (t[0],t[1],t[2])!=(lab,lo,hi) and not (a==4 and k==2 and lab=="OCTS") :
            # SNAKE PATH tsv 1..14 vs proposal 0..13 (index)
            print("DIFF",ALG[a],k,"tsv",t,"proposal",(lab,lo,hi)); bad+=1
print("diffs:",bad)
# label ink table
def ink(f,s): b=text_ink(f,s); return b[2]-b[0]
rows=[]
print()
print("### (e) knob cell sweep: min / max / longest-name value, per algorithm and knob (60 px cell, 58 px drawable, AF_L ink limit 56)")
print("| algo | K | label (AF_M ink) | min -> text (font, ink) | max -> text (font, ink) | longest value -> text (font, ink) | verdict |"); print("|---|---|---|---|---|---|---|")
problems=[]
for a in range(6):
    for k in range(8):
        lab,lo,hi,en=P.KNOBS[a][k]
        vals=[lo,hi]
        cand=range(lo,hi+1)
        def vw(v):
            l,t,f,m=P.knob_view(a,k,v); return ink(f,t)+(1000 if False else 0), l
        # longest by (ink, label ink) among all values
        best=max(cand,key=lambda v:(ink(P.knob_view(a,k,v)[2],P.knob_view(a,k,v)[1]),ink(AF_M,P.knob_view(a,k,v)[0])))
        out=[]; flags=[]
        for v in (lo,hi,best):
            l,t,f,m=P.knob_view(a,k,v); w=ink(f,t); lw=ink(AF_M,l)
            out.append((v,l,t,f.name,w,lw))
            if f is AF_L and w>56: flags.append("value %s AF_L ink %d > 56"%(t,w))
            if f is AF_M and w>58: flags.append("value %s AF_M ink %d > 58"%(t,w))
            if lw>58: flags.append("label %s ink %d > 58"%(l,lw))
            elif lw>=56: flags.append("label %s ink %d (>=56)"%(l,lw))
            if f is AF_M: flags.append("drops to AF_M")
        # distinct set of all labels (PICK varies)
        labs=sorted({P.knob_view(a,k,v)[0] for v in cand},key=lambda s:-ink(AF_M,s))
        lmax=ink(AF_M,labs[0])
        fl=sorted(set(flags))
        desc=lambda o:"%s -> `%s` (AF_%s, %d)"%(o[0],o[2],o[3],o[4])
        print("| %s | %s | %s (%d) | %s | %s | %s | %s |"%(ALG[a],"K%d"%(k+1) if k<4 else "S%d"%(k-3),",".join(labs[:3]) if len(labs)>1 else labs[0],lmax,desc(out[0]),desc(out[1]),desc(out[2]),"; ".join(fl) or "fits"))
# enum font table
print()
print("### enum font choice (one font per enum)")
for en,names in P.ENUMS.items():
    if not names: continue
    inL=P.enum_in_L(en)
    ws=[(ink(AF_L,ln) if ln else None,ink(AF_M,mn)) for ln,mn in names]
    print(en,"all in AF_L" if inL else "AF_M (fallback)", "AF_L inks",[w[0] for w in ws],"AF_M inks",[w[1] for w in ws])
n_L=sum(len(P.ENUMS[e]) for e in P.ENUMS if P.ENUMS[e] and P.enum_in_L(e)); n_M=sum(len(P.ENUMS[e]) for e in P.ENUMS if P.ENUMS[e] and not P.enum_in_L(e))
print("enum names in AF_L:",n_L,"in AF_M:",n_M,"total",n_L+n_M)
# AF_L values: widest digits
print("widest AF_L numeric value strings:")
for s in ("0","8","10","16","20","24","36","47","48","63","64","100","127","-2","+2"): print(s,ink(AF_L,s),end=" | ")
print()
# numeric maxima in the proposal
mx=max(P.KNOBS[a][k][2] for a in range(6) for k in range(8) if P.KNOBS[a][k][3] not in ("KEYS","PICK") or True)
print("largest hi of any knob:",mx,"; any lo<0:",any(P.KNOBS[a][k][1]<0 for a in range(6) for k in range(8)))
# the pick category label + number
print("PICK labels ink AF_M:",{c:ink(AF_M,c) for c in P.PICK_CAT},"numbers 1..20 max AF_L ink",max(ink(AF_L,str(n)) for n in range(1,21)))
print("KEY (v=0 on DENS/HITS): AF_L ink",ink(AF_L,"KEY"),"; firmware KEYS",ink(AF_L,"KEYS"))
