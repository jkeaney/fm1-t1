# SPDX-License-Identifier: GPL-3.0-only (evidence script; run from the repo root with .venv/bin/python)
import sys; sys.path.insert(0,"docs/design/ui_review/critique_v3")
from evidence_b_lib import *
mock.palette_set("GREY")
tok={n:getattr(T,n) for n in "BG SURF TEXT THEME ACCENT MID DIM LINE SEL TINT INK REC RAISE KEY LANE GRID".split()}
print("GREY tokens:", {k:"0x%04X #%02x%02x%02x"%((v,)+tuple(to_rgb888(v))) for k,v in tok.items()})
PH,AC,RD=P.T_PLAYHEAD,P.T_ACCSTEP,P.T_RECRED
def C(n): return {"PH":PH,"AC":AC,"RD":RD}.get(n) or tok[n]
pairs=[
# (what, fg, bg, need)
("TEXT: bpm/preset/cue AF_M","TEXT","BG",4.5),
("TEXT: unselected channel number+algo AF_L/M on RAISE","TEXT","RAISE",4.5),
("TEXT: selected channel number+algo on SEL (INK on SEL)","INK","SEL",4.5),
("TEXT: muted channel number/algo/OFF text MID on BG","MID","BG",4.5),
("TEXT: M letter on muted chip (INK on MID)","INK","MID",4.5),
("TEXT: M letter on selected muted chip (SEL on INK)","SEL","INK",4.5),
("TEXT: knob label/value TEXT on SURF","TEXT","SURF",4.5),
("TEXT: toast title/detail INK on SEL","INK","SEL",4.5),
("TEXT MAP: encoder/OCT/printed name MID on SURF","MID","SURF",4.5),
("TEXT MAP: white-key number TEXT on RAISE","TEXT","RAISE",4.5),
("TEXT MAP: knob ids MID on BG","MID","BG",4.5),
("NT playhead column PH vs LANE column","PH","LANE",3),
("NT playhead column PH vs BG (gap, neighbours)","PH","BG",3),
("NT bar inside playhead column BG on PH","BG","PH",3),
("NT normal bar TEXT on LANE","TEXT","LANE",3),
("NT accent bar AC on LANE","AC","LANE",3),
("NT accent bar AC vs normal bar TEXT (distinguishing colour)","AC","TEXT",3),
("NT accent triangle AC on LANE","AC","LANE",3),
("NT slide tie bar TEXT on LANE","TEXT","LANE",3),
("NT slide tie bar in the 1 px gap column (x+14) TEXT on BG","TEXT","BG",3),
("NT rest dash MID on LANE","MID","LANE",3),
("NT beat pip MID on LANE","MID","LANE",3),
("NT lane column LANE vs BG","LANE","BG",3),
("NT muted selected hollow bar frame MID on LANE","MID","LANE",3),
("NT muted unselected cell: frame MID on BG","MID","BG",3),
("NT live unselected cell fill RAISE vs BG","RAISE","BG",3),
("NT selected cell fill SEL vs BG","SEL","BG",3),
("NT selected SEL vs unselected RAISE","SEL","RAISE",3),
("NT muted cell (BG+MID frame) vs live cell (RAISE)  [fill vs fill]","BG","RAISE",3),
("NT M chip fill MID vs BG","MID","BG",3),
("NT M chip fill INK(sel) vs SEL cell","INK","SEL",3),
("NT struck-through line MID on BG","MID","BG",3),
("NT strike INK on SEL (selected muted)","INK","SEL",3),
("NT OFF chip frame+text TEXT on RAISE (live, unselected, MIDI OFF)","TEXT","RAISE",3),
("NT OFF chip frame+text MID on BG (muted, unselected, MIDI OFF)","MID","BG",3),
("NT activity square PH on RAISE","PH","RAISE",3),
("NT activity square INK on SEL","INK","SEL",3),
("NT meter fill TEXT vs track LANE","TEXT","LANE",3),
("NT meter track LANE vs cell SURF","LANE","SURF",3),
("NT enum pips unselected MID on SURF","MID","SURF",3),
("NT enum pip selected TEXT on SURF","TEXT","SURF",3),
("NT hot-knob frame TEXT on SURF","TEXT","SURF",3),
("NT knob cell SURF vs BG","SURF","BG",3),
("NT ROW bar live TEXT on BG","TEXT","BG",3),
("NT ROW bar I/II INK on TEXT","INK","TEXT",3),
("NT ROW bar idle frame MID on BG","MID","BG",3),
("NT ROW bar idle numerals MID on BG","MID","BG",3),
("NT page box viewed TEXT on BG","TEXT","BG",3),
("NT page box other MID on BG","MID","BG",3),
("NT page playing marker PH on BG (hollow page)","PH","BG",3),
("NT page playing marker BG on TEXT (viewed page)","BG","TEXT",3),
("NT transport play PH on BG","PH","BG",3),
("NT transport stop MID on BG","MID","BG",3),
("NT REC disc RD on BG","RD","BG",3),
("NT toast algo unchosen pips MID on SEL","MID","SEL",3),
("NT toast countdown track MID on SEL","MID","SEL",3),
("NT toast fill INK on SEL","INK","SEL",3),
("NT toast block SEL vs BG","SEL","BG",3),
]
print("| # | pair | fg | bg | GREY ratio | need | verdict |"); print("|---|---|---|---|---|---|---|")
for i,(w,f,b,need) in enumerate(pairs,1):
    r=cr(C(f),C(b)); print("| %d | %s | %s | %s | %.2f:1 | %.1f | %s |"%(i,w,f,b,r,need,"ok" if r>=need else "UNDER"))
print()
print("Mono fallback (PH=AC=RD=TEXT): ")
for w,f,b in [("playhead col TEXT vs LANE","TEXT","LANE"),("bar in playhead column BG on TEXT","BG","TEXT"),("accent bar vs normal bar (both TEXT)","TEXT","TEXT"),("REC disc TEXT vs BG","TEXT","BG"),("play triangle TEXT vs BG","TEXT","BG"),("activity square TEXT vs RAISE","TEXT","RAISE"),("playing page marker TEXT on BG","TEXT","BG"),("playing page marker on hollow page","TEXT","BG")]:
    print("| %s | %.2f:1 |"%(w,cr(C(f),C(b))))
print("new tokens vs each other: PH-AC %.2f, PH-RD %.2f, AC-RD %.2f"%(cr(PH,AC),cr(PH,RD),cr(AC,RD)))
print("new tokens vs TEXT: PH %.2f AC %.2f RD %.2f"%(cr(PH,tok['TEXT']),cr(AC,tok['TEXT']),cr(RD,tok['TEXT'])))
print("GREY RD in mono = TEXT; triangle vs disc: shape only")
# pixel checks from real render
sc,w,recs=render(scene("glance_playing"),"gp")
px=lambda x,y: sc.px[y*240+x]
def hx(c): return "0x%04X"%c
print("pixel samples glance_playing: playhead col (5*15+7,100)",hx(px(82,100)),"lane col other (7,100)",hx(px(7,100)),"gap (14,100)",hx(px(14,100)))
