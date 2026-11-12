# S06: Ableton Live is the clock master, the clock is lost, then the FM-1 is the master with clock out

Walks SEQUENCER_UX 2.8, SCREEN_SPEC 6 and TIMING_SPEC 4 to 5 literally. Setup: FM-1 USB-C -> Mac running Ableton Live;
Ableton routes the FM-1 notes to the TB-3 through a USB-MIDI host box; CH1 ACID on MIDI channel 2; the FM-1 is stopped, BPM
`120`, no clock heard since power-on. Rows marked HOST are Ableton actions, not FM-1 controls. Notation as S01. Rule ids R1 to R15 are TIMING_SPEC 4.2; M12 is TIMING_SPEC section 3.

**Screen words after P4-016 (SCREEN_SPEC_V2 4.1, 4.7; owner-approved 2026-11-13): the header has no clock dot and the cue strip is the footer line.** Read `HD o INT n` as the header `n`, `EXT n` as `EXT` over `n`, `EXT --` (the first pulses) as `EXT` over the tempo (v12 draws no separate first-pulses state; the SET K1 tile reads `HOST --` until the tempo is known), `WAIT` as `EXT` over `WAIT`, the lost state as `EXT` over `--` in red. The SET rows below (3, 4, 8, 10, 11, 12) are written in the as-built SET screen; the other rows keep their older words until the external clock task (P4-031) re-draws them.

Dot legend (SCREEN_SPEC 6.3, superseded): amber solid = internal clock; cyan with a 2 x 2 centre hole = a host clock drives the
tempo; cyan ring with a 4 x 4 hole blinking at 2 Hz = armed, waiting for the host.

