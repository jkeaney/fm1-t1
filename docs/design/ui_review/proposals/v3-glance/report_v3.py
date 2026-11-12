# SPDX-License-Identifier: GPL-3.0-only
"""report_v3.py: prints the markdown tables quoted in NOTES.md (text inventory, contrast, enumerated knob names, warnings).
   .venv/bin/python docs/design/ui_review/proposals/v3-glance/proposal_v3.py --report"""
import io
import math
import contextlib
import mock
from mock import AF_S, AF_M, AF_L, text_ink, contrast_ratio, palette_set, T

CAP = {"S": 9, "M": 11, "L": 21}


def arcmin(px):
    mm = px / 8.0
    return mm, 2 * math.atan(mm / 2 / 500.0) * 180 / math.pi * 60


def main():
    import proposal_v3 as P
    mod = P
    LOGS = {}
    warns = {}
    for name, st in P.STATE["scenes"].items():
        P.LOG.clear()
        buf = io.StringIO()
        with contextlib.redirect_stdout(buf):
            mock.render_state(mod, st)
        LOGS[name] = list(P.LOG)
        warns[name] = [l.strip() for l in buf.getvalue().splitlines() if "warning" in l]
    glance = [n for n in LOGS if n != "map" and not n.endswith("_mono")]

    def table(scene_names, title):
        rows = {}
        for n in scene_names:
            for e in LOGS[n]:
                r = rows.setdefault(e["role"], {"f": e["f"], "ex": set(), "min": 99.0, "cnt": {}, "cnt_max": 0})
                r["ex"].add(e["s"])
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

    rows = table(glance, "(a) GLANCE screen text items (all scenes)")
    print("\nroles (distinct text kinds) on the glance screen: %d" % len(rows))
    for n in ("glance_idle", "glance_playing", "row2", "mutes", "stress", "toast_algo", "toast_preset_pending", "toast_dice_notes", "toast_undo"):
        ent = LOGS[n]
        print("- %s: %d strings drawn (%s)" % (n, len(ent), ", ".join("%s x%d" % (f, sum(1 for e in ent if e["f"] == f)) for f in "LMS" if any(e["f"] == f for e in ent))))
    mrows = table(["map"], "(b) MAP (HOME held) text items")
    print("\nMAP roles: %d, strings drawn: %d" % (len(mrows), len(LOGS["map"])))
    print("\n### (c) enumerated knobs\n")
    print("| enum | value | firmware string | proposed | font (60 px cell) | ink px |")
    print("| --- | --- | --- | --- | --- | --- |")
    for k, names in P.ENUMS.items():
        if k == "KEYS":
            continue
        inL = P.enum_in_L(k)
        for i, (ln, mn) in enumerate(names):
            f, s = ("AF_L", ln) if inL else ("AF_M", mn)
            b = text_ink(AF_L if inL else AF_M, s)
            print("| %s | %d | %s | `%s` | %s | %d |" % (k, i, P.FW_NAMES[k][i] if k in P.FW_NAMES else "path %d" % (i + 1), s, f, b[2] - b[0]))
    print("\n### (e) tool warnings per scene\n")
    for n, w in warns.items():
        print("- %s: %s" % (n, "none" if not w else "; ".join(w)))
    print("\n### contrast of the shapes and tokens\n")
    palette_set("GREY")
    pairs = [("TEXT", "BG"), ("TEXT", "RAISE"), ("TEXT", "SURF"), ("INK", "SEL"), ("MID", "BG"), ("MID", "RAISE"), ("MID", "LANE"),
             ("TEXT", "LANE"), ("MID", "SURF"), ("TEXT", "SEL"), ("INK", "MID")]
    for a, b in pairs:
        print("- %s on %s: %.2f:1" % (a, b, contrast_ratio(mock.color(a), mock.color(b))))
    for nm, c in (("T_PLAYHEAD", P.T_PLAYHEAD), ("T_ACCSTEP", P.T_ACCSTEP), ("T_RECRED", P.T_RECRED)):
        print("- %s 0x%04X: on BG %.2f, on LANE %.2f, on RAISE %.2f, on SEL %.2f; BG ink on it %.2f" % (
            nm, c, contrast_ratio(c, mock.color("BG")), contrast_ratio(c, mock.color("LANE")), contrast_ratio(c, mock.color("RAISE")),
            contrast_ratio(c, mock.color("SEL")), contrast_ratio(mock.color("BG"), c)))
    print("- bar (TEXT) on LANE %.2f; playhead column T_PLAYHEAD vs LANE column %.2f; mono playhead TEXT vs LANE %.2f" % (
        contrast_ratio(mock.color("TEXT"), mock.color("LANE")), contrast_ratio(P.T_PLAYHEAD, mock.color("LANE")),
        contrast_ratio(mock.color("TEXT"), mock.color("LANE"))))
