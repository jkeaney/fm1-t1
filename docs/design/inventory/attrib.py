#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Attribute symbol sizes in build/felucca.elf to source files (docs/design/SYSTEM_INVENTORY.md).
usage (from the repo root): python3 docs/design/inventory/attrib.py build/inventory/syms.txt
  syms.txt = `objdump -t build/felucca.elf` from the JieLi toolchain (see SYSTEM_INVENTORY.md "Re-running").
Writes build/inventory/{res,unm,nm}.json and prints one line per file. Method: each ELF symbol is matched to the
source file that defines it (regex on the definition line); symbols nested inside a larger one and the compiler's
.L_MergedGlobals are dropped (no double counting). Inlined functions count toward the file they were inlined into."""
import re, sys, glob, os, collections, json
ROOT = os.getcwd()
SRC = ROOT + "/firmware/src"
files = {os.path.basename(p): p for p in glob.glob(SRC + "/*.[ch]") + glob.glob(ROOT + "/build/gen/*.h") + glob.glob(ROOT + "/firmware/hal/*.h")}
text = {n: re.sub(r"/\*.*?\*/", lambda m: "\n" * m.group().count("\n"), open(p, errors="replace").read(), flags=re.S) for n, p in files.items()}
ATTR = re.compile(r"__attribute__\s*\(\((?:[^()]|\([^()]*\))*\)\)")
text = {n: ATTR.sub(" ", t) for n, t in text.items()}
# include tree for .c files
inc = {}
for n, t in text.items():
    inc[n] = re.findall(r'^\s*#\s*include\s+"([^"]+)"', t, re.M)
def top_of():
    top = {}
    def walk(n, root):
        for c in inc.get(n, []):
            if c in files and c not in top:
                top[c] = root; walk(c, root)
    for c in inc["felucca.c"]:
        if c.endswith(".c"):
            top[c] = c; walk(c, c)
        elif c in files:
            top[c] = "(hal/gen header)"
    return top
TOP = top_of()
MANUAL = {"hc_tab": "gfx.c", "HELP_PAGE": "ui_help.c", "NOISE_SEQ": "eng_noise.c", "FM4_ALG": "fm4_convert.c",
          "graph_scope.snap": "ui_graph.c", "t1_all_off": "t1_glue.c", "tl_open": "ui_tools.c",
          "con_dbg.NAMES": "console.c", "draw_menu.KEYS": "ui_menu.c", "st_crc32_add.T": "storage.c", "eng_abbr.A": "engines.c",
          "engine_icon.M": "icons.c"}
def owner(name, typ):
    if name in MANUAL: return [MANUAL[name]]
    if name.startswith("ota_stage."): return ["ota.c"]
    base = re.sub(r"\.\d+$", "", name)
    if typ == "F":
        pat = re.compile(r"^[A-Za-z_][^;{}#\n]*?\b" + re.escape(base) + r"\s*\([^;{}]*\)\s*(?:__attribute__\s*\(\([^;{}]*\)\)\s*)?\{", re.M)
    else:
        pat = re.compile(r"^(?:[A-Za-z_}]|[ \t]+static\b)[^;{}()#\n]*?\b" + re.escape(base) + r"\b\s*(?:\[[^\]]*\]\s*)*(?:=|;|,)", re.M)
    return [n for n, t in text.items() if n != "felucca.c" and pat.search(t)]
syms = []
for ln in open(sys.argv[1]):
    p = ln.split()
    if len(p) < 5 or ".GJT" in ln: continue
    m = re.match(r"([0-9a-f]{8}) (\w)\s+(F|O|d|df)?\s*(\S+)\s+([0-9a-f]{8}) (.+)$", ln.strip())
    if not m: continue
    addr, bind, typ, sec, size, name = m.groups()
    size = int(size, 16)
    if size == 0 or sec in ("*ABS*", "*UND*"): continue
    syms.append((addr, name, sec, size, typ or ""))
best = {}
for a, n, s, z, t in syms:
    if (s, a) not in best or z > best[(s, a)][2]: best[(s, a)] = (n, s, z, t)
syms = list(best.values())
# drop compiler-merged globals (.L_MergedGlobals*: their members are separate symbols) and symbols wholly inside a larger one
addr_of = {(s_, a): v for (s_, a), v in best.items()}
iv = sorted([(int(a, 16), int(a, 16) + v[2], v) for (s_, a), v in best.items() if not v[0].startswith(".L_Merged")], key=lambda t: (t[0], -t[1]))
keep, end, cursec = [], -1, None
for lo, hi, v in iv:
    if v[1] != cursec: cursec, end = v[1], -1
    if hi <= end: continue
    keep.append(v); end = max(end, hi)
syms = keep
res = collections.defaultdict(lambda: collections.Counter())
unm = collections.defaultdict(list)
for name, sec, size, typ in syms:
    h = owner(name, typ)
    if len(h) == 0: key = "?unmatched"; unm[sec].append((name, size))
    else:
        h = [x for x in h if x.endswith('.c')] or h
        tops = sorted(set(h))
        key = tops[0] if len(tops) == 1 else "?ambiguous:" + "|".join(tops)
        if len(tops) > 1:
            unm[sec].append((name, size, tops))
    res[key][sec] += size
    res[key][sec + "#"] += 1
out = {k: dict(v) for k, v in res.items()}
json.dump({"res": out, "top": TOP, "inc": {k: v for k, v in inc.items() if k.endswith(".c") or True}}, open("build/inventory/res.json", "w"), indent=1)
json.dump({s: v for s, v in unm.items()}, open("build/inventory/unm.json", "w"))
tot = collections.Counter()
for k, v in sorted(out.items()):
    print(f"{k:28} " + " ".join(f"{s}={v[s]}" for s in v if not s.endswith("#")))
    for s in v:
        if not s.endswith("#"): tot[s] += v[s]
print(dict(tot))
# --- name -> file map (for dependency scan)
nm = {}
for name, sec, size, typ in syms:
    base = re.sub(r"\.\d+$", "", name)
    if "." in base: continue
    h = owner(name, typ); h = [x for x in h if x.endswith(".c")] or h
    if len(set(h)) == 1: nm[base] = h[0]
json.dump(nm, open("build/inventory/nm.json", "w"))
