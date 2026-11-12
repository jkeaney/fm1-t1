# SPDX-License-Identifier: GPL-3.0-only (run from repo root: .venv/bin/python <this>)
import sys; sys.path.insert(0, "docs/design/ui_review/critique_note_edit/scripts")
from ne_lib import *
print("cap px measured: ", {f: mock.measure("H", f)["cap_h"] for f in "SML"}, "; 8 px/mm; arcmin = cap_mm / dist_mm * 3438")
print("| font | cap px | cap mm | arcmin @35 cm | arcmin @50 cm |\n|---|---|---|---|---|")
for f in "SML": print("| AF_%s | %d | %.3f | %.1f | %.1f |" % (f, CAP[f], cap_mm(f), arcmin(f, 35), arcmin(f, 50)))
for n in RENDERS + EXTRA:
    T = vtexts(n); H = hidden(n)
    print("\n#### %s (%d text draws visible; later filled rects cover (fraction of ink box): %s)\n| # | string | font | x,y(line top) | advance px | ink box x0,y0,x1,y1 | ink w x h | cap mm | arcmin@35 | arcmin@50 | fg (token) | bg passed (token) | fg/bg |\n|---|---|---|---|---|---|---|---|---|---|---|---|---|" % (n, len(T), ", ".join("`%s` %.2f" % (t["s"], f) for t, f in H) or "none"))
    for i, t in enumerate(T, 1):
        x0, y0, x1, y1 = t["ink"]
        print("| %d | `%s` | %s | %d,%d | %d | %d,%d,%d,%d | %dx%d | %.3f | %.1f | %.1f | %s %s | %s %s | %.2f |" % (i, t["s"], t["f"], t["x"], t["y"], text_w(FONT[t["f"]], t["s"]), x0, y0, x1, y1, x1 - x0, y1 - y0, cap_mm(t["f"]), arcmin(t["f"], 35), arcmin(t["f"], 50), names_of(t["fg"]), hexof(t["fg"]), names_of(t["bg"]), hexof(t["bg"]), cr(t["fg"], t["bg"])))
# icons
print("\n### icons / glyphs drawn (non-text, 1 bpp), per render")
for n in RENDERS:
    I = [o for o in ops(n) if o["op"] == "I"]
    print("- %s: %d icons: %s" % (n, len(I), ", ".join("%s %dx%d@%d,%d %s" % (icon_name(o["id"]), o["w"], o["h"], o["x"], o["y"], names_of(o["c"])) for o in I)))
