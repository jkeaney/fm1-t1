from common import *
import math
print("# 4b visual size at 35 cm: 30 mm panel = 240 px -> 8 px/mm; arcmin = size_mm / 350 * 3438")
am = lambda px: px / 8.0 / 350 * 3438
print("1 px = %.3f mm = %.2f arcmin" % (1/8, am(1)))
print("\n## measured ink heights of capital 'H' per font (text_ink), advance of 'M'")
for nm, f in (("AF_S", AF_S), ("AF_M", AF_M), ("AF_L", AF_L)):
    b = M.text_ink(f, "H"); h = b[3] - b[1]
    print("%s: cap ink height %d px = %.2f mm = %.1f arcmin ; stroke (est) -- ; 'M' advance %d" % (nm, h, h / 8, am(h), text_w(f, "M")))
print("\n## feature sizes used for meaning (px -> mm -> arcmin)")
for name, px in [("hit strip height (3)", 3), ("rest tick (3x1) height", 1), ("accent cap height (2)", 2), ("slide bridge (2x3)", 2), ("playhead white bar height (2)", 2), ("selection bar under chip (2)", 2), ("OFF_RED cap on chip (3 high)", 3), ("level edge (2 high x 7 wide)", 2), ("position dot (3 high)", 3), ("row marker (7x7)", 7), ("page dot (9x7)", 7), ("muted diagonal (1 px line)", 1), ("hollow bar frame (1 px line)", 1), ("bar gap (2)", 2), ("button gap vertical (1)", 1), ("button gap horizontal (2)", 2), ("button cell (38x17) height", 17), ("white key gap (1)", 1), ("hit-strip dash (5 wide)", 5), ("hit-strip (13 wide)", 13), ("icon 13x13", 13), ("black-key glyph 7x7", 7), ("x7 muted glyph", 7), ("hold progress bar (2 high)", 2), ("header chip 13x19 width", 13)]:
    print("  %-34s %2d px  %.3f mm  %.1f arcmin" % (name, px, px / 8, am(px)))
