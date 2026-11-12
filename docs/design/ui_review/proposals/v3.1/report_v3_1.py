# SPDX-License-Identifier: GPL-3.0-only
"""report_v3_1.py: prints the markdown tables quoted in NOTES.md (text inventory, enumerated names, palette tokens and contrast, item-9 fits, warnings).
   .venv/bin/python docs/design/ui_review/proposals/v3.1/report_v3_1.py > /tmp/report_v3_1.md"""
import contextlib
import io
import math
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "..", "mock"))
sys.path.insert(0, HERE)
import mock                                                                  # noqa: E402
from mock import AF_S, AF_M, AF_L, text_ink, contrast_ratio, palette_set, T   # noqa: E402
import proposal_v3_1 as P                                                    # noqa: E402

CAP = {"S": 9, "M": 11, "L": 21}
GLANCE = ("glance_idle", "glance_playing", "row2", "mutes", "stress", "stress_row2", "toast_algo", "toast_preset_pending", "toast_dice_notes",
          "toast_undo", "snake_path")
MAPS = ("map", "map_menu_countdown", "map_touch")
PALS = ("GREY", "GREEN", "AMBER", "ICE", "VIOLET", "ROSE", "PAPER", "NIGHT", "MONO")


def arcmin(px):
    mm = px / 8.0
    return mm, 2 * math.atan(mm / 2 / 500.0) * 180 / math.pi * 60


def ink_w(f, s):
    b = text_ink(f, s)
    return b[2] - b[0]


