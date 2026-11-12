---
id: P4-012
title: Cut-over R8: the new core plays, the T-1 pipeline, state and UI are deleted
phase: P4
type: task
status: done
priority: 1
executor: sonnet
depends_on: [P4-006, P4-009, P4-011, P5-001, P4-026, P4-027, P4-028, P4-029, P4-030]
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
- seq.c `events_block` calls `sq_block` in place of `t1_block`; keyboard_block/MIDI IN call `sq_live_on/off`; main.c calls `sq_boot` and the main loop calls `sq_chan_sync()` once per frame, before the UI runs (ADR-0003 section 6); ui_draw_page/ui_input dispatch to ui_sq (the screen v2 of ADR-0006 / SCREEN_SPEC_V2, built by P4-026..P4-030, not the old zones)
- Delete torso_t1_pipeline.c/.h, torso_t1_state.c/.h, torso_t1_ui.c/.h, ui_t1.c, t1_glue.c, torso_t1.c/.h (micro queue, rings) and the `t1_mode`/`t1_off_req` plumbing; keep torso_t1_algo.c only for the functions ADR-0003 §13 marks Reuse (bjorklund, rotate, pace curve, scale masks, fixed phrase contour) and torso_t1_port.c
- torso_phase4/5 tests deleted; phase1-3 tests deleted or replaced by the sq_* tests (Log lists which); tests/t1_link_test.c replaced by tests/sq_link_test.c (2 bars of ACID on channel 1 reach midi_out_event; STOP sends offs then CC123)

- (P4-009 review) define `SQ_IS_MASTER()` as `clkx_master()` (clk_ext.c) and act on the clk_ext action bits (Start/Continue/SPP, source changes) in the cut-over; `grep -n SQ_IS_MASTER firmware/src` shows the new definition

- (FB-014, P5-001 review) a null-row guard: `sq_rebuild` and every other caller of an `ALGOS[]` function pointer treats a row with a NULL `build`/`step` as an inert (silent) channel and never dereferences it, because rows 1-5 of `ALGOS[]` stay empty until P5-002..P5-004 and P5-007/P5-008 fill them; `sq_default_chan` may keep EUCLID/SNAKE/STAGE as the intended defaults. A host test selects each empty row, runs ticks, and checks no crash and no MIDI from that channel.

## Acceptance criteria
- [x] `grep -n sq_half_begin firmware/src/audio.c` shows the call in the half-buffer interrupt
- [x] `grep -rn 't1_block\|t1_mode\|ui_t1' firmware/src` returns nothing
- [x] sq_link_test passes
- [x] Host render boots into the PERF page stub
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
- Page behaviour beyond stubs
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
- 2026-11-12: BLOCKED by owner decision (design session): the cut-over would wire the old dense ui_sq layout; blocked until the screen design is locked (docs/design/ui_review/HANDOFF/07_ACCEPTANCE.md section D).
- 2026-11-12: UNBLOCKED after "Design locked: James Keaney 2026-11-12" (ADR-0006). Re-scoped: it wires the screen v2 (P4-026..P4-030), so it now depends on them; backlog next will not pick it before they are done.

### 2026-11-13 executor (sonnet)
Commands and decisive output:
```
sh tools/check.sh                       -> RAM .data+.bss 28888 B of 73728 ... CHECK PASS
./build.sh | grep RAM                   -> ok    image 174680 B; RAM .data+.bss 28888 B of 73728; pool 78520 B of 368640   (headroom 44840 B, above 8192; was 23844 B before)
grep -n sq_half_begin firmware/src/audio.c   -> 116:        sq_half_begin(t0);   (fm1_alnk0_irq, once per half buffer, before the block loop)
grep -rn 't1_block\|t1_mode\|ui_t1' firmware/src -> (nothing)
grep -n SQ_IS_MASTER firmware/src/felucca.c  -> 82:#define SQ_IS_MASTER() clkx_master()
build/host/sq_link_test                 -> sq_link: all checks ok (2 bars: channel-1 note-ons 23; empty ALGOS[1..5] rows: no crash, no note)
ui_test                                 -> ui: all passed   (test_sq_screen: boots into the PERF page stub, drawn from the first frame, keys silent, PLAY starts the transport, HOME held opens the MENU)
```
What changed:
- felucca.c includes sq_core.c, sq_chan.c, ui_sq.c, ui_sq_glue.c (new, replaces ui_t1.c); `SQ_IS_MASTER()` is `clkx_master()`. seq.c `events_block` runs `sq_block(seq_n)` when `sq_ready` (set last by `sq_boot`), else the pattern tracks as before (the host tests that never boot the core keep their goldens); `seq_start/seq_stop` call `sq_on_start/sq_on_stop`. MIDI IN notes go to `sq_live_on/off` (new, sq_core.c, ISR: selected channel `sq_sel`, note-off follows its note-on after the selection moved); keyboard keys stay silent (song.grid 2) until P4-013. main.c: `sq_boot()` replaces `t1_glue_init()`, `sq_frame()` (ui_sq_glue.c: `sq_chan_sync()` then `song.g[G_BPM]` <-> `sq_bpm`) runs once per frame before `ui_input()`. ui.c/ui_input.c/ui_draw.c dispatch to `ui_sq_poll/ui_sq_controls/ui_sq_screen`. audio.c calls `sq_half_begin(t0)`.
- clk_ext.c is fed (clkx_event for 0xF8..0xFC and 0xF2, clkx_poll per block, internal_only from the CLOCK setting): clock out is silent whenever a host clock is followed. The transport and tempo of a followed clock still come from midi_clock.c; the PLL outputs and CLKX_A_* actions are not used by the core yet: FB-017 item 1.
- FB-014 null-row guard: `sq_build_buf` zeroes the cache when `build` is NULL; `sq_step_start` treats `!a->step` as an inert step (clock runs, nothing evaluated or sent). `sq_gen_hit` and ui_sq.c already guarded. Test: tests/sq_link_test.c selects each empty row (1..5), morphs, dices, plays 2 bars, draws.
- Deleted: torso_t1.c/.h, torso_t1_pipeline.c/.h, torso_t1_state.c/.h, torso_t1_ui.c/.h, ui_t1.c, t1_glue.c. torso_t1_algo.c/.h trimmed to Bjorklund, rotate, pace curve, T1_SCALE_MASKS and the phrase contour (no CycleParams); torso_t1_port.c kept.
- Tests: deleted torso_phase1/3/4/5_test.c and t1_link_test.c, host_t1_hooks.h; torso_phase2_test.c cut down to the kept functions; new tests/host_sq_hooks.h (hostsim.c includes clk_ext.c and it after seq.c) and tests/sq_link_test.c (replaces t1_link_test.c); ui_test.c / ui_render.c moved from the T-1 state to the sequencer screen (test_sq_screen replaces test_t1_screen; the screen-off tests compare the sq channel state; the KNOB ACCEL test calls accel() directly because no knob is wired yet). GREY check accepts pure white 0xFFFF (SCREEN_SPEC_V2 section 3 pointer / bar / cap; G = 63, not 62).
- tests/target_budget.txt: `fm1_alnk0_irq` 17319 -> 62489 (BUDGET_UPDATE; the static estimate now includes the inlined sq_block / sq_dispatch loops that replaced t1_block; only that line kept, cpu_baseline.txt untouched). Real cost on the device is for P4-021/P4-022 to measure (felucca_dbg.max_us).
- web/emu/felucca_web.c include list updated (cannot be built here, no emcc).
Notes for the reviewer:
- A build made with the system python3 (no `.venv` first in PATH) renders the fonts a little differently and fails tests/v12_compare.py; always run build and tests with `PATH="$PWD/.venv/bin:$PATH"` (tools/check.sh does).
- CLAUDE.md still describes the T-1 files (architecture, T-1 sections, the phase-test recipe); I did not edit it.
- NOT wired (P4-013, see FB-017 item 2): SELECT adds a channel, M-key held 1.5 s removes / held for preview, white keys as steps, K1-K4 morphs, dice buttons; SELECT no longer sets the tempo (SET page, P4-016). Page behaviour is stubs, as the task's Out of scope says.
- UX: no device test was possible; "every change must be checked on a real FM-1" is P4-021 (HW-1).

