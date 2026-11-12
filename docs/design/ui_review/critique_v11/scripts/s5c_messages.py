from lib import *
import contextlib, io
v = mod
def run(toast=None, hold=None):
    S = v.base(used=[0, 1, 2, 3], toast=toast, hold=hold)
    buf = io.StringIO()
    with contextlib.redirect_stdout(buf):
        sc, tx, arts = texts_with_bbox(S)
    sc2, cv2 = None, None
    sc0, tx0, _ = texts_with_bbox(v.base(used=[0, 1, 2, 3]))
    first = [r for r in tx if r["y"] < 120 + 0 and r["bbox"][1] < 120]
    first0 = [r for r in tx0 if r["bbox"][1] < 120]
    # strip 0 holds everything with canvas y < 120; the top-bar texts come last in strip 0
    m = [(r["s"], r["font"], r["bbox"], bool(r["flags"] & 2)) for r in first[len(first0):]]
    return m
def report(label, toast=None, hold=None):
    m = run(toast, hold)
    mx = max((b[2] for _, _, b, _ in m), default=0)
    fs = [f for s, f, b, c in m]
    cut = mx > 239 or any(c for *_, c in m)
    print("| %s | %s | %s | x1=%d | %s |" % (label, [s for s, *_ in m], "/".join(fs), mx, "CUT" if cut else "ok"))
    return cut
print("| message | texts in panel | fonts | rightmost ink x (exclusive) | cut? |\n|--|--|--|--|--|")
n = c = 0
for al in list(A.ORDER) + ["PATTERNS"]:
    nm = A.ALGOS[al]["short"]
    for ch in range(1, 5):
        c += report("enc SELECT 'CH%d %s'" % (ch, al), ("enc", "SELECT", "CH%d %s" % (ch, al))); n += 1
    c += report("enc ALGO '%s AT STEP' (full name)" % al, ("enc", "ALGO", "%s AT STEP" % al)); n += 1
for pv in (2, 16, 99):
    c += report("enc PRESETS 'P%02d AT BAR'" % pv, ("enc", "PRESETS", "P%02d AT BAR" % pv)); n += 1
    c += report("enc PRESETS 'P%02d LOADS AT BAR'" % pv, ("enc", "PRESETS", "P%02d LOADS AT BAR" % pv)); n += 1
for r in ("ROW 2 OF 2", "ROW 3 TB-3", "ROW 3 TB-3 BITE", "ROW 3 OF 3", "LAST ROW", "LAST ROW: OCT- = ROW 1"):
    c += report("enc OCT+ '%s'" % r, ("enc", "OCT+", r)); n += 1
c += report("enc OCT- 'ROW 1 OF 2'", ("enc", "OCT-", "ROW 1 OF 2")); n += 1
for h in ("RESET", "HARMONY", "HELP / MENU", "UNDO LOAD", "REDO LOAD", "HLP/MN"):
    c += report("hold '%s'" % h, hold=("home", h, 60)); n += 1
for w, hnt in (("MIDI OFF", "TAKEN"), ("NEW", "ALGO=PICK"), ("NOTES", "EDIT=UNDO"), ("ALL", "EDIT=UNDO"), ("ACCNT", "EDIT=UNDO"), ("SHAPE", "EDIT=UNDO"), ("ARTIC", "EDIT=UNDO"), ("HITS", "EDIT=UNDO"),
               ("PAST THE END", ""), ("64 STEPS = MAX", ""), ("NOTHING TO RESET", ""), ("ONE PAGE: K1 = SHORTER", ""), ("LAST ROW", ""), ("HOME", ""), ("MUTE CH3", ""), ("PREVIEW CH4", "")):
    c += report("toast '%s' | '%s' (chip)" % (w, hnt), ("x", w, hnt, 3)); n += 1
print("messages tested:", n, "cut:", c)
# the longest strings that still fit
from mock import text_w
for f, nmx in (("AF_M", AF_M), ("AF_S", AF_S)):
    print(f, "width of 'HELP / MENU' =", text_w(nmx, "HELP / MENU"), "'ALGO=PICK' =", text_w(nmx, "ALGO=PICK"), "'EUCLID' =", text_w(nmx, "EUCLID"), "'PATTERNS' =", text_w(nmx, "PATTERNS"))
w = 240 - v.TB_X
print("hold text budget (fit maxw) =", w - 22, "px from x=124; enc budget = w - name chip - 8; toast budget = 240-4-hint-6-x")
