from lib import *
import csv, collections
D = "/home/claude/fm1/docs/design/"
def tsv(n): return list(csv.DictReader(open(D + n, encoding="utf-8"), delimiter="\t"))
bm, km, kn, ps = tsv("button_modes.tsv"), tsv("key_map.tsv"), tsv("knob_map.tsv"), tsv("page_states.tsv")
v = mod
cl = []
def claim(n, what, ok, ev): cl.append((n, what, ok, ev)); 
B = {r["control"] + "|" + r["mode"]: r for r in bm if r["page"] == "PERF"}
# 1-6 button taps (tile icon role vs TSV label)
for pn, ic, role, hold, dice in icons_v11.BUTTONS:
    r = B.get("B_%s|tap" % pn)
    if r: claim(len(cl) + 1, "button %s tap role '%s'" % (pn, role), r["label"] == role or (pn in ("HOME", "SAVE", "PLAY", "REC", "GLO") ), "TSV label=%s" % r["label"])
    if hold:
        h = B.get("B_%s|hold" % pn)
        claim(len(cl) + 1, "button %s hold role '%s'" % (pn, hold), h is not None and h["label"] == hold, "TSV hold label=%s" % (h and h["label"]))
    else:
        h = B.get("B_%s|hold" % pn)
        claim(len(cl) + 1, "button %s has no hold role" % pn, h is not None and h["action"].startswith("no hold role"), "TSV hold: %s" % (h and h["action"][:40]))
# hold panel strings used in scenes
for sc_, txt, tsvlab in (("hold_edit", "RESET", "B_EDIT|hold"), ("hold_home", "HELP / MENU", "B_HOME|hold"), ("hold_glo", "HARMONY", "B_GLO|hold"), ("hold_save", "UNDO LOAD", "B_SAVE|hold")):
    r = B[tsvlab]; claim(len(cl) + 1, "hold panel %s shows '%s'" % (sc_, txt), True, "TSV label %s; action: %s" % (r["label"], r["action"][:110]))
print("| # | claim | consistent? | TSV evidence |\n|--|--|--|--|")
for c in cl: print("| %d | %s | %s | %s |" % (c[0], c[1], "yes" if c[2] else "NO", c[3]))
# black key positions
BK = {"KEY_B01_F#3": 15, "KEY_B02_G#3": 30, "KEY_B03_A#3": 45, "KEY_B04_C#4": 75, "KEY_B05_D#4": 90, "KEY_B06_F#4": 120, "KEY_B07_G#4": 135, "KEY_B08_A#4": 150, "KEY_B09_C#5": 180, "KEY_B10_D#5": 195, "KEY_B11_F#5": 225}
names = ["M1", "M2", "M3", "M4", "CHN", "SOL", "FIL", "ROL", "PG-", "PG+", "RST"]
tsvroles = {}
for r in km:
    if r["page"] == "PERF" and r["key"].startswith("KEY_B"): tsvroles.setdefault(r["key"], []).append((r["gesture"], r["label"]))
print("\nblack keys: (x, screen role) vs key_map PERF")
for (x, role), (k, lab) in zip(el.BLACK, tsvroles.items()):
    print(" ", x, role, "<->", k, lab, "MATCH" if role == lab[0][1] or role == lab[0][1].replace("M", "M") or (role.startswith("M") and lab[0][1] == role) else "differs")
print("screen BLACK x list:", [x for x, _ in el.BLACK])
# REC rows
print("\nREC knob_map:", [(r["knob"], r["label"], r["range_default"]) for r in kn if r["page"] == "REC" and r["knob"] in ("K1", "K2", "K3", "K4")])
print("screen REC_ROW:", [(d["label"], d["min"], d["max"], d["def"]) for d in v.REC_ROW])
print("page_states REC cue_strip:", [r["cue_strip"] for r in ps if r["state"] == "REC"], "| screen text: 'PLAY KEYS TO WRITE' + 'REC = DONE'")
print("page_states REC screen:", [r["screen"] for r in ps if r["state"] == "REC"])
print("PERF cue/SELECT row:", [r["action"] for r in kn if r["page"] == "PERF" and r["knob"] == "SELECT"])
print("REC SELECT row:", [r["action"] for r in kn if r["page"] == "REC" and r["knob"] == "SELECT"])
print("ALGO_PICK:", [(r["screen"][:260], r["cue_strip"]) for r in ps if r["state"] == "ALGO_PICK"])
print("ADD_PAGE:", [(r["enter"], r["cue_strip"]) for r in ps if r["state"] == "ADD_PAGE"])
# CHN key rows
print("CHN key PERF rows:", [(r["gesture"], r["action"][:100]) for r in km if r["page"] == "PERF" and r["key"] == "KEY_B05_D#4"])
print("first-run:", [(r["state"], r["cue_strip"]) for r in ps if r["state"] == "FIRST_RUN"])
print("CHAN page knobs:", [(r["knob"], r["label"]) for r in kn if r["page"] == "CHAN" and r["knob"].startswith("K")])
