# SPDX-License-Identifier: GPL-3.0-only (evidence script; repo root, .venv/bin/python): 6(g) the cue string of every control; 6 map header help lines
import sys, os, csv, itertools; sys.path.insert(0, "docs/design/ui_review/critique_v3_2")
from evidence_b_lib import *
mock.palette_set("GREY")
iw = lambda s, f=AF_M: (lambda b: b[2] - b[0])(text_ink(f, s))
def total(chip, text):
    cw = iw(chip) + 4 if chip else 0
    return (cw + 4 + iw(text)) if chip else iw(text), cw
print("## A. every CUES entry (chip + text) as drawn by draw_cue: width = chip block + 4 + text ink; limit 240; the mock has NO ellipsis path for cues (text_on), over-width text is cut by the canvas edge / assertion")
print("| key | chip | text | chip block px | text ink px | total px | spare px (240 - total) |  in CUE_ORDER (rendered in cue_table_1..3) |\n|---|---|---|---|---|---|---|---|")
for k, (chip, text) in P.CUES.items():
    if k in ("oct_up", "oct_dn"): continue
    t, cw = total(chip, text)
    print("| %s | `%s` | `%s` | %d | %d | %d | %d | %s |" % (k, chip, text, cw, iw(text), t, 240 - t, "yes" if k in P.CUE_ORDER else "NO"))
print("\n## B. OCT+/OCT- cue for every algorithm and row (cue_oct), channel with / without a device")
worst = []
for a in range(6):
    for rig in (0, 1):
        for row in (0, 1, 2):
            for up in (True, False):
                S = dict(channels=[dict(algo=a, rig=rig)] * 4, row=row, sel=0)
                chip, text = P.cue_oct(S, 0, up)
                t, cw = total(chip, text)
                worst.append((t, P.ALGO_NAMES[a], rig, row + 1, "OCT+" if up else "OCT-", chip, text))
worst.sort(reverse=True)
print("combinations: %d; widest 8 (total px, algo, rig, current row, button, chip, text):" % len(worst))
for w in worst[:8]: print("  ", w)
print("over 240:", [w for w in worst if w[0] > 240] or "none")
print("OCT- from ROW 1 goes to ROW 1 (r = 0): cue names the ROW 1 names even when ROW 1 is already live: example", [w for w in worst if w[4] == "OCT-" and w[3] == 1][:1])
print("\n## C. strings with a variable part, worst case")
digits = [(iw(str(d)), d) for d in range(10)]
print("digit ink widths (AF_M):", digits, " advance:", [text_w(AF_M, str(d)) for d in range(10)])
var = []
for ch in range(1, 5):
    var += [("mute", "M%d" % ch, "CH%d MUTED  TAP AGAIN = ON" % ch), ("unmute", "M%d" % ch, "CH%d ON  TAP AGAIN = MUTE" % ch), ("mute_hold", "M%d" % ch, "HOLD = PREVIEW CH%d" % ch), ("solo", "SOL", "ONLY CH%d SOUNDS" % ch)]
for pg in range(1, 5):
    var += [("pg_next", "PG+", "PAGE %d OF 4  FOLLOW OFF" % pg), ("pg_add", "PG+", "AGAIN = ADD PAGE %d" % pg), ("follow_off", "PG-", "VIEW %d OF 4  FOLLOW OFF" % pg)]
