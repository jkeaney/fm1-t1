<!-- SPDX-License-Identifier: GPL-3.0-only -->
# v11 (design session 2026-11-12): after critique v10 (27/40) and the owner's decisions

Render: `python3 ../../mock/mock.py render v11.py --out out && python3 sheets_v11.py`. Interface notes: ../v9/NOTES.md.

- Owner: **MIDI setup cue**, the one standing exception: a just-added channel or a channel with MIDI OFF shows the
  footer `CHN KEY = MIDI  CHn` (inverted) and lights the CHN black key.
- Owner: **messages on the right** (x >= 104): toasts, hold hints and encoder/OCT names never hide play, BPM or the
  channel chips. Text uses AF_M, falling back to AF_S when it does not fit (never cut). Hold hints name the action
  (`RESET`, `HARMONY`, `UNDO LOAD`, `HELP / MENU`) beside the held button's icon; the bar fills to the fire time.
- Owner: **a fixed hit-strip row per channel** (CH1..CH4 top to bottom; the selected and unused rows stay empty).
- Critique v10 fixes: knob rows numbered (1 2 [3]) and `PAGE 1/2`; ARTIC = note lengths (one long gate, three short),
  HITS = drum-notation x heads; CH3 = Okabe-Ito bluish green (was yellow, collapsed with CH1 for deuteranopes);
  ALGO stays the old algorithm on screen until the new one lands (`ALGO  SNAKE AT STEP`); ROW 3 shows the device
  profile's wire names (TB-3 set BITE: CUT RESO ENV-X ENV-Y, HW_SUPPORT_LOWLEVEL.md), from a DEVICES table, not hard-coded;
  REC draws the write cursor; value lists longer than 10 get a meter instead of dots (algos_v11.MAX_DOTS).
- Spec gaps to close in the hand-off: SELECT past the last channel adds one (knob_map says clamped: needs a row);
  HOME hold text vs decision 118 (map while held, keep holding = MENU) is not settled.
