from common import *
import el
palette_set("HI-CON")
print("tokens:", {t: hexc(getattr(T,t)) for t in ["BG","SURF","TEXT","MID","DIM","REC","RAISE","KEY"]})
cr=contrast_ratio
CH=mod.CH; W=mod.WHITE; INK=mod.INK
names=["CH1 amber","CH2 sky","CH3 yellow","CH4 pink"]
print("\n## channel colours: hex, contrast vs BG, vs SURF, vs dice tile")
for i,c in enumerate(CH): print(names[i],hexc(c),"BG %.2f SURF %.2f DICE %.2f RAISE %.2f"%(cr(c,T.BG),cr(c,T.SURF),cr(c,mod.DICE_BG),cr(c,T.RAISE)))
fc=lambda c: el.mix(c,120)
def row(label,a,b,thr=3.0):
    r=cr(a,b); print("%-62s %s on %s  %.2f %s"%(label,hexc(a),hexc(b),r,"FLAG<%.1f"%thr if r<thr else ""))
print("\n## non-text meaning-bearing pairs (flag <3)")
for i in range(4):
    row("ch%d hit strip (13x2) on BG"%(i+1),CH[i],T.BG)
    row("ch%d knob fill mix120 vs SURF (empty)"%(i+1),fc(CH[i]),T.SURF)
    row("ch%d knob fill mix120 vs BG"%(i+1),fc(CH[i]),T.BG)
    row("ch%d selected chip fill on BG"%(i+1),CH[i],T.BG)
    row("ch%d position dot ON vs SURF"%(i+1),CH[i],T.SURF)
row("white pos dot ON (REC) vs SURF",W,T.SURF)
row("position dot OFF (DIM frame) vs SURF",T.DIM,T.SURF)
row("muted/silent hit strip dash DIM on BG",T.DIM,T.BG)
row("muted hollow bar frame MID on BG",T.MID,T.BG)
row("MIDI OFF hollow bar frame OFF_RED on BG",mod.OFF_RED,T.BG)
row("OFF_RED cap on chip over ch2 sky (chip selected fill)",mod.OFF_RED,CH[1])
row("OFF_RED cap on chip over BG (unselected frame)",mod.OFF_RED,T.BG)
row("OFF_RED cap over CH1 amber",mod.OFF_RED,CH[0])
row("OFF_RED chip on BG",mod.OFF_RED,T.BG)
row("white MIDI/OFF text on OFF_RED (text, 4.5 threshold)",W,mod.OFF_RED,4.5)
row("playhead column PHC on BG",mod.PHC,T.BG)
row("playhead white bar on BG",W,T.BG)
row("playhead white bar on PHC",W,mod.PHC)
for i in range(4): row("ch%d bar on PHC (playhead col)"%(i+1),CH[i],mod.PHC)
row("white accent cap on BG",W,T.BG)
row("white accent cap on PHC",W,mod.PHC)
row("dice tile DICE_BG vs plain tile SURF",mod.DICE_BG,T.SURF)
row("dice tile DICE_BG vs BG",mod.DICE_BG,T.BG)
row("plain tile SURF vs BG",T.SURF,T.BG)
row("tile (SURF) vs BG knob tiles",T.SURF,T.BG)
row("dice icon WHITE on DICE_BG",W,mod.DICE_BG)
row("absent die icon DIM on DICE_BG",T.DIM,mod.DICE_BG)
row("plain icon TEXT on SURF",T.TEXT,T.SURF)
row("row marker active (ch1) vs BG",CH[0],T.BG)
row("row marker inactive MID frame vs BG",T.MID,T.BG)
row("page dot active TEXT vs BG",T.TEXT,T.BG)
row("page dot inactive MID vs BG",T.MID,T.BG)
row("selection bar WHITE under chip vs BG",W,T.BG)
row("muted chip SURF vs BG",T.SURF,T.BG)
row("muted diagonal MID on SURF",T.MID,T.SURF)
row("muted digit DIM on SURF (text)",T.DIM,T.SURF,4.5)
row("unselected chip frame ch4 on BG",CH[3],T.BG)
row("selected chip ink digit INK on ch1 (text)",INK,CH[0],4.5)
row("REC dot T.REC on BG",T.REC,T.BG)
row("hold hint progress bar INK on TEXT-bg",INK,T.TEXT)
row("hold hint progress bar OFF_RED(100%) on TEXT-bg",mod.OFF_RED,T.TEXT)
row("hatch (dice-diced) CH on WHITE ch1",CH[0],W)
row("hatch ch2 on WHITE",CH[1],W)
row("hatch ch3 yellow on WHITE",CH[2],W)
row("hatch ch4 on WHITE",CH[3],W)
row("diced bar (hatched WHITE) vs BG",W,T.BG)
row("black key RAISE vs BG gap",T.RAISE,T.BG)
row("black key glyph ch colour vs RAISE: ch1",CH[0],T.RAISE)
row("black key glyph x7 MID on RAISE",T.MID,T.RAISE)
row("white key SURF (lane step) ",T.SURF,T.BG)
row("keybed step marker ch1 on SURF",CH[0],T.SURF)
row("rec keys T.KEY vs BG",T.KEY,T.BG)
row("keybed playhead WHITE bar vs BG",W,T.BG)
row("knob value-fill ch3 yellow-mix vs WHITE value text (text 4.5)",fc(CH[2]),W,4.5)
for i in range(4): row("value text WHITE on fill ch%d mix120 (text)"%(i+1),W,fc(CH[i]),4.5)
row("empty-lane rest tick DIM on BG",T.DIM,T.BG)
row("turning frame ch on SURF/BG: ch1",CH[0],T.BG)
row("turning white pegs vs fill ch3",W,fc(CH[2]))
print("\n## deuteranopia / protanopia simulation of 4 channel colours")
import numpy as np
M_lms=np.array([[0.31399022,0.63951294,0.04649755],[0.15537241,0.75789446,0.08670142],[0.01775239,0.10944209,0.87256922]]) # linRGB->LMS (Smith/Pokorny via Vienot? use Machado-free Vienot 1999 approach)
# Vienot 1999: simulate in LMS
RGB2LMS=np.array([[17.8824,43.5161,4.11935],[3.45565,27.1554,3.86714],[0.0299566,0.184309,1.46709]])
LMS2RGB=np.linalg.inv(RGB2LMS)
SIM={"prot":np.array([[0,2.02344,-2.52581],[0,1,0],[0,0,1]]),"deut":np.array([[1,0,0],[0.494207,0,1.24827],[0,0,1]])}
def lin2(c): return np.array([lin(v) for v in to_rgb888(c)])
def srgb(v): v=np.clip(v,0,1); return np.where(v<=0.0031308,12.92*v,1.055*v**(1/2.4)-0.055)
def sim(c,kind):
    l=lin2(c); out=LMS2RGB@SIM[kind]@RGB2LMS@l; return srgb(out)*255
