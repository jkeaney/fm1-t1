import sys, os, itertools, math
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from common import *
import el
import numpy as np
palette_set("HI-CON")
cr = contrast_ratio
CH = mod.CH; W_ = mod.WHITE
def row(label, a, b):
    r = cr(a, b); print("  %-58s %-8s on %-8s %5.2f %s" % (label, name_of(a), name_of(b), r, "FAIL<3" if r < 3 else ""))
print("NON-TEXT PAIRS (WCAG contrast ratio; flag <3:1)")
for i in range(4):
    row("channel colour CH%d on BG (bars, chip frame, marker)" % (i+1), CH[i], T.BG)
    row("channel colour CH%d on SURF (keybed lane step on key)" % (i+1), CH[i], T.SURF)
    row("channel colour CH%d on PHC (bar over playhead column)" % (i+1), CH[i], mod.PHC)
    row("knob fill mix(CH%d,120) on SURF (empty tile)" % (i+1), el.mix(CH[i],120), T.SURF)
    row("knob fill mix(CH%d,120) on BG" % (i+1), el.mix(CH[i],120), T.BG)
    row("WHITE value text/tick on knob fill CH%d" % (i+1), W_, el.mix(CH[i],120))
    row("INK digit on CH%d selected chip" % (i+1), T.INK, CH[i])
    row("diced hatch CH%d on WHITE fill" % (i+1), CH[i], W_)
row("muted dashed grey (T.DIM) on BG (music-row muted bars)", T.DIM, T.BG)
row("muted dashed grey (T.DIM) on PHC (muted bar on playhead col)", T.DIM, mod.PHC)
row("muted header strike/number T.MID on SURF", T.MID, T.SURF)
row("muted header strike T.MID over T.MID digit (same colour)", T.MID, T.MID)
row("empty-step tick T.DIM on BG", T.DIM, T.BG)
row("empty-step tick T.DIM on PHC", T.DIM, mod.PHC)
row("playhead column PHC on BG", mod.PHC, T.BG)
row("playhead white bar on BG", W_, T.BG)
row("playhead white bar on PHC (adjacent below it? gap rows y-2..y-1)", W_, mod.PHC)
row("playhead column PHC vs SURF", mod.PHC, T.SURF)
row("accent white cap on BG", W_, T.BG)
row("accent white cap on PHC", W_, mod.PHC)
row("dice icon DICE_FG on DICE_BG", mod.DICE_FG, mod.DICE_BG)
row("dice tile DICE_BG on BG", mod.DICE_BG, T.BG)
row("plain tile SURF on BG", T.SURF, T.BG)
row("dice tile DICE_BG vs plain tile SURF", mod.DICE_BG, T.SURF)
row("plain icon TEXT on SURF", T.TEXT, T.SURF)
row("hold notch T.MID on SURF", T.MID, T.SURF)
row("hold notch T.MID on DICE_BG (dice tiles have no notch)", T.MID, mod.DICE_BG)
row("hold notch INK on WHITE (lit button)", T.INK, W_)
row("lit button tile WHITE on BG", W_, T.BG)
row("mute X glyph T.MID on RAISE (black key)", T.MID, T.RAISE)
row("black key RAISE on BG", T.RAISE, T.BG)
row("black key RAISE vs SURF white-key tile", T.RAISE, T.SURF)
row("black key glyph TEXT on RAISE", T.TEXT, T.RAISE)
row("black key RAISE on KEY (rec mode: DIM key on KEY tile)", T.DIM, T.KEY)
row("rec-mode white key KEY on BG", T.KEY, T.BG)
row("row marker active CH1 on BG", CH[0], T.BG)
row("row marker inactive frame T.MID on BG", T.MID, T.BG)
row("turning frame CH on BG", CH[1], T.BG)
row("turning old-value tick WHITE on SURF (above fill)", W_, T.SURF)
row("playing bar under key WHITE on BG", W_, T.BG)
row("header REC indicator T.REC on BG", T.REC, T.BG)
row("header stop icon T.MID on BG", T.MID, T.BG)
row("quarter tick marks T.MID on BG (y=125..127)", T.MID, T.BG)
row("pending-preset frame WHITE on BG", W_, T.BG)
row("toast WHITE on BG (edge)", W_, T.BG)
row("toast icon INK on WHITE", T.INK, W_)
print("\nPAIRWISE CHANNEL COLOURS, normal vision (luminance ratio / CIEDE2000)")
# colour science
def srgb(c): return np.array(to_rgb888(c), float)/255
def lin_(v): return np.where(v<=0.04045, v/12.92, ((v+0.055)/1.055)**2.4)
def gam(v): v=np.clip(v,0,1); return np.where(v<=0.0031308, v*12.92, 1.055*v**(1/2.4)-0.055)
MACH = {  # Machado, Oliveira, Fernandes 2009, severity 1.0, applied in linear RGB
 "deuteranopia": np.array([[0.367322,0.860646,-0.227968],[0.280085,0.672501,0.047413],[-0.011820,0.042940,0.968881]]),
 "protanopia":   np.array([[0.152286,1.052583,-0.204868],[0.114503,0.786281,0.099216],[-0.003882,-0.048116,1.051998]])}
