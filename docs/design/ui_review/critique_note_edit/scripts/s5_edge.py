# SPDX-License-Identifier: GPL-3.0-only (run from repo root: .venv/bin/python <this>)
"""Edge cases from the generated text table ui_ne_text (build/gen/ui_tables.h) and the strings ui_sq.c builds from it: widths vs slots, worst cases, truncation."""
import sys, csv; sys.path.insert(0, "docs/design/ui_review/critique_note_edit/scripts")
from ne_lib import *
rows = list(csv.DictReader(open(os.path.join(DATA, "widths.tsv")), delimiter="\t"))
def W(r, k): return int(r[k])
print("slots (from ui_sq.c uisq_draw_notetiles / uisq_ne_status_layout / uisq_draw_keybed): tile 58 px wide at x=1+60k; value font AF_L when text_w(AF_L,t) <= 54 else AF_M (no further fallback; no ellipsis call on the page);")
print("label AF_M centred in 58 px; GEN line AF_S centred in 58 px; chips AF_S + 4 px pad + 2 px gap, row must end <= 237 else the legend is dropped (chips themselves are never dropped); footer AF_M left at x=2 and right-aligned to x=238.\n")
print("### ui_ne_text, every string (generated table), advance px in each font")
print("| id | string | AF_S | AF_M | AF_L | used as |\n|---|---|---|---|---|---|")
use = {"AUTO": "tile value AF_M (AUTO is never set: AF_M always)", "ACC": "VELO value (AF_L if fits else AF_M) and GEN line", "TIE": "GATE value and GEN line", "X": "RATCH suffix (2X)", "GEN": "GEN line prefix + space", "STEP": "status line AF_S", "ON": "chip", "OFF": "chip", "SLIDE": "chip", "NO SLIDE": "chip", "PROB": "chip prefix (+ ' 50')", "OCT": "chip prefix (+ sign + digit)", "+": "chip sign", "-": "chip / note-name sign", "= SET BY YOU": "legend (when room)", "KNOBS EDIT": "footer left AF_M", "HOME = DONE": "footer right AF_M"}
for r in [r for r in rows if r["kind"] == "ne_text"]:
    print("| %s | `%s` | %d | %d | %d | %s |" % (r["value"], r["string"], W(r, "wS"), W(r, "wM"), W(r, "wL"), use.get(r["string"], "note-name part" if len(r["string"]) <= 2 and r["string"][0] in "ABCDEFG" else "")))
print("\n### knob labels (AF_M in a 58 px tile)")
for r in [r for r in rows if r["kind"] == "label"]: print("- K%d `%s` AF_M %d px (slot 58, spare %d)" % (int(r["value"]) + 1, r["string"], W(r, "wM"), 58 - W(r, "wM")))
def sec(kind, title, fontrule):
    R = [r for r in rows if r["kind"] == kind]
    print("\n### %s (%d values)" % (title, len(R)))
    wm = max(R, key=lambda r: W(r, "wM")); wl = max(R, key=lambda r: W(r, "wL")); wg = max(R, key=lambda r: W(r, "genS"))
    print("widest AF_M value: `%s` (%d px); widest AF_L value: `%s` (%d px); widest GEN line (AF_S): `%s` (%d px, slot 58)" % (wm["string"], W(wm, "wM"), wl["string"], W(wl, "wL"), wg["genline"], W(wg, "genS")))
    L = [r for r in R if W(r, "wL") <= 54]; M = [r for r in R if W(r, "wL") > 54]
    print("set value drawn in AF_L (wL <= 54): %d values; in AF_M (wL > 54): %d values%s" % (len(L), len(M), (": " + ", ".join(sorted({"`%s`(L %d)" % (r["string"], W(r, "wL")) for r in M})) ) if M else ""))
    over = [r for r in R if W(r, "genS") > 58 or W(r, "wM") > 58]
    print("values over a slot of 58 px (AF_M value or GEN line): %s" % (", ".join("`%s`" % r["string"] for r in over) or "none"))
    return R
P = sec("pitch", "PITCH, MIDI notes 0..127 (note name + octave n/12-1; the knob range is 36 + KEY + 12*OCTAVE up to +96 semitones; every one of 128 is printable)", None)
neg = [r for r in P if int(r["value"]) < 12]
print("negative-octave names (notes 0..11): " + ", ".join("`%s` M%d L%d genS%d" % (r["string"], W(r, "wM"), W(r, "wL"), W(r, "genS")) for r in neg))
print("note names by length: " + "; ".join("%d chars: %d names" % (n, sum(1 for r in P if len(r["string"]) == n)) for n in sorted({len(r['string']) for r in P})))
fonts = collections.Counter("L" if W(r, "wL") <= 54 else "M" for r in P)
print("font chosen across the 128: %s" % dict(fonts))
print("AF_L widths: min %d max %d; AF_M widths: min %d max %d; GEN-line (AF_S) widths: min %d max %d" % (min(W(r, 'wL') for r in P), max(W(r, 'wL') for r in P), min(W(r, 'wM') for r in P), max(W(r, 'wM') for r in P), min(W(r, 'genS') for r in P), max(W(r, 'genS') for r in P)))
for lo, hi, lab in ((0, 11, "octave -1 (notes 0-11)"), (12, 23, "octave 0"), (24, 119, "octaves 1-8"), (120, 127, "octave 9")):
    S = [r for r in P if lo <= int(r["value"]) <= hi]
    print("  %s: AF_L fits %d of %d; AF_M widths %d..%d; GEN-line widths %d..%d" % (lab, sum(1 for r in S if W(r, "wL") <= 54), len(S), min(W(r, "wM") for r in S), max(W(r, "wM") for r in S), min(W(r, "genS") for r in S), max(W(r, "genS") for r in S)))
