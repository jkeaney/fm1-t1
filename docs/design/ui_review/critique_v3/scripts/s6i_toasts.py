# SPDX-License-Identifier: GPL-3.0-only (evidence script; run from the repo root with .venv/bin/python)
import sys,os; sys.path.insert(0,"docs/design/ui_review/critique_v3")
from evidence_b_lib import *
mock.palette_set("GREY")
base=lambda: scene("glance_playing")
print("### toasts: ink boxes (screen coords)")
print("| toast | title | title ink x0..x1 (w) | detail | detail ink x0..x1 (w) | box limit | free margin L/R in box |"); print("|---|---|---|---|---|---|---|")
def tt(name,st,box):
    sc,w,recs=render(st,name)
    t=[r for r in recs if r["role"]=="toast title"]; d=[r for r in recs if r["role"]=="toast detail"]
    tr=t[0] if t else None; dr=d[0] if d else None
    bx0,bx1=box
    print("| %s | %s | %d..%d (%d) | %s | %d..%d (%d) | %d..%d | title %d/%d, detail %d/%d |"%(name,tr["s"] if tr else "(drawn ?)",tr["ink"][0],tr["ink"][2],tr["ink"][2]-tr["ink"][0],dr["s"],dr["ink"][0],dr["ink"][2],dr["ink"][2]-dr["ink"][0],bx0,bx1,tr["ink"][0]-bx0 if tr else 0,bx1-tr["ink"][2] if tr else 0,dr["ink"][0]-bx0,bx1-dr["ink"][2]),w)
for a,nm in enumerate(P.ALGO_FULL):
    s=base(); s["channels"][0]["algo"]=a; s["toast"]=dict(kind="algo",algo=a); tt("algo "+nm,s,(0,240))
for layer in ("ALL","NOTES","ACCNT","SHAPE","ARTIC","HITS"):
    s=base(); s["toast"]=dict(kind="dice",layer=layer,detail="CH 1  NEW PITCHES"); tt("dice "+layer,s,(66,226))
for layer in ("ALL","NOTES","ACCNT","SHAPE","ARTIC","HITS"):
    s=base(); s["toast"]=dict(kind="undo",detail="DICE "+layer); tt("undo "+layer,s,(66,226))
# preset toast: title P16 + drawn ?
s=base(); s["preset"]=16;s["preset_pend"]=1; s["toast"]=dict(kind="preset",slot=16,frac=0.3)
sc,w,recs=render(s,"p16t"); 
for r in recs:
    if r["role"].startswith("toast"): print("preset toast P16:",r["role"],r["s"],r["ink"],"x0 of ? glyph",(240-(text_w(AF_L,"P16")+6+15))//2+text_w(AF_L,"P16")+6,"..+15 =",(240-(text_w(AF_L,"P16")+6+15))//2+text_w(AF_L,"P16")+6+15)
# longest string candidates not defined by proposal: measure widths
print("--- candidate strings the proposal does not define (widths only)")
for s_,f in [("P16>","M"),("P16?","M"),("CH 1  NEW PITCHES","M"),("DICE ARTIC","M"),("CH 1  GT+SLD","M"),("CH 1  NEW ACCENTS","M"),("MIDI CH 16","M"),("CH 1  MIDI 16","M"),("MIDI CH 16 TAKEN FROM CH 2","M"),("UNDO","L"),("100","L"),("127","L"),("-2","L"),("+2","L"),("OCT -2","M")]:
    m=mock.measure(s_,f); print("%-28s AF_%s advance %3d ink %3d"%(s_,f,m["advance"],m["ink_w"]))
# glyph existence: '>' in AF_M? '?' in AF_L
for ch,f in [(">",AF_M),("?",AF_M),("?",AF_L),(">",AF_L),("Z",AF_L),(":",AF_L),("-",AF_L),("+",AF_L),("Z",AF_M)]:
    g=f.glyph(ord(ch)); print("glyph %r in %s -> index %s (a missing glyph maps to the fallback)"%(ch,f.name,g))
print("header: P16? ink", mock.measure("P16?","M")["ink_w"], "EXT 126 ink",mock.measure("EXT 126","M")["ink_w"], "EXT 126 x30 ends",30+mock.measure("EXT 126","M")["advance"],"preset starts 110, page boxes 172; P16? at 110 ends",110+mock.measure("P16?","M")["advance"])
print("ink of '?' vs 'P16?' etc")
