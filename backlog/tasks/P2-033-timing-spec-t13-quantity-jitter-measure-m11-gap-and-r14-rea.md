---
id: P2-033
title: TIMING_SPEC fixes from the P4-009 review (T13 quantity, jitter measure, M11.3 gap, R14 reading)
phase: P2
type: task
status: in_progress
priority: 3
executor: sonnet
depends_on: []
gate: none
---
## Goal
Make docs/design/TIMING_SPEC.md say which quantity each PLL target measures, so the G3 hardware gate (P4-022) checks what clk_ext.c and tests/clock_test.c assert.

## Read first
- docs/design/TIMING_SPEC.md M11 (line ~281), T10, T13 (line ~154), R14 (line ~347), section 6.3 PLL row (line ~468)
- firmware/src/clk_ext.c header comment and tests/clock_test.c (t_pll_jitter, t_pll_ramp)

## Deliverable
- T13 / M11.2: state that "tempo within 0.3 BPM" is the period the loop advances its phase by (P + e/4, `clkx.peff`), which is what places ticks; the loop's P alone lags a 1 BPM/beat ramp by about 0.65 BPM with the 1/4 and 1/64 gains, and the shown integer BPM may lag up to 1.5 BPM. Say whether the display lag is accepted.
- M11 / section 6.3: state that "output jitter under 0.15 ms" is RMS with +-0.5 ms uniform input noise (measured 111-121 us RMS, 0.65-0.72 ms peak-to-peak) and that peak-to-peak sits inside the T10 budget (3.0 ms).
- M11.3: a gap over 1.5 P is "counted as the missing pulses": say whether that means position (contradicts 4.3 "pulse count, exact") or only loop phase. The code credits the phase only, so a host that drops a pulse shifts the music until its next Start; state that this is accepted (USB bulk is lossless; a stalled host is not a pulse the host counted).
- R14: say "two consecutive agreeing intervals (within 25 %)" re-seed P, and that exactly 25 % is tracked by the loop. Note a jump faster than 4x (e.g. 40 to 240 BPM) is treated as bursts (dt < P/4) and is not followed.

## Acceptance criteria
- [ ] Each of the four items is written into TIMING_SPEC.md
- [ ] `sh tools/check.sh` ends with `CHECK PASS`

## Verification
```sh
sh tools/check.sh
```

## UX gate
n/a, internal work (docs); no producer-visible change.

## Out of scope
- Any firmware change.

## Log
<!-- filed by the P4-009 review -->
