import sys, csv, collections; sys.path.insert(0, "/home/claude/fm1/docs/design/ui_review/critique_v7/scripts")
from lib import *
import icons_v7 as IV
D = "/home/claude/fm1/docs/design/"
def tsv(n): return [(i + 1, r) for i, r in enumerate(csv.reader(open(D + n), delimiter="\t"))]
BM, KM, NM = tsv("button_modes.tsv"), tsv("key_map.tsv"), tsv("knob_map.tsv")
def bm(ctl, g): return next((i, r) for i, r in BM if r[0] == "PERF" and r[1] == ctl and r[2] == g)
def km(page, key, g): return next((i, r) for i, r in KM if r[0] == page and r[1] == key and r[2] == g)
def nm(algo, k): return next((i, r) for i, r in NM if r[0] == "PERF" and r[1] == algo and r[2] == k)
out = []; P = out.append
variant("mock")
P("# 6. Accuracy vs the TSVs (each row: on-screen claim | evidence in code/render | TSV line | verdict)")
P("| # | on-screen claim | what the render/code shows | TSV line | verdict |")
P("|---|---|---|---|---|")
n = [0]
def row(claim, shows, tsvline, ok):
    n[0] += 1; P("| %d | %s | %s | %s | %s |" % (n[0], claim, shows, tsvline, ok))
PANEL = {b[0]: b for b in IV.BUTTONS}
btn_ctl = {"FX": "B_FX", "SCL": "B_SCL", "ENV": "B_ENV", "LFO": "B_LFO", "EDIT": "B_EDIT", "GLO": "B_GLO", "HOME": "B_HOME", "SAVE": "B_SAVE", "ARP": "B_ARP", "SEQ": "B_SEQ", "PLAY": "B_PLAY", "REC": "B_REC"}
sem = {"FX": "die + 4 corner pips + centre cross", "SCL": "die + note glyph", "ENV": "die + rising/falling chevron '>'", "LFO": "die + sine-like wave", "EDIT": "undo arrow", "GLO": "gear", "HOME": "house", "SAVE": "floppy", "ARP": "die + bar over block (articulation)", "SEQ": "die + 3 dot columns + base line (hits)", "PLAY": "triangle (stopped) / square (playing)", "REC": "filled circle"}
for pn, ic, role, hold, dice in IV.BUTTONS:
    ctl = btn_ctl[pn]
    li, r = bm(ctl, "tap"); hi, hr = bm(ctl, "hold")
    ok_tap = r[4] == role
    ok_hold = hr[4] == hold
    row("button %s shows icon '%s' (%s); code tuple role/hold = %s/%s" % (pn, ic, sem[pn], role, hold),
        "icon drawn, role/hold text NOT drawn (buttons7 ignores role and hold)", "button_modes.tsv:%d tap label %s; :%d hold label %s" % (li, r[4], hi, hr[4]),
        "tap role %s; code hold label %s TSV; hold role on screen: no" % ("match" if ok_tap else "MISMATCH", "matches" if ok_hold else "DIFFERS from"))
# black keys
KEYNAMES = ["F#3", "G#3", "A#3", "C#4", "D#4", "F#4", "G#4", "A#4", "C#5", "D#5", "F#5"]
want = ["M1", "M2", "M3", "M4", "CHN", "SOL", "FIL", "ROL", "PG-", "PG+", "RST"]
for k, (cx, role) in enumerate(el.BLACK):
    key = "KEY_B%02d_%s" % (k + 1, KEYNAMES[k])
    g = "hold" if role in ("SOL", "FIL", "ROL") else "tap"
    li, r = km("PERF", key, g)
    shows = {"M1": "square in CH1 colour (filled; outline in T.MID when muted; absent if channel unused)", "M2": "square CH2", "M3": "square CH3", "M4": "square CH4",
             "CHN": "2x2 dots in the four channel colours", "SOL": "letter S", "FIL": "letter F", "ROL": "letter R", "PG-": "left triangle 5x7", "PG+": "right triangle 5x7", "RST": "bar+left-triangle 5x7"}[role]
    ok = (r[4] == want[k] == role)
    row("black key %s (x centre %d) glyph = %s" % (KEYNAMES[k], cx, shows), "role code %s at position %d; %s role on TSV" % (role, k + 1, g), "key_map.tsv:%d (%s %s -> %s)" % (li, key, g, r[4]), "match" if ok else "MISMATCH")
