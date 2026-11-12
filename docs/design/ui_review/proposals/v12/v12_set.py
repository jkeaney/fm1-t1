# SPDX-License-Identifier: GPL-3.0-only
"""v12_set.py: the SET page (GLO tap) drawn in the v12 idiom: P4-016 Gate A proposal. NOT part of the locked v12 scene set (v12.py STATE is untouched,
so tests/v12_export.py and v12_compare.py do not see these scenes until the owner signs the design off and the page is built).
   python3 ../../mock/mock.py render v12_set.py --out out_set     (from this directory, with the repo venv)
The SET page keeps every v12 band: header (tempo + its source), message slot (page title `GLO  ALL CHANNELS`), the four knob tiles (BPM SWING GATE DIV,
knob_map SET rows), the status line (preset left; clock out centre; OCTAVE right), the music row, the 12 button icons and the keybed with its mode line.
Only three things differ from PERF: the tiles, the status line's middle and right, and the footer."""
import os, sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import v12
from v12 import *            # noqa: F401,F403  (draw, base, CH, WHITE, INK, ... the v12 reference implementation)
import algos_v12 as A
from mock import T, AF_S, AF_M, AF_L, text_w

PALETTE = v12.PALETTE
DIV_NAMES = ["1/32", "1/16T", "1/16", "1/8T", "1/8", "1/4"]


def kd(label, lo, hi, d, names=None, fmt=None):
    return dict(label=label, min=lo, max=hi, names=names, fmt=fmt, zero=None, **{"def": d})


SET_ROW = [kd("BPM", 40, 240, 120), kd("SWING", 50, 75, 50, fmt="%d%%"),
           kd("GATE", 10, 100, 50, fmt="%d%%"), kd("DIV", 0, 5, 2, names=DIV_NAMES)]
HOST_TILE = kd("HOST", 40, 240, 120)                     # K1 while an external clock runs: read-only mirror of the host tempo


def ext(S):
    return S.get("clock", "INT") != "INT"


_knob_row, _knobs, _status, _keybed, _topbar = v12.knob_row, v12.knobs, v12.status, v12.keybed, v12.topbar


def knob_row(S):
    if S.get("page") != "SET":
        return _knob_row(S)
    return [HOST_TILE] + SET_ROW[1:] if ext(S) else SET_ROW


def knobs(p, S):
    if S.get("page") != "SET" or not ext(S):
        return _knobs(p, S)
    # EXT: K1 is a mirror, not a control: no fill (the shape changes, not only the colour), a dashed-look frame, the host tempo in MID, `--` when unknown
    S2 = dict(S, vals=dict(S.get("vals", {})))
    S2["vals"].pop(0, None)
    S2["turning"] = S.get("turning") if S.get("turning") and S["turning"][0] != 0 else None
    _knobs(p, S2)
    x, y = 1, S.get("ky", 24)
    p.rect(x, y + 17, 58, 36, T.SURF)
    for i in range(0, 58, 6):                            # a dashed frame: nothing to turn here
        p.rect(x + i, y + 17, 3, 1, T.MID); p.rect(x + i, y + 52, 3, 1, T.MID)
    for j in range(17, 53, 6):
        p.rect(x, y + j, 1, 3, T.MID); p.rect(x + 57, y + j, 1, 3, T.MID)
    host = str(S.get("bpm", 120)) if S["clock"] == "EXT" else "--"
    p.text_c(x, 58, y + 24, AF_L, host, T.MID, T.SURF)
    if S.get("turning") and S["turning"][0] == 0:        # K1 touched: the tile frames itself in white (no channel colour: it is not a channel value)
        p.frame(x - 1, y - 1, 60, 55, WHITE)
        p.frame(x, y, 58, 53, WHITE)


def clock_out(S):
    """(label, word) of the clock-out block (SCREEN_SPEC 6.4): MASTER only while playing as master; ON PLAY when stopped; FOLLOW on any external source"""
    if ext(S):
        return "OUT", "FOLLOW"
    return "OUT", ("MASTER" if S.get("playing", True) else "ON PLAY")


