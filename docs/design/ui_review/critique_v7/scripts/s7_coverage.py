import sys, csv, collections; sys.path.insert(0, "/home/claude/fm1/docs/design/ui_review/critique_v7/scripts")
from lib import *
D = "/home/claude/fm1/docs/design/"
def tsv(n): return [(i + 1, r) for i, r in enumerate(csv.reader(open(D + n), delimiter="\t"))][1:]
BM = [(i, r) for i, r in tsv("button_modes.tsv") if r[0] == "PERF"]
KM = [(i, r) for i, r in tsv("key_map.tsv") if r[0] == "PERF"]
NM = [(i, r) for i, r in tsv("knob_map.tsv") if r[0] == "PERF"]
out = []; P = out.append
P("# 7. Coverage of PERF controls x gestures (strict: shown in at least one of the six scenes; 'only on press' = appears only as press/turn feedback)")
rows = []   # (file, line, control, gesture, role, shown, how)
BTN_ICON = {"B_FX", "B_SCL", "B_ENV", "B_LFO", "B_EDIT", "B_GLO", "B_HOME", "B_SAVE", "B_ARP", "B_SEQ", "B_PLAY", "B_REC"}
for i, r in BM:
    ctl, g, lab = r[1], r[2], r[4]
    if g == "tap":
        if ctl in BTN_ICON: s, how = "yes", "icon in the 2x6 block"
        else: s, how = "no", "button not drawn; only its result (row marker, 3 squares) is drawn"
    elif g == "hold" and "no hold role" in r[3]:
        s, how = "n/a", "TSV: no hold role"
    else:
        s, how = "no", "hold role label (%s) is in the code tuple but not drawn" % lab
    rows.append(("button_modes", i, ctl, g, lab, s, how))
for i, r in KM:
    key, g, lab = r[1], r[2], r[4]
    if key.startswith("KEY_W"):
        rows.append(("key_map", i, key, g, lab, "yes", "caption %s; lit key = step on" % ("'KEYS = STEPS'" if g == "tap" else "'HOLD = EDIT'")))
    else:
        if lab in ("M1", "M2", "M3", "M4") and g == "tap": s, how = "yes", "square glyph (absent when channel not in use)"
        elif lab.startswith("PRV"): s, how = "no", "no preview indicator on screen"
        elif lab == "CHN": s, how = "yes", "2x2 colour-dot glyph"
        elif lab == "RIG": s, how = "no", "hold role not drawn"
        else: s, how = "yes", "glyph"
        rows.append(("key_map", i, key, g, lab, s, how))
seen = set()
for i, r in NM:
    algo, k, lab = r[1], r[2], r[3]
    ctl = k
    if algo == "*":
        if k in ("K1", "K2", "K3", "K4"): s, how = "yes", "tile row 1"
        elif k in ("SELECT",): s, how = "yes", "channel chips"
        elif k == "ALGO": s, how = "yes", "algo glyph + name (PATTERNS cut to PATTER)"
        elif k == "PRESET": s, how = "yes", "Pnn text; Pnn? frame only while pending"
        else: s, how = "no", "ROW 3 (WIRE n) not drawn; el.KN has no 3rd row (IndexError)"
        rows.append(("knob_map", i, "*/" + k, "turn", lab, s, how))
    else:
        if algo == "ACID" and k[0] == "K": s, how = "yes", "ACID tile %s in all 5 non-turning scenes" % lab
        elif algo == "ACID" and k[0] == "S": s, how = "no", "row 2 data exists in el.KN but no scene has row=1"
        elif algo == "EUCLID" and k[0] == "K": s, how = "no", "el.KN has EUCLID row 1 but no scene selects CH2 (EUCLID)"
        else: s, how = "no", "algo not in el.KN (falls back to ACID labels)" if algo != "EUCLID" else "no row 2 in el.KN (IndexError)"
        rows.append(("knob_map", i, "%s/%s" % (algo, k), "turn", lab, s, how))
def tally(rs):
    c = collections.Counter(r[5] for r in rs); return c
P("")
P("## row-level (every PERF row in the three TSVs)")
for f in ("button_modes", "key_map", "knob_map"):
    rs = [r for r in rows if r[0] == f]; c = tally(rs)
    P("- %s: %d rows: yes %d, no %d, n/a (no role) %d, only on press 0" % (f, len(rs), c["yes"], c["no"], c["n/a"]))
c = tally(rows)
tot = len(rows)
P("- ALL: %d rows: yes %d (%.1f%%), no %d, n/a %d; excluding the %d 'no hold role' rows: yes %d of %d = %.1f%%" % (tot, c["yes"], 100.0 * c["yes"] / tot, c["no"], c["n/a"], c["n/a"], c["yes"], tot - c["n/a"], 100.0 * c["yes"] / (tot - c["n/a"])))
P("")
P("## control-level (knob rows collapsed to one control per knob name: K1-4, S1-4, R1-4, SELECT, ALGO, PRESET = 15 controls x turn)")
bk = [r for r in rows if r[0] != "knob_map"]
kn = collections.OrderedDict()
for r in rows:
    if r[0] == "knob_map":
        name = r[2].split("/")[-1]
        kn.setdefault(name, []).append(r[5])
# a knob control counts as shown if its '*'/universal or ACID row is shown
knob_ctl = {}
for r in rows:
    if r[0] == "knob_map":
        name = r[2].split("/")[-1]
        if r[2].startswith("*/") : knob_ctl[name] = r[5]
for name in ("S1", "S2", "S3", "S4"):
    knob_ctl[name] = "no"
for name in ("K1", "K2", "K3", "K4", "SELECT", "ALGO", "PRESET", "R1", "R2", "R3", "R4"):
    pass
P("knob controls: %s" % dict(knob_ctl))
ck = collections.Counter(knob_ctl.values())
cb = collections.Counter(r[5] for r in bk)
ctot = len(bk) + len(knob_ctl)
cy = cb["yes"] + ck["yes"]; cn = ctot - cb["n/a"] - ck["n/a"]
P("units = %d button rows + %d key rows + %d knob controls = %d; yes = %d (%.1f%%); excluding %d 'no hold role' units: %d of %d = %.1f%%" % (len([r for r in rows if r[0] == 'button_modes']), len([r for r in rows if r[0] == 'key_map']), len(knob_ctl), ctot, cy, 100.0 * cy / ctot, cb["n/a"], cy, cn, 100.0 * cy / cn))
P("")
P("## detail (file:line control gesture role -> shown / how)")
P("| file:line | control | gesture | TSV label | shown | how |")
P("|---|---|---|---|---|---|")
for f, i, ctl, g, lab, s, how in rows:
    if f == "key_map" and ctl.startswith("KEY_W") and not ctl.endswith("F3") and not ctl.endswith("G5"): continue
    if f == "knob_map" and "/" in ctl and not ctl.startswith("*/") and ctl.split("/")[0] not in ("ACID", "EUCLID"): continue
    P("| %s:%d | %s | %s | %s | %s | %s |" % (f, i, ctl, g, lab, s, how))
P("(white keys W02..W15 and the 4 other algorithms' knob rows are collapsed in this listing; counted above)")
open("/home/claude/fm1/docs/design/ui_review/critique_v7/scripts/s7_out.md", "w").write("\n".join(out))
print("\n".join(out[:20]))
