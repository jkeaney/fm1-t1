<!-- SPDX-License-Identifier: GPL-3.0-only -->
# v10 (design session 2026-11-12): after critique v9 (24/40, no P0) and the owner's decisions

Render: `python3 ../../mock/mock.py render v10.py --out out && python3 sheets_v10.py`. Builds on v9 (see ../v9/NOTES.md
for the algorithm interface; `algos_v10.py` is the only per-algorithm source).

- Owner: encoders and OCT named **only while touched**: the top bar shows the printed name (boxed) and what it does now
  (`SELECT  CH2 EUCLID`, `ALGO  SNAKE AT NEXT STEP`, `PRESETS  P02 AT NEXT BAR`, `OCT+  ROW 2 OF 2`, `OCT+  ROW 3: RIG`).
- Owner: **button tiles shrunk** (24 -> 17 px, icons unchanged): music row 33 -> 45 px, hit strips 2 -> 3 px.
- Hold hints for GLO (`HOLD GLO: HARMONY`) and SAVE (`HOLD SAVE: UNDO LOAD`) drawn; ROW 3 = RIG wires (knob_map R1-R4).
- REC: the cue is `PLAY KEYS TO WRITE` + `REC = DONE`; REC while playing = the red dot with a play mark.
- Fixes: pending pill no longer meets the playhead bar; muted digit MID (was DIM, 2.69:1); knob meters get a full-colour
  level edge at both sides of the tile.
