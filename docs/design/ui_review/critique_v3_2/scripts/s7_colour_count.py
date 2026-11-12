# SPDX-License-Identifier: GPL-3.0-only (evidence script; repo root, .venv/bin/python): colours per glance screen; new tokens on every palette
import sys, collections; sys.path.insert(0, "docs/design/ui_review/critique_v3_2")
from evidence_b_lib import *
PALS = ["GREY", "GREEN", "AMBER", "ICE", "VIOLET", "ROSE", "PAPER", "NIGHT", "MONO"]
GL = ["glance_idle", "glance_playing", "row2", "row3", "mutes", "stress", "stress_row2", "stage_total", "stress_pal_paper", "stress_pal_mono", "stress_pal_ice", "toast_dice_notes", "toast_preset_pending", "state_sol", "state_fil", "state_rol", "state_follow_off", "lane_full_playhead"]
print("| scene | palette | distinct RGB565 pixel values | existing T_* token values present | new tokens present (HEAD/ACC/RED) | derived present (LANE2/MUTEC/DIMINK, not table rows) | other = 4-bit anti-alias ramp values | pixels by class: token / new / derived / ramp (of 57600) |\n|---|---|---|---|---|---|---|---|")
for n in GL:
    s = scene(n); pal = s.get("palette", "GREY")
    sc, w, recs = render(s, n)
    toks = {k: getattr(T, k) for k in ("BG", "SURF", "TEXT", "MID", "DIM", "LINE", "SEL", "INK", "RAISE", "LANE", "KEY", "THEME", "ACCENT", "REC", "GRID", "TINT")}
    new = dict(zip(("HEAD", "ACC", "RED"), P.pal({})))
    der = dict(LANE2=P.lane2(), MUTEC=P.mutec(), DIMINK=P.dimink())
    cnt = collections.Counter(sc.px)
    tv = set(toks.values()); nv = set(new.values()); dv = set(der.values())
    present_t = sorted({k for k, v in toks.items() if v in cnt})
    # distinct token VALUES present
    tval = {v for v in tv if v in cnt}
    nval = {v for v in nv if v in cnt and v not in tv}
    dval = {v for v in dv if v in cnt and v not in tv and v not in nv}
    ramp = set(cnt) - tval - nval - dval
    cls = lambda S_: sum(cnt[v] for v in S_)
    print("| %s | %s | %d | %d | %d (%s) | %d (%s) | %d | %d / %d / %d / %d |" % (n, pal, len(cnt), len(tval), len(nval), ",".join("%s=%04X" % (k, v) for k, v in new.items() if v in nval), len(dval), ",".join("%s=%04X" % (k, v) for k, v in der.items() if v in dval), len(ramp), cls(tval), cls(nval), cls(dval), cls(ramp)))
print("\n## token values on every palette in build/gen/ui_palettes.h (mock.py palette NAME), and the proposal's new / derived values with WCAG ratio\n")
print("| palette | BG | TEXT | LANE | LANE2 (derived) | MUTEC (derived) | DIMINK (derived) | HEAD | ACC | RED |\n|---|---|---|---|---|---|---|---|---|---|")
h = lambda v: "%04X #%02x%02x%02x" % (v, *to_rgb888(v))
for p in PALS:
    mock.palette_set(p)
    t = dict(BG=T.BG, TEXT=T.TEXT, LANE=T.LANE, LANE2=P.lane2(), MUTEC=P.mutec(), DIMINK=P.dimink()); t["HEAD"], t["ACC"], t["RED"] = P.pal({})
    print("| %s | %s | %s | %s | %s | %s | %s | %s | %s | %s |" % (p, h(t["BG"]), h(t["TEXT"]), h(t["LANE"]), h(t["LANE2"]), h(t["MUTEC"]), h(t["DIMINK"]), h(t["HEAD"]), h(t["ACC"]), h(t["RED"])))
print("\n| palette | HEAD/BG | HEAD/LANE | HEAD/LANE2 | HEAD/SURF | ACC/BG | ACC/LANE | ACC/LANE2 | ACC/TEXT | RED/BG | RED/SURF | RED/LANE | MUTEC/LANE | MUTEC/LANE2 | MUTEC/BG | DIMINK/SEL | LANE2/LANE | HEAD/ACC | HEAD/RED | ACC/RED |\n|---|" + "---|" * 19)
for p in PALS:
    mock.palette_set(p)
    hd, ac, rd = P.pal({}); L = T.LANE; L2 = P.lane2(); B = T.BG; S_ = T.SURF; X = T.TEXT; M = P.mutec(); D = P.dimink()
    v = [cr(hd, B), cr(hd, L), cr(hd, L2), cr(hd, S_), cr(ac, B), cr(ac, L), cr(ac, L2), cr(ac, X), cr(rd, B), cr(rd, S_), cr(rd, L), cr(M, L), cr(M, L2), cr(M, B), cr(D, T.SEL), cr(L2, L), cr(hd, ac), cr(hd, rd), cr(ac, rd)]
    print("| %s | %s |" % (p, " | ".join(("%.2f%s" % (x, "!" if (i in (0, 1, 2, 4, 5, 6, 8, 11, 12, 13) and x < 3) or (i == 14 and x < 4.5) else "")) for i, x in enumerate(v))))
print("('!' = below 3:1 for a meaning-bearing mark, or below 4.5:1 for DIMINK text)")
print("\nGREY palette: T_ACCENT == T_REC == T_TEXT (is_grey), so on GREY the proposal's HEAD/ACC/RED literals are the only chromatic values; on the other palettes RED = 0xFA49 = the palette's own REC value:")
for p in PALS:
    mock.palette_set(p); print("  %-7s REC=%04X ACCENT=%04X THEME=%04X  RED(proposal)=%04X" % (p, T.REC, T.ACCENT, T.THEME, P.pal({})[2]))
