# SPDX-License-Identifier: GPL-3.0-only (evidence script; run from the repo root with .venv/bin/python)
import sys,hashlib,glob,os; sys.path.insert(0,"docs/design/ui_review/critique_v3")
from evidence_b_lib import *
def m(s,f): r=mock.measure(s,f); return (r["advance"],r["ink_w"])
print("claims: MELODY/TECHNO AF_M",m("MELODY","M"),m("TECHNO","M"),"| KEYS AF_L",m("KEYS","L"),"KEY",m("KEY","L"),"| FWD AF_L",m("FWD","L"),"| PATTRN AF_M",m("PATTRN","M"),"PATT",m("PATT","M"),"| PATTERNS AF_L",m("PATTERNS","L"),"| '1 OCT' AF_L",m("1 OCT","L"),"| 120 AF_L",m("120","L"),"| EXT 126 AF_S",m("EXT 126","S"),"AF_M",m("EXT 126","M"))
print("NOTES vs ACCNT gap in MAP:")
sc,w,recs=render(scene("map"),"map")
b=[r for r in recs if r["role"]=="map button tap"]
for i in range(1,len(b)):
    pass
tap=[(r["s"],r["ink"]) for r in recs if r["role"]=="map button tap"]; print(tap[:6])
# palettes REC
import subprocess
print(subprocess.run([".venv/bin/python","docs/design/ui_review/mock/mock.py","palette"],capture_output=True,text=True).stdout[:600])