VIEN = {  # Vienot/Brettel/Mollon 1999 (linear sRGB)
 "deuteranopia": np.array([[0.29275,0.70725,0.0],[0.29275,0.70725,0.0],[-0.02234,0.02234,1.0]]),
 "protanopia":   np.array([[0.10889,0.89111,0.0],[0.10889,0.89111,0.0],[0.00447,-0.00447,1.0]])}
def sim(c, M): return np.clip(M @ lin_(srgb(c)), 0, 1)
def lum(l): return 0.2126*l[0]+0.7152*l[1]+0.0722*l[2]
def lab(l):
    X = np.array([[0.4124564,0.3575761,0.1804375],[0.2126729,0.7151522,0.0721750],[0.0193339,0.1191920,0.9503041]]) @ l
    wp = np.array([0.95047,1.0,1.08883]); t = X/wp
    f = np.where(t>216/24389, np.cbrt(t), (24389/27*t+16)/116)
    return np.array([116*f[1]-16, 500*(f[0]-f[1]), 200*(f[1]-f[2])])
def de2000(a, b):
    L1,a1,b1=a; L2,a2,b2=b
    C1=math.hypot(a1,b1); C2=math.hypot(a2,b2); Cb=(C1+C2)/2
    G=0.5*(1-math.sqrt(Cb**7/(Cb**7+25**7)))
    a1p=(1+G)*a1; a2p=(1+G)*a2
    C1p=math.hypot(a1p,b1); C2p=math.hypot(a2p,b2)
    h1=math.degrees(math.atan2(b1,a1p))%360; h2=math.degrees(math.atan2(b2,a2p))%360
    dL=L2-L1; dC=C2p-C1p
    if C1p*C2p==0: dh=0
    else:
        dh=h2-h1
        if dh>180: dh-=360
        elif dh<-180: dh+=360
    dH=2*math.sqrt(C1p*C2p)*math.sin(math.radians(dh/2))
    Lb=(L1+L2)/2; Cbp=(C1p+C2p)/2
    if C1p*C2p==0: hb=h1+h2
    elif abs(h1-h2)<=180: hb=(h1+h2)/2
    else: hb=(h1+h2+360)/2 if h1+h2<360 else (h1+h2-360)/2
    T_=1-0.17*math.cos(math.radians(hb-30))+0.24*math.cos(math.radians(2*hb))+0.32*math.cos(math.radians(3*hb+6))-0.20*math.cos(math.radians(4*hb-63))
    dth=30*math.exp(-((hb-275)/25)**2)
    Rc=2*math.sqrt(Cbp**7/(Cbp**7+25**7))
    Sl=1+0.015*(Lb-50)**2/math.sqrt(20+(Lb-50)**2); Sc=1+0.045*Cbp; Sh=1+0.015*Cbp*T_
    Rt=-math.sin(math.radians(2*dth))*Rc
    return math.sqrt((dL/Sl)**2+(dC/Sc)**2+(dH/Sh)**2+Rt*(dC/Sc)*(dH/Sh))
def lab_n(c): return lab(lin_(srgb(c)))
for (i,j) in itertools.combinations(range(4),2):
    la,lb = lum(lin_(srgb(CH[i]))), lum(lin_(srgb(CH[j])))
    print("  CH%d-CH%d  Lratio %.2f  dE2000 %.1f  dE76 %.1f" % (i+1,j+1,(max(la,lb)+.05)/(min(la,lb)+.05), de2000(lab_n(CH[i]),lab_n(CH[j])), np.linalg.norm(lab_n(CH[i])-lab_n(CH[j]))))
for meth, M in (("Machado2009 sev1.0", MACH), ("Vienot1999", VIEN)):
    for kind in ("deuteranopia","protanopia"):
        print("\n%s %s: simulated sRGB hex and pairwise (luminance ratio / dE2000 / dE76)" % (kind, meth))
        sims = [sim(c, M[kind]) for c in CH]
        print("  simulated:", ["#%02x%02x%02x" % tuple(int(round(x*255)) for x in gam(s)) for s in sims])
        mn = (99,99)
        for (i,j) in itertools.combinations(range(4),2):
            lr = (max(lum(sims[i]),lum(sims[j]))+.05)/(min(lum(sims[i]),lum(sims[j]))+.05)
            la, lb = lab(sims[i]), lab(sims[j])
            d00 = de2000(la,lb)
            print("  CH%d-CH%d  Lratio %.2f  dE2000 %.1f  dE76 %.1f" % (i+1,j+1,lr,d00,np.linalg.norm(la-lb)))
        # also vs BG and SURF for each simulated channel colour
        print("  sim contrast on BG:", ["%.2f" % ((lum(s)+.05)/.05) for s in sims])
