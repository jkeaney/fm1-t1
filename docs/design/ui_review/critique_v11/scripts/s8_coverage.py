# PERF control x gesture rows (button_modes / knob_map / key_map, page PERF) vs what the v11 screens show.
# Y = shown persistently on the PERF screen; H = shown only while touched / pressed / held (toast, hold panel, encoder panel); N = not shown.
# 'no role' rows (TSV: "no hold role: acted once on press") are not counted.
R = []
def add(c, g, s, why): R.append((c, g, s, why))
for b, ic in (("FX", "ALL die"), ("SCL", "NOTES die"), ("ENV", "ACCNT die"), ("LFO", "SHAPE die"), ("ARP", "ARTIC die"), ("SEQ", "HITS die")):
    add("B_" + b, "tap", "Y", "tile icon (%s); dice toast on press" % ic)
add("B_EDIT", "tap", "Y", "undo icon"); add("B_EDIT", "hold", "H", "hold panel RESET (needs the 0.25 s hold)")
add("B_GLO", "tap", "Y", "settings icon"); add("B_GLO", "hold", "H", "hold panel HARMONY")
add("B_HOME", "tap", "Y", "home icon"); add("B_HOME", "hold", "H", "hold panel HELP / MENU")
add("B_SAVE", "tap", "Y", "save icon"); add("B_SAVE", "hold", "H", "hold panel UNDO LOAD")
add("B_PLAY", "tap", "Y", "play icon + header play/stop glyph"); add("B_REC", "tap", "Y", "rec ring icon + REC state")
add("B_OCTDN", "tap", "Y", "row boxes 1..n in the status line (OCT buttons themselves are not drawn)")
add("B_OCTUP", "tap", "Y", "row boxes; OCT+ toast ROW n")
add("B_OCTDN", "hold", "N", "peek: nothing shown"); add("B_OCTUP", "hold", "N", "peek: nothing shown")
add("B_OCTDN", "hold-both", "N", "UPDATE MODE / CALIBRATION"); add("B_OCTUP", "hold-both", "N", "UPDATE MODE / CALIBRATION")
for k in ("K1", "K2", "K3", "K4"): add(k, "turn", "Y", "knob tile (label, value, meter)")
add("SELECT", "turn", "Y", "channel chips + SELECT toast"); add("ALGO", "turn", "Y", "header algorithm name + ALGO toast (no six-algorithm cue list)")
add("PRESET", "turn", "Y", "P01 text in status line; Pnn? pending box")
for r in ("R1", "R2", "R3", "R4"): add(r, "turn", "Y", "ROW 3 tiles (only when a RIG exists)")
for i in range(1, 17): add("W%02d" % i, "tap", "Y", "lane step bars + keybed step colours")
for i in range(1, 17): add("W%02d" % i, "hold", "Y", "cue strip 'HOLD = EDIT'")
for i in range(1, 5): add("M%d" % i, "tap", "Y", "black-key square/x + chip diagonal")
for i in range(1, 5): add("M%d" % i, "hold", "N", "PREVIEW (speaker icon / PRVn) not drawn")
add("CHN", "tap", "Y", "MIDI glyph on the key (lit white + cue when MIDI OFF/new channel)"); add("CHN", "hold", "N", "RIG page entry not drawn")
for r in ("SOL", "FIL", "ROL"): add(r, "hold", "Y", "letter S / F / R on the key")
for r in ("PG-", "PG+", "RST"): add(r, "tap", "Y", "arrow / reset glyph on the key")
n = len(R); y = sum(r[2] == "Y" for r in R); h = sum(r[2] == "H" for r in R); no = [r for r in R if r[2] == "N"]
print("PERF control x gesture rows counted:", n)
print("yes (persistent):", y, "%.0f%%" % (100 * y / n), "| only on touch/press/hold:", h, "%.0f%%" % (100 * h / n), "| no:", len(no), "%.0f%%" % (100 * len(no) / n))
print("yes + touch/hold:", y + h, "%.0f%%" % (100 * (y + h) / n))
print("rows = no:", [(r[0], r[1]) for r in no])
for r in R: print("|", " | ".join(r), "|")
