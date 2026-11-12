# SPDX-License-Identifier: GPL-3.0-only
"""evidence_b_edge.py: builds the edge-case fixtures of EVIDENCE_B_v3.md section 6 (a)-(d), (f)-(k) from proposal_v3's own draw(),
renders each to docs/design/ui_review/critique_v3/edge/NAME.png + NAME_4x.png and prints the tool warnings and ink boxes.
Run from the repo root:  .venv/bin/python docs/design/ui_review/critique_v3/evidence_b_edge.py"""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from evidence_b_lib import *
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "edge")
mock.palette_set("GREY")

def R(name, st, note=""):
    sc, w, recs = render(st, name)
    save(sc, OUT, name)
    print("RENDER %-34s warnings: %s %s" % (name, w or "none", note))
    return sc, recs

def lane_all(on=1, vel=127, acc=1, slide=1, rep=4):
    return [dict(on=on, vel=vel, ed=0, acc=acc, slide=slide, rep=rep) for _ in range(16)]

def pl(st, ph, ch=0):
    """playhead on step index ph (0-based) of channel ch: the draw uses (p + len - 1) % len"""
    n = st["channels"][ch]["len"]
    st["channels"][ch]["p"] = (ph + 1) % n
    st["playing"] = 1
    return st

base = lambda: scene("glance_playing")

# (a) all four muted
s = base()
for c in s["channels"]: c["muted"] = 1; c["snd0"] = 0
R("a_all_muted_sel1", s)
s2 = copy.deepcopy(s); s2["sel"] = 2
R("a_all_muted_sel3", s2)
s3 = copy.deepcopy(s); 
for c in s3["channels"]: c["midi_ch"] = 16
R("a_all_muted_all_off", s3, "(all four muted AND MIDI OFF)")

# (b) 16 steps ON, accent+slide+ratchet on every step
for ph in (0, 7, 15):
    s = base(); s["lane"] = lane_all(); pl(s, ph)
    R("b_full_ph%d" % (ph + 1), s)
s = base(); s["lane"] = lane_all(vel=1); pl(s, 7)
R("b_full_vel1_ph8", s)
s = base(); s["lane"] = lane_all(); s["channels"][0]["muted"] = 1; pl(s, 7)
R("b_full_muted_ph8", s)
s = base(); s["lane"] = [dict(on=1, vel=100, ed=0, acc=i % 2, slide=0, rep=(i % 8) + 1) for i in range(16)]; pl(s, 3)
R("b_ratchet_sweep_1to8", s, "(rep 1..8 twice; NOTE_EDIT K4 range is 1x..4x, so rep 5..8 is hypothetical)")
s = base(); s["lane"] = lane_all(rep=1, acc=0); pl(s, 7)
R("b_slide_chain_ph8", s, "(slide on all 16, no accent/ratchet)")

# (c) no step ON
s = base(); s["lane"] = lane_all(on=0, vel=1, acc=0, slide=0, rep=1); pl(s, 5)
R("c_empty_playing", s)
s = copy.deepcopy(s); s["playing"] = 0
R("c_empty_stopped", s)

# (d) 64-step pattern, viewing page 2, playhead on page 4 (FOLLOW off is not a state of the proposal's draw())
s = base(); c = s["channels"][0]; c["len"] = 64; c["pg_used"] = 15
s["vpage"] = 1; pl(s, 55)
R("d_view_p2_playhead_p4", s)
s = base(); c = s["channels"][0]; c["len"] = 64; s["vpage"] = 3; pl(s, 5)
R("d_view_p4_playhead_p1", s)
s = base(); c = s["channels"][0]; c["len"] = 64; s["vpage"] = 1; pl(s, 20)
R("d_view_p2_playhead_p2", s, "(playhead on the viewed page: reference)")

# (f) clock states + REC
s = base(); s["ext"] = 1; s["bpm"] = 0; s["playing"] = 0
R("f_ext_unknown_stopped", s, "(bpm 0: draw prints 'EXT 0'; no 'EXT --' / 'WAIT' state exists in draw())")
s = base(); s["ext"] = 1; s["bpm"] = 126
R("f_ext126_playing", s)
s = base(); s["ext"] = 1; s["bpm"] = 126; s["playing"] = 0
R("f_ext126_stopped", s, "(stopped on external clock)")
s = base(); s["rec"] = 1
R("f_rec_armed_playing", s)
s = base(); s["rec"] = 1; s["playing"] = 0
R("f_rec_armed_stopped", s, "(same inputs as recording-while-playing minus 'playing')")
s = base(); s["rec"] = 1; s["ext"] = 1; s["bpm"] = 126
R("f_rec_ext_playing", s)

# (g) selected AND muted AND MIDI OFF
s = base(); s["sel"] = 2
R("g_sel_muted_off_ref", s, "(ch3 of glance_playing is already muted + OFF; selected here)")
s = base(); s["sel"] = 0; s["channels"][0].update(muted=1, midi_ch=16, snd0=1)
R("g_sel_ch1_muted_off_snd", s, "(selected ch1 muted + OFF + snd0=1)")
s["lane"] = lane_all(rep=1, acc=0, slide=0, vel=60)
R("g_sel_muted_off_lane_full", s)

# (h) preset pending / loading
s = base(); s["preset"] = 16; s["preset_pend"] = 1
R("h_preset_p16_pending_header", s)
s = base(); s["preset"] = 16; s["preset_pend"] = 1; s["toast"] = dict(kind="preset", slot=16, frac=0.3)
R("h_preset_p16_pending_toast", s)
s = base(); s["preset"] = 16; s["preset_pend"] = 1; s["ext"] = 1; s["bpm"] = 126; s["rec"] = 1
R("h_preset_p16_ext126_rec_header", s)

# (i) toasts
for a, nm in enumerate(P.ALGO_FULL):
    s = base(); s["channels"][0]["algo"] = a; s["toast"] = dict(kind="algo", algo=a)
    R("i_toast_algo_%s" % nm, s)
for layer in ("ALL", "NOTES", "ACCNT", "SHAPE", "ARTIC", "HITS"):
    s = base(); s["toast"] = dict(kind="dice", layer=layer, detail="CH 1  NEW PITCHES")
    R("i_toast_dice_%s" % layer, s, "(detail string is the proposal's only one: NOTES)")
for layer in ("ALL", "NOTES", "ACCNT", "SHAPE", "ARTIC", "HITS"):
    s = base(); s["toast"] = dict(kind="undo", detail="DICE " + layer)
    R("i_toast_undo_%s" % layer, s)
