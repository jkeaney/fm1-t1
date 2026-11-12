# SPDX-License-Identifier: GPL-3.0-only (evidence script; repo root, .venv/bin/python): WCAG ratios of every pair on all nine palettes
import sys; sys.path.insert(0, "docs/design/ui_review/critique_v3_2")
from evidence_b_lib import *
PALS = ["GREY", "GREEN", "AMBER", "ICE", "VIOLET", "ROSE", "PAPER", "NIGHT", "MONO"]
def toks(p):
    mock.palette_set(p)
    t = {k: getattr(T, k) for k in ("BG", "SURF", "TEXT", "MID", "DIM", "LINE", "SEL", "INK", "RAISE", "LANE", "KEY", "THEME", "ACCENT", "REC", "GRID", "TINT")}
    t["LANE2"] = P.lane2(); t["MUTEC"] = P.mutec(); t["DIMINK"] = P.dimink()
    t["HEAD"], t["ACC"], t["RED"] = P.pal({})
    return t
TK = {p: toks(p) for p in PALS}
# (kind, id, description, fg, bg)  kind TXT (4.5) / TXL (large text AF_L, 3.0) / NT (3.0)
PAIRS = [
 ("TXT", "bpm / preset / cue / map caption: TEXT on BG", "TEXT", "BG"),
 ("TXT", "page numeral viewed: TEXT on SURF", "TEXT", "SURF"),
 ("TXT", "page numeral other: MID on SURF", "MID", "SURF"),
 ("TXT", "page numeral on the PLAYING page: BG on HEAD", "BG", "HEAD"),
 ("TXT", "chan numeral/algo/MIDI slot unselected live: TEXT on RAISE", "TEXT", "RAISE"),
 ("TXT", "chan numeral/algo/MIDI slot selected: INK on SEL", "INK", "SEL"),
 ("TXT", "chan numeral/algo/MIDI slot muted unselected: MID on BG", "MID", "BG"),
 ("TXT", "chan numeral muted SELECTED: DIMINK on SEL", "DIMINK", "SEL"),
 ("TXT", "mute chip M unselected: INK on MID", "INK", "MID"),
 ("TXT", "mute chip M on selected tile: SEL on INK", "SEL", "INK"),
 ("TXT", "SOL-dimmed tile text: MID on BG (hatched with DIM)", "MID", "BG"),
 ("TXT", "knob label / value (M) ROW 1: TEXT on SURF", "TEXT", "SURF"),
 ("TXT", "knob label ROW 2/3 inverted band: INK on TEXT", "INK", "TEXT"),
 ("TXT", "knob value / label ROW 3 body: TEXT on RAISE", "TEXT", "RAISE"),
 ("TXT", "STAGE heard '(n)': MID on SURF", "MID", "SURF"),
 ("TXT", "ROW cell live: INK on TEXT", "INK", "TEXT"),
 ("TXT", "ROW cell not live: MID on SURF", "MID", "SURF"),
 ("TXT", "cue chip: INK on TEXT", "INK", "TEXT"),
 ("TXT", "toast: title/detail TEXT on SURF", "TEXT", "SURF"),
 ("TXT", "toast: hint MID on SURF", "MID", "SURF"),
 ("TXT", "toast: dimmed (NO LAYER) title/detail MID on SURF", "MID", "SURF"),
 ("TXT", "toast: chip INK on TEXT / dimmed chip INK on MID", "INK", "MID"),
 ("TXT", "ratchet digit unmuted: TEXT on LANE", "TEXT", "LANE"),
 ("TXT", "ratchet digit unmuted: TEXT on LANE2", "TEXT", "LANE2"),
 ("TXT", "ratchet digit muted: MUTEC on LANE", "MUTEC", "LANE"),
 ("TXT", "ratchet digit muted: MUTEC on LANE2", "MUTEC", "LANE2"),
 ("TXT", "ratchet digit in playing column: BG on HEAD", "BG", "HEAD"),
 ("TXT", "MAP printed AF_S: MID on SURF", "MID", "SURF"),
 ("TXT", "MAP printed AF_S hot: INK on SEL", "INK", "SEL"),
 ("TXT", "MAP role: TEXT on SURF", "TEXT", "SURF"),
 ("TXT", "MAP role/hold hot: INK on SEL", "INK", "SEL"),
 ("TXT", "MAP page hint / MENU word: MID on BG", "MID", "BG"),
 ("TXT", "MAP white key numerals / black key labels: TEXT on BG", "TEXT", "BG"),
 ("TXT", "MAP knob row2 not live: MID on SURF", "MID", "SURF"),
 ("TXT", "MAP key word M chip: INK on MID", "INK", "MID"),
 ("NT", "playhead column HEAD vs LANE", "HEAD", "LANE"),
 ("NT", "playhead column HEAD vs LANE2", "HEAD", "LANE2"),
 ("NT", "playhead column HEAD vs BG (gap)", "HEAD", "BG"),
 ("NT", "accent cap ACC vs LANE", "ACC", "LANE"),
 ("NT", "accent cap ACC vs LANE2", "ACC", "LANE2"),
 ("NT", "accent cap ACC vs normal bar TEXT (distinguishing colour)", "ACC", "TEXT"),
 ("NT", "normal bar TEXT vs LANE", "TEXT", "LANE"),
 ("NT", "normal bar TEXT vs LANE2", "TEXT", "LANE2"),
 ("NT", "accent mark in playing column: BG cap on HEAD", "BG", "HEAD"),
 ("NT", "normal bar in playing column: BG on HEAD", "BG", "HEAD"),
 ("NT", "accent cap vs normal bar INSIDE playing column (both BG)", "BG", "BG"),
 ("NT", "slide tie TEXT on LANE", "TEXT", "LANE"),
 ("NT", "slide tie in playing column: TEXT on HEAD", "TEXT", "HEAD"),
 ("NT", "slide tie in playing column: TEXT on BG outline (gap column x+14)", "TEXT", "BG"),
 ("NT", "slide tie outline BG on HEAD", "BG", "HEAD"),
 ("NT", "rest: baseline MUTEC vs LANE (rest = empty column)", "MUTEC", "LANE"),
 ("NT", "quiet note: bar TEXT 4 px vs baseline MUTEC (the only difference)", "TEXT", "MUTEC"),
 ("NT", "quiet note in playing column: BG bar vs HEAD", "BG", "HEAD"),
 ("NT", "edited foot ACC vs LANE", "ACC", "LANE"),
 ("NT", "edited foot vs plain baseline MUTEC", "ACC", "MUTEC"),
 ("NT", "muted lane bar outline MUTEC vs LANE", "MUTEC", "LANE"),
 ("NT", "muted lane bar outline MUTEC vs LANE2", "MUTEC", "LANE2"),
 ("NT", "muted hollow bar: outline MUTEC vs hollow interior LANE", "MUTEC", "LANE"),
 ("NT", "ghost column frame HEAD vs LANE", "HEAD", "LANE"),
 ("NT", "ghost column frame HEAD vs LANE2", "HEAD", "LANE2"),
 ("NT", "beat shading LANE vs LANE2", "LANE2", "LANE"),
 ("NT", "lane column LANE vs BG", "LANE", "BG"),
 ("NT", "lane column LANE2 vs BG", "LANE2", "BG"),
 ("NT", "past-LENGTH hatch DIM vs BG", "DIM", "BG"),
 ("NT", "unselected live tile RAISE vs BG", "RAISE", "BG"),
 ("NT", "selected tile SEL vs BG", "SEL", "BG"),
 ("NT", "selected SEL vs unselected RAISE", "SEL", "RAISE"),
 ("NT", "muted tile frame MID vs BG (the tile fill = BG)", "MID", "BG"),
 ("NT", "muted tile (BG) vs live tile (RAISE) fill", "BG", "RAISE"),
 ("NT", "muted tile frame MID vs live tile RAISE", "MID", "RAISE"),
 ("NT", "SOL-dimmed tile frame DIM vs BG", "DIM", "BG"),
 ("NT", "mute chip fill MID vs BG (unselected muted)", "MID", "BG"),
 ("NT", "mute chip fill INK vs SEL (selected muted)", "INK", "SEL"),
 ("NT", "MIDI arrow TEXT on RAISE", "TEXT", "RAISE"),
 ("NT", "MIDI arrow INK on SEL", "INK", "SEL"),
 ("NT", "MIDI arrow MID on BG (muted)", "MID", "BG"),
 ("NT", "first-run attention frame INK on SEL", "INK", "SEL"),
 ("NT", "activity bar HEAD vs BG", "HEAD", "BG"),
 ("NT", "meter fill TEXT vs track LANE", "TEXT", "LANE"),
 ("NT", "meter track LANE vs knob tile SURF", "LANE", "SURF"),
 ("NT", "meter track LANE vs ROW 3 body RAISE", "LANE", "RAISE"),
 ("NT", "meter fill TEXT vs ROW 3 RAISE", "TEXT", "RAISE"),
 ("NT", "enum pips unchosen MID vs SURF", "MID", "SURF"),
 ("NT", "knob tile SURF vs BG", "SURF", "BG"),
 ("NT", "ROW 2/3 inverted band TEXT vs SURF tile", "TEXT", "SURF"),
 ("NT", "ROW 3 body RAISE vs BG", "RAISE", "BG"),
 ("NT", "ROW 3 frame TEXT vs BG", "TEXT", "BG"),
 ("NT", "ROW cell live TEXT vs BG", "TEXT", "BG"),
 ("NT", "ROW cell not live SURF vs BG", "SURF", "BG"),
 ("NT", "page box SURF vs BG", "SURF", "BG"),
 ("NT", "page box playing HEAD vs BG", "HEAD", "BG"),
 ("NT", "page under-bar viewed TEXT vs BG", "TEXT", "BG"),
 ("NT", "transport play HEAD vs BG", "HEAD", "BG"),
 ("NT", "transport stop MID vs BG", "MID", "BG"),
 ("NT", "REC disc RED vs BG", "RED", "BG"),
 ("NT", "REC ring (stopped) RED vs BG", "RED", "BG"),
 ("NT", "toast slab SURF vs BG", "SURF", "BG"),
 ("NT", "toast slab SURF vs knob tile SURF (it overlays tiles' bottoms)", "SURF", "SURF"),
 ("NT", "toast top rule TEXT vs SURF", "TEXT", "SURF"),
 ("NT", "toast slab SURF vs playhead HEAD brightness (HEAD vs SURF)", "HEAD", "SURF"),
 ("NT", "toast algo unchosen pips MID vs SURF", "MID", "SURF"),
 ("NT", "toast countdown bar: fill TEXT vs frame/track TEXT (frame only) on SURF", "TEXT", "SURF"),
 ("NT", "settle bar under P: TEXT vs MID track", "TEXT", "MID"),
 ("NT", "settle bar track MID vs BG", "MID", "BG"),
 ("NT", "MAP hold box outline MID vs SURF", "MID", "SURF"),
 ("NT", "MAP hot tile SEL vs SURF", "SEL", "SURF"),
 ("NT", "MAP black key KEY vs BG", "KEY", "BG"),
 ("NT", "MAP leader line MID vs BG", "MID", "BG"),
 ("NT", "MAP white key tile LANE vs BG", "LANE", "BG"),
 ("NT", "MAP white key tile LANE2 vs BG", "LANE2", "BG"),
 ("NT", "MAP header rule LINE vs BG", "LINE", "BG"),
 ("NT", "MAP countdown fill TEXT vs frame TEXT on BG", "TEXT", "BG"),
 ("NT", "cue chip INK/TEXT tab vs BG: TEXT vs BG", "TEXT", "BG"),
 ("NT", "FIL up-triangle INK on TEXT band", "INK", "TEXT"),
]
res = {}
for kind, d, a, b in PAIRS:
    res[d] = (kind, {p: cr(TK[p][a], TK[p][b]) for p in PALS}, a, b)
