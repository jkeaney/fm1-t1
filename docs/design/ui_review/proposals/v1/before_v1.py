# SPDX-License-Identifier: GPL-3.0-only
"""before_v1.py: the CURRENT PERF screen (mock/current_perf.draw) on the six scenes of proposal_v1, for the before/after sheets.
The current firmware has no tempo-source word and no toast on this mock path except the Z6 toast, reproduced below as SCREEN_SPEC 2.7."""
import os, sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "..", "mock"))
from mock import T, AF_S                      # noqa: E402
import current_perf                           # noqa: E402
import proposal_v1                            # noqa: E402

PALETTE = "GREY"
STATE = {"scenes": proposal_v1.scenes()}


def draw(cv, S):
    current_perf.draw(cv, S)
    if S.get("toast"):                        # ui_sq.c line 451: T_TINT bar, T_ACCENT text at x=4
        cv.begin(240, 28, T.BG)
        current_perf.draw_z5(cv, S)
        cv.rect(0, 12, 240, 12, T.TINT)
        cv.text_fit(4, 9, AF_S, S["toast"], T.ACCENT, T.TINT, 236)
        cv.blit(0, 212)
