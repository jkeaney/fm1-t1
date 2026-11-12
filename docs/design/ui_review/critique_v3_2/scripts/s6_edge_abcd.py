# SPDX-License-Identifier: GPL-3.0-only (evidence script; repo root, .venv/bin/python): 6 (a) all muted, (b) full lane, (c) empty, (d) off-page playhead
import sys, os, copy; sys.path.insert(0, "docs/design/ui_review/critique_v3_2")
from evidence_b_lib import *
OUT = "docs/design/ui_review/critique_v3_2/edge"
mock.palette_set("GREY")
W = 240
def knobs_for(s):
    a = s["channels"][s["sel"]]["algo"] % 6
    s["knobs"] = [(lo + hi) // 2 if en != "KEYS" else max(lo + 1, (lo + hi) // 2) for (lb, lo, hi, en) in P.P1.KNOBS[a]]
def do(name, s, pal=None):
    knobs_for(s)
    if pal: s["palette"] = pal
    sc, w, recs = render(s, name)
    save(sc, OUT, name)
    print("RENDER %-34s tool warnings: %s" % (name, w or "none"))
    return sc, recs
def vprofile(sc, x, y0=73, y1=143):
    """runs of colour along one column of the lane: [(first row rel, last rel, hex)]"""
    out = []; st = None; cur = None
    for y in range(y0, y1):
        c = sc.px[y * W + x]
        if c != cur:
            if cur is not None: out.append((st - y0, y - 1 - y0, "%04X" % cur))
            st, cur = y, c
    out.append((st - y0, y1 - 1 - y0, "%04X" % cur)); return out
def hrun(sc, y, x0, x1, col):
    best = cur = 0
    for x in range(x0, x1):
        cur = cur + 1 if sc.px[y * W + x] == col else 0; best = max(best, cur)
    return best
print("=== (a) all four channels muted")
for nm, sel, offs in (("a_all_muted_sel1", 0, False), ("a_all_muted_sel4", 3, False), ("a_all_muted_all_off_sel2", 1, True)):
    s = scene("glance_playing"); s["sel"] = sel
    for c in s["channels"]:
        c["muted"] = 1
        if offs: c["midi_ch"] = 16
    sc, recs = do(nm, s)
    act = sum(1 for x in range(W) for y in range(64, 68) if sc.px[y * W + x] == P.pal({})[0])
    print("  %s: HEAD px in activity row y64..67: %d; M chips drawn: %d; texts: %d; header HEAD px (y0..24): %d" % (nm, act, sum(1 for r in recs if r["role"] == "mute chip"), len(recs), sum(1 for x in range(W) for y in range(0, 24) if sc.px[y * W + x] == P.pal({})[0])))
    mm = recs and [r for r in recs if r["role"] in ("chan numeral", "algo name")]
    print("  numerals/names contrast (fg/bg):", sorted({(round(cr(r["fg"], r["bg"]), 2)) for r in mm}))
base = scene("glance_playing")
print("  reference unmuted glance_playing: HEAD px in activity row:", sum(1 for x in range(W) for y in range(64, 68) if render(base, "r")[0].px[y * W + x] == P.pal({})[0]))
print("\n=== (b) 16 steps ON, accent+slide+ratchet 4 on every step, playhead on step 1, 8, 16")
res = {}
for nm, col in (("b_full_ph1", 0), ("b_full_ph8", 7), ("b_full_ph16", 15)):
    s = P.scene_lane_full(col)
    sc, recs = do(nm, s); res[nm] = (sc, col)
    head = P.pal({})[0]
    tr = LANE_TIE = 73 + 12
    longest_bg = hrun(sc, tr, 0, 240, T.BG)
    print("  %s playhead col %d: longest BG run on the slide-tie centre row y%d = %d px; tie colour run count: %d" % (nm, col + 1, tr, longest_bg, sum(1 for x in range(1, 240) if sc.px[tr * W + x] == T.TEXT and sc.px[tr * W + x - 1] != T.TEXT)))
    print("   column profile, playing col x=%d (rel rows, colour): %s" % (15 * col + 1 + 6, vprofile(sc, 15 * col + 7)))
    other = 15 * (col + 1 if col < 15 else 0) + 7
    print("   column profile, other col x=%d: %s" % (other, vprofile(sc, other)))
    print("   accent-cap pixels (rel row 1..8, centre col) colour in playing col:", ["%04X" % sc.px[(73 + 2) * W + 15 * col + 7]], " normal bar pixel (rel 40):", "%04X" % sc.px[(73 + 40) * W + 15 * col + 7])
    # digit region vs tie outline: ink box of digit
    d = [r for r in recs if r["role"] == "ratchet digit"]
    print("   ratchet digit ink boxes (first 2):", [r["ink"] for r in d[:2]], "count", len(d))
    # tie right end in col 16
    print("   row y%d from x=221..239 colours:" % tr, ["%04X" % sc.px[tr * W + x] for x in range(221, 240)])
print("\n=== (c) no steps ON")
for nm, play in (("c_empty_playing", 1), ("c_empty_stopped", 0)):
    s = scene("glance_playing") if play else P.scene_idle(); s["lane"] = P.lane_of([(0, 1, 0, 0, 0, 1)] * 16); s["playing"] = play
    s["cue"] = "idle_playing" if play else "idle_stopped"
    sc, recs = do(nm, s)
    cols = {}
    for y in range(73, 143):
        for x in range(240):
            cols[sc.px[y * W + x]] = cols.get(sc.px[y * W + x], 0) + 1
    print("  %s: lane pixel colours (hex:count): %s; texts in lane rows (y72..143): %s" % (nm, {"%04X" % k: v for k, v in sorted(cols.items(), key=lambda kv: -kv[1])}, [r["s"] for r in recs if 72 <= r["ink"][1] < 144]))
print("\n=== (d) playhead on a page that is not viewed (len 64)")
for nm, vp, ph in (("d_view_p1_play_p2", 0, 20), ("d_view_p1_play_p4", 0, 60), ("d_view_p4_play_p1", 3, 4), ("d_view_p2_play_p2_ref", 1, 20), ("d_view_p3_play_p4_step64", 2, 63)):
    s = scene("glance_playing"); s["vpage"] = vp
    s["channels"][0].update(len=64, pg_used=4, p=ph + 1)
    sc, recs = do(nm, s)
    head = P.pal({})[0]
    lane_head = sum(1 for y in range(72, 144) for x in range(W) if sc.px[y * W + x] == head)
    box = [(x, sc.px[8 * W + x]) for x in (181, 196, 211, 226)]
    print("  %s: viewed page %d, playing step %d (page %d): HEAD px in lane %d; header boxes (x: colour at y8): %s; viewed under-bars at x: %s" % (nm, vp + 1, ph + 1, ph // 16 + 1, lane_head, ["%d:%04X" % b for b in box], [x for x in (181, 196, 211, 226) if sc.px[21 * W + x] == T.TEXT]))
    gx = 15 * (ph % 16) + 1
    print("    ghost column x=%d: row73 %04X, row 75 %04X inner px(x+3,y100) %04X; lane step label drawn? " % (gx, sc.px[73 * W + gx], sc.px[75 * W + gx], sc.px[100 * W + gx + 3]))
# the lane of the viewed page: dark-on-lane under which step? is there any step number shown? (texts in lane rows)
print("  digits/text inside lane rows in d_view_p1_play_p4:", [r["s"] for r in do("d_tmp", (lambda s: (s["channels"][0].update(len=64, pg_used=4, p=61), s)[1])(scene("glance_playing")))[1] if 72 <= r["ink"][1] < 144])
