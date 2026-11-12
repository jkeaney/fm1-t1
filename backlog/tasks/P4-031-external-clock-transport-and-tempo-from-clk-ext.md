---
id: P4-031
title: External clock: transport and tempo of a followed clock come from clk_ext, not midi_clock.c
phase: P4
type: task
status: todo
priority: 2
executor: sonnet
depends_on: [P4-012]
gate: none
---
## Goal
When the FM-1 follows a host or TRS clock, Start / Continue / Stop and tempo are driven by clk_ext.c (the AUTO clock/PLL) and the core, so playback stays locked to the external clock (TIMING_SPEC). Today P4-012 only silences clock out; the old midi_clock.c still decides transport and tempo.

## Read first
- backlog/feedback/FB-017-*.md item 1
- firmware/src/clk_ext.c (header), firmware/src/seq.c `events_block`, firmware/src/midi_clock.c
- docs/design/TIMING_SPEC.md; tests/clock_test.c

## Deliverable
- Use CLKX_A_START / CONT / STOP / PULSE, SPP + Continue at `clkx.pulse_idx * 8`, `clkx_beat_q4()` as the beat, `clkx_tick_time(j)` as the stamp of tick 8k+j, and `clkx_play()` / `clkx_stop()` for the panel, in the core path.
- Remove the transport/tempo duties of midi_clock.c that this replaces.
- Host tests for each action (extend tests/clock_test.c / sq_link_test.c).

## Acceptance criteria
- [ ] A followed clock starts, continues and stops the core; tempo follows the PLL.
- [ ] Internal clock behaviour is unchanged (existing tests pass).
- [ ] `sh tools/check.sh` ends CHECK PASS; RAM headroom >= 8 KB.

## Verification
```sh
sh tools/check.sh
```

## UX gate
Producer-visible only through the header tempo/EXT/WAIT/lost indicator already specified in SCREEN_SPEC_V2 (drawn by P4-028). Use case: docs/design/features/P4-031-external-clock.md; Gate A (steps: connect a host clock, press PLAY on the host, see EXT BPM, stop the host, see WAIT then lost) filled before the build, Gate B after.

## Out of scope
- Flashing. Hardware verification (Ableton sync check, decision 83) is part of HW-T / HW-1 follow-up.

## Log
- 2026-11-13: filed from FB-017 item 1.
