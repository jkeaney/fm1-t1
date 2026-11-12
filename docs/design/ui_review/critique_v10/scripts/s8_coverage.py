from common import *
import csv
DES = os.path.abspath(os.path.join(UI, ".."))
def tsv(n):
    with open(os.path.join(DES, n), encoding="utf-8") as f: return list(csv.DictReader(f, delimiter="\t", quoting=csv.QUOTE_NONE))
BM, KM, KEYM = tsv("button_modes.tsv"), tsv("knob_map.tsv"), tsv("key_map.tsv")
Y, T_, N, NA = "yes", "touch/press/hold only", "no", "n/a (no action)"
rows = []   # (source, control, gesture, class, evidence)
# buttons
cls = {("B_FX", "tap"): (Y, "dice_all icon"), ("B_SCL", "tap"): (Y, "dice_notes icon (algorithm sub-label under it: no)"), ("B_ENV", "tap"): (Y, "dice_accnt icon (dimmed if absent: DEMO only)"),
       ("B_LFO", "tap"): (Y, "dice_shape icon"), ("B_ARP", "tap"): (Y, "dice_artic icon"), ("B_SEQ", "tap"): (Y, "dice_hits icon"),
       ("B_EDIT", "tap"): (Y, "undo icon; toast EDIT=UNDO after a dice"), ("B_EDIT", "hold"): (T_, "hold hint HOLD EDIT: RESET"),
       ("B_GLO", "tap"): (Y, "settings icon"), ("B_GLO", "hold"): (T_, "hold hint HOLD GLO: HARMONY"),
       ("B_HOME", "tap"): (Y, "home icon"), ("B_HOME", "hold"): (T_, "hold hint TOUCH=HELP LET GO=MENU"),
       ("B_SAVE", "tap"): (Y, "save icon"), ("B_SAVE", "hold"): (T_, "hold hint HOLD SAVE: UNDO LOAD"),
       ("B_PLAY", "tap"): (Y, "play icon + header play/stop mark"), ("B_REC", "tap"): (Y, "rec ring icon; REC state in header + button"),
       ("B_OCTUP", "tap"): (T_, "no OCT+ button on screen; row markers show the row (persistent) but the action/label ROW n appears only as the OCT+ toast"),
       ("B_OCTDN", "tap"): (N, "no OCT- toast scene; row markers only show the current row"),
       ("B_OCTUP", "hold"): (N, "peek not shown"), ("B_OCTDN", "hold"): (N, "peek not shown"),
       ("B_OCTUP", "hold-both"): (N, "update mode not shown"), ("B_OCTDN", "hold-both"): (N, "update mode not shown")}
for r in BM:
    if r["page"] != "PERF": continue
    k = (r["control"], r["mode"])
    if r["mode"] == "hold" and ("no hold role" in r["action"] or "act on press" in r["action"] or r["action"].startswith("press")):
        rows.append(("button_modes", r["control"], r["mode"], NA, r["action"][:50])); continue
    c, e = cls.get(k, (N, "?"))
    rows.append(("button_modes", r["control"], r["mode"], c, e))
# keys
for r in KEYM:
    if r["page"] != "PERF": continue
    k = r["key"]; g = r["gesture"]
    if k.startswith("KEY_W"):
        rows.append(("key_map", k, g, Y if g == "tap" else Y, "bar/key lane block shows step on/off" if g == "tap" else "message line 'HOLD = EDIT' (static text)"))
    else:
        lab = r["label"]
        if lab in ("M1", "M2", "M3", "M4") and g == "tap": rows.append(("key_map", k, g, Y, "black-key block / x7 glyph (blank for an unused channel)"))
        elif lab.startswith("PRV"): rows.append(("key_map", k, g, N, "PREVIEW hold not shown"))
        elif lab == "CHN": rows.append(("key_map", k, g, Y, "DIN glyph"))
        elif lab == "RIG": rows.append(("key_map", k, g, N, "RIG hold not shown"))
        else: rows.append(("key_map", k, g, Y, "glyph/letter %s" % lab))
# knobs
for r in KM:
    if r["page"] != "PERF": continue
    if r["context"] == "*":
        k = r["knob"]
        ev = {"K1": "tile 1", "K2": "tile 2", "K3": "tile 3", "K4": "tile 4", "SELECT": "channel chips + selection bar (+ toast)", "ALGO": "algorithm name in header (+ toast)", "PRESET": "P01 text / P02? pill",
              "R1": "tile WIRE1 (generic label)", "R2": "tile WIRE2 (generic label)", "R3": "tile WIRE3 (generic label)", "R4": "tile WIRE4 (generic label)"}[k]
        rows.append(("knob_map", "*/" + k, "turn", Y, ev))
    else:
        rows.append(("knob_map", r["context"] + "/" + r["knob"], "turn", Y, "label + value tile via algos_v10 (TSV-parsed)"))
from collections import Counter
def summ(sel, title):
    sel = list(sel); c = Counter(r[3] for r in sel); n = len(sel)
    den = n - c[NA]
    print("%s: %d rows; yes %d, touch/press/hold-only %d, no %d, n/a (no hold role) %d -> coverage 'yes' = %d/%d = %.0f%%; yes + touch = %d/%d = %.0f%%" % (title, n, c[Y], c[T_], c[N], c[NA], c[Y], den, 100.0 * c[Y] / den, c[Y] + c[T_], den, 100.0 * (c[Y] + c[T_]) / den))
print("# 8 coverage of PERF control x gesture rows (classification stated per row below)")
print("classes: yes = a persistent element on the PERF screen identifies it; touch/press/hold-only = appears only while touching/pressing/holding (toast, hold hint, lit tile); no = nowhere; n/a = TSV says no hold role / act on press (excluded from the denominator)")
summ(rows, "ALL (buttons + keys + knobs)")
summ([r for r in rows if r[0] == "button_modes"], "buttons (button_modes PERF)")
summ([r for r in rows if r[0] == "key_map"], "keys (key_map PERF)")
summ([r for r in rows if r[0] == "knob_map"], "knobs (knob_map PERF, incl. 48 per-algorithm K/S rows)")
summ([r for r in rows if r[0] == "knob_map" and r[1].startswith("*/")], "knobs, '*' rows only (K1-4, R1-4, SELECT, ALGO, PRESET)")
print("\nrows classified 'no':")
for r in rows:
    if r[3] == N: print("  -", r[0], r[1], r[2], "|", r[4])
print("\nrows classified 'touch/press/hold only':")
for r in rows:
    if r[3] == T_: print("  -", r[0], r[1], r[2], "|", r[4])
print("\nfull row table: %d rows" % len(rows))
print("| source | control | gesture | class | evidence |"); print("|---|---|---|---|---|")
for r in rows:
    if r[1].count("/") and r[0] == "knob_map" and not r[1].startswith("*/"): continue   # 48 per-algorithm rows summarised
    print("| %s | %s | %s | %s | %s |" % r)
print("| knob_map | 48 per-algorithm K/S rows | turn | yes | tiles ROW 1 / ROW 2 |")