def main():
    LOGS, warns = {}, {}
    for name, st in P.STATE["scenes"].items():
        P.LOG.clear()
        buf = io.StringIO()
        with contextlib.redirect_stdout(buf):
            mock.render_state(P, st)
        LOGS[name] = list(P.LOG)
        warns[name] = [l.strip() for l in buf.getvalue().splitlines() if "warning" in l]

    def table(scene_names, title):
        rows = {}
        for n in scene_names:
            for e in LOGS[n]:
                r = rows.setdefault(e["role"], {"f": e["f"], "ex": set(), "min": 99.0, "cnt": {}})
                r["ex"].add(e["s"])
                palette_set(P.STATE["scenes"][n].get("palette", "GREY"))
                r["min"] = min(r["min"], contrast_ratio(e["fg"], e["bg"]))
                r["cnt"][n] = r["cnt"].get(n, 0) + 1
        print("\n### %s\n" % title)
        print("| role | font | strings drawn (all scenes) | cap px | cap mm | arcmin @50 cm | worst contrast | need | instances in busiest scene |")
        print("| --- | --- | --- | --- | --- | --- | --- | --- | --- |")
        for role, r in rows.items():
            px = CAP[r["f"]]
            mm, am = arcmin(px)
            need = 3.0 if r["f"] == "L" else 4.5
            ok = "ok" if r["min"] >= need else "FAIL"
            ex = sorted(r["ex"], key=lambda s: (len(s), s))
            exs = ", ".join(ex[:7]) + (" ..." if len(ex) > 7 else "")
            print("| %s | AF_%s | `%s` | %d | %.2f | %.1f | %.2f:1 | %.1f %s | %d |" % (role, r["f"], exs, px, mm, am, r["min"], need, ok, max(r["cnt"].values())))
        return rows

    rows = table(GLANCE, "(a) GLANCE screen text items (all glance scenes, palette GREY)")
    print("\nroles (distinct text kinds) on the glance screen: %d" % len(rows))
    for n in GLANCE:
        ent = LOGS[n]
        print("- %s: %d strings drawn (%s)" % (n, len(ent), ", ".join("%s x%d" % (f, sum(1 for e in ent if e["f"] == f)) for f in "LMS" if any(e["f"] == f for e in ent))))
    mrows = table(MAPS, "(b) MAP (HOME held) text items")
    for n in MAPS:
        print("- %s: %d strings drawn" % (n, len(LOGS[n])))
    print("\nMAP roles: %d, strings drawn in `map`: %d" % (len(mrows), len(LOGS["map"])))

    print("\n### (c) enumerated knobs: what is drawn\n")
    print("| enum | value | firmware string | drawn as | name font | name ink px |")
    print("| --- | --- | --- | --- | --- | --- |")
    for k, (kind, names) in P.ENUMS.items():
        if k == "KEYS":
            continue
        for i, nm in enumerate(names):
            if kind == "path":
                print("| PATH | %d | path %d | route picture 31x31 (from snake/proto.c SHAPE) + numeral | AF_M | %d |" % (i, i + 1, ink_w(AF_M, str(i + 1))))
            elif kind == "pic":
                print("| %s | %d | %s | picture 46x24 + name | AF_M | %d |" % (k, i, nm, ink_w(AF_M, nm)))
            else:
                print("| %s | %d | %s | text | AF_L | %d |" % (k, i, nm, ink_w(AF_L, nm)))

    print("\n### (d) item 9: algorithm names, ink width px (cell 58, AF_L limit 56)\n")
    print("| name | AF_M | AF_L | fits AF_L 56? |")
    print("| --- | --- | --- | --- |")
    for nm in ("ACID", "EUCL", "SNAK", "STAG", "ARP", "PATT", "ACD", "EUC", "SNK", "STG", "PAT", "EUCLID", "SNAKE", "STAGE"):
        print("| %s | %d | %d | %s |" % (nm, ink_w(AF_M, nm), ink_w(AF_L, nm), "yes" if ink_w(AF_L, nm) <= 56 else "NO"))
    print("\nenum text widths in AF_L (56): DIR3 UP %d DN %d P-P %d PING %d" % (ink_w(AF_L, "UP"), ink_w(AF_L, "DN"), ink_w(AF_L, "P-P"), ink_w(AF_L, "PING")))

    print("\n### (e) palette tokens and contrast, all 9 palettes\n")
    print("`PLAY` = T_PLAYHEAD, `ACC` = T_ACCSTEP, `REC` = T_REC. Pairs: PLAY on BG / LANE / RAISE (activity square on an unselected cell) / SURF; ACC on BG / LANE; REC on BG;")
    print("BG (ink) on PLAY and on ACC (bars inside the playhead column; the accent bar's own ink). Non-text needs 3:1.\n")
    print("| palette | PLAY | ACC | REC | PLAY/BG | PLAY/LANE | PLAY/RAISE | PLAY/SURF | ACC/BG | ACC/LANE | REC/BG | BG on PLAY | min | verdict |")
    print("| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |")
    for pn in PALS:
        palette_set(pn)
        tk = P.PAL_TOKENS[pn]
        if tk is None:
            tk = (T.TEXT, T.TEXT, T.TEXT)
        pl, ac, rc = tk
        bg, lane, rs, sf = (mock.color(k) for k in ("BG", "LANE", "RAISE", "SURF"))
        vals = [contrast_ratio(pl, bg), contrast_ratio(pl, lane), contrast_ratio(pl, rs), contrast_ratio(pl, sf), contrast_ratio(ac, bg),
                contrast_ratio(ac, lane), contrast_ratio(rc, bg), contrast_ratio(bg, pl)]
        m = min(vals)
        print("| %s | %04X | %04X | %04X | %s | %.2f | %s |" % (pn, pl, ac, rc, " | ".join("%.2f" % v for v in vals), m, "ok" if m >= 3 else "FAIL"))

    print("\n### (f) every other drawn pair, per palette (text 4.5:1, shapes 3:1)\n")
    pairs = (("TEXT", "BG", 4.5, "text"), ("TEXT", "RAISE", 4.5, "text"), ("TEXT", "SURF", 4.5, "text"), ("INK", "SEL", 4.5, "text"), ("MID", "BG", 4.5, "inactive ROW label, shapes"),
             ("MID", "SURF", 4.5, "map name text"), ("INK", "MID", 4.5, "M chip letter"), ("MID", "RAISE", 3.0, "shape: rest dash, hollow bar"),
             ("MID", "LANE", 3.0, "shape: rest dash, hollow bar, beat pip"), ("TEXT", "LANE", 3.0, "shape: bars"), ("SEL", "RAISE", 3.0, "selected vs normal cell"),
             ("SEL", "BG", 3.0, "selected vs muted (hollow) cell"), ("INK", "TEXT", 4.5, "ROW label on the live half"), ("MID", "BG", 3.0, "hollow cell frame"))
    print("| pair | need | use | " + " | ".join(PALS) + " |")
    print("| --- | --- | --- | " + " | ".join("---" for _ in PALS) + " |")
    for a, b, need, use in pairs:
        cells = []
        for pn in PALS:
            palette_set(pn)
            r = contrast_ratio(mock.color(a), mock.color(b))
            cells.append("%.2f%s" % (r, "" if r >= need else " **FAIL**"))
        print("| %s on %s | %.1f | %s | %s |" % (a, b, need, use, " | ".join(cells)))

    print("\n### (g) the alternative: palette-native tokens (PLAY = T_ACCENT, ACC = T_THEME), contrast\n")
    print("| palette | T_ACCENT | T_THEME | ACCENT/BG | ACCENT/LANE | ACCENT/RAISE | THEME/BG | THEME/LANE | min |")
    print("| --- | --- | --- | --- | --- | --- | --- | --- | --- |")
    for pn in PALS:
        palette_set(pn)
        ac, th = mock.color("ACCENT"), mock.color("THEME")
        bg, lane, rs = (mock.color(k) for k in ("BG", "LANE", "RAISE"))
        v = [contrast_ratio(ac, bg), contrast_ratio(ac, lane), contrast_ratio(ac, rs), contrast_ratio(th, bg), contrast_ratio(th, lane)]
        print("| %s | %04X | %04X | %s | %.2f |" % (pn, ac, th, " | ".join("%.2f" % x for x in v), min(v)))

    print("\n### (h) tool warnings per scene\n")
    for n, w in warns.items():
        print("- %s: %s" % (n, "none" if not w else "; ".join(w)))


if __name__ == "__main__":
    main()
