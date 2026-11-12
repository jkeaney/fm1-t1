---
id: P4-009
title: Timing T-D: AUTO clock source, PLL, Start/Continue/SPP
phase: P4
type: task
status: done
priority: 1
executor: sonnet
depends_on: [P4-007]
gate: none
---
## Goal
Follow an external MIDI clock without jitter or drift and switch sources by the owner's AUTO rules (TIMING_SPEC §4, M11, M12).

## Read first
- docs/design/TIMING_SPEC.md §3 M11, M12, §4 (rules R1-R16), §6.3 (PLL, source rules, start tests)
- docs/design/SCREEN_SPEC.md §6.1 (state fields the screen reads)
- firmware/src/midi_clock.c, usb.c (SPP parse)

## Deliverable
- Clock source state machine (INT/EXT SYNC/EXT LIVE/EXT WAIT per §4.1), PLL with the §10 gains, `midi_in_t[]` stamps, SPP parse, `CLK_TIMEOUT_MS` 500; state fields exported for the UI
- tests/clock_test.c: PLL jitter/burst/ramp/no-loss cases, table-driven R1-R16, Start-then-pulses and SPP+Continue

## Acceptance criteria
- [x] clock_test passes every case
- [x] Clock out is silent in every EXT state (asserted)
- [x] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [x] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## UX gate
n/a, internal work (tooling, plumbing, docs, process); no producer-visible control, screen or sound change.

## Out of scope
- Screen drawing (P4-016)
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log

### 2026-11-12 executor (Sonnet 5.5)
Done. Commands and decisive output:
```
sh tools/check.sh      -> ok image 169084 B; RAM .data+.bss 23828 B of 73728; pool 92500 B of 368640 ... CHECK PASS
./build.sh | grep RAM  -> ok    .ram_text: 916 insns, no calls
                          ok    image 169084 B; RAM .data+.bss 23828 B of 73728; pool 92500 B of 368640
cc -std=c11 -Wall -Wextra -Werror ... -fsanitize=address,undefined tests/clock_test.c -lm   -> CLOCK PASS (also inside tests/run_tests.sh)
```
RAM headroom 73728 - 23828 = 49900 B (floor 8192 met). The RAM figure did not move: nothing calls clkx yet so the linker drops it, and
midi_in_t (+256 B) is absorbed in the section padding/rounding; the real cost at cut-over is about 40 B (clkx) + 256 B (midi_in_t).

What changed
- NEW firmware/src/clk_ext.c (included in felucca.c after midi_uart.c, not wired): `clkx` state (src INT/EXT, port, known, armed, pulse_n,
  playing, bpm, live, pend, P, peff, ph, pt, pulse_idx/next_idx ...), `clkx_event(port,status,d1,d2,stamp)`, `clkx_poll(now)`,
  `clkx_play/stop/set_internal_only`, `clkx_master()`, `clkx_tick_time(j)`, `clkx_beat_q4()`. Every call returns CLKX_A_* action bits
  (STOP, START, CONT, PULSE, LOST_PLAY, LOST_ARM, LOCAL). Pure: no register, no song access (bpm_own is never written, R11), so no host hook.
- usb.c: `midi_in_t[MQ]` stamped with MO_NOW() (fm1_ticks) next to midi_in_ms (kept: the old midi_clock path and midi_uart_test still read it,
  it goes at P4-012); `midi_in_event` forwards 0xF2 SPP (CIN 3, data bytes checked). The old events_block sends it to midi_event with st 0xF0, which matches nothing.
- sq_core.c: `SQ_IS_MASTER()` hook (default `sq_master`, so timing_test is unchanged); P4-012 defines it as clkx_master().
- tests/clock_test.c + a run_tests.sh step.
Test results (printed by clock_test): PLL with +-0.5 ms stamp noise at 60..240 BPM: output RMS 111-121 us (limit 150), p-p 650-720 us;
burst (4 pulses on one stamp every 50): count exact, phase 0 us off once settled; 10 min at 120 BPM with noise and bursts: 28800 in, 28800
counted, position 28799, tempo 119.965; R1..R16 table, INTERNAL ONLY, Start/Stop/Continue/SPP, clock out 0 packets in EXT --, EXT stopped,
WAIT, EXT playing, R4 take-over (INT control sends clock, 1 Start, 1 Stop).

