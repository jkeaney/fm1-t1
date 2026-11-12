# SPDX-License-Identifier: GPL-3.0-only (evidence script; repo root, .venv/bin/python): 6 (h) toasts, (i) preset header, (j) clock, (k) REC / sel+muted+OFF / first-run, (l) MAP
import sys, os, copy; sys.path.insert(0, "docs/design/ui_review/critique_v3_2")
from evidence_b_lib import *
OUT = "docs/design/ui_review/critique_v3_2/edge"
mock.palette_set("GREY")
W = 240
iw = lambda s, f=AF_M: (lambda b: b[2] - b[0])(text_ink(f, s))
def knobs_for(s):
    a = s["channels"][s["sel"]]["algo"] % 6
    s["knobs"] = [(lo + hi) // 2 if en != "KEYS" else max(lo + 1, (lo + hi) // 2) for (lb, lo, hi, en) in P.P1.KNOBS[a]]
def do(name, s, save_png=True):
    if not s.get("map") and s.get("sel") is not None and "knobs" in s and not s.get("_keepknobs"): knobs_for(s)
    sc, w, recs = render(s, name)
    if save_png: save(sc, OUT, name)
    return sc, w, recs
def rightmost(recs, role_prefix="toast"):
    return max((r["ink"][2] for r in recs if r["role"].startswith(role_prefix)), default=None)
def toast(t, **kw):
    s = P.toast_scene(t, **kw); return s
print("=== (h1) dice toasts: 6 layers x 6 algorithms (P.dice_text), rendered h_dice_ALGO_LAYER")
print("| algo | layer | title (chip + role) | title right edge | detail | detail right edge | hint drawn | warnings | overlaps |\n|---|---|---|---|---|---|---|---|---|")
mx = 0
for a in range(6):
    for ly in P.LAYERS:
        s = P.scene_dice(ly, a)
        sc, w, recs = do("h_dice_%s_%s" % (P.ALGO_NAMES[a], ly), s)
        tl = [r for r in recs if r["role"] in ("toast chip", "toast title")]; dt = [r for r in recs if r["role"] == "toast detail"]; ht = [r for r in recs if r["role"] == "toast hint"]
        tr = max(r["ink"][2] for r in tl); dr = max(r["ink"][2] for r in dt)
        ov = [(x["s"], y["s"]) for x in dt for y in ht if x["ink"][2] > y["ink"][0] - 0]
        mx = max(mx, dr)
        print("| %s | %s | `%s %s` | %d | `%s` | %d | %s | %s | %s |" % (P.ALGO_NAMES[a], ly, tl[0]["s"], tl[1]["s"], tr, dt[0]["s"], dr, ("`%s` at x%d" % (ht[0]["s"], ht[0]["ink"][0])) if ht else "no", w or "none", ov or "-"))
print("widest detail right edge over 36:", mx, "(slab inner limit 237)")
print("\n=== (h2) undo / redo toast: the detail string is not specified; plausible 'what' strings from button_modes.tsv B_EDIT ('last dice, dice-all, step edit, page add, page clear, reset ... HARMONY change')")
print("| kind | what | detail right edge | hint left edge | overlap px (hint left - detail right; negative = overlap) | warnings |\n|---|---|---|---|---|---|")
WH = ["CH1 NOTES", "CH4 ACCNT", "CH1 ALL LAYERS", "CH1 STEP 12", "CH1 PAGE ADDED", "CH1 PAGE CLEARED", "CH1 RESET", "CH1 PRESET LOAD", "HARMONY: VOICE", "CH1 ALL BUT ARTIC", "CH1 NEW PITCHES", "CH4 STEP EDIT"]
for kind in ("undo", "redo"):
    for what in WH:
        sc, w, recs = do("h_%s_%s" % (kind, what.replace(" ", "_").replace(":", "")), toast(dict(kind=kind, what=what)), save_png=what in ("CH1 NOTES", "CH1 PAGE CLEARED", "CH1 ALL BUT ARTIC"))
        d = [r for r in recs if r["role"] == "toast detail"][0]; h = [r for r in recs if r["role"] == "toast hint"][0]
        print("| %s | `%s` | %d | %d | %d | %s |" % (kind, what, d["ink"][2], h["ink"][0], h["ink"][0] - d["ink"][2], w or "none"))
print("   undo/redo branch of draw_slab draws the hint unconditionally (no width test), unlike the dice branch")
print("\n=== (h3) preset toasts, P01..P16, pending/stopped/loading")
for slot in (1, 4, 9, 10, 16):
    for kind, extra in (("pending", dict(kind="preset", slot=slot, frac=0.9, when="LOADS AT BAR")), ("stopped", dict(kind="preset", slot=slot, frac=0.9, when="LOADS IN 1.5 S")), ("loading", dict(kind="loading", slot=slot))):
        s = toast(extra); s.update(preset=slot, preset_pend=(kind != "loading"), preset_load=(kind == "loading"))
        if kind == "stopped": s["playing"] = 0; s["cue"] = None
        sc, w, recs = do("h_preset_P%02d_%s" % (slot, kind), s, save_png=slot in (1, 16))
        d = [r for r in recs if r["role"].startswith("toast") ]
        bar = None
        if kind != "loading":
            # countdown bar frame: scan row ct+5 (y209) for TEXT-coloured frame start
            xs = [x for x in range(64, 240) if sc.px[209 * W + x] == T.TEXT]
            bar = (min(xs), max(xs)) if xs else None
        print("  P%02d %-8s warnings %s; toast strings %s; countdown-bar frame px extent (x) %s; header text %s" % (slot, kind, w or "none", [(r["s"], r["ink"][0], r["ink"][2]) for r in d], bar, [(r["s"], r["ink"][0], r["ink"][2]) for r in recs if r["role"] == "preset"]))
print("\n=== (h4) MIDI channel toasts: CH1..CH4 x MIDI 1..16, OFF")
rows = []
for ch in range(4):
    for m in list(range(16)) + [16]:
        title = "MIDI OFF" if m >= 16 else "MIDI %d" % (m + 1)
        what = "CH%d SENDS NOTHING" % (ch + 1) if m >= 16 else "CH%d SENDS ON MIDI CHANNEL %d" % (ch + 1, m + 1)
        rows.append((iw(what), iw(title, AF_L), ch + 1, m + 1 if m < 16 else "OFF", what))
rows.sort(reverse=True)
print("widest detail:", rows[:3], "(slab inner right limit 237 -> max ink width 234); titles widest:", sorted(rows, key=lambda r: -r[1])[:2])
s = toast(dict(kind="midi", midi=15, what="CH4 SENDS ON MIDI CHANNEL 16"), sel=3); sc, w, recs = do("h_midi_ch4_16", s); print("render h_midi_ch4_16 warnings:", w or "none", "; detail right edge", rightmost(recs))
print("  note: proposal's MIDI_TOAST and toast_midi_off use sel=0 (CH1); the toast text hardcodes the channel in `what`; no 'taken from CHn' duplicate warning is drawn.")
print("\n=== (h5) algorithm toast, six algorithms")
for a in range(6):
    s = toast(dict(kind="algo", algo=a)); s["channels"][0]["algo"] = a
    s["knobs"] = [(lo + hi) // 2 if en != "KEYS" else max(lo + 1, (lo + hi) // 2) for (lb, lo, hi, en) in P.P1.KNOBS[a]]
    sc, w, recs = do("h_algo_%s" % P.ALGO_FULL[a], s)
    t = [r for r in recs if r["role"] == "toast title"][0]
    print("  %-9s title ink %d..%d (%d px) warnings %s" % (P.ALGO_FULL[a], t["ink"][0], t["ink"][2], t["ink"][2] - t["ink"][0], w or "none"))
print("\n=== (i) header preset states and the settle bar")
for nm, kw in (("i_hdr_p16_pending", dict(preset=16, preset_pend=1)), ("i_hdr_p16_loading", dict(preset=16, preset_load=1)), ("i_hdr_p16_pending_ext240_4pages", dict(preset=16, preset_pend=1, ext=1, clock="ext", bpm=240, vpage=3)), ("i_hdr_p04_pending_toast", dict(preset=4, preset_pend=1))):
    s = scene("glance_playing"); s.update(kw)
    if "4pages" in nm: s["channels"][0].update(len=64, pg_used=4, p=20)
    if "toast" in nm: s["toast"] = dict(kind="preset", slot=4, frac=0.5, when="LOADS AT BAR")
    sc, w, recs = do(nm, s)
    hd = [(r["role"], r["s"], r["ink"][0], r["ink"][2]) for r in recs if r["ink"][1] < 24]
    bar = [x for x in range(100, 200) if sc.px[20 * W + x] in (T.TEXT, T.MID)]
    print("  %s warnings %s; header strings %s; settle bar y20 x-range %s (rows 20-21: frac drawn %s)" % (nm, w or "none", hd, (min(bar), max(bar)) if bar else None, sum(1 for x in range(100, 200) if sc.px[20 * W + x] == T.TEXT)))
print("  fraction rule in code: sv.rect(121, 20, int(34*frac) if toast has 'frac' else 17, 2, TEXT): with NO toast on screen (a knob turn dismissed it) the fill is a constant 17 of 34 px:")
s = scene("glance_playing"); s.update(preset=4, preset_pend=1); sc, w, recs = do("i_hdr_pending_no_toast", s)
print("   TEXT px in settle bar row y20 with toast absent:", sum(1 for x in range(121, 155) if sc.px[20 * W + x] == T.TEXT), "of 34")
print("\n=== (j) clock states: header strings and what changes against the internal-clock playing screen")
ref = scene("glance_playing"); scr, _, _ = do("j_ref_int", ref, save_png=False)
for nm, s in (("state_ext_sync", P.scene_state("ext_sync")), ("state_wait", P.scene_state("wait")), ("state_ext_lost", P.scene_state("ext_lost")), ("j_ext_240", dict(scene("glance_playing"), clock="ext", ext=1, bpm=240)), ("j_ext_40", dict(scene("glance_playing"), clock="ext", ext=1, bpm=40))):
    sc, w, recs = do("j_" + nm, s, save_png=True)
    diff = sum(1 for y in range(0, 24) for x in range(W) if sc.px[y * W + x] != scr.px[y * W + x])
    print("  %-16s warnings %s; header strings %s; header pixels differing from the internal-clock playing header: %d" % (nm, w or "none", [(r["s"], r["ink"][0], r["ink"][2]) for r in recs if r["role"] in ("tempo", "preset")], diff))
print("\n=== (k) REC, selected+muted+OFF, first-run")
for nm, s in (("k_rec_stopped", P.scene_state("rec_stopped")), ("k_rec_playing", P.scene_state("rec_playing")), ("k_rec_playing_ext_lost", dict(P.scene_state("rec_playing"), clock="sync", bpm=0)), ("k_sel_muted_off", P.scene_stress()), ("k_first_run", P.scene_state("first_run"))):
    sc, w, recs = do(nm, s)
    print("  %s warnings %s; transport px colours at (1..13, 5..18): %s" % (nm, w or "none", sorted({"%04X" % sc.px[y * W + x] for y in range(5, 19) for x in range(1, 14)})))
sc, w, recs = do("k_first_run_b", P.scene_state("first_run"), save_png=False)
x = 1; y = 25
print("  first-run frame is drawn at x+12..x+38, y+3..y+18 (2 px, INK on SEL). Row y+9 colours x+10..x+30 (SEL=%04X INK=%04X):" % (T.SEL, T.INK), ["%s" % ("I" if sc.px[(y + 9) * W + x + i] == T.INK else "." if sc.px[(y + 9) * W + x + i] == T.SEL else "?") for i in range(10, 31)])
print("  first-run tile 0 midi slot ink:", [(r["s"], r["ink"]) for r in recs if r["role"] == "midi slot"][:1], " numeral ink:", [(r["s"], r["ink"]) for r in recs if r["role"] == "chan numeral"][:1])
