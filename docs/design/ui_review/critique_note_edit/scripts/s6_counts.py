# SPDX-License-Identifier: GPL-3.0-only (run from repo root: .venv/bin/python <this>)
import sys; sys.path.insert(0, "docs/design/ui_review/critique_note_edit/scripts")
from ne_lib import *
print("| render | text draws (visible) | distinct strings | distinct (string,font) | AF_L / AF_M / AF_S draws | distinct text fg colours | distinct text bg colours | icons | filled rects (R) | frames (F) | unique RGB values in the frame | of which exact palette tokens | AA blends (other) | colours drawn by rect/frame/icon/text ops (cv_pset hatches not logged) |\n|---|---|---|---|---|---|---|---|---|---|---|---|---|---|")
for n in RENDERS + EXTRA:
    T = vtexts(n); O = ops(n); im = img(n)
    c = collections.Counter(im.get_flattened_data() if hasattr(im, 'get_flattened_data') else im.getdata())
    tokv = {}
    for k, v in TOK.items(): tokv.setdefault(v, k)
    tok_rgb = {}
    for v, k in tokv.items():
        r, g, b = v >> 11 & 31, v >> 5 & 63, v & 31; tok_rgb[(r * 255 // 31, g * 255 // 63, b * 255 // 31)] = k
    hit = [k for rgb, k in tok_rgb.items() if rgb in c]
    other = [rgb for rgb in c if rgb not in tok_rgb]
    fs = collections.Counter(t["f"] for t in T)
    print("| %s | %d | %d | %d | %d / %d / %d | %d (%s) | %d (%s) | %d | %d | %d | %d | %d | %d | %s |" % (
        n, len(T), len({t["s"] for t in T}), len({(t["s"], t["f"]) for t in T}), fs["L"], fs["M"], fs["S"],
        len({t["fg"] for t in T}), ", ".join(sorted({names_of(t["fg"]) for t in T})), len({t["bg"] for t in T}), ", ".join(sorted({names_of(t["bg"]) for t in T})),
        sum(1 for o in O if o["op"] == "I"), sum(1 for o in O if o["op"] == "R"), sum(1 for o in O if o["op"] == "F"), len(c), len(c) - len(other), len(other), ", ".join(sorted({names_of(o["c"]) for o in O if o["op"] in "RFI"} | {names_of(t["fg"]) for t in T}))))
print("\n### the same counts on the PERF page, same state (the locked main-screen code path), for comparison")
print("| render | text draws (visible) | distinct strings | AF_L / AF_M / AF_S | distinct text fg | icons | unique RGB values |\n|---|---|---|---|---|---|---|")
for n in RENDERS:
    m = n + "_perf"; T = vtexts(m); O = ops(m); im = img(m); fs = collections.Counter(t["f"] for t in T)
    print("| %s | %d | %d | %d / %d / %d | %d | %d | %d |" % (m, len(T), len({t["s"] for t in T}), fs["L"], fs["M"], fs["S"], len({t["fg"] for t in T}), sum(1 for o in O if o["op"] == "I"), len(set(im.get_flattened_data() if hasattr(im, 'get_flattened_data') else im.getdata()))))
print("\n### page-specific (NOTE_EDIT) vs persistent text, per render: strings that exist only here (not on the same-state PERF render)")
for n in RENDERS:
    a = collections.Counter((t["s"], t["x"], t["y"]) for t in vtexts(n)); b = collections.Counter((t["s"], t["x"], t["y"]) for t in vtexts(n + "_perf"))
    only = [k for k in a if k not in b]; gone = [k for k in b if k not in a]
    print("- %s: NOTE_EDIT-only: %s; PERF-only (absent on NOTE_EDIT): %s" % (n, ", ".join("`%s`@%d,%d" % k for k in only), ", ".join("`%s`@%d,%d" % k for k in gone)))