### 2026-11-13 review (sonnet)
Verdict: PASS (gate none). Re-ran `sh tools/check.sh` -> CHECK PASS; `./build.sh` -> `image 174664 B; RAM .data+.bss 28888 B of 73728; pool 78520 B of 368640` (headroom 44840 B, above 8192).
Checked: sq_half_begin(t0) at audio.c:116 in fm1_alnk0_irq before the block loop; events_block -> sq_block gated by sq_ready, set last in sq_boot, which runs before audio_init; seq_start/seq_stop call sq_on_start/sq_on_stop (offs for every sounding slot incl. live notes, then CC123, 0xFC first); no stale t1_block/t1_mode/ui_t1/torso_t1_pipeline|state|ui refs in firmware, tests, web/emu (only comments naming t1_glue in sq_core.c and usb_ring_test.c); SQ_IS_MASTER() = clkx_master(); null-row guard in sq_build_buf (zeroes cache) and sq_step_start (inert, clock runs) and covered by sq_link_test (rows 1..5); deleted phase1/3/4/5 and t1_link tests cover removed code (micro queue, rings, pipeline, state, ui), replaced by sq_link_test/sq_core_test; phase2 kept for the retained functions.
Budget bump 17319 -> 62489: the metric is a static loop-weight count (x4 per nesting), not time, so the jump from inlined sq_dispatch (96-slot scans) / sq_tick loops is plausible but is NOT evidence of real-time safety. Worst-case reasoning: ticks/block <= ~1 even at high tempo; a tick where all 4 channels start a step costs ~3 sq_eval each, plus sq_tie_end (up to 64 evals) only for TIE steps; sq_dispatch is O(events x 96). Likely well under the 725 us block, and audio.c shed (85 % of half) is a backstop, but nothing was measured. This is correctly deferred to P4-019/P4-022 (G6, felucca_dbg.max_us); do not treat the bump as a measured pass.
External clock (FB-017 item 1): transport/tempo of a followed clock still run through midi_clock.c, which calls seq_start/stop -> sq_on_*, so a followed clock still plays; clock out is silenced via clkx_master(). PLL/SPP-Continue/tick stamps unused. Scoped to FB-017 (still todo, no P-id yet). Not a boot/safety risk for HW-1 on the internal clock; HW-1/HW-T must be run with CLOCK = INTERNAL and the external-clock result must not be read as a G-gate pass.
Non-blocking notes for later tasks: (1) FB-017 needs P-ids before HW-T is claimed complete. (2) Live notes share sq_rt[c].snd slots with the sequence, so a held MIDI IN note can be cut by a sequencer note on the selected channel (P4-013 decides). (3) CLAUDE.md still describes the T-1 architecture; schedule a docs task.
HW-1 boot risks: none found in code. Only ACID (row 0) has a build/step; channels 2-4 default EUCLID/SNAKE/STAGE are empty rows and muted, so silent by design. Keys, knobs, SELECT, dice are drained stubs until P4-013, which HW-1 already depends on transitively (P4-021 -> P4-020 -> P4-018 -> P4-015 -> P4-013). If a build is flashed earlier it will boot to the PERF stub and only play CH1 on PLAY.
