---
id: P4-012
title: Cut-over R8: the new core plays, the T-1 pipeline, state and UI are deleted
phase: P4
type: task
status: todo
priority: 1
executor: sonnet
depends_on: [P4-006, P4-009, P4-011, P5-001]
gate: none
---
## Goal
Switch the device to the new sequencer: `events_block` calls `sq_block`, keys and MIDI IN go to `sq_live_*`, main.c calls `sq_boot`, ui_draw/ui_input route to ui_sq. Delete what the new core replaces (SYSTEM_MAP R8, ADR-0003 §13).

## Read first
- docs/project/decisions/ADR-0003-sequencer-core.md §7.2, §12, §13 (fate of each T-1 function), §16.9
- docs/design/SYSTEM_MAP.md §6 row R8
- firmware/src/seq.c (events_block, the t1 hooks), main.c, ui_draw.c, ui_input.c

## Deliverable
- audio.c `fm1_alnk0_irq` calls `sq_half_begin(t0)` once per half buffer with the TIMER4 time at the half start (sq_core.c line 96; P4-007 left it uncalled, so M2 stamps are not filtered until this lands); `fm1_ticks` clock-out and stamped release stay inactive-by-default until then
- seq.c `events_block` calls `sq_block` in place of `t1_block`; keyboard_block/MIDI IN call `sq_live_on/off`; main.c calls `sq_boot`; ui_draw_page/ui_input dispatch to ui_sq
- Delete torso_t1_pipeline.c/.h, torso_t1_state.c/.h, torso_t1_ui.c/.h, ui_t1.c, t1_glue.c, torso_t1.c/.h (micro queue, rings) and the `t1_mode`/`t1_off_req` plumbing; keep torso_t1_algo.c only for the functions ADR-0003 §13 marks Reuse (bjorklund, rotate, pace curve, scale masks, fixed phrase contour) and torso_t1_port.c
- torso_phase4/5 tests deleted; phase1-3 tests deleted or replaced by the sq_* tests (Log lists which); tests/t1_link_test.c replaced by tests/sq_link_test.c (2 bars of ACID on channel 1 reach midi_out_event; STOP sends offs then CC123)

- (P4-009 review) define `SQ_IS_MASTER()` as `clkx_master()` (clk_ext.c) and act on the clk_ext action bits (Start/Continue/SPP, source changes) in the cut-over; `grep -n SQ_IS_MASTER firmware/src` shows the new definition

## Acceptance criteria
- [ ] `grep -n sq_half_begin firmware/src/audio.c` shows the call in the half-buffer interrupt
- [ ] `grep -rn 't1_block\|t1_mode\|ui_t1' firmware/src` returns nothing
- [ ] sq_link_test passes
- [ ] Host render boots into the PERF page stub
- [ ] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [ ] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
```

## UX gate
n/a, internal work (tooling, plumbing, docs, process); no producer-visible control, screen or sound change.

## Out of scope
- Page behaviour beyond stubs
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
