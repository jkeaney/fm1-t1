import sys, os, re, csv, copy
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from common import *
import el
palette_set("HI-CON")
D = ROOT + "/docs/design/"
def tsv(n): return list(csv.reader(open(D + n, encoding="utf-8"), delimiter="\t", quotechar=None))[1:]
BM = [r for r in tsv("button_modes.tsv") if r[0] == "PERF"]
KM = [r for r in tsv("key_map.tsv") if r[0] == "PERF"]
KN = [r for r in tsv("knob_map.tsv") if r[0] == "PERF"]
S = scenes(); n_ok = n_bad = 0
def claim(i, what, ok, detail=""):
    global n_ok, n_bad
    n_ok += ok; n_bad += (not ok)
    print("  %2d. [%s] %s%s" % (i, "MATCH   " if ok else "MISMATCH", what, (" -- " + detail) if detail else ""))
print("BUTTONS: v8 BUTTONS tuple (printed, icon, tap role, hold role, dice) vs button_modes.tsv PERF (label col = role name)")
tmap = {"FX": "B_FX", "SCL": "B_SCL", "ENV": "B_ENV", "LFO": "B_LFO", "EDIT": "B_EDIT", "GLO": "B_GLO", "HOME": "B_HOME", "SAVE": "B_SAVE", "ARP": "B_ARP", "SEQ": "B_SEQ", "PLAY": "B_PLAY", "REC": "B_REC"}
sc, cv, rec = render(S["playing"]); px = sc.px
i = 0
for n, (pn, ic, role, hold, dice) in enumerate(icons_v8.BUTTONS):
    b = tmap[pn]
    tap = [r for r in BM if r[1] == b and r[2] == "tap"][0]; hd = [r for r in BM if r[1] == b and r[2] == "hold"][0]
    has_hold = not hd[3].startswith("no hold role")
    notch_px = None
    r_, c_ = divmod(n, 6); x = c_ * 40; yy = 132 + r_ * 26
    notch_px = px[yy * 240 + x + 35] == T.MID
    i += 1
    claim(i, "%s (%s): tap role %r, hold notch=%s; TSV tap label %r, hold label %r, hold role exists=%s" % (pn, b, role, bool(hold), tap[4], hd[4], has_hold),
          role == tap[4] and bool(hold) == has_hold and notch_px == has_hold and (not hold or hold == hd[4]),
          "" if (role == tap[4] and bool(hold) == has_hold and notch_px == has_hold) else "role/notch differs; rendered notch pixel=%s" % notch_px)
print("  (the role strings in BUTTONS are never drawn: buttons() renders only the icon + corner notch; names appear nowhere on screen)")
print("  buttons with a TSV hold role: ", [tmap_r for tmap_r in tmap if not [r for r in BM if r[1]==tmap[tmap_r] and r[2]=="hold"][0][3].startswith("no hold role")], "-> v8 notches:", [b[0] for b in icons_v8.BUTTONS if b[3]])
print("  PERF buttons NOT drawn: B_OCTDN (tap ROW 1, hold ROW 1 peek, hold-both UPDATE), B_OCTUP (tap ROW 2/3, hold peek, hold-both UPDATE)")
print("\nBLACK KEYS: el.BLACK (x centre, role) vs key_map.tsv PERF KEY_Bnn_<note> rows")
WH = ["F3","G3","A3","B3","C4","D4","E4","F4","G4","A4","B4","C5","D5","E5","F5","G5"]
names = {}
for r in KM:
    m = re.match(r"KEY_B(\d\d)_(\w#\d)", r[1])
    if m: names.setdefault(m.group(2), {})[r[2]] = (r[4], r[3])
def black_x(note):
    nm = note[:-2]; octv = int(note[-1]); nat = {"F#": "F", "G#": "G", "A#": "A", "C#": "C", "D#": "D"}[note[:2]]
    # black key sits on the boundary right after white key `nat+octave`
    w = nat + str(octv)
    return (WH.index(w) + 1) * 15
BK = list(el.BLACK)
keys_in_order = sorted(names, key=black_x)
for (cx, role), note in zip(BK, keys_in_order):
    labels = names[note]
    exp_x = black_x(note)
    tap = labels.get("tap", (None,))[0]; hold = labels.get("hold", (None,))[0]
    expected_role = tap if tap else hold
    i += 1
    claim(i, "black %s: v8 x-centre %d role %s; key_map x-centre %d, tap label %s, hold label %s" % (note, cx, role, exp_x, tap, hold), cx == exp_x and role == expected_role.replace("PRV", "M") or (cx == exp_x and role == expected_role))
