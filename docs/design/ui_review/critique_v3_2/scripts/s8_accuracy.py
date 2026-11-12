# SPDX-License-Identifier: GPL-3.0-only (evidence script; repo root, .venv/bin/python): v3.2 labels / ranges / strings vs docs/design/*.tsv
import sys, re; sys.path.insert(0, "docs/design/ui_review/critique_v3_2")
from evidence_b_lib import *
P1 = P.P1
L = lambda p: [l.rstrip("\n").split("\t") for l in open("docs/design/" + p)]
kmap = L("knob_map.tsv"); bm = L("button_modes.tsv"); km = L("key_map.tsv"); cm = L("control_map.tsv")
print("## 8.1 knob labels and ranges, proposal P1.KNOBS (as used by v3.2 knob_view, OCTS->SPAN) vs docs/design/knob_map.tsv PERF rows")
ALGN = ("ACID", "EUCLID", "SNAKE", "STAGE", "ARP", "PATTERNS")
mism = []
for ln, r in enumerate(kmap, 1):
    if r[0] != "PERF" or r[1] not in ALGN: continue
    a = ALGN.index(r[1]); kk = r[2]; idx = ("K1", "K2", "K3", "K4", "S1", "S2", "S3", "S4").index(kk)
    lab, lo, hi, en = P1.KNOBS[a][idx]
    lab = lab.replace("OCTS", "SPAN")
    rng = r[5]
    m = re.search(r"\b(\d+)\.\.(\d+)", rng)
    okl = lab == r[3]
    okr = True
    if m and en not in ("PICK",):
        okr = (int(m.group(1)), int(m.group(2))) == (lo, hi)
    if not (okl and okr): mism.append((ln, r[1], kk, "proposal %s %d..%d %s" % (lab, lo, hi, en), "spec %s: %s" % (r[3], rng[:70])))
print("rows compared: %d; label/range mismatches: %d" % (sum(1 for r in kmap if r[0] == "PERF" and r[1] in ALGN), len(mism)))
for m in mism: print("  knob_map.tsv:%d %s %s | %s | %s" % m)
print("\n## 8.2 value text the spec prescribes vs what v3.2 draws")
def cell(a, idx, v):
    s = scene("glance_playing"); s["channels"][0]["algo"] = a; s["knobs"] = [lo for (lb, lo, hi, en) in P1.KNOBS[a]]; s["knobs"][idx] = v; s["row"] = idx // 4
    return P.knob_view(s, 0, idx % 4)
print("ACID DENS=0 ->", cell(0, 0, 0)[:3], "(spec knob_map.tsv:2 'cell value reads KEYS')")
print("ARP SPAN=1..4 ->", [cell(4, 2, v)[1] for v in (1, 2, 3, 4)], "(spec knob_map.tsv:36 'cell reads 1 OCT..4 OCT')")
print("ARP RATE ->", [cell(4, 3, v)[1] for v in range(6)], "(spec knob_map.tsv:37 '1/4 1/2 1X 2X 3T 4X')")
print("ARP DIR ->", [cell(4, 7, v)[1] for v in range(6)], "(spec knob_map.tsv:41 'UP DOWN UPDN RAND ASPL CHORD')")
print("ARP SHAPE ->", [cell(4, 1, v)[1] for v in range(6)], "(spec knob_map.tsv:35 'LINE SKIP THUMB OCTV REPT WAVE')")
print("EUCLID CURVE ->", [cell(1, 7, v)[1] for v in range(7)], "(spec knob_map.tsv:17 'AUTO SINE TRI SAW+ SAW- ARCH INV')")
print("PATTERNS PICK=1,21,41,61,81,101 -> label/value", [cell(5, 4, v)[:2] for v in (1, 21, 41, 61, 81, 101)], "(spec knob_map.tsv:46 'the cell reads CAT nn, for example HOUSE 3')")
print("STAGE TOTAL=48 with heard=32: tile draws the value", cell(3, 1, 48)[1], "and the meter slot text '(32)' separately (spec knob_map.tsv:27 'value reads 48 (32)')")
print("\n## 8.3 MAP button labels vs button_modes.tsv labels (tap / hold) and control_map.tsv")
tsv = {}
for ln, r in enumerate(bm, 1):
    if r[0] == "PERF": tsv[(r[1], r[2])] = (ln, r[4])