def status(p, S):
    if S.get("page") != "SET":
        return _status(p, S)
    y = 80
    p.text(3, y, AF_M, "P%02d" % S.get("preset", 1), T.TEXT, T.BG)
    lab, word = clock_out(S)
    blocks = []
    if ext(S):
        blocks.append(("EXT", S.get("port", "USB")))
    blocks.append((lab, word))
    x = 42
    for a, b in blocks:                                  # label in AF_S MID, word in AF_M: the same pairing as `MIDI n` and `PAGE v/n`
        xa = p.text(x, y + 2, AF_S, a, T.MID, T.BG)
        x = p.text(xa + 3, y, AF_M, b, T.TEXT, T.BG) + 7
    ot0 = "%+d" % S.get("octave", 0) if S.get("octave", 0) else "0"
    assert x - 7 < 236 - text_w(AF_M, ot0) - text_w(AF_S, "OCT") - 4, "clock-out block runs into the OCTAVE readout"
    o = S.get("octave", 0)
    ot = "%+d" % o if o else "0"
    p.text_r(238, y, AF_M, ot, T.TEXT, T.BG)
    p.text_r(236 - text_w(AF_M, ot), y + 2, AF_S, "OCT", T.MID, T.BG)


def keybed(p, S):
    if S.get("page") != "SET":
        return _keybed(p, S)
    # the keys and the black-key roles are PERF's (steps and moves stay live on SET); only the mode line differs
    S2 = dict(S, page=None)
    _keybed(p, S2)
    ym = 186 + 33 + 6
    p.rect(0, ym - 3, W, 16, T.BG)
    c = S.get("clock", "INT")
    left = {"INT": "KEYS = STEPS", "EXT": "HOST SETS TEMPO", "WAIT": "WAIT FOR HOST START", "LOST": "CLOCK LOST: STOPPED"}[c]
    right = {"INT": "GLO = BACK", "EXT": "GLO = BACK", "WAIT": "PLAY=CANCEL", "LOST": "PLAY=START"}[c]
    fl = AF_M if c in ("INT", "EXT") else AF_S
    fr = AF_M if c in ("INT", "EXT") else AF_S
    assert text_w(fl, left) + text_w(fr, right) + 12 <= W, (left, right)
    p.text(2, ym + (0 if fl is AF_M else 2), fl, left, T.TEXT if c != "LOST" else T.REC, T.BG)
    p.text_r(238, ym + (0 if fr is AF_M else 2), fr, right, T.MID, T.BG)


def topbar(p, S):
    if S.get("page") == "SET" and not (S.get("toast") or S.get("hold")):
        S = dict(S, toast=("enc", "GLO", "ALL CHANNELS"))   # a page names itself while it is open (like CHN CHn SETUP)
    return _topbar(p, S)


for n, f in (("knob_row", knob_row), ("knobs", knobs), ("status", status), ("keybed", keybed), ("topbar", topbar)):
    setattr(v12, n, f)

MESSAGES = [("enc", "GLO", "ALL CHANNELS"), ("enc", "K1", "HOST SETS TEMPO"), ("enc", "OCT+", "OCTAVE +1"), ("enc", "OCT-", "OCTAVE -2"),
            ("enc", "K2", "SWING 75%"), ("enc", "K3", "GATE 100%"), ("enc", "K4", "DIV 1/16T")]


def check_messages():
    v12.MESSAGES[:] = MESSAGES
    return v12.check_messages()


def sset(**kw):
    s = v12.base(used=[0, 1, 3], page="SET", **kw)
    return s


STATE = {"scenes": {
    "set_int": sset(),
    "set_int_stopped": sset(playing=False),
    "set_turn_swing": sset(turning=(1, 40), vals={1: 62}),
    "set_octave": sset(octave=1, toast=("enc", "OCT+", "OCTAVE +1")),
    "set_ext_live": sset(clock="EXT", bpm=124),
    "set_ext_k1_turned": sset(clock="EXT", bpm=124, turning=(0, 40), toast=("enc", "K1", "HOST SETS TEMPO")),
    "set_ext_wait": sset(clock="WAIT", playing=False),
    "set_ext_lost": sset(clock="LOST"),
}}

draw = v12.draw