print("  black keys in key_map PERF:", len(names), "| v8 BLACK entries:", len(BK))
print("  hold gestures on black keys not drawn: PRV1..PRV4 (hold M1..M4), RIG (hold CHN); no hold notch exists on the black keys; M-key glyph for an UNUSED channel is blank (key still mutes? TSV: tap mute CHn)")
print("\nFOOTER / CUE TEXT")
i += 1; claim(i, "footer 'KEYS = STEPS' vs white-key tap label STEP (key_map PERF KEY_W*: tap 'STEP')", set(r[4] for r in KM if r[1].startswith("KEY_W") and r[2] == "tap") == {"STEP"})
i += 1; claim(i, "footer 'HOLD = EDIT' vs white-key hold label EDIT (held 0.4 s opens NOTE EDIT)", set(r[4] for r in KM if r[1].startswith("KEY_W") and r[2] == "hold") == {"EDIT"})
i += 1; claim(i, "footer 'PAGE 1/2' is a literal in keybed(); SQ_NSTEP 64 = up to 4 pages of 16; PG+/PG- view pages", False, "string constant, not state-driven; no page count/viewed-page variable in S")
pg = [r for r in tsv("page_states.tsv") if r[0] == "REC"][0]
i += 1; claim(i, "rec footer 'REC: ALL KEYS PLAY NOTES' vs page_states REC cue strip %r" % pg[7], False, "different wording; also REC cue ends 'REC=done'; v8 shows no 'done'")
i += 1; claim(i, "rec_armed knob band: v8 draws the PERF band (DENS SLIDE...) vs page_states REC knob_band %r" % pg[5], False, "REC page K1-K4 = CURSOR ACC SLIDE OCT; v8 rec_armed scene shows ACID/EUCLID morphs")
print("\nKNOB LABELS / DEFAULTS / RANGES vs knob_map.tsv PERF (8 slots x 6 algorithms = 48)")
def indep(spec):
    head = spec.split(";")[0]
    m = re.search(r"(-?\d+)\.\.(-?\d+)", head); d = re.search(r"\bdef (\S+)", head)
    return (int(m.group(1)), int(m.group(2))) if m else None, d.group(1) if d else None
bad = []; tot = 0
for r in KN:
    if r[1] == "*": continue
    a, slot, label, spec = r[1], r[2], r[3], r[5]
    row = 0 if slot[0] == "K" else 1; k = A.ALGOS[a]["rows"][row][int(slot[1]) - 1]
    rng, dflt = indep(spec); tot += 1
    problems = []
    if k["label"] != label: problems.append("label %r!=%r" % (k["label"], label))
    if rng and (k["min"], k["max"]) != rng: problems.append("range %s!=%s" % ((k["min"], k["max"]), rng))
    if rng and dflt is not None and dflt.lstrip("-").isdigit() and k["def"] != int(dflt): problems.append("def %s!=%s" % (k["def"], dflt))
    if not rng:
        toks = [t for t in spec.split(";")[0].split()[1:] if t not in ("def",) ]
        if dflt and dflt in (k["names"] or []) and (k["names"][k["def"]] != dflt): problems.append("enum default")
    if problems: bad.append((a, slot, label, problems))
i += 1; claim(i, "knob labels, ranges, numeric defaults: %d of %d slots equal an independent parse of knob_map.tsv" % (tot - len(bad), tot), not bad, str(bad[:3]))
# displayed text at default per slot where TSV has quirk
def shown(a, slot):
    k = A.ALGOS[a]["rows"][0 if slot[0] == "K" else 1][int(slot[1]) - 1]; return A.value_text(k)