for (pr, tap, hold), ctl in zip(P.MAP_BTN, P.MAP_BTN_CTL):
    t = tsv.get((ctl, "tap")); h = tsv.get((ctl, "hold"))
    hs = h[1] if h and "no hold role" not in [r for r in bm if r[0] == "PERF" and r[1] == ctl and r[2] == "hold"][0][3] else ""
    flag = []
    if t and t[1] != tap: flag.append("TAP v3.2 `%s` vs tsv `%s` (button_modes.tsv:%d)" % (tap, t[1], t[0]))
    if hold != hs: flag.append("HOLD v3.2 `%s` vs tsv `%s` (button_modes.tsv:%d)" % (hold, hs, h[0] if h else 0))
    print("  %-7s %-6s tap `%s` hold `%s`  %s" % (ctl, pr, tap, hold, ("MISMATCH: " + "; ".join(flag)) if flag else "match"))
print("  OCT-: v3.2 ROW 1 vs tsv", tsv[("B_OCTDN", "tap")], "; OCT+: v3.2 `ROW+` vs tsv", tsv[("B_OCTUP", "tap")])
print("\n## 8.4 MAP encoder tiles vs control_map.tsv")
for ln, r in enumerate(cm, 1):
    if r[0] in ("EN_SELECT", "EN_ALGO", "EN_PRESET", "MASTER") and r[1] == "PERF": print("  control_map.tsv:%d %s label `%s`" % (ln, r[0], r[3]))
print("  v3.2 MAP_ENC =", P.MAP_ENC)
print("\n## 8.5 MAP page B black keys vs key_map.tsv")
for ln, r in enumerate(km, 1):
    if r[0] == "PERF" and r[1].startswith("KEY_B"): print("  key_map.tsv:%d %s %s -> label `%s`" % (ln, r[1], r[2], r[4]))
print("  v3.2 MAP_BLACK (tap, hold) =", P.MAP_BLACK)
print("\n## 8.6 cue / toast strings against the spec text")
chk = [
 ("cue dice_*", "`EDIT = UNDO`", "button_modes.tsv B_EDIT tap: UNDO / REDO the last dice ... of the selected channel"),
 ("cue save_hold", P.CUES["save_hold"], "button_modes.tsv B_SAVE hold: 0.7 s: undo the last preset load ... again = redo; while a slot is pending: cancel it instead"),
 ("cue edit_hold", P.CUES["edit_hold"], "button_modes.tsv B_EDIT hold: 0.4 s: open RESET (latched, one shot): the next control touched goes back to its default"),
 ("cue oct_peek", P.CUES["oct_peek"], "button_modes.tsv B_OCTUP hold: hold and turn a knob: ROW 2 live while held (cue text names OCT+ only; OCT- hold peek has no cue)"),
 ("cue chn", P.CUES["chn"], "key_map.tsv KEY_B05 tap: open the CHAN page (LENGTH MIDI SOURCE FOLLOW)"),
 ("cue gloset", P.CUES["gloset"], "button_modes.tsv B_GLO tap: open SET (latched): BPM SWING GATE DIV"),
 ("cue home", P.CUES["home"], "button_modes.tsv B_HOME tap: only cancels a pending prompt / skips first-run; hold: HELP / MENU"),
 ("cue mute_hold", P.CUES["mute_hold"], "key_map.tsv KEY_B0n hold 0.4 s: PREVIEW on/off of CHn (label PRVn)"),
 ("cue fill", P.CUES["fill"], "key_map.tsv KEY_B07 hold: the selected channel's K1 density goes up while held"),
 ("cue roll", P.CUES["roll"], "key_map.tsv KEY_B08 hold: every step repeats (force: 2x light, 3x medium, 4x firm)"),
 ("cue restart", P.CUES["restart"], "key_map.tsv KEY_B11 tap: all channels to step 1 at the next step"),
 ("cue past_end", P.CUES["past_end"], "key_map.tsv KEY_W tap: past LENGTH: dim, toast PAST THE END"),
 ("cue pg_next", P.CUES["pg_next"], "key_map.tsv KEY_B10 tap: view the next page (FOLLOW off); on the last page: ADD PAGE prompt"),
]
for a_, b_, c_ in chk: print("  %-14s %-52s | %s" % (a_, b_, c_))
