---
id: FB-017
title: P4-012 cut-over: the external clock still follows midi_clock.c, and the panel controls are stubs until P4-013
phase: P4
type: feedback
status: todo
priority: 2
executor: human
depends_on: [P4-012]
gate: none
session: 2026-11-13, host tests only (no device), P4-012 executor
---
## Goal
Triage the two things P4-012 deliberately did not do into tasks, or drop them with a reason.

## Read first
- docs/project/VISION.md
- firmware/src/clk_ext.c (header), firmware/src/seq.c `events_block`, firmware/src/ui_sq_glue.c (header)

## Deliverable
- Linked P-ids for each item below, or a drop reason, recorded in the log.

## Observation
1. **External clock.** P4-012 feeds clk_ext.c (clkx_event for every 0xF8..0xFC and 0xF2, clkx_poll once per block, internal_only from the CLOCK setting) and defines `SQ_IS_MASTER()` as `clkx_master()`, so clock out is silent whenever a host clock is followed. The transport (Start / Continue / Stop) and the tempo (pulse counter, `midi_beat_samples`) of a followed clock still come from the old midi_clock.c. NOT used by the core yet: the CLKX_A_START / CONT / STOP / PULSE actions, SPP + Continue at `clkx.pulse_idx * 8`, `clkx_beat_q4()` as the beat, `clkx_tick_time(j)` as the stamp of tick 8k+j, `clkx_play()` / `clkx_stop()` for the panel. This is a task of its own (replace midi_clock.c's transport by the clkx actions; tests/clock_test.c already covers clk_ext.c + sq_core.c).
2. **Panel controls.** ui_sq_glue.c wires only PLAY (transport) and the page dispatch (stubs). P4-013 must wire: SELECT adds a channel; an M-key (the mute key of a channel) held 1.5 s removes it, held for the preview (tables: build/gen/ui_tables.h `ui_remove_rows`, key_map.tsv, button_modes.tsv, decision 125), the white keys as steps, K1-K4 as morphs (`sq_morph_turn`), the dice buttons. The BPM knob of the old T-1 screen (SELECT) is gone: tempo returns with the SET page (P4-016). Live keys (`sq_live_on/off`, ISR) exist and MIDI IN notes use them; the keys themselves stay silent (song.grid 2) until the PERF page decides what a key means.
3. **Pool and flash.** None. RAM after the cut-over: 28888 B of 73728 (headroom 44840 B).

## Acceptance criteria
- [ ] Converted into backlog items (ids listed in the log) or dropped with a reason.

## Verification
```sh
python3 tools/backlog.py check
```

## UX gate
n/a, triage item; the resulting tasks carry their own Gate A.

## Log
- 2026-11-13: filed by the P4-012 executor.
