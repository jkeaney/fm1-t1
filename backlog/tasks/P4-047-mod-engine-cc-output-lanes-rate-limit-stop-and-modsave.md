---
id: P4-047
title: MOD engine: CC output from the scheduler, lanes, rate limit, stop and mute, ModSave in presets
phase: P4
type: task
status: todo
priority: 8
executor: sonnet
depends_on: [P4-046, P4-042]
gate: none
---
## Goal
Make the modulation lanes audible: call the `AlgoDesc.mod` hook from the scheduler, compute the CC value of each active wire, and emit it through the existing event queue to the channel's device, with a rate limit, deterministic lanes, safe stop and mute behaviour, and `ModSave` stored in presets. This task touches the interrupt path, so the reviewer does a FULL review (the ISR checklist below), and one agent builds at a time.

Owner ruling 2026-11-13: "MOD output is IN v1." Lane forms and slot fields are as fixed by P4-045 / ADR-0008 (proposed: RAND = `sp18_cell`, ACC = cell/2 + `sp20_lean`, RAMP = `sp19_lfo`, WALK = interpolated `sp18_cell`).

## Read first
- docs/project/decisions/ADR-0008-rig-mod-v1.md and docs/design/hardware/RIG_MOD_V1.md (P4-045)
- docs/design/MODULATION.md sections 2 (remote control, what happens on stop), 3.1-3.4, 7, 9 (H-MOD tests)
- docs/design/algorithms/acid/SPEC.md section 2.1; firmware/src/sq_algo_acid.c `acid_mod`
- firmware/src/sq_prims.c ~120-150 (`sp18_cell`, `sp19_lfo`, `sp20_lean`, `sq_mod_lane`), sq_types.h (ModIdx, AlgoDesc.mod, PulseOut, SeqEvent, SQ_EVQ_N = 96)
- firmware/src/sq_core.c: `sq_emit` (~142), `sq_eval` (~261), `sq_tick` (~532), `sq_tick_pos`, `sq_all_off` (~443), `sq_emit_cc123`, the CC-lead/stamp rules (ADR-0003 sections on timing); ring full behaviour
- firmware/src/sq_chan.c: `ModSave` (~36-43, 13 B reserved zero), `sq_smod`, `sq_stage_smod`, `sq_preset_save` / `sq_preset_load`, UndoRec seeds (SQ_UNDO_SEEDS), `sq_reset_algo`
- firmware/src/sq_hw.c (P4-046) for `hw_scale`, `hw_default_slots`, the park value
- docs/design/knob_map.tsv PERF R1-R4 note "at most one per 10 ms"; HW_SUPPORT_LOWLEVEL.md 3.3 and 3.5 (ring thresholds 24 / 12 / 8 free entries, DIN needs 0.96 ms per CC)
- tests/sq_core_test.c, sq_link_test.c, host_sq_hooks.h; docs/project/decisions/ADR-0003 section on `sq_load_pending` and the epoch guard
- backlog/tasks/P4-042-*.md (the ISR bound it set for `sq_tie_end`) and P4-034-*.md (the other ISR queue consumer, as a task-file model)