sec("gate", "GATE field 1..11 (10..100 %, 11 = TIE)", None)
sec("velo", "VELO 1..127 (>= 100 reads ACC)", None)
sec("ratch", "RATCH 1..4", None)
G = [r for r in rows if r["kind"] == "gate"]
print("\nGATE: " + ", ".join("`%s` L%d" % (r["string"], W(r, "wL")) for r in G))
V = [r for r in rows if r["kind"] == "velo"]
print("VELO AF_L widths: 1-digit %d..%d, 2-digit %d..%d, ACC (L) %d" % (min(W(r, "wL") for r in V if len(r["string"]) == 1), max(W(r, "wL") for r in V if len(r["string"]) == 1), min(W(r, "wL") for r in V if len(r["string"]) == 2), max(W(r, "wL") for r in V if len(r["string"]) == 2), [W(r, "wL") for r in V if r["string"] == "ACC"][0]))
print("\n### status line: STEP n widths and chip row ends (the firmware's formula; the test asserts <= 237 for STEP 64 + four widest chips)")
for n in (1, 9, 10, 64):
    print("- `STEP` AF_S %d + gap 3 + `%d` AF_M %d + 5 = chips start x=%d" % (text_w(AF_S, "STEP"), n, text_w(AF_M, str(n)), 3 + text_w(AF_S, "STEP") + 3 + text_w(AF_M, str(n)) + 5))
chipw = {c: text_w(AF_S, c) + 4 + 2 for c in ("ON", "OFF", "SLIDE", "NO SLIDE", "PROB 50", "OCT+1", "OCT-2", "OCT+2")}
print("chip pitch (text + 4 pad + 2 gap): " + ", ".join("%s %d" % kv for kv in chipw.items()))
print("legend `= SET BY YOU` AF_S advance %d + 8 + square; drawn only when chips_end + 8 + %d <= 237, i.e. chips_end <= %d" % (text_w(AF_S, "= SET BY YOU"), text_w(AF_S, "= SET BY YOU"), 237 - 8 - text_w(AF_S, "= SET BY YOU")))
for step in (9, 64):
    x0 = 3 + text_w(AF_S, "STEP") + 3 + text_w(AF_M, str(step)) + 5
    print("  STEP %d: chips start x=%d; chip width (pitch) that can precede the legend = %d px" % (step, x0, 237 - 8 - text_w(AF_S, "= SET BY YOU") - x0))
    # which sets of chips fit the legend
import itertools
fields = [("ON/OFF", ["ON", "OFF"]), ("SLIDE", ["SLIDE", "NO SLIDE"]), ("PROB", ["PROB 50"]), ("OCT", ["OCT+1", "OCT-2", "OCT+2"])]
x0 = 3 + text_w(AF_S, "STEP") + 3 + text_w(AF_M, "9") + 5
print("\nchip-field combinations (each field absent or one of its values) for STEP 9, count that keep the legend / that overflow 237:")
keep = drop = over = 0; worst = 0
for combo in itertools.product(*[[None] + v for _, v in fields]):
    x = x0 + sum(chipw[c] for c in combo if c); worst = max(worst, x)
    if x <= 237 - 8 - text_w(AF_S, "= SET BY YOU"): keep += 1
    else: drop += 1
    if x > 237: over += 1
print("  %d combinations: legend shown for %d, dropped for %d; widest chip row ends x=%d; rows ending past 237: %d" % (keep + drop, keep, drop, worst, over))
print("\n### footer: `KNOBS EDIT` AF_M %d at x=2 (ends %d), `HOME = DONE` AF_M %d right-aligned to 238 (starts %d): gap between them %d px" % (text_w(AF_M, "KNOBS EDIT"), 2 + text_w(AF_M, "KNOBS EDIT"), text_w(AF_M, "HOME = DONE"), 238 - text_w(AF_M, "HOME = DONE"), 238 - text_w(AF_M, "HOME = DONE") - 2 - text_w(AF_M, "KNOBS EDIT")))
print("\n### truncation / ellipsis flags in the op logs (ink flag bit 0 = text_fit ellipsised; bit 1 = cut by the strip canvas, which is expected for a text lying wholly in the other strip)")
for n in RENDERS + EXTRA:
    fl = collections.Counter()
    for l in open(os.path.join(DATA, n + ".ops")):
        p = l.rstrip("\n").split("\t")
        if p[0] == "T": fl[int(p[12]) & 1] += 1
    print("- %s: ellipsised texts %d of %d draws" % (n, fl[1], sum(fl.values())))
