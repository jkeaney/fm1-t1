from common import *
import csv, io
D=os.path.join(UR,"..")
def tsv(n): return list(csv.DictReader(open(os.path.join(D,n),encoding="utf-8"),delimiter="\t"))
BM,KM,KEY,PS=tsv("button_modes.tsv"),tsv("knob_map.tsv"),tsv("key_map.tsv"),tsv("page_states.tsv")
palette_set("HI-CON")
import icons_v9
claims=[]
def C(n,what,v9,tsvv,ok): claims.append((n,what,v9,tsvv,ok)); 
# 1-12 button tap labels
order=["B_FX","B_SCL","B_ENV","B_LFO","B_EDIT","B_GLO","B_HOME","B_SAVE","B_ARP","B_SEQ","B_PLAY","B_REC"]
for (pn,ic,role,hold,dice),b in zip(icons_v9.BUTTONS,order):
    r=[x for x in BM if x["page"]=="PERF" and x["control"]==b and x["mode"]=="tap"][0]
    h=[x for x in BM if x["page"]=="PERF" and x["control"]==b and x["mode"]=="hold"][0]
    C(len(claims)+1,"button %s tap role"%pn,role,r["label"],role==r["label"])
    hl=h["label"] if "no hold role" not in h["action"] else None
    C(len(claims)+1,"button %s hold role"%pn,hold,"%s (%s)"%(h["label"],"no hold role" if hl is None else "hold"),(hold is None and hl is None) or (hold==hl))
# hold hints
sc=mod.STATE["scenes"]
C(len(claims)+1,"hold_edit hint text",sc["hold_edit"]["hold"][1],"EDIT hold 0.4 s: open RESET (label RESET)","RESET" in sc["hold_edit"]["hold"][1])
C(len(claims)+1,"hold_home hint text",sc["hold_home"]["hold"][1],"strip reads 'HOME: touch=HELP  let go=MENU' from 0.25 s",True)
C(len(claims)+1,"hold_edit bar fraction (60%) vs 0.25 s hint / 0.4 s fire","%d%% =%.2f s"%(sc["hold_edit"]["hold"][2],0.4*sc["hold_edit"]["hold"][2]/100),"fires 0.4 s",True)
C(len(claims)+1,"GLO hold hint (HARM) rendered anywhere","no scene, no string in v9.py","hold 0.4 s opens HARMONY",False)
C(len(claims)+1,"SAVE hold hint (UNDO L) rendered anywhere","no scene, no string in v9.py","hold 0.7 s undo last preset load",False)
# black keys
bk={r["key"]:r for r in KEY if r["page"]=="PERF"}
import el
names=["B01","B02","B03","B04","B05","B06","B07","B08","B09","B10","B11"]
for (cx,role),n in zip(el.BLACK,names):
    k=[r for r in KEY if r["page"]=="PERF" and r["key"].startswith("KEY_"+n)]
    labs=[(r["gesture"],r["label"]) for r in k]
    exp=[l for g,l in labs][0]
    C(len(claims)+1,"black key %s glyph role"%n,role,labs,role[0:1]==exp[0:1] and (role==exp or role.startswith("M") and exp.startswith("M") or role in("CHN","SOL","FIL","ROL","PG-","PG+","RST") and role==exp))
# black key x vs key position: (index of white key left of it)*15+15
whites=["F3","G3","A3","B3","C4","D4","E4","F4","G4","A4","B4","C5","D5","E5","F5","G5"]
for (cx,role),n in zip(el.BLACK,names):
    kn=[r["key"] for r in KEY if r["key"].startswith("KEY_"+n)][0].split("_")[2]
    base=kn[0]+kn[2:]; # F#3 -> F3
    idx=whites.index(base); C(len(claims)+1,"black key %s x centre = right edge of white %s"%(n,base),cx,(idx+1)*15,cx==(idx+1)*15)
C(len(claims)+1,"CHN tap/hold","glyph DIN only (tap CHAN; hold RIG not shown)","tap CHAN page (LENGTH MIDI SOURCE FOLLOW); hold RIG",False)
C(len(claims)+1,"B01-B04 hold = PREVIEW (PRV1-4)","not shown","hold 0.4 s PREVIEW on/off",False)
# REC
rk=[r for r in KM if r["page"]=="REC" and r["knob"] in("K1","K2","K3","K4")]
for r,x in zip(rk,mod.REC_ROW):
    C(len(claims)+1,"REC knob %s label"%r["knob"],x["label"],r["label"],x["label"]==r["label"])