print("tokens per palette (RGB565 hex):")
for p in PALS:
    print(p, " ".join("%s=%04X" % (k, v) for k, v in TK[p].items()))
def mark(kind, v):
    need = 4.5 if kind == "TXT" else 3.0
    return "%.2f%s" % (v, "!" if v < need else "")
for title, kinds in (("TEXT pairs (need 4.5:1; AF_L-only text needs 3:1; '!' = under 4.5)", ("TXT",)), ("NON-TEXT pairs (need 3:1; '!' = under)", ("NT",))):
    print("\n### " + title)
    print("| # | pair | fg / bg | " + " | ".join(PALS) + " |\n|---|---|---|" + "---|" * len(PALS))
    i = 0
    for d, (kind, v, a, b) in res.items():
        if kind not in kinds: continue
        i += 1
        print("| %d | %s | %s / %s | %s |" % (i, d, a, b, " | ".join(mark(kind, v[p]) for p in PALS)))
print("\n### UNDER the threshold, per palette")
for p in PALS:
    t = [d + " (%.2f)" % v[p] for d, (kind, v, a, b) in res.items() if kind == "TXT" and v[p] < 4.5]
    n = [d + " (%.2f)" % v[p] for d, (kind, v, a, b) in res.items() if kind == "NT" and v[p] < 3.0]
    print("**%s** text under 4.5: %d; non-text under 3: %d" % (p, len(t), len(n)))
    print("  text: " + "; ".join(t)); print("  non-text: " + "; ".join(n))
