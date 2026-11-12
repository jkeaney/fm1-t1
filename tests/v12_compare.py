# SPDX-License-Identifier: GPL-3.0-only
"""v12_compare.py (P4-028, P4-029): compares the host renders of ui_sq.c (build/ui_sq/v12/<scene>.png, tests/ui_sq_v12_test.c) with the v12 mock.

   python3 tests/v12_compare.py [renders_dir]

Two references, both pixel for pixel over the WHOLE 240 x 240 frame of a scene (header, knob tiles, status line, music row, buttons, keybed, footer and
message slot: P4-028 + P4-029; the knob sheets whole):
  1. the mock re-rendered NOW from v12.py with the repo's fonts (build/ui_sq/v12_ref): must match exactly, bar the one listed difference below;
  2. the committed goldens docs/design/ui_review/proposals/v12/out/*.png: reported, and it fails only on a difference that is not text anti-aliasing
     (the goldens were rendered with an earlier text placement of the mock: 0-4 % of the pixels of a scene differ, always inside glyphs).
Listed difference (not a failure): the knob meter fill. The mock mixes the channel colour x 120/255 per RGB565 channel; the firmware uses the palette
token T_CHFn (tools/gen_ui_palettes.py mixes in RGB888). For CH2 and CH4 that is one red LSB (0x22ad -> 0x2aad, 0x59c9 -> 0x61c9), so the pixels of a
selected CH2 / CH4 knob tile (y 24-76) may differ by up to 10 per RGB888 channel; each tolerated pixel is counted and printed.
Exit 0 = pass."""
import os, subprocess, sys
from PIL import Image

ROOT = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
V12 = os.path.join(ROOT, "docs", "design", "ui_review", "proposals", "v12")
SCENES = os.path.join(ROOT, "build", "gen", "v12_scenes.h")


def names():
    import re
    txt = open(SCENES).read()
    sc = [(n, 0) for n in re.findall(r'\{ \.name = "(\w+)",', txt) if not n.startswith("sheet_")]
    sh = re.findall(r'\{ "(sheet_\w+)", \d, \d', txt)
    return sc, sh


def sel_of(txt, name):
    import re
    m = re.search(r'\.name = "%s",.*?\.sel = (\d)' % name, txt)
    return int(m.group(1)) if m else 0


def cluster(pts):
    """size of the largest group of differing pixels lying within 2 px of each other (a glyph or a word is < 100; a shape is hundreds)"""
    left, best = set(pts), 0
    while left:
        stack, n = [left.pop()], 1
        while stack:
            x, y = stack.pop()
            for dx in range(-2, 3):
                for dy in range(-2, 3):
                    q = (x + dx, y + dy)
                    if q in left:
                        left.remove(q)
                        stack.append(q)
                        n += 1
        best = max(best, n)
    return best


def px(img):
    return img.convert("RGB").load()


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else os.path.join(ROOT, "build", "ui_sq", "v12")
    ref = os.path.join(os.path.dirname(out), "v12_ref")
    os.makedirs(ref, exist_ok=True)
    r = subprocess.run([sys.executable, os.path.join(V12, "..", "..", "mock", "mock.py"), "render", os.path.join(V12, "v12.py"), "--out", ref],
                       cwd=V12, capture_output=True, text=True)
    if r.returncode:
        print("FAIL: the mock did not render:", r.stderr[-400:])
        return 1
    sc, sh = names()
    txt = open(SCENES).read()
    fails = tol_total = gold_text = worst_cluster = 0
    for name, msg in [(n, int(m)) for n, m in sc] + [(n, 0) for n in sh]:
        sheet = name.startswith("sheet_")
        mine = Image.open(os.path.join(out, name + ".png")).convert("RGB")
        regen = Image.open(os.path.join(ref, "v12_%s.png" % name)).convert("RGB")
        gold = Image.open(os.path.join(V12, "out", "v12_%s.png" % name)).convert("RGB")
        y1 = 240
        sel = 0 if sheet else sel_of(txt, name)
        a, b, g = px(mine), px(regen), px(gold)
        bad_regen = tol = bad_gold = 0
        first = None
        for y in range(y1):
            for x in range(240):
                if msg and y < 22 and x >= 104:
                    continue
                p, q, o = a[x, y], b[x, y], g[x, y]
                if p != q:
                    if sel in (1, 3) and 24 <= y <= 76 and max(abs(p[i] - q[i]) for i in range(3)) <= 10:
                        tol += 1
                    else:
                        bad_regen += 1
                        first = first or (x, y, p, q)
                if p != o:
                    if q != o:
                        gold_text += 1                      # the golden itself differs from today's mock render: text placement drift
                    elif not (sel in (1, 3) and 24 <= y <= 76 and max(abs(p[i] - o[i]) for i in range(3)) <= 10):
                        bad_gold += 1
        tol_total += tol
        drift = [(x, y) for y in range(y1) for x in range(240) if not (msg and y < 22 and x >= 104) and b[x, y] != g[x, y]]
        cl = cluster(drift)
        worst_cluster = max(worst_cluster, cl)
        if cl > 150:
            bad_gold += 1                                     # a shape, not glyph anti-aliasing, differs from the golden
        status = "ok" if not bad_regen and not bad_gold else "FAIL"
        if status == "FAIL":
            fails += 1
        print("%-22s %-4s vs mock render: %4d differ (+%d T_CHF rounding)  vs golden beyond drift: %d%s" % (
            name, status, bad_regen, tol, bad_gold, "  first %s" % (first,) if first else ""))
    print("v12_compare: %d scenes/sheets, %d failed; %d pixels differ only by the T_CHF rounding; %d pixels differ between the committed goldens and today's mock render "
          "(glyph anti-aliasing: the largest connected group is %d px)" % (len(sc) + len(sh), fails, tol_total, gold_text, worst_cluster))
    return 1 if fails else 0


if __name__ == "__main__":
    sys.exit(main())