C(len(claims)+1,"REC CURSOR range","1..16 (REC_ROW max 16)","1..LENGTH, wraps (LENGTH up to 64)",False)
C(len(claims)+1,"REC OCT range","-2..+2 default 0","-2..+2, default 0",True)
cue=[r for r in PS if r["state"]=="REC"][0]["cue_strip"]
full="REC: PLAY KEYS TO WRITE  REC=done"
C(len(claims)+1,"REC cue strip text","REC: PLAY KEYS TO WRITE (%d px)"%text_w(AF_M,"REC: PLAY KEYS TO WRITE"),"%s (%d px at AF_M) | tsv=%r"%(full,text_w(AF_M,full),cue),False)
C(len(claims)+1,"REC lane write cursor","no rec branch in music(); lane identical to PERF","banner REC + lane with the write cursor",False)
C(len(claims)+1,"REC: PRESET inert toast / REC band 'KEYS = NOTES legend'","not shown","KEYS = NOTES legend",False)
# toasts
C(len(claims)+1,"SCL toast word","NOTES","B_SCL tap label NOTES",True)
C(len(claims)+1,"toast carries algorithm sub-label (AlgoDesc.sub)","no (word + EDIT=UNDO only)","'algorithm sub-label under it' (B_SCL/B_ENV/B_LFO/B_ARP/B_SEQ tap)",False)
C(len(claims)+1,"toast hint EDIT=UNDO","EDIT=UNDO","PERF B_EDIT tap UNDO/REDO last dice",True)
C(len(claims)+1,"MIDI OFF toast","MIDI OFF / CHN=SET","CHN = D#4 tap opens CHAN page (MIDI is K2 there)",True)
C(len(claims)+1,"MIDI OFF value","header OFF chip; None=OFF, 1..16","ChanSave.midi_ch 0..15, 16=OFF (SQ_MIDI_OFF); knob_map CHAN K2 1..16, OFF",True)
C(len(claims)+1,"MIDI OFF bars hollow 'nothing is sounding'","hollow OFF_RED frames","PERF: preview speaker icon never dimmed by mute/MIDI OFF; preview still plays (decision 111)",False)
C(len(claims)+1,"preview speaker icon on channel chip","not drawn","6x6 speaker while PREVIEW ON",False)
# header / page states
C(len(claims)+1,"header tempo source / clock dot / LENGTH","absent","PERF header: clock dot + BPM + tempo source INT/EXT n/EXT --/WAIT, ..., length",False)
C(len(claims)+1,"knob band","4 tiles of live row (row dots 2 or 3)","8-cell knob band (live row filled); ROW 3 exists with a RIG",False)
C(len(claims)+1,"ROW 3 (RIG) knob tiles","status draws 3 dots when S['rig']; knobs() IndexError on row 2","ROW 3 = LEVEL of 4 wires",False)
C(len(claims)+1,"button line 2 hold sub-labels (h:RST h:HARM ...)","not drawn","PERF button picture line 2",False)
# algos
C(len(claims)+1,"ALGO list","ORDER[:6]=%s"%A.ORDER[:6],"ACID EUCLID SNAKE STAGE ARP PATTERNS",A.ORDER[:6]==["ACID","EUCLID","SNAKE","STAGE","ARP","PATTERNS"])
C(len(claims)+1,"PATTERNS header name","PATT (UI_EXTRA short)","AlgoDesc.name[8] 'PATTERNS' (8 chars; comment says <= 6); AlgoDesc has no short field",False)
C(len(claims)+1,"PATTERNS PICK cell text",A.value_text(A.ALGOS["PATTERNS"]["rows"][1][0],81),"cell reads CAT nn, e.g. HOUSE 3",False)
C(len(claims)+1,"STAGE TOTAL cell text",A.value_text(A.ALGOS["STAGE"]["rows"][0][1],48,{"heard":32}),"value reads 48 (32)",False)
C(len(claims)+1,"ARP SPAN cell",A.value_text(A.ALGOS["ARP"]["rows"][0][2],1),"1 OCT..4 OCT",True)
C(len(claims)+1,"DENS 0 reads KEYS",A.value_text(A.ALGOS["ACID"]["rows"][0][0],0),"0 = KEYS ONLY (cell reads KEYS)",True)
C(len(claims)+1,"EUCLID CURVE list",A.ALGOS["EUCLID"]["rows"][1][3]["names"],"AUTO SINE TRI SAW+ SAW- ARCH INV",A.ALGOS["EUCLID"]["rows"][1][3]["names"]==["AUTO","SINE","TRI","SAW+","SAW-","ARCH","INV"])
C(len(claims)+1,"ARP DIR list",A.ALGOS["ARP"]["rows"][1][3]["names"],"UP DOWN UPDN RAND ASPL CHORD",A.ALGOS["ARP"]["rows"][1][3]["names"]==["UP","DOWN","UPDN","RAND","ASPL","CHORD"])
C(len(claims)+1,"'adding an algorithm = AlgoDesc row + TSV rows' (docstring)","short/lane/value_fn from UI_EXTRA (python only)","AlgoDesc fields: name,id,n_seed,flags,knob_morph,morph,layer,sub,fns - no short/lane/format",False)
C(len(claims)+1,"channels max","4","SQ_NCH=4 (CH1..CH4 clamped)",True)
print("n|claim|v9|tsv/firmware|match")
for c in claims: print("%d|%s|%s|%s|%s"%(c[0],c[1],c[2],c[3],"MATCH" if c[4] else "MISMATCH/ABSENT"))
print("matches",sum(1 for c in claims if c[4]),"of",len(claims))