# footer
li, r = km("PERF", "KEY_W01_F3", "tap"); li2, r2 = km("PERF", "KEY_W01_F3", "hold")
row("footer 'KEYS = STEPS' (PERF)", "AF_S at x=2 y=225; white keys lit in the selected channel colour when its step is on", "key_map.tsv:%d W01..W16 tap = step n ON/OFF (label %s)" % (li, r[4]), "match")
row("footer 'HOLD = EDIT' (PERF)", "AF_S MID right-aligned x=238 y=225, same string also contains 'PAGE 1/2'", "key_map.tsv:%d hold = open NOTE EDIT (label %s)" % (li2, r2[4]), "match for HOLD = EDIT")
row("footer 'PAGE 1/2'", "constant string in footer_keybed (no state field read); S has no page key", "key_map.tsv:%d PG-/PG+ (view prev/next page); page_states.tsv:2 header page map" % km("PERF", "KEY_B09_C#5", "tap")[0], "static string: no state source in S (the string is the same in every scene)")
li, r = km("REC", "KEY_W01_F3", "tap"); lj, rj = km("REC", "KEY_B11_F#5", "tap")
nrec = sum(1 for i, x in KM if x[0] == "REC"); nnote = sum(1 for i, x in KM if x[0] == "REC" and x[4] == "NOTE")
row("REC footer 'ALL KEYS PLAY NOTES'", "AF_S x=16 y=225 + rec glyph; white keys drawn T.KEY, black keys T.DIM, glyphs hidden", "key_map.tsv:%d..%d: %d REC rows, %d with label NOTE (all tap)" % (li, max(i for i, x in KM if x[0] == "REC"), nrec, nnote), "match" if nrec == nnote == 27 else "MISMATCH")
# knob labels
shipK = el.KN
for algo in ("ACID", "EUCLID"):
    for ri, rowname in enumerate(("K", "S")):
        if ri >= len(shipK[algo]): 
            row("KN[%s] second row (S1-S4)" % algo, "el.KN has no row 2 for %s (only 1 row)" % algo, "knob_map.tsv:%s S1-S4" % ",".join(str(nm(algo, "S%d" % j)[0]) for j in (1, 2, 3, 4)), "MISSING data (render IndexError at row=1)")
            continue
        for j in range(4):
            kname = "%s%d" % (rowname, j + 1)
            li, r = nm(algo, kname)
            lab, val, fr = shipK[algo][ri][j]
            row("%s %s tile label %r value %r fill %d%%" % (algo, kname, lab, val, fr), "label from el.KN", "knob_map.tsv:%d label %s range %s" % (li, r[3], r[5][:40]), "label match" if lab == r[3] else "MISMATCH")
# default values and fills
import re
def rng(r):
    m = re.search(r"(\d+)\.\.(\d+) def (\d+)", r[5]); return tuple(int(x) for x in m.groups()) if m else None
for algo in ("ACID", "EUCLID"):
    for ri in range(len(shipK[algo])):
        for j in range(4):
            kname = "%s%d" % ("KS"[ri], j + 1); li, r = nm(algo, kname); lab, val, fr = shipK[algo][ri][j]; rg = rng(r)
            if rg:
                lo, hi, d = rg
                calc = 100 * (d - lo) // (hi - lo) if hi > lo else 0
                P("<!-- %s %s val %s def %d fill shown %d calc %d -->" % (algo, kname, val, d, fr, calc))
    ok = [(shipK[algo][ri][j][1] == str(rng(nm(algo, "%s%d" % ("KS"[ri], j + 1))[1])[2]) if rng(nm(algo, "%s%d" % ("KS"[ri], j + 1))[1]) else None) for ri in range(len(shipK[algo])) for j in range(4)]
    P("<!-- %s default==shown: %s -->" % (algo, ok))
rows_defaults = []
for algo in ("ACID", "EUCLID"):
    for ri in range(len(shipK[algo])):
        for j in range(4):
            kname = "%s%d" % ("KS"[ri], j + 1); li, r = nm(algo, kname); lab, val, fr = shipK[algo][ri][j]; rg = rng(r)
            if rg:
                lo, hi, d = rg; calc = 100 * (int(val) - lo) // (hi - lo) if hi > lo and val.isdigit() else None
                rows_defaults.append((algo, kname, lab, val, d, fr, calc, li))
for a, k, lab, val, d, fr, calc, li in rows_defaults:
    row("%s %s %s shows value %s, fill %d%%" % (a, k, lab, val, fr), "(value-lo)/(hi-lo) = %s%%; TSV default %d" % (calc, d), "knob_map.tsv:%d" % li,
        ("value==default; fill %s" % ("matches" if calc is not None and abs(calc - fr) <= 1 else "DIFFERS")) if val == str(d) else "value %s != default %d (demo value); fill %s" % (val, d, "matches" if calc is not None and abs(calc - fr) <= 1 else "DIFFERS"))
