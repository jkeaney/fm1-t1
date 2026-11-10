---
id: P2-010
title: Timing accuracy and clock architecture: tight MIDI timing from any clock source, with the source always visible
phase: P2
type: task
status: done
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

## Owner rulings to implement in the spec (2026-11-10)
- Tempo source AUTO: internal by default (own tempo kept separately, never overwritten); if a MIDI clock is heard it becomes the tempo source, and when it stops return to internal.
- K1 inert while external clock drives. Clock out automatic when internal master, no menu row.
- PLAY with external source and no host start = labelled wait for the clock to start.
- Visibility: blinking, colour-coded dots showing tempo (blink on the beat) and source (internal vs external). Specify colours, size, placement on every page, and the LED behaviour. Supersedes SCREEN_SPEC section 6 where they differ (update SCREEN_SPEC/page_states.tsv/ADR-0003 7.4 to match).

## Acceptance criteria
- [x] The five areas above are specified with numbers (block length, jitter target, clock-out accuracy), not adjectives
- [x] Every claim about current behaviour cites source lines
- [x] Visibility spec added to the screen docs without changing locked UX decisions
- [x] A hardware-test gate with thresholds is defined, run on our own firmware in its test mode
- [x] `python3 tools/check_sequencer_ux.py`, `tools/check_control_map.py` and `backlog.py check` pass
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

### 2026-11-10 executor (sonnet)
Deliverables: `docs/design/TIMING_SPEC.md` (new), ADR-0003 section 19 addendum plus inline pointers in 7.1, 5 and 7.4, SCREEN_SPEC section 6 rewritten to the owner rulings (2.1 H2, 5, 6, 7, 8, 10, 11 updated), `page_states.tsv` (PERF and SET screen text), SEQUENCER_UX 2.8 / 11 / header line. No Approved line written; no firmware changed; no 20-minute suite.

Measured (probe compiled against `build/gen/felucca_tables.h`, widths with the real `text_w` from `gfx.c`):
- `FS=44100 CTL=32 HALF_FRAMES=128`: control block 32 samples = **0.7256 ms**; one audio interrupt = 4 blocks = 2.9025 ms; DAC rate 24 MHz / 544 = 44117.647 Hz = **+400.2 ppm** against `FS` (24.0 ms per minute at any BPM). Ticks per block 0.093 (40 BPM) .. 0.557 (240 BPM).
- Header strings: `INT 120` 42, `EXT 126` 45, `EXT --` 34, `WAIT` 28, cue `WAIT FOR HOST START  PLAY=cancel` 198 px of 240.

Findings the reviewer should weigh (all in TIMING_SPEC, cited to lines):
1. ADR-0003 7.1's "jitter at most one block (0.73 ms)" is wrong on the wire: `audio.c:116-120` renders 4 blocks per interrupt, `main.c:42-52` defers `usb_poll`/`ep1_tx` until the interrupt ends, so a half's notes leave together: ~3.4 ms p-p on the FM-1 side (+1 ms USB frame). Corrected in the addendum.
2. Internal tempo +400 ppm (`fx.c:221` vs `audio.c:9`); marked unconfirmed until the test image measures the real rate.
3. Today tick 0 on an external Start runs at the Start message, not the first `0xF8` (`midi_clock.c:13-18,68-77`, `t1_glue.c:117-118`); `0xF2` SPP is dropped (`usb.c:821-824`); no clock out exists.
4. Risk R1 flagged in M3: releasing MIDI from TIMER5 while nested in the audio interrupt touches the SIE index; fallback specified.
Owner questions: TIMING_SPEC section 11 (7 items; recommended answers given). Changes that go beyond the four rulings and need the owner: CLOCK menu row after AUTO (Q1), clock lost while playing stops (Q2), host Start takes over a playing FM-1 (Q4), PLAY green LED becomes mid-steady with beat flash (Q5), separate timing-test build (Q6).

Verification run:
```
python3 tools/check_sequencer_ux.py   -> pages=9 button-rows=254 knob-rows=95 key-rows=307 control-rows=441 scored=69 exceptions=3 / OK
python3 tools/check_control_map.py     -> rows tsv=441 md=441 / OK
python3 tools/backlog.py check         -> 40 items, 0 errors
```
Owner approval criterion left unticked (human-review gate).

### 2026-11-10 review (sonnet)
Verdict: PASS on design review. Left at `review`. Awaiting owner: read TIMING_SPEC section 0 and the section 11 questions (Q1 CLOCK menu row, Q2 clock lost while playing stops, Q4 host Start takes over, Q5 PLAY LED mid level, Q6 separate timing-test build), then write the Approved line. No Approved line written by the reviewer; no 20-minute suite (docs only).

Re-run: check_sequencer_ux.py OK (control-rows=441), check_control_map.py OK (441/441), backlog.py check 0 errors.

Verified against source:
- CTL=32 (tools/gen_tables.py:14), FS=44100, HALF_FRAMES=128 (core.h:15), loop `for b += CTL` audio.c:116-120 gives 4 blocks per interrupt; events_block per block fx.c:929. 32/44100 = 0.7256 ms correct.
- main.c:42-52: nested TIMER5 returns before usb_poll when in_audio, so the ADR-0003 7.1 "at most one block" claim is indeed wrong on the wire; correction is sound.
- Clock arithmetic: 24e6/544 = 44117.647; ratio 1.000400; 24 ms per minute correct. The only evidence is the audio.c:9 comment and DAC_TICKS use, and the spec flags it unconfirmed and gives a test-image measurement; that is the right handling.
- Start (midi_clock.c:13-18, t1_glue.c:117-118), 0xF8/FA/FB/FC only forwarded and SPP dropped (usb.c:821-824), BPM overwrite (midi_clock.c:65), fx.c:221 FS use: citations match.
- R1: usb.c:5 does state all SIE access is in the TIMER5 handler; risk and fallback (release between audio interrupts, T1 becomes render time) are specified and gated by G6.
- Jitter targets (T1 0.25 ms, T2 1.5 ms, T5 100 ppm etc.) numeric, each with a derivation, hard limit and current figure; gates G1-G7 have thresholds and run on the timing-test image (zero RAM in the normal build).
- Locked UX: diff touches only page_states.tsv (PERF header and SET status text), SEQUENCER_UX 2.8 / header line / lock item 12; no knob_map/button_map change; SET K1 inert kept. Owner rulings (AUTO, own tempo kept, K1 inert in EXT, automatic clock out, PLAY waits, colour-coded blinking dots, test mode) are all present.

Minor, non-blocking (fix when the owner next touches it):
1. TIMING_SPEC.md:40 says cost "about 1 KB .bss" but the table at :518 totals "+0.6 KB"; reconcile.
2. R1 reasoning: the spec should also state that audio can preempt a running usb_poll and TIMER5 must not re-enter it (guard) when the nested release is added; the dev task should test that, not only 85 % load.

- Approved: owner 2026-11-10 (section 11 answers: 1A CLOCK row AUTO/INTERNAL ONLY, 2A stop on clock loss, 3A PLAY again cancels wait, 4A host Start takes over, 5A PLAY LED mid-level + flash, 6A separate timing test build, accept LEAD 2.90 ms).
