# SPDX-License-Identifier: GPL-3.0-only
"""example_proposal.py: a template proposal. It draws the CURRENT PERF screen (current_perf.draw) and then changes it the way a
proposal would: a custom 7x7 icon in the header, a re-drawn cue strip, and marks for the annotated render.
   python mock.py render example_proposal.py --out /tmp/mock_out"""
from mock import T, AF_S, AF_M, measure, text_w, fixture
import current_perf

PALETTE = "GREY"
STATE = {"scenes": {"perf": fixture("perf"), "playing": fixture("perf_playing")}}

ARROW = """
..#....
..##...
#####..
.#####.
#####..
..##...
..#....
"""


def draw(cv, S):
    current_perf.draw(cv, S)                       # three blits: A (y 0..108), B (108..212), C (212..240)
    cv.begin(240, 28, T.BG)                        # redraw blit C (Z5..Z7) with a proposed cue strip
    current_perf.draw_z5(cv, S)
    cv.glyph1(2, 12 + 2, ARROW, T.ACCENT)          # a 7x7 1-bit icon, then the cue after it
    cv.text_fit(12, 12 - 3, AF_S, "PLAY: start   K1-K4: morph", T.TEXT, T.BG, 228)
    cv.mark("Z6 cue", 0, 12, 240, 12)
    cv.mark("icon", 2, 14, 7, 7)
    cv.blit(0, 212)
    # measure() as you draw: advance, ink, cap height, fit
    m = measure("PLAY: start   K1-K4: morph", "S", 228)
    print("  cue: advance %d px, fits 228: %s" % (m["advance"], m["fits"]))
