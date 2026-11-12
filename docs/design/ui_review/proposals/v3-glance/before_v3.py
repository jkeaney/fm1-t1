# SPDX-License-Identifier: GPL-3.0-only
"""before_v3.py: the CURRENT PERF screen (mock/current_perf.draw) on the scenes of proposal_v3, for the before/after sheets.
The current screen has no tempo-source word for EXT beyond `EXT n`, no REC glyph in these fixtures and no big toast: its toast is the Z6 strip
(SCREEN_SPEC 2.7, ui_sq.c: T_TINT bar, T_ACCENT text at x=4), reproduced below with the text each event would show."""
import copy
import os
import sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "..", "mock"))
from mock import T, AF_S                      # noqa: E402
import current_perf                           # noqa: E402
import proposal_v3 as P                       # noqa: E402

PALETTE = "GREY"
TOAST = {"algo": "ALGO: EUCLID", "preset": "P04: SAVE = SAVE HERE", "dice": "DICE NOTES", "undo": "UNDO: DICE NOTES"}


def cur_state(S):
    s = copy.deepcopy(S)
    s["morph"] = list(S["knobs"])             # the shipping screen reads the morph array in morph-id order (identity for ACID/EUCLID/SNAKE/STAGE)
    if s["channels"][s["sel"]]["algo"] == 4:  # ARPT's knob_morph in the test table is reversed; keep knob order for the comparison
        s["morph"] = [S["knobs"][3], S["knobs"][2], S["knobs"][1], S["knobs"][0]] + list(S["knobs"][4:])
    return s


STATE = {"scenes": {n: cur_state(s) for n, s in P.scenes().items()}}


def draw(cv, S):
    if S.get("map"):                           # HOME held: the shipping screen only changes its cue strip (SCREEN_SPEC cue row 1)
        current_perf.draw(cv, S)
        cv.begin(240, 28, T.BG)
        current_perf.draw_z5(cv, S)
        cv.text_fit(0, 9, AF_S, "HOME: touch=HELP  let go=MENU", T.TEXT, T.BG, 240)
        cv.blit(0, 212)
        return
    current_perf.draw(cv, S)
    if S.get("toast"):
        cv.begin(240, 28, T.BG)
        current_perf.draw_z5(cv, S)
        cv.rect(0, 12, 240, 12, T.TINT)
        cv.text_fit(4, 9, AF_S, TOAST[S["toast"]["kind"]], T.ACCENT, T.TINT, 236)
        cv.blit(0, 212)
