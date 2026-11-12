# SPDX-License-Identifier: GPL-3.0-only
"""report_v3_2.py: every measurement behind NOTES.md.  Prints markdown tables to stdout (fragments are marked '## [name]') and writes geometry_v3_2.tsv.
   .venv/bin/python docs/design/ui_review/proposals/v3.2/report_v3_2.py > /tmp/report_v3_2.md
Sections: geometry (module remainder of every element), text (every text item, per scene), contrast (every non-text and text pair, nine palettes), evidence
(pixel checks of v3.1 against v3.2 for the critique's findings), dice (per algorithm and layer: title, detail, widths), cues (every string and its width),
rows (off-row name widths), maps (counts, AF_M vs AF_S)."""
import os
import sys
from collections import OrderedDict, defaultdict

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "..", "mock"))
sys.path.insert(0, HERE)
import mock                                   # noqa: E402
sys.modules["mock"] = mock
import proposal_v3_2 as P                     # noqa: E402
from mock import T, AF_S, AF_M, AF_L, contrast_ratio   # noqa: E402

P1 = P.P1
SC = P.STATE["scenes"]
NAMES = {"S": AF_S, "M": AF_M, "L": AF_L}
MM = 1 / 8.0                                   # 8 px/mm
ARCMIN = 3438.0 / 500.0                        # arcmin per mm at 50 cm


def render(scene, mod=P, palette=None, state=None):
    st = dict(state if state is not None else SC[scene])
    screen, cv = mock.render_state(mod, st, palette or st.get("palette", "GREY"))
    return screen, cv


def px(screen, x, y):
    return screen.px[y * 240 + x]