## Deliverable
- `Slot` / `ModRT` state: per channel a device id, 4 slots (param, level, depth, form as ADR-0008 decides), the runtime (last value sent, pending select zone, park state) in `.bss`; main-loop API `sq_mod_set_dev`, `sq_slot_set(ch, i, param, level, depth, form)` (clamps; loader clamps), cap of 8 active slots (`MOD FULL: 8 ACTIVE`), single-byte or irq-guarded multi-byte hand-off to the ISR as for other main-loop state
- The scheduler calls `a->mod(...)` ONCE per sounded or held step from the ISR emit path (never from `uisq_view_fill`/`sq_eval` UI calls: the screen path must not pay for it), turns each active slot's lane into `v = clamp(LEVEL + lane x DEPTH / 127, lo, hi)` via `hw_scale`, and emits `CC` through `sq_emit` (a new `SQ_EV_CC` kind or the existing mechanism, per ADR-0008) on the channel's MIDI channel, stamped just ahead of its note (CC lead rules of ADR-0003), later CCs 1 ms apart. Dedupe: nothing is sent when `v` equals the last value sent. Ratchets and rolls share the step's value (no CC burst). Select parameters send only on a bar line and only on a zone change. A `PO_HOLD`, tie or silent step sends nothing new
- **Rate limit:** at most one CC per 10 ms per wire (HAND/ROW 3 sends included: a hand turn is coalesced to the latest value and sent on the next free 10 ms slot; the last value always arrives). The shared ring never starves note events: CCs are dropped before notes when the ring is short (thresholds per LOWLEVEL 3.3), a dropped CC is not retried stale, and a counter records drops
- **Lanes** RAND / WALK / RAMP / ACC computed only from `s_mod`, the slot number, the generator position `g`, the algorithm's `ModIdx` and the step velocity (MODULATION 3.2): same settings and seed give the same CC stream; a page that reads another page copies its lane; thinning the pattern never re-rolls the lane at the remaining steps
- **Restore and ranges:** every `v` is within 0..127 and within the parameter's lo / hi; a loaded or reset slot with an out-of-range param, level, depth or form is clamped or emptied; device NONE or all depths 0 sends nothing at all, so the byte stream of notes is identical to today (T-MOD1)
- **Mute and stop:** STOP, PANIC, mute and channel removal send each ACTIVE wire once to its park value (LEVEL by default; the profile's neutral value if the profile row says so) before or with the CC123 in `sq_all_off`, only for wires that were driven since the last park (no flood, no repeat); a wire made idle (depth 0) sends nothing and parks nothing; no CC state hangs: after STOP then PLAY the first step re-sends its wire (the dedupe memory is cleared). Document in the Log what the device is left at
- **ModSave in presets:** `ModSave` (25 B, or the size ADR-0008 sets) now carries device and slots (the reserved 13 B), `sq_preset_save/load` round-trip them, validated and clamped; sibling array stays outside `SeqPreset`; `UndoRec` restores `s_mod` (already built) and the SHAPE dice re-rolls `s_mod`; slot edits (pick, level, depth) are NOT undoable (decision 100)
- Tests (host, wired in run_tests.sh): `tests/sq_mod_test.c` with T-MOD1 (all idle or device NONE: core event stream and the ACID note golden byte-identical to before), T-MOD2 (active slots: NOTE events identical to T-MOD1, only CC events added; the worked ACID/TB-3 example of MODULATION 4.1 as a fixture with `sp18_cell` values from the code, golden `tests/mod_golden.txt`), T-MOD3 (primitives pure), T-MOD4 (SHAPE dice changes the lanes of exactly the active slots, undo restores them), rate-limit test (a fast hand sweep gives <= 1 CC per 10 ms per wire and the final value arrives), ring-short test (CCs dropped before notes), stop/mute park test, save/load round trip, fuzz of random slot/param/level/depth/form values (no value out of range, no crash)
- **ISR cost bound, stated in the Log with the counting method:** operations per active slot per step (target <= about 25), worst case = 8 active slots x the step rate of the fastest division x ratchets (which add no CC), plus the cost of one `a->mod` call; report it next to the P4-042 `sq_tie_end` bound and the time budget of one control block (half buffer about 2.9 ms). If a measurement path exists after P4-019 use it; otherwise the bound is a static count and the Log says so
- ISR review checklist the reviewer must tick: no loop without a bound; no call from `.ram_text` code (the build checks it); no write to main-loop-owned state; single-byte or guarded multi-byte reads; ring-full handled; nothing allocates; deterministic

## Acceptance criteria
- [ ] T-MOD1..T-MOD4, rate-limit, ring-short, park and round-trip tests pass; the ACID note golden and the core goldens are unchanged
- [ ] With an active wire, the TB-3 CC stream for the MODULATION 4.1 example is in the golden (exact bytes) and is the same on every run
- [ ] Stop/mute leaves no hung state: after STOP every driven wire was parked once, after PLAY the first step re-sends
- [ ] The ISR cost bound (operations, method, worst case) is in the Log and is within ADR-0003's +5 % CPU allowance, or an FB is filed and the task is `blocked`
- [ ] `sh tools/check.sh` ends with `CHECK PASS` (the known failures listed in CLAUDE.md are not regressions)
- [ ] The build line `image ... RAM .data+.bss N of 73728; pool ...` is pasted into the Log; RAM headroom (73728 - N) is at least 8192 B, else stop and file an FB (E9 trigger)
- [ ] Reviewer's ISR checklist ticked in the review Log (full review, not a skim)

## Verification
```sh
sh tools/check.sh            # last line: CHECK PASS
./build.sh | grep 'RAM'      # paste into the Log
sh tests/run_tests.sh        # sq_mod_test, sq_link_test, sq_core_test, the ACID golden
```

## UX gate
n/a for the engine (no new control or screen); the player-visible behaviour is gated in P4-048 and P4-049 and heard in P4-050. The one visible effect, the toast `+ MOD` on a SHAPE dice with an active wire, is checked in P4-048.

## Out of scope
- ROW 3 knobs and the RIG page (P4-048, P4-049), device choice UI
- `mod` rows of algorithms other than ACID (P5-002..P5-008 add theirs; this task defines the contract and a mock algorithm test)
- Flash storage of presets (P7)
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log
- 2026-11-13: created. Owner ruling 2026-11-13: "MOD output is IN v1." and "full setup so that we have everything set up to be workable. with the Tb-3 setup as the first device that we fully support, and the next device we add will easily be added in via data and not code."