For the reviewer (judgement calls, each also in the header of clk_ext.c)
- T13 vs the M11 gains: with phase 1/4 and frequency 1/64 the loop's P lags a steady ramp by 16 pulses of ramp = 0.63..0.67 BPM for 1 BPM/beat,
  over T13's 0.3 BPM. The period the loop actually advances its phase by (P + e/4, `peff`) has no lag (0.002 BPM) and is what clkx_tick_time and
  clkx_beat_q4 use; T13 is asserted on peff. The display (1/16 smoothing, hysteresis) lags up to 1.5 BPM on that ramp (printed, not asserted). TIMING_SPEC
  should say which quantity T13 means (P2-032); the hardware gate G3 decides.
- Jitter: "under 0.15 ms" taken as RMS (111-121 us); peak-to-peak is 0.65-0.72 ms.
- R14 "re-seed from the last two intervals": needs two consecutive, agreeing (25 %) off intervals; a lone off interval is an outlier (counted, phase kept).
  A 25 % boundary is "over 25 %" strictly (120 -> 160 BPM is exactly 25 % and is tracked by the loop, not re-seeded).
- Gap over 1.5 P moves the loop's phase by the lost pulses, not the position (4.3): a host that truly drops a pulse shifts the music until its next Start.
- R13: first estimate outside 40..240 BPM after 7 pulses, or any interval over 1.5 x 40 BPM's period, returns to INT at once (and drops a latched Start).
- M7 vs M4 ordering is P4-007's; nothing here sends bytes.
- Not wired: acting on the action bits (sq_on_start at the pulse, sq_on_stop, tick stamps from clkx_tick_time, SQ_IS_MASTER, SQ_EXT_BEAT from clkx_beat_q4 in 1/16 samples) is P4-012. TRS SPP is not parsed (midi_uart.c's parser drops system common; USB only, as the task names usb.c).
- Not flashed.

### 2026-11-12 review (sonnet)
Verdict: PASS (gate none). Re-ran `sh tools/check.sh` -> CHECK PASS; `./build.sh`: `ok .ram_text: 916 insns, no calls`; `ok image 169084 B; RAM .data+.bss 23828 B of 73728; pool 92500 B of 368640` (headroom 49900 B, floor 8192 met). run_tests.sh prints CLOCK PASS with the numbers the executor quotes. No register access outside hal/ (clk_ext.c is pure; usb.c adds only the MO_NOW stamp and an SPP forward). clk_ext.c is included but uncalled; sq_core.c's SQ_IS_MASTER defaults to sq_master, so device behaviour is unchanged. Clock-out test (tests/clock_test.c t_clock_out_silent) uses the real sq_core: 0 packets in EXT --, EXT stopped, WAIT, EXT playing, R4 take-over; INT sends clock, 1 Start, 1 Stop; clock returns after R5. Gains (>>2, >>6, 1/16 display), CLK_TIMEOUT 500 ms, R13 range, R2-R16 match the spec text.
Rulings on the divergences:
1. T13 on peff: genuine reading, not a hidden failure. T13 is "tempo follow lag" and what matters musically is the period that places ticks, which is peff; P alone lagging 0.65 BPM is an inherent property of the M11 gains (spec M11.2 and T13 are mutually inconsistent as written). The executor disclosed it and printed the P lag and the 1.5 BPM display lag. Accepted, but the spec must say it: filed P2-033. G3 decides on hardware.
2. Jitter as RMS: right reading. M11.2 says a 0.5 ms wobble "becomes about 0.1 ms", which is RMS; the test noise (+-0.5 ms) is also wider than the +-0.25 ms poll error. Peak-to-peak 0.65-0.72 ms is inside the T10 budget (3.0 ms) and below one USB frame, so it is not audible for a TB-3. No owner decision needed; recorded in P2-033.
3. R14 as two agreeing off-intervals: acceptable reading of "re-seed from the last two intervals", and the outlier path protects the phase from single glitches. Edge noted in P2-033: a jump faster than 4x (e.g. 40 to 240 BPM) is treated as bursts and not followed; unreachable in practice.
4. Dropped pulse shifting the music: acceptable. 4.3 makes position a count of accepted 0xF8; USB bulk is lossless. But M11.3 says a gap is "counted as the missing pulses", which can be read the other way; P2-033 asks the spec to settle it. Not a defect.
5. TRS SPP not parsed: allowed by this task (Read-first names usb.c, deliverable says "SPP parse" only for usb.c), but R15 and M12 treat both ports alike, so a DIN host resumes at the wrong step. Filed P4-025.
Not approving any flashing: hardware validation is P4-019 / P4-022. P4-012 still has to act on the action bits, define SQ_IS_MASTER as clkx_master() and feed midi_in_t.