# ------------------------------------------------------------------------------------------------------------------------------ geometry
def geometry():
    rows = []
    for scene in ("stress", "toast_preset_pending", "state_add_page", "row3", "map_A", "map_B"):
        render(scene)
        for e in P.GEOM:
            rows.append(dict(e, scene=scene))
    seen = OrderedDict()
    for r in rows:
        k = (r["name"], r["x"], r["y"], r["w"], r["h"])
        seen.setdefault(k, r)
    out = []
    for r in seen.values():
        kind = r["kind"]
        ix, iy = (0, 0) if kind == "toast" else (0, 1) if kind == "cue" else (1, 0) if kind == "bar" else (1, 1)
        if kind == "keybed":                                                  # black key: centred on a module line (15 px) in x, inset 1 in y
            rx, rw = (r["x"] + r["w"] // 2) % 15, r["w"] % 2
            ry, rh = (r["y"] - 1) % 4, (r["h"] + 2) % 4
        elif kind == "text" and r["name"].startswith("cue chip"):
            continue
        else:
            rx, rw = (r["x"] - ix) % 15, (r["w"] + 2 * ix) % 15
            ry, rh = (r["y"] - iy) % 4, (r["h"] + 2 * iy) % 4
        r.update(rx=rx, rw=rw, ry=ry, rh=rh, ok=int(rx == 0 and rw == 0 and ry == 0 and rh == 0))
        out.append(r)
    with open(os.path.join(HERE, "geometry_v3_2.tsv"), "w") as f:
        f.write("scene\tname\tkind\tx\ty\tw\th\tx_rem\tw_rem\ty_rem\th_rem\ton_module\n")
        for r in out:
            f.write("%s\t%s\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n" % (r["scene"], r["name"], r["kind"], r["x"], r["y"], r["w"], r["h"], r["rx"], r["rw"], r["ry"], r["rh"], r["ok"]))
    fam = OrderedDict()
    for r in out:
        n = r["name"]
        for pre in ("lane col", "page box", "page bar", "chan tile", "activity bar", "knob tile", "row cell", "map tile K", "map white key", "map black key", "map tile"):
            if n.startswith(pre):
                n = pre
                break
        fam.setdefault((n, r["kind"]), []).append(r)
    print("## [geometry]")
    print("| element family | kind | n | x | y | w | h | x rem | w rem | y rem | h rem | on module |")
    print("| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |")
    tot = ok = 0
    for (n, kind), rs in fam.items():
        r = rs[0]
        xs = sorted(set(q["x"] for q in rs))
        print("| %s | %s | %d | %s | %d | %d | %d | %s | %s | %s | %s | %s |" % (
            n, kind, len(rs), ("%d..%d" % (xs[0], xs[-1])) if len(xs) > 1 else str(xs[0]), r["y"], r["w"], r["h"],
            max(q["rx"] for q in rs), max(q["rw"] for q in rs), max(q["ry"] for q in rs), max(q["rh"] for q in rs), "yes" if all(q["ok"] for q in rs) else "NO"))
        tot += len(rs)
        ok += sum(q["ok"] for q in rs)
    print("\nelements measured: %d, on the module: %d (%d off). Module = 15 px (x) by 4 px (y); remainder columns are the worst over the instances of the family." % (tot, ok, tot - ok))
    bad = [r for r in out if not r["ok"]]
    for r in bad:
        print("- OFF MODULE: %s (%s) x=%d y=%d w=%d h=%d rem x/w/y/h = %d/%d/%d/%d" % (r["name"], r["scene"], r["x"], r["y"], r["w"], r["h"], r["rx"], r["rw"], r["ry"], r["rh"]))
    return out


# ------------------------------------------------------------------------------------------------------------------------------ text
def text_tables():
    glance = ("glance_idle", "glance_playing", "row2", "row3", "mutes", "stress", "stress_row2", "toast_dice_notes", "toast_preset_pending", "toast_undo", "toast_algo")
    roles = OrderedDict()
    per = OrderedDict()
    for sc in glance:
        del P.LOG[:]
        render(sc)
        per[sc] = list(P.LOG)
        for it in P.LOG:
            roles.setdefault((it["role"], it["f"]), []).append(it)
    print("## [text_glance]")
    print("| role | font | strings drawn (all glance scenes) | cap px | cap mm | arcmin @50 cm | worst contrast | need |  max instances in one scene |")
    print("| --- | --- | --- | --- | --- | --- | --- | --- | --- |")
    for (role, fn), its in roles.items():
        f = NAMES[fn[-1]]
        cap = f.cap_h
        strs = []
        for i in its:
            if i["s"] not in strs:
                strs.append(i["s"])
        worst = min(contrast_ratio(i["fg"], i["bg"]) for i in its)
        need = 3.0 if f is AF_L else 4.5
        mx = max(sum(1 for i in per[sc] if i["role"] == role and i["f"] == fn) for sc in per)
        print("| %s | %s | `%s` | %d | %.2f | %.1f | %.2f:1 | %.1f %s | %d |" % (role, fn, ", ".join(strs[:8]) + (" ..." if len(strs) > 8 else ""), cap, cap * MM, cap * MM * ARCMIN, worst, need, "ok" if worst >= need else "LOW", mx))
    print("\nper scene: strings drawn, by font; roles in the scene")
    print("| scene | strings | AF_L | AF_M | AF_S | distinct roles |")
    print("| --- | --- | --- | --- | --- | --- |")
    for sc, its in per.items():
        c = defaultdict(int)
        for i in its:
            c[i["f"]] += 1
        print("| %s | %d | %d | %d | %d | %d |" % (sc, len(its), c["L"], c["M"], c["S"], len(set(i["role"] for i in its))))
    allroles = set(i["role"] for sc in ("glance_playing", "row3") for i in per[sc])
    print("\nroles on glance_playing + row3 together: %d: %s" % (len(allroles), ", ".join(sorted(allroles))))
    print("\n## [text_map]")
    for sc in ("map_A", "map_B"):
        del P.LOG[:]
        render(sc)
        its = list(P.LOG)
        c = defaultdict(int)
        for i in its:
            c[i["f"]] += 1
        print("- %s: %d strings: AF_M %d, AF_S %d, AF_L %d" % (sc, len(its), c["M"], c["S"], c["L"]))
        r = OrderedDict()
        for i in its:
            r.setdefault((i["role"], i["f"]), []).append(i)
        print("| role | font | strings | cap mm | arcmin @50 cm | worst contrast | n |")
        print("| --- | --- | --- | --- | --- | --- | --- |")
        for (role, fn), ii in r.items():
            f = NAMES[fn[-1]]
            strs = []
            for i in ii:
                if i["s"] not in strs:
                    strs.append(i["s"])
            print("| %s | %s | `%s` | %.2f | %.1f | %.2f:1 | %d |" % (role, fn, ", ".join(strs[:9]) + (" ..." if len(strs) > 9 else ""), f.cap_h * MM, f.cap_h * MM * ARCMIN, min(contrast_ratio(i["fg"], i["bg"]) for i in ii), len(ii)))
        print()
    # v3.1 map for comparison
    n31 = 0
    scr, cv = render("map", P1, state=P1.STATE["scenes"]["map"])
    print("v3.1 map: %d text boxes drawn, every one AF_S except the MENU word (see v3.1 NOTES (b))." % len(scr.texts))
    # ink widths vs cell
    tight = []
    for sc in ("map_A", "map_B", "glance_playing", "stress", "row2", "row3"):
        del P.LOG[:]
        render(sc)
        for i in P.LOG:
            if i["role"] in ("map role", "map printed", "knob label", "knob value", "knob value (M)", "map knob row1", "map knob row2", "algo name", "map hold") and i["ink"] >= 56:
                tight.append((sc, i["role"], i["s"], i["ink"]))
    print("\ntightest ink widths (>= 56 px in a 58 px tile): " + "; ".join("%s %s `%s` %d" % t for t in sorted(set(tight))))


# ------------------------------------------------------------------------------------------------------------------------------ contrast
def contrast():
    print("## [contrast]")
    cols = list(P1.PAL_TOKENS.keys())
    print("Non-text pairs need 3:1; text 4.5:1 (AF_L 3:1).  head = T_PLAYHEAD, acc = T_ACCSTEP (per palette, P1.PAL_TOKENS; MONO = TEXT).\n")
    print("| pair | need | use | " + " | ".join(cols) + " |")
    print("| --- | --- | --- | " + " | ".join("---" for _ in cols) + " |")
    rows = []

    def row(name, need, use, fn):
        vals = []
        for pn in cols:
            mock.palette_set(pn)
            head, acc = (P1.PAL_TOKENS[pn][0], P1.PAL_TOKENS[pn][1]) if P1.PAL_TOKENS[pn] else (T.TEXT, T.TEXT)
            vals.append(fn(head, acc))
        rows.append((name, need, use, vals))
        print("| %s | %.1f | %s | %s |" % (name, need, use, " | ".join(("%.2f" % v) + ("" if v >= need else "!") for v in vals)))
    L, S2 = mock.ux, None
    row("head on BG", 3, "playhead column, play shape, activity bar", lambda h, a: contrast_ratio(h, T.BG))
    row("head on LANE", 3, "playhead vs lane (beat 1/3)", lambda h, a: contrast_ratio(h, T.LANE))
    row("head on LANE2", 3, "playhead vs lane (beat 2/4)", lambda h, a: contrast_ratio(h, P.lane2()))
    row("acc on LANE", 3, "accent cap, edited foot", lambda h, a: contrast_ratio(a, T.LANE))
    row("acc on LANE2", 3, "accent cap on beat 2/4", lambda h, a: contrast_ratio(a, P.lane2()))
    row("TEXT on LANE", 3, "bars, slide tie", lambda h, a: contrast_ratio(T.TEXT, T.LANE))
    row("TEXT on LANE2", 3, "bars on beat 2/4", lambda h, a: contrast_ratio(T.TEXT, P.lane2()))
    row("MID on LANE", 3, "inactive ROW cell text on BG/SURF is 4.5 elsewhere", lambda h, a: contrast_ratio(T.MID, T.LANE))
    row("muted mark on LANE", 3, "baseline, muted bar outline, cap, tie, digit, foot", lambda h, a: contrast_ratio(P.mutec(), T.LANE))
    row("MID on LANE2", 3, "baseline on beat 2/4", lambda h, a: contrast_ratio(T.MID, P.lane2()))
    row("muted mark on LANE2", 3, "muted marks on beat 2/4", lambda h, a: contrast_ratio(P.mutec(), P.lane2()))
    row("LANE2 vs LANE", 1.0, "beat shading (grouping only; not a text/shape pair)", lambda h, a: contrast_ratio(P.lane2(), T.LANE))
    row("acc vs TEXT (the v3.1 cyan whole bar vs a normal bar)", 3, "v3.1 accent bar (the 1.15:1 finding)", lambda h, a: contrast_ratio(a, T.TEXT))
    row("BG on head", 3, "marks inside the playing column (bar, cap, digit)", lambda h, a: contrast_ratio(T.BG, h))
    row("TEXT on head", 3, "slide tie inside the playing column (needs its dark outline)", lambda h, a: contrast_ratio(T.TEXT, h))
    row("TEXT on SURF", 4.5, "toast text, knob values, ROW 2 cell", lambda h, a: contrast_ratio(T.TEXT, T.SURF))
    row("MID on SURF", 4.5, "toast hint, page numerals, inactive ROW cell", lambda h, a: contrast_ratio(T.MID, T.SURF))
    row("TEXT on BG (toast rule)", 3, "toast top rule vs BG", lambda h, a: contrast_ratio(T.TEXT, T.BG))
    row("INK on TEXT", 4.5, "cue chip, toast chip, ROW live cell, inverted label band", lambda h, a: contrast_ratio(T.INK, T.TEXT))
    row("SEL vs SURF (v3.1 toast slab vs knob tile)", 3, "v3.1 slab brightness", lambda h, a: contrast_ratio(T.SEL, T.SURF))
    row("dimmed numeral on SEL", 3.0, "muted selected tile numeral (dim on purpose)", lambda h, a: contrast_ratio(P.dimink(), T.SEL))
    row("INK on SEL", 4.5, "selected tile text", lambda h, a: contrast_ratio(T.INK, T.SEL))
    row("MID on RAISE", 4.5, "none drawn at 4.5 (muted tiles are BG)", lambda h, a: contrast_ratio(T.MID, T.BG))
    row("SEL on RAISE", 3, "selected vs normal tile", lambda h, a: contrast_ratio(T.SEL, T.RAISE))
    row("DIM on BG", 1.0, "hatch of past-LENGTH steps (deliberately faint)", lambda h, a: contrast_ratio(T.DIM, T.BG))
    mock.palette_set("GREY")
    print("\nluminance contrast of the accent cap vs a plain bar in greyscale (the shape must carry it): see lane_accent_test.png; numbers:")
    for pn in ("GREY", "PAPER", "ICE"):
        mock.palette_set(pn)
        head, acc = (P1.PAL_TOKENS[pn][0], P1.PAL_TOKENS[pn][1])
        print("- %s: acc %.2f:1 on LANE, TEXT bar %.2f:1 on LANE, acc vs TEXT %.2f:1 (v3.1 bar)" % (pn, contrast_ratio(acc, T.LANE), contrast_ratio(T.TEXT, T.LANE), contrast_ratio(acc, T.TEXT)))
    mock.palette_set("GREY")
    # accent / bar sizes
    print("\naccent cap 13x8 filled triangle (area %d px, %.1f x %.1f mm), v3.1 triangle 11x6 (%d px).  Edited foot 13x5 vs baseline 13x2." % (sum(max(1, int(round((r + 0.5) * 13 / 8.0))) + 0 for r in range(8)), 13 * MM, 8 * MM, sum(max(1, int(round((r + 0.5) * 11 / 6.0))) for r in range(6))))


# ------------------------------------------------------------------------------------------------------------------------------ evidence
def lane_region(screen, x0, x1, y0=72, y1=144):
    return [px(screen, x, y) for y in range(y0, y1) for x in range(x0, x1)]


def evidence():
    print("## [evidence]")
    mock.palette_set("GREY")
    # 1 bottom band empty while playing
    s31, _ = render("glance_playing", P1, state=P1.STATE["scenes"]["glance_playing"])
    s32, _ = render("glance_playing")
    bg = P1.mock.ux.bg
    n31 = sum(1 for y in range(220, 240) for x in range(240) if px(s31, x, y) != bg)
    n32 = sum(1 for y in range(220, 240) for x in range(240) if px(s32, x, y) != bg)
    print("- bottom band y220..240 while playing: non-BG pixels v3.1 = %d, v3.2 = %d (the cue)." % (n31, n32))
    # 2 slide chain at the playhead: lane_full_playhead
    st = dict(SC["lane_full_playhead"])
    a31, _ = render("x", P1, state=dict(st, toast=None, row=0))
    a32, _ = render("lane_full_playhead")
    # v3.1 slide cap rows 14..19 of lane y0=72 (cap y 86..91); v3.2 tie rows 10..14 of lane y0=73 (y 83..87)
    def bgrun(scr, y, bgc):
        best = cur = 0
        for x in range(0, 240):
            if px(scr, x, y) == bgc:
                cur += 1
                best = max(best, cur)
            else:
                cur = 0
        return best
    print("- slide chain, 16 slides, playhead on step 8: longest run of BG-coloured pixels along the tie row: v3.1 = %d px (the dark gap at the playhead), v3.2 = %d px." % (bgrun(a31, 88, bg), bgrun(a32, 85, bg)))
    # 3 vel 1-3 vs rest under the playhead
    def vel_diff(mod, tag):
        res = []
        for vel in (1, 2, 3):
            base = dict(SC["glance_playing"], toast=None)
            lane_on = [dict(on=1, vel=vel, ed=0, acc=0, slide=0, rep=1)] + [dict(on=1, vel=60, ed=0, acc=0, slide=0, rep=1)] * 15
            lane_off = [dict(on=0, vel=1, ed=0, acc=0, slide=0, rep=1)] + lane_on[1:]
            base["channels"] = [dict(c) for c in base["channels"]]
            base["channels"][0]["p"] = 1          # playhead on step 1 (index 0)
            sa, _ = render("x", mod, state=dict(base, lane=lane_on))
            sb, _ = render("x", mod, state=dict(base, lane=lane_off))
            res.append(sum(1 for y in range(72, 144) for x in range(0, 15) if px(sa, x, y) != px(sb, x, y)))
        return res
    print("- ON step with vel 1/2/3 vs a REST, both under the playhead (first lane column): differing pixels v3.1 = %s, v3.2 = %s." % (vel_diff(P1, "v31"), vel_diff(P, "v32")))
    # 4 ratchet 5..8 vs 1 (and 4)
    def rat(mod, col_x):
        out = []
        ims = {}
        for r in range(1, 9):
            base = dict(SC["glance_playing"], toast=None)
            base["channels"] = [dict(c) for c in base["channels"]]
            base["channels"][0]["p"] = 12              # playhead far from column 0
            base["lane"] = [dict(on=1, vel=100, ed=0, acc=0, slide=0, rep=r)] + [dict(on=0, vel=1, ed=0, acc=0, slide=0, rep=1)] * 15
            sc_, _ = render("x", mod, state=base)
            ims[r] = [px(sc_, x, y) for y in range(72, 144) for x in range(0, 15)]
        def d(a, b):
            return sum(1 for i in range(len(ims[a])) if ims[a][i] != ims[b][i])
        return {"1 vs 6": d(1, 6), "1 vs 7": d(1, 7), "1 vs 8": d(1, 8), "1 vs 4": d(1, 4), "4 vs 5": d(4, 5), "4 vs 8": d(4, 8)}
    print("- ratchet counts, differing pixels in the column: v3.1 %s; v3.2 %s.  v3.2 draws the digit min(count, 4): counts above 4 do not exist in the firmware (gr bits 4..6 = 0 AUTO, 1..4; knob_map NOTE_EDIT K4 1x..4x), so 5..8 repeat the 4." % (rat(P1, 0), rat(P, 0)))
    # 5 playhead on another page
    st = dict(SC["lane_offpage"])
    b31, _ = render("x", P1, state=dict(st, toast=None))
    b32, _ = render("lane_offpage")
    head = P1.AMBER_
    am31 = sum(1 for p_ in lane_region(b31, 0, 240) if p_ == head)
    am32 = sum(1 for p_ in lane_region(b32, 0, 240) if p_ == head)
    print("- playhead on page 3 while page 2 is viewed: amber pixels in the lane: v3.1 = %d, v3.2 = %d (ghost column 13x70, 2 px frame + 5 px cap); header marker v3.1 = 4x4 px dot, v3.2 = a filled 13x14 box + under-bar on the viewed page." % (am31, am32))
    # 6 contrast of accent
    mock.palette_set("GREY")
    print("- accent: v3.1 cyan bar vs a normal bar = %.2f:1 (the whole bar, 11x34 px); v3.2 cap 13x8 vs LANE = %.2f:1 and the bars stay TEXT." % (contrast_ratio(P1.CYAN_, T.TEXT), contrast_ratio(P1.CYAN_, T.LANE)))
    # 7 toast slab vs playhead luminance
    print("- toast fill vs BG: v3.1 SEL slab %.2f:1 over 240x88 = %d px (v3.1 bottom band 238x32 = %d px); the playhead column is %.2f:1 over 13x70 = %d px, so the slab is the largest and, with 10.4 vs 11.1:1, an equally bright object; v3.2 slab SURF %.2f:1 over 240x40 with a 2 px TEXT rule (%d px of TEXT)." % (contrast_ratio(T.SEL, T.BG), 240 * 88, 238 * 32, contrast_ratio(P1.AMBER_, T.BG), 13 * 70, contrast_ratio(T.SURF, T.BG), 240 * 2))
    # v3.1 elements against the same 15 x 4 module
    v31 = (("transport shape", 1, 4, 15, 15), ("channel cell", 1, 26, 58, 44), ("lane column", 1, 72, 13, 70), ("knob cell", 1, 144, 58, 58), ("ROW half", 1, 204, 118, 16), ("page box", 177, 5, 14, 14), ("BPM text origin", 22, 6, 0, 0))
    print("- v3.1 elements against the 15 x 4 module (inset 1): " + "; ".join("%s x%d y%d w%d h%d -> rem %d/%d/%d/%d" % (n, x, y, w, h, (x - 1) % 15, (w + 2) % 15, (y - 1) % 4, (h + 2) % 4) for (n, x, y, w, h) in v31 if w))
    # 8 hot frame
    import re
    src31 = open(os.path.join(HERE, "..", "v3.1", "proposal_v3_1.py")).read()
    src32 = open(os.path.join(HERE, "proposal_v3_2.py")).read()
    print("- hot-knob frame: v3.1 draws it (`if hot:` frame in draw_knob_cell: %s); v3.2: `hot` is not read anywhere (grep S[\"hot\"] / .get(\"hot\"): %d hits)." % ("yes" if "if hot:" in src31 else "no", len(re.findall(r'\[.hot.\]|get\(.hot.\)', src32))))
    # 9 map facts
    print("- map labels: v3.1 SAVE hold label `%s` (BTN_ROWS), ROW 3 in the map: %s; v3.2 SAVE hold `UNDO L` (the button_modes.tsv label), OCT+ role `ROW+` (ROW 2 then ROW 3)." % ("BACK" if '"SAVE", "SAVE", "BACK"' in src31 else "?", "no" if "ROW 3" not in src31 else "yes"))
    print("- map black keys: v3.1 BLACK_X pitch 21/23 px (%s); v3.2 centres on the 15 px white-key pitch, group gaps %d px between groups, %d px inside a group." % (P1.BLACK_X[:4], 30 - 8, 15 - 8))
    # 10 knob labels
    print("- v3.1 knob cell `hot` frame, `OCTS` label for ARP SPAN: v3.2 label `SPAN` (firmware table); KEYS word at DENS 0 (v3.1 `KEY`).")


# ------------------------------------------------------------------------------------------------------------------------------ dice / cues / rows
def dice_table():
    print("## [dice]")
    print("| algorithm | layer | printed | toast title (AF_L tab + role) | detail (AF_M) | detail px | + `EDIT: UNDO` fits |")
    print("| --- | --- | --- | --- | --- | --- | --- |")
    for a in range(6):
        for ly in P.LAYERS:
            title, detail, changed = P.dice_text(a, ly, 0)
            wd = P.ink_w(AF_M, detail)
            fits = 3 + wd + 12 + P.ink_w(AF_M, "EDIT: UNDO") <= 237
            tw = P.ink_w(AF_L, PRINTED_PAD(ly)) + 8 + 8 + P.ink_w(AF_L, ly)
            sub = P.SUBS[a].get(ly)
            print("| %s | %s | %s | %s (title %d px) | %s%s | %d | %s |" % (P.ALGO_NAMES[a], ly, P.PRINTED[ly], title, tw, detail, "" if changed else " (NONE: layer absent)", wd, "yes" if (fits and changed) else "no"))


def PRINTED_PAD(ly):
    return P.PRINTED[ly]


def cue_table():
    print("## [cues]")
    print("| key | chip | text | total px (chip + 4 + text) | limit 240 |")
    print("| --- | --- | --- | --- | --- |")
    worst = 0
    for k in P.CUE_ORDER:
        c, t = P.CUES[k]
        if k in ("oct_up", "oct_dn"):
            continue
        w = (P.ink_w(AF_M, c) + 4 + 4 if c else 0) + P.ink_w(AF_M, t)
        worst = max(worst, w)
        print("| %s | %s | %s | %d | %s |" % (k, c or "-", t, w, "ok" if w <= 240 else "OVER"))
    print("\nOCT+ / OCT- cue (chip + the four names of the row the press goes to), every algorithm and row:")
    print("| algorithm | to ROW 1 | px | to ROW 2 | px | ROW 3 (TB-3 profile) | px |")
    print("| --- | --- | --- | --- | --- | --- | --- |")
    wmax = 0
    for a in range(6):
        cells = []
        for r in (0, 1):
            names = " ".join(P1.KNOBS[a][r * 4 + k][0].replace("OCTS", "SPAN") for k in range(4))
            w = P.ink_w(AF_M, "OCT+") + 4 + 4 + P.ink_w(AF_M, names)
            wmax = max(wmax, w)
            cells += [names, str(w)]
        r3 = " ".join(n for n, _v in P.RIG_TB3)
        w3 = P.ink_w(AF_M, "OCT+") + 8 + P.ink_w(AF_M, r3)
        wmax = max(wmax, w3)
        print("| %s | %s | %s | %s | %s | %s | %d |" % (P.ALGO_NAMES[a], cells[0], cells[1], cells[2], cells[3], r3, w3))
    print("\nwidest OCT cue = %d px of 240 (chip flush at x 0, 2 px padding each side, 4 px gap)." % wmax)


def row_names():
    print("## [rows]")
    print("The off-row names, can they live in the strip?  Strip = 238 px.  Four names at AF_M with 6 px gaps, plus a `ROW n` cell (58 px) in front:")
    print("| algorithm | row | names | px (6 px gaps) | + ROW cell 58 + gap 4 | fits 238 |")
    print("| --- | --- | --- | --- | --- | --- |")
    for a in range(6):
        for r in (0, 1):
            ns = [P1.KNOBS[a][r * 4 + k][0].replace("OCTS", "SPAN") for k in range(4)]
            w = sum(P.ink_w(AF_M, n) for n in ns) + 18
            print("| %s | %d | %s | %d | %d | %s |" % (P.ALGO_NAMES[a], r + 1, " ".join(ns), w, w + 62, "yes" if w + 62 <= 238 else "NO"))
    print("\nAligned under their knobs (one name per 58 px tile) every name fits (widest `LEGATO` %d, `OFFSET` %d px) but then the ROW cell has no room: the strip would be four names and no ROW label." % (P.ink_w(AF_M, "LEGATO"), P.ink_w(AF_M, "OFFSET")))


def centring():
    """Ink margins of text centred in its cell (ink box from the real glyphs).  Rule: horizontally floor((w - ink) / 2) left, the remainder right (so L and R differ by at most
    1 px); vertically the CAP box is centred: top = (h - cap) // 2, the odd px goes below."""
    print("## [centring]")
    print("| text | font | cell w x h | ink w | margin L / R | cap h | margin T / B |")
    print("| --- | --- | --- | --- | --- | --- | --- |")
    for s, f, w, h in (("ACCNT", AF_M, 58, 18), ("OFFSET", AF_M, 58, 18), ("12", AF_L, 58, 21), ("40", AF_L, 58, 21), ("ROW 1", AF_M, 58, 14), ("ROW 3", AF_M, 58, 14), ("3", AF_M, 13, 14),
                       ("M", AF_M, 14, 14), ("EDIT", AF_M, 0, 14), ("OCT+", AF_M, 0, 14), ("FWD", AF_M, 58, 11)):
        ink = P.ink_w(f, s)
        if w == 0:
            w = ink + 4
        left = (w - ink) // 2
        print("| `%s` | %s | %dx%d | %d | %d / %d | %d | %d / %d |" % (s, f.name, w, h, ink, left, w - ink - left, f.cap_h, (h - f.cap_h) // 2, h - f.cap_h - (h - f.cap_h) // 2))
    print("\nMeasured on the rendered screens (ink boxes of the real glyphs, `screen.texts`, against the cell they were centred in):")
    print("| scene | text | cell x,y,w,h | ink box | margin L / R | margin T / B |")
    print("| --- | --- | --- | --- | --- | --- |")
    for sc, s, (cx, cy, cw, ch_) in (("glance_playing", "ACCNT", (181, 145, 58, 18)), ("row2", "OFFSET", (1, 145, 58, 18)), ("glance_playing", "ROW 1", (1, 205, 58, 14)),
                                     ("stress", "4", (226, 5, 13, 14)), ("mutes", "M", (103, 29, 14, 14)), ("glance_playing", "ACID", (1, 25, 58, 38))):
        scr, _cv = render(sc)
        hit = [b for b in scr.texts if b[4] == s and cx - 1 <= b[0] and b[2] <= cx + cw + 1 and cy - 1 <= b[1] and b[3] <= cy + ch_ + 1]
        if hit:
            b = hit[0]
            print("| %s | `%s` | %d,%d,%d,%d | %d,%d..%d,%d | %d / %d | %d / %d |" % (sc, s, cx, cy, cw, ch_, b[0], b[1], b[2], b[3], b[0] - cx, cx + cw - b[2], b[1] - cy, cy + ch_ - b[3]))
        else:
            print("| %s | `%s` | %d,%d,%d,%d | not found | | |" % (sc, s, cx, cy, cw, ch_))
    print("(the tool's ink box includes the glyph's anti-aliasing rows, so it is 1 row taller above and below than the 11 px cap box used by the rule; the algorithm name is left-aligned in its tile on purpose.)")
    print("\nThe channel tile is a left-aligned group, not a centred cell: numeral and algorithm name share one ink-left edge (x + 4), the MIDI slot starts at x + 14 (arrow) / x + 23 (digits), the chip's right edge is x + 56.")


def main():
    geometry()
    print()
    text_tables()
    print()
    centring()
    print()
    contrast()
    print()
    evidence()
    print()
    dice_table()
    print()
    cue_table()
    print()
    row_names()


if __name__ == "__main__":
    main()
