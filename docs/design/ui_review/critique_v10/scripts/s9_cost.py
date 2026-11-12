from common import *
import icons_v10 as IV, math
from collections import Counter, OrderedDict
print("# 9 firmware cost (HI-CON)")
M.palette_set("HI-CON")
used = OrderedDict()
nm = {}
for k_, v_ in list(IV.ICONS.items()) + list(el.IC.items()): nm.setdefault(v_.strip("\n"), k_)
allrects = {}; allcols = Counter(); sceneof = {}
for n in mod.STATE["scenes"]:
    R = rec(n)
    for a in R["arts"]:
        key = nm.get(a["art"].strip("\n"), "?")
        used.setdefault(key, a)
    for r in R["rects"]:
        allcols[r[5]] += 1; sceneof.setdefault(r[5], n)
    for a in R["arts"]:
        allcols[a["c"]] += 1; sceneof.setdefault(a["c"], n)
print("\n## icon bitmaps drawn in the 28 scenes: %d distinct" % len(used))
tb1 = tb2 = 0
print("| icon | w x h | lit px | bytes row-packed (ceil(w/8)*h) | bytes bit-packed (ceil(w*h/8)) |"); print("|---|---|---|---|---|")
for k, a in used.items():
    w, h = a["w"], a["h"]; b1 = math.ceil(w / 8) * h; b2 = math.ceil(w * h / 8); tb1 += b1; tb2 += b2
    print("| %s | %dx%d | %d | %d | %d |" % (k, w, h, a["bits"], b1, b2))
print("TOTAL distinct icons %d : %d bytes row-packed, %d bytes bit-packed (1 bpp, no per-icon header)" % (len(used), tb1, tb2))
# defined but unused
defined = set(IV.ICONS) | set(el.IC)
bt = set(b[1] for b in IV.BUTTONS)
print("v10-specific icons (icons_v10.py new/overridden keys): %s" % sorted(k for k in IV.ICONS if k not in IV.V7 or IV.ICONS[k] != IV.V7[k]))
print("button icons: %d distinct of 12 buttons; rec_ring + `rec` (armed) -> %s" % (len(bt), "rec" in used))
print("icons in the table that no scene draws: %s" % sorted(k for k in IV.ICONS if k not in used and k in bt | {"x7", "midi_s", "play_s", "stop_s", "din", "midi"}))
print("(din/midi/stop_s are defined but drawn? din drawn: %s ; midi: %s ; stop_s: %s ; rec: %s ; undo/home/save/settings in toasts)" % tuple("yes" if k in used else "no" for k in ("din", "midi", "stop_s", "rec")))
# colours outside HI-CON
M.palette_set("HI-CON")
toks = {getattr(M.ux, t): t.upper().rstrip("_") for t in M.Theme.TOKENS}
print("\n## colours drawn as flat fills / icon pixels (rect, frame, art) in the 28 scenes that are NOT a HI-CON token")
print("HI-CON tokens: " + ", ".join("%s %s" % (t, hexc(c)) for c, t in toks.items()))
print("| RGB565 | hex | first scene | draws | what it is |"); print("|---|---|---|---|---|")
what = {mod.WHITE: "WHITE (255,255,255); HI-CON TEXT is #fffaff", mod.PHC: "PHC playhead column (80,80,80)", mod.DICE_BG: "DICE_BG (72,72,72)", mod.OFF_RED: "OFF_RED (220,40,40); HI-CON REC is #ff484a"}
for i, c in enumerate(mod.CH): what[c] = "CH%d Okabe-Ito" % (i + 1)
from el import mix
for i, c in enumerate(mod.CH): what[mix(c, 120)] = "mix(CH%d,120/255) knob fill" % (i + 1)
what[mix(mod.WHITE, 120)] = "mix(WHITE,120/255) REC knob fill"
out = [(c, n) for c, n in allcols.items() if c not in toks]
for c, n in sorted(out, key=lambda x: -x[1]):
    print("| 0x%04X | %s | %s | %d | %s |" % (c, hexc(c), sceneof[c], n, what.get(c, "?")))
print("count of off-palette flat colours: %d  (of %d distinct flat/icon colours drawn)" % (len(out), len(allcols)))
print("off-palette flat colours that are channel identity (4) / fills derived by mix (%d) / neutrals+red (WHITE, PHC, DICE_BG, OFF_RED = 4)" % sum(1 for c, n in out if c in [mix(x, 120) for x in mod.CH] + [mix(mod.WHITE, 120)]))
print("HI-CON tokens used by v10: " + ", ".join(sorted(set(toks[c] for c in allcols if c in toks))))
print("HI-CON tokens never drawn as flat fill: " + ", ".join(sorted(set(toks.values()) - set(toks[c] for c in allcols if c in toks))))
print("note: text anti-aliasing ramps (Canvas.ramp) add intermediate colours; the same count would arise on device from the gfx ramp, not counted here")
R = rec("playing"); img = R["sc"].to_image(); print("distinct RGB values in the rendered 'playing' frame (incl. AA ramps): %d" % len(set(img.getdata())))
# per-palette theme dependency
print("\nmodule PALETTE =", mod.PALETTE, "; hard-coded colours (not theme tokens) in v10.py: CH (4), WHITE, INK(=black, equals HI-CON BG only), PHC, DICE_BG, OFF_RED")
for pal in ("HI-CON", "GREY"):
    M.palette_set(pal); print("  INK == T.BG under %s: %s" % (pal, mod.INK == M.ux.bg))
M.palette_set("HI-CON")