def lab(rgb):
    v=np.array([lin(x) for x in rgb]) if True else None
    X=np.array([[0.4124,0.3576,0.1805],[0.2126,0.7152,0.0722],[0.0193,0.1192,0.9505]])@v
    X=X/np.array([0.95047,1,1.08883])
    f=np.where(X>0.008856,np.cbrt(X),7.787*X+16/116)
    return np.array([116*f[1]-16,500*(f[0]-f[1]),200*(f[1]-f[2])])
def dE(a,b): return float(np.linalg.norm(lab(a)-lab(b)))
for kind in ("none","deut","prot"):
    cols=[np.array(to_rgb888(c),float) if kind=="none" else sim(c,kind) for c in CH]
    print(kind,[ "#%02x%02x%02x"%tuple(int(round(x)) for x in c) for c in cols])
    print("  pairwise dE76 (CIELAB):"," ".join("%d-%d:%.1f"%(i+1,j+1,dE(cols[i],cols[j])) for i in range(4) for j in range(i+1,4)))
    print("  luminance contrast pairwise:"," ".join("%d-%d:%.2f"%(i+1,j+1,(max(sum(np.array([lin(x) for x in cols[i]])*[.2126,.7152,.0722]),0)+.05)/(max(sum(np.array([lin(x) for x in cols[j]])*[.2126,.7152,.0722]),0)+.05)) if False else 0 for i in range(0)))
# luminance
def Y(rgb): return float(sum(np.array([lin(max(0,min(255,x))) for x in rgb])*[.2126,.7152,.0722]))
for kind in ("none","deut","prot"):
    cols=[np.array(to_rgb888(c),float) if kind=="none" else sim(c,kind) for c in CH]
    ys=[Y(c) for c in cols]
    print(kind,"relative lum:",["%.3f"%y for y in ys]," pairwise lum-contrast:"," ".join("%d-%d:%.2f"%(i+1,j+1,(max(ys[i],ys[j])+.05)/(min(ys[i],ys[j])+.05)) for i in range(4) for j in range(i+1,4)))