# ---------- 8 coverage
cov=[]
def R(grp,ctl,g,state,note=""): cov.append((grp,ctl,g,state,note))
for i in range(1,17):
    R("key","W%02d"%i,"tap","yes","lane step colours + 'KEYS = STEPS'")
    R("key","W%02d"%i,"hold","yes","'HOLD = EDIT' (text only)")
for n,lab in zip((1,2,3,4),("M1","M2","M3","M4")):
    R("key","B0%d"%n,"tap","yes","coloured square / x glyph")
    R("key","B0%d"%n,"hold","no","PRV preview not shown")
R("key","B05 CHN","tap","yes","DIN glyph")
R("key","B05 CHN","hold","no","RIG not shown")
for n,l in (("B06 SOL","S"),("B07 FIL","F"),("B08 ROL","R")): R("key",n,"hold","yes","letter %s"%l)
R("key","B09 PG-","tap","yes","arrow"); R("key","B10 PG+","tap","yes","arrow (ADD PAGE prompt not shown)"); R("key","B11 RST","tap","yes","glyph")
for b,n in zip(order,("FX","SCL","ENV","LFO","EDIT","GLO","HOME","SAVE","ARP","SEQ","PLAY","REC")):
    pass
for b,hold in (("B_FX","none"),("B_SCL","none"),("B_ENV","none"),("B_LFO","none"),("B_ARP","none"),("B_SEQ","none"),("B_PLAY","none"),("B_REC","none")):
    R("button",b,"tap","yes","icon")
    R("button",b,"hold","n/a","no hold role (acts on press)")
R("button","B_EDIT","tap","yes","undo icon"); R("button","B_EDIT","hold","on press or hold","hold hint scene 'hold_edit'")
R("button","B_HOME","tap","yes","home icon"); R("button","B_HOME","hold","on press or hold","hold hint scene 'hold_home'")
R("button","B_GLO","tap","yes","settings icon"); R("button","B_GLO","hold","no","no hint string/scene")
R("button","B_SAVE","tap","yes","save icon"); R("button","B_SAVE","hold","no","no hint string/scene")
for b in ("B_OCTDN","B_OCTUP"):
    for g in ("tap","hold","hold-both"): R("button",b,g,"no","no OCT buttons on the screen")
for k in ("K1","K2","K3","K4"): R("knob",k,"turn","yes","tile")
R("knob","SELECT","turn","yes","channel chips"); R("knob","ALGO","turn","yes","header name"); R("knob","PRESET","turn","yes","P01 / P02? pending")
for k in ("R1","R2","R3","R4"): R("knob",k,"turn","no","ROW 3 not drawn (IndexError)")
from collections import Counter
cnt=Counter(c[3] for c in cov); tot=len(cov)
print("\ncoverage rows",tot,dict(cnt))
act=[c for c in cov if c[3]!="n/a"]
print("excluding n/a:",len(act),"yes %.0f%% | on press or hold %.0f%% | no %.0f%%"%tuple(100*sum(1 for c in act if c[3]==s)/len(act) for s in ("yes","on press or hold","no")))
print("NO list:"); [print("  ",c[0],c[1],c[2],"-",c[4]) for c in cov if c[3]=="no"]
for g in ("key","button","knob"):
    a=[c for c in cov if c[0]==g and c[3]!="n/a"]; print(g,len(a),"rows; yes+hold:",sum(1 for c in a if c[3]!="no"))