| Step | User action | Exact screen text | MIDI out | Row ids |
| --- | --- | --- | --- | --- |
| 1 | Baseline: look at the FM-1, stopped | header stop square, `120`; CUE `PLAY: start   K1-K4: morph`; PLAY green LED dark | none (a stopped FM-1 sends no clock, M7) | - |
| 2 | HOST: set Ableton to send MIDI clock to the FM-1 at 126 BPM and press Play in Ableton | header `EXT` over `126` (the first 7 pulses, 139 ms at 126 BPM, are not a separate state in v12); transport `>`; PLAY green LED mid steady with a beat flash; CUE `TAP STEP=ON/OFF  HOLD=EDIT NOTE` | host to FM-1: `FA`, then `F8`; FM-1 to host: no clock (R12), only notes; step 1 starts at the first `F8` after `FA` (M12), `ch2 ON/OFF` pairs of the ACID line at 126 BPM | - |
| 3 | Tap GLO | Page SET. Header `EXT` over `126`; K1 tile `HOST 126` (read-only: no fill, dashed frame, MID digits) `SWING 50%` `GATE 50%` `DIV 1/16`; status line `P01` `EXT USB` `OUT FOLLOW` `OCT 0`; footer `HOST SETS TEMPO` / `GLO = BACK` | unchanged | B_GLO/PERF |
| 4 | Turn K1 three detents clockwise | nothing changes; the K1 tile stays `HOST 126` and frames white; slot `K1 HOST SETS TEMPO`; the user's own tempo `120` is untouched (R11; required of P4-031, today midi_clock.c overwrites it) | unchanged | EN_K1/SET |
| 5 | HOST: change the Ableton tempo to 130 | within about three beats the header `EXT` over `130` (at most 4 display updates a second, hysteresis of 1 BPM) | notes follow the new tempo; the position never skips (R14, count-locked) | - |
| 6 | Tap GLO (BACK) | PERF; HD `EXT 130` | unchanged | B_GLO/SET |
| 7 | Press PLAY on the FM-1 while it follows a running host | HD `.` (stopped locally); source stays EXT, HD `EXT 130`; PLAY's own LED flashes on the host beat; CUE `PLAY: start   K1-K4: morph` | `ch2 OFF` for sounding notes, `B1 7B 00`; no `FC` is sent (R10, not the master) | B_PLAY/PERF |
| 8 | Press PLAY again | header `EXT` over `WAIT`; transport `.`; PLAY green LED full and dark at 2 Hz; CUE `WAIT FOR HOST START  PLAY=cancel` (198 px); on SET the K1 tile reads `HOST --` (or the host tempo if known), the footer `WAIT FOR HOST START` / `PLAY=CANCEL` (AF_S) | none; the host is mid-song and sends no Start, so the FM-1 waits (R8). PLAY again cancels the wait; nothing else starts it (owner decision 76: no hold-to-start-internal, FINDINGS Q2 answered) | B_PLAY/PERF |
| 9 | HOST: stop and start Ableton from the top | on the host `FA`: header `>` and `EXT` over `130` | step 1 on the first `F8` after `FA`; `ch2` notes | - |
| 10 | HOST: quit Ableton (the clock stops without a stop message) | for 500 ms the display is unchanged; then the transport stops: header `.`, `120` (the own tempo; required of P4-031); CUE `CLOCK LOST: STOPPED` for 3 s (row 10b, above the stopped row 13 since P2-011: F-01 fixed); SET shows the tile `BPM 120` (the own tempo, back in the same frame), `OUT ON PLAY` (stopped; F-08 fixed) and the footer `KEYS = STEPS` / `GLO = BACK`; while the clock is lost the header reads `EXT` over `--` in red, the K1 tile `HOST --` and the footer `CLOCK LOST: STOPPED` / `PLAY=START` (red) | `ch2 OFF` for sounding notes, CC123 `B1 7B 00` (R6, M8); the sequencer does not continue on the internal clock | - |
| 11 | Tap GLO, turn K1 four detents clockwise | tile `BPM 124`; header `124`; status line `OUT ON PLAY` (stopped) | none | B_GLO/PERF, EN_K1/SET |
| 12 | Press PLAY | header `>` and `124`; SET status line now `OUT MASTER` (playing); PLAY green LED mid steady with a beat flash | `FA`; `F8` x24 per quarter note at 124 BPM; `ch2 ON/OFF` pairs of the ACID line; the preview synth plays CH1 | B_PLAY/SET |
| 13 | HOST: set Ableton to follow the FM-1 clock; HOST: Ableton echoes the received clock back to the FM-1 | HD unchanged (`INT 124`): a `F8` that arrives while the FM-1 plays as master is ignored (R3) | host to FM-1: `F8` stream, ignored; FM-1 to host: the clock continues | - |
| 14 | HOST: press Play in Ableton while the FM-1 is the master | the host takes over (R4): the FM-1 stops, header `.`, then restarts at step 1 on the next pulse; header `EXT` over `126`; clock out stops | `ch2 OFF`, `B1 7B 00`; then no clock out; notes resume on the host beat | - |
| 15 | HOST: stop Ableton (the clock stops), wait 500 ms | the header returns to `124`; the user's tempo 124 is kept (required of P4-031) | none | - |
| 16 | Press PLAY, then PLAY again to stop | `>` then `.`; the header keeps `124` in both states | start: `FA`, `F8` stream; stop: `FC`, `ch2 OFF`, `B1 7B 00`; after the stop no clock bytes | B_PLAY/SET |
| 17 | Tap GLO (BACK) | PERF; HD `INT 124` | none | B_GLO/SET |

Result against the locked tables:
- Every source state (INT, EXT SYNC, EXT LIVE, WAIT) and every transition of R1 to R10 is readable from the header, the
  SET status lines and the PLAY LED, so rule 1 of the vision holds for the clock.
- Gaps found by this walk, all fixed by P2-011 (see FINDINGS): stale and conflicting cue rows 13 / 13a / 13b (F-01, now rows
  10a / 10b above row 13); a host Start that arrives while the FM-1 is stopped on its own clock, before any pulse, was not in the
  rule table (F-07, now TIMING_SPEC R16); `OUT MASTER` was drawn while stopped although nothing is sent (F-08, now `OUT ON PLAY`);
  R13 said a tempo outside the range is "not accepted" and also that `EXT --` stays (F-18, now `EXT --` for 7 pulses, then `INT`).
  The WAIT exit (old Q2) and the CLOCK menu row (old Q3) were answered by owner decision 76: PLAY again cancels; the row is
  AUTO / INTERNAL ONLY.
