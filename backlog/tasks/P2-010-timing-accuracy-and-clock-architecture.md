---
id: P2-010
title: Timing accuracy and clock architecture: tight MIDI timing from any clock source, with the source always visible
phase: P2
type: task
status: todo
priority: 0
executor: sonnet
depends_on: [P2-004]
gate: human-review
---
## Goal
Owner requirement (2026-11-10): "make sure our code has all the tools to ensure timing is accurate, no matter where the clock is set, and that it's really obvious to the user the clock and where it's coming from." Design (not build) how the firmware keeps MIDI out tight and how clock source is shown, so dev tasks can implement and the hardware test can prove it. Trigger: notes recorded into Ableton from the FM-1 looked loose.

## Read first
- firmware/src/t1_glue.c (t1_block: ticks generated per control block, MIDI flushed once per block), torso_t1_pipeline.c (micro-queue, tick evaluator), midi_clock.c, seq.c (events_block, beat_samples), usb.c (midi_out_event, 64-entry ring, no timestamps)
- docs/design/SCREEN_SPEC.md section 6 and docs/project/decisions/ADR-0003-sequencer-core.md 7.4 (tempo states INT/EXT LIVE/EXT SYNC/EXT WAIT; clock out master/follower)
- backlog/feedback/FB-001-show-external-clock-sync-and-optional-clock-out.md
- docs/project/decisions/ADR-0004-design-lock.md

## Deliverable
- docs/design/TIMING_SPEC.md and an ADR-0003 addendum covering:
  1. Measured/derived facts: control block length in samples and ms, worst-case added latency and jitter on every path (tick to ring to USB packet to host), and how the ISR flush works today.
  2. The timing budget and target (for example worst-case jitter in ms, constant latency, no drift), with reasoning.
  3. Mechanisms to hit it: where notes are emitted inside the block (sub-block scheduling by sample offset), flush frequency, USB ring sizing/priority, ordering of note-off before note-on, clock-out pulses (24 PPQN) placed at exact sample positions, Start/Stop alignment, swing/delay applied before scheduling so they are intentional only, and how external-clock following avoids drift (the existing 500 ms timeout, PLL/averaging, jitter filtering, tempo smoothing).
  4. Clock source model: INT master, EXT follow, hand-over rules, what happens when the clock stops or jumps, and how the sequencer position stays locked (Start/Continue/SPP handling).
  5. A visibility spec that makes the clock and its source obvious on every page (a persistent clock badge with source and BPM, LED behaviour, a beat pulse), consistent with SCREEN_SPEC section 6; add rows to SCREEN_SPEC/page_states.tsv if needed.
  6. A measurement plan: extend tools/fm1_midi_listen.py (or a new tool) to capture timestamped MIDI and report inter-note jitter and clock-out accuracy; define host tests where timing logic can be tested, and a hardware-test gate (FM-1 to Ableton with sync on/off, FM-1 to TB-3) with pass thresholds.
  6b. Test mode (owner, 2026-11-10: "we can test it properly once our own firmware is in test mode"): specify a firmware test mode (a build flag or a hidden menu entry) that plays a fixed timing pattern (for example straight 16ths on one note, a known BPM, clock out on) and can follow or send clock, so the measurement tool can compare expected against received timestamps with no musical variation. Define what it plays, how it is entered, that it costs no RAM in the normal build, and which dev task implements it.
  7. RAM/CPU cost and which dev tasks (E4/E5) must implement it.
- Owner questions listed with options.

## Acceptance criteria
- [ ] The five areas above are specified with numbers (block length, jitter target, clock-out accuracy), not adjectives
- [ ] Every claim about current behaviour cites source lines
- [ ] Visibility spec added to the screen docs without changing locked UX decisions
- [ ] A hardware-test gate with thresholds is defined, run on our own firmware in its test mode
- [ ] `python3 tools/check_sequencer_ux.py`, `tools/check_control_map.py` and `backlog.py check` pass
- [ ] Owner approval

## Verification
```sh
python3 tools/check_sequencer_ux.py && python3 tools/check_control_map.py
python3 tools/backlog.py check
```

## Out of scope
- Implementing any firmware change.

## Log
- 2026-11-10 created by the orchestrator from owner feedback after a loose Ableton recording.
- 2026-11-10 owner: timing will be tested properly once our own firmware has a test mode; added to the deliverable (6b).
