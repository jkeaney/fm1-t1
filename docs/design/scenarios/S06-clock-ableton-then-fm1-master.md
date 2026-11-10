# S06: Ableton Live is the clock master, the clock is lost, then the FM-1 is the master with clock out

Walks SEQUENCER_UX 2.8, SCREEN_SPEC 6 and TIMING_SPEC 4 to 5 literally. Setup: FM-1 USB-C -> Mac running Ableton Live;
Ableton routes the FM-1 notes to the TB-3 through a USB-MIDI host box; CH1 ACID on MIDI channel 2; the FM-1 is stopped, BPM
`120`, no clock heard since power-on. Rows marked HOST are Ableton actions, not FM-1 controls. Notation as S01; `o`
is the clock dot. Rule ids R1 to R15 are TIMING_SPEC 4.2; M12 is TIMING_SPEC section 3.

Dot legend (SCREEN_SPEC 6.3): amber solid = internal clock; cyan with a 2 x 2 centre hole = a host clock drives the
tempo; cyan ring with a 4 x 4 hole blinking at 2 Hz = armed, waiting for the host.

| Step | User action | Exact screen text | MIDI out | Row ids |
| --- | --- | --- | --- | --- |
| 1 | Baseline: look at the FM-1, stopped | HD `.` `o`amber solid, blinking on the internal beat, `INT 120`; CUE `PLAY: start   K1-K4: morph`; PLAY green LED dark | none (a stopped FM-1 sends no clock, M7) | - |
| 2 | HOST: set Ableton to send MIDI clock to the FM-1 at 126 BPM and press Play in Ableton | HD for the first 7 pulses (146 ms at 126 BPM): `o`cyan with a hole, steady dim, `EXT --`; then `o`cyan with a hole blinking on the host beat, `EXT 126` (T_ACCENT); transport `>`; PLAY green LED mid steady with a beat flash; CUE `TAP STEP=ON/OFF  HOLD=EDIT NOTE` | host to FM-1: `FA`, then `F8`; FM-1 to host: no clock (R12), only notes; step 1 starts at the first `F8` after `FA` (M12), `ch2 ON/OFF` pairs of the ACID line at 126 BPM | - |
| 3 | Tap GLO | Page SET. K1R `BPM EXT 126` (read-only, T_ACCENT) `SWING 50` `GATE 50` `DIV 1/16`; status line 1 `TEMPO EXT 126 USB` and `OUT FOLLOW`; status line 2 `K1 off: the host sets tempo`; GLO cell reads `BACK` (hold mark `SCALE`); CUE `EXT: K1 OFF, HOST SETS TEMPO` | unchanged | B_GLO/PERF |
| 4 | Turn K1 three detents clockwise | nothing changes; K1 cell stays `EXT 126`; CUE stays `EXT: K1 OFF, HOST SETS TEMPO`; the user's own tempo `120` is untouched (R11) | unchanged | EN_K1/SET |
| 5 | HOST: change the Ableton tempo to 130 | within about three beats HD `EXT 130` (at most 4 display updates a second, hysteresis of 1 BPM); the dot keeps blinking on the host beat | notes follow the new tempo; the position never skips (R14, count-locked) | - |
| 6 | Tap GLO (BACK) | PERF; HD `EXT 130` | unchanged | B_GLO/SET |
| 7 | Press PLAY on the FM-1 while it follows a running host | HD `.` (stopped locally); source stays EXT, HD `EXT 130`; PLAY's own LED flashes on the host beat; CUE `PLAY: start   K1-K4: morph` | `ch2 OFF` for sounding notes, `B1 7B 00`; no `FC` is sent (R10, not the master) | B_PLAY/PERF |
| 8 | Press PLAY again | HD `o`cyan ring blinking at 2 Hz, `WAIT` (T_ACCENT); transport `.`; PLAY green LED full and dark at 2 Hz; CUE `WAIT FOR HOST START  PLAY=cancel` (198 px); on SET the status line 2 reads `WAITING FOR HOST START` | none; the host is mid-song and sends no Start, so the FM-1 waits (R8): there is no way to start on the internal clock from here (FINDINGS Q2) | B_PLAY/PERF |
| 9 | HOST: stop and start Ableton from the top | on the host `FA`: HD `>` and `EXT 130`; the ring becomes the beat dot | step 1 on the first `F8` after `FA`; `ch2` notes | - |
| 10 | HOST: quit Ableton (the clock stops without a stop message) | for 500 ms the display is unchanged; then the transport stops: HD `.`, the dot turns amber solid, `INT 120`; CUE `CLOCK LOST: STOPPED` for 3 s (but see F-01: the stopped strip row 13 outranks it); SET shows `BPM 120`, `TEMPO INT 120` and `OUT MASTER`, `host clock takes over` | `ch2 OFF` for sounding notes, CC123 `B1 7B 00` (R6, M8); the sequencer does not continue on the internal clock | - |
| 11 | Tap GLO, turn K1 four detents clockwise | K1R `BPM 124`; HD `INT 124`; status line 1 `TEMPO INT 124` / `OUT MASTER` | none | B_GLO/PERF, EN_K1/SET |
| 12 | Press PLAY | HD `>` and amber dot blinking on the internal beat; SET status unchanged `OUT MASTER`; PLAY green LED mid steady with a beat flash | `FA`; `F8` x24 per quarter note at 124 BPM; `ch2 ON/OFF` pairs of the ACID line; the preview synth plays CH1 | B_PLAY/SET |
| 13 | HOST: set Ableton to follow the FM-1 clock; HOST: Ableton echoes the received clock back to the FM-1 | HD unchanged (`INT 124`): a `F8` that arrives while the FM-1 plays as master is ignored (R3) | host to FM-1: `F8` stream, ignored; FM-1 to host: the clock continues | - |
| 14 | HOST: press Play in Ableton while the FM-1 is the master | the host takes over (R4): the FM-1 stops, HD `.`, then restarts at step 1 on the next pulse; HD `o`cyan with a hole, `EXT --` then `EXT 126`; clock out stops | `ch2 OFF`, `B1 7B 00`; then no clock out; notes resume on the host beat | - |
| 15 | HOST: stop Ableton (the clock stops), wait 500 ms | HD returns to `INT 124` with the amber dot; the user's tempo 124 is kept | none | - |
| 16 | Press PLAY, then PLAY again to stop | `>` then `.`; the amber dot keeps blinking on the internal beat in both states | start: `FA`, `F8` stream; stop: `FC`, `ch2 OFF`, `B1 7B 00`; after the stop no clock bytes | B_PLAY/SET |
| 17 | Tap GLO (BACK) | PERF; HD `INT 124` | none | B_GLO/SET |

Result against the locked tables:
- Every source state (INT, EXT SYNC, EXT LIVE, WAIT) and every transition of R1 to R10 is readable from the header, the
  dot, the SET status lines and the PLAY LED, so rule 1 of the vision holds for the clock.
- Gaps: stale and conflicting cue rows 13 / 13a / 13b (F-01); a host Start that arrives while the FM-1 is stopped on its
  own clock, before any pulse, is not in the rule table (F-07); `OUT MASTER` is drawn while stopped although nothing is
  sent (F-08); R13 says a tempo outside the range is "not accepted" and also that `EXT --` stays (F-18); the WAIT
  state has no exit except PLAY or the host (Q2).
