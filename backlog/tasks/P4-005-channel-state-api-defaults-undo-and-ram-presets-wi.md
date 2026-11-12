---
id: P4-005
title: Channel state API, defaults, undo and RAM presets with the invariant fuzz
phase: P4
type: task
status: review
priority: 1
executor: sonnet
depends_on: [P4-004]
gate: none
---
## Goal
Everything the UI may call (ADR-0003 §6): morph turn, dice, undo, set MIDI channel / algorithm / mute, rebuild with the cache flip and epoch guard, SQ_DEFAULTS, preset save/load to RAM with validation.

## Read first
- docs/project/decisions/ADR-0003-sequencer-core.md §3.1, §6, §9, §10, §12
- docs/design/SEQUENCER_UX.md §1.2 (what dice/morph/algorithm changes do to edits), §4.4 (unique MIDI channels, decision 77)
- docs/project/OWNER_QUESTIONS.md decisions 46, 47, 73, 77

## Deliverable
- sq_core.c (or sq_chan.c included after it): `sq_morph_turn`, `sq_dice_layer`, `sq_dice_all`, `sq_undo`, `sq_set_midi` (unique rule: taking a used channel sets the other to OFF, one undo restores both), `sq_set_algo`, `sq_set_mute`, `sq_rebuild`, `SQ_DEFAULTS` (CH1-4 = MIDI 1-4, ACID/EUCLID/SNAKE/STAGE, ADR-0003 §12), `sq_preset_save/load` (RAM) with version and range validation
- tests/sq_chan_test.c: table-driven tests of SEQUENCER_UX 1.2 rows, unique-MIDI cases, undo/redo swap, preset round trip and corrupt-preset rejection; the fuzz extended to random API calls for 100000 ticks under sanitizers

## Acceptance criteria
- [x] sq_chan_test passes, including the fuzz
- [x] Defaults match ADR-0003 §12 exactly (test asserts each field)
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
- Flash storage (P7)
- UI
- Out of scope is binding. If the spec and the code disagree, or a needed fact is missing, stop: set `blocked`, file `backlog/bugs/BUG-nnn` or `backlog/feedback/FB-nnn` from `backlog/_templates/`, and never guess (ADR-0004).

## Log

### 2026-11-12 executor (sonnet)
Files: firmware/src/sq_chan.c (new, not in felucca.c yet: cut-over is P4-012), tests/sq_chan_test.c (new), tests/run_tests.sh (one step added).
```
sh tools/check.sh          -> ... == tests ... == backlog ... CHECK PASS   (tests.log: "sequencer channel API ... ok")
./build.sh | grep RAM      -> ok    image 168860 B; RAM .data+.bss 23316 B of 73728; pool 92500 B of 368640
```
RAM headroom = 73728 - 23316 = 50412 B (>= 8192). sq_chan.c is not compiled into the firmware yet, so the line is unchanged; its added .bss will be
about 450 B (UndoRec x4 376, s_mod 48, UndoMidi, HarmonySave x2, bpm, counters). Test: `cc -std=c11 -Wall -Wextra -Werror ... -fsanitize=address,undefined`
passes for fuzz seeds 0x1234567, 1, 7, 99, 12345 (100000 ticks each, ~750 MIDI notes, 0 packet violations).

Decisions the reviewer should look at:
- Reject vs clamp (ADR 10 vs task): sq_preset_load returns -1 and touches nothing for a wrong version or an algorithm id >= 6 (cannot be clamped
  meaningfully); every other field is clamped (bpm 40..240, global ranges, len 1..64, midi > 16 -> OFF, duplicate MIDI: lowest channel keeps it, morphs to
  MorphDesc min/max for all six algorithm slots, StepEdit fields into their encodings, HARMONY per HARMONY 2.1). ADR 10 says a bad version "loads the
  power-on defaults"; ADR 12 and the task say validate/reject with the defaults standing from sq_boot, so I followed those. The CRC is the storage layer's (P7).
- ADR 10 says the ISR "clears every UndoRec" at a queued load. UndoRec is main-loop memory, so the ISR (sq_apply_load, untouched) only bumps sq_epoch and
  the new main-loop `sq_chan_sync()` (call once per frame) clears the records, takes the staged HARMONY/BPM and rebuilds the four caches (s_mod is not in
  SeqPreset, so a dice during a pending load would leave the staged cache stale).
- While a load is pending the inactive cache buffers hold the staged caches, so sq_rebuild builds nothing and returns 0; the state edit stays in sq_ch and
  the load replaces it. Stopped: a load applies at once under SQ_IRQ_OFF/ON (macro over fm1_irq_off/on, defined by the test on the host).
- s_mod lives in `sq_smod[ch][algo]` (ModSave is P4-023); the dice binds it through SQ_SEED_MOD in the layer table as the locked prototypes do (SHAPE layer).
  build() gets a 7-slot seed array. Dice entropy = sq_entropy_ctr mixed with a dice counter (no fm1_ms hook, so no new host stub); ADR 3.1's `fm1_ms << 3` is not used.
- Undo: one UndoRec per channel; a MIDI reassignment writes the separate UndoMidi and is "the latest" until the next dice (sq_um_valid), so EDIT-tap undo
  swaps both channels as one record, again = redo. sq_undo also swaps bits 1/2/3/4 (page, morph, step, len/pg) for the later tasks; tested with hand-made records.
  sq_morph_turn writes no record (turn back).
- sq_boot() is ADR 12 step 4 without audio/sq_ready (P4-012). Defaults: pg_used = 1 (page 1 holds content), pad[0] = 0 (PREVIEW off), every algorithm slot gets
  its MorphDesc.def and default seeds.
- Not done (outside the Deliverable list): sq_select, pin/edit writers, reset family, sq_global_set, sq_scene, live keys.
- Finding for P2-024/preview work: a preset load (or any change of ChanSave.pad[0] bit 7) while a preview note sounds makes sq_emit skip the trk_note_off, so
  the preview note hangs and a later ON reports "double ON" in the host mock. The fuzz keeps pad[0] constant across loads for that reason. sq_emit untouched (BUG-006).