for p_ in range(1, 17): var.append(("save_tap", "SAVE", "SAVED TO P%02d" % p_))
for b in (40, 99, 100, 120, 126, 188, 240): var.append(("ext_lost", "", "CLOCK LOST: OWN TEMPO %d" % b))
var += [("past_end", "", "PAST THE END: CHN K1 = LENGTH"), ("first_run", "CHN", "K2 = MIDI CHANNEL"), ("gloset", "GLO", "SET: BPM SWING GATE DIV"), ("edit_hold", "EDIT", "RESET: TOUCH A CONTROL")]
rows = sorted(((total(c, t)[0], k, c, t) for k, c, t in var), reverse=True)
print("widest variable-part cues:", rows[:6]); print("over 240:", [r for r in rows if r[0] > 240] or "none")
print("per key worst:", sorted({(k): max(total(c, t)[0] for kk, cc, t in [(0,0,0)] for _ in [0]) for k, c, t in []}.items()) if False else "")
bykey = {}
for k, c, t in var: bykey[k] = max(bykey.get(k, 0), total(c, t)[0])
print("worst total per key:", bykey)
print("\n## D. coverage: controls of docs/design/button_modes.tsv PERF and key_map.tsv PERF vs a cue entry in proposal_v3_2.CUES")
bm = [l.rstrip("\n").split("\t") for l in open("docs/design/button_modes.tsv")][1:]
km = [l.rstrip("\n").split("\t") for l in open("docs/design/key_map.tsv")][1:]
CUEFOR = {  # (control, gesture) -> CUES key(s) in the proposal that speak for it (my reading of CUES + draw_cue)
 ("B_FX", "tap"): "dice_all", ("B_SCL", "tap"): "dice_notes", ("B_ENV", "tap"): "dice_accnt", ("B_LFO", "tap"): "dice_shape", ("B_ARP", "tap"): "dice_artic", ("B_SEQ", "tap"): "dice_hits",
 ("B_EDIT", "tap"): "undo/redo", ("B_EDIT", "hold"): "edit_hold", ("B_GLO", "tap"): "gloset", ("B_HOME", "tap"): "home", ("B_SAVE", "tap"): "save_tap", ("B_SAVE", "hold"): "save_hold",
 ("B_PLAY", "tap"): "play", ("B_REC", "tap"): "rec_stopped/rec_playing", ("B_OCTDN", "tap"): "oct_dn", ("B_OCTUP", "tap"): "oct_up", ("B_OCTDN", "hold"): "oct_peek (OCT+ text only)", ("B_OCTUP", "hold"): "oct_peek",
}
print("| control | gesture | spec label | spec action (start) | cue in CUES | cue text |\n|---|---|---|---|---|---|")
miss = []
for r in bm:
    if r[0] != "PERF": continue
    ctl, g, act, lab = r[1], r[2], r[3], r[4]
    if g == "hold" and "no hold role" in act: continue
    key = CUEFOR.get((ctl, g))
    txt = ""
    if key and key.split("/")[0].split(" ")[0] in P.CUES: txt = " / ".join("%s %s" % P.CUES[x.split(" ")[0]] if x.split(" ")[0] in P.CUES else x for x in key.split("/"))
    if not key: miss.append((ctl, g))
    print("| %s | %s | %s | %s | %s | %s |" % (ctl, g, lab, act[:70], key or "**none**", txt))
KEYCUE = {"M": "mute/unmute/mute_hold (drawn for M2 only)", "SOL": "solo", "FIL": "fill", "ROL": "roll", "PG-": "follow_off", "PG+": "pg_next/pg_add", "RST": "restart", "CHN": "chn"}
for r in km:
    if r[0] != "PERF" or "KEY_W" in r[1] and r[1][5:7] != "01": continue
    ctl, g, lab, act = r[1], r[2], r[4], r[3]
    key = None
    if lab.startswith("M") and lab[1:].isdigit(): key = "mute/unmute"
    elif lab.startswith("PRV"): key = "mute_hold ('HOLD = PREVIEW CHn')"
    elif lab == "CHN": key = "chn"
    elif lab == "RIG": key = None
    elif lab in ("SOL", "FIL", "ROL", "PG-", "PG+", "RST"): key = {"SOL": "solo", "FIL": "fill", "ROL": "roll", "PG-": "follow_off", "PG+": "pg_next", "RST": "restart"}[lab]
    elif lab in ("STEP", "EDIT"): key = None
    if key is None: miss.append((ctl, g, lab))
    print("| %s | %s | %s | %s | %s | %s |" % (ctl, g, lab, act[:70], key or "**none**", ""))
print("\ncontrol/gesture rows with NO cue entry:", miss)
print("\n## E. MAP header help line for every PERF control in help_text.tsv: line1 in AF_M vs 236 px; fall-back font AF_S")
ht = [l.rstrip("\n").split("\t") for l in open("docs/design/help_text.tsv")][1:]
over = []; n = 0
for c in ht:
    if c[1] != "PERF": continue
    n += 1
    w = iw(c[2]); ws = iw(c[2], AF_S)
    if w > 236: over.append((c[0], c[2], w, ws))
print("PERF help lines: %d; line1 wider than 236 px in AF_M (these fall back to AF_S in map_header): %d" % (n, len(over)))
for o in over: print("  ", o, "AF_S ink >236:", o[3] > 236)
print("line2 is never drawn by map_header (only c[2] = line1 is returned by help_line). PERF rows with a non-empty line2:", sum(1 for c in ht if c[1] == "PERF" and len(c) > 3 and c[3]))
