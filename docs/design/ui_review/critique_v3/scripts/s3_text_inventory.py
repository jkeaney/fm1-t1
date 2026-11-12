# SPDX-License-Identifier: GPL-3.0-only (evidence script; run from the repo root with .venv/bin/python)
import sys; sys.path.insert(0,"docs/design/ui_review/critique_v3")
from evidence_b_lib import *
GL=["glance_idle","glance_playing","row2","mutes","stress"]
CAP={"S":9,"M":11,"L":21}
# verify cap heights via measure
for f in "SML": print("cap_h measured", f, mock.measure("H",f)["cap_h"], "line", mock.measure("H",f)["line_h"])
allrec={}
for n in GL+["stress_row2","glance_playing_mono","stress_mono","map"]+["toast_algo","toast_preset_pending","toast_dice_notes","toast_undo"]:
    sc,w,recs=render(scene(n),n); allrec[n]=recs
    print("##",n,"warnings:",w or "none","strings",len(recs),{f:sum(1 for r in recs if r['f']==f) for f in "LMS"}, "roles",len({r['role'] for r in recs}))
print()
for n in GL:
    print("### ",n)
    print("| # | role | string | font | advance px | ink px (w x h) | cap px | cap mm | arcmin@50cm | ink box (x0,y0,x1,y1) | fg/bg contrast |")
    print("|---|---|---|---|---|---|---|---|---|---|---|")
    for i,r in enumerate(allrec[n],1):
        f=r["f_obj"]; ink=r["ink"]; cp=CAP[r["f"][-1]] if r["f"].startswith("AF_") else CAP[r["f"]]
        mm,am=arcmin(cp)
        print("| %d | %s | `%s` | %s | %d | %dx%d | %d | %.3f | %.1f | %d,%d,%d,%d | %.2f |"%(i,r["role"],r["s"],r["f"],text_w(f,r["s"]),ink[2]-ink[0],ink[3]-ink[1],cp,mm,am,*ink,cr(r["fg"],r["bg"])))
    print()
print("### MAP")
rows={}
for r in allrec["map"]:
    d=rows.setdefault(r["role"],[]); d.append(r)
print("| role | font | count | strings | min contrast | max advance |")
print("|---|---|---|---|---|---|")
for k,v in rows.items():
    print("| %s | %s | %d | %s | %.2f | %d |"%(k,v[0]["f"],len(v),", ".join("`%s`"%x["s"] for x in v[:8])+(" ..." if len(v)>8 else ""),min(cr(x["fg"],x["bg"]) for x in v),max(text_w(x["f_obj"],x["s"]) for x in v)))
print("MAP total strings",len(allrec["map"]),"roles",len(rows),"fonts",{f:sum(1 for r in allrec['map'] if r['f']==f) for f in ('AF_L','AF_M','AF_S','L','M','S')})
print("any AF_S on glance scenes:",{n:sum(1 for r in allrec[n] if r['f'].endswith('S')) for n in GL+['toast_algo','toast_preset_pending','toast_dice_notes','toast_undo']})
print("fonts names:", {r['f'] for r in allrec['glance_idle']})
# roles in playing
for n in GL: print(n,"roles",sorted({r['role'] for r in allrec[n]}),len({r['role'] for r in allrec[n]}))
# clipped: ink outside cell w for 'in' mode; and over tx cells
print("--- text ink wider than its declared cell")
for n,recs in allrec.items():
    for r in recs:
        if r["mode"]=="in" and r["w"]:
            iw=r["ink"][2]-r["ink"][0]
            if iw>r["w"]: print(n,r["role"],r["s"],"ink",iw,"cell",r["w"])
        if r["flags"]&3: print("FLAGGED",n,r["role"],r["s"])
