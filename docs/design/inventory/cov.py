# SPDX-License-Identifier: GPL-3.0-only
"""Which host tests compile which firmware file (include walk of tests/*.c + the torso cc lines in tests/run_tests.sh).
usage (repo root): python3 docs/design/inventory/cov.py > /dev/null; writes build/inventory/cov.json"""
import re,os,glob,json,collections
R=os.getcwd(); SRC=R+"/firmware/src"
def incs(path):
    try: t=open(path,errors="replace").read()
    except: return []
    return re.findall(r'^\s*#\s*include\s+"([^"]+)"',t,re.M)
def resolve(frm,name):
    for d in (os.path.dirname(frm),SRC,R+"/tests",R+"/build/gen"):
        p=os.path.normpath(os.path.join(d,name))
        if os.path.isfile(p): return p
cov=collections.defaultdict(set)
def walk(p,seen):
    if p in seen: return
    seen.add(p)
    for i in incs(p):
        q=resolve(p,i)
        if q: walk(q,seen)
tests=sorted(glob.glob(R+"/tests/*_test.c")+[R+"/tests/regress.c",R+"/tests/hostsim.c",R+"/tests/descdump.c",R+"/tests/ui_render.c",R+"/tests/fuzz_ed.c",R+"/tests/fuzz_proj.c",R+"/tests/fuzz_smp.c"])
for t in tests:
    seen=set(); walk(t,seen)
    name=os.path.basename(t)[:-2]
    for s in seen:
        if s.startswith(SRC+"/"): cov[os.path.basename(s)].add(name)
# torso cc lines
for m in re.finditer(r'-o "\$OUT/(torso_phase\d_test)" tests/\S+ ((?:firmware/src/\S+\.c ?)+)',open("tests/run_tests.sh").read()):
    for f in m.group(2).split(): cov[os.path.basename(f)].add(m.group(1))
json.dump({k:sorted(v) for k,v in cov.items()},open("build/inventory/cov.json","w"))
for k,v in sorted(cov.items()): print(k,len(v),",".join(sorted(v))[:100])
