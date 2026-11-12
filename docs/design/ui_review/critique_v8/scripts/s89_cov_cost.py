import sys, os, csv, math
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from common import *
import el
palette_set("HI-CON")
D = ROOT + "/docs/design/"
def tsv(n): return [r for r in csv.reader(open(D + n, encoding="utf-8"), delimiter="\t", quotechar=None)][1:]
BM = [r for r in tsv("button_modes.tsv") if r[0] == "PERF"]
KM = [r for r in tsv("key_map.tsv") if r[0] == "PERF"]
KN = [r for r in tsv("knob_map.tsv") if r[0] == "PERF"]
rows = []   # (control, gesture, status, note, has_action)
btn_name = {"B_FX": "FX", "B_SCL": "SCL", "B_ENV": "ENV", "B_LFO": "LFO", "B_EDIT": "EDIT", "B_GLO": "GLO", "B_HOME": "HOME", "B_SAVE": "SAVE", "B_ARP": "ARP", "B_SEQ": "SEQ", "B_PLAY": "PLAY", "B_REC": "REC"}
for r in BM:
    b, g, label, act = r[1], r[2], r[4], r[3]
    has = not act.startswith("no hold role")
    if b in btn_name:
        if g == "tap": st, note = "yes", "icon on its tile (role name not drawn)"
        elif g == "hold" and has: st, note = "partial", "corner notch shows a hold exists; role %r not named" % label
        else: st, note = "n/a", "no hold role"
        if b == "B_REC" and g == "tap": note = "icon; ring idle / disc+red when armed (rec_armed scene)"
        if b == "B_PLAY": note = "icon; header play/stop icon shows state"
    else:
        if g == "tap": st, note = "partial", "ROW markers show the live row; the OCT button and its role are not drawn"
        elif g == "hold": st, note = "no", "peek hold not shown"
        else: st, note = "no", "hold-both UPDATE (hidden maintenance)"
    rows.append((b, g, st, note, has or g == "tap"))
for r in KM:
    k, g, label = r[1], r[2], r[4]
    if k.startswith("KEY_W"):
        rows.append((k, g, "yes", "footer 'KEYS = STEPS' / 'HOLD = EDIT'; lane steps lit on the key; playhead bar", True))
    elif label.startswith("M") and g == "tap": rows.append((k, g, "yes", "channel-colour square / X glyph (muted) on the black key", True))
    elif label.startswith("PRV"): rows.append((k, g, "no", "preview hold: nothing drawn", True))
    elif label == "CHN": rows.append((k, g, "yes", "DIN glyph (no text)", True))
    elif label == "RIG": rows.append((k, g, "no", "hold RIG: nothing drawn", True))
    elif label in ("SOL", "FIL", "ROL"): rows.append((k, g, "yes", "letter S/F/R glyph", True))
    else: rows.append((k, g, "yes", "arrow / reset glyph", True))
for r in KN:
    ctx, kn, label = r[1], r[2], r[3]
    if ctx == "*":
        if kn in ("K1", "K2", "K3", "K4"): rows.append(("knob " + kn + " (generic)", "turn", "yes", "tile label/value/fill for the live row", True))
        elif kn == "SELECT": rows.append(("SELECT", "turn", "yes", "channel chip highlight", True))
        elif kn == "ALGO": rows.append(("ALGO", "turn", "yes", "algorithm name in header (ALGO_PICK overlay not drawn)", True))
        elif kn == "PRESET": rows.append(("PRESET", "turn", "yes", "header Pnn; the pending 'Pnn?' state is drawable but in no scene", True))
        else: rows.append(("knob " + kn + " (RIG ROW 3)", "turn", "no", "ROW 3 not drawn; row=2 raises IndexError", True))
    else:
        rows.append(("%s %s %s" % (ctx, kn, label), "turn", "yes" if kn[0] == "K" else "only-after-OCT+", "ROW 1 tile" if kn[0] == "K" else "ROW 2 tile: drawn only when the OCT+ row is latched (sheet scenes); the default view shows ROW 1", True))
from collections import Counter
def summ(sel, title):
    c = Counter(x[2] for x in sel); n = len(sel)
    print("%-52s n=%3d  " % (title, n) + "  ".join("%s=%d (%.0f%%)" % (k, v, 100 * v / n) for k, v in sorted(c.items(), key=lambda kv: -kv[1])))
print("COVERAGE of PERF control x gesture rows (button_modes 30 + key_map 48 + knob_map 59 = %d)" % len(rows))
print("status: yes = a persistent glyph/text/state on the default main screen identifies the control+gesture; partial = existence indicated, role/name not; only-after-OCT+ = shown only once the other knob row is latched; no = not drawn in any scene; n/a = row says no hold role")
summ(rows, "ALL rows")
summ([x for x in rows if x[2] != "n/a"], "rows with an action (n/a removed)")
for nm, sel in (("buttons", [x for x in rows if x[0].startswith("B_")]), ("keys", [x for x in rows if x[0].startswith("KEY_")]), ("knobs", [x for x in rows if not x[0].startswith(("B_", "KEY_"))])):
    summ(sel, nm)
