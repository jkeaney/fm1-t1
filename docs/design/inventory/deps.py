# SPDX-License-Identifier: GPL-3.0-only
"""File-to-file symbol references: file A mentions an identifier that attrib.py (nm.json) says file B defines.
usage (repo root, after attrib.py): python3 docs/design/inventory/deps.py   -> build/inventory/deps.json
Edges are kept only if the definition comes earlier in the unity-build include order or the user has a prototype.
Approximation: functions and variables only (no macros, types, enums), and anything the compiler inlined away is
absent from the symbol table, so a name used only from an inlined helper may be missed."""
import json, re, glob, os, collections
nm = json.load(open('build/inventory/nm.json'))
SRC = 'firmware/src/'
# translation-unit order (felucca.c include DFS, FELUCCA_FM4=0 so eng_digital.c is not reached): in a unity build a
# file can only use what an earlier file defines, unless it forward-declares the name (a prototype line)
order, seen = {}, set()
def walk(f):
    if f in seen or not os.path.exists(SRC + f): return
    seen.add(f); order[f] = len(order)
    t = open(SRC + f, errors='replace').read()
    for m in re.finditer(r'^\s*#\s*include\s+"([^"]+)"', t, re.M):
        if m.group(1) != 'eng_digital.c': walk(m.group(1))
walk('felucca.c')
deps = collections.defaultdict(collections.Counter)
for p in glob.glob('firmware/src/*.c') + glob.glob('firmware/src/*.h'):
    f = os.path.basename(p)
    t = re.sub(r'/\*.*?\*/|//[^\n]*', '', open(p, errors='replace').read(), flags=re.S)
    for w in set(re.findall(r'[A-Za-z_]\w*', t)):
        g = nm.get(w)
        if not g or g == f: continue
        if order.get(g, 1e9) < order.get(f, -1): deps[f][g] += 1          # defined earlier in the unit
        elif re.search(r'^[^\n{};]*\b' + re.escape(w) + r'\s*\([^{};]*\)\s*;', t, re.M): deps[f][g] += 1   # forward-declared
json.dump({k: dict(v) for k, v in deps.items()}, open('build/inventory/deps.json', 'w'))