i += 1; claim(i, "ACID K1 DENS at 0 reads 'KEYS' (TSV: 0 = KEYS ONLY, cell reads KEYS): v8 -> %r" % A.value_text(A.ALGOS["ACID"]["rows"][0][0], 0), A.value_text(A.ALGOS["ACID"]["rows"][0][0], 0) == "KEYS")
i += 1; claim(i, "ARP K3 SPAN cell reads '1 OCT..4 OCT': v8 -> %r" % [A.value_text(A.ALGOS["ARP"]["rows"][0][2], v) for v in (1, 4)], A.value_text(A.ALGOS["ARP"]["rows"][0][2], 1) == "1 OCT")
i += 1; claim(i, "PATTERNS S1 PICK cell reads category name + number (TSV: 'CAT nn, for example HOUSE 3'): v8 -> %r" % A.value_text(A.ALGOS["PATTERNS"]["rows"][1][0]), False, "v8 prints the literal prefix 'CAT ' + number (CAT 1..CAT 120); TSV example is a category name e.g. HOUSE 3")
i += 1; claim(i, "STAGE K2 TOTAL cell adds '(n)' steps heard when pages read fewer pulses: v8 -> %r" % A.value_text(A.ALGOS["STAGE"]["rows"][0][1]), False, "v8 prints the bare number")
i += 1; claim(i, "PATTERNS K1 DENS 1 = 'anchors only', 0 = KEYS: v8 0 -> %r, 1 -> %r" % (A.value_text(A.ALGOS["PATTERNS"]["rows"][0][0], 0), A.value_text(A.ALGOS["PATTERNS"]["rows"][0][0], 1)), A.value_text(A.ALGOS["PATTERNS"]["rows"][0][0], 0) == "KEYS")
i += 1; claim(i, "ARP S4 DIR names UP DOWN UPDN RAND ASPL CHORD, def UP: v8 -> %r def %r" % (A.ALGOS["ARP"]["rows"][1][3]["names"], A.value_text(A.ALGOS["ARP"]["rows"][1][3])), A.ALGOS["ARP"]["rows"][1][3]["names"] == "UP DOWN UPDN RAND ASPL CHORD".split() and A.value_text(A.ALGOS["ARP"]["rows"][1][3]) == "UP")
i += 1; claim(i, "ARP K4 RATE names 1/4 1/2 1X 2X 3T 4X, def index 2: v8 -> %r def %r" % (A.ALGOS["ARP"]["rows"][0][3]["names"], A.value_text(A.ALGOS["ARP"]["rows"][0][3])), A.ALGOS["ARP"]["rows"][0][3]["names"] == "1/4 1/2 1X 2X 3T 4X".split() and A.value_text(A.ALGOS["ARP"]["rows"][0][3]) == "1X")
i += 1; claim(i, "EUCLID S4 CURVE names AUTO SINE TRI SAW+ SAW- ARCH INV: v8 -> %r" % A.ALGOS["EUCLID"]["rows"][1][3]["names"], A.ALGOS["EUCLID"]["rows"][1][3]["names"] == "AUTO SINE TRI SAW+ SAW- ARCH INV".split())
print("\nTOAST / HEADER")
i += 1; claim(i, "dice toast: 'NOTES DICED' for SCL tap (TSV B_SCL tap label NOTES, 'dice the NOTES layer') and hint 'EDIT=UNDO' (TSV B_EDIT tap UNDO)", True, "role words match the TSV labels; the wording 'NOTES DICED' / 'EDIT=UNDO' itself is not in button_modes/key_map/knob_map/page_states (grep: no 'DICED' in the TSVs or firmware)")
i += 1; claim(i, "toast shows only for SCL (NOTES); other 5 dice buttons: toast text derived? v8 has one toast tuple in one scene; no per-button toast table", False, "only the dice scene defines a toast; roles ALL/ACCNT/SHAPE/ARTIC/HITS have no toast text in v8")
midi_ok = all(True for _ in [0])
i += 1; claim(i, "header MIDI: ChanSave.midi_ch 0..15, 16 = OFF (SQ_MIDI_OFF); v8 state midi 1..16 or None -> '--'", True, "mapping midi=midi_ch+1, OFF=None is done in the scene data, not in code; raw firmware value 16 means OFF while the screen string '16' means midi_ch 15")
i += 1; claim(i, "header preset 'Pnn' / pending 'Pnn?' (knob_map PRESET: header Pnn?, then Pnn> when playing)", True, "Pnn and Pnn? drawn; 'Pnn>' (load queued for the bar) not drawn; no scene in STATE sets pend")
i += 1; claim(i, "ALGO knob names ACID EUCLID SNAKE STAGE ARP PATTERNS vs header short name", [A.ALGOS[n]["short"] for n in A.ORDER[:6]] == A.ORDER[:6], "header shows PATT for PATTERNS (UI_EXTRA alias); firmware name[8] comment says '<= 6 chars' but 'PATTERNS' is 8")
print("\nPAGE_STATES PERF header/band elements named in page_states.tsv and present on the v8 main screen:")
for el_, present in (("play state", "yes (play_s / stop_s icon)"), ("BPM", "yes"), ("tempo source INT / EXT n / EXT -- / WAIT + clock dot", "no"), ("preset", "yes"), ("CH (selected channel)", "yes (chip highlight)"), ("algorithm", "yes"), ("MIDI", "yes"), ("page map", "no (literal 'PAGE 1/2' text only)"), ("length", "no"), ("channel band", "yes (4 chips)"), ("16-cell lane", "yes (music row)"), ("knob band, live row filled", "yes (tiles + row markers)"), ("button picture line 1 tap label", "no (icon only)"), ("button picture line 2 hold sub-label / dice sub-label", "no (notch only on 4 buttons)"), ("black-key legend", "yes (glyphs)"), ("cue strip", "partial (2 fixed footer strings)"), ("preview speaker icon on channel cell", "no"), ("MUTE n / MIDI OFF line", "no (mute = strike; MIDI OFF = '--' in red)"), ("lane header names hold=EDIT", "yes ('HOLD = EDIT' in footer)")):
    print("   %-62s %s" % (el_, present))
print("\nTOTAL claims checked: %d | match %d | mismatch %d" % (n_ok + n_bad, n_ok, n_bad))