print("\n'only on press' rows (visible only in a transient scene): none by status; transient-only features: dice toast (1 scene), turning frame + old-value tick (turning_k2), lit tile (dice scene), REC mode glyph swap (rec_armed), pending preset (no scene)")
print("\nROWS WITH STATUS no:")
for x in rows:
    if x[2] == "no": print("   %-32s %-9s %s" % (x[0], x[1], x[3]))
print("\nROWS WITH STATUS partial:")
for x in rows:
    if x[2] == "partial": print("   %-32s %-9s %s" % (x[0], x[1], x[3]))
print("\nROWS only-after-OCT+:", [x[0] for x in rows if x[2] == "only-after-OCT+"])
print("\n=== 9. FIRMWARE COST ===")
S = scenes(); allart = {}
for n, st in S.items():
    ART.clear(); render(st)
    for a in ART: allart.setdefault(a["name"], (a["w"], a["h"]))
def by(w, h): return (math.ceil(w / 8) * h, math.ceil(w * h / 8))
print("distinct icon bitmaps across all 15 scenes: %d" % len(allart))
tot_row = tot_pack = 0
for k, (w, h) in sorted(allart.items()):
    r_, p_ = by(w, h); tot_row += r_; tot_pack += p_
    print("   %-12s %2dx%-2d  row-padded %2d B  packed %2d B" % (k, w, h, r_, p_))
print("   TOTAL row-padded 1 bpp: %d B ; bit-packed: %d B" % (tot_row, tot_pack))
main = {}
for n in ["playing","two_channels","one_channel","sel_muted","dice","turning_k2","rec_armed","midi_off"]:
    ART.clear(); render(S[n])
    for a in ART: main[a["name"]] = (a["w"], a["h"])
print("main 8 scenes: %d distinct bitmaps, row-padded %d B, packed %d B" % (len(main), sum(by(*v)[0] for v in main.values()), sum(by(*v)[1] for v in main.values())))
print("also drawn from rects, not bitmaps: header REC disc (3 rects 9x9, 11x7, 7x11), hold notch (4 one-px lines), hatch, bars")
print("v7/v5 icon dictionaries not drawn by v8 are not counted. Note 'rec' (armed) is the v7 icon: size", allart.get("rec"))
# colours
pal = {name: getattr(T, name) for name in ("BG","SURF","TEXT","THEME","ACCENT","MID","DIM","LINE","SEL","TINT","INK","REC","RAISE","KEY","LANE","GRID")}
palset = set(pal.values())
print("\nHI-CON palette token values (RGB565):", {k: "0x%04X" % v for k, v in pal.items()})
consts = {"CH1": mod.CH[0], "CH2": mod.CH[1], "CH3": mod.CH[2], "CH4": mod.CH[3], "WHITE": mod.WHITE, "PHC": mod.PHC, "DICE_BG": mod.DICE_BG, "DICE_FG": mod.DICE_FG}
for i in range(4): consts["knob fill CH%d (mix 120/255)" % (i + 1)] = el.mix(mod.CH[i], 120)
print("\nv8-defined colours NOT in the HI-CON token set:")
for k, v in consts.items():
    print("   %-28s 0x%04X  %s  %s" % (k, v, hexc(v), "(in palette)" if v in palset else "NOT in palette"))
# all pixel colours
seen = Counter()
for n, st in S.items():
    sc, cv, rec = render(st); seen.update(sc.px)
others = {c: n for c, n in seen.items() if c not in palset and c not in consts.values()}
# AA ramps: blends of any (fg,bg) pair from text records
ramp_cols = set()
for n in ["playing","two_channels","one_channel","sel_muted","dice","turning_k2","rec_armed","midi_off","euclid_midi16","arp","demo_algo"]:
    sc, cv, rec = render(S[n])
    for r in rec: ramp_cols.update(mock.ramp(r["fg"], r["bg"]) if hasattr(mock, "ramp") else [])
    # background under text can differ: also ramps against SURF and fill colours
    for r in rec:
        for b in list(pal.values()) + list(consts.values()): ramp_cols.update(mock.ramp(r["fg"], b))
unexplained = {c: n for c, n in others.items() if c not in ramp_cols}
print("\ndistinct pixel colours over 15 scenes: %d ; palette tokens used: %d ; v8 constants/fills: %d ; AA text-ramp blends (tool-generated): %d ; unexplained: %d" % (len(seen), len([c for c in seen if c in palset]), len([c for c in seen if c in consts.values() and c not in palset]), len([c for c in others if c in ramp_cols]), len(unexplained)))
print("palette tokens NOT found in any pixel:", [k for k, v in pal.items() if v not in seen])
print("\nRGB565 list of colours outside the HI-CON token set that v8 introduces (non-AA):", sorted({"0x%04X" % v for v in consts.values() if v not in palset}))
print("   count:", len({v for v in consts.values() if v not in palset}), "of which compile-time constants:", len({v for k, v in consts.items() if not k.startswith('knob') and v not in palset}), "; runtime mixes (knob fills):", len({v for k, v in consts.items() if k.startswith('knob') and v not in palset}))
print("   note: T.MID, T.DIM, T.RAISE, T.KEY, T.SURF are derived tokens of HI-CON (counted in palette).")