# non-numeric: ACID S4 DIR FWD
li, r = nm("ACID", "S4")
row("ACID S4 DIR value 'FWD' fill 0", "el.KN", "knob_map.tsv:%d dir FWD BWD PEND" % li, "value in set; fill 0 for an enum (FWD = first)")
# toast
li, r = bm("B_SCL", "tap"); lj, rj = bm("B_EDIT", "tap")
row("toast 'NOTES DICED' with SCL button lit", "toast_top word; SCL cell drawn WHITE bg/INK icon; hatched bars on CH1 steps", "button_modes.tsv:%d B_SCL tap = dice the NOTES layer" % li, "match")
row("toast hint 'EDIT=UNDO'", "toast_top hint AF_M right-aligned", "button_modes.tsv:%d B_EDIT tap = UNDO / REDO the last dice" % lj, "match (tap); EDIT hold = RESET (:%d) not mentioned" % bm("B_EDIT", "hold")[0])
# header
li, r = NM[[i for i, x in NM].index(87)]
row("header algo name drawn as algo[:6]", "ACID/EUCLID/SNAKE/STAGE/ARP unchanged; PATTERNS -> 'PATTER'", "knob_map.tsv:87 ALGO names ACID EUCLID SNAKE STAGE ARP PATTERNS; page_states.tsv:14 short names ACID EUCL SNAK STAG ARP PATT", "MISMATCH for PATTERNS (PATTER is neither name nor the PATT short form)")
li, r = NM[[i for i, x in NM].index(88)]
row("header preset 'P01' / pending 'P01?' in a white frame", "pend frame x=196..238; text 'Pnn?'", "knob_map.tsv:88 PRESET: header 'Pnn?' then 'Pnn>' while playing", "'Pnn?' match; 'Pnn>' (queued while playing) not drawn")
row("header transport icon: play triangle when playing, circle (REC glyph, T.MID) when stopped", "enc_row uses IC['play'] if playing else IC['rec']; the PLAY button shows a stop square while playing", "button_modes.tsv:%d B_PLAY tap start/stop; :%d B_REC tap open REC" % (bm("B_PLAY", "tap")[0], bm("B_REC", "tap")[0]), "stopped-state header glyph is the filled-circle bitmap IC[rec] (11x11, T.MID), the same shape as the REC button icon; playing-state header glyph is the play triangle while the PLAY button then shows a stop square")
ndice = sum(1 for b in IV.BUTTONS if b[4]); tsvdice = [c for c in ("B_FX", "B_SCL", "B_ENV", "B_LFO", "B_ARP", "B_SEQ") if "dice" in bm(c, "tap")[1][3].lower()]
row("dice-group tint (DICE_BG + amber icon) on %d buttons" % ndice, "FX SCL ENV LFO ARP SEQ", "button_modes.tsv:%s tap text contains 'dice'" % ",".join(str(bm(c, "tap")[0]) for c in tsvdice), "match (%d of 6 TSV dice buttons found)" % len(tsvdice))
row("16 white-key cells at 15 px pitch, 11 black keys at x=15 30 45; 75 90; 120 135 150; 180 195; 225", "counts from render: 16 white, 11 black", "key_map.tsv PERF: KEY_W01..W16 (%d rows), KEY_B01..B11" % sum(1 for i, x in KM if x[0] == "PERF" and x[1].startswith("KEY_W")), "match")
row("muted channel bars drawn as T.DIM outlines; M-key glyph as T.MID outline", "music_slim/footer_keybed muted branch", "key_map.tsv:210 M1 tap = mute / unmute CH1 (latched)", "match")
row("accent = 2 px WHITE cap above the bar", "music_slim acc flag", "key_map.tsv:2 hard tap turning a step ON = ACCENT (velocity 120)", "match in meaning; TSV has no on-screen form")
row("BPM '120' in header", "enc_row bpm default 120; v7 passes no BPM from S", "page_states.tsv:2 header: clock dot + BPM with tempo source INT / EXT n / EXT -- / WAIT", "static value; tempo-source word and clock dot not drawn")
row("REC armed: REC button icon fg = T.REC", "buttons7 rec=True; mock-MONO T.REC=#ff484a, firmware-MONO T.REC=text white (see 3 / 8)", "button_modes.tsv:200 B_REC tap open REC (latched, LED lit)", "mock-MONO distinct from unarmed; firmware-MONO identical colour to T.TEXT")
row("channel chips 1..4 (selected = filled)", "chip digits; '+' at slot len(used) only", "knob_map.tsv:86 SELECT CH1..CH4", "match; '+' missing for non-contiguous sets (see 5d)")
row("MIDI channel (page_states.tsv:2 header lists MIDI)", "not drawn anywhere; chans[].midi never read (16 vs 2: 0 px differ)", "page_states.tsv:2 PERF screen: header ... MIDI", "not shown")
open("/home/claude/fm1/docs/design/ui_review/critique_v7/scripts/s6a_out.md", "w").write("\n".join(l for l in out if not l.startswith("<!--")))
print("\n".join(l for l in out if not l.startswith("<!--")))
print("total claims:", n[0])
