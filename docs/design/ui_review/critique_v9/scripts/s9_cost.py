from common import *
import el, math
palette_set("HI-CON")
USED={}
_a=el.P.art
def art(self,x,y,a,c):
    USED.setdefault(a,0); USED[a]+=1; return _a(self,x,y,a,c)
el.P.art=art
for n in mod.STATE["scenes"]:
    if n.startswith("sheet"): continue
    render(scene(n))
import icons_v9
rev={}
for k,v in list(icons_v9.ICONS.items())+list(el.IC.items()): rev.setdefault(v,k)
def dims(a):
    rows=a.strip("\n").split("\n"); return max(map(len,rows)),len(rows)
tot1=totr=0
print("icons drawn in the 20 scenes (unique bitmaps):")
for a in sorted(USED,key=lambda a:rev.get(a,"?")):
    w,h=dims(a); b1=math.ceil(w*h/8); br=h*math.ceil(w/8)
    tot1+=b1; totr+=br
    print("  %-12s %2dx%-2d  packed %3d B  row-aligned %3d B"%(rev.get(a,"?"),w,h,b1,br))
print("count",len(USED),"total packed",tot1,"B, row-aligned",totr,"B")
# potential: every icon the code references
ref=set(i[1] for i in icons_v9.BUTTONS)|{"rec","play_s","stop_s","x7","midi_s","din","dice_all","dice_notes","undo","home"}
print("code-referenced ICONS keys:",len(ref),sorted(ref))
print("el.IC referenced: left right rst")
print("ICONS dict total entries (v7+v9):",len(icons_v9.ICONS))
# colours
import re
print("\nHI-CON tokens:")
toks={t:hexc(getattr(T,t)) for t in [x.upper().rstrip("_") for x in mock.Theme.TOKENS]}
print(toks)
pal=set(getattr(T,t) for t in [x.upper().rstrip("_") for x in mock.Theme.TOKENS])
cols={"CH1":mod.CH[0],"CH2":mod.CH[1],"CH3":mod.CH[2],"CH4":mod.CH[3],"WHITE":mod.WHITE,"INK":mod.INK,"PHC":mod.PHC,"DICE_BG":mod.DICE_BG,"OFF_RED":mod.OFF_RED}
for k,v in cols.items(): print(k,hexc(v),"in HI-CON tokens" if v in pal else "NOT a HI-CON token")
for i,c in enumerate(mod.CH): print("fill mix120 ch%d"%(i+1),hexc(el.mix(c,120)),"derived (not a token)")
# unique pixel colours per scene minus text AA
for n in ("playing","dice","hold_edit","rec_armed","midi_off_after"):
    sc,_=render(scene(n)); u=set(sc.px); print(n,"unique RGB565 colours on screen",len(u),"; not tokens:",len(u-pal))
    ex=[c for c in u-pal]
sc,_=render(scene("playing")); 
# colours not tokens and not AA ramps: print the list for playing
print("playing non-token colours:",sorted(hexc(c) for c in set(sc.px)-pal)[:60])
